#include "features.h"
#include "../engine/creation_bridge.h"
#include <imgui.h>
#include <vector>
#include <string>

namespace Overboss::Features::PowerArmor {

    struct ArmorSet {
        const char* name;
        const char* helm;
        const char* torso;
        const char* leftArm;
        const char* rightArm;
        const char* leftLeg;
        const char* rightLeg;
    };

    static const std::vector<ArmorSet> s_armorSets = {
        { "T-45 Standard Suit", "00154ABF", "00154AC2", "00154ABD", "00154ABE", "00154AC0", "00154AC1" },
        { "T-51 Military Suit", "00140C4E", "00140C51", "00140C4C", "00140C4D", "00140C4F", "00140C50" },
        { "T-60 Brotherhood Suit", "00140C3D", "00140C42", "00140C3F", "00140C40", "00140C43", "00140C44" },
        { "X-01 Advanced Enclave Suit", "00154AC3", "00154AC8", "00154AC4", "00154AC5", "00154AC6", "00154AC7" }
    };

    static bool s_freezeFusionCore = false;

    void ToggleFusionCoreFreeze() {
        s_freezeFusionCore = !s_freezeFusionCore;
        if (s_freezeFusionCore) {
            Engine::CreationBridge::Get().ExecuteCommand("player.setav PowerArmorBattery 100");
        }
    }

    void SpawnFrame() {
        // Place an empty Power Armor chassis directly in front of the player
        Engine::CreationBridge::Get().PlaceAtMe("0002079E", 1);
        // Deliver fusion core to operate frame immediately
        DeliverFusionCores(5);
    }

    void DeliverSuit(std::string_view suitType) {
        for (const auto& s : s_armorSets) {
            if (suitType == s.name) {
                Engine::CreationBridge::Get().AddItem(s.helm, 1);
                Engine::CreationBridge::Get().AddItem(s.torso, 1);
                Engine::CreationBridge::Get().AddItem(s.leftArm, 1);
                Engine::CreationBridge::Get().AddItem(s.rightArm, 1);
                Engine::CreationBridge::Get().AddItem(s.leftLeg, 1);
                Engine::CreationBridge::Get().AddItem(s.rightLeg, 1);
                break;
            }
        }
    }

    void DeliverFusionCores(uint32_t count) {
        Engine::CreationBridge::Get().AddItem("00075FE4", count);
    }

    void RenderTab() {
        ImGui::TextColored(ImVec4(0.20f, 0.95f, 0.35f, 1.0f), "POWER ARMOR FABRICATION & BATTERY CONTROL");
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Checkbox("Freeze Fusion Core Drain to 100%", &s_freezeFusionCore)) {
            ToggleFusionCoreFreeze();
        }

        ImGui::Spacing();
        if (ImGui::Button("Spawn Empty Power Armor Chassis / Frame", ImVec2(320, 0))) {
            SpawnFrame();
        }
        ImGui::SameLine();
        if (ImGui::Button("Add 25 Fusion Cores", ImVec2(180, 0))) {
            DeliverFusionCores(25);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.20f, 0.95f, 0.35f, 1.0f), "COMPLETE POWER ARMOR SUITS");

        for (const auto& suit : s_armorSets) {
            ImGui::PushID(suit.name);
            ImGui::Text("%s", suit.name);
            ImGui::SameLine(260.0f);
            if (ImGui::Button("Inject Full Suit", ImVec2(140, 0))) {
                DeliverSuit(suit.name);
            }
            ImGui::PopID();
            ImGui::Spacing();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.20f, 0.95f, 0.35f, 1.0f), "JETPACK & POWER ARMOR MODS");
        if (ImGui::Button("Attach Jetpack Mod to Current Torso", ImVec2(280, 0))) {
            // Jetpack Mod FormID for T-60 / X-01
            Engine::CreationBridge::Get().ExecuteCommand("player.attachmod 00182CE2");
        }
        ImGui::SameLine();
        if (ImGui::Button("Tesla Bracers", ImVec2(140, 0))) {
            Engine::CreationBridge::Get().ExecuteCommand("player.attachmod 00182CDA");
        }
    }

} // namespace Overboss::Features::PowerArmor
