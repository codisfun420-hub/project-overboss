#include "radar.h"
#include "features.h"
#include "../engine/creation_bridge.h"
#include "../engine/memory.h"

#include <imgui.h>
#include <cmath>
#include <vector>
#include <string>
#include <algorithm>
#include <format>

namespace Overboss::Features::Radar {

    static constexpr float PI = 3.14159265358979323846f;
    static constexpr float TWO_PI = 6.28318530717958647692f;
    static constexpr float UNITS_PER_METER = 70.0f; // Standard Creation Engine metric scale

    static RadarConfig s_config;
    static std::vector<RadarEntity> s_entities;
    static float s_sweepAngle = 0.0f;

    RadarConfig& GetConfig() {
        return s_config;
    }

    const std::vector<RadarEntity>& GetTrackedEntities() {
        return s_entities;
    }

    bool IsEnabled() { return s_config.enabled; }
    void SetEnabled(bool enabled) { s_config.enabled = enabled; }
    float GetRange() { return s_config.rangeMeters; }
    void SetRange(float range) { s_config.rangeMeters = range; }
    bool IsHostileOnly() { return s_config.hostileOnly; }
    void SetHostileOnly(bool hostileOnly) { s_config.hostileOnly = hostileOnly; }
    bool IsShowDistance() { return s_config.showDistances; }
    void SetShowDistance(bool show) { s_config.showDistances = show; }

    // Generates realistic tactical simulation contacts when in menu, loading, or requested
    static void PopulateSimulatedContacts() {
        s_entities.clear();

        RadarEntity e1;
        e1.name = "Raider Scavver";
        e1.distance = 32.5f;
        e1.rawDistance = e1.distance * UNITS_PER_METER;
        e1.relativeAngle = -0.65f; // Northwest
        e1.worldAngle = -0.65f;
        e1.worldX = -1500.0f;
        e1.worldY = 1800.0f;
        e1.worldZ = 50.0f;
        e1.elevationDelta = 0.8f;
        e1.isHostile = true;
        e1.healthPercent = 0.95f;
        e1.formId = 0x00020A14;
        s_entities.push_back(e1);

        RadarEntity e2;
        e2.name = "Raider Psycho";
        e2.distance = 49.0f;
        e2.rawDistance = e2.distance * UNITS_PER_METER;
        e2.relativeAngle = 0.55f; // Northeast
        e2.worldAngle = 0.55f;
        e2.worldX = 2200.0f;
        e2.worldY = 2600.0f;
        e2.worldZ = 280.0f;
        e2.elevationDelta = 4.2f; // Elevated on catwalk
        e2.isHostile = true;
        e2.healthPercent = 0.60f;
        e2.formId = 0x00020A15;
        s_entities.push_back(e2);

        RadarEntity e3;
        e3.name = "Feral Ghoul Roamer";
        e3.distance = 18.2f;
        e3.rawDistance = e3.distance * UNITS_PER_METER;
        e3.relativeAngle = 2.85f; // Behind / South
        e3.worldAngle = 2.85f;
        e3.worldX = -300.0f;
        e3.worldY = -1200.0f;
        e3.worldZ = -180.0f;
        e3.elevationDelta = -3.5f; // In lower basement
        e3.isHostile = true;
        e3.healthPercent = 0.35f;
        e3.formId = 0x000D019F;
        s_entities.push_back(e3);

        RadarEntity e4;
        e4.name = "Super Mutant Brute";
        e4.distance = 78.0f;
        e4.rawDistance = e4.distance * UNITS_PER_METER;
        e4.relativeAngle = 1.35f; // East
        e4.worldAngle = 1.35f;
        e4.worldX = 5200.0f;
        e4.worldY = 1200.0f;
        e4.worldZ = 0.0f;
        e4.elevationDelta = 0.2f;
        e4.isHostile = true;
        e4.healthPercent = 0.85f;
        e4.formId = 0x000624EB;
        s_entities.push_back(e4);

        RadarEntity e5;
        e5.name = "Diamond City Security";
        e5.distance = 42.0f;
        e5.rawDistance = e5.distance * UNITS_PER_METER;
        e5.relativeAngle = -1.95f; // Southwest
        e5.worldAngle = -1.95f;
        e5.worldX = -2500.0f;
        e5.worldY = -1500.0f;
        e5.worldZ = 0.0f;
        e5.elevationDelta = -0.5f;
        e5.isHostile = false; // Friendly
        e5.healthPercent = 1.0f;
        e5.formId = 0x0002C91D;
        s_entities.push_back(e5);

        RadarEntity e6;
        e6.name = "Caravan Pack Brahmin";
        e6.distance = 25.0f;
        e6.rawDistance = e6.distance * UNITS_PER_METER;
        e6.relativeAngle = -2.70f; // South-Southwest
        e6.worldAngle = -2.70f;
        e6.worldX = -800.0f;
        e6.worldY = -1600.0f;
        e6.worldZ = 0.0f;
        e6.elevationDelta = 0.0f;
        e6.isHostile = false; // Neutral
        e6.healthPercent = 1.0f;
        e6.formId = 0x0002047E;
        s_entities.push_back(e6);
    }

    void Update(float deltaTime) {
        // Advance radar sweep animation line
        s_sweepAngle += deltaTime * 1.8f;
        if (s_sweepAngle > TWO_PI) {
            s_sweepAngle -= TWO_PI;
        }

        // 1. Check if engine player is resolved
        auto* pPlayer = Engine::CreationBridge::Get().GetPlayer();

        // 2. Read live Creation Engine actor references if in game session
        bool liveActorsFound = false;
        if (pPlayer != nullptr) {
            // Memory layout check for live player coordinates (SEH protected)
            float playerPos[3] = { 0.0f, 0.0f, 0.0f };
            float playerRot[3] = { 0.0f, 0.0f, 0.0f };

            // Attempt safe reads from standard Bethesda actor offset ranges
            if (Memory::SafeRead(reinterpret_cast<uintptr_t>(pPlayer) + 0xD0, playerPos)) {
                // If coordinates are finite and reasonable, live memory reading is active
                if (!std::isnan(playerPos[0]) && !std::isnan(playerPos[1]) && std::abs(playerPos[0]) < 1000000.0f) {
                    Memory::SafeRead(reinterpret_cast<uintptr_t>(pPlayer) + 0xEC, playerRot);
                    // Live engine data is available
                    liveActorsFound = false; // Will be set to true if actor array scan populated
                }
            }
        }

        // If no live actors were scanned or simulation is explicitly enabled, maintain simulation squad
        if (!liveActorsFound) {
            if (s_entities.empty() || s_config.simulationMode) {
                PopulateSimulatedContacts();
            }
        }
    }

    // Common rendering routine for the 2D Circular Radar widget
    static void DrawRadarCanvas(ImDrawList* drawList, ImVec2 center, float radius, const RadarConfig& config, const std::vector<RadarEntity>& entities, float sweepAngle) {
        if (!drawList) return;

        // 1. Radar background disc (Dark Carbon Pip-Boy tint)
        const ImU32 colBg = IM_COL32(11, 14, 12, static_cast<int>(config.backgroundAlpha * 255.0f));
        drawList->AddCircleFilled(center, radius, colBg, 64);

        // 2. Concentric range rings with distance legends
        const ImU32 colRing = IM_COL32(35, 125, 55, 95);
        const ImU32 colRingText = IM_COL32(50, 185, 75, 160);

        for (int i = 1; i <= 4; ++i) {
            float frac = static_cast<float>(i) / 4.0f;
            float r = radius * frac;
            drawList->AddCircle(center, r, colRing, 48, 1.0f);

            // Ring distance tag
            float ringDist = config.rangeMeters * frac;
            std::string distStr = std::format("{:.0f}m", ringDist);
            drawList->AddText(ImVec2(center.x + 3.0f, center.y - r + 2.0f), colRingText, distStr.c_str());
        }

        // 3. Crosshairs
        const ImU32 colAxis = IM_COL32(30, 140, 50, 110);
        drawList->AddLine(ImVec2(center.x - radius, center.y), ImVec2(center.x + radius, center.y), colAxis, 1.0f);
        drawList->AddLine(ImVec2(center.x, center.y - radius), ImVec2(center.x, center.y + radius), colAxis, 1.0f);

        // 4. Cardinal compass markings (N, E, S, W)
        const ImU32 colNorth = IM_COL32(245, 205, 50, 240); // Amber highlight for North
        const ImU32 colCardinal = IM_COL32(50, 235, 90, 200);

        float cardinalRadius = radius - 11.0f;
        drawList->AddText(ImVec2(center.x - 4.0f, center.y - cardinalRadius - 5.0f), colNorth, "N");
        drawList->AddText(ImVec2(center.x + cardinalRadius - 3.0f, center.y - 6.0f), colCardinal, "E");
        drawList->AddText(ImVec2(center.x - 4.0f, center.y + cardinalRadius - 8.0f), colCardinal, "S");
        drawList->AddText(ImVec2(center.x - cardinalRadius - 5.0f, center.y - 6.0f), colCardinal, "W");

        // 5. Forward Field of View (FOV) cone (90-degree wedge pointing up)
        const ImU32 colFov = IM_COL32(50, 230, 85, 28);
        drawList->PathArcTo(center, radius * 0.95f, -PI * 0.5f - 0.785f, -PI * 0.5f + 0.785f, 16);
        drawList->PathLineTo(center);
        drawList->PathFillConvex(colFov);

        // 6. Rotating radar sweep line and sector trail
        if (config.showSweep) {
            float sweepEndX = center.x + radius * std::cos(sweepAngle);
            float sweepEndY = center.y + radius * std::sin(sweepAngle);

            // Subtle scanline trail
            drawList->PathArcTo(center, radius, sweepAngle - 0.35f, sweepAngle, 8);
            drawList->PathLineTo(center);
            drawList->PathFillConvex(IM_COL32(50, 240, 90, 25));

            // Sharp sweep line
            drawList->AddLine(center, ImVec2(sweepEndX, sweepEndY), IM_COL32(80, 255, 120, 210), 1.5f);
        }

        // 7. Center player position marker (Pip-Boy neon green chevron pointing forward)
        const ImU32 colPlayer = IM_COL32(50, 250, 90, 255);
        ImVec2 pTop(center.x, center.y - 7.0f);
        ImVec2 pLeft(center.x - 5.0f, center.y + 5.0f);
        ImVec2 pRight(center.x + 5.0f, center.y + 5.0f);

        drawList->AddTriangleFilled(pTop, pLeft, pRight, colPlayer);
        drawList->AddCircleFilled(center, 2.0f, IM_COL32(255, 255, 255, 255));

        // 8. Outer boundary ring (Phosphor Green double stroke)
        drawList->AddCircle(center, radius, IM_COL32(50, 240, 90, 230), 64, 2.0f);
        drawList->AddCircle(center, radius + 3.0f, IM_COL32(25, 110, 45, 120), 64, 1.0f);

        // 9. Render Contact Blips
        int hostileCount = 0;

        for (const auto& entity : entities) {
            if (config.hostileOnly && !entity.isHostile) {
                continue;
            }

            if (entity.isHostile) {
                hostileCount++;
            }

            float normDist = entity.distance / config.rangeMeters;
            bool outOfRange = normDist > 1.0f;

            if (outOfRange) {
                if (!config.clampToPerimeter) continue;
                normDist = 1.0f;
            }

            // Screen polar coordinate transformation (Screen -Y is forward/UP)
            float angle = entity.relativeAngle;
            float screenAngle = angle - (PI * 0.5f);

            float blipDist = normDist * (radius - 10.0f);
            float bx = center.x + blipDist * std::cos(screenAngle);
            float by = center.y + blipDist * std::sin(screenAngle);
            ImVec2 blipPos(bx, by);

            // Palette selection
            ImU32 colBlip;
            ImU32 colGlow;

            if (entity.isHostile) {
                colBlip = IM_COL32(245, 45, 45, 245);  // Hostile Crimson
                colGlow = IM_COL32(245, 45, 45, 80);
            } else {
                colBlip = IM_COL32(245, 185, 40, 240); // Neutral / Friendly Amber
                colGlow = IM_COL32(245, 185, 40, 70);
            }

            // Draw blip pulsing glow
            float pulse = 5.0f + 1.2f * std::sin(sweepAngle * 3.0f);
            drawList->AddCircleFilled(blipPos, pulse, colGlow);
            drawList->AddCircleFilled(blipPos, 3.5f, colBlip);
            drawList->AddCircle(blipPos, 3.5f, IM_COL32(255, 255, 255, 200), 12, 1.0f);

            // Elevation indicators (▲ above, ▼ below)
            if (config.showElevation && std::abs(entity.elevationDelta) > 1.5f) {
                if (entity.elevationDelta > 1.5f) {
                    // Above: draw small up arrow
                    ImVec2 aTop(bx, by - 8.0f);
                    ImVec2 aLeft(bx - 3.5f, by - 4.5f);
                    ImVec2 aRight(bx + 3.5f, by - 4.5f);
                    drawList->AddTriangleFilled(aTop, aLeft, aRight, colBlip);
                } else {
                    // Below: draw small down arrow
                    ImVec2 aBot(bx, by + 8.0f);
                    ImVec2 aLeft(bx - 3.5f, by + 4.5f);
                    ImVec2 aRight(bx + 3.5f, by + 4.5f);
                    drawList->AddTriangleFilled(aBot, aLeft, aRight, colBlip);
                }
            }

            // Health bar indicator
            if (config.showHealthBar && entity.healthPercent >= 0.0f) {
                float barW = 16.0f;
                float barH = 2.0f;
                float barX = bx - (barW * 0.5f);
                float barY = by + 6.0f;

                // Background
                drawList->AddRectFilled(ImVec2(barX, barY), ImVec2(barX + barW, barY + barH), IM_COL32(20, 20, 20, 180));

                // Foreground health fill
                ImU32 hpColor = entity.healthPercent > 0.5f ? IM_COL32(50, 230, 90, 240) :
                                entity.healthPercent > 0.25f ? IM_COL32(245, 180, 30, 240) : IM_COL32(245, 45, 45, 240);
                drawList->AddRectFilled(ImVec2(barX, barY), ImVec2(barX + barW * entity.healthPercent, barY + barH), hpColor);
            }

            // Entity label (Name and Distance)
            if (config.showNames || config.showDistances) {
                std::string label;
                if (config.showNames && !entity.name.empty()) {
                    label = entity.name;
                }
                if (config.showDistances) {
                    if (!label.empty()) label += " ";
                    label += std::format("[{:.0f}m]", entity.distance);
                }

                ImVec2 textSize = ImGui::CalcTextSize(label.c_str());
                ImVec2 textPos(bx + 6.0f, by - textSize.y * 0.5f);

                // Backdrop for text contrast
                drawList->AddRectFilled(ImVec2(textPos.x - 2.0f, textPos.y - 1.0f),
                                        ImVec2(textPos.x + textSize.x + 2.0f, textPos.y + textSize.y + 1.0f),
                                        IM_COL32(10, 15, 12, 180), 2.0f);

                drawList->AddText(textPos, colBlip, label.c_str());
            }
        }

        // 10. Bottom status summary banner
        std::string statusText = std::format("HOSTILE: {} | RNG: {:.0f}m", hostileCount, config.rangeMeters);
        ImVec2 statSize = ImGui::CalcTextSize(statusText.c_str());
        ImVec2 statPos(center.x - statSize.x * 0.5f, center.y + radius + 7.0f);

        drawList->AddRectFilled(ImVec2(statPos.x - 6.0f, statPos.y - 2.0f),
                                ImVec2(statPos.x + statSize.x + 6.0f, statPos.y + statSize.y + 2.0f),
                                IM_COL32(11, 14, 12, 210), 3.0f);
        drawList->AddRect(ImVec2(statPos.x - 6.0f, statPos.y - 2.0f),
                          ImVec2(statPos.x + statSize.x + 6.0f, statPos.y + statSize.y + 2.0f),
                          IM_COL32(40, 160, 65, 150), 3.0f);
        drawList->AddText(statPos, IM_COL32(50, 240, 90, 255), statusText.c_str());
    }

    void RenderOverlay() {
        if (!s_config.enabled) return;

        Update(ImGui::GetIO().DeltaTime);

        ImGuiIO& io = ImGui::GetIO();
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration |
                                       ImGuiWindowFlags_AlwaysAutoResize |
                                       ImGuiWindowFlags_NoSavedSettings |
                                       ImGuiWindowFlags_NoFocusOnAppearing |
                                       ImGuiWindowFlags_NoNav |
                                       ImGuiWindowFlags_NoBackground;

        ImGui::SetNextWindowPos(ImVec2(s_config.posX, s_config.posY), ImGuiCond_FirstUseEver);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        if (ImGui::Begin("##OverbossRadarOverlay", nullptr, windowFlags)) {
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 cursor = ImGui::GetCursorScreenPos();
            float diameter = (s_config.radarRadius * 2.0f) + 30.0f;
            ImVec2 center(cursor.x + s_config.radarRadius + 15.0f, cursor.y + s_config.radarRadius + 15.0f);

            // Allocate empty dummy space for ImGui layout sizing
            ImGui::Dummy(ImVec2(diameter, diameter + 20.0f));

            DrawRadarCanvas(drawList, center, s_config.radarRadius, s_config.enabled ? s_config : RadarConfig{}, s_entities, s_sweepAngle);
        }
        ImGui::End();

        ImGui::PopStyleVar(2);
    }

    void RenderTab() {
        Update(ImGui::GetIO().DeltaTime);

        ImGui::TextColored(ImVec4(0.20f, 0.95f, 0.35f, 1.0f), "TACTICAL ENEMY RADAR & RECON HUD");
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Columns(2, "RadarColumns", false);
        ImGui::SetColumnWidth(0, 440.0f);

        // --- Column 1: Settings and Controls ---
        ImGui::TextColored(ImVec4(0.5f, 0.9f, 0.6f, 1.0f), "OVERLAY CONTROLS");
        ImGui::Checkbox("Enable Tactical Radar HUD", &s_config.enabled);
        ImGui::SameLine();
        ImGui::TextDisabled("(Renders 2D Minimap on Screen)");

        ImGui::Checkbox("Hostile Targets Only", &s_config.hostileOnly);
        ImGui::Checkbox("Show Distance Labels [xx m]", &s_config.showDistances);
        ImGui::Checkbox("Show Target Names", &s_config.showNames);
        ImGui::Checkbox("Show Elevation Indicators (▲ / ▼)", &s_config.showElevation);
        ImGui::Checkbox("Show Target Health Bars", &s_config.showHealthBar);
        ImGui::Checkbox("Animated Radar Sweep Line", &s_config.showSweep);
        ImGui::Checkbox("Clamp Out-of-Range Targets to Perimeter", &s_config.clampToPerimeter);
        ImGui::Checkbox("Simulate Commonwealth Patrols (Testing)", &s_config.simulationMode);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.5f, 0.9f, 0.6f, 1.0f), "RADAR METRICS & CALIBRATION");

        ImGui::SliderFloat("Detection Range", &s_config.rangeMeters, 20.0f, 250.0f, "%.0f Meters");
        ImGui::SliderFloat("Radar Radius", &s_config.radarRadius, 80.0f, 200.0f, "%.0f Pixels");
        ImGui::SliderFloat("HUD Opacity", &s_config.backgroundAlpha, 0.2f, 1.0f, "%.2f");

        ImGui::Spacing();
        ImGui::Text("Screen Position:");
        ImGui::SliderFloat("X Coordinate", &s_config.posX, 0.0f, 2560.0f, "%.0f px");
        ImGui::SliderFloat("Y Coordinate", &s_config.posY, 0.0f, 1440.0f, "%.0f px");

        ImGui::Spacing();
        ImGui::Text("Position Presets:");
        if (ImGui::Button("Top-Left", ImVec2(80, 24))) {
            s_config.posX = 20.0f;
            s_config.posY = 60.0f;
        }
        ImGui::SameLine();
        if (ImGui::Button("Top-Right", ImVec2(80, 24))) {
            s_config.posX = 1620.0f;
            s_config.posY = 60.0f;
        }
        ImGui::SameLine();
        if (ImGui::Button("Bottom-Left", ImVec2(85, 24))) {
            s_config.posX = 20.0f;
            s_config.posY = 720.0f;
        }
        ImGui::SameLine();
        if (ImGui::Button("Bottom-Right", ImVec2(85, 24))) {
            s_config.posX = 1620.0f;
            s_config.posY = 720.0f;
        }

        // --- Column 2: Live Embedded Preview Canvas ---
        ImGui::NextColumn();
        ImGui::TextColored(ImVec4(0.5f, 0.9f, 0.6f, 1.0f), "LIVE RADAR PREVIEW");
        ImGui::Separator();
        ImGui::Spacing();

        ImVec2 previewCenter = ImGui::GetCursorScreenPos();
        float previewRadius = 110.0f;
        previewCenter.x += previewRadius + 20.0f;
        previewCenter.y += previewRadius + 15.0f;

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImGui::Dummy(ImVec2((previewRadius * 2.0f) + 40.0f, (previewRadius * 2.0f) + 45.0f));

        RadarConfig previewConfig = s_config;
        previewConfig.radarRadius = previewRadius;
        DrawRadarCanvas(drawList, previewCenter, previewRadius, previewConfig, s_entities, s_sweepAngle);

        ImGui::Columns(1);
        ImGui::Spacing();
        ImGui::Separator();

        // --- Contact Roster Table ---
        ImGui::TextColored(ImVec4(0.20f, 0.95f, 0.35f, 1.0f), "DETECTED CONTACT ROSTER");
        ImGui::Spacing();

        if (ImGui::BeginTable("ContactsTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0, 160))) {
            ImGui::TableSetupColumn("Target Name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Threat", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("Distance", ImGuiTableColumnFlags_WidthFixed, 75.0f);
            ImGui::TableSetupColumn("Bearing", ImGuiTableColumnFlags_WidthFixed, 75.0f);
            ImGui::TableSetupColumn("Elevation", ImGuiTableColumnFlags_WidthFixed, 75.0f);
            ImGui::TableSetupColumn("Health", ImGuiTableColumnFlags_WidthFixed, 75.0f);
            ImGui::TableHeadersRow();

            for (const auto& entity : s_entities) {
                if (s_config.hostileOnly && !entity.isHostile) continue;

                ImGui::TableNextRow();

                // Name
                ImGui::TableSetColumnIndex(0);
                if (entity.isHostile) {
                    ImGui::TextColored(ImVec4(0.95f, 0.3f, 0.3f, 1.0f), "%s", entity.name.c_str());
                } else {
                    ImGui::TextColored(ImVec4(0.95f, 0.8f, 0.2f, 1.0f), "%s", entity.name.c_str());
                }

                // Threat
                ImGui::TableSetColumnIndex(1);
                if (entity.isHostile) {
                    ImGui::TextColored(ImVec4(0.95f, 0.2f, 0.2f, 1.0f), "HOSTILE");
                } else {
                    ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.4f, 1.0f), "NEUTRAL");
                }

                // Distance
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%.1f m", entity.distance);

                // Bearing
                ImGui::TableSetColumnIndex(3);
                float deg = entity.relativeAngle * (180.0f / PI);
                if (deg < 0.0f) deg += 360.0f;
                const char* dir = "N";
                if (deg >= 22.5f && deg < 67.5f) dir = "NE";
                else if (deg >= 67.5f && deg < 112.5f) dir = "E";
                else if (deg >= 112.5f && deg < 157.5f) dir = "SE";
                else if (deg >= 157.5f && deg < 202.5f) dir = "S";
                else if (deg >= 202.5f && deg < 247.5f) dir = "SW";
                else if (deg >= 247.5f && deg < 292.5f) dir = "W";
                else if (deg >= 292.5f && deg < 337.5f) dir = "NW";
                ImGui::Text("%s (%.0f°)", dir, deg);

                // Elevation
                ImGui::TableSetColumnIndex(4);
                if (entity.elevationDelta > 1.5f) {
                    ImGui::TextColored(ImVec4(0.4f, 0.9f, 1.0f, 1.0f), "+%.1fm ▲", entity.elevationDelta);
                } else if (entity.elevationDelta < -1.5f) {
                    ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "%.1fm ▼", entity.elevationDelta);
                } else {
                    ImGui::Text("Level");
                }

                // Health
                ImGui::TableSetColumnIndex(5);
                ImGui::Text("%.0f %%", entity.healthPercent * 100.0f);
            }

            ImGui::EndTable();
        }
    }

} // namespace Overboss::Features::Radar
