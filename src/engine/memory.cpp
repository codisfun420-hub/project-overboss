#include "memory.h"
#include <sstream>
#include <psapi.h>

namespace Overboss::Memory {

    namespace {
        // Parses a space-delimited hex string with '?' wildcards
        // Example: "48 8B 05 ? ? ? ? 48 8B D9"
        bool ParsePattern(std::string_view patternStr, std::vector<uint8_t>& outBytes, std::vector<bool>& outMask) {
            outBytes.clear();
            outMask.clear();

            std::istringstream stream{ std::string(patternStr) };
            std::string token;

            while (stream >> token) {
                if (token == "?" || token == "??") {
                    outBytes.push_back(0x00);
                    outMask.push_back(false); // Wildcard
                } else {
                    try {
                        size_t idx = 0;
                        auto byteVal = static_cast<uint8_t>(std::stoul(token, &idx, 16));
                        outBytes.push_back(byteVal);
                        outMask.push_back(true); // Exact match
                    } catch (...) {
                        return false;
                    }
                }
            }
            return !outBytes.empty();
        }

        // Safe block read
        bool SafeReadBytes(uintptr_t address, void* buffer, size_t size) {
            if (!address || !buffer || size == 0) return false;
            __try {
                std::memcpy(buffer, reinterpret_cast<const void*>(address), size);
                return true;
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                return false;
            }
        }
    }

    PatternMatch FindPatternInRange(std::string_view pattern, uintptr_t startAddress, size_t length) {
        std::vector<uint8_t> patternBytes;
        std::vector<bool> patternMask;
        if (!ParsePattern(pattern, patternBytes, patternMask) || length < patternBytes.size()) {
            return { 0, false };
        }

        const size_t patternLen = patternBytes.size();
        const auto* memoryBytes = reinterpret_cast<const uint8_t*>(startAddress);
        const size_t scanLimit = length - patternLen;

        for (size_t i = 0; i <= scanLimit; ++i) {
            bool matched = true;
            for (size_t j = 0; j < patternLen; ++j) {
                if (patternMask[j] && memoryBytes[i + j] != patternBytes[j]) {
                    matched = false;
                    break;
                }
            }

            if (matched) {
                return { startAddress + i, true };
            }
        }

        return { 0, false };
    }

    PatternMatch FindPattern(std::string_view pattern, const wchar_t* moduleName) {
        HMODULE hMod = moduleName ? GetModuleHandleW(moduleName) : GetModuleHandleW(nullptr);
        if (!hMod) {
            return { 0, false };
        }

        MODULEINFO modInfo{};
        if (!GetModuleInformation(GetCurrentProcess(), hMod, &modInfo, sizeof(modInfo))) {
            return { 0, false };
        }

        const auto base = reinterpret_cast<uintptr_t>(modInfo.lpBaseOfDll);
        const auto size = static_cast<size_t>(modInfo.SizeOfImage);

        // Scan executable module memory range
        return FindPatternInRange(pattern, base, size);
    }

    uintptr_t ResolveRipRelative(uintptr_t instructionAddress, int32_t offsetToDisplacement, int32_t instructionLength) {
        if (!instructionAddress) return 0;

        int32_t displacement = 0;
        if (!SafeRead(instructionAddress + offsetToDisplacement, displacement)) {
            return 0;
        }

        return instructionAddress + instructionLength + displacement;
    }

    bool PatchBytes(uintptr_t address, const std::vector<uint8_t>& bytes, std::vector<uint8_t>* outOriginal) {
        if (!address || bytes.empty()) return false;

        DWORD oldProtect = 0;
        if (!VirtualProtect(reinterpret_cast<LPVOID>(address), bytes.size(), PAGE_EXECUTE_READWRITE, &oldProtect)) {
            return false;
        }

        if (outOriginal) {
            outOriginal->resize(bytes.size());
            std::memcpy(outOriginal->data(), reinterpret_cast<const void*>(address), bytes.size());
        }

        std::memcpy(reinterpret_cast<void*>(address), bytes.data(), bytes.size());

        VirtualProtect(reinterpret_cast<LPVOID>(address), bytes.size(), oldProtect, &oldProtect);
        FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<LPCVOID>(address), bytes.size());
        return true;
    }

    MemoryPatch::MemoryPatch(uintptr_t address, std::vector<uint8_t> patchBytes)
        : m_address(address), m_patchBytes(std::move(patchBytes)) {
        if (m_address && !m_patchBytes.empty()) {
            m_originalBytes.resize(m_patchBytes.size());
            SafeReadBytes(m_address, m_originalBytes.data(), m_originalBytes.size());
        }
    }

    MemoryPatch::~MemoryPatch() {
        if (m_isApplied) {
            Restore();
        }
    }

    MemoryPatch::MemoryPatch(MemoryPatch&& other) noexcept
        : m_address(other.m_address),
          m_originalBytes(std::move(other.m_originalBytes)),
          m_patchBytes(std::move(other.m_patchBytes)),
          m_isApplied(other.m_isApplied) {
        other.m_address = 0;
        other.m_isApplied = false;
    }

    MemoryPatch& MemoryPatch::operator=(MemoryPatch&& other) noexcept {
        if (this != &other) {
            if (m_isApplied) {
                Restore();
            }
            m_address = other.m_address;
            m_originalBytes = std::move(other.m_originalBytes);
            m_patchBytes = std::move(other.m_patchBytes);
            m_isApplied = other.m_isApplied;

            other.m_address = 0;
            other.m_isApplied = false;
        }
        return *this;
    }

    bool MemoryPatch::Apply() {
        if (!m_address || m_patchBytes.empty() || m_isApplied) return false;

        std::vector<uint8_t> backup;
        if (PatchBytes(m_address, m_patchBytes, &backup)) {
            if (m_originalBytes.empty()) {
                m_originalBytes = std::move(backup);
            }
            m_isApplied = true;
            return true;
        }
        return false;
    }

    bool MemoryPatch::Restore() {
        if (!m_address || m_originalBytes.empty() || !m_isApplied) return false;

        if (PatchBytes(m_address, m_originalBytes, nullptr)) {
            m_isApplied = false;
            return true;
        }
        return false;
    }

} // namespace Overboss::Memory
