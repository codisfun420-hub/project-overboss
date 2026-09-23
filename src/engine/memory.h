#pragma once

#include <windows.h>
#include <cstdint>
#include <vector>
#include <string>
#include <string_view>
#include <optional>
#include <span>
#include <cstring>

namespace Overboss::Memory {

    // Forward declaration of pattern scanning result
    struct PatternMatch {
        uintptr_t address = 0;
        bool found = false;

        explicit operator bool() const noexcept { return found && address != 0; }
        uintptr_t Get() const noexcept { return address; }
    };

    // Scans the main module (Fallout4.exe) or a specified module for an AOB pattern.
    // Pattern format: "48 8B 05 ? ? ? ? 48 8B D9" or with wildcard '?' / "??"
    PatternMatch FindPattern(std::string_view pattern, const wchar_t* moduleName = nullptr);

    // Scans a specific memory range
    PatternMatch FindPatternInRange(std::string_view pattern, uintptr_t startAddress, size_t length);

    // Resolves an x64 RIP-relative displacement:
    // target = instructionAddress + instructionLength + *(int32_t*)(instructionAddress + offsetToDisplacement)
    uintptr_t ResolveRipRelative(uintptr_t instructionAddress, int32_t offsetToDisplacement = 3, int32_t instructionLength = 7);

    // RAII Memory Patch wrapper for safely modifying executable bytes and restoring them
    class MemoryPatch {
    public:
        MemoryPatch() = default;
        MemoryPatch(uintptr_t address, std::vector<uint8_t> patchBytes);
        ~MemoryPatch();

        // Disallow copying, allow moving
        MemoryPatch(const MemoryPatch&) = delete;
        MemoryPatch& operator=(const MemoryPatch&) = delete;
        MemoryPatch(MemoryPatch&& other) noexcept;
        MemoryPatch& operator=(MemoryPatch&& other) noexcept;

        bool Apply();
        bool Restore();
        [[nodiscard]] bool IsApplied() const noexcept { return m_isApplied; }
        [[nodiscard]] uintptr_t GetAddress() const noexcept { return m_address; }

    private:
        uintptr_t m_address = 0;
        std::vector<uint8_t> m_originalBytes;
        std::vector<uint8_t> m_patchBytes;
        bool m_isApplied = false;
    };

    // Direct byte write with VirtualProtect and FlushInstructionCache
    bool PatchBytes(uintptr_t address, const std::vector<uint8_t>& bytes, std::vector<uint8_t>* outOriginal = nullptr);

    // Safe memory reading protected by Structured Exception Handling (__try / __except)
    template <typename T>
    bool SafeRead(uintptr_t address, T& outValue) {
        if (!address) return false;
        __try {
            std::memcpy(&outValue, reinterpret_cast<const void*>(address), sizeof(T));
            return true;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }
    }

    // Safe pointer dereference with null checks
    template <typename T>
    T* SafeDeref(uintptr_t address) {
        if (!address) return nullptr;
        __try {
            return reinterpret_cast<T*>(address);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return nullptr;
        }
    }

} // namespace Overboss::Memory
