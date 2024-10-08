#ifndef GAMEAPP_H
#define GAMEAPP_H

#include "d3dApp.h"
#include "Geometry.h"
#include "LightHelper.h"
#include "Camera.h"
#include <memory>

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
    struct VSConstantBufferEveryDrawing
    {
        XMMATRIX world;
        XMMATRIX worldInvTranspose;
        Material material;
    };

    struct VSConstantBufferEveryFrame
    {
        XMMATRIX view;
        XMFLOAT4 eyePos;
    };

    struct VSConstantBufferOnResize
    {
        XMMATRIX projection;
    };
    struct PSConstantBuffer
    {
        DirectionalLight dirLight[10];
        PointLight pointLight[10];
        SpotLight spotLight[10];
        int NumDirLight;
        int NumPointLight;
        int NumSpotLight;
        float pad1;
    };

    // 一个尽可能小的游戏对象类
    class GameObject
    {
      public:
        GameObject();

        // 获取物体变换
        Transform &GetTransform();
        // 获取物体变换
        const Transform &GetTransform() const;

        // 设置缓冲区
        template <class VertexType, class IndexType>
        void SetBuffer(ID3D11Device *device, const Geometry::MeshData<VertexType, IndexType> &meshData);
        // 设置纹理
        void SetTexture(ID3D11ShaderResourceView *texture);

        // 绘制
        void Draw(ID3D11DeviceContext *deviceContext);

        // 设置调试对象名
        // 若缓冲区被重新设置，调试对象名也需要被重新设置
        void SetDebugObjectName(const std::string &name);

      private:
        Transform m_Transform;                       // 物体变换信息
        ComPtr<ID3D11ShaderResourceView> m_pTexture; // 纹理
        ComPtr<ID3D11Buffer> m_pVertexBuffer;        // 顶点缓冲区
        ComPtr<ID3D11Buffer> m_pIndexBuffer;         // 索引缓冲区
        Material m_Material;                         // 物体表面材质
        UINT m_VertexStride;                         // 顶点字节大小
        UINT m_IndexCount;                           // 索引数目
    };

  public:
    GameApp(HINSTANCE hInstance, const std::wstring &windowName, int initWidth, int initHeight);
    ~GameApp();

    bool Init();
    void OnResize();
    void UpdateScene(float dt);
    void DrawScene();

    template <typename VertexType> bool ResetMesh(const Geometry::MeshData<VertexType> &meshData);

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
    UINT m_IndexCount;
    bool m_bNeedUpdatePS = false;

    ComPtr<ID3D11InputLayout> m_pInputLayout;

    ComPtr<ID3D11VertexShader> m_pVertexShader;
    ComPtr<ID3D11PixelShader> m_pPixelShader;

    ComPtr<ID3D11Buffer> m_pVSConstantBufferEveryDrawing;
    ComPtr<ID3D11Buffer> m_pVSConstantBufferEveryFrame;
    ComPtr<ID3D11Buffer> m_pVSConstantBufferOnResize;
    ComPtr<ID3D11Buffer> m_pPSConstantBuffer;
    VSConstantBufferEveryDrawing m_VSConstantBufferEveryDrawing;
    VSConstantBufferEveryFrame m_VSConstantBufferEveryFrame;
    VSConstantBufferOnResize m_VSConstantBufferOnResize;
    PSConstantBuffer m_PSConstantBuffer;

    ComPtr<ID3D11RasterizerState> m_pRSWireframe;

    ComPtr<ID3D11SamplerState> m_pSamplerState;
    ComPtr<ID3D11ShaderResourceView> m_pTexture;

    DirectionalLight m_DirLight;
    PointLight m_PointLight;
    SpotLight m_SpotLight;
    Material m_Material;

    int m_ModelType = 0;
    int m_LightType = 0;

    std::shared_ptr<Camera> m_pCamera;
    enum class CameraMode
    {
        FirstPerson,
        ThirdPerson,
        Free
    };
    CameraMode m_CameraMode = CameraMode::FirstPerson;

    std::vector<GameObject> Objects;
};

#endif