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
#include "player.h"

static bool spinBotReset = false;
static float spinAngle = 0.f;

static std::vector<SDK::AFortPlayerPawnAthena*> cachedEnemies;
static int cacheRefreshCounter = 0;
static const int CACHE_REFRESH_RATE = 10;

bool IsEnemy(SDK::APawn* localPawn, SDK::AFortPlayerPawnAthena* otherPawn) {

    if (localPawn == otherPawn) return false;

    if (otherPawn->IsDBNO() || otherPawn->bIsDying) return false;

    return true;
}

static bool IsValidTarget(SDK::AFortPawn* Pawn, SDK::AFortPawn* LocalPawn) {
    if (!Pawn || !LocalPawn) return false;
    if (Pawn == LocalPawn) return false;
    if (Pawn->IsDead()) return false;
    if (Pawn->IsDBNO()) return false;
    return true;
}

std::vector<SDK::AFortPlayerPawnAthena*> GetEnemyPawns(SDK::APawn* localPawn) {
    cacheRefreshCounter++;

    if (cacheRefreshCounter >= CACHE_REFRESH_RATE || cachedEnemies.empty()) {
        cachedEnemies.clear();
        cacheRefreshCounter = 0;

        SDK::UWorld* world = SDK::UWorld::GetWorld();
        if (!world) return cachedEnemies;

        SDK::ULevel* level = world->PersistentLevel;
        if (!level) return cachedEnemies;

        SDK::TArray<SDK::AActor*> actors = level->Actors;

        for (int i = 0; i < actors.Num(); i++) {
            SDK::AActor* actor = actors[i];
            if (!actor) continue;

            if (actor->IsA(SDK::AFortPlayerPawnAthena::StaticClass())) {
                SDK::AFortPlayerPawnAthena* pawn = static_cast<SDK::AFortPlayerPawnAthena*>(actor);

                if (IsEnemy(localPawn, pawn)) {
                    cachedEnemies.push_back(pawn);
                }
            }
        }
    }

    cachedEnemies.erase(
        std::remove_if(cachedEnemies.begin(), cachedEnemies.end(),
            [](SDK::AFortPlayerPawnAthena* pawn) {
                return !pawn || pawn->IsDBNO() || pawn->bIsDying;
            }),
        cachedEnemies.end()
    );

    return cachedEnemies;
}

void changeFov(float FOV) {
    __try {
        SDK::UWorld* world = SDK::UWorld::GetWorld();
        if (!world || !world->OwningGameInstance) {
            return;
        }

        if (world->OwningGameInstance->LocalPlayers.Num() <= 0) {
            return;
        }

        SDK::ULocalPlayer* localPlayer = world->OwningGameInstance->LocalPlayers[0];
        if (!localPlayer || !localPlayer->PlayerController) {
            return;
        }

        SDK::APlayerController* pc = localPlayer->PlayerController;
        if (!pc || !pc->PlayerCameraManager) {
            return;
        }

        SDK::APlayerCameraManager* camera = pc->PlayerCameraManager;

        float currentFov = camera->GetFOVAngle();

        if (currentFov == FOV) return;

        pc->FOV(FOV);

    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return;
    }
}

void changeName() {
    __try {
        SDK::UWorld* world = SDK::UWorld::GetWorld();
        if (!world || !world->OwningGameInstance) return;
        if (world->OwningGameInstance->LocalPlayers.Num() == 0) return;

        SDK::APlayerController* player = world->OwningGameInstance->LocalPlayers[0]->PlayerController;
        if (!player) return;
        SDK::APawn* pawn = player->Pawn;
        if (!pawn) return;

        if (PlayerConfig::name.empty()) return;

        wchar_t nameBuffer[256];
        MultiByteToWideChar(CP_ACP, 0, PlayerConfig::name.c_str(), -1, nameBuffer, 256);
        const SDK::FString playerName = SDK::FString(nameBuffer);

        player->ServerChangeName(playerName);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return;
    }
}

void ziplineFly() {
    SDK::UWorld* world = SDK::UWorld::GetWorld();
    SDK::APlayerController* player = world->OwningGameInstance->LocalPlayers[0]->PlayerController;
    SDK::APawn* pawn = player->Pawn;

    if (!pawn || !pawn->IsA(SDK::AFortPlayerPawnAthena::StaticClass())) return;

    SDK::AFortPlayerPawnAthena* fortPawn = (SDK::AFortPlayerPawnAthena*)pawn;
    fortPawn->ZiplineState.bIsZiplining = true;
}

void reviveSelf() {
    if (!PlayerConfig::selfRevive) return;

    SDK::UWorld* world = SDK::UWorld::GetWorld();
    if (!world || !world->OwningGameInstance) return;

    if (world->OwningGameInstance->LocalPlayers.Num() == 0) return;

    SDK::APlayerController* localPlayer = world->OwningGameInstance->LocalPlayers[0]->PlayerController;

    if (!localPlayer) return;

    SDK::APawn* localPawn = localPlayer->AcknowledgedPawn;

    if (!localPawn || !localPawn->IsA(SDK::AFortPlayerPawnAthena::StaticClass())) return;

    SDK::AFortPlayerPawnAthena* fortPawn = static_cast<SDK::AFortPlayerPawnAthena*>(localPawn);

    if (!fortPawn->IsDBNO()) return;

    fortPawn->ServerReviveFromDBNO(localPlayer);
}

void instantRevive() {
    if (!PlayerConfig::instantRevive) return;
    SDK::UWorld* world = SDK::UWorld::GetWorld();
    if (!world || !world->OwningGameInstance) return;
    if (world->OwningGameInstance->LocalPlayers.Num() == 0) return;

    SDK::APlayerController* localPlayer = world->OwningGameInstance->LocalPlayers[0]->PlayerController;

    if (!localPlayer) return;

    SDK::APawn* localPawn = localPlayer->AcknowledgedPawn;
    if (!localPawn || !localPawn->IsA(SDK::AFortPlayerPawnAthena::StaticClass())) return;
    SDK::AFortPlayerPawnAthena* fortPawn = static_cast<SDK::AFortPlayerPawnAthena*>(localPawn);
    if (!fortPawn->IsDBNO()) return;
    fortPawn->ReviveFromDBNOTime = 0.1f;
}

void teleportEnemiesToYou() {
    static bool ConfigTPEnemiesToYou = true;

    if (!ConfigTPEnemiesToYou) return;

    SDK::UWorld* world = SDK::UWorld::GetWorld();
    if (!world || !world->OwningGameInstance) return;
    if (world->OwningGameInstance->LocalPlayers.Num() == 0) return;

    SDK::APlayerController* localPlayer = world->OwningGameInstance->LocalPlayers[0]->PlayerController;
    if (!localPlayer || !localPlayer->PlayerCameraManager) return;

    SDK::APawn* localPawn = localPlayer->AcknowledgedPawn;
    if (!localPawn) return;

    SDK::FVector cameraLocation = localPlayer->PlayerCameraManager->GetCameraLocation();
    SDK::FRotator cameraRotation = localPlayer->PlayerCameraManager->GetCameraRotation();

    float teleportDistance = PlayerConfig::tpDistance;

    SDK::FVector forwardVector = Math::GetForwardVector(cameraRotation);
    SDK::FVector targetPosition = cameraLocation + (forwardVector * teleportDistance);

    SDK::AFortTeamInfo* localTeam = nullptr;
    if (localPawn->PlayerState && localPawn->PlayerState->IsA(SDK::AFortPlayerState::StaticClass())) {
        SDK::AFortPlayerState* localPS = static_cast<SDK::AFortPlayerState*>(localPawn->PlayerState);
        localTeam = localPS->PlayerTeam;
    }

    if (!world->PersistentLevel) return;

    int teleportedCount = 0;
    const int maxTeleportsPerTick = 3;

    for (auto* actor : world->PersistentLevel->Actors) {
        if (!actor) continue;

        if (!actor->IsA(SDK::AFortPlayerPawn::StaticClass())) continue;

        SDK::AFortPawn* enemy = static_cast<SDK::AFortPawn*>(actor);
        if (enemy == localPawn) continue;

        if (enemy->IsDead()) continue;

        if (localTeam && enemy->PlayerState && enemy->PlayerState->IsA(SDK::AFortPlayerState::StaticClass())) {
            SDK::AFortPlayerState* enemyPS = static_cast<SDK::AFortPlayerState*>(enemy->PlayerState);
            if (enemyPS->PlayerTeam == localTeam) continue;

        }

        enemy->K2_SetActorLocation(targetPosition, false, nullptr, true);

        teleportedCount++;
        if (teleportedCount >= maxTeleportsPerTick) break;

    }
}


void Player::Tick() {
    SDK::UWorld* world = SDK::UWorld::GetWorld();
    if (!world || !world->OwningGameInstance) return;
    if (world->OwningGameInstance->LocalPlayers.Num() == 0) return;

    SDK::APlayerController* localPlayer = world->OwningGameInstance->LocalPlayers[0]->PlayerController;
    if (!localPlayer) return;

    SDK::APawn* pawn = localPlayer->AcknowledgedPawn;
    if (!pawn) return;

    if (PlayerConfig::fovChange) {
        changeFov(PlayerConfig::fovChangeAmount);
    }
    else {
        changeFov(0);

    }

    if (PlayerConfig::selfRevive) {
        reviveSelf();
    }

    if (PlayerConfig::instantRevive) {
        instantRevive();
    }

    if (PlayerConfig::ziplineFly) {
        ziplineFly();
    }

    if (PlayerConfig::spinBotEnabled) {

        SDK::USkeletalMeshComponent* mesh = nullptr;

        if (pawn->IsA(SDK::ACharacter::StaticClass())) {
            SDK::ACharacter* character = static_cast<SDK::ACharacter*>(pawn);
            mesh = character->Mesh;
        }

        if (mesh) {

            spinAngle += PlayerConfig::spinBotSpeed;
            if (spinAngle > 360.f) spinAngle -= 360.f;

            SDK::FRotator meshRotation = mesh->RelativeRotation;
            meshRotation.Yaw = spinAngle;
            mesh->RelativeRotation = meshRotation;

            spinBotReset = false;
        }
    }
    else if (!PlayerConfig::spinBotEnabled && !spinBotReset) {

        SDK::USkeletalMeshComponent* mesh = nullptr;

        if (pawn->IsA(SDK::ACharacter::StaticClass())) {
            SDK::ACharacter* character = static_cast<SDK::ACharacter*>(pawn);
            mesh = character->Mesh;
        }

        if (mesh) {
            SDK::FRotator meshRotation = mesh->RelativeRotation;
            meshRotation.Yaw = -90.f;

            mesh->RelativeRotation = meshRotation;

            spinBotReset = true;
            spinAngle = 0.f;

        }
    }

    if (PlayerConfig::tpEnemies) {
        teleportEnemiesToYou();
    }
}