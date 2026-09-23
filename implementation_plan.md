# Implementation Plan: Project Overboss (Fallout 4 Internal Mod Menu / Trainer DLL)

Project Overboss is a high-performance, internal mod menu and memory trainer DLL for **Fallout 4 (x64)** built with modern **C++20**, **MinHook**, **DirectX 11**, and **Dear ImGui**. It supports both standalone `LoadLibrary` injection and native **F4SE** (Fallout 4 Script Extender) plugin loading.

---

## User Review Required

> [!IMPORTANT]
> **Compilation Environment**: Project Overboss targets Microsoft Visual C++ (MSVC) x64 with C++20 standard support. Building requires CMake 3.20+ and MSVC build tools (Visual Studio 2022 / Build Tools).
>
> **Dependency Strategy**: MinHook and Dear ImGui are managed via CMake `FetchContent`, ensuring zero manual external dependencies or submodule setup.
>
> **Target Executable Versions**: Compatible with both Fallout 4 Pre-Next-Gen (1.10.163) and Next-Gen (1.10.980 - 1.10.984+) via runtime Array-of-Bytes (AOB) pattern scanning and RIP-relative address resolution.

---

## Open Questions

None currently blocking. Default behavior:
- Keybind to toggle overlay: `VK_INSERT`
- Keybind for emergency unhook & panic ejection: `VK_END`
- Menu styling: Dark Pip-Boy Phosphor Green (`#33F259` accents over `#0D0E12` dark carbon background).

---

## Architecture & System Design

```
+---------------------------------------------------------------------------------+
|                               Fallout4.exe (x64)                                |
|                                                                                 |
|   +-----------------------+     +-------------------+     +-----------------+   |
|   | IDXGISwapChain (DX11) |     |  Creation Engine  |     |  Win32 Window   |   |
|   |  - Present (Idx 8)    |     |  Singletons & VMT |     |  - WndProc      |   |
|   |  - ResizeBuf (Idx 13) |     |  - g_player       |     |                 |   |
|   +-----------^-----------+     |  - ConsoleManager |     +--------^--------+   |
|               |                 +---------^---------+              |            |
+---------------|---------------------------|------------------------|------------+
                | Detour                    | AOB Scanned            | Subclassed
+---------------|---------------------------|------------------------|------------+
|               |                           |                        |            |
|       [dx11_hook.cpp]           [creation_bridge.cpp]      [dx11_hook.cpp]      |
|        (MinHook VMT)            (Console Exec & Ptrs)      (WndProc Detour)     |
|               |                           |                        |            |
|               +------------+              |             +----------+            |
|                            |              |             |                       |
|                     [gui.cpp (Pip-Boy)] <-+-------------+                       |
|                     - Phosphor Green ImGui Theme                                |
|                     - Input Isolation (WantCaptureMouse)                        |
|                     - Tabs: Vitals | Armory | Armor | Settl | Teleport          |
|                                                                                 |
|                     [memory.cpp]                                                |
|                     - Robust Wildcard AOB Scanner                               |
|                     - RIP-relative 32-bit displacement resolution               |
|                     - VirtualProtect Patch & Restore wrappers                   |
|                     - SEH __try / __except memory safety                        |
|                                                                                 |
|                     [features/]                                                 |
|                     - vitals.cpp: God Mode, Speed, Jump, Carry, NoClip          |
|                     - armory.cpp: Spawn Weapons, Legendary Mods, Ammo Patches   |
|                     - power_armor.cpp: Full Frames/Sets, Fusion Core Freeze     |
|                     - settlement.cpp: Caps, Bobby Pins, Batch Materials, Budget |
|                     - world.cpp: Teleport Hubs, Map Markers, Timescale          |
|                                                                                 |
|                     [dllmain.cpp]                                               |
|                     - Dual-mode: Native Injected Thread + F4SE Plugin Exports   |
+---------------------------------------------------------------------------------+
```

---

## Detailed Specifications

### 1. Target Binary Reconnaissance Strategy
- **AOB Signature Scanning**:
  Pattern searcher scanning `.text` and `.rdata` sections of `Fallout4.exe`.
  - `PlayerCharacter` singleton resolution signature:
    `48 8B 05 ? ? ? ? 48 8B D9 48 85 C0 74 ? 48 8B` (RIP displacement at offset `+3`, instruction length `7`).
  - `ConsoleManager::ExecuteCommand` entry point signature:
    `48 89 5C 24 ? 57 48 83 EC ? 48 8B FA 48 8B D9 48 85 D2`
- **RIP-Relative Displacement Math**:
  $$\text{Target Address} = \text{Instruction Address} + \text{Instruction Length} + *(\text{int32\_t}*)(\text{Instruction Address} + \text{Displacement Offset})$$
- **Script Bridge vs Direct Memory Patching**:
  - Script Bridge leverages `ConsoleManager::ExecuteCommand(pConsoleMgr, strCommand, pTargetRef)`. This is the safest mechanism to deliver inventory items, add legendary modifications, unlock map markers (`tmm 1`), set actor values (`player.setav`), and toggle engine states without corrupting internal actor state machine tables.
  - Direct memory patching is utilized for freeze features (such as freezing fusion core drain or infinite ammo code caves/NOPs) with original byte caching for unhook restoration.

### 2. DirectX 11 SwapChain Hook & Input Isolation
- **Dummy Device Strategy**:
  On startup, create a hidden dummy Win32 window, invoke `D3D11CreateDeviceAndSwapChain`, and extract the `IDXGISwapChain` virtual method table pointer before destroying the dummy instance.
- **Hooked VMT Indices**:
  - Index 8: `IDXGISwapChain::Present`
  - Index 13: `IDXGISwapChain::ResizeBuffers`
- **Resource Management**:
  In `ResizeBuffers`, release `ID3D11RenderTargetView` references prior to calling original, preventing `DXGI_ERROR_INVALID_CALL`. Re-acquire in `Present`.
- **WndProc Subclassing & Input Isolation**:
  - When `bMenuOpen` is active: route events to `ImGui_ImplWin32_WndProcHandler`, enable `WantCaptureMouse` and `WantCaptureKeyboard`, call `ShowCursor(TRUE)`, and suppress game movement/shooting messages.
  - When closed: pass-through to original `WndProc` and call `ShowCursor(FALSE)`.

### 3. Pip-Boy UI & Theme System
- Dear ImGui configured with custom dark carbon palette (`#0D0E12`), glowing nuclear phosphor accents (`#33F259`), custom rounding, and high-visibility contrast.
- Modular tabs:
  1. **Vitals**: God Mode (`tgm`), Demi-God (`tim`), Speed Multiplier, Jump Height, Carry Weight, NoClip (`tcl`), Infinite AP.
  2. **Armory**: One-click weapon delivery (Fat Man, Gauss Rifle, Deliverer, Gatling Laser, etc.), Legendary effect attacher (Explosive, Two-Shot, Instigating, Furious), Ammo supply, Infinite ammo patch.
  3. **Power Armor**: Spawn full frames with T-45, T-51, T-60, X-01, and Hellfire armor sets, auto-install fusion cores, and freeze core drain.
  4. **Economy & Settlements**: Max Caps (`player.additem 0000000F`), Bobby Pins (`0000000A`), 5k Batch Shipments of all 29 crafting materials, Settlement build-budget bypass.
  5. **World / Teleport**: Fast travel coordinates to Diamond City, Sanctuary, Goodneighbor, The Prydwen, Railroad HQ, Glowing Sea, Institute; `tmm 1` (Show all map markers), Timescale slider.
  6. **Settings & Panic Eject**: Panic key (`VK_END`) triggering clean unhooking, VMT restore, memory patch revert, and `FreeLibraryAndExitThread`.

---

## Proposed Changes

### Core Engine & Memory Subsystem
- `src/engine/memory.h` & `src/engine/memory.cpp`: Pattern scanning, memory protection patching, RIP-relative addressing, and SEH wrappers.
- `src/engine/creation_bridge.h` & `src/engine/creation_bridge.cpp`: Fallout 4 engine bridges (`PlayerCharacter` singleton retrieval and `ConsoleManager::ExecuteCommand` execution).

### DirectX 11 Hooking & Input Dispatch
- `src/hooks/dx11_hook.h` & `src/hooks/dx11_hook.cpp`: Dummy device creation, MinHook detours on `Present` and `ResizeBuffers`, RTV life cycle, and `WndProc` detour.

### Graphical User Interface (Pip-Boy Phosphor Theme)
- `src/gui/gui.h` & `src/gui/gui.cpp`: Pip-Boy dark carbon & phosphor green style, tab bar navigation, interactive controls, notification toast overlay.

### Feature Modules
- `src/features/features.h`: Unified feature interface.
- `src/features/vitals.cpp`: God mode, demi-god, actor values modification, speedmult, jump height, and noclip.
- `src/features/armory.cpp`: Weapon spawner, legendary effect attacher, infinite ammo memory patch.
- `src/features/power_armor.cpp`: Spawning complete Power Armor frames, T-45/51/60/X-01/Hellfire component injection, fusion core charge freeze.
- `src/features/settlement.cpp`: Caps, lockpicks, 5000x bulk resource delivery, settlement budget bypass.
- `src/features/world.cpp`: Fast travel teleport locations (`coc`), map marker reveal (`tmm 1`), timescale controls.

### Entry Point & Build System
- `src/dllmain.cpp`: Dual-profile DLL entry point (native injection thread + F4SE plugin exports).
- `CMakeLists.txt`: CMake 3.20+ build script with C++20, MSVC configuration, FetchContent for MinHook and Dear ImGui.

---

## Verification Plan

### Automated Build Verification
1. Configure project with CMake:
   ```powershell
   cmake -B build -A x64 -DCMAKE_BUILD_TYPE=Release
   ```
2. Build the DLL target:
   ```powershell
   cmake --build build --config Release
   ```
3. Verify that `ProjectOverboss.dll` is produced, exporting `F4SEPlugin_Query` and `F4SEPlugin_Load`.

### Manual & Runtime Verification Instructions
1. **Standalone Injection Test**:
   - Inject `ProjectOverboss.dll` into `Fallout4.exe`.
   - Press `INSERT` to verify that the dark green Pip-Boy ImGui overlay opens, the Windows cursor appears, and game inputs are blocked.
   - Click feature toggles (Vitals, Armory, Settlements, etc.).
   - Press `INSERT` again to return control to the game.
2. **Panic Eject Test**:
   - Press `END` to trigger panic unhooking.
   - Verify that hooks are cleanly disabled and `Fallout4.exe` continues running without crashing.
3. **F4SE Plugin Deployment**:
   - Copy `ProjectOverboss.dll` to `<Fallout 4 Directory>/Data/F4SE/Plugins/`.
   - Launch via `f4se_loader.exe` and confirm plugin initializes automatically.
