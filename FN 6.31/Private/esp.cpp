#include "MinHook.h"
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
#include <chrono>
#include "../imgui/imgui.h"
#include "Configuration.h"
#include "../Game/SDK/FortniteGame_classes.hpp"
#include "../Game/SDK/Engine_classes.hpp"
#include "esp.h"
#include "Bones.h"
#include "aimbot.h"

#undef max
#undef min

static SDK::UFont* CachedFont = nullptr;

static auto LastFPSUpdateTime = std::chrono::steady_clock::now();
static int FrameCount = 0;
static float CurrentFPS = 0.0f;

bool SafeWorldToScreen(SDK::APlayerController* playerController, const SDK::FVector& worldPos, SDK::FVector2D& screenPos) {
    if (!playerController) return false;

    if (worldPos.X == 0.0f && worldPos.Y == 0.0f && worldPos.Z == 0.0f)
        return false;

    bool success = playerController->ProjectWorldLocationToScreen(worldPos, &screenPos, false);

    if (!success) return false;

    int screenWidth, screenHeight;
    playerController->GetViewportSize(&screenWidth, &screenHeight);

    const float margin = 100.0f;
    bool isOnScreen = (screenPos.X >= -margin && screenPos.X <= screenWidth + margin &&
        screenPos.Y >= -margin && screenPos.Y <= screenHeight + margin);

    return isOnScreen;
}

SDK::FVector GetForwardVectorFromRotation(const SDK::FRotator& rotation) {
    float pitchRad = rotation.Pitch * (3.14159265358979323846f / 180.0f);
    float yawRad = rotation.Yaw * (3.14159265358979323846f / 180.0f);

    float sp = sin(pitchRad);
    float cp = cos(pitchRad);
    float sy = sin(yawRad);
    float cy = cos(yawRad);

    return SDK::FVector(cp * cy, cp * sy, sp);
}

bool IsBehindCamera(SDK::APlayerController* playerController, const SDK::FVector& worldPos) {
    if (!playerController || !playerController->Pawn) return true;

    SDK::APawn* localPawn = playerController->Pawn;
    SDK::FVector cameraLocation;
    SDK::FRotator cameraRotation;

    localPawn->GetActorEyesViewPoint(&cameraLocation, &cameraRotation);

    SDK::FVector direction = worldPos - cameraLocation;

    if (direction.X == 0.0f && direction.Y == 0.0f && direction.Z == 0.0f)
        return false;

    float length = sqrt(direction.X * direction.X +
        direction.Y * direction.Y +
        direction.Z * direction.Z);
    if (length > 0.0f) {
        direction.X /= length;
        direction.Y /= length;
        direction.Z /= length;
    }

    SDK::FVector cameraForward = GetForwardVectorFromRotation(cameraRotation);

    float forwardLength = sqrt(cameraForward.X * cameraForward.X +
        cameraForward.Y * cameraForward.Y +
        cameraForward.Z * cameraForward.Z);
    if (forwardLength > 0.0f) {
        cameraForward.X /= forwardLength;
        cameraForward.Y /= forwardLength;
        cameraForward.Z /= forwardLength;
    }

    float dotProduct = (direction.X * cameraForward.X +
        direction.Y * cameraForward.Y +
        direction.Z * cameraForward.Z);

    return dotProduct < 0.0f;
}

void UpdateFPS()
{
    FrameCount++;
    auto CurrentTime = std::chrono::steady_clock::now();
    auto ElapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(CurrentTime - LastFPSUpdateTime).count();

    if (ElapsedTime >= 1000)
    {
        CurrentFPS = static_cast<float>(FrameCount) * 1000.0f / static_cast<float>(ElapsedTime);
        FrameCount = 0;
        LastFPSUpdateTime = CurrentTime;
    }
}

void DrawWatermark()
{
    UpdateFPS();

    char watermarkText[64];
    snprintf(watermarkText, sizeof(watermarkText), "Made by Hexa - %.1f FPS", CurrentFPS);

    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    if (!drawList) return;

    ImVec2 textPos = ImVec2(115.0f, 10.0f);
    ImVec2 shadowPos = ImVec2(116.0f, 11.0f);

    drawList->AddText(shadowPos, IM_COL32(0, 0, 0, 255), watermarkText);
    drawList->AddText(textPos, IM_COL32(0, 255, 0, 255), watermarkText);
}

void DrawESP() {
    if (!ESPConfig::enabled) return;

    SDK::UWorld* World = SDK::UWorld::GetWorld();
    if (!World || !World->OwningGameInstance) return;
    if (World->OwningGameInstance->LocalPlayers.Num() <= 0) return;

    SDK::APlayerController* localPlayer = World->OwningGameInstance->LocalPlayers[0]->PlayerController;
    if (!localPlayer || !localPlayer->Pawn) return;

    SDK::APawn* localPawn = localPlayer->Pawn;

    SDK::FVector viewPoint;
    SDK::FRotator viewRotation;
    localPawn->GetActorEyesViewPoint(&viewPoint, &viewRotation);

    if (!World->PersistentLevel) return;

    SDK::AFortTeamInfo* localTeam = nullptr;
    if (localPawn)
    {
        if (localPawn->PlayerState &&
            localPawn->PlayerState->IsA(SDK::AFortPlayerState::StaticClass()))
        {
            SDK::AFortPlayerState* localPS =
                static_cast<SDK::AFortPlayerState*>(localPawn->PlayerState);
            localTeam = localPS->PlayerTeam;
        }
    }

    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    if (!drawList) return;

    int screenWidth, screenHeight;
    localPlayer->GetViewportSize(&screenWidth, &screenHeight);

    for (auto* Actor : World->PersistentLevel->Actors)
    {
        if (!Actor) continue;
        if (!Actor->IsA(SDK::AFortPlayerPawn::StaticClass())) continue;

        SDK::AFortPlayerPawn* targetPawn = static_cast<SDK::AFortPlayerPawn*>(Actor);
        if (targetPawn == localPawn) continue;

        if (!targetPawn->PlayerState ||
            !targetPawn->PlayerState->IsA(SDK::AFortPlayerState::StaticClass()))
            continue;

        SDK::AFortPlayerState* targetPlayerState = static_cast<SDK::AFortPlayerState*>(targetPawn->PlayerState);
        if (!targetPlayerState) continue;

        if (localTeam && targetPlayerState->PlayerTeam == localTeam)
            continue;

        if (targetPawn->IsDead()) continue;

        SDK::FVector targetPos = targetPawn->K2_GetActorLocation();
        if (targetPos.X == 0.0f && targetPos.Y == 0.0f && targetPos.Z == 0.0f) continue;

        if (IsBehindCamera(localPlayer, targetPos))
            continue;

        SDK::FVector directionToTarget = targetPos - viewPoint;
        float distanceSq = directionToTarget.X * directionToTarget.X +
            directionToTarget.Y * directionToTarget.Y +
            directionToTarget.Z * directionToTarget.Z;
        float maxDistSq = ESPConfig::maxDistance * ESPConfig::maxDistance;
        maxDistSq *= 1000.0f;
        if (distanceSq > maxDistSq) continue;
        if (distanceSq < 1.0f) continue;

        SDK::FVector2D screenPos;
        if (!SafeWorldToScreen(localPlayer, targetPos, screenPos))
            continue;

        float distance = sqrt(distanceSq);
        const float characterHeight = 180.0f;
        const float halfHeight = characterHeight * 0.5f;

        SDK::FVector headPos = targetPos;
        headPos.Z += halfHeight;
        SDK::FVector feetPos = targetPos;
        feetPos.Z -= halfHeight;

        SDK::FVector2D headScreen;
        SDK::FVector2D feetScreen;

        if (!SafeWorldToScreen(localPlayer, headPos, headScreen) ||
            !SafeWorldToScreen(localPlayer, feetPos, feetScreen))
            continue;

        float boxHeight = fabsf(feetScreen.Y - headScreen.Y);
        if (boxHeight < 5.0f) continue;
        float boxWidth = boxHeight * 0.4f;

        float left = headScreen.X - boxWidth * 0.5f;
        float right = headScreen.X + boxWidth * 0.5f;
        float top = headScreen.Y;
        float bottom = feetScreen.Y;

        left = std::max(0.0f, std::min(left, (float)screenWidth));
        right = std::max(0.0f, std::min(right, (float)screenWidth));
        top = std::max(0.0f, std::min(top, (float)screenHeight));
        bottom = std::max(0.0f, std::min(bottom, (float)screenHeight));

        ImU32 boxColor;
        if (ESPConfig::boxVisibilityCheck)
        {
            SDK::FVector dirToHead = headPos - viewPoint;
            float distToHead = sqrt(dirToHead.X * dirToHead.X + dirToHead.Y * dirToHead.Y + dirToHead.Z * dirToHead.Z);
            boxColor = (distToHead < 2000.0f) ?
                IM_COL32(0, 255, 0, (int)(ESPConfig::boxColorA * 255 / 255)) :
                IM_COL32(255, 0, 0, (int)(ESPConfig::boxColorA * 255 / 255));
        }
        else
        {
            boxColor = IM_COL32(
                (int)ESPConfig::boxColorR,
                (int)ESPConfig::boxColorG,
                (int)ESPConfig::boxColorB,
                (int)ESPConfig::boxColorA
            );
        }

        if (ESPConfig::filledBox)
        {
            ImU32 fillColor = IM_COL32(
                (int)ESPConfig::filledBoxColorR,
                (int)ESPConfig::filledBoxColorG,
                (int)ESPConfig::filledBoxColorB,
                (int)ESPConfig::filledBoxColorA
            );
            drawList->AddRectFilled(
                ImVec2(left, top),
                ImVec2(right, bottom),
                fillColor, 0, 0
            );
        }

        if (ESPConfig::showBoxes)
        {
            drawList->AddRect(
                ImVec2(left, top),
                ImVec2(right, bottom),
                boxColor, 0, 0,
                ESPConfig::boxThickness
            );
        }

        if (ESPConfig::showSnaplines)
        {
            ImU32 snaplineColor = IM_COL32(
                (int)ESPConfig::snaplineColorR,
                (int)ESPConfig::snaplineColorG,
                (int)ESPConfig::snaplineColorB,
                (int)ESPConfig::snaplineColorA
            );

            ImVec2 screenPosition;
            switch (ESPConfig::snaplinePosition) {
            case 0: screenPosition = ImVec2((float)screenWidth * 0.5f, (float)screenHeight); break;
            case 1: screenPosition = ImVec2((float)screenWidth * 0.5f, (float)screenHeight * 0.5f); break;
            case 2: screenPosition = ImVec2((float)screenWidth * 0.5f, 0.0f); break;
            default: screenPosition = ImVec2((float)screenWidth * 0.5f, (float)screenHeight);
            }

            ImVec2 boxBottomCenter = ImVec2((left + right) * 0.5f, bottom);
            drawList->AddLine(screenPosition, boxBottomCenter, snaplineColor, ESPConfig::snaplineThickness);
        }

        float textOffsetTop = top;
        float textOffsetBottom = bottom;

        if (ESPConfig::showNames && targetPlayerState)
        {
            std::string name = targetPlayerState->PlayerNamePrivate.ToString();
            if (name.empty()) name = "Bot/NPC";

            ImVec2 textSize = ImGui::CalcTextSize(name.c_str());

            textOffsetTop -= textSize.y + 2.0f;

            float boxCenterX = (left + right) * 0.5f;
            ImVec2 textPos = ImVec2(boxCenterX - textSize.x * 0.5f, textOffsetTop);
            ImVec2 shadowPos = ImVec2(textPos.x + 1.0f, textPos.y + 1.0f);

            ImU32 nameColor = IM_COL32(255, 255, 255, 255);
            ImU32 shadowColor = IM_COL32(0, 0, 0, 255);

            drawList->AddText(shadowPos, shadowColor, name.c_str());

            drawList->AddText(textPos, nameColor, name.c_str());
        }

        if (ESPConfig::showPlatform && targetPlayerState)
        {
            SDK::FString platform = targetPlayerState->Platform;
            std::string platformStr = platform.ToString();

            std::string displayPlatform = platformStr;

            if (platformStr.find("PSN") != std::string::npos) {
                displayPlatform = "PlayStation";
            }
            else if (platformStr.find("XBL") != std::string::npos) {
                displayPlatform = "Xbox";
            }
            else if (platformStr.find("WIN") != std::string::npos) {
                displayPlatform = "Windows";
            }
            else if (platformStr.find("MAC") != std::string::npos) {
                displayPlatform = "MacOS";
            }
            else if (platformStr.find("LNX") != std::string::npos) {
                displayPlatform = "Linux";
            }
            else if (platformStr.find("IOS") != std::string::npos) {
                displayPlatform = "iOS";
            }
            else if (platformStr.find("AND") != std::string::npos) {
                displayPlatform = "Android";
            }
            else if (platformStr.find("SWT") != std::string::npos) {
                displayPlatform = "Switch";
            }
            else {
                displayPlatform = "Unknown";
            }

            ImVec2 textSize = ImGui::CalcTextSize(displayPlatform.c_str());

            float platformY = (ESPConfig::showNames) ? (textOffsetTop - textSize.y - 2.0f) : (top - textSize.y - 2.0f);
            textOffsetTop = platformY;

            float boxCenterX = (left + right) * 0.5f;
            ImVec2 textPos = ImVec2(boxCenterX - textSize.x * 0.5f, platformY);
            ImVec2 shadowPos = ImVec2(textPos.x + 1.0f, textPos.y + 1.0f);

            ImU32 platformColor = IM_COL32(150, 200, 255, 255);
            ImU32 shadowColor = IM_COL32(0, 0, 0, 255);

            drawList->AddText(shadowPos, shadowColor, displayPlatform.c_str());

            drawList->AddText(textPos, platformColor, displayPlatform.c_str());
        }

        if (ESPConfig::showWeapons && targetPawn)
        {
            SDK::AFortWeapon* currentWeapon = targetPawn->CurrentWeapon;
            if (currentWeapon && currentWeapon->WeaponData)
            {
                SDK::FText displayName = currentWeapon->WeaponData->DisplayName;
                std::string weaponName = displayName.ToString();

                if (!weaponName.empty())
                {
                    ImVec2 textSize = ImGui::CalcTextSize(weaponName.c_str());

                    ImVec2 textPos = ImVec2((left + right) * 0.5f - textSize.x * 0.5f, textOffsetBottom + 2.0f);
                    ImVec2 shadowPos = ImVec2(textPos.x + 1.0f, textPos.y + 1.0f);

                    textOffsetBottom = textPos.y + textSize.y;

                    ImU32 weaponColor = IM_COL32(255, 215, 0, 255);
                    ImU32 shadowColor = IM_COL32(0, 0, 0, 255);

                    drawList->AddText(shadowPos, shadowColor, weaponName.c_str());

                    drawList->AddText(textPos, weaponColor, weaponName.c_str());
                }
            }
        }

        if (ESPConfig::showHP && targetPawn)
        {
            float health = targetPawn->GetHealth();
            float healthPercentage = health / 100.0f;
            healthPercentage = std::max(0.0f, std::min(1.0f, healthPercentage));

            float healthBarHeight = boxHeight * healthPercentage;
            float healthBarTop = bottom - healthBarHeight;

            drawList->AddRectFilled(
                ImVec2(left - 6.0f, top),
                ImVec2(left - 3.0f, bottom),
                IM_COL32(50, 50, 50, 200),
                2.0f
            );

            ImU32 healthColor;
            if (healthPercentage > 0.5f) {
                int green = 255;
                int red = (int)(255 * (1.0f - healthPercentage) * 2.0f);
                healthColor = IM_COL32(red, green, 0, 255);
            }
            else {
                int red = 255;
                int green = (int)(255 * healthPercentage * 2.0f);
                healthColor = IM_COL32(red, green, 0, 255);
            }

            drawList->AddRectFilled(
                ImVec2(left - 6.0f, healthBarTop),
                ImVec2(left - 3.0f, bottom),
                healthColor,
                2.0f
            );

            char healthText[16];
            sprintf_s(healthText, "%d HP", (int)health);

            ImVec2 textSize = ImGui::CalcTextSize(healthText);

            ImVec2 healthTextPos = ImVec2(right + 5.0f, top + (boxHeight - textSize.y) * 0.5f);
            ImVec2 healthTextShadow = ImVec2(healthTextPos.x + 1.0f, healthTextPos.y + 1.0f);

            ImU32 healthTextColor = healthColor;
            ImU32 shadowColor = IM_COL32(0, 0, 0, 255);

            drawList->AddText(healthTextShadow, shadowColor, healthText);

            drawList->AddText(healthTextPos, healthTextColor, healthText);
        }

        if (ESPConfig::showDistance)
        {
            int distanceMeters = (int)(distance / 100.0f);
            char distanceText[32];
            sprintf_s(distanceText, "[%dm]", distanceMeters);

            ImVec2 textSize = ImGui::CalcTextSize(distanceText);

            float distanceY = (ESPConfig::showWeapons) ? (textOffsetBottom + 2.0f) : (bottom + 2.0f);

            float boxCenterX = (left + right) * 0.5f;
            ImVec2 textPos = ImVec2(boxCenterX - textSize.x * 0.5f, distanceY);
            ImVec2 shadowPos = ImVec2(textPos.x + 1.0f, textPos.y + 1.0f);

            ImU32 distanceColor = IM_COL32(200, 200, 200, 255);
            ImU32 shadowColor = IM_COL32(0, 0, 0, 255);

            drawList->AddText(shadowPos, shadowColor, distanceText);

            drawList->AddText(textPos, distanceColor, distanceText);
        }

        if (ESPConfig::headCircle) {
            SDK::FVector headCirclePos = targetPos;
            headCirclePos.Z += halfHeight;
            SDK::FVector2D headCircleScreen;
            if (SafeWorldToScreen(localPlayer, headCirclePos, headCircleScreen)) {
                float radius = boxHeight * 0.15f;
                ImU32 circleColor = IM_COL32(0, 0, 0, 0);
                drawList->AddCircle(
                    ImVec2(headCircleScreen.X, headCircleScreen.Y),
                    radius,
                    circleColor,
                    0,
                    7.5f
                );
			}
        }
    }
}

void DrawBoneESP() {
    if (!ESPConfig::showBones || !ESPConfig::enabled) return;

    SDK::UWorld* World = SDK::UWorld::GetWorld();
    if (!World || !World->OwningGameInstance) return;
    if (World->OwningGameInstance->LocalPlayers.Num() <= 0) return;

    SDK::APlayerController* localPlayer = World->OwningGameInstance->LocalPlayers[0]->PlayerController;
    if (!localPlayer || !localPlayer->Pawn) return;

    SDK::APawn* localPawn = localPlayer->Pawn;

    SDK::FVector viewPoint;
    SDK::FRotator viewRotation;
    localPawn->GetActorEyesViewPoint(&viewPoint, &viewRotation);

    if (!World->PersistentLevel) return;

    SDK::AFortTeamInfo* localTeam = nullptr;
    if (localPawn && localPawn->PlayerState && localPawn->PlayerState->IsA(SDK::AFortPlayerState::StaticClass())) {
        SDK::AFortPlayerState* localPS = static_cast<SDK::AFortPlayerState*>(localPawn->PlayerState);
        localTeam = localPS->PlayerTeam;
    }

    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    if (!drawList) return;

    HWND gameWindow = GetForegroundWindow();
    if (!gameWindow) return;

    RECT gameRect;
    if (!GetClientRect(gameWindow, &gameRect)) return;

    std::vector<std::pair<int, int>> newBoneLinks = {

        {Bones::Head, Bones::Neck_02},
        {Bones::Neck_02, Bones::Neck_01},

        {Bones::Neck_01, Bones::Spine_05},

        {Bones::Spine_05, Bones::Spine_04},

        {Bones::Spine_04, Bones::Spine_03},

        {Bones::Spine_03, Bones::Spine_02},

        {Bones::Spine_02, Bones::Spine_01},

        {Bones::Spine_01, Bones::Pelvis},

        {Bones::Spine_05, Bones::Clavicle_l},

        {Bones::Spine_05, Bones::Clavicle_r},

        {Bones::Clavicle_l, Bones::Upperarm_l},
        {Bones::Upperarm_l, Bones::Lowerarm_l},
        {Bones::Lowerarm_l, Bones::Hand_l},

        {Bones::Clavicle_r, Bones::Upperarm_r},
        {Bones::Upperarm_r, Bones::Lowerarm_r},
        {Bones::Lowerarm_r, Bones::Hand_r},

        {Bones::Pelvis, Bones::Thigh_l},

        {Bones::Pelvis, Bones::Thigh_r},

        {Bones::Thigh_l, Bones::Calf_l},
        {Bones::Calf_l, Bones::Foot_l},
        {Bones::Foot_l, Bones::Ball_l},

        {Bones::Thigh_r, Bones::Calf_r},
        {Bones::Calf_r, Bones::Foot_r},
        {Bones::Foot_r, Bones::Ball_r},

        {73, 2},

        {80, 2},

    };

    for (auto* Actor : World->PersistentLevel->Actors) {
        if (!Actor) continue;
        if (!Actor->IsA(SDK::AFortPlayerPawn::StaticClass())) continue;

        SDK::AFortPlayerPawn* targetPawn = static_cast<SDK::AFortPlayerPawn*>(Actor);
        if (targetPawn == localPawn) continue;

        if (!targetPawn->PlayerState || !targetPawn->PlayerState->IsA(SDK::AFortPlayerState::StaticClass())) continue;

        SDK::AFortPlayerState* targetPlayerState = static_cast<SDK::AFortPlayerState*>(targetPawn->PlayerState);
        if (!targetPlayerState) continue;

        if (localTeam && targetPlayerState->PlayerTeam == localTeam) continue;
        if (targetPawn->IsDead()) continue;

        SDK::FVector targetPos = targetPawn->K2_GetActorLocation();
        if (targetPos.X == 0.0f && targetPos.Y == 0.0f && targetPos.Z == 0.0f) continue;

        if (IsBehindCamera(localPlayer, targetPos))
            continue;

        SDK::FVector directionToTarget = targetPos - viewPoint;
        float distanceSq = directionToTarget.X * directionToTarget.X +
            directionToTarget.Y * directionToTarget.Y +
            directionToTarget.Z * directionToTarget.Z;
        float maxDistSq = ESPConfig::maxDistance * ESPConfig::maxDistance;
        maxDistSq *= 1000.0f;
        if (distanceSq > maxDistSq) continue;
        if (distanceSq < 1.0f) continue;

        SDK::ACharacter* character = static_cast<SDK::ACharacter*>(targetPawn);
        if (!character) continue;

        SDK::USkeletalMeshComponent* skeletalMesh = character->Mesh;
        if (!skeletalMesh) continue;

        for (const auto& boneLink : newBoneLinks) {
            int boneIndex1 = boneLink.first;
            int boneIndex2 = boneLink.second;

            SDK::FName boneName1 = skeletalMesh->GetBoneName(boneIndex1);
            SDK::FName boneName2 = skeletalMesh->GetBoneName(boneIndex2);

            if (boneName1.ComparisonIndex == 0 || boneName2.ComparisonIndex == 0) continue;

            SDK::FVector boneWorldPos1 = skeletalMesh->GetSocketLocation(boneName1);
            SDK::FVector boneWorldPos2 = skeletalMesh->GetSocketLocation(boneName2);

            if ((boneWorldPos1.X == 0.0f && boneWorldPos1.Y == 0.0f && boneWorldPos1.Z == 0.0f) ||
                (boneWorldPos2.X == 0.0f && boneWorldPos2.Y == 0.0f && boneWorldPos2.Z == 0.0f))
                continue;

            SDK::FVector2D boneScreenPos1;
            SDK::FVector2D boneScreenPos2;

            if (!SafeWorldToScreen(localPlayer, boneWorldPos1, boneScreenPos1) ||
                !SafeWorldToScreen(localPlayer, boneWorldPos2, boneScreenPos2))
                continue;

            POINT point1 = { (LONG)boneScreenPos1.X, (LONG)boneScreenPos1.Y };
            POINT point2 = { (LONG)boneScreenPos2.X, (LONG)boneScreenPos2.Y };

            ClientToScreen(gameWindow, &point1);
            ClientToScreen(gameWindow, &point2);

            ImVec2 screenPos1((float)point1.x, (float)point1.y);
            ImVec2 screenPos2((float)point2.x, (float)point2.y);

            bool isVisible = true;
            if (ESPConfig::boneVisibilityCheck) {
                SDK::FVector dirToBone1 = boneWorldPos1 - viewPoint;
                SDK::FVector dirToBone2 = boneWorldPos2 - viewPoint;

                float distToBone1 = sqrt(dirToBone1.X * dirToBone1.X + dirToBone1.Y * dirToBone1.Y + dirToBone1.Z * dirToBone1.Z);
                float distToBone2 = sqrt(dirToBone2.X * dirToBone2.X + dirToBone2.Y * dirToBone2.Y + dirToBone2.Z * dirToBone2.Z);

                isVisible = (distToBone1 < 2000.0f && distToBone2 < 2000.0f);
            }

            ImU32 color;
            if (isVisible) {
                color = IM_COL32(
                    ESPConfig::boneColorR,
                    ESPConfig::boneColorG,
                    ESPConfig::boneColorB,
                    255
                );
            }
            else {
                color = IM_COL32(
                    ESPConfig::boneColorHiddenR,
                    ESPConfig::boneColorHiddenG,
                    ESPConfig::boneColorHiddenB,
                    255
                );
            }

            drawList->AddLine(screenPos1, screenPos2, color, ESPConfig::boneThickness);
        }
    }
}

void UpdateESP()
{
    if (ESPConfig::enabled)
    {
        DrawESP();
    }

    if (ESPConfig::showBones)
    {
        DrawBoneESP();
    }

    DrawWatermark();
}