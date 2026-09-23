#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace Overboss::Features {

    namespace Vitals {
        void RenderTab();
        void ToggleGodMode();
        void ToggleDemiGod();
        void ToggleNoClip();
        void ApplySpeedMult(float mult);
        void ApplyJumpHeight(float height);
        void ApplyCarryWeight(float weight);
        void HealPlayer();
        void RestoreActionPoints();
        void CureRadiation();
    }

    namespace Armory {
        void RenderTab();
        void ToggleInfiniteAmmo();
        void GiveWeapon(std::string_view name, std::string_view formId);
        void GiveAmmo(std::string_view name, std::string_view formId, uint32_t count);
        void AttachLegendaryMod(std::string_view modId);
    }

    namespace PowerArmor {
        void RenderTab();
        void ToggleFusionCoreFreeze();
        void SpawnFrame();
        void DeliverSuit(std::string_view suitType);
        void DeliverFusionCores(uint32_t count = 50);
    }

    namespace Settlement {
        void RenderTab();
        void AddCaps(uint32_t count = 50000);
        void AddBobbyPins(uint32_t count = 500);
        void DeliverAllCraftingShipments(uint32_t count = 5000);
        void BypassBuildBudget();
    }

    namespace World {
        void RenderTab();
        void FastTravel(std::string_view destination, std::string_view cellOrLoc);
        void RevealMapMarkers();
        void SetGameTimescale(float scale);
        void SetGameWeather(std::string_view weatherId);
    }

} // namespace Overboss::Features
