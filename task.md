# Task Tracking: Project Overboss (Fallout 4 Internal Mod Menu / Trainer DLL)

## Phase 1: Setup & Planning
- [x] Create project directory structure and `.agent/skills/` configurations
- [x] Install & verify skill files (`fallout4-binary-reversing`, `creation-engine-memory-hooking`, `imgui-dx11-overlay-scaffold`)
- [x] Establish `implementation_plan.md` architecture and obtain review/approval

## Phase 2: Core Engine & Memory Subsystem
- [x] Implement `src/engine/memory.h` & `src/engine/memory.cpp`
  - [x] AOB signature scanner with wildcard support (`?` and standard hex format)
  - [x] RIP-relative address resolution helper
  - [x] Memory patching routines (`VirtualProtect`, `FlushInstructionCache`, original bytes backup & restore)
  - [x] SEH protected pointer dereferencing
- [x] Implement `src/engine/creation_bridge.h` & `src/engine/creation_bridge.cpp`
  - [x] Resolution for `PlayerCharacter` singleton (`g_player`)
  - [x] Resolution for `ConsoleManager` singleton and `ExecuteCommand` dispatcher
  - [x] Safe command execution queue and string sanitize routines

## Phase 3: DirectX 11 Hooking & Input Dispatch
- [x] Implement `src/hooks/dx11_hook.h` & `src/hooks/dx11_hook.cpp`
  - [x] Dummy swap chain creation (`D3D11CreateDeviceAndSwapChain`)
  - [x] MinHook detour for `Present` (index 8) and `ResizeBuffers` (index 13)
  - [x] Win32 `WndProc` detour with input isolation, cursor control, and `VK_INSERT` toggle
  - [x] Panic unhooking (`VK_END`) and clean resource deallocation

## Phase 4: Pip-Boy ImGui GUI & Feature Modules
- [x] Implement `src/gui/gui.h` & `src/gui/gui.cpp`
  - [x] Dark carbon matte and nuclear phosphor green Pip-Boy styling
  - [x] Category tabs: Vitals, Armory, Power Armor, Settlement/Economy, World/Teleport, System & Eject
- [x] Implement feature modules in `src/features/`
  - [x] `vitals.h` / `vitals.cpp`: God mode, demi-god, speed, jump height, carry weight, noclip
  - [x] `armory.h` / `armory.cpp`: Weapon delivery, infinite ammo, legendary effect applicator
  - [x] `power_armor.h` / `power_armor.cpp`: Frame spawns, T-45/51/60/X-01 suits, fusion core freeze
  - [x] `settlement.h` / `settlement.cpp`: Caps, bobby pins, resource shipments, budget limit unlock
  - [x] `world.h` / `world.cpp`: Fast travel/teleport hubs, map markers, timescale control

## Phase 5: Dual Entry Point & Build Automation
- [x] Implement `src/dllmain.cpp`
  - [x] Standalone DLL entry point with loader-lock safe thread bootstrap
  - [x] F4SE plugin exports (`F4SEPlugin_Query`, `F4SEPlugin_Load`)
  - [x] Graceful thread cleanup and panic shutdown logic
- [x] Implement `CMakeLists.txt`
  - [x] C++20 standard, MSVC flags, x64 architecture
  - [x] FetchContent configuration for MinHook and Dear ImGui
  - [x] Post-build output artifact packaging

## Phase 6: Verification & Walkthrough
- [x] Generate `walkthrough.md` with compilation commands, injection instructions, and verification steps
