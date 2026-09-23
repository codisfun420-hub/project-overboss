#include "features.h"
#include "../engine/creation_bridge.h"
#include <imgui.h>
#include <vector>

namespace Overboss::Features::World {

    struct TeleportLocation {
        const char* name;
        const char* cellId;
    };

    static const std::vector<TeleportLocation> s_destinations = {
        { "Sanctuary Hills", "SanctuaryExt01" },
        { "Diamond City Market", "DiamondCityExt" },
        { "Goodneighbor", "GoodneighborExt" },
        { "The Prydwen (Main Deck)", "PrydwenHull01" },
        { "Railroad Headquarters", "RailroadHQ" },
        { "The Institute Concourse", "InstituteConcourse" },
        { "Glowing Sea (Crater of Atom)", "CraterOfAtomExt" },
        { "Fort Hagen Exterior", "FortHagenExt01" },
        { "Bunker Hill Settlement", "BunkerHillExt" },
        { "Red Rocket Truck Stop", "RedRocketExt" },
        { "Vault 111 Exterior", "Vault111Ext" }
    };

    struct WeatherEntry {
        const char* name;
        const char* weatherId;
    };

    static const std::vector<WeatherEntry> s_weathers = {
        { "Clear Commonwealth Sky", "0002B52A" },
        { "Radioactive Radstorm", "001C3D5E" },
        { "Heavy Rain / Thunderstorm", "0000015E" },
        { "Dense Wasteland Fog", "001C3D5A" },
        { "Overcast Mist", "0004A770" }
    };

    static float s_timescale = 20.0f; // Default Fallout 4 timescale is 20

    void FastTravel(std::string_view destination, std::string_view cellOrLoc) {
        Engine::CreationBridge::Get().TeleportCoc(cellOrLoc);
    }

    void RevealMapMarkers() {
        Engine::CreationBridge::Get().UnlockMapMarkers();
    }

    void SetGameTimescale(float scale) {
        s_timescale = scale;
        Engine::CreationBridge::Get().SetTimescale(scale);
    }

    void SetGameWeather(std::string_view weatherId) {
        Engine::CreationBridge::Get().ExecuteCommand(std::string("sw ") + std::string(weatherId));
    }

    void RenderTab() {
        ImGui::TextColored(ImVec4(0.20f, 0.95f, 0.35f, 1.0f), "TELEPORTATION & MAJOR HUBS");
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("DISCOVER ALL MAP MARKERS (TMM 1)", ImVec2(340, 28))) {
            RevealMapMarkers();
        }
        ImGui::SameLine();
        ImGui::TextDisabled("Unlocks all Commonwealth fast travel points");

        ImGui::Spacing();
        if (ImGui::BeginTable("TeleportTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Destination", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Cell ID", ImGuiTableColumnFlags_WidthFixed, 180.0f);
            ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 120.0f);
            ImGui::TableHeadersRow();

            for (const auto& dest : s_destinations) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(dest.name);

                ImGui::TableSetColumnIndex(1);
                ImGui::TextDisabled("%s", dest.cellId);

                ImGui::TableSetColumnIndex(2);
                std::string btn = std::string("Teleport##") + dest.cellId;
                if (ImGui::Button(btn.c_str(), ImVec2(100, 0))) {
                    FastTravel(dest.name, dest.cellId);
                }
            }
            ImGui::EndTable();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.20f, 0.95f, 0.35f, 1.0f), "ENVIRONMENT & TIMESCALE CONTROL");

        if (ImGui::SliderFloat("Timescale (Default: 20)", &s_timescale, 0.1f, 120.0f, "%.1fx")) {
            SetGameTimescale(s_timescale);
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset Timescale", ImVec2(130, 0))) {
            SetGameTimescale(20.0f);
        }

        ImGui::Spacing();
        ImGui::Text("Weather Systems:");
        for (const auto& w : s_weathers) {
            ImGui::SameLine();
            std::string btn = std::string(w.name) + "##" + w.weatherId;
            if (ImGui::Button(btn.c_str())) {
                SetGameWeather(w.weatherId);
            }
        }
    }

} // namespace Overboss::Features::World
