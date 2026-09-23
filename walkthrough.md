# Walkthrough: Project Overboss (Fallout 4 Internal Mod Menu / Trainer DLL)

Project Overboss has been completely scaffolded with all production safety features, pattern scanning routines, DirectX 11 VMT hooks, Dear ImGui phosphor-green Pip-Boy theme, and dual F4SE / Standalone injection entry points.

---

## Project Structure Overview

```
project-overboss/
├── .agent/
│   └── skills/
│       ├── fallout4-binary-reversing/
│       │   └── SKILL.md
│       ├── creation-engine-memory-hooking/
│       │   └── SKILL.md
│       └── imgui-dx11-overlay-scaffold/
│           └── SKILL.md
├── src/
│   ├── dllmain.cpp                  # Dual entry point (F4SE plugin exports & Native DllMain)
│   ├── engine/
│   │   ├── memory.h / .cpp          # Wildcard AOB scanner, RIP-relative math, SEH safety
│   │   └── creation_bridge.h / .cpp # Fallout 4 PlayerCharacter & ConsoleManager bridges
│   ├── hooks/
│   │   └── dx11_hook.h / .cpp       # DX11 SwapChain VMT (Present/ResizeBuffers) & WndProc detour
│   ├── gui/
│   │   └── gui.h / .cpp             # Pip-Boy dark phosphor green theme & ImGui render loop
│   └── features/
│       ├── features.h               # Unified features interface
│       ├── vitals.cpp               # TGM, TIM, TCL, SpeedMult, JumpHeight, CarryWeight
│       ├── armory.cpp               # Weapon delivery, ammo supply, legendary effect attacher
│       ├── power_armor.cpp          # Chassis spawn, T-45/51/60/X-01 suits, fusion core freeze
│       ├── settlement.cpp           # Caps, Bobby Pins, 5k batch shipments, budget bypass
│       └── world.cpp                # Fast travel hubs (Diamond City, Prydwen, etc.), TMM 1, Timescale
├── CMakeLists.txt                   # CMake 3.20+ script with C++20 and FetchContent
├── task.md                          # Phase tracking checklist
└── implementation_plan.md           # Architecture design document
```

---

## 1. How to Compile the Project

Ensure you have **Visual Studio 2022** (or MSVC Build Tools with C++20 support) and **CMake 3.20+** installed.

Open PowerShell or Developer Command Prompt in the `project-overboss` folder:

```powershell
# 1. Configure the x64 build (FetchContent will automatically pull MinHook and Dear ImGui)
cmake -B build -A x64 -DCMAKE_BUILD_TYPE=Release

# 2. Compile the Release binary
cmake --build build --config Release
```

The compiled output will be generated at:
`build/Release/ProjectOverboss.dll`

---

## 2. Deployment & Injection Instructions

Project Overboss natively supports **two loading methodologies**:

### Method A: Native F4SE Plugin (Recommended)
1. Ensure [Fallout 4 Script Extender (F4SE)](https://f4se.silverlock.org/) is installed in your Fallout 4 root folder.
2. Navigate to `<Fallout 4 Root>/Data/F4SE/Plugins/` (create this folder if it does not exist).
3. Copy `ProjectOverboss.dll` into the `Plugins/` folder.
4. Launch the game using `f4se_loader.exe`.
5. F4SE will automatically call `F4SEPlugin_Query` and `F4SEPlugin_Load` upon boot.

### Method B: Standalone DLL Injection
1. Launch `Fallout4.exe`.
2. Use an x64 injector (such as Xenos, Process Hacker, or Cheat Engine) to inject `ProjectOverboss.dll` into the `Fallout4.exe` process.
3. The internal worker thread initializes safely outside of loader lock, waits for DirectX to initialize, and sets the VMT hooks.

---

## 3. In-Game Controls & Features

| Hotkey | Action | Description |
| :--- | :--- | :--- |
| `VK_INSERT` | **Toggle Menu** | Opens/closes the Dark Carbon Phosphor Green Pip-Boy overlay. Blocks game input and displays Windows cursor while open. |
| `VK_END` | **Panic Eject** | Instantly and cleanly removes all MinHook detours, restores `WndProc`, releases DirectX objects, and calls `FreeLibraryAndExitThread`. |

### Feature Tabs
- **Vitals & Physics**: Toggle God Mode (`tgm`), Demi-God (`tim`), No-Clip (`tcl`), speed multiplier, jump height, carry weight, and instant recovery (full heal, restore AP, cure rads).
- **Armory & Weapons**: Instant weapon injection (Deliverer, Fat Man, Gauss Rifle, Minigun, Gatling Laser), bulk ammo delivery (up to 10k per click), and legendary effect attacher (Explosive, Two Shot, Instigating, Furious, Never Ending).
- **Power Armor**: Spawn empty chassis/frames directly in front of the player, inject full armor sets (T-45, T-51, T-60, X-01), deliver fusion cores, and freeze core discharge.
- **Settlement & Economy**: Inject 50k+ Caps, 500 Bobby Pins, batch deliver 5,000x of all 22 crafting materials, and bypass workshop triangle/draw limits.
- **World & Teleport**: Instantly teleport (`coc`) to major Commonwealth hubs (Diamond City, Sanctuary, The Prydwen, Railroad HQ, The Institute, Glowing Sea), reveal all map markers (`tmm 1`), and adjust the daylight timescale.
- **System & Eject**: Live diagnostics of resolved engine pointers (`PlayerCharacter*`, `ConsoleManager*`) and one-click panic detachment.

---

## 4. Stability & Verification Notes

- **Input Isolation**: When the menu is active, all mouse and keyboard events are consumed by `HookedWndProc`, preventing accidental weapon firing or character movement while interacting with sliders.
- **Null Safety & SEH**: All memory scans and pointer dereferences are wrapped in `__try / __except` Structured Exception Handling to guarantee the game will not crash during loading screens or cell transitions.
- **DirectX Resize Handling**: `ID3D11RenderTargetView` is completely released inside `HookedResizeBuffers` before calling original, completely avoiding `DXGI_ERROR_INVALID_CALL` crashes when changing game resolution or toggling fullscreen.
