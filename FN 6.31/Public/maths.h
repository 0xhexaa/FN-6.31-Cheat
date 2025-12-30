#pragma once
#include "../Game/SDK/Engine_classes.hpp"

namespace Math {
    constexpr float PI = 3.14159265358979323846f;
    constexpr float DEG_TO_RAD = PI / 180.0f;
    constexpr float RAD_TO_DEG = 180.0f / PI;

    void SinCos(float* Sine, float* Cosine, float AngleRad);
    float InvSqrt(float F);
    float DegreesToRadians(float Degrees);
    float RadiansToDegrees(float Radians);

    SDK::FVector Normalize(const SDK::FVector& v);
    float Dot(const SDK::FVector& a, const SDK::FVector& b);
    SDK::FVector Cross(const SDK::FVector& a, const SDK::FVector& b);
    float Distance(const SDK::FVector& a, const SDK::FVector& b);
    float Length(const SDK::FVector& v);
    SDK::FVector Lerp(const SDK::FVector& a, const SDK::FVector& b, float t);

    SDK::FQuat RotatorToQuat(const SDK::FRotator& Rotator);
    SDK::FRotator FindLookAtRotation(const SDK::FVector& Start, const SDK::FVector& Target);
    SDK::FRotator LookAt(const SDK::FVector& Start, const SDK::FVector& Target);

    SDK::FVector GetForwardVector(float Pitch, float Yaw);
    SDK::FVector GetForwardVector(const SDK::FRotator& InRot);
    SDK::FVector GetRightVector(float Pitch, float Yaw);
    SDK::FVector GetRightVector(const SDK::FRotator& Rot);
    SDK::FVector GetUpVector(float Pitch, float Yaw);
    SDK::FVector GetUpVector(const SDK::FRotator& Rot);

    float GetDegreeDistance(const SDK::FRotator& Rotator1, const SDK::FRotator& Rotator2);
    bool IsOnScreen(const SDK::FVector2D& Position, int ScreenWidth, int ScreenHeight);
    float Clamp(float value, float min, float max);
    float Lerp(float a, float b, float t);
}