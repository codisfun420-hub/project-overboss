#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace Overboss::Features::Radar {

    struct RadarEntity {
        std::string name;
        float distance;        // Distance in meters (game units / 70.0f)
        float rawDistance;     // Raw distance in Creation Engine units
        float relativeAngle;   // Angle in radians relative to player's forward vector (-PI to +PI)
        float worldAngle;      // Angle in world space radians
        float worldX;
        float worldY;
        float worldZ;
        float elevationDelta;  // Z delta relative to player in meters
        bool isHostile;
        float healthPercent;   // 0.0f to 1.0f
        uint32_t formId;
    };

    struct RadarConfig {
        bool enabled = true;               // Persistent HUD radar enabled
        bool hostileOnly = false;          // Filter: only show hostile enemies
        bool showDistances = true;         // Render distance tags [xx m]
        bool showNames = true;             // Render entity names
        bool showElevation = true;         // Render ▲ / ▼ elevation indicators
        bool showHealthBar = true;         // Render health gauge below blip
        bool rotateWithPlayer = true;      // Forward is UP (rotates with camera) vs North is UP
        bool showSweep = true;             // Animated radar sweep line
        bool clampToPerimeter = true;      // Clamp out-of-range targets to circle edge
        float rangeMeters = 100.0f;        // Max detection radius in meters
        float radarRadius = 120.0f;        // Display circle radius in screen pixels
        float backgroundAlpha = 0.80f;     // HUD background opacity
        float posX = 24.0f;                // Screen X position
        float posY = 65.0f;                // Screen Y position
        bool simulationMode = false;       // Preview mock hostile squad if game in loading/menu
    };

    // Configuration access
    RadarConfig& GetConfig();

    // Renders the radar overlay on the in-game HUD (called every frame in DX11 Present)
    void RenderOverlay();

    // Renders the dedicated Radar & ESP configuration and preview tab in the Pip-Boy menu
    void RenderTab();

    // Updates entity tracking and scans memory/Creation Engine
    void Update(float deltaTime);

    // Entity querying
    const std::vector<RadarEntity>& GetTrackedEntities();

    // Quick helpers
    bool IsEnabled();
    void SetEnabled(bool enabled);
    float GetRange();
    void SetRange(float range);
    bool IsHostileOnly();
    void SetHostileOnly(bool hostileOnly);
    bool IsShowDistance();
    void SetShowDistance(bool show);

} // namespace Overboss::Features::Radar
