#include "maths.h"
#include <cmath>
#include <immintrin.h>

namespace Math {
    void SinCos(float* Sine, float* Cosine, float AngleRad) {
        if (Sine)
            *Sine = sinf(AngleRad);
        if (Cosine)
            *Cosine = cosf(AngleRad);
    }

    float InvSqrt(float F) {
        __m128 val = _mm_set_ss(F);
        __m128 estimate = _mm_rsqrt_ss(val);
        __m128 muls = _mm_mul_ss(_mm_mul_ss(val, estimate), estimate);
        estimate = _mm_mul_ss(_mm_mul_ss(_mm_set_ss(0.5f), estimate), _mm_sub_ss(_mm_set_ss(3.0f), muls));
        float result;
        _mm_store_ss(&result, estimate);
        return result;
    }

    float DegreesToRadians(float Degrees) {
        return Degrees * DEG_TO_RAD;
    }

    float RadiansToDegrees(float Radians) {
        return Radians * RAD_TO_DEG;
    }

    SDK::FQuat RotatorToQuat(const SDK::FRotator& Rotator) {
        float PitchNoWinding = fmodf(Rotator.Pitch, 360.0f);
        float YawNoWinding = fmodf(Rotator.Yaw, 360.0f);
        float RollNoWinding = fmodf(Rotator.Roll, 360.0f);
        float SP, CP, SY, CY, SR, CR;
        SinCos(&SP, &CP, PitchNoWinding * DEG_TO_RAD * 0.5f);
        SinCos(&SY, &CY, YawNoWinding * DEG_TO_RAD * 0.5f);
        SinCos(&SR, &CR, RollNoWinding * DEG_TO_RAD * 0.5f);
        SDK::FQuat RotationQuat;
        RotationQuat.X = CR * SP * SY - SR * CP * CY;
        RotationQuat.Y = -CR * SP * CY - SR * CP * SY;
        RotationQuat.Z = CR * CP * SY - SR * SP * CY;
        RotationQuat.W = CR * CP * CY + SR * SP * SY;
        return RotationQuat;
    }

    SDK::FRotator FindLookAtRotation(const SDK::FVector& Start, const SDK::FVector& Target) {
        SDK::FVector Direction = Target - Start;
        Direction = Normalize(Direction);
        float Yaw = atan2f(Direction.Y, Direction.X) * RAD_TO_DEG;
        float HorizontalDistance = sqrtf(Direction.X * Direction.X + Direction.Y * Direction.Y);
        float Pitch = atan2f(Direction.Z, HorizontalDistance) * RAD_TO_DEG;
        return SDK::FRotator(Pitch, Yaw, 0.0f);
    }

    SDK::FVector GetForwardVector(const SDK::FRotator& InRot) {
        float PitchNoWinding = fmodf(InRot.Pitch, 360.0f);
        float YawNoWinding = fmodf(InRot.Yaw, 360.0f);
        float SP, CP, SY, CY;
        SinCos(&SP, &CP, DegreesToRadians(PitchNoWinding));
        SinCos(&SY, &CY, DegreesToRadians(YawNoWinding));
        return SDK::FVector(CP * CY, CP * SY, SP);
    }

    float GetDegreeDistance(const SDK::FRotator& Rotator1, const SDK::FRotator& Rotator2) {
        SDK::FVector Forward1 = GetForwardVector(Rotator1);
        SDK::FVector Forward2 = GetForwardVector(Rotator2);
        Forward1 = Normalize(Forward1);
        Forward2 = Normalize(Forward2);
        float DotValue = Dot(Forward1, Forward2);
        if (DotValue < -1.0f) DotValue = -1.0f;
        if (DotValue > 1.0f) DotValue = 1.0f;
        return RadiansToDegrees(acosf(DotValue));
    }

    bool IsOnScreen(const SDK::FVector2D& Position, int ScreenWidth, int ScreenHeight) {
        return Position.X >= 0 && Position.Y >= 0 && Position.X <= ScreenWidth && Position.Y <= ScreenHeight;
    }

    SDK::FVector Normalize(const SDK::FVector& v) {
        float length = sqrtf(v.X * v.X + v.Y * v.Y + v.Z * v.Z);
        if (length > 0.00001f) {
            float invLength = 1.0f / length;
            return SDK::FVector(v.X * invLength, v.Y * invLength, v.Z * invLength);
        }
        return SDK::FVector(0, 0, 0);
    }

    float Dot(const SDK::FVector& a, const SDK::FVector& b) {
        return a.X * b.X + a.Y * b.Y + a.Z * b.Z;
    }

    float Distance(const SDK::FVector& a, const SDK::FVector& b) {
        float dx = a.X - b.X;
        float dy = a.Y - b.Y;
        float dz = a.Z - b.Z;
        return sqrtf(dx * dx + dy * dy + dz * dz);
    }

    float Length(const SDK::FVector& v) {
        return sqrtf(v.X * v.X + v.Y * v.Y + v.Z * v.Z);
    }

    SDK::FVector GetForwardVector(float Pitch, float Yaw) {
        float pitchRad = DegreesToRadians(Pitch);
        float yawRad = DegreesToRadians(Yaw);
        float sp, cp, sy, cy;
        SinCos(&sp, &cp, pitchRad);
        SinCos(&sy, &cy, yawRad);
        return SDK::FVector(cp * cy, cp * sy, sp);
    }

    SDK::FVector GetRightVector(float Pitch, float Yaw) {
        float yawRad = DegreesToRadians(Yaw);
        float sy, cy;
        SinCos(&sy, &cy, yawRad);
        return SDK::FVector(-sy, cy, 0);
    }

    SDK::FVector GetRightVector(const SDK::FRotator& Rot) {
        return GetRightVector(Rot.Pitch, Rot.Yaw);
    }

    SDK::FVector GetUpVector(float Pitch, float Yaw) {
        float pitchRad = DegreesToRadians(Pitch);
        float yawRad = DegreesToRadians(Yaw);
        float sp, cp, sy, cy;
        SinCos(&sp, &cp, pitchRad);
        SinCos(&sy, &cy, yawRad);
        return SDK::FVector(sp * cy, sp * sy, -cp);
    }

    SDK::FVector GetUpVector(const SDK::FRotator& Rot) {
        return GetUpVector(Rot.Pitch, Rot.Yaw);
    }

    SDK::FRotator LookAt(const SDK::FVector& Start, const SDK::FVector& Target) {
        SDK::FVector direction = Target - Start;
        direction = Normalize(direction);
        float pitch = -asinf(direction.Z) * RAD_TO_DEG;
        float yaw = atan2f(direction.Y, direction.X) * RAD_TO_DEG;
        return SDK::FRotator(pitch, yaw, 0);
    }

    float Clamp(float value, float min, float max) {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }

    float Lerp(float a, float b, float t) {
        return a + t * (b - a);
    }

    SDK::FVector Lerp(const SDK::FVector& a, const SDK::FVector& b, float t) {
        return SDK::FVector(Lerp(a.X, b.X, t), Lerp(a.Y, b.Y, t), Lerp(a.Z, b.Z, t));
    }

    SDK::FVector Cross(const SDK::FVector& a, const SDK::FVector& b) {
        return SDK::FVector(a.Y * b.Z - a.Z * b.Y, a.Z * b.X - a.X * b.Z, a.X * b.Y - a.Y * b.X);
    }
}