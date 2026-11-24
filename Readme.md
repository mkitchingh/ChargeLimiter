# ChargeLimiter

Control when your iPhone or iPad charges. ChargeLimiter supports charge limits,
temperature thresholds, battery history, and a floating status window. It is
inspired by [AlDente](https://apphousekitchen.com/) for macOS and was originally
developed by [lich4](https://github.com/lich4/ChargeLimiter).

[English](#chargelimiter) · [简体中文](#简体中文)

> **Language note:** This project was originally written in Chinese. This fork is
> maintained primarily in English. All Chinese-language updates in this fork are
> best-effort updates made with AI tools; they may contain errors or lag behind
> the English documentation. The original Chinese documentation is preserved at
> the bottom of this page.

## Development status

The **1.7.1 reliability changes have been merged into the source**, but there is
no new installable TrollStore package yet. Sections marked **1.7.1 development**
describe these source changes; the original upstream 1.7 packages do not include
them. See [PR #1](https://github.com/mkitchingh/ChargeLimiter/pull/1) and the
[validation and device-test checklist](RELIABILITY.md).

The policy, interface, and history tests and native arm64/arm64e syntax checks
have passed. Those checks do not produce an installer or establish device
compatibility. Building the package and testing it on devices are still required.

This fork focuses on predictable charging behavior and recovery:

- Check charging rules every five seconds, independently of screen refresh.
- Show whether a stop was requested, appears to have worked, or remains unconfirmed.
- Restore normal charging without having to uninstall the app.
- Pause automatic limiting when you use a manual charging or input-power control.
- Restore the original settings changed by Speedup Charging.
- Fix refresh-interval changes and external-battery history storage.

## Compatibility and installation

ChargeLimiter requires **TrollStore or a supported jailbreak**. It uses private
APIs and special entitlements, so ordinary IPA sideloading is not sufficient.

| Installation | Package |
| --- | --- |
| TrollStore | `.tipa` |
| Rootful jailbreak | `iphoneos-arm.deb` |
| Rootless jailbreak | `iphoneos-arm64.deb` |

Upstream targeted iOS 12–17.0 and reported testing on several iPhone 6, 7, X, and
11 configurations using Checkra1n, Unc0ver, Odyssey, Palera1n, Dopamine, or
TrollStore. Those reports apply to the original project, not to validation of this
fork. Support depends on the device, battery controller, OS version, and
installation method.

The original packages are available from
[upstream releases](https://github.com/lich4/ChargeLimiter/releases). They **do not
include this fork's fixes**. Follow the release-specific installation instructions;
upstream instructed TrollStore users to uninstall the older app before upgrading.

## Charging controls — 1.7.1 development

### Automatic limiting

Set a lower threshold for resuming charging and an upper threshold for stopping
it, then turn on **Enable**. The lower threshold must be below the upper threshold.
The default charge range is 20–80%.

| Mode | Behavior |
| --- | --- |
| **Plug and charge** | Allows charging after a cable reconnect when the charge and temperature rules permit it. Also supports resuming at the lower charge or temperature threshold. |
| **Edge trigger** | Uses the lower and upper charge thresholds to control charging while the device stays plugged in. A reconnect alone does not start charging above the lower threshold. |

With temperature control enabled, reaching the upper temperature threshold pauses
charging. Charging remains paused until the resume temperature is reached; a low
battery reading does not bypass an active temperature stop. In edge-trigger mode,
the lower charge threshold must also permit charging to resume.

The **Update frequency** setting controls the app and floating-window display.
The daemon's five-second charging check runs independently of that setting.

### Manual control and recovery

Using the **Charging** or **External connected** switch pauses automatic limiting
in this fork. This prevents an automatic rule from immediately undoing your
manual action. Turn **Enable** back on when you want automatic limiting to resume.

**Restore normal charging** turns automatic limiting off, requests external input
power, and clears charging inhibition. The app reports a rejected command rather
than silently treating it as successful. A successful request still depends on
the device's hardware accepting and applying the change.

Some devices can take up to 120 seconds to apply a charging change. The status
panel distinguishes a pending stop from an unconfirmed one. Check battery current
and the percentage trend as well as the switch position; the system charging icon
may not reflect charging inhibition.

### Charging methods

| Method | Purpose |
| --- | --- |
| **Charge inhibition** | Requests that the battery stop charging while external power remains available. |
| **Predictive charging inhibition** | Uses an alternative inhibition property supported by some newer devices. |
| **Disable inflow** | Cuts external input power, so the device draws from its battery. This differs from stopping battery charging. |
| **Limit inflow / thermal simulation** | Uses thermal-management settings to reduce power use and charging current. It can also reduce performance. |

Existing charging-method preferences are retained. Fresh configurations on older
iPhones use the legacy method; defaults for newer iPhones, iPads, and unknown
families are retained. An older iPhone with an unconfirmed stop can try the legacy
method from the status panel. The app does not silently switch methods or enable
input cutoff as a fallback.

### Speedup Charging

This optional feature can change airplane mode, Wi-Fi, Bluetooth, brightness,
automatic brightness, and Low Power Mode. The update saves the original values
once per session and restores them when charging stops, the feature is disabled,
power disconnects, normal charging is restored, or the daemon exits normally.
An abrupt process termination cannot run cleanup.

For an always-on relay or other device that must stay reachable, leave Speedup
Charging off unless you specifically want those connectivity and power changes.

## Status, history, and troubleshooting

The status panel explains the current charging decision and reports failed or
unconfirmed commands. A stopped reading is also treated as unconfirmed if the
battery percentage rises by two points after the request, even when the current
sensor reports zero.

Battery history includes five-minute, hourly, daily, and monthly views. Battery
health and capacity figures are estimates reported by the device. ChargeLimiter
does not repair a battery or guarantee that its reported health will stay fixed.

If charging continues beyond a limit or will not resume:

1. Use **Restore normal charging** to pause automatic limiting and request recovery.
2. Allow up to 120 seconds for the hardware to respond.
3. Check the status message, current, and percentage trend. A switch changing
   appearance is not sufficient proof that the hardware changed state.
4. If the device does not respond, test the available charging methods one at a
   time with automatic limiting paused. Hardware and replacement-battery support
   vary; do not assume that a method works because its API call was accepted.

When file access is available, the daemon log is at `/var/root/aldente.log`.
Include the device model, iOS version, installation method, app version, charging
method, and observed behavior when reporting a problem.

### Known limitations

- TrollStore cannot guarantee that the daemon restarts after iOS terminates it or
  after a reboot. Reopening the app attempts to reconnect and start the service.
- Floating-window support is unavailable on iOS 12 and earlier.
- On jailbroken devices, injected tweaks can interfere with the app or daemon.
- Thermal simulation can be overridden by iOS and may not persist while locked.
- Optimized Battery Charging can interact with ChargeLimiter's rules. The app has
  an option to disable it; review the system setting when you stop using the app.
- Some power banks turn off when the load is low. If the power bank disconnects,
  ChargeLimiter cannot make it supply power again. Consult the power bank's
  instructions for a low-current mode, if one is available.

## Shortcuts — 1.7.1 development

Use **Open URLs** in Shortcuts to open one of these URLs:

| URL | Action |
| --- | --- |
| `cl:///` | Open ChargeLimiter. |
| `cl:///exit` | Open the app, start the service if needed, then exit the interface. |
| `cl:///charge` | Request charging and pause automatic limiting. |
| `cl:///charge/exit` | Request charging, then exit the interface. |
| `cl:///nocharge` | Request a charging stop and pause automatic limiting. |
| `cl:///nocharge/exit` | Request a stop, then exit the interface. |

These URL commands use the live daemon so that manual actions and automatic
control stay consistent.

## HTTP API — 1.7.1 development

The daemon accepts JSON POST requests on the device at
`http://127.0.0.1:1230/bridge`. It binds to localhost; the examples must run on the
device, not on another computer on the network.

```sh
curl http://127.0.0.1:1230/bridge \
  -H 'Content-Type: application/json' \
  -d '{"api":"get_conf"}'
```

| API | Additional request fields | Result |
| --- | --- | --- |
| `get_conf` | Optional `key` | All configuration fields, or the requested value. The full response includes `data.control` with the controller state and reason. |
| `set_conf` | `key`, `val` | Update a configuration value. Invalid keys, types, or threshold combinations are rejected. |
| `reset_conf` | None | Reset the configurable defaults handled by the daemon. |
| `get_bat_info` | None | Battery telemetry in `data`, plus `data_ups` when available. |
| `get_statistics` | `conf` mapping each table to `n` and `last_id` | Stored battery history. |
| `set_charge_status` | Boolean `flag` | Pause automatic limiting and request charging on or off. |
| `set_inflow_status` | Boolean `flag` | Pause automatic limiting and request external input on or off. |
| `restore_charging` | None | Pause limiting and request normal charging and external input. |

Responses contain a numeric `status`: `0` means the request was accepted, and a
negative value indicates an error. Acceptance is not confirmation that the
hardware has finished applying the change. Status `-11` indicates an invalid
request; `-10` indicates an unknown API.

Common configuration fields:

| Key | Type | Meaning |
| --- | --- | --- |
| `enable` | Boolean | Enable automatic charge limiting. |
| `mode` | String | `charge_on_plug` or `edge_trigger`. |
| `charge_below` / `charge_above` | Integer | Lower and upper charge thresholds, in percent. |
| `enable_temp` | Boolean | Enable temperature-based control. |
| `charge_temp_below` / `charge_temp_above` | Number | Resume and stop temperatures in the configured unit. |
| `update_freq` | Number | Display refresh interval in seconds, from 1 to 600. |
| `floatwnd` / `floatwnd_auto` | Boolean | Floating-window visibility and automatic hiding. |
| `action` | String | `noti` for notifications, or an empty string for none. |
| `adv_prefer_smart` | Boolean | Prefer the SmartBattery service when available. |
| `adv_predictive_inhibit_charge` | Boolean | Use predictive charging inhibition. |
| `adv_disable_inflow` | Boolean | Enable automatic input-power cutoff. |
| `acc_charge` | Boolean | Enable Speedup Charging. |

The daemon's [request handler](ChargeLimiter/daemon.mm) defines the remaining
advanced fields. In battery telemetry, `CurrentCapacity` is a percentage,
`InstantAmperage` is in milliamps, and `Temperature` is in hundredths of a degree
Celsius. Positive battery current usually indicates charging; negative current
indicates discharge. Individual devices can report unreliable readings.

To request recovery:

```sh
curl http://127.0.0.1:1230/bridge \
  -H 'Content-Type: application/json' \
  -d '{"api":"restore_charging"}'
```

## Development and validation

With a C++11 compiler, Node.js 18 or later, and Python 3, run the regression
tests from the repository root:

```sh
git switch main
bash tests/run.sh
```

The original Xcode project depends on external MonkeyDev, Theos headers, and a
GCDWebServers framework. The old Theos Makefiles refer to the removed `main.mm`
and are not a supported build path for this branch. The CI workflow checks policy,
interface, and history behavior, and includes native syntax checks; it does not
produce an installable package. See [RELIABILITY.md](RELIABILITY.md) for the full
validation scope and device acceptance checklist.

Additional upstream development tools:

- [Device debugging](https://github.com/lich4/debugserver_azj)
- [Web interface debugging](https://github.com/lich4/inspectorplus)
- [TrollStore Remote Helper](https://github.com/lich4/TrollStoreRemoteHelper)

To contribute a translation, update `www/lang.json` and the relevant help file.
New interface text falls back to English when a translation is missing. When
changing features or English instructions, update the Chinese section on a
best-effort basis as well.

## Screenshots

These screenshots show the original upstream interface, before this fork's
reliability changes.

<details>
<summary>Show upstream screenshots</summary>

![Charging settings](screenshots/screenshots_en0.png)
![Advanced settings](screenshots/screenshots_en1.png)
![Battery history](screenshots/screenshots_en_stat0.png)

</details>

## Credits and license

Thanks to the original author, lich4, and the upstream contributors:

- Elfulanopr — app icon
- Olivertzeng — Traditional Chinese
- Nawaf — Arabic
- Trickbox0411 — Vietnamese
- InnovatorPrime — dark mode
- Cast — Shortcuts integration

See [ChangeLog.md](ChangeLog.md) for the upstream version history and this fork's
changes. ChargeLimiter is free, open-source software licensed under
[GPL-3.0](LICENSE).

---

## 简体中文

ChargeLimiter 用于控制 iPhone 或 iPad 的充电时机，支持电量上限、温度阈值、电池历史记录和悬浮状态窗口。项目受 macOS 版 [AlDente](https://apphousekitchen.com/) 启发，最初由 [lich4](https://github.com/lich4/ChargeLimiter) 开发。

[返回英文说明](#chargelimiter)

> **语言说明：** 本项目最初以中文编写。本分支目前以英文文档为主要维护版本，中文说明也会随功能和文档变化尽力更新。所有中文更新均借助 AI 工具完成，可能存在翻译错误或更新滞后；如有不一致，请以英文说明为准。

### 开发状态

**1.7.1 可靠性改进已合并到源代码中，但尚未提供新的 TrollStore 安装包。** 下文标注为“1.7.1 开发版”的内容描述这些源代码改进；原项目的 1.7 安装包不包含这些改动。详情见 [PR #1](https://github.com/mkitchingh/ChargeLimiter/pull/1) 和[验证范围及真机测试清单](RELIABILITY.md)。

充电策略、界面、历史记录测试及原生 arm64/arm64e 语法检查已通过。这些检查不会生成安装包，也不能证明所有设备均兼容。后续仍需构建安装包并进行真机测试。

本分支主要改善充电行为的可预测性和恢复能力：

- 后台每五秒检查一次充电规则，不受界面刷新频率影响。
- 区分“已请求停充”“电池数据表明已停充”和“尚未确认停充”。
- 提供“恢复正常充电”按钮，减少因控制状态异常而需要卸载应用的情况。
- 手动操作充电或输入电源开关时，暂停自动限制，避免自动规则立即撤销手动操作。
- 恢复“加速充电”修改前的系统设置。
- 修复刷新频率切换和外接电池历史记录存储问题。

### 兼容性与安装

ChargeLimiter 需要 **TrollStore 或受支持的越狱环境**。应用使用私有 API 和特殊权限，不能通过普通 IPA 侧载方式获得完整功能。

| 安装方式 | 安装包 |
| --- | --- |
| TrollStore | `.tipa` |
| 有根越狱 | `iphoneos-arm.deb` |
| 无根越狱 | `iphoneos-arm64.deb` |

原项目面向 iOS 12–17.0，并报告在部分 iPhone 6、7、X 和 11 上，使用 Checkra1n、Unc0ver、Odyssey、Palera1n、Dopamine 或 TrollStore 进行过测试。这些记录属于原项目，不代表本分支已完成相同验证。实际支持情况取决于设备、电池控制器、系统版本和安装方式。

原版安装包可从[原项目发布页](https://github.com/lich4/ChargeLimiter/releases)获取，**其中不包含本分支的修复**。请遵循对应版本的安装说明；原项目要求 TrollStore 用户在升级前先卸载旧版。

### 充电控制——1.7.1 开发版

#### 自动限制

设置恢复充电的下限和停止充电的上限，然后打开“启用”（Enable）。下限必须低于上限，默认电量范围为 20–80%。

| 模式 | 行为 |
| --- | --- |
| 插电即充（Plug and charge） | 重新连接充电线后，在电量和温度规则允许时充电；也可在达到电量下限或恢复温度时恢复充电。 |
| 边缘触发（Edge trigger） | 适用于持续连接电源的设备，按照电量上下限控制充电。电量高于下限时，仅重新插线不会启动充电。 |

启用温控后，达到温度上限会暂停充电，并等待温度降至恢复阈值；低电量不会绕过已经触发的温度暂停。边缘触发模式还需满足恢复充电的电量条件。

“更新频率”（Update frequency）仅影响应用和悬浮窗显示。后台每五秒执行一次的充电检查不受此设置影响。

#### 手动控制与恢复

在本分支中，操作“正在充电”（Charging）或“电源已连接”（External connected）开关会暂停自动限制，以免自动规则立即撤销手动操作。需要重新启用自动控制时，请再次打开“启用”。

“恢复正常充电”（Restore normal charging）会关闭自动限制、请求恢复外部输入电源，并清除充电抑制状态。命令被拒绝时，应用会报告错误。请求成功仍不等于硬件已经完成切换。

部分设备应用充电状态变更最多可能需要 120 秒。状态面板会区分等待中的请求和未能确认的停充。除开关位置外，还应观察电池电流和电量变化；系统充电图标可能无法准确反映停充状态。

#### 充电控制方式

| 方式 | 用途 |
| --- | --- |
| 停充（Charge inhibition） | 请求停止给电池充电，同时保留外部供电。 |
| 智能停充（Predictive charging inhibition） | 使用部分较新设备支持的另一种充电抑制属性。 |
| 禁流（Disable inflow） | 切断外部输入电源，使设备改由电池供电。这与单纯停止给电池充电不同。 |
| 限流／温度模拟（Limit inflow / thermal simulation） | 通过温度管理设置降低功耗和充电电流，也可能降低设备性能。 |

已保存的充电方式设置会保留。较旧 iPhone 的全新配置默认使用传统方式；较新 iPhone、iPad 和未知设备系列的默认方式保持不变。旧款 iPhone 无法确认停充时，可从状态面板尝试传统方式。应用不会静默切换充电方式，也不会自动开启禁流作为后备方案。

#### 加速充电

“加速充电”（Speedup Charging）是可选功能，可以修改飞行模式、Wi-Fi、蓝牙、亮度、自动亮度和低电量模式。更新后的实现会在每个充电会话开始时保存原始值，并在停充、关闭该功能、断开电源、恢复正常充电或后台正常退出时恢复。进程被突然终止时，清理逻辑无法执行。

对于需要持续联网的中继设备或其他常开设备，除非明确需要这些连接和电源设置变更，否则建议关闭加速充电。

### 状态、历史记录与故障排查

状态面板显示当前充电决策的原因，并报告失败或尚未确认的命令。即使电流传感器报告为零，如果停充请求后电量又上升了两个百分点，也会将停充视为未确认。

电池历史记录提供五分钟、每小时、每天和每月视图。健康度与容量均为设备报告的估算值。ChargeLimiter 不能修复电池，也不保证健康度读数保持不变。

如果电量超过上限仍在充电，或无法恢复充电：

1. 点击“恢复正常充电”，暂停自动限制并请求恢复。
2. 最多等待 120 秒，让硬件应用变更。
3. 检查状态消息、电流和电量趋势。仅开关外观改变不足以证明硬件已经切换。
4. 如果设备没有响应，在自动限制暂停时逐一测试可用的充电方式。不同硬件及更换后的电池支持情况不同，不能仅凭 API 返回成功判断兼容性。

在具备文件访问权限的环境中，可查看后台日志 `/var/root/aldente.log`。反馈问题时，请提供设备型号、iOS 版本、安装方式、应用版本、充电控制方式及实际观察到的行为。

#### 已知限制

- 在 TrollStore 环境下，iOS 终止后台进程或设备重启后，无法保证服务自动恢复。重新打开应用会尝试连接并启动服务。
- iOS 12 及更早版本不支持悬浮窗。
- 在越狱设备上，注入的插件可能干扰应用或后台服务。
- iOS 可能覆盖温度模拟设置，锁屏时也可能无法保持。
- 系统“优化电池充电”可能与 ChargeLimiter 的规则相互影响。应用提供关闭该设置的选项；停止使用应用时，请检查系统设置。
- 部分充电宝会在负载较低时自动关机。充电宝停止供电后，ChargeLimiter 无法让其重新输出电力。如设备提供小电流模式，请查阅其说明。

### 快捷指令——1.7.1 开发版

在“快捷指令”中添加“打开 URL”操作，并使用下列地址：

| URL | 操作 |
| --- | --- |
| `cl:///` | 打开 ChargeLimiter。 |
| `cl:///exit` | 打开应用，按需启动服务，然后退出界面。 |
| `cl:///charge` | 暂停自动限制并请求充电。 |
| `cl:///charge/exit` | 请求充电后退出界面。 |
| `cl:///nocharge` | 暂停自动限制并请求停充。 |
| `cl:///nocharge/exit` | 请求停充后退出界面。 |

这些 URL 命令通过正在运行的后台服务执行，使手动操作与自动控制保持一致。

### HTTP 接口——1.7.1 开发版

后台服务在设备本机的 `http://127.0.0.1:1230/bridge` 接受 JSON POST 请求。服务仅绑定 localhost，因此示例必须在设备本机执行，不能直接从同一网络的其他电脑访问。

```sh
curl http://127.0.0.1:1230/bridge \
  -H 'Content-Type: application/json' \
  -d '{"api":"get_conf"}'
```

| API | 额外请求字段 | 结果 |
| --- | --- | --- |
| `get_conf` | 可选的 `key` | 返回全部配置或指定值。完整响应的 `data.control` 包含控制器状态及原因。 |
| `set_conf` | `key`、`val` | 更新配置值；无效字段、类型或阈值组合会被拒绝。 |
| `reset_conf` | 无 | 重置后台负责的可配置默认值。 |
| `get_bat_info` | 无 | `data` 返回电池信息；如有外接电池，还会返回 `data_ups`。 |
| `get_statistics` | `conf`，将表名映射到 `n` 和 `last_id` | 返回已存储的电池历史记录。 |
| `set_charge_status` | 布尔值 `flag` | 暂停自动限制，并请求开启或关闭充电。 |
| `set_inflow_status` | 布尔值 `flag` | 暂停自动限制，并请求开启或关闭外部输入电源。 |
| `restore_charging` | 无 | 暂停限制，并请求恢复正常充电及外部输入电源。 |

响应中的数值 `status` 为 `0` 时表示请求已被接受，负数表示错误。请求被接受不代表硬件已经完成切换。`-11` 表示请求无效，`-10` 表示未知 API。

常用配置字段：

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `enable` | 布尔值 | 启用自动充电限制。 |
| `mode` | 字符串 | `charge_on_plug` 或 `edge_trigger`。 |
| `charge_below` / `charge_above` | 整数 | 恢复和停止充电的电量阈值，单位为百分比。 |
| `enable_temp` | 布尔值 | 启用温度控制。 |
| `charge_temp_below` / `charge_temp_above` | 数值 | 按配置温度单位表示的恢复和停止温度。 |
| `update_freq` | 数值 | 界面刷新间隔，范围为 1–600 秒。 |
| `floatwnd` / `floatwnd_auto` | 布尔值 | 控制悬浮窗显示和自动隐藏。 |
| `action` | 字符串 | `noti` 表示发送通知，空字符串表示不执行通知。 |
| `adv_prefer_smart` | 布尔值 | 优先使用可用的 SmartBattery 服务。 |
| `adv_predictive_inhibit_charge` | 布尔值 | 使用智能停充方式。 |
| `adv_disable_inflow` | 布尔值 | 启用自动禁流。 |
| `acc_charge` | 布尔值 | 启用加速充电。 |

其余高级字段以[后台请求处理代码](ChargeLimiter/daemon.mm)为准。在电池数据中，`CurrentCapacity` 是电量百分比，`InstantAmperage` 的单位为毫安，`Temperature` 的单位为百分之一摄氏度。电池电流为正通常表示充电，为负表示放电；个别设备的读数可能不可靠。

请求恢复正常充电：

```sh
curl http://127.0.0.1:1230/bridge \
  -H 'Content-Type: application/json' \
  -d '{"api":"restore_charging"}'
```

### 开发与验证

准备 C++11 编译器、Node.js 18 或更高版本及 Python 3 后，在仓库根目录执行回归测试：

```sh
git switch main
bash tests/run.sh
```

原 Xcode 工程依赖外部安装的 MonkeyDev、Theos 头文件和 GCDWebServers 框架。旧 Theos Makefile 引用了已移除的 `main.mm`，本分支不将其作为受支持的构建方式。CI 工作流验证充电策略、界面和历史记录，并包含原生语法检查，但不会生成安装包。完整验证范围和真机验收清单见 [RELIABILITY.md](RELIABILITY.md)。

原项目的其他开发工具：

- [设备调试](https://github.com/lich4/debugserver_azj)
- [Web 界面调试](https://github.com/lich4/inspectorplus)
- [TrollStore Remote Helper](https://github.com/lich4/TrollStoreRemoteHelper)

贡献翻译时，请更新 `www/lang.json` 和对应帮助文件。缺失翻译的新界面文字会回退到英文。修改英文说明或功能说明时，也应尽力同步更新本节中文文档。

### 截图

以下为原项目界面截图，尚未包含本分支的可靠性改进。

<details>
<summary>显示原项目截图</summary>

![充电设置](screenshots/screenshots_cn0.png)
![高级设置](screenshots/screenshots_cn1.png)
![电池历史记录](screenshots/screenshots_cn_stat0.png)

</details>

### 致谢与许可证

感谢原作者 lich4 及原项目贡献者：

- Elfulanopr——应用图标
- Olivertzeng——繁体中文
- Nawaf——阿拉伯语
- Trickbox0411——越南语
- InnovatorPrime——深色模式
- Cast——快捷指令集成

原项目版本历史和本分支改动见 [ChangeLog.md](ChangeLog.md)。ChargeLimiter 是采用 [GPL-3.0](LICENSE) 许可证的免费开源软件。
