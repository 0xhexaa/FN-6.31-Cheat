#pragma once
#include <d3d11.h>
#include <dxgi.h>

namespace gui {
    constexpr int WIDTH = 600;
    constexpr int HEIGHT = 450;
    inline bool is_running = true;
    inline bool open = true;
    inline bool initDx = false;
    inline HWND window = nullptr;
    inline WNDPROC oWndProc = nullptr;
    inline ID3D11Device* pDevice = nullptr;
    inline ID3D11DeviceContext* pContext = nullptr;
    inline ID3D11RenderTargetView* mainRenderTargetView = nullptr;
    inline HRESULT(__stdcall* oPresent)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) = nullptr;

    void SetupImGuiStyle() noexcept;
    void CreateImGui() noexcept;
    void DestroyImGui() noexcept;

    void RenderAimbotTab();
    void RenderESPTab();
    void RenderExploitsTab();
    void RenderUEConsoleTab();
    void Render() noexcept;

    HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
}