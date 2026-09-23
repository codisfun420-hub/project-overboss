#include "features.h"
#include "../engine/creation_bridge.h"
#include "../engine/memory.h"
#include <imgui.h>
#include <vector>
#include <string>

namespace Overboss::Features::Armory {

    struct WeaponEntry {
        const char* name;
        const char* formId;
        const char* defaultAmmoId;
        uint32_t defaultAmmoQty;
    };

    static const std::vector<WeaponEntry> s_weapons = {
        { "Deliverer (Silenced 10mm)", "000DC8E7", "0001F276", 500 },
        { "Fat Man", "000BD56F", "000E6B2E", 50 },
        { "Minigun", "0001F669", "0001F66C", 5000 },
        { "Gauss Rifle", "000D1EB0", "0018ABDF", 500 },
        { "Gatling Laser", "000E27BC", "00075FE4", 25 },
        { "Alien Blaster Pistol", "000FF995", "001025AA", 1000 },
        { "Plasma Rifle", "00100AE9", "0001DBB7", 1000 },
        { "Combat Rifle", "000DF42E", "0001F66A", 1000 },
        { "Missile Launcher", "0003F6F8", "000CABA3", 100 },
        { "Super Sledge", "000FF964", nullptr, 0 }
    };

    struct AmmoEntry {
        const char* name;
        const char* formId;
        uint32_t defaultQty;
    };

    static const std::vector<AmmoEntry> s_ammunition = {
        { "10mm Rounds", "0001F276", 1000 },
        { ".45 Rounds", "0001F66A", 1000 },
        { ".50 Caliber", "0001F279", 500 },
        { "5.56mm Rounds", "0001F66B", 1000 },
        { "5mm Rounds", "0001F66C", 5000 },
        { "Shotgun Shells", "0001F673", 500 },
        { "Fusion Cells", "000C1897", 1000 },
        { "Plasma Cartridges", "0001DBB7", 1000 },
        { "2mm EC (Gauss)", "0018ABDF", 500 },
        { "Mini Nukes", "000E6B2E", 50 },
        { "Missiles", "000CABA3", 100 },
        { "Alien Blaster Rounds", "001025AA", 1000 }
    };

    struct LegendaryEntry {
        const char* name;
        const char* formId;
        const char* description;
    };

    static const std::vector<LegendaryEntry> s_legendaries = {
        { "Explosive", "001E7FF6", "Bullets explode on impact for 15 pts AoE" },
        { "Two Shot", "001CC2A6", "Fires an additional projectile" },
        { "Instigating", "001F04B5", "Double damage if target is at full HP" },
        { "Furious", "001F61E4", "Increased damage after each consecutive hit" },
        { "Wounding", "001E7174", "Targets bleed for 25 points additional damage" },
        { "Never Ending", "001ED37E", "Unlimited magazine capacity (No reload)" },
        { "Deadeye", "001F4426", "Slows down time when aiming" },
        { "Incendiary", "001E7177", "Sets target on fire for 15 points" }
    };

    static bool s_infiniteAmmoEnabled = false;

    void ToggleInfiniteAmmo() {
        s_infiniteAmmoEnabled = !s_infiniteAmmoEnabled;
        // In Creation Engine, attaching the Never Ending mod (001ED37E) or executing infinite ammo command
        if (s_infiniteAmmoEnabled) {
            Engine::CreationBridge::Get().ExecuteCommand("player.attachmod 001ED37E");
        }
    }

    void GiveWeapon(std::string_view name, std::string_view formId) {
        Engine::CreationBridge::Get().AddItem(formId, 1);
    }

    void GiveAmmo(std::string_view name, std::string_view formId, uint32_t count) {
        Engine::CreationBridge::Get().AddItem(formId, count);
    }

    void AttachLegendaryMod(std::string_view modId) {
        // player.attachmod attaches the modification to currently equipped item
        Engine::CreationBridge::Get().ExecuteCommand(std::string("player.attachmod ") + std::string(modId));
    }

    void RenderTab() {
        ImGui::TextColored(ImVec4(0.20f, 0.95f, 0.35f, 1.0f), "ARMORY: WEAPON DELIVERY");
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::BeginTable("WeaponsTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Weapon Name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("FormID", ImGuiTableColumnFlags_WidthFixed, 90.0f);
            ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 180.0f);
            ImGui::TableHeadersRow();

            for (const auto& w : s_weapons) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(w.name);

                ImGui::TableSetColumnIndex(1);
                ImGui::TextDisabled("%s", w.formId);

                ImGui::TableSetColumnIndex(2);
                std::string btnLabel = std::string("Add##") + w.formId;
                if (ImGui::Button(btnLabel.c_str(), ImVec2(75, 0))) {
                    GiveWeapon(w.name, w.formId);
                }
                if (w.defaultAmmoId) {
                    ImGui::SameLine();
                    std::string ammoBtn = std::string("+Ammo##") + w.formId;
                    if (ImGui::Button(ammoBtn.c_str(), ImVec2(80, 0))) {
                        GiveAmmo(w.name, w.defaultAmmoId, w.defaultAmmoQty);
                    }
                }
            }
            ImGui::EndTable();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.20f, 0.95f, 0.35f, 1.0f), "BULK AMMUNITION RESUPPLY");

        static int s_ammoBatch = 1000;
        ImGui::SliderInt("Batch Ammo Quantity", &s_ammoBatch, 100, 10000);

        if (ImGui::Button("Resupply All Common Ammo (5000 each)", ImVec2(320, 0))) {
            for (const auto& a : s_ammunition) {
                GiveAmmo(a.name, a.formId, 5000);
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.20f, 0.95f, 0.35f, 1.0f), "LEGENDARY EFFECT ATTACHER (EQUIPPED WEAPON)");
        ImGui::TextDisabled("Equip weapon in hands before applying legendary effects.");

        if (ImGui::BeginTable("LegendaryTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Effect", ImGuiTableColumnFlags_WidthFixed, 130.0f);
            ImGui::TableSetupColumn("Description", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableHeadersRow();

            for (const auto& l : s_legendaries) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(l.name);

                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(l.description);

                ImGui::TableSetColumnIndex(2);
                std::string btn = std::string("Apply##") + l.formId;
                if (ImGui::Button(btn.c_str(), ImVec2(90, 0))) {
                    AttachLegendaryMod(l.formId);
                }
            }
            ImGui::EndTable();
        }
    }

} // namespace Overboss::Features::Armory
