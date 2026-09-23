#include "features.h"
#include "../engine/creation_bridge.h"
#include <imgui.h>

namespace Overboss::Features::Vitals {

    static bool s_godMode = false;
    static bool s_demiGod = false;
    static bool s_noClip = false;

    static float s_speedMult = 100.0f;
    static float s_jumpHeight = 90.0f;     // Default game jump height is 90
    static float s_carryWeight = 300.0f;

    void ToggleGodMode() {
        s_godMode = !s_godMode;
        Engine::CreationBridge::Get().ToggleGodMode();
    }

    void ToggleDemiGod() {
        s_demiGod = !s_demiGod;
        Engine::CreationBridge::Get().ToggleImmortal();
    }

    void ToggleNoClip() {
        s_noClip = !s_noClip;
        Engine::CreationBridge::Get().ToggleNoClip();
    }

    void ApplySpeedMult(float mult) {
        s_speedMult = mult;
        Engine::CreationBridge::Get().SetActorValue("speedmult", mult);
    }

    void ApplyJumpHeight(float height) {
        s_jumpHeight = height;
        Engine::CreationBridge::Get().ExecuteCommand(std::string("setgs fJumpHeightMin ") + std::to_string(height));
    }

    void ApplyCarryWeight(float weight) {
        s_carryWeight = weight;
        Engine::CreationBridge::Get().SetActorValue("carryweight", weight);
    }

    void HealPlayer() {
        Engine::CreationBridge::Get().ExecuteCommand("player.resethealth");
    }

    void RestoreActionPoints() {
        Engine::CreationBridge::Get().ExecuteCommand("player.restoreav actionpoints 10000");
    }

    void CureRadiation() {
        Engine::CreationBridge::Get().ExecuteCommand("player.modav rads -10000");
    }

    void RenderTab() {
        ImGui::TextColored(ImVec4(0.20f, 0.95f, 0.35f, 1.0f), "PLAYER VITALS & MOVEMENT");
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Checkbox("God Mode (TGM) - Invulnerability, Infinite AP & Ammo", &s_godMode)) {
            Engine::CreationBridge::Get().ToggleGodMode();
        }

        if (ImGui::Checkbox("Demi-God (TIM) - Immortal (Health caps at 1 HP)", &s_demiGod)) {
            Engine::CreationBridge::Get().ToggleImmortal();
        }

        if (ImGui::Checkbox("Toggle Collision / No-Clip (TCL)", &s_noClip)) {
            Engine::CreationBridge::Get().ToggleNoClip();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.20f, 0.95f, 0.35f, 1.0f), "INSTANT RECOVERY");

        if (ImGui::Button("Full Heal (HP)", ImVec2(140, 0))) {
            HealPlayer();
        }
        ImGui::SameLine();
        if (ImGui::Button("Restore AP", ImVec2(140, 0))) {
            RestoreActionPoints();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cure All Radiation", ImVec2(160, 0))) {
            CureRadiation();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.20f, 0.95f, 0.35f, 1.0f), "ACTOR ATTRIBUTES & PHYSICS");

        if (ImGui::SliderFloat("Speed Multiplier", &s_speedMult, 100.0f, 1000.0f, "%.0f%%")) {
            ApplySpeedMult(s_speedMult);
        }

        if (ImGui::SliderFloat("Jump Height", &s_jumpHeight, 90.0f, 1200.0f, "%.0f")) {
            ApplyJumpHeight(s_jumpHeight);
        }

        if (ImGui::SliderFloat("Carry Weight Limit", &s_carryWeight, 100.0f, 50000.0f, "%.0f lbs")) {
            ApplyCarryWeight(s_carryWeight);
        }

        ImGui::Spacing();
        if (ImGui::Button("Reset Attributes to Default", ImVec2(220, 0))) {
            ApplySpeedMult(100.0f);
            ApplyJumpHeight(90.0f);
            ApplyCarryWeight(300.0f);
        }
    }

} // namespace Overboss::Features::Vitals
