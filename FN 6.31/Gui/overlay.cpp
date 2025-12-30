#include "overlay.h"
#include "Configuration.h"
#include <Windows.h>
#include <cmath>

void DrawFOVCircle() {
    if (!AimConfig::drawFov || !AimConfig::aimbot) return;

    HWND gameWindow = GetForegroundWindow();
    if (!gameWindow) return;

    RECT gameRect;
    if (!GetClientRect(gameWindow, &gameRect)) return;
    HDC hdc = GetDC(gameWindow);
    if (!hdc) return;
    int centerX = (gameRect.right - gameRect.left) / 2;
    int centerY = (gameRect.bottom - gameRect.top) / 2;
    float fovRadians = AimConfig::fov * (3.14159f / 180.0f) / 2.0f;
    float fovRadius = centerY * tan(fovRadians) * AimConfig::fovCircleScale;
    HPEN whitePen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
    HPEN redPen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
    HPEN oldPen = (HPEN)SelectObject(hdc, whitePen);
    Ellipse(hdc,
        centerX - (int)fovRadius,
        centerY - (int)fovRadius,
        centerX + (int)fovRadius,
        centerY + (int)fovRadius);

    SelectObject(hdc, redPen);
    MoveToEx(hdc, centerX - 15, centerY, NULL);
    LineTo(hdc, centerX - 5, centerY);
    MoveToEx(hdc, centerX + 5, centerY, NULL);
    LineTo(hdc, centerX + 15, centerY);

    MoveToEx(hdc, centerX, centerY - 15, NULL);
    LineTo(hdc, centerX, centerY - 5);
    MoveToEx(hdc, centerX, centerY + 5, NULL);
    LineTo(hdc, centerX, centerY + 15);
    SelectObject(hdc, oldPen);
    DeleteObject(whitePen);
    DeleteObject(redPen);
    ReleaseDC(gameWindow, hdc);
}