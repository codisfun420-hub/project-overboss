---
name: fallout4-binary-reversing
description: Reverse engineer Fallout4.exe (x64 Creation Engine) to identify RTTI classes, global singletons, virtual method tables, and generate resilient byte signatures (AOBs).
triggers:
  - reverse engineer fallout 4
  - pattern scan
  - find player character offset
  - fallout4 memory offsets
  - creation engine reverse engineering
---

# Fallout 4 Binary Reverse Engineering Skill

## Purpose
Enables the agent to inspect `Fallout4.exe`, extract offsets, locate engine singletons (`PlayerCharacter`, `ConsoleManager`), and generate resilient Array-of-Bytes (AOB) signatures compatible across game versions (1.10.163 Pre-Next-Gen through 1.10.984+ Next-Gen).

## Analysis Workflow

### 1. Engine Architecture & Memory Layout
- **Target Executable**: `Fallout4.exe` (PE32+ 64-bit).
- **Calling Convention**: Microsoft x64 (`rcx`, `rdx`, `r8`, `r9`, stack for parameters 5+).
- **Key Singletons to Locate**:
  - `PlayerCharacter` (`g_player`): Inherits from `TESObjectREFR`. Holds inventory, stat attributes, actor values, and equipment states.
  - `ConsoleManager`: Holds the execution dispatcher for engine console commands.
  - `BSInputManager`: Intercepts raw mouse/keyboard input before game camera movement.

### 2. Signature Scanning Strategy (AOB)
Never rely exclusively on raw hardcoded addresses. Write pattern scanning routines that support wildcard masks (`?` or `\x00`):
- Scan executable `.text` and `.rdata` sections.
- Identify the target instructions using RIP-relative addressing:
  $$\text{Effective Address} = \text{Instruction Pointer} + \text{Instruction Length} + \text{Displacement}$$

#### Critical AOB Reference Signatures:
- **`PlayerCharacter` Singleton Resolution**:
  Look for functions referencing actor values or player camera transitions:
  `48 8B 05 ? ? ? ? 48 8B D9 48 85 C0 74 ? 48 8B`
  Calculate rip-relative offset at index `+3`.
- **`ConsoleManager::ExecuteCommand` Dispatcher**:
  Target the entry point accepting `(const char* command, void* targetRef)`:
  `48 89 5C 24 ? 57 48 83 EC ? 48 8B FA 48 8B D9 48 85 D2`

### 3. Automated Reversing Scripts
When inspecting local files, use Python with `pefile` or `capstone` to extract headers, sections, and verify patterns:
```python
import pefile
import re

def find_pattern(pe_path, pattern, mask):
    pe = pefile.PE(pe_path)
    for section in pe.sections:
        if b'.text' in section.Name or b'.rdata' in section.Name:
            data = section.get_data()
            # Compile regex pattern from byte string and mask
            # Scan memory bounds and output relative virtual address (RVA)
            pass
```

### 4. Address Library Interop

When available, write lookup bindings targeting the Fallout 4 Address Library database (`versionlib-*.bin`) to resolve IDs dynamically across game updates without manual recompilation.
