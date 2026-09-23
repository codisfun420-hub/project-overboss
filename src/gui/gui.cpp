#include "gui.h"
#include "../hooks/dx11_hook.h"
#include "../engine/creation_bridge.h"
#include "../features/features.h"

#include <imgui.h>

namespace Overboss::GUI {

    void ApplyPipBoyTheme() {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        style.WindowRounding    = 4.0f;
        style.FrameRounding     = 2.0f;
        style.PopupRounding     = 2.0f;
        style.ScrollbarRounding = 2.0f;
        style.GrabRounding      = 2.0f;
        style.WindowBorderSize  = 1.0f;
        style.FrameBorderSize   = 1.0f;

        // Palette: Pitch Carbon Matte + Nuclear Phosphor Accent
        colors[ImGuiCol_WindowBg]             = ImVec4(0.05f, 0.05f, 0.07f, 0.95f);
        colors[ImGuiCol_Border]               = ImVec4(0.12f, 0.40f, 0.20f, 0.80f);
        colors[ImGuiCol_Text]                 = ImVec4(0.92f, 0.95f, 0.92f, 1.00f);
        colors[ImGuiCol_Header]               = ImVec4(0.08f, 0.25f, 0.12f, 0.80f);
        colors[ImGuiCol_HeaderHovered]        = ImVec4(0.12f, 0.40f, 0.18f, 0.80f);
        colors[ImGuiCol_Button]               = ImVec4(0.08f, 0.20f, 0.10f, 0.85f);
        colors[ImGuiCol_ButtonHovered]        = ImVec4(0.14f, 0.38f, 0.18f, 1.00f);
        colors[ImGuiCol_ButtonActive]         = ImVec4(0.18f, 0.55f, 0.24f, 1.00f);
        colors[ImGuiCol_CheckMark]            = ImVec4(0.20f, 0.95f, 0.35f, 1.00f);
        colors[ImGuiCol_SliderGrab]           = ImVec4(0.18f, 0.65f, 0.28f, 1.00f);
        colors[ImGuiCol_SliderGrabActive]     = ImVec4(0.22f, 0.95f, 0.38f, 1.00f);

        colors[ImGuiCol_Tab]                  = ImVec4(0.06f, 0.16f, 0.09f, 0.85f);
        colors[ImGuiCol_TabHovered]           = ImVec4(0.12f, 0.38f, 0.18f, 1.00f);
        colors[ImGuiCol_TabActive]            = ImVec4(0.14f, 0.45f, 0.22f, 1.00f);
        colors[ImGuiCol_TitleBg]              = ImVec4(0.04f, 0.10f, 0.06f, 1.00f);
        colors[ImGuiCol_TitleBgActive]        = ImVec4(0.08f, 0.25f, 0.12f, 1.00f);
    }

    void RenderWatermark() {
        ImGuiIO& io = ImGui::GetIO();
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration |
                                       ImGuiWindowFlags_AlwaysAutoResize |
                                       ImGuiWindowFlags_NoSavedSettings |
                                       ImGuiWindowFlags_NoFocusOnAppearing |
                                       ImGuiWindowFlags_NoNav;

        ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.65f);

        if (ImGui::Begin("##Watermark", nullptr, windowFlags)) {
            ImGui::TextColored(ImVec4(0.20f, 0.95f, 0.35f, 1.0f), "PROJECT OVERBOSS v1.0.0");
            ImGui::SameLine();
            ImGui::TextDisabled("| FO4 Creation Engine [INSERT: MENU | END: PANIC]");
        }
        ImGui::End();
    }

    void Render() {
        // Render persistent watermark
        RenderWatermark();

        // Render persistent Enemy Radar overlay HUD
        Features::Radar::RenderOverlay();

        auto& hook = Hooks::DX11Hook::Get();
        if (!hook.IsMenuOpen()) {
            return;
        }

        ImGui::SetNextWindowSize(ImVec2(820, 620), ImGuiCond_FirstUseEver);

        if (ImGui::Begin("PROJECT OVERBOSS // PIP-BOY INTERNAL MOD MENU", nullptr, ImGuiWindowFlags_MenuBar)) {

            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("Close Menu", "INSERT")) {
                        hook.SetMenuOpen(false);
                        ShowCursor(FALSE);
                    }
                    if (ImGui::MenuItem("Panic Eject & Unhook", "END")) {
                        hook.RequestPanic();
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }

            if (ImGui::BeginTabBar("OverbossTabBar", ImGuiTabBarFlags_None)) {

                if (ImGui::BeginTabItem("Vitals & Physics")) {
                    Features::Vitals::RenderTab();
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Armory & Weapons")) {
                    Features::Armory::RenderTab();
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Power Armor")) {
                    Features::PowerArmor::RenderTab();
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Settlement & Economy")) {
                    Features::Settlement::RenderTab();
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("World & Teleport")) {
                    Features::World::RenderTab();
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Enemy Radar")) {
                    Features::Radar::RenderTab();
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("System & Eject")) {
                    ImGui::TextColored(ImVec4(0.20f, 0.95f, 0.35f, 1.0f), "CREATION ENGINE INTERNALS & DIAGNOSTICS");
                    ImGui::Separator();
                    ImGui::Spacing();

                    bool bridgeReady = Engine::CreationBridge::Get().IsReady();
                    ImGui::Text("Engine Bridge Status: ");
                    ImGui::SameLine();
                    if (bridgeReady) {
                        ImGui::TextColored(ImVec4(0.2f, 0.95f, 0.2f, 1.0f), "OPERATIONAL (Offsets Resolved)");
                    } else {
                        ImGui::TextColored(ImVec4(0.95f, 0.3f, 0.2f, 1.0f), "STANDBY (Scanning / In Loading Screen)");
                    }

                    auto* pPlayer = Engine::CreationBridge::Get().GetPlayer();
                    ImGui::Text("PlayerCharacter Pointer: 0x%p", pPlayer);

                    auto* pConsole = Engine::CreationBridge::Get().GetConsoleManager();
                    ImGui::Text("ConsoleManager Pointer:  0x%p", pConsole);

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::TextColored(ImVec4(0.95f, 0.3f, 0.3f, 1.0f), "EMERGENCY DETACHMENT / PANIC EJECT");
                    ImGui::TextWrapped("Restores original SwapChain VMT, restores WndProc, releases all DirectX handles, restores patched bytes, and safely unloads the DLL from Fallout4.exe memory.");

                    ImGui::Spacing();
                    if (ImGui::Button("EJECT & UNHOOK DLL (VK_END)", ImVec2(320, 36))) {
                        hook.RequestPanic();
                    }

                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
        }
        ImGui::End();
    }

} // namespace Overboss::GUI
