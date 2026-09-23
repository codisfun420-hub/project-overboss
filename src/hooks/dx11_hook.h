#pragma once

#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <atomic>

namespace Overboss::Hooks {

    class DX11Hook {
    public:
        static DX11Hook& Get() noexcept {
            static DX11Hook instance;
            return instance;
        }

        // Sets up dummy device and initializes MinHook detours on Present & ResizeBuffers
        bool Initialize();

        // Shuts down hooks, restores WndProc, and cleans up resources
        void Shutdown();

        // Menu visibility state
        [[nodiscard]] bool IsMenuOpen() const noexcept { return m_menuOpen.load(); }
        void ToggleMenu() noexcept { m_menuOpen.store(!m_menuOpen.load()); }
        void SetMenuOpen(bool open) noexcept { m_menuOpen.store(open); }

        // Panic ejection requested
        [[nodiscard]] bool IsPanicRequested() const noexcept { return m_panicRequested.load(); }
        void RequestPanic() noexcept { m_panicRequested.store(true); }

        [[nodiscard]] HWND GetWindow() const noexcept { return m_hWnd; }

    private:
        DX11Hook() = default;
        ~DX11Hook() { Shutdown(); }

        std::atomic<bool> m_initialized = false;
        std::atomic<bool> m_menuOpen = false;
        std::atomic<bool> m_panicRequested = false;

        HWND m_hWnd = nullptr;
        WNDPROC m_originalWndProc = nullptr;

        ID3D11Device* m_pDevice = nullptr;
        ID3D11DeviceContext* m_pContext = nullptr;
        ID3D11RenderTargetView* m_pRenderTargetView = nullptr;

        void* m_presentTarget = nullptr;
        void* m_resizeBuffersTarget = nullptr;

        // Friend functions for detours
        friend HRESULT WINAPI HookedPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
        friend HRESULT WINAPI HookedResizeBuffers(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
        friend LRESULT CALLBACK HookedWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    };

} // namespace Overboss::Hooks
