#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."

if [[ $(uname -s) != Darwin ]]; then
  echo 'This build requires macOS and Xcode with an iPhoneOS SDK.' >&2
  exit 1
fi
command -v ldid >/dev/null
sdk=$(xcrun --sdk iphoneos --show-sdk-path)
build_root="$PWD/.build/trollstore"
app="$build_root/Payload/ChargeLimiter.app"
headers="$PWD/.ci/include"
web="$PWD/.ci/webserver"
[[ -d .ci/headers/IOKit && -d "$web/GCDWebServer/Core" ]]
mkdir -p "$app/Base.lproj" "$app/Licenses" "$build_root/objects" "$headers/GCDWebServers" dist
ln -sfn "$PWD/.ci/headers/IOKit" "$headers/IOKit"
find "$web" -name '*.h' -exec ln -sf '{}' "$headers/GCDWebServers/" \;

# Build device code explicitly. The host runner's architecture is never inferred.
common=(-target arm64-apple-ios12.0 -isysroot "$sdk" -fobjc-arc -fblocks -Os
        -DNDEBUG -DNS_BLOCK_ASSERTIONS=1 -Wno-deprecated-declarations
        -I "$headers" -I "$headers/GCDWebServers")
objects=()
while IFS= read -r source; do
  name=$(basename "${source%.m}")
  object="$build_root/objects/$name.o"
  xcrun --sdk iphoneos clang "${common[@]}" -c "$source" -o "$object"
  objects+=("$object")
done < <(find "$web/GCDWebServer/Core" "$web/GCDWebServer/Requests" "$web/GCDWebServer/Responses" -name '*.m' | sort)

frameworks=(-framework Foundation -framework UIKit -framework CoreFoundation
            -framework CFNetwork -framework CoreServices -framework UserNotifications
            -framework Security -framework SystemConfiguration -lMobileGestalt
            ChargeLimiter/IOKit.tbd ChargeLimiter/BackBoardServices.tbd
            ChargeLimiter/GraphicsServices.tbd -lsqlite3 -lz)
link=(-Wl,-dead_strip -Wl,-ObjC -Wl,-rpath,@executable_path/Frameworks)
xcrun --sdk iphoneos clang++ "${common[@]}" -std=c++11 \
  ChargeLimiter/ui.mm ChargeLimiter/utils.mm "${frameworks[@]}" \
  -framework JavaScriptCore "${link[@]}" -o "$app/ChargeLimiter"
xcrun --sdk iphoneos clang++ "${common[@]}" -std=c++11 \
  ChargeLimiter/daemon.mm ChargeLimiter/utils.mm "${objects[@]}" \
  "${frameworks[@]}" "${link[@]}" -o "$app/ChargeLimiterDaemon"

for storyboard in Main LaunchScreen; do
  xcrun ibtool --compile "$app/Base.lproj/$storyboard.storyboardc" \
    "ChargeLimiter/Base.lproj/$storyboard.storyboard" \
    --minimum-deployment-target 12.0 --target-device iphone --target-device ipad
done
xcrun actool ChargeLimiter/Assets.xcassets --compile "$app" \
  --platform iphoneos --minimum-deployment-target 12.0 \
  --target-device iphone --target-device ipad --app-icon AppIcon \
  --output-partial-info-plist "$build_root/assets-info.plist"
cp -R www "$app/"
cp splash.png "$app/"
cp LICENSE "$app/Licenses/ChargeLimiter.txt"
cp "$web/LICENSE" "$app/Licenses/GCDWebServer.txt"

python3 - "$app" "$build_root" <<'PLIST'
import os, plistlib, sys
from pathlib import Path
app, root = map(Path, sys.argv[1:])
with open('ChargeLimiter/Info.plist', 'rb') as stream:
    info = plistlib.load(stream)
info.update({
    'CFBundleDevelopmentRegion': 'en',
    'CFBundleExecutable': 'ChargeLimiter',
    'CFBundleIdentifier': 'chaoge.ChargeLimiter',
    'CFBundleName': 'ChargeLimiter',
    'CFBundleDisplayName': 'ChargeLimiter',
    'CFBundlePackageType': 'APPL',
    'CFBundleShortVersionString': '1.7.1',
    'CFBundleVersion': '2.' + os.environ.get('GITHUB_RUN_NUMBER', '0'),
    'MinimumOSVersion': '12.0',
    'UIDeviceFamily': [1, 2],
    'UIRequiredDeviceCapabilities': ['arm64'],
    'CFBundleSupportedPlatforms': ['iPhoneOS'],
})
with open(root / 'assets-info.plist', 'rb') as stream:
    info.update(plistlib.load(stream))
with open(app / 'Info.plist', 'wb') as stream:
    plistlib.dump(info, stream)
PLIST

for executable in ChargeLimiter ChargeLimiterDaemon; do
  # Strip before signing; modifying a signed binary would invalidate its signature.
  xcrun strip -x "$app/$executable"
  ldid -SChargeLimiter/ChargeLimiter.entitlements "$app/$executable"
  ldid -e "$app/$executable" > "$build_root/$executable.entitlements.plist"
done
python3 - "$build_root" <<'ENTITLEMENTS'
import plistlib, sys
from pathlib import Path
root = Path(sys.argv[1])
for name in ['ChargeLimiter', 'ChargeLimiterDaemon']:
    with open(root / (name + '.entitlements.plist'), 'rb') as stream:
        entitlements = plistlib.load(stream)
    for key in ['platform-application', 'com.apple.private.security.no-sandbox',
                'com.apple.private.powersource-write']:
        assert entitlements.get(key) is True, (name, key)
ENTITLEMENTS

package="$PWD/dist/ChargeLimiter_1.7.1_test_arm64.tipa"
# Only the Payload tree is archived: no SDK, host tools, object files or dSYMs.
(cd "$build_root" && COPYFILE_DISABLE=1 zip -qry -9 "$package" Payload)
python3 scripts/validate-tipa.py "$package" > dist/build-report.json
shasum -a 256 "$package" > dist/SHA256SUMS.txt
cat dist/build-report.json
