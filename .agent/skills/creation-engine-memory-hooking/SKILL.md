---
name: creation-engine-memory-hooking
description: Safely hook x64 functions, DirectX 11 SwapChain VMT, and manage memory protection without crashing the Creation Engine.
triggers:
  - hook dx11 present
  - hook creation engine
  - minhook implementation
  - memory patch ammo
  - f4se plugin loader
---

# Creation Engine Memory Hooking Skill

## Purpose
Guides the agent through placing safe hooks inside `Fallout4.exe` using MinHook, hijacking the DX11 SwapChain virtual method table, patching opcodes cleanly, and structuring dual F4SE / Standalone DLL injection.

## Implementation Guidelines

### 1. DirectX 11 SwapChain Hooking
Hooking `IDXGISwapChain::Present` requires either:
1. Creating a temporary dummy device and swap chain via `D3D11CreateDeviceAndSwapChain` to grab the VMT base pointer.
2. Hooking target indices:
   - **Index 8**: `HRESULT __stdcall Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)`
   - **Index 13**: `HRESULT __stdcall ResizeBuffers(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)`

*Rule*: Inside `ResizeBuffers`, release all render target views (`ID3D11RenderTargetView`) before calling the original function to prevent `DXGI_ERROR_INVALID_CALL`. Recreate them in the next `Present` call.

### 2. Dual Injection Architecture
Support both load profiles in `dllmain.cpp`:
- **F4SE Plugin Mode**:
  ```cpp
  extern "C" __declspec(dllexport) bool F4SEPlugin_Query(const void* f4se, void* info);
  extern "C" __declspec(dllexport) bool F4SEPlugin_Load(const void* f4se);
  ```
- **Internal Native DLL**:
  On `DLL_PROCESS_ATTACH`, spawn a worker thread via `CreateThread`. Do not initialize DirectX or hook Windows APIs inside `DllMain` (loader lock avoidance).

### 3. Safe Memory Patching Rules
- Always use `VirtualProtect` to mark memory `PAGE_EXECUTE_READWRITE` before writing opcodes.
- Cache original bytes to allow clean, unhooked ejection via panic key.
- Flush instruction cache: `FlushInstructionCache(GetCurrentProcess(), address, size)`.
- Wrap dereferencing operations in structured exception handling (`__try / __except`) to survive null pointers during engine loading screens.
