#include "features.h"
#include "../engine/creation_bridge.h"
#include <imgui.h>
#include <vector>

namespace Overboss::Features::Settlement {

    struct MaterialEntry {
        const char* name;
        const char* formId;
    };

    static const std::vector<MaterialEntry> s_materials = {
        { "Adhesive", "001BF72E" },
        { "Aluminum", "0006907A" },
        { "Ballistic Fiber", "000AEC41" },
        { "Ceramic", "000AEC40" },
        { "Circuitry", "0006907B" },
        { "Cloth", "000AEC5C" },
        { "Concrete", "00106D52" },
        { "Copper", "0006907D" },
        { "Crystal", "0006907E" },
        { "Fiberglass", "0006907F" },
        { "Fiber Optics", "00069079" },
        { "Gears", "0006907E" },
        { "Glass", "00069081" },
        { "Lead", "000AEC62" },
        { "Nuclear Material", "00069086" },
        { "Oil", "001BF732" },
        { "Plastic", "0006907F" },
        { "Rubber", "00106D98" },
        { "Screws", "00069082" },
        { "Springs", "00069083" },
        { "Steel", "000731A3" },
        { "Wood", "000731A4" }
    };

    void AddCaps(uint32_t count) {
        Engine::CreationBridge::Get().AddItem("0000000F", count);
    }

    void AddBobbyPins(uint32_t count) {
        Engine::CreationBridge::Get().AddItem("0000000A", count);
    }

    void DeliverAllCraftingShipments(uint32_t count) {
        for (const auto& mat : s_materials) {
            Engine::CreationBridge::Get().AddItem(mat.formId, count);
        }
    }

    void BypassBuildBudget() {
        // Multiplies the workshop build budget cap significantly
        Engine::CreationBridge::Get().ExecuteCommand("setav 343 1000000");
        Engine::CreationBridge::Get().ExecuteCommand("setav 344 1000000");
        Engine::CreationBridge::Get().ExecuteCommand("setav 348 1000000");
    }

    void RenderTab() {
        ImGui::TextColored(ImVec4(0.20f, 0.95f, 0.35f, 1.0f), "ECONOMY & CURRENCY SUPPLY");
        ImGui::Separator();
        ImGui::Spacing();

        static int s_capsAmount = 50000;
        ImGui::SliderInt("Caps Increment", &s_capsAmount, 5000, 500000);
        if (ImGui::Button("Inject Caps (0000000F)", ImVec2(220, 0))) {
            AddCaps(static_cast<uint32_t>(s_capsAmount));
        }

        ImGui::SameLine();
        if (ImGui::Button("Add 500 Bobby Pins (0000000A)", ImVec2(240, 0))) {
            AddBobbyPins(500);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.20f, 0.95f, 0.35f, 1.0f), "SETTLEMENT BUILDING & CRAFTING MATERIALS");
        ImGui::TextDisabled("Adds materials directly to player inventory or active workshop workbench.");

        static int s_batchCount = 5000;
        ImGui::SliderInt("Shipment Batch Size", &s_batchCount, 500, 20000);

        if (ImGui::Button("DELIVER COMPLETE BATCH: 5000x OF EVERY MATERIAL", ImVec2(450, 32))) {
            DeliverAllCraftingShipments(static_cast<uint32_t>(s_batchCount));
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.20f, 0.95f, 0.35f, 1.0f), "WORKSHOP BUILD BUDGET BYPASS");
        ImGui::TextWrapped("Target the settlement workshop workbench in console/crosshair, then click bypass to unlock unlimited building limit.");

        if (ImGui::Button("Bypass Settlement Size Cap (Expand Budget Max)", ImVec2(380, 0))) {
            BypassBuildBudget();
        }
    }

} // namespace Overboss::Features::Settlement
