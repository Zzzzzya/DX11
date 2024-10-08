#include "GameApp.h"
#include "DXTrace.h"
#include "d3dUtil.h"
#include "DDSTextureLoader11.h"
#include "Camera.h"
using namespace DirectX;
using DirectX::XMMatrixTranspose;

static Material DefaultMaterial{XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
                                XMFLOAT4(0.5f, 0.5f, 0.5f, 25.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)};

GameApp::GameApp(HINSTANCE hInstance, const std::wstring &windowName, int initWidth, int initHeight)
    : D3DApp(hInstance, windowName, initWidth, initHeight)
{
}

GameApp::~GameApp()
{
}

bool GameApp::Init()
{
    if (!D3DApp::Init())
        return false;

    if (!InitEffect())
        return false;

    if (!InitResource())
        return false;

    return true;
}

void GameApp::OnResize()
{
    D3DApp::OnResize();

    // 摄像机变更显示
    if (m_pCamera != nullptr)
    {
        m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
        m_pCamera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
        m_VSConstantBufferOnResize.projection = XMMatrixTranspose(m_pCamera->GetProjXM());

        D3D11_MAPPED_SUBRESOURCE mappedData;
        HR(m_pd3dImmediateContext->Map(m_pVSConstantBufferOnResize.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
        memcpy_s(mappedData.pData, sizeof(VSConstantBufferOnResize), &m_VSConstantBufferOnResize,
                 sizeof(VSConstantBufferOnResize));
        m_pd3dImmediateContext->Unmap(m_pVSConstantBufferOnResize.Get(), 0);
    }
}

void GameApp::ImGUISet()
{
    // ImGui::ShowAboutWindow();
    // ImGui::ShowDemoWindow();
    // ImGui::ShowUserGuide();
    if (ImGui::Begin("Camera"))
    {
        ImGui::Text("W/S/A/D in FPS/Free camera");
        ImGui::Text("Hold the right mouse button and drag the view");
        ImGui::Text("The box moves only at First Person mode");

        // 获取子类
        auto cam1st = std::dynamic_pointer_cast<FirstPersonCamera>(m_pCamera);
        auto cam3rd = std::dynamic_pointer_cast<ThirdPersonCamera>(m_pCamera);
        static int curr_item = 0;
        static const char *modes[] = {"First Person", "Third Person", "Free Camera"};
        if (ImGui::Combo("Camera Mode", &curr_item, modes, ARRAYSIZE(modes)))
        {
            if (curr_item == 0 && m_CameraMode != CameraMode::FirstPerson)
            {

                m_CameraMode = CameraMode::FirstPerson;
            }
            else if (curr_item == 2 && m_CameraMode != CameraMode::Free)
            {
                if (!cam1st)
                {
                    cam1st = std::make_shared<FirstPersonCamera>();
                    cam1st->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
                    m_pCamera = cam1st;
                }
                // 从箱子上方开始
                XMFLOAT3 pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
                XMFLOAT3 to = XMFLOAT3(0.0f, 0.0f, 1.0f);
                XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);
                pos.y += 3;
                cam1st->LookTo(pos, to, up);

                m_CameraMode = CameraMode::Free;
            }
        }

        auto cameraPos = m_pCamera->GetPosition();
        ImGui::Text("Camera Position\n%.2f %.2f %.2f", cameraPos.x, cameraPos.y, cameraPos.z);
    }
    ImGui::End();
    if (ImGui::Begin("Main Menu"))
    {
        // auto woodPos = woodCrateTransform.GetPosition();
        // ImGui::Text("Box Position\n%.2f %.2f %.2f", woodPos.x, woodPos.y, woodPos.z);
        auto cameraPos = m_pCamera->GetPosition();
        ImGui::Text("Camera Position\n%.2f %.2f %.2f", cameraPos.x, cameraPos.y, cameraPos.z);
        if (ImGui::CollapsingHeader("Model Change"))
        {
            static int curr_model = static_cast<int>(m_ModelType);
            const char *mode_strs[] = {"Box", "Sphere"};
            if (ImGui::Combo("Model Type", &curr_model, mode_strs, IM_ARRAYSIZE(mode_strs)))
            {
                m_ModelType = curr_model;
                Geometry::MeshData<VertexPosNormalTex> meshData;
                switch (m_ModelType)
                {
                case 0:
                    meshData = Geometry::CreateBox();
                    break;
                case 1:
                    meshData = Geometry::CreateSphere(0.5f, 100, 100);
                    break;
                default:
                    break;
                }
                ResetMesh(meshData);
            }
        }
        if (ImGui::CollapsingHeader("Light Type Change"))
        {
            static int curr_light = static_cast<int>(m_LightType);
            const char *mode_strs[] = {"Direct", "Point", "Spot"};
            if (ImGui::Combo("Light Type", &curr_light, mode_strs, IM_ARRAYSIZE(mode_strs)))
            {
                if (m_LightType != curr_light)
                    m_bNeedUpdatePS = true;
                m_LightType = curr_light;
                switch (m_LightType)
                {
                case 0:
                    m_PSConstantBuffer.NumDirLight = 1;
                    m_PSConstantBuffer.NumPointLight = 0;
                    m_PSConstantBuffer.NumSpotLight = 0;
                    break;
                case 1:
                    m_PSConstantBuffer.NumDirLight = 0;
                    m_PSConstantBuffer.NumPointLight = 1;
                    m_PSConstantBuffer.NumSpotLight = 0;
                    break;

                case 2:
                    m_PSConstantBuffer.NumDirLight = 0;
                    m_PSConstantBuffer.NumPointLight = 0;
                    m_PSConstantBuffer.NumSpotLight = 1;
                    break;
                default:
                    break;
                }
            }
        }
        if (ImGui::CollapsingHeader("Light Change"))
        {
            bool changed = false;
            switch (m_LightType)
            {
            case 0:
                changed |= ImGui::SliderFloat3("Direction", &m_PSConstantBuffer.dirLight[0].direction.x, -1.0f, 1.0f);
                changed |= ImGui::SliderFloat4("Ambient", &m_PSConstantBuffer.dirLight[0].ambient.x, 0.0f, 1.0f);
                changed |= ImGui::SliderFloat4("Diffuse", &m_PSConstantBuffer.dirLight[0].diffuse.x, 0.0f, 1.0f);
                changed |= ImGui::SliderFloat4("Specular", &m_PSConstantBuffer.dirLight[0].specular.x, 0.0f, 1.0f);
                break;
            case 1:
                changed |= ImGui::SliderFloat3("Position", &m_PSConstantBuffer.pointLight[0].position.x, -10.0f, 10.0f);
                changed |= ImGui::SliderFloat4("Ambient", &m_PSConstantBuffer.pointLight[0].ambient.x, 0.0f, 1.0f);
                changed |= ImGui::SliderFloat4("Diffuse", &m_PSConstantBuffer.pointLight[0].diffuse.x, 0.0f, 1.0f);
                changed |= ImGui::SliderFloat4("Specular", &m_PSConstantBuffer.pointLight[0].specular.x, 0.0f, 1.0f);
                break;

            case 2:

                break;
            default:
                break;
            }

            m_bNeedUpdatePS = changed;
        }
    }
    ImGui::End();
    ImGui::Render();
}

void GameApp::UpdateCamera(float dt)
{
    // 获取子类
    auto cam1st = std::dynamic_pointer_cast<FirstPersonCamera>(m_pCamera);
    auto cam3rd = std::dynamic_pointer_cast<ThirdPersonCamera>(m_pCamera);

    ImGuiIO &io = ImGui::GetIO();
    if (m_CameraMode == CameraMode::FirstPerson || m_CameraMode == CameraMode::Free)
    {
        // 第一人称/自由摄像机的操作
        float d1 = 0.0f, d2 = 0.0f;
        if (ImGui::IsKeyDown(ImGuiKey_W))
            d1 += dt;
        if (ImGui::IsKeyDown(ImGuiKey_S))
            d1 -= dt;
        if (ImGui::IsKeyDown(ImGuiKey_A))
            d2 -= dt;
        if (ImGui::IsKeyDown(ImGuiKey_D))
            d2 += dt;

        if (m_CameraMode == CameraMode::FirstPerson)
            cam1st->Walk(d1 * 6.0f);
        else
            cam1st->MoveForward(d1 * 6.0f);
        cam1st->Strafe(d2 * 6.0f);

        // 将摄像机位置限制在[-8.9, 8.9]x[-8.9, 8.9]x[0.0, 8.9]的区域内
        // 不允许穿地
        XMFLOAT3 adjustedPos;
        XMStoreFloat3(&adjustedPos, XMVectorClamp(cam1st->GetPositionXM(), XMVectorSet(-8.9f, 0.0f, -8.9f, 0.0f),
                                                  XMVectorReplicate(8.9f)));
        cam1st->SetPosition(adjustedPos);

        if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
        {
            cam1st->Pitch(io.MouseDelta.y * 0.01f);
            cam1st->RotateY(io.MouseDelta.x * 0.01f);
        }
    }
    else if (m_CameraMode == CameraMode::ThirdPerson)
    {

        // 绕物体旋转
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
        {
            cam3rd->RotateX(io.MouseDelta.y * 0.01f);
            cam3rd->RotateY(io.MouseDelta.x * 0.01f);
        }
        cam3rd->Approach(-io.MouseWheel * 1.0f);
    }

    XMStoreFloat4(&m_VSConstantBufferEveryFrame.eyePos, m_pCamera->GetPositionXM());
    m_VSConstantBufferEveryFrame.view = XMMatrixTranspose(m_pCamera->GetViewXM());
}

void GameApp::UpdateMVP(float dt)
{

    D3D11_MAPPED_SUBRESOURCE mappedData;
    HR(m_pd3dImmediateContext->Map(m_pVSConstantBufferEveryFrame.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
    memcpy_s(mappedData.pData, sizeof(VSConstantBufferEveryFrame), &m_VSConstantBufferEveryFrame,
             sizeof(VSConstantBufferEveryFrame));
    m_pd3dImmediateContext->Unmap(m_pVSConstantBufferEveryFrame.Get(), 0);

    if (m_bNeedUpdatePS)
    {
        HR(m_pd3dImmediateContext->Map(m_pPSConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
        memcpy_s(mappedData.pData, sizeof(PSConstantBuffer), &m_PSConstantBuffer, sizeof(PSConstantBuffer));
        m_pd3dImmediateContext->Unmap(m_pPSConstantBuffer.Get(), 0);
        m_bNeedUpdatePS = false;
    }
}

void GameApp::UpdateScene(float dt)
{
    ImGUISet();

    UpdateCamera(dt);

    UpdateMVP(dt);
}

void GameApp::DrawScene()
{
    assert(m_pd3dImmediateContext);
    assert(m_pSwapChain);
    static float ClearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), ClearColor);
    m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                                  1.0f, 0);

    // m_pd3dImmediateContext->DrawIndexed(m_IndexCount, 0, 0);

    for (auto &obj : Objects)
    {
        obj.Draw(m_pd3dImmediateContext.Get());
    }

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    HR(m_pSwapChain->Present(0, 0));
}

bool GameApp::InitEffect()
{
    ComPtr<ID3DBlob> blog = nullptr;

    // 顶点着色器
    HR(CompileShaderFromFile(nullptr, L"HLSL\\Basic3D_VS.hlsl", "VS", "vs_5_0", blog.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreateVertexShader(blog->GetBufferPointer(), blog->GetBufferSize(), nullptr,
                                        m_pVertexShader.GetAddressOf()));

    // 输入布局
    HR(m_pd3dDevice->CreateInputLayout(VertexPosNormalTex::inputLayout, ARRAYSIZE(VertexPosNormalTex::inputLayout),
                                       blog->GetBufferPointer(), blog->GetBufferSize(), m_pInputLayout.GetAddressOf()));

    // 片段着色器
    HR(CompileShaderFromFile(nullptr, L"HLSL\\Basic3D_PS.hlsl", "PS", "ps_5_0", blog.ReleaseAndGetAddressOf()));

    HR(m_pd3dDevice->CreatePixelShader(blog->GetBufferPointer(), blog->GetBufferSize(), nullptr,
                                       m_pPixelShader.GetAddressOf()));

    return true;
}

bool GameApp::InitResource()
{

    // 模型
    Geometry::MeshData<VertexPosNormalTex> meshData = Geometry::CreateBox();
    Geometry::MeshData<VertexPosNormalTex> meshData2 = Geometry::CreateSphere(2.0f, 100, 100);
    Objects.push_back(GameObject());
    // Objects.push_back(GameObject());
    Objects[0].SetBuffer(m_pd3dDevice.Get(), meshData);
    // Objects[1].SetBuffer(m_pd3dDevice.Get(), meshData2);

    // 常量缓冲区
    D3D11_BUFFER_DESC cbd;
    ZeroMemory(&cbd, sizeof(cbd));
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.ByteWidth = sizeof(VSConstantBufferEveryDrawing);
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pVSConstantBufferEveryDrawing.GetAddressOf()));

    cbd.ByteWidth = sizeof(VSConstantBufferEveryFrame);
    HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pVSConstantBufferEveryFrame.GetAddressOf()));

    cbd.ByteWidth = sizeof(VSConstantBufferOnResize);
    HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pVSConstantBufferOnResize.GetAddressOf()));

    cbd.ByteWidth = sizeof(PSConstantBuffer);
    HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pPSConstantBuffer.GetAddressOf()));

    // 相机
    auto camera = std::make_shared<FirstPersonCamera>();
    m_pCamera = camera;
    camera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
    camera->LookAt(XMFLOAT3(), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));
    m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
    m_pCamera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
    m_VSConstantBufferOnResize.projection = XMMatrixTranspose(m_pCamera->GetProjXM());
    D3D11_MAPPED_SUBRESOURCE mappedData;
    HR(m_pd3dImmediateContext->Map(m_pVSConstantBufferOnResize.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
    memcpy_s(mappedData.pData, sizeof(VSConstantBufferOnResize), &m_VSConstantBufferOnResize,
             sizeof(VSConstantBufferOnResize));
    m_pd3dImmediateContext->Unmap(m_pVSConstantBufferOnResize.Get(), 0);

    // 初始化常量缓冲区的值
    // 初始化默认光照
    // 方向光
    m_DirLight.ambient = XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f);
    m_DirLight.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    m_DirLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_DirLight.direction = XMFLOAT3(-0.577f, -0.577f, 0.577f);
    // 点光
    m_PointLight.position = XMFLOAT3(5.0f, 5.0f, -10.0f);
    m_PointLight.ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
    m_PointLight.diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
    m_PointLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_PointLight.att = XMFLOAT3(0.0f, 0.1f, 0.0f);
    m_PointLight.range = 100.0f;
    // 聚光灯
    m_SpotLight.position = XMFLOAT3(0.0f, 0.0f, -5.0f);
    m_SpotLight.direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
    m_SpotLight.ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    m_SpotLight.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    m_SpotLight.specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    m_SpotLight.att = XMFLOAT3(1.0f, 0.0f, 0.0f);
    m_SpotLight.spot = 12.0f;
    m_SpotLight.range = 10000.0f;
    // 使用默认平行光
    m_PSConstantBuffer.dirLight[0] = m_DirLight;
    m_PSConstantBuffer.pointLight[0] = m_PointLight;
    m_PSConstantBuffer.spotLight[0] = m_SpotLight;
    m_PSConstantBuffer.NumDirLight = 1;
    m_PSConstantBuffer.NumPointLight = 0;
    m_PSConstantBuffer.NumSpotLight = 0;

    // 更新PS常量缓冲区资源
    // D3D11_MAPPED_SUBRESOURCE mappedData;
    HR(m_pd3dImmediateContext->Map(m_pPSConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
    memcpy_s(mappedData.pData, sizeof(PSConstantBuffer), &m_PSConstantBuffer, sizeof(PSConstantBuffer));
    m_pd3dImmediateContext->Unmap(m_pPSConstantBuffer.Get(), 0);

    UINT stride = sizeof(VertexPosNormalTex);
    UINT offset = 0;

    // 设置图元类型，设定输入布局
    m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pd3dImmediateContext->IASetInputLayout(m_pInputLayout.Get());
    // 将着色器绑定到渲染管线
    m_pd3dImmediateContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
    // VS常量缓冲区对应HLSL寄存于b0的常量缓冲区
    m_pd3dImmediateContext->VSSetConstantBuffers(0, 1, m_pVSConstantBufferEveryDrawing.GetAddressOf());
    m_pd3dImmediateContext->VSSetConstantBuffers(1, 1, m_pVSConstantBufferEveryFrame.GetAddressOf());
    m_pd3dImmediateContext->VSSetConstantBuffers(2, 1, m_pVSConstantBufferOnResize.GetAddressOf());
    // PS常量缓冲区对应HLSL寄存于b1的常量缓冲区
    m_pd3dImmediateContext->PSSetConstantBuffers(0, 1, m_pVSConstantBufferEveryDrawing.GetAddressOf());
    m_pd3dImmediateContext->PSSetConstantBuffers(1, 1, m_pVSConstantBufferEveryFrame.GetAddressOf());
    m_pd3dImmediateContext->PSSetConstantBuffers(3, 1, m_pPSConstantBuffer.GetAddressOf());
    m_pd3dImmediateContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);

    // 光栅化状态
    D3D11_RASTERIZER_DESC rasterizerDesc;
    ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;
    rasterizerDesc.FrontCounterClockwise = false;
    rasterizerDesc.DepthClipEnable = true;
    HR(m_pd3dDevice->CreateRasterizerState(&rasterizerDesc, m_pRSWireframe.GetAddressOf()));
    m_pd3dImmediateContext->RSSetState(m_pRSWireframe.Get());

    // 纹理和采样器状态
    // Sampler
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    HR(m_pd3dDevice->CreateSamplerState(&sampDesc, m_pSamplerState.GetAddressOf()));
    m_pd3dImmediateContext->PSSetSamplers(0, 1, m_pSamplerState.GetAddressOf());

    // Texture
    HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\WoodCrate.dds", nullptr, m_pTexture.GetAddressOf()));
    // m_pd3dImmediateContext->PSSetShaderResources(0, 1, m_pTexture.GetAddressOf());
    Objects[0].SetTexture(m_pTexture.Get());
    // Objects[1].SetTexture(m_pTexture.Get());

    // ******************
    // 设置调试对象名
    //
    D3D11SetDebugObjectName(m_pInputLayout.Get(), "VertexPosColorLayout");
    D3D11SetDebugObjectName(m_pVertexShader.Get(), "Trangle_VS");
    D3D11SetDebugObjectName(m_pPixelShader.Get(), "Trangle_PS");
    return true;
}

template <class VertexType> bool GameApp::ResetMesh(const Geometry::MeshData<VertexType> &meshData)
{
    // // 释放旧资源
    // m_pVertexBuffer.Reset();
    // m_pIndexBuffer.Reset();

    // // 设置顶点缓冲区描述
    // D3D11_BUFFER_DESC vbd;
    // ZeroMemory(&vbd, sizeof(vbd));
    // vbd.Usage = D3D11_USAGE_IMMUTABLE;
    // vbd.ByteWidth = (UINT)meshData.vertexVec.size() * sizeof(VertexType);
    // vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    // vbd.CPUAccessFlags = 0;
    // // 新建顶点缓冲区
    // D3D11_SUBRESOURCE_DATA InitData;
    // ZeroMemory(&InitData, sizeof(InitData));
    // InitData.pSysMem = meshData.vertexVec.data();
    // HR(m_pd3dDevice->CreateBuffer(&vbd, &InitData, m_pVertexBuffer.GetAddressOf()));

    // // 输入装配阶段的顶点缓冲区设置
    // UINT stride = sizeof(VertexType); // 跨越字节数
    // UINT offset = 0;                  // 起始偏移量

    // m_pd3dImmediateContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);

    // // 设置索引缓冲区描述
    // m_IndexCount = (UINT)meshData.indexVec.size();
    // D3D11_BUFFER_DESC ibd;
    // ZeroMemory(&ibd, sizeof(ibd));
    // ibd.Usage = D3D11_USAGE_IMMUTABLE;
    // ibd.ByteWidth = m_IndexCount * sizeof(DWORD);
    // ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    // ibd.CPUAccessFlags = 0;
    // // 新建索引缓冲区
    // InitData.pSysMem = meshData.indexVec.data();
    // HR(m_pd3dDevice->CreateBuffer(&ibd, &InitData, m_pIndexBuffer.GetAddressOf()));
    // // 输入装配阶段的索引缓冲区设置
    // m_pd3dImmediateContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

    // // 设置调试对象名
    // D3D11SetDebugObjectName(m_pVertexBuffer.Get(), "VertexBuffer");
    // D3D11SetDebugObjectName(m_pIndexBuffer.Get(), "IndexBuffer");

    // return true;
    return true;
}

GameApp::GameObject::GameObject() : m_IndexCount(), m_VertexStride(), m_Material(DefaultMaterial)
{
}

Transform &GameApp::GameObject::GetTransform()
{
    return m_Transform;
}

const Transform &GameApp::GameObject::GetTransform() const
{
    return m_Transform;
}

template <class VertexType, class IndexType>
void GameApp::GameObject::SetBuffer(ID3D11Device *device, const Geometry::MeshData<VertexType, IndexType> &meshData)
{
    // 释放旧资源
    m_pVertexBuffer.Reset();
    m_pIndexBuffer.Reset();

    // 设置顶点缓冲区描述
    m_VertexStride = sizeof(VertexType);
    D3D11_BUFFER_DESC vbd;
    ZeroMemory(&vbd, sizeof(vbd));
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = (UINT)meshData.vertexVec.size() * m_VertexStride;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    // 新建顶点缓冲区
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = meshData.vertexVec.data();
    HR(device->CreateBuffer(&vbd, &InitData, m_pVertexBuffer.GetAddressOf()));

    // 设置索引缓冲区描述
    m_IndexCount = (UINT)meshData.indexVec.size();
    D3D11_BUFFER_DESC ibd;
    ZeroMemory(&ibd, sizeof(ibd));
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = m_IndexCount * sizeof(IndexType);
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    // 新建索引缓冲区
    InitData.pSysMem = meshData.indexVec.data();
    HR(device->CreateBuffer(&ibd, &InitData, m_pIndexBuffer.GetAddressOf()));
}

void GameApp::GameObject::SetTexture(ID3D11ShaderResourceView *texture)
{
    m_pTexture = texture;
}

void GameApp::GameObject::Draw(ID3D11DeviceContext *deviceContext)
{
    // 设置顶点/索引缓冲区
    UINT strides = m_VertexStride;
    UINT offsets = 0;
    deviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &strides, &offsets);
    deviceContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

    // 获取之前已经绑定到渲染管线上的常量缓冲区并进行修改
    ComPtr<ID3D11Buffer> cBuffer = nullptr;
    deviceContext->VSGetConstantBuffers(0, 1, cBuffer.GetAddressOf());
    VSConstantBufferEveryDrawing cbDrawing;

    // 内部进行转置
    XMMATRIX W = m_Transform.GetLocalToWorldMatrixXM();
    cbDrawing.world = XMMatrixTranspose(W);
    cbDrawing.worldInvTranspose = XMMatrixTranspose(InverseTranspose(W));
    cbDrawing.material = m_Material;

    // 更新常量缓冲区
    D3D11_MAPPED_SUBRESOURCE mappedData;
    HR(deviceContext->Map(cBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
    memcpy_s(mappedData.pData, sizeof(VSConstantBufferEveryDrawing), &cbDrawing, sizeof(VSConstantBufferEveryDrawing));
    deviceContext->Unmap(cBuffer.Get(), 0);

    // 设置纹理
    deviceContext->PSSetShaderResources(0, 1, m_pTexture.GetAddressOf());
    // 可以开始绘制
    deviceContext->DrawIndexed(m_IndexCount, 0, 0);
}

void GameApp::GameObject::SetDebugObjectName(const std::string &name)
{
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
    D3D11SetDebugObjectName(m_pVertexBuffer.Get(), name + ".VertexBuffer");
    D3D11SetDebugObjectName(m_pIndexBuffer.Get(), name + ".IndexBuffer");
#else
    UNREFERENCED_PARAMETER(name);
#endif
}