#include <windows.h>
#include <chrono>
#include <thread>

#include "hooks/dx11_hook.h"
#include "engine/creation_bridge.h"

// F4SE Plugin Interface Definitions
struct PluginInfo {
    enum { kInfoVersion = 1 };
    uint32_t infoVersion;
    const char* name;
    uint32_t version;
};

static HMODULE g_hModule = nullptr;

DWORD WINAPI MainThread(LPVOID lpParam) {
    auto hModule = static_cast<HMODULE>(lpParam);

    // Allow game engine window and graphics pipeline to stabilize
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    // Initialize Creation Engine pattern scans and singletons
    Overboss::Engine::CreationBridge::Get().Initialize();

    // Hook DirectX 11 SwapChain Present & ResizeBuffers via dummy device
    if (!Overboss::Hooks::DX11Hook::Get().Initialize()) {
        return 1;
    }

    // Worker supervision loop: check for panic ejection hotkey (VK_END)
    while (!Overboss::Hooks::DX11Hook::Get().IsPanicRequested()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Gracefully shutdown hooks, release DirectX resources, and restore original WndProc
    Overboss::Hooks::DX11Hook::Get().Shutdown();

    // Wait to ensure any in-flight Present or WndProc calls exit cleanly
    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    // Zero-trace ejection: unmap DLL from Fallout4.exe process
    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

// -----------------------------------------------------------------------------
// F4SE Plugin Loader Exports
// -----------------------------------------------------------------------------
extern "C" __declspec(dllexport) bool F4SEPlugin_Query(const void* f4se, PluginInfo* info) {
    if (!info) return false;
    info->infoVersion = PluginInfo::kInfoVersion;
    info->name = "ProjectOverboss";
    info->version = 1;
    return true;
}

extern "C" __declspec(dllexport) bool F4SEPlugin_Load(const void* f4se) {
    // When loaded by f4se_loader.exe, spawn supervisor thread
    HANDLE hThread = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(MainThread), g_hModule, 0, nullptr);
    if (hThread) {
        CloseHandle(hThread);
        return true;
    }
    return false;
}

// -----------------------------------------------------------------------------
// Standard Injected DLL Entry Point
// -----------------------------------------------------------------------------
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            g_hModule = hModule;
            DisableThreadLibraryCalls(hModule);
            // Spawn worker thread to avoid loader lock deadlocks
            CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(MainThread), hModule, 0, nullptr);
            break;

        case DLL_PROCESS_DETACH:
            // Handled during clean shutdown sequence
            break;
    }
    return TRUE;
}
