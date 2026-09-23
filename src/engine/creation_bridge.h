#pragma once

#include <windows.h>
#include <cstdint>
#include <string>
#include <string_view>
#include <mutex>
#include <queue>

namespace Overboss::Engine {

    // Opaque structures representing Creation Engine types
    struct TESObjectREFR;
    struct PlayerCharacter;
    struct ConsoleManager;

    // Signature definition for ConsoleManager::ExecuteCommand
    // Prototype: void __fastcall ExecuteCommand(ConsoleManager* pThis, const char* command, TESObjectREFR* targetRef)
    using FnConsoleExecute = void(__fastcall*)(ConsoleManager* pThis, const char* command, TESObjectREFR* targetRef);

    class CreationBridge {
    public:
        static CreationBridge& Get() noexcept {
            static CreationBridge instance;
            return instance;
        }

        // Initialize pattern scans and resolve singletons/dispatchers
        bool Initialize();

        // Check if vital engine hooks/pointers are resolved
        [[nodiscard]] bool IsReady() const noexcept { return m_initialized; }

        // Pointer resolution
        [[nodiscard]] PlayerCharacter* GetPlayer();
        [[nodiscard]] ConsoleManager* GetConsoleManager();

        // Executes a console command synchronously or places it in the execution queue
        bool ExecuteCommand(std::string_view command, TESObjectREFR* target = nullptr);

        // Process pending queued console commands (called from main hook thread/Present)
        void ProcessQueue();

        // High-level Creation Engine action helpers
        void ToggleGodMode();
        void ToggleImmortal();
        void ToggleNoClip();
        void SetActorValue(std::string_view avName, float value);
        void ModActorValue(std::string_view avName, float delta);
        void AddItem(std::string_view formId, uint32_t count = 1);
        void PlaceAtMe(std::string_view formId, uint32_t count = 1);
        void TeleportCoc(std::string_view cellName);
        void UnlockMapMarkers();
        void SetTimescale(float scale);

    private:
        CreationBridge() = default;
        ~CreationBridge() = default;

        bool m_initialized = false;

        // Resolved pointers
        uintptr_t m_pPlayerCharacterPtr = 0;       // Address of g_player pointer
        uintptr_t m_pConsoleManagerPtr = 0;        // Address of g_consoleManager pointer
        FnConsoleExecute m_fnExecuteCommand = nullptr;

        // Thread-safe command queue
        std::mutex m_queueMutex;
        std::queue<std::pair<std::string, TESObjectREFR*>> m_commandQueue;
    };

} // namespace Overboss::Engine
