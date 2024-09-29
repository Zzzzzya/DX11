#include "GameApp.h"
#include "DXTrace.h"
#include "d3dUtil.h"

GameApp::GameApp(HINSTANCE hInstance, const std::wstring &windowName, int initWidth, int initHeight)
    : D3DApp(hInstance, windowName, initWidth, initHeight) {
}

GameApp::~GameApp() {
}

bool GameApp::Init() {
    if (!D3DApp::Init())
        return false;

    return true;
}

void GameApp::OnResize() {
    D3DApp::OnResize();
}

void GameApp::UpdateScene(float dt) {
}

void GameApp::DrawScene() {
    assert(m_pd3dImmediateContext);
    assert(m_pSwapChain);
    static float ClearColor[4] = {1.0f, 0.0f, 0.0f, 1.0f};
    m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), ClearColor);
    m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                                  1.0f, 0);

    HR(m_pSwapChain->Present(0, 0));
}
