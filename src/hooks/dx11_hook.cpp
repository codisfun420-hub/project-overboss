#include "dx11_hook.h"
#include "../gui/gui.h"
#include "../engine/creation_bridge.h"

#include <MinHook.h>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

// Forward declaration of Win32 ImGui message handler
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Overboss::Hooks {

    using FnPresent = HRESULT(WINAPI*)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
    using FnResizeBuffers = HRESULT(WINAPI*)(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);

    static FnPresent oPresent = nullptr;
    static FnResizeBuffers oResizeBuffers = nullptr;

    LRESULT CALLBACK HookedWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        auto& hook = DX11Hook::Get();

        // Check for toggle hotkey (VK_INSERT)
        if (uMsg == WM_KEYDOWN && wParam == VK_INSERT) {
            hook.ToggleMenu();
            ShowCursor(hook.IsMenuOpen() ? TRUE : FALSE);
            return 0;
        }

        // Check for panic hotkey (VK_END)
        if (uMsg == WM_KEYDOWN && wParam == VK_END) {
            hook.RequestPanic();
            return 0;
        }

        if (hook.IsMenuOpen()) {
            // Forward input to Dear ImGui
            ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

            // If ImGui captures the input or menu is open, isolate engine from clicks/keystrokes
            ImGuiIO& io = ImGui::GetIO();
            if (io.WantCaptureMouse || io.WantCaptureKeyboard || hook.IsMenuOpen()) {
                switch (uMsg) {
                    case WM_LBUTTONDOWN:
                    case WM_LBUTTONUP:
                    case WM_RBUTTONDOWN:
                    case WM_RBUTTONUP:
                    case WM_MBUTTONDOWN:
                    case WM_MBUTTONUP:
                    case WM_MOUSEWHEEL:
                    case WM_MOUSEMOVE:
                    case WM_KEYDOWN:
                    case WM_KEYUP:
                    case WM_CHAR:
                        return 1; // Suppress from game engine
                    default:
                        break;
                }
            }
        }

        if (hook.m_originalWndProc) {
            return CallWindowProcW(hook.m_originalWndProc, hWnd, uMsg, wParam, lParam);
        }

        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }

    HRESULT WINAPI HookedPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
        auto& hook = DX11Hook::Get();

        // Check if unhooking was requested
        if (hook.IsPanicRequested()) {
            return oPresent(pSwapChain, SyncInterval, Flags);
        }

        // First-time DirectX initialization
        if (!hook.m_pDevice) {
            if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&hook.m_pDevice)))) {
                hook.m_pDevice->GetImmediateContext(&hook.m_pContext);

                DXGI_SWAP_CHAIN_DESC desc{};
                pSwapChain->GetDesc(&desc);
                hook.m_hWnd = desc.OutputWindow;

                // Subclass window procedure
                hook.m_originalWndProc = reinterpret_cast<WNDPROC>(
                    SetWindowLongPtrW(hook.m_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(HookedWndProc))
                );

                // Setup ImGui context
                IMGUI_CHECKVERSION();
                ImGui::CreateContext();
                ImGuiIO& io = ImGui::GetIO();
                io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

                // Apply Pip-Boy Phosphor Theme
                Overboss::GUI::ApplyPipBoyTheme();

                // Initialize ImGui backends
                ImGui_ImplWin32_Init(hook.m_hWnd);
                ImGui_ImplDX11_Init(hook.m_pDevice, hook.m_pContext);
            }
        }

        // Recreate render target view if invalidated (e.g. after ResizeBuffers)
        if (!hook.m_pRenderTargetView && hook.m_pDevice) {
            ID3D11Texture2D* pBackBuffer = nullptr;
            if (SUCCEEDED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer)))) {
                hook.m_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &hook.m_pRenderTargetView);
                pBackBuffer->Release();
            }
        }

        // Process deferred console commands from Creation Engine bridge
        Engine::CreationBridge::Get().ProcessQueue();

        // Render Dear ImGui frame
        if (hook.m_pRenderTargetView) {
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            // Render Overboss GUI overlay
            Overboss::GUI::Render();

            ImGui::Render();

            hook.m_pContext->OMSetRenderTargets(1, &hook.m_pRenderTargetView, nullptr);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        }

        return oPresent(pSwapChain, SyncInterval, Flags);
    }

    HRESULT WINAPI HookedResizeBuffers(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) {
        auto& hook = DX11Hook::Get();

        // Release render target view prior to resizing backbuffers to prevent DXGI_ERROR_INVALID_CALL
        if (hook.m_pRenderTargetView) {
            hook.m_pRenderTargetView->Release();
            hook.m_pRenderTargetView = nullptr;
        }

        return oResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
    }

    bool DX11Hook::Initialize() {
        if (m_initialized) return true;

        if (MH_Initialize() != MH_OK) {
            return false;
        }

        // 1. Create a dummy window for swapchain creation
        WNDCLASSEXW wc{};
        wc.cbSize = sizeof(wc);
        wc.style = CS_CLASSDC;
        wc.lpfnWndProc = DefWindowProcW;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.lpszClassName = L"OverbossDummyWindowClass";

        RegisterClassExW(&wc);
        HWND hDummyWnd = CreateWindowExW(0, wc.lpszClassName, L"OverbossDummy", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, nullptr, nullptr, wc.hInstance, nullptr);

        if (!hDummyWnd) {
            UnregisterClassW(wc.lpszClassName, wc.hInstance);
            return false;
        }

        // 2. Setup dummy DirectX 11 device and swap chain
        DXGI_SWAP_CHAIN_DESC scd{};
        scd.BufferCount = 1;
        scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scd.OutputWindow = hDummyWnd;
        scd.SampleDesc.Count = 1;
        scd.Windowed = TRUE;

        D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
        ID3D11Device* pDummyDevice = nullptr;
        ID3D11DeviceContext* pDummyContext = nullptr;
        IDXGISwapChain* pDummySwapChain = nullptr;

        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            0,
            &featureLevel,
            1,
            D3D11_SDK_VERSION,
            &scd,
            &pDummySwapChain,
            &pDummyDevice,
            nullptr,
            &pDummyContext
        );

        if (FAILED(hr) || !pDummySwapChain) {
            DestroyWindow(hDummyWnd);
            UnregisterClassW(wc.lpszClassName, wc.hInstance);
            return false;
        }

        // 3. Extract IDXGISwapChain VMT pointers
        void** pVMT = *reinterpret_cast<void***>(pDummySwapChain);
        m_presentTarget = pVMT[8];         // Present
        m_resizeBuffersTarget = pVMT[13];  // ResizeBuffers

        // 4. Release dummy objects
        pDummySwapChain->Release();
        pDummyContext->Release();
        pDummyDevice->Release();
        DestroyWindow(hDummyWnd);
        UnregisterClassW(wc.lpszClassName, wc.hInstance);

        // 5. Hook Present and ResizeBuffers using MinHook
        if (MH_CreateHook(m_presentTarget, reinterpret_cast<void*>(&HookedPresent), reinterpret_cast<void**>(&oPresent)) != MH_OK) {
            return false;
        }

        if (MH_CreateHook(m_resizeBuffersTarget, reinterpret_cast<void*>(&HookedResizeBuffers), reinterpret_cast<void**>(&oResizeBuffers)) != MH_OK) {
            MH_RemoveHook(m_presentTarget);
            return false;
        }

        if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
            MH_RemoveHook(m_presentTarget);
            MH_RemoveHook(m_resizeBuffersTarget);
            return false;
        }

        m_initialized = true;
        return true;
    }

    void DX11Hook::Shutdown() {
        if (!m_initialized.exchange(false)) return;

        // Restore original WndProc
        if (m_hWnd && m_originalWndProc) {
            SetWindowLongPtrW(m_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(m_originalWndProc));
            m_originalWndProc = nullptr;
        }

        // Disable and remove MinHook hooks
        MH_DisableHook(MH_ALL_HOOKS);
        if (m_presentTarget) MH_RemoveHook(m_presentTarget);
        if (m_resizeBuffersTarget) MH_RemoveHook(m_resizeBuffersTarget);
        MH_Uninitialize();

        // Release ImGui
        if (ImGui::GetCurrentContext()) {
            ImGui_ImplDX11_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
        }

        // Release D3D11 resources
        if (m_pRenderTargetView) {
            m_pRenderTargetView->Release();
            m_pRenderTargetView = nullptr;
        }
        if (m_pContext) {
            m_pContext->Release();
            m_pContext = nullptr;
        }
        if (m_pDevice) {
            m_pDevice->Release();
            m_pDevice = nullptr;
        }

        ShowCursor(FALSE);
    }

} // namespace Overboss::Hooks
