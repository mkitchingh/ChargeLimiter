# ChargeLimiter fork review

Reviewed 2026-11-24T18:41:37.576248+00:00. Upstream reference: [lich4/ChargeLimiter v1.7 source](https://github.com/lich4/ChargeLimiter/tree/27d42eb1789744eee8e68cf69a806d2465bd2cd4).

## Recommendation

**Evaluate [tunecc/ChargeLimiter v1.17.5](https://github.com/tunecc/ChargeLimiter/releases/tag/v1.17.5) before investing further in our fork.** It is a substantial existing continuation with published TrollStore installers and source changes directly relevant to charging reliability. Keeping our smaller fork as a fallback is sensible until it has been tested on the actual iPhone 6s / iOS 15.8.4.

The published [TrollStore installer](https://github.com/tunecc/ChargeLimiter/releases/download/v1.17.5/ChargeLimiter_1.17.5_TrollStore.tipa) is **986,825 bytes**. Source declares minimum iOS 13.0 and documents TrollStore support and validation primarily on iOS 15–17. This is declared compatibility, **not independent proof of reliable operation on this particular 6s/battery**. Published release date: October 10, 2026; 40 releases in the inventory.

Use the battery compatibility test / charge-control probe to check real behavior. For the relay, start with capacity limiting and temperature protection; optional fast-charge, thermal simulation and full-charge scheduling are unnecessary initially. Observe whether the 50% stop and lower-threshold resume actually work while OpenBubbles remains usable. Record settings and restore normal charging/stop our controller before enabling the alternative: both use the same local service port (1230) and must not compete.

## Scope and limits

- Recursively enumerated all public forks returned by GitHub, including nested forks, then listed **every returned branch** and releases for each.
- **71 fork entries**, **69 accessible repositories**, **73 branches**.
- **12 external forks with commits not in the upstream reference**, plus our own fork.
- **56 accessible forks have no unique commits** on any listed branch. Recent timestamps alone were not counted as substantive development.
- Two entries returned 404 for branch listing and direct repository lookup: `wozuishuai2015/ChargeLimiter` and `zhyan1991/ChargeLimiter`. Their contents could not be inspected. GitHub still returned them in the fork network; this audit cannot determine why they were inaccessible.
- This covers the discoverable public fork network, not private forks or unrelated repositories copied without a fork relationship.
- Classification used branch comparisons, changed files, commit history, releases, and source inspection of the meaningful implementations. It is not a full security review or hardware validation of every line.
- Large compare summaries retain sampled commits/files; the audit workflow artifact retains the full fetched API responses.
- Ahead/behind counts describe Git history, not feature completeness. Some forks rewrote history: tunecc reports 371 ahead / 102 behind while its current source explicitly continues upstream v1.7. Do not interpret that as 102 missing upstream fixes.

[Successful inventory run and downloadable raw JSON artifact](https://github.com/mkitchingh/ChargeLimiter/actions/runs/45462192709). The read-only audit workflow and this report live on `research/fork-audit`, separate from application changes.

## Every external fork with unique commits

| Fork | Last push (UTC date) | Releases | Assessment |
|---|---|---:|---|
| [xurayg/ChargeLimiter](https://github.com/xurayg/ChargeLimiter) | 2026-10-15 | 0 | Native UIKit/diagnostics rewrite, persistent storage, startup/event/20-second control, separate process watchdog with lease/identity checks and bounded recovery, remembered charge/inflow decisions. Strong source of recovery ideas; no published release. Watchdog integration is broad and requires packaging, lifecycle and storage work. |
| [Zero-Day-IOS-Institute/ChargeLimiter](https://github.com/Zero-Day-IOS-Institute/ChargeLimiter) | 2026-11-22 | 0 | Daily upstream-sync workflow only; no app behavior changes. Nothing to adopt. |
| [ZHOUXIN-PM/ChargeLimiter](https://github.com/ZHOUXIN-PM/ChargeLimiter) | 2026-08-06 | 0 | RootHide/Relaxin packaging and launch compatibility on codex/relaxin-roothide-compat; main unchanged. Relevant only to jailbreak users, not this TrollStore relay. |
| [tunecc/ChargeLimiter](https://github.com/tunecc/ChargeLimiter) | 2026-10-10 | 40 | Strongest existing continuation: native UIKit, English/Chinese interface, predictive-inhibit fallback, resume verification, diagnostics/probe and battery compatibility test, persistent configuration, SQLite serialization, cleanup, and rootful/rootless/roothide/TrollStore releases. Recommend evaluating v1.17.5 before continuing custom development. |
| [tlsftlsftlsf/ChargeLimiter](https://github.com/tlsftlsftlsf/ChargeLimiter) | 2026-11-19 | 1 | RootHide/Dopamine iPhone X / iOS 16.7 compatibility, detection, packaging and daemon-launch work. One .deb release; no TrollStore release. Not a replacement for the non-jailbroken relay. |
| [MissThee/ChargeLimiter](https://github.com/MissThee/ChargeLimiter) | 2026-08-28 | 2 | Substantial tunecc-related continuation with v1.15.2/v1.15.3 releases; separate autoLaunch branch adds TrollStore wake/restart experiments using NEHotspotHelper callbacks, app timers and system notifications. These do not establish reliable Ethernet-only or reboot startup. Current tunecc is the stronger default candidate. |
| [Gary542784/chargelimiter](https://github.com/Gary542784/chargelimiter) | 2026-02-27 | 0 | Earlier native UIKit/custom HTTP server work, v1.9-era configuration persistence and layout fixes. Useful history, but no releases or clear advantage over current tunecc. |
| [gastontole/ChargeLimiter](https://github.com/gastontole/ChargeLimiter) | 2025-10-30 | 0 | One-file Spanish translation (www/lang.json). JSON parses; Spanish has all 129 English keys. Optional small cherry-pick; language quality was not independently reviewed. |
| [giorgiovipmassage-droid/ChargeLimiter](https://github.com/giorgiovipmassage-droid/ChargeLimiter) | 2026-03-06 | 0 | Three build/workflow commits; no charging runtime changes or releases. No reason to replace our validated build workflow. |
| [21dh114172/ChargeLimiter](https://github.com/21dh114172/ChargeLimiter) | 2025-12-07 | 0 | Main unchanged; v1.5 branch has nine unique commits, mostly Theos/SDK/build workflow work against older code. No charging-control improvement or releases. |
| [ALTNT/ChargeLimiter](https://github.com/ALTNT/ChargeLimiter) | 2026-06-19 | 0 | Dynamic thermal policy, screen-state logic, optional notifications and pocket-overheat mode, with later wakeup reductions. Not a fix for stop/resume reliability; adds policies this always-connected relay does not need. |
| [blucelebi/ChargeLimiter](https://github.com/blucelebi/ChargeLimiter) | 2026-08-15 | 0 | Native UI, diagnostics/export, database and jailbreak compatibility changes in the tunecc family. No releases and no demonstrated reason to prefer it over current tunecc. |

## Useful changes to carry over if we retain our fork

### 1. Independent process watchdog — highest-value recovery work

[xurayg commit e137c7e](https://github.com/xurayg/ChargeLimiter/commit/e137c7e1ed1d7e45d78171000c34ac5ac6cfbc5c) adds a second process that checks service health, verifies lease/process identity before terminating a stuck service, applies bounded restart backoff, and attempts normal-charging recovery. Its daemon explicitly starts the watchdog on TrollStore.

Our five-second timer can correct a missed battery event while our daemon runs; it cannot revive its own dead or hung process. That makes a separate watchdog a meaningful additional improvement. **Port deliberately, not as a blind cherry-pick:** this commit depends on the preceding native UI/storage/diagnostics rewrite, and both the installer and lifecycle/disable behavior need to account for the extra executable. Neither process is a guaranteed survivor of iOS killing background processes, and it does not establish reboot autostart.

Source: [watchdog.mm](https://github.com/xurayg/ChargeLimiter/blob/6214ffe528c4bc2ada5c64f625c7c5acb2da5bc4/ChargeLimiter/watchdog.mm), [daemon.mm](https://github.com/xurayg/ChargeLimiter/blob/6214ffe528c4bc2ada5c64f625c7c5acb2da5bc4/ChargeLimiter/daemon.mm).

### 2. Remember successful stop/resume decisions across daemon restarts

[xurayg commit 91b8bd8](https://github.com/xurayg/ChargeLimiter/commit/91b8bd86e6b15d761cb715c6db22028a2953843a) persists the last successful charge/inflow decisions, so a restart while capacity lies between the lower and upper thresholds can restore the prior decision instead of choosing a default. Useful with a watchdog and on a relay meant to remain plugged in. Depends on that fork's storage and control-plan implementation; adapt and test stale-state/reset behavior.

### 3. Actual compatibility testing and control-path fallback

[tunecc daemon source](https://github.com/tunecc/ChargeLimiter/blob/main/ChargeLimiter/daemon.mm) includes fallback from predictive inhibit when a write fails, verification when the stop does not take effect, and charging-enable verification/recovery. Its native UI also exposes a charge-control probe and battery compatibility test; see [CHANGELOG](https://github.com/tunecc/ChargeLimiter/blob/main/CHANGELOG.md).

These are valuable ideas for heterogeneous phones and replacement batteries. Our existing command checks/current observation/watchdog overlap part of this work. Keep model-specific delays: upstream and tunecc document charging-state changes taking up to 120 seconds on some devices, while one tunecc predictive fallback uses an eight-second check under specific conditions. Do not transplant timing assumptions wholesale.

### 4. Restore system Optimized Battery Charging settings

tunecc's `disable_smart_charge` recovery covers configuration changes, reset and startup self-healing; its reset/exit paths also restore charging and side effects. This is a worthwhile audit area in our fork, distinct from the basic charge-enable bit. Preserve the user's intended system setting rather than unconditionally forcing a new preference. The exact integration should follow our lifecycle rules.

### 5. Optional small translation cherry-pick

[gastontole commit 1228362](https://github.com/gastontole/ChargeLimiter/commit/122836229e25373760904e7be770bab38222ac30) adds Spanish in one JSON file. All 129 English keys exist in its Spanish locale and the JSON parses. This is the closest to a small standalone cherry-pick; new strings we added still require translation. Spanish prose was not independently validated.

### Lower priority / already addressed

- Database concurrency: tunecc serializes SQLite access; our recent main-loop serialization addresses the concurrency problem in our architecture. Avoid layering locks blindly.
- Packaging-only RootHide/Relaxin changes can help jailbreak users, but need separate build/install validation. They do not repair this non-jailbroken relay.
- ALTNT thermal/pocket policies are optional product features, not necessary charging reliability repairs.
- Native UI is a major project; if that is desired, using tunecc avoids repeating its rewrite.

## Why not choose the other large forks?

MissThee, blucelebi and Gary contain substantial related UI/control work, but current tunecc provides the clearest released continuation. MissThee's `autoLaunch` work deserves caution: the actual added code registers NEHotspotHelper for Wi-Fi callbacks, starts/checks the service from app callbacks, and reacts to system notifications. Those source paths are not evidence of reliable Ethernet-only recovery after a reboot. This user never intentionally reboots, so investing in this experimental startup mechanism is lower priority than verifying charging stop/resume.

At review time tunecc's open issue list returned one feature request for a manual temporary full-charge action. An absence of open charging bugs is not proof of correctness. Many of its tests are static source-contract checks; a release and passing CI do not substitute for observing the user's actual device.

## Complete public fork inventory

All entries returned by the traversal are below, including unchanged and inaccessible entries. Counts are against the upstream reference above. Branch notation is `name (+ahead/-behind)`; “unchanged” means no branch had unique commits. Last push is GitHub repository metadata, not necessarily a unique change by that fork owner.

| Repository | Last push | Every listed branch | Releases | Result |
|---|---|---|---:|---|
| [Wanglanhua450/ChargeLimiter](https://github.com/Wanglanhua450/ChargeLimiter) | 2024-10-09 | `main (+0/-0)` | 0 | No unique commits |
| [WhiteNightChan/ChargeLimiter](https://github.com/WhiteNightChan/ChargeLimiter) | 2024-10-10 | `main (+0/-0)` | 0 | No unique commits |
| [wozuishuai2015/ChargeLimiter](https://github.com/wozuishuai2015/ChargeLimiter) | 2024-03-01 | Unavailable | 0 | Inaccessible (404) |
| [xsw2009/ChargeLimiter](https://github.com/xsw2009/ChargeLimiter) | 2024-10-09 | `main (+0/-0)` | 0 | No unique commits |
| [xurayg/ChargeLimiter](https://github.com/xurayg/ChargeLimiter) | 2026-10-15 | `main (+4/-0)` | 0 | Unique commits; assessed above |
| [ytx0574/ChargeLimiter](https://github.com/ytx0574/ChargeLimiter) | 2024-03-25 | `main (+0/-29)` | 0 | No unique commits |
| [yuzhouheike/ChargeLimiter](https://github.com/yuzhouheike/ChargeLimiter) | 2024-02-18 | `main (+0/-71)` | 0 | No unique commits |
| [Zero-Day-IOS-Institute/ChargeLimiter](https://github.com/Zero-Day-IOS-Institute/ChargeLimiter) | 2026-11-22 | `main (+1/-0)` | 0 | Unique commits; assessed above |
| [ZHOUXIN-PM/ChargeLimiter](https://github.com/ZHOUXIN-PM/ChargeLimiter) | 2026-08-06 | `codex/relaxin-roothide-compat (+1/-0)`; `main (+0/-0)` | 0 | Unique commits; assessed above |
| [zhyan1991/ChargeLimiter](https://github.com/zhyan1991/ChargeLimiter) | 2026-09-21 | Unavailable | 0 | Inaccessible (404) |
| [zw12420682/ChargeLimiter](https://github.com/zw12420682/ChargeLimiter) | 2024-10-09 | `main (+0/-0)` | 0 | No unique commits |
| [Zzy0706/AlDente](https://github.com/Zzy0706/AlDente) | 2024-02-04 | `main (+0/-104)` | 0 | No unique commits |
| [tunecc/ChargeLimiter](https://github.com/tunecc/ChargeLimiter) | 2026-10-10 | `main (+371/-102)` | 40 | Unique commits; assessed above |
| [mkitchingh/ChargeLimiter](https://github.com/mkitchingh/ChargeLimiter) | 2026-11-24 | `main (+20/-0)`; `research/fork-audit (+22/-0)` | 0 | Our fork |
| [mraxu/AlDente](https://github.com/mraxu/AlDente) | 2024-02-04 | `main (+0/-104)` | 0 | No unique commits |
| [natpacket/ChargeLimiter](https://github.com/natpacket/ChargeLimiter) | 2024-10-17 | `main (+0/-0)` | 0 | No unique commits |
| [olivertzeng/AlDente](https://github.com/olivertzeng/AlDente) | 2024-02-13 | `main (+0/-81)` | 0 | No unique commits |
| [ouaisun/ChargeLimiter](https://github.com/ouaisun/ChargeLimiter) | 2024-10-09 | `main (+0/-0)` | 0 | No unique commits |
| [piowind/AlDente](https://github.com/piowind/AlDente) | 2024-02-04 | `main (+0/-104)` | 0 | No unique commits |
| [powerjiayun/ChargeLimiter](https://github.com/powerjiayun/ChargeLimiter) | 2024-10-09 | `main (+0/-0)` | 0 | No unique commits |
| [qqizai/AlDente](https://github.com/qqizai/AlDente) | 2024-02-04 | `main (+0/-104)` | 0 | No unique commits |
| [qxmao/ChargeLimiter](https://github.com/qxmao/ChargeLimiter) | 2024-02-24 | `main (+0/-63)` | 0 | No unique commits |
| [romlayvn-0411/ChargeLimiter](https://github.com/romlayvn-0411/ChargeLimiter) | 2024-11-08 | `main (+0/-0)` | 0 | No unique commits |
| [samuridream/ChargeLimiter](https://github.com/samuridream/ChargeLimiter) | 2024-10-09 | `main (+0/-0)` | 0 | No unique commits |
| [StephenLovino/ChargeLimiter](https://github.com/StephenLovino/ChargeLimiter) | 2024-10-09 | `main (+0/-0)` | 0 | No unique commits |
| [thespace2214/ChargeLimiter](https://github.com/thespace2214/ChargeLimiter) | 2024-06-17 | `main (+0/-6)` | 0 | No unique commits |
| [tlsftlsftlsf/ChargeLimiter](https://github.com/tlsftlsftlsf/ChargeLimiter) | 2026-11-19 | `main (+1/-0)` | 1 | Unique commits; assessed above |
| [tmasiff/ChargeLimiter](https://github.com/tmasiff/ChargeLimiter) | 2024-02-14 | `main (+0/-76)` | 0 | No unique commits |
| [Torrekie/ChargeLimiter](https://github.com/Torrekie/ChargeLimiter) | 2024-10-09 | `main (+0/-0)` | 0 | No unique commits |
| [tristan-k/ChargeLimiter](https://github.com/tristan-k/ChargeLimiter) | 2024-10-09 | `main (+0/-0)` | 0 | No unique commits |
| [MissThee/ChargeLimiter](https://github.com/MissThee/ChargeLimiter) | 2026-08-28 | `autoLaunch (+316/-102)`; `main (+283/-102)` | 2 | Unique commits; assessed above |
| [Gary542784/chargelimiter](https://github.com/Gary542784/chargelimiter) | 2026-02-27 | `main (+22/-0)` | 0 | Unique commits; assessed above |
| [gastontole/ChargeLimiter](https://github.com/gastontole/ChargeLimiter) | 2025-10-30 | `main (+1/-0)` | 0 | Unique commits; assessed above |
| [giorgiovipmassage-droid/ChargeLimiter](https://github.com/giorgiovipmassage-droid/ChargeLimiter) | 2026-03-06 | `main (+3/-0)` | 0 | Unique commits; assessed above |
| [hellaf1ush/ChargeLimiter](https://github.com/hellaf1ush/ChargeLimiter) | 2024-04-29 | `main (+0/-13)` | 0 | No unique commits |
| [hwl513782273/AlDente](https://github.com/hwl513782273/AlDente) | 2024-02-03 | `main (+0/-105)` | 0 | No unique commits |
| [iherwo/ChargeLimiter](https://github.com/iherwo/ChargeLimiter) | 2024-03-01 | `main (+0/-47)` | 0 | No unique commits |
| [jerrycen/ChargeLimiter](https://github.com/jerrycen/ChargeLimiter) | 2024-10-09 | `main (+0/-0)` | 0 | No unique commits |
| [JiayunPower/AlDente](https://github.com/JiayunPower/AlDente) | 2024-05-02 | `main (+0/-13)` | 0 | No unique commits |
| [jin2005-issues/ChargeLimiter](https://github.com/jin2005-issues/ChargeLimiter) | 2024-10-09 | `main (+0/-0)` | 0 | No unique commits |
| [Jook3r/ChargeLimiter](https://github.com/Jook3r/ChargeLimiter) | 2024-10-09 | `main (+0/-0)` | 0 | No unique commits |
| [JustF1ne01/ChargeLimiter](https://github.com/JustF1ne01/ChargeLimiter) | 2024-04-11 | `main (+0/-22)` | 0 | No unique commits |
| [KapitanPirata/ChargeLimiter](https://github.com/KapitanPirata/ChargeLimiter) | 2024-06-30 | `main (+0/-5)` | 0 | No unique commits |
| [Kids91/ChargeLimiter](https://github.com/Kids91/ChargeLimiter) | 2024-10-09 | `main (+0/-0)` | 0 | No unique commits |
| [lgq2015/ChargeLimiter](https://github.com/lgq2015/ChargeLimiter) | 2024-07-01 | `main (+0/-5)` | 0 | No unique commits |
| [lixuqiang888/chargelimiter](https://github.com/lixuqiang888/chargelimiter) | 2024-10-09 | `main (+0/-0)` | 0 | No unique commits |
| [LK26/ChargeLimiter](https://github.com/LK26/ChargeLimiter) | 2024-03-30 | `main (+0/-26)` | 0 | No unique commits |
| [meiizyyyy/ChargeLimiter](https://github.com/meiizyyyy/ChargeLimiter) | 2024-07-01 | `main (+0/-5)` | 0 | No unique commits |
| [MiracleF/ChargeLimiter](https://github.com/MiracleF/ChargeLimiter) | 2024-10-09 | `main (+0/-0)` | 0 | No unique commits |
| [11265/ChargeLimiter](https://github.com/11265/ChargeLimiter) | 2024-10-09 | `main (+0/-0)` | 0 | No unique commits |
| [21dh114172/ChargeLimiter](https://github.com/21dh114172/ChargeLimiter) | 2025-12-07 | `main (+0/-0)`; `v1.5 (+9/-26)` | 0 | Unique commits; assessed above |
| [6a6179/ChargeLimiter](https://github.com/6a6179/ChargeLimiter) | 2024-10-09 | `main (+0/-0)` | 0 | No unique commits |
| [98cy/ChargeLimiter](https://github.com/98cy/ChargeLimiter) | 2024-10-09 | `main (+0/-0)` | 0 | No unique commits |
| [ALTNT/ChargeLimiter](https://github.com/ALTNT/ChargeLimiter) | 2026-06-19 | `main (+5/-0)` | 0 | Unique commits; assessed above |
| [aoconch/ChargeLimiter](https://github.com/aoconch/ChargeLimiter) | 2024-10-09 | `main (+0/-0)` | 0 | No unique commits |
| [apploHah/ChargeLimiter](https://github.com/apploHah/ChargeLimiter) | 2024-10-09 | `main (+0/-0)` | 0 | No unique commits |
| [benzhonglu5-dotcom/ChargeLimiter](https://github.com/benzhonglu5-dotcom/ChargeLimiter) | 2024-10-09 | `main (+0/-0)` | 0 | No unique commits |
| [blucelebi/ChargeLimiter](https://github.com/blucelebi/ChargeLimiter) | 2026-08-15 | `main (+287/-102)` | 0 | Unique commits; assessed above |
| [BoringApp/ChargeLimiter](https://github.com/BoringApp/ChargeLimiter) | 2024-10-09 | `main (+0/-0)` | 0 | No unique commits |
| [camenling/ChargeLimiter](https://github.com/camenling/ChargeLimiter) | 2024-03-02 | `main (+0/-46)` | 0 | No unique commits |
| [capslock199299/ChargeLimiter](https://github.com/capslock199299/ChargeLimiter) | 2024-10-09 | `main (+0/-0)` | 0 | No unique commits |
| [chenwq8/ChargeLimiter](https://github.com/chenwq8/ChargeLimiter) | 2024-02-20 | `main (+0/-69)` | 0 | No unique commits |
| [dodooooo/ChargeLimiter](https://github.com/dodooooo/ChargeLimiter) | 2024-10-09 | `main (+0/-0)` | 0 | No unique commits |
| [doimty/ChargeLimiter](https://github.com/doimty/ChargeLimiter) | 2024-10-09 | `main (+0/-0)` | 0 | No unique commits |
| [DYY-Studio/ChargeLimiter](https://github.com/DYY-Studio/ChargeLimiter) | 2024-10-09 | `main (+0/-0)` | 0 | No unique commits |
| [e10514001/ChargeLimiter](https://github.com/e10514001/ChargeLimiter) | 2024-04-11 | `main (+0/-22)` | 0 | No unique commits |
| [FasaMo/AlDente_iOS](https://github.com/FasaMo/AlDente_iOS) | 2024-02-04 | `main (+0/-104)` | 0 | No unique commits |
| [FedHoo/ChargeLimiter](https://github.com/FedHoo/ChargeLimiter) | 2024-07-01 | `main (+0/-5)` | 0 | No unique commits |
| [FlechazoPh/AlDente](https://github.com/FlechazoPh/AlDente) | 2024-10-10 | `main (+0/-0)` | 0 | No unique commits |
| [FreZZyBuilds/ChargeLimiter](https://github.com/FreZZyBuilds/ChargeLimiter) | 2024-07-01 | `main (+0/-5)` | 0 | No unique commits |
| [funcard2008/ChargeLimiter](https://github.com/funcard2008/ChargeLimiter) | 2024-04-11 | `main (+0/-22)` | 0 | No unique commits |
