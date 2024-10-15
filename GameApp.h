#ifndef GAMEAPP_H
#define GAMEAPP_H

#include "d3dApp.h"
#include "Geometry.h"
#include "LightHelper.h"
#include "Camera.h"
#include "RenderStates.h"
#include "Texture2D.h"
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

    struct LightBuffer
    {
        XMMATRIX view;
        XMMATRIX pro;
    };
    struct PSConstantBuffer
    {
        XMMATRIX reflection;
        DirectionalLight dirLight[10];
        PointLight pointLight[10];
        SpotLight spotLight[10];
        int NumDirLight;
        int NumPointLight;
        int NumSpotLight;
        float pad1;
        XMMATRIX ShadowMatrix;
    };
    struct CBDrawingStates
    {
        int isReflection;
        float pad[3];
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

        Material &GetMaterial();

        const Material &GetMaterial() const;

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
    ComPtr<ID3D11InputLayout> m_pScreenInputLayout;
    ComPtr<ID3D11InputLayout> m_pShadowInputLayout;

    ComPtr<ID3D11VertexShader> m_pVertexShader;
    ComPtr<ID3D11PixelShader> m_pPixelShader;

    ComPtr<ID3D11VertexShader> m_pScreenVS;
    ComPtr<ID3D11PixelShader> m_pScreenPS_None;

    ComPtr<ID3D11VertexShader> m_pShadowVS;
    ComPtr<ID3D11PixelShader> m_pShadowPS;

    ComPtr<ID3D11Buffer> m_pVSConstantBufferEveryDrawing;
    ComPtr<ID3D11Buffer> m_pVSConstantBufferEveryFrame;
    ComPtr<ID3D11Buffer> m_pVSConstantBufferOnResize;
    ComPtr<ID3D11Buffer> m_pPSConstantBuffer;
    ComPtr<ID3D11Buffer> m_pCBDrawingStates;

    VSConstantBufferEveryDrawing m_VSConstantBufferEveryDrawing;
    VSConstantBufferEveryFrame m_VSConstantBufferEveryFrame;
    VSConstantBufferOnResize m_VSConstantBufferOnResize;
    PSConstantBuffer m_PSConstantBuffer;
    CBDrawingStates m_CBDrawingStates;

    ComPtr<ID3D11Buffer> m_pLightBuffer;
    LightBuffer m_LightBuffers[4];

    // ComPtr<ID3D11Buffer> m_pShadowConstantBufferView;
    // ComPtr<ID3D11Buffer> m_pShadowConstantBufferPro;
    // VSConstantBufferEveryFrame m_ShadowConstantBufferView;
    // VSConstantBufferOnResize m_ShadowConstantBufferPro;

    ComPtr<ID3D11RasterizerState> m_pRSWireframe;

    ComPtr<ID3D11SamplerState> m_pSamplerState;
    ComPtr<ID3D11ShaderResourceView> m_pTexture;
    ComPtr<ID3D11ShaderResourceView> m_pTexture_floor;
    ComPtr<ID3D11ShaderResourceView> m_pTexture_wireFence;
    ComPtr<ID3D11ShaderResourceView> m_pTexture_water;
    ComPtr<ID3D11ShaderResourceView> m_pTexture_brick;

    DirectionalLight m_DirLight;
    PointLight m_PointLight;
    SpotLight m_SpotLight;
    Material m_Material;

    int m_ModelType = 0;
    int m_LightType = 0;

    std::shared_ptr<Camera> m_pCamera;
    std::shared_ptr<Camera> m_pLightCamera;
    enum class CameraMode
    {
        FirstPerson,
        ThirdPerson,
        Free
    };
    CameraMode m_CameraMode = CameraMode::Free;

    std::vector<GameObject> Objects;
    std::vector<GameObject> TransparentObjects;
    std::vector<GameObject> PointLightsObjects;
    GameObject o_mirror;

    int nCSM = 4;
    float nearZ = 0.5f;
    float farZ = 200.0f;

    int maxCsmWidth = 8192;
    std::vector<float> csmWidths = {8192.0f, 4096.0f, 2048.0f, 1024.0f};

    std::vector<GameObject> Frustums;
    std::vector<GameObject> FrustumAABBs;
    std::shared_ptr<Camera> m_pShadowCamera;
    GameObject oFrustum;
    GameObject oFrustumAABB;
    bool bShowFrustum = false;
    bool bShowFrustumAABB = false;
    bool bShowScene = true;
    XMMATRIX LightView;
    XMMATRIX InvLightView;

    std::shared_ptr<Texture2D> m_pTexture_Screen;
    std::shared_ptr<Depth2D> m_pTexture_ScreenDepth;

    std::shared_ptr<Texture2D> m_pTexture_ShadowMap;
    std::shared_ptr<Depth2D> m_pTexture_ShadowMapDepth;

    std::vector<std::shared_ptr<Texture2D>> m_pShadowMaps;
    std::vector<std::shared_ptr<Depth2D>> m_pShadowMapDepths;
};

#endif