#include "creation_bridge.h"
#include "memory.h"
#include <format>
#include <iostream>

namespace Overboss::Engine {

    bool CreationBridge::Initialize() {
        if (m_initialized) return true;

        // 1. Scan for PlayerCharacter singleton pointer (g_player)
        // Pattern: 48 8B 05 ? ? ? ? 48 8B D9 48 85 C0 74 ? 48 8B
        auto playerScan = Memory::FindPattern("48 8B 05 ? ? ? ? 48 8B D9 48 85 C0 74 ? 48 8B");
        if (playerScan) {
            m_pPlayerCharacterPtr = Memory::ResolveRipRelative(playerScan.Get(), 3, 7);
        } else {
            // Secondary signature fallback across patches
            auto fallback = Memory::FindPattern("48 8B 05 ? ? ? ? 48 85 C0 74 ? 48 8B 88");
            if (fallback) {
                m_pPlayerCharacterPtr = Memory::ResolveRipRelative(fallback.Get(), 3, 7);
            }
        }

        // 2. Scan for ConsoleManager::ExecuteCommand
        // Target: 48 89 5C 24 ? 57 48 83 EC ? 48 8B FA 48 8B D9 48 85 D2
        auto execScan = Memory::FindPattern("48 89 5C 24 ? 57 48 83 EC ? 48 8B FA 48 8B D9 48 85 D2");
        if (execScan) {
            m_fnExecuteCommand = reinterpret_cast<FnConsoleExecute>(execScan.Get());
        } else {
            // Alternative signature
            auto altExec = Memory::FindPattern("40 53 48 83 EC 20 48 8B D9 48 8B FA 48 85 D2 74");
            if (altExec) {
                m_fnExecuteCommand = reinterpret_cast<FnConsoleExecute>(altExec.Get());
            }
        }

        // 3. Scan for ConsoleManager singleton pointer (g_consoleManager)
        auto consoleScan = Memory::FindPattern("48 8B 0D ? ? ? ? 48 85 C9 74 ? 48 8B 01 FF 50");
        if (consoleScan) {
            m_pConsoleManagerPtr = Memory::ResolveRipRelative(consoleScan.Get(), 3, 7);
        } else {
            auto altConsole = Memory::FindPattern("48 8B 0D ? ? ? ? E8 ? ? ? ? 48 85 C0 74 ? 48 8B C8 E8");
            if (altConsole) {
                m_pConsoleManagerPtr = Memory::ResolveRipRelative(altConsole.Get(), 3, 7);
            }
        }

        m_initialized = (m_pPlayerCharacterPtr != 0 || m_fnExecuteCommand != nullptr);
        return m_initialized;
    }

    PlayerCharacter* CreationBridge::GetPlayer() {
        if (!m_pPlayerCharacterPtr) return nullptr;
        uintptr_t pPlayer = 0;
        if (Memory::SafeRead(m_pPlayerCharacterPtr, pPlayer)) {
            return reinterpret_cast<PlayerCharacter*>(pPlayer);
        }
        return nullptr;
    }

    ConsoleManager* CreationBridge::GetConsoleManager() {
        if (!m_pConsoleManagerPtr) return nullptr;
        uintptr_t pConsole = 0;
        if (Memory::SafeRead(m_pConsoleManagerPtr, pConsole)) {
            return reinterpret_cast<ConsoleManager*>(pConsole);
        }
        return nullptr;
    }

    bool CreationBridge::ExecuteCommand(std::string_view command, TESObjectREFR* target) {
        if (command.empty()) return false;

        ConsoleManager* pConsole = GetConsoleManager();

        if (m_fnExecuteCommand && pConsole) {
            __try {
                std::string cmdCopy{ command };
                m_fnExecuteCommand(pConsole, cmdCopy.c_str(), target);
                return true;
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                // Command failed or engine state was unstable; queue for safe retry
            }
        }

        // Queue for safe execution
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            m_commandQueue.emplace(std::string(command), target);
        }
        return true;
    }

    void CreationBridge::ProcessQueue() {
        if (!m_fnExecuteCommand) return;
        ConsoleManager* pConsole = GetConsoleManager();
        if (!pConsole) return;

        std::pair<std::string, TESObjectREFR*> item;
        while (true) {
            {
                std::lock_guard<std::mutex> lock(m_queueMutex);
                if (m_commandQueue.empty()) break;
                item = std::move(m_commandQueue.front());
                m_commandQueue.pop();
            }

            __try {
                m_fnExecuteCommand(pConsole, item.first.c_str(), item.second);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                // Discard invalid command during exceptional state
                break;
            }
        }
    }

    void CreationBridge::ToggleGodMode() {
        ExecuteCommand("tgm");
    }

    void CreationBridge::ToggleImmortal() {
        ExecuteCommand("tim");
    }

    void CreationBridge::ToggleNoClip() {
        ExecuteCommand("tcl");
    }

    void CreationBridge::SetActorValue(std::string_view avName, float value) {
        ExecuteCommand(std::format("player.setav {} {}", avName, value));
    }

    void CreationBridge::ModActorValue(std::string_view avName, float delta) {
        ExecuteCommand(std::format("player.modav {} {}", avName, delta));
    }

    void CreationBridge::AddItem(std::string_view formId, uint32_t count) {
        ExecuteCommand(std::format("player.additem {} {}", formId, count));
    }

    void CreationBridge::PlaceAtMe(std::string_view formId, uint32_t count) {
        ExecuteCommand(std::format("player.placeatme {} {}", formId, count));
    }

    void CreationBridge::TeleportCoc(std::string_view cellName) {
        ExecuteCommand(std::format("coc {}", cellName));
    }

    void CreationBridge::UnlockMapMarkers() {
        ExecuteCommand("tmm 1");
    }

    void CreationBridge::SetTimescale(float scale) {
        ExecuteCommand(std::format("set timescale to {}", scale));
    }

} // namespace Overboss::Engine
