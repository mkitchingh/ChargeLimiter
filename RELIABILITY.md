# Reliability update

This branch keeps the existing TrollStore, rootful, rootless and floating-window
implementations. It does not install anything on a device automatically.

## Behavior

- A five-second daemon watchdog supplements IOKit notifications. Screen refresh
  frequency changes only the display, not enforcement.
- Battery reads from the interface no longer consume the controller's previous
  plug/unplug snapshot. Policy and HTTP requests run on the same run loop.
- The upper charge limit stops charging in both modes. Temperature stops remain
  latched until the resume temperature; a low charge level cannot bypass an
  enabled temperature stop. Edge mode still waits for its lower charge threshold.
- Accepted hardware writes get a full 120-second settling window before retrying;
  rejected writes retry no faster than every 15 seconds. The status panel shows
  command failures and continued charging after that window. Notifications follow
  observed current rather than merely submitting a command.
- Manual Charging and External Connected controls pause automatic limiting.
  Restore normal charging pauses limiting, requests external input and clears
  both charging-inhibit properties. The result is reported; hardware support is
  still required. Enable limiting again explicitly when ready.
- Speedup Charging snapshots settings once and restores the original values on
  stop, disconnect, disable, recovery, or normal daemon exit. Turning the feature
  off also restores settings. Abrupt process termination cannot run cleanup.
- Fresh configurations on pre-iPhone-8 hardware use the legacy charging default.
  Saved choices are retained. Newer iPhone, iPad and unknown-family defaults stay
  unchanged. An older iPhone that fails to stop gets an optional legacy-method
  action; there is no automatic method switch or automatic input cutoff.
- External-battery history identifiers are quoted, SQL preparation failures are
  checked, and UI retries no longer sleep on the main thread.
- Existing translations remain. New status text falls back to English.

## Validation

Run `bash tests/run.sh` with a C++11 compiler, Node 18+ and Python 3. Tests cover
the charge-threshold matrix, both charging modes, thermal hysteresis, old/new
model defaults, driver settling windows, current-based stop checks, refresh
changes, manual callbacks, recovery failures, and missing adapter data.

GitHub Actions runs these checks on Linux and macOS, plus arm64/arm64e native
syntax checks against the installed iOS SDK and pinned Theos/GCDWebServer headers.
Syntax checks do not produce a signed installable app or establish hardware
compatibility. The existing Xcode project still depends on the author's external
MonkeyDev, Theos and GCDWebServers framework setup. Its old Theos Makefiles refer
to the removed `main.mm` and are not a supported build route for this branch.

## Device acceptance checks before merging

1. On the iPhone 6s / iOS 15.8.4 TrollStore relay, verify a 40–50% range: stop at
   the upper threshold, remain stopped inside the band, resume at the lower one.
   Repeat while locked and with the interface closed. Ethernet/OpenBubbles
   should remain available with Speedup Charging off.
2. Test manual input off/on above the upper threshold. Confirm automatic limiting
   pauses and input stays restored. Test Restore normal charging after both
   charging inhibition and input cutoff. Confirm percentage/current behavior,
   not only the toggle state.
3. Test saved legacy and predictive methods on a newer iPhone and an iPad, plus
   existing rootless/rootful packages and an external battery if available.
4. Test temperature pause/resume, cable reconnects, missing notifications, and
   failed/unsupported hardware writes. Never label accepted writes as guaranteed
   charging control.
5. Confirm original brightness, automatic brightness, radios and Low Power Mode
   are restored after stopping or disabling Speedup Charging. Confirm the feature
   does not touch these settings when disabled.

TrollStore still cannot guarantee automatic daemon restart after iOS kills it or
the phone reboots. This update does not claim to remove that platform limitation.
