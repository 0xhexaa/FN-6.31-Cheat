#include "aimbot.h"
#include "../Game/SDK/Engine_classes.hpp"
#include "../Game/SDK/Basic.hpp"
#include "../Game/SDK/FortniteGame_classes.hpp"
#include <algorithm>
#include <vector>
#include <cmath>
#include <Windows.h>
#include <cfloat>
#include "Configuration.h"
#include "maths.h"
#include "Bones.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_dx11.h"
#include "../imgui/imgui_impl_win32.h"

namespace Aimbot {

    std::wstring Utf8ToWide(const std::string& utf8_str) {
        if (utf8_str.empty()) {
            return std::wstring();
        }
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), static_cast<int>(utf8_str.length()), NULL, 0);
        if (size_needed == 0) {
            return std::wstring();
        }
        std::wstring wstr(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), static_cast<int>(utf8_str.length()), &wstr[0], size_needed);
        return wstr;
    }

    void DrawFOVCircle() noexcept {
        if (!AimConfig::drawFov || !AimConfig::aimbot) return;
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        if (!drawList) return;
        HWND gameWindow = GetForegroundWindow();
        if (!gameWindow) return;

        RECT gameRect;
        if (!GetClientRect(gameWindow, &gameRect)) return;
        float centerX = (gameRect.right - gameRect.left) / 2.0f;
        float centerY = (gameRect.bottom - gameRect.top) / 2.0f;

        float gameFOV = 90.0f;
        SDK::UWorld* World = SDK::UWorld::GetWorld();
        if (World && World->OwningGameInstance && World->OwningGameInstance->LocalPlayers.Num() > 0) {
            SDK::APlayerController* PC = World->OwningGameInstance->LocalPlayers[0]->PlayerController;
            if (PC && PC->PlayerCameraManager) gameFOV = PC->PlayerCameraManager->GetFOVAngle();
        }

        float aimbotFOVRadians = (AimConfig::fov * 0.5f) * (3.14159f / 180.0f);
        float gameFOVRadians = (gameFOV * 0.5f) * (3.14159f / 180.0f);
        float fovRadius = (centerY * tan(aimbotFOVRadians) / tan(gameFOVRadians)) * AimConfig::fovCircleScale;

        POINT clientPoint = { (LONG)centerX, (LONG)centerY };
        ClientToScreen(gameWindow, &clientPoint);
        float screenX = (float)clientPoint.x;
        float screenY = (float)clientPoint.y;

        drawList->AddCircle(ImVec2(screenX, screenY), fovRadius, IM_COL32(0, 0, 0, 255), 64, 3.0f);
        drawList->AddCircle(ImVec2(screenX, screenY), fovRadius, IM_COL32(255, 255, 255, 255), 64, 1.5f);

        if (AimConfig::customCrosshair) {
            drawList->AddLine(ImVec2(screenX - 15, screenY), ImVec2(screenX - 5, screenY), IM_COL32(0, 0, 0, 255), 3.0f);
            drawList->AddLine(ImVec2(screenX - 15, screenY), ImVec2(screenX - 5, screenY), IM_COL32(255, 255, 255, 255), 1.5f);
            drawList->AddLine(ImVec2(screenX + 5, screenY), ImVec2(screenX + 15, screenY), IM_COL32(0, 0, 0, 255), 3.0f);
            drawList->AddLine(ImVec2(screenX + 5, screenY), ImVec2(screenX + 15, screenY), IM_COL32(255, 255, 255, 255), 1.5f);
            drawList->AddLine(ImVec2(screenX, screenY - 15), ImVec2(screenX, screenY - 5), IM_COL32(0, 0, 0, 255), 3.0f);
            drawList->AddLine(ImVec2(screenX, screenY - 15), ImVec2(screenX, screenY - 5), IM_COL32(255, 255, 255, 255), 1.5f);
            drawList->AddLine(ImVec2(screenX, screenY + 5), ImVec2(screenX, screenY + 15), IM_COL32(0, 0, 0, 255), 3.0f);
            drawList->AddLine(ImVec2(screenX, screenY + 5), ImVec2(screenX, screenY + 15), IM_COL32(255, 255, 255, 255), 1.5f);
        }
    }

    static bool WorldToScreen(SDK::APlayerController* Controller, const SDK::FVector& WorldPos, SDK::FVector2D& ScreenPos) {
        if (!Controller) return false;
        bool bW2S = Controller->ProjectWorldLocationToScreen(WorldPos, &ScreenPos, false);
        return bW2S;
    }

    SDK::FVector GetBoneLocation(SDK::AFortPlayerPawn* player, const wchar_t* boneName) {
        if (!player || !player->Mesh) {
            return SDK::FVector(0, 0, 0);
        }

        try {
            SDK::FName boneFName = SDK::UKismetStringLibrary::Conv_StringToName(boneName);
            SDK::FTransform boneTransform = player->Mesh->GetSocketTransform(boneFName, SDK::ERelativeTransformSpace::RTS_World);
            return boneTransform.Translation;
        }
        catch (...) {
            return SDK::FVector(0, 0, 0);
        }
    }

    SDK::FVector GetBoneLocationByIndex(SDK::AFortPlayerPawn* player, int32_t boneIndex) {
        if (!player || !player->Mesh) return SDK::FVector(0, 0, 0);
        SDK::FName Bone = player->Mesh->GetBoneName(boneIndex);
        return GetBoneLocation(player, Utf8ToWide(Bone.ToString()).c_str());
    }

    static bool IsValidTarget(SDK::AFortPawn* targetPawn, SDK::AFortPawn* localPawn, SDK::APlayerController* Controller, const SDK::FVector& cameraLoc) {
        if (!targetPawn || !localPawn || !Controller || targetPawn == localPawn) return false;

        if (targetPawn->IsDead() || targetPawn->IsDBNO()) return false;

        SDK::AFortPlayerState* localPs = static_cast<SDK::AFortPlayerState*>(localPawn->PlayerState);
        SDK::AFortPlayerState* targetPs = static_cast<SDK::AFortPlayerState*>(targetPawn->PlayerState);

        if (!localPs || !targetPs) return false;

        if (localPs->PlayerTeam && targetPs->PlayerTeam) {
            if (localPs->PlayerTeam == targetPs->PlayerTeam) return false;
        }

        SDK::FVector2D screenPos;
        SDK::FVector targetHead = GetBoneLocationByIndex((SDK::AFortPlayerPawn*)targetPawn, Bones::Head);
        if (!WorldToScreen(Controller, targetHead, screenPos)) return false;

        SDK::FVector targetPos = targetPawn->K2_GetActorLocation();
        SDK::FVector directionToTarget = targetPos - cameraLoc;
        float distanceSq = directionToTarget.X * directionToTarget.X + directionToTarget.Y * directionToTarget.Y + directionToTarget.Z * directionToTarget.Z;

        if (distanceSq < 1.0f) return false;

        return true;
    }

    static float Square(float value) {
        return value * value;
    }

    static float VectorLength(const SDK::FVector& Vec) {
        return sqrtf(Vec.X * Vec.X + Vec.Y * Vec.Y + Vec.Z * Vec.Z);
    }

    static float CalculateDistance(const SDK::FVector& A, const SDK::FVector& B) {
        return VectorLength(A - B);
    }

    static float CalculateScreenDistance(const SDK::FVector2D& A, const SDK::FVector2D& B) {
        float X = A.X - B.X;
        float Y = A.Y - B.Y;
        return sqrtf(Square(X) + Square(Y));
    }

    static SDK::FVector FRotatorToVector(const SDK::FRotator& rot) {
        float radPitch = rot.Pitch * (3.14159f / 180.0f);
        float radYaw = rot.Yaw * (3.14159f / 180.0f);
        return SDK::FVector(cosf(radPitch) * cosf(radYaw), cosf(radPitch) * sinf(radYaw), sinf(radPitch));
    }

    static float CalculateFOV(const SDK::FRotator& viewAngle, const SDK::FRotator& targetAngle) {
        SDK::FVector viewVec = FRotatorToVector(viewAngle);
        SDK::FVector targetVec = FRotatorToVector(targetAngle);
        float dot = viewVec.X * targetVec.X + viewVec.Y * targetVec.Y + viewVec.Z * targetVec.Z;
        dot = std::clamp(dot, -1.0f, 1.0f);
        return acosf(dot) * (180.0f / 3.14159f);
    }

    static SDK::FRotator CalculateAngle(const SDK::FVector& source, const SDK::FVector& destination) {
        SDK::FVector delta = destination - source;
        SDK::FRotator angle;
        angle.Yaw = atan2f(delta.Y, delta.X) * (180.0f / 3.14159f);
        float horizontalDistance = sqrtf(delta.X * delta.X + delta.Y * delta.Y);
        angle.Pitch = -atan2f(delta.Z, horizontalDistance) * (180.0f / 3.14159f);
        return angle;
    }

    SDK::AFortPawn* getBestTarget(SDK::AFortPawn* localPawn, SDK::APlayerController* localController, float maxDistance) {
        SDK::AFortPawn* bestTarget = nullptr;
        float bestScore = FLT_MAX;

        SDK::UWorld* world = SDK::UWorld::GetWorld();
        if (!world || !world->PersistentLevel) return nullptr;

        SDK::FVector cameraLoc;
        SDK::FRotator cameraRot;
        localController->GetActorEyesViewPoint(&cameraLoc, &cameraRot);

        auto Actors = world->PersistentLevel->Actors;
        float maxDistSq = maxDistance * maxDistance * 1000.0f;

        for (int i = 0; i < Actors.Num(); i++) {
            auto actor = Actors[i];
            if (!actor) continue;

            if (!actor->IsA(SDK::AFortPawn::StaticClass())) continue;

            SDK::AFortPawn* targetPawn = static_cast<SDK::AFortPawn*>(actor);
            if (!targetPawn) continue;

            if (!IsValidTarget(targetPawn, localPawn, localController, cameraLoc)) continue;

            SDK::FVector targetPos = targetPawn->K2_GetActorLocation();
            SDK::FVector directionToTarget = targetPos - cameraLoc;
            float distanceSq = directionToTarget.X * directionToTarget.X + directionToTarget.Y * directionToTarget.Y + directionToTarget.Z * directionToTarget.Z;

            if (distanceSq > maxDistSq) continue;

            SDK::FVector targetHead = GetBoneLocationByIndex((SDK::AFortPlayerPawn*)targetPawn, Bones::Head);
            SDK::FRotator angleToTarget = CalculateAngle(cameraLoc, targetHead);
            float fovDiff = CalculateFOV(cameraRot, angleToTarget);

            if (fovDiff > (AimConfig::fov / 2.0f)) continue;

            float distance = sqrtf(distanceSq);
            float score = (distance / 1000.0f) + (fovDiff * 2.0f);

            if (score < bestScore) {
                bestScore = score;
                bestTarget = targetPawn;
            }
        }

        return bestTarget;
    }

    static SDK::FRotator SmoothAngle(const SDK::FRotator& currentAngle, const SDK::FRotator& targetAngle, float smoothness) {
        if (smoothness <= 1.0f) return targetAngle;

        SDK::FRotator smoothedAngle;
        SDK::FRotator delta = targetAngle - currentAngle;
        delta.Normalize();
        smoothedAngle.Pitch = currentAngle.Pitch + delta.Pitch / smoothness;
        smoothedAngle.Yaw = currentAngle.Yaw + delta.Yaw / smoothness;
        smoothedAngle.Roll = 0;
        return smoothedAngle;
    }

    void Tick() {
        if (!AimConfig::aimbot) return;

        SDK::UWorld* world = SDK::UWorld::GetWorld();
        if (!world || !world->OwningGameInstance || world->OwningGameInstance->LocalPlayers.Num() <= 0) return;

        SDK::APlayerController* localController = world->OwningGameInstance->LocalPlayers[0]->PlayerController;
        if (!localController) return;

        SDK::AFortPawn* localPawn = reinterpret_cast<SDK::AFortPawn*>(localController->Pawn);
        if (!localPawn) return;

        bool shouldAim = (GetAsyncKeyState(AimConfig::aimKey) & 0x8000) != 0;
        if (!shouldAim) return;

        SDK::AFortPawn* target = getBestTarget(localPawn, localController, AimConfig::maxDistance);
        if (!target) return;

        SDK::FVector cameraLoc;
        SDK::FRotator cameraRot;
        localController->GetActorEyesViewPoint(&cameraLoc, &cameraRot);

        SDK::FVector targetBone = GetBoneLocationByIndex((SDK::AFortPlayerPawn*)target, Bones::Head);

        SDK::FRotator targetAngle = CalculateAngle(cameraLoc, targetBone);
        SDK::FRotator finalAngle = targetAngle;
        if (AimConfig::smoothAim && AimConfig::smoothness > 1.0f) {
            finalAngle = SmoothAngle(cameraRot, targetAngle, AimConfig::smoothness);
        }

        localController->SetControlRotation(finalAngle);
    }
}