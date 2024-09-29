#ifndef GAMEAPP_H
#define GAMEAPP_H

#include "d3dApp.h"
class GameApp : public D3DApp
{
    /**
     * @brief 顶点输入布局
     *
     */
  public:
    struct VertexPosColor
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT4 color;
        static const D3D11_INPUT_ELEMENT_DESC inputLayout[2];
    };

  public:
    GameApp(HINSTANCE hInstance, const std::wstring &windowName, int initWidth, int initHeight);
    ~GameApp();

    bool Init();
    void OnResize();
    void UpdateScene(float dt);
    void DrawScene();

  private:
    /**
     * @brief 初始化着色器 && 特效
     *
     */
    bool InitEffect();

    /**
     * @brief 初始化初始顶点资源
     *
     *
     */
    bool InitResource();

    /**
     * @brief Shaders
     *
     */
  private:
    ComPtr<ID3D11VertexShader> m_pVertexShader;
    ComPtr<ID3D11PixelShader> m_pPixelShader;
    ComPtr<ID3D11InputLayout> m_pInputLayout;
    ComPtr<ID3D11Buffer> m_pVertexBuffer;
};

#endif