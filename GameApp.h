#ifndef GAMEAPP_H
#define GAMEAPP_H

#include "d3dApp.h"
#include "Geometry.h"
#include "LightHelper.h"

using DirectX::XMFLOAT3;
using DirectX::XMFLOAT4;
using DirectX::XMMATRIX;

class GameApp : public D3DApp
{
    /**
     * @brief 常量缓冲区
     *
     */
  public:
    struct VSConstantBuffer
    {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX projection;
        DirectX::XMMATRIX worldInvTranspose;
    };

    struct PSConstantBuffer
    {
        DirectionalLight dirLight;
        PointLight pointLight;
        SpotLight spotLight;
        Material material;
        DirectX::XMFLOAT4 eyePos;
    };

  public:
    GameApp(HINSTANCE hInstance, const std::wstring &windowName, int initWidth, int initHeight);
    ~GameApp();

    bool Init();
    void OnResize();
    void UpdateScene(float dt);
    void DrawScene();
    bool ResetMesh(const Geometry::MeshData<VertexPosNormalColor> &meshData);

  private:
    void ImGUISet();
    void UpdateCamera(float);
    void UpdateMVP(float);

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
    ComPtr<ID3D11Buffer> m_pVertexBuffer;
    ComPtr<ID3D11Buffer> m_pIndexBuffer;
    ComPtr<ID3D11Buffer> m_pVSConstantBuffer;
    ComPtr<ID3D11Buffer> m_pPSConstantBuffer;
    ComPtr<ID3D11InputLayout> m_pInputLayout;
    UINT m_IndexCount;

    ComPtr<ID3D11VertexShader> m_pVertexShader;
    ComPtr<ID3D11PixelShader> m_pPixelShader;
    VSConstantBuffer m_VSConstantBuffer;
    PSConstantBuffer m_PSConstantBuffer;

    ComPtr<ID3D11RasterizerState> m_pRSWireframe;

    DirectionalLight m_DirLight;
    PointLight m_PointLight;
    SpotLight m_SpotLight;
};

#endif