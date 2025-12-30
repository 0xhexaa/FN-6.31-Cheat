#include "../Public/Configuration.h"
#include "../Game/SDK/Engine_classes.hpp"
#include <Windows.h>
#include "../Public/log.h"
#include <string>
#include "gui.h"
#include <algorithm>
#include <vector>
#include "../Game/SDK/FortniteGame_classes.hpp"
#include "../Game/SDK/FortniteGame_parameters.hpp"
#include "../Game/SDK/FortniteGame_structs.hpp"
#include <unordered_map>
#include <float.h>
#include "Bones.h"
#include "maths.h"
#include "aimbot.h"
#include <player.h>
#include "../Game/SDK/B_Melee_Generic_classes.hpp"
#include "weapon.h"

SDK::FFortBaseWeaponStats* GetWeaponStats(SDK::AFortWeapon* weapon) {
    if (!weapon) return nullptr;

    constexpr int GetWeaponStats_Idx = 0xCF;

    void** vtable = *(void***)weapon;
    auto GetWeaponStatsFunc = reinterpret_cast<SDK::FFortBaseWeaponStats * (*)(SDK::AFortWeapon*)>(
        vtable[GetWeaponStats_Idx]
        );

    return GetWeaponStatsFunc(weapon);
}

struct WeaponBaseBackup {
    float MinChargeTime;
    float MaxChargeTime;
    float ChargeDownTime;
    float MinChargeDamageMultiplier;
    float MaxChargeDamageMultiplier;
    float RecoilVert;
    float RecoilHoriz;
    float RecoilVertScaleGamepad;
    float Spread;
    float SpreadDownsights;
    float StandingStillSpreadMultiplier;
    float AthenaCrouchingSpreadMultiplier;
    float AthenaJumpingFallingSpreadMultiplier;
    float AthenaSprintingSpreadMultiplier;
    float MinSpeedForSpreadMultiplier;
    float MaxSpeedForSpreadMultiplier;
    float FiringRate;
    float ReloadTime;
    float BulletsPerCartridge;
    float SwingPlaySpeed;
    float RangeVSBuildings2D;
    float RangeVSBuildingsZ;
    float RangeVSWeakSpots;
    float RangeVSEnemies;
};

std::unordered_map<SDK::AFortWeapon*, WeaponBaseBackup> weaponBackup;

void WeaponExploits() {
    SDK::UWorld* world = SDK::UWorld::GetWorld();
    if (!world) return;

    SDK::APlayerController* localPlayer = world->OwningGameInstance->LocalPlayers[0]->PlayerController;
    if (!localPlayer) return;

    SDK::APawn* localPawn = localPlayer->AcknowledgedPawn;
    if (!localPawn) return;

    SDK::AFortPlayerPawnAthena* fortPawn = static_cast<SDK::AFortPlayerPawnAthena*>(localPawn);
    if (!fortPawn) return;

    SDK::AFortWeapon* currentWeapon = static_cast<SDK::AFortWeapon*>(fortPawn->CurrentWeapon);
    if (!currentWeapon) return;

    SDK::FFortBaseWeaponStats* baseStats = GetWeaponStats(currentWeapon);
    if (!baseStats) return;

    if (weaponBackup.find(currentWeapon) == weaponBackup.end()) {
        WeaponBaseBackup backup;
        backup.MinChargeTime = baseStats->MinChargeTime;
        backup.MaxChargeTime = baseStats->MaxChargeTime;
        backup.ChargeDownTime = baseStats->ChargeDownTime;
        backup.MinChargeDamageMultiplier = baseStats->MinChargeDamageMultiplier;
        backup.MaxChargeDamageMultiplier = baseStats->MaxChargeDamageMultiplier;

        SDK::FFortRangedWeaponStats* stats = static_cast<SDK::FFortRangedWeaponStats*>(baseStats);
        if (stats) {
            backup.RecoilVert = stats->RecoilVert;
            backup.RecoilHoriz = stats->RecoilHoriz;
            backup.RecoilVertScaleGamepad = stats->RecoilVertScaleGamepad;
            backup.Spread = stats->Spread;
            backup.SpreadDownsights = stats->SpreadDownsights;
            backup.StandingStillSpreadMultiplier = stats->StandingStillSpreadMultiplier;
            backup.AthenaCrouchingSpreadMultiplier = stats->AthenaCrouchingSpreadMultiplier;
            backup.AthenaJumpingFallingSpreadMultiplier = stats->AthenaJumpingFallingSpreadMultiplier;
            backup.AthenaSprintingSpreadMultiplier = stats->AthenaSprintingSpreadMultiplier;
            backup.MinSpeedForSpreadMultiplier = stats->MinSpeedForSpreadMultiplier;
            backup.MaxSpeedForSpreadMultiplier = stats->MaxSpeedForSpreadMultiplier;
            backup.FiringRate = stats->FiringRate;
            backup.ReloadTime = stats->ReloadTime;
            backup.BulletsPerCartridge = stats->BulletsPerCartridge;
        }

        SDK::AB_Melee_Generic_C* melee = static_cast<SDK::AB_Melee_Generic_C*>(currentWeapon);
        if (melee) {
            SDK::FFortMeleeWeaponStats* meleeStats = static_cast<SDK::FFortMeleeWeaponStats*>(baseStats);
            backup.SwingPlaySpeed = meleeStats->SwingPlaySpeed;
        }

        weaponBackup[currentWeapon] = backup;
    }

    SDK::FFortRangedWeaponStats* stats = static_cast<SDK::FFortRangedWeaponStats*>(baseStats);
    if (stats) {
        auto& backup = weaponBackup[currentWeapon];

        if (WeaponConfig::noCharge) {
            stats->MinChargeTime = 0.f;
            stats->MaxChargeTime = 0.f;
            stats->ChargeDownTime = 0.f;
            stats->MinChargeDamageMultiplier = 0.f;
            stats->MaxChargeDamageMultiplier = 0.f;
        }
        else {
            stats->MinChargeTime = backup.MinChargeTime;
            stats->MaxChargeTime = backup.MaxChargeTime;
            stats->ChargeDownTime = backup.ChargeDownTime;
            stats->MinChargeDamageMultiplier = backup.MinChargeDamageMultiplier;
            stats->MaxChargeDamageMultiplier = backup.MaxChargeDamageMultiplier;
        }

        stats->RecoilVert = WeaponConfig::noRecoil ? 0.f : backup.RecoilVert;
        stats->RecoilHoriz = WeaponConfig::noRecoil ? 0.f : backup.RecoilHoriz;
        stats->RecoilVertScaleGamepad = WeaponConfig::noRecoil ? 0.f : backup.RecoilVertScaleGamepad;

        stats->Spread = WeaponConfig::noSpread ? 0.f : backup.Spread;
        stats->SpreadDownsights = WeaponConfig::noSpread ? 0.f : backup.SpreadDownsights;
        stats->StandingStillSpreadMultiplier = WeaponConfig::noSpread ? 0.f : backup.StandingStillSpreadMultiplier;
        stats->AthenaCrouchingSpreadMultiplier = WeaponConfig::noSpread ? 0.f : backup.AthenaCrouchingSpreadMultiplier;
        stats->AthenaJumpingFallingSpreadMultiplier = WeaponConfig::noSpread ? 0.f : backup.AthenaJumpingFallingSpreadMultiplier;
        stats->AthenaSprintingSpreadMultiplier = WeaponConfig::noSpread ? 0.f : backup.AthenaSprintingSpreadMultiplier;
        stats->MinSpeedForSpreadMultiplier = WeaponConfig::noSpread ? FLT_MAX : backup.MinSpeedForSpreadMultiplier;
        stats->MaxSpeedForSpreadMultiplier = WeaponConfig::noSpread ? FLT_MAX : backup.MaxSpeedForSpreadMultiplier;

        stats->FiringRate = WeaponConfig::fireRate ? backup.FiringRate * WeaponConfig::fireRateMulitplier : backup.FiringRate;
        stats->ReloadTime = WeaponConfig::noReload ? 0.f : backup.ReloadTime;
        stats->BulletsPerCartridge = WeaponConfig::damageEnable ? backup.BulletsPerCartridge * WeaponConfig::damageMulitplier : backup.BulletsPerCartridge;

        return;
    }
    return;
}



void Weapon::Tick() {
    WeaponExploits();
}