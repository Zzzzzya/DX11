#include "GameApp.h"
#include "DXTrace.h"
#include "d3dUtil.h"
#include "DDSTextureLoader11.h"
#include "Camera.h"
using namespace DirectX;
using DirectX::XMMatrixTranspose;

static Material DefaultMaterial{XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
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

            auto camera = std::dynamic_pointer_cast<FirstPersonCamera>(m_pCamera);
            if (camera)
            {
                camera->LookTo(camera->GetPosition(), m_PSConstantBuffer.dirLight[0].direction,
                               XMFLOAT3(0.0f, 1.0f, 0.0f));
                m_ShadowConstantBufferView.view = XMMatrixTranspose(camera->GetViewXM());
                auto pos = camera->GetPosition();
                m_ShadowConstantBufferView.eyePos = XMFLOAT4(pos.x, pos.y, pos.z, 1.0f);
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
        XMStoreFloat3(&adjustedPos, XMVectorClamp(cam1st->GetPositionXM(), XMVectorSet(-100.0f, 0.0f, -100.0f, 0.0f),
                                                  XMVectorReplicate(100.0f)));
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

        HR(m_pd3dImmediateContext->Map(m_pShadowConstantBufferView.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
        memcpy_s(mappedData.pData, sizeof(VSConstantBufferEveryFrame), &m_VSConstantBufferOnResize,
                 sizeof(VSConstantBufferEveryFrame));
        m_pd3dImmediateContext->Unmap(m_pShadowConstantBufferView.Get(), 0);

        m_bNeedUpdatePS = false;
    }
}

void GameApp::UpdateScene(float dt)
{
    ImGUISet();

    UpdateCamera(dt);

    UpdateMVP(dt);

    PointLightsObjects[0].GetTransform().SetPosition(m_PSConstantBuffer.pointLight[0].position);
}

void GameApp::DrawScene()
{
    static float ClearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    assert(m_pd3dImmediateContext);
    assert(m_pSwapChain);

    //  主渲染阶段
    UINT stride = sizeof(VertexPosNormalTex);
    UINT offset = 0;

    // Shadow Map
    m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pd3dImmediateContext->IASetInputLayout(m_pShadowInputLayout.Get());

    m_pd3dImmediateContext->VSSetShader(m_pShadowVS.Get(), nullptr, 0);
    m_pd3dImmediateContext->PSSetShader(m_pShadowPS.Get(), nullptr, 0);

    m_pd3dImmediateContext->VSSetConstantBuffers(0, 1, m_pVSConstantBufferEveryDrawing.GetAddressOf());
    m_pd3dImmediateContext->VSSetConstantBuffers(1, 1, m_pShadowConstantBufferView.GetAddressOf());
    m_pd3dImmediateContext->VSSetConstantBuffers(2, 1, m_pShadowConstantBufferPro.GetAddressOf());
    m_pd3dImmediateContext->PSSetSamplers(0, 1, RenderStates::SSAnisotropicWrap.GetAddressOf());

    m_pd3dImmediateContext->RSSetState(nullptr);
    m_pd3dImmediateContext->OMSetDepthStencilState(nullptr, 0);
    m_pd3dImmediateContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);

    CD3D11_VIEWPORT shadowViewPort(0.0f, 0.0f, 4096.0f, 4096.0f);
    m_pd3dImmediateContext->RSSetViewports(1, &shadowViewPort);
    m_pd3dImmediateContext->OMSetRenderTargets(1, m_pTexture_ShadowMap->GetAddressOfRenderTarget(),
                                               m_pTexture_ShadowMapDepth->GetDepthStencil());

    m_pd3dImmediateContext->ClearRenderTargetView(m_pTexture_ShadowMap->GetRenderTarget(), ClearColor);
    m_pd3dImmediateContext->ClearDepthStencilView(m_pTexture_ShadowMapDepth->GetDepthStencil(),
                                                  D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL | D3D11_CLEAR_STENCIL, 1.0f,
                                                  0);

    for (auto &obj : Objects)
    {
        obj.Draw(m_pd3dImmediateContext.Get());
    }
    static ID3D11ShaderResourceView *const pSRV[1] = {nullptr};
    m_pd3dImmediateContext->PSSetShaderResources(0, 1, pSRV);

    // 设置图元类型，设定输入布局
    CD3D11_VIEWPORT viewPort(0.0f, 0.0f, static_cast<float>(m_ClientWidth), static_cast<float>(m_ClientHeight));
    m_pd3dImmediateContext->RSSetViewports(1, &viewPort);
    m_pd3dImmediateContext->OMSetRenderTargets(1, m_pTexture_Screen->GetAddressOfRenderTarget(),
                                               m_pTexture_ScreenDepth->GetDepthStencil());

    m_pd3dImmediateContext->ClearRenderTargetView(m_pTexture_Screen->GetRenderTarget(), ClearColor);
    m_pd3dImmediateContext->ClearDepthStencilView(m_pTexture_ScreenDepth->GetDepthStencil(),
                                                  D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL | D3D11_CLEAR_STENCIL, 1.0f,
                                                  0);
    m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pd3dImmediateContext->IASetInputLayout(m_pInputLayout.Get());
    // 将着色器绑定到渲染管线
    m_pd3dImmediateContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
    // VS常量缓冲区对应HLSL寄存于b0的常量缓冲区
    m_pd3dImmediateContext->VSSetConstantBuffers(0, 1, m_pVSConstantBufferEveryDrawing.GetAddressOf());
    m_pd3dImmediateContext->VSSetConstantBuffers(1, 1, m_pVSConstantBufferEveryFrame.GetAddressOf());
    m_pd3dImmediateContext->VSSetConstantBuffers(2, 1, m_pVSConstantBufferOnResize.GetAddressOf());
    m_pd3dImmediateContext->VSSetConstantBuffers(3, 1, m_pPSConstantBuffer.GetAddressOf());
    m_pd3dImmediateContext->VSSetConstantBuffers(4, 1, m_pCBDrawingStates.GetAddressOf());

    // PS常量缓冲区对应HLSL寄存于b1的常量缓冲区
    m_pd3dImmediateContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);
    m_pd3dImmediateContext->PSSetConstantBuffers(0, 1, m_pVSConstantBufferEveryDrawing.GetAddressOf());
    m_pd3dImmediateContext->PSSetConstantBuffers(1, 1, m_pVSConstantBufferEveryFrame.GetAddressOf());
    m_pd3dImmediateContext->PSSetConstantBuffers(2, 1, m_pVSConstantBufferOnResize.GetAddressOf());
    m_pd3dImmediateContext->PSSetConstantBuffers(3, 1, m_pPSConstantBuffer.GetAddressOf());
    m_pd3dImmediateContext->PSSetConstantBuffers(4, 1, m_pCBDrawingStates.GetAddressOf());

    m_pd3dImmediateContext->PSSetShaderResources(1, 1, m_pTexture_ShadowMap->GetAddressOfShaderResource());

    m_pd3dImmediateContext->RSSetState(RenderStates::RSNoCull.Get());

    m_pd3dImmediateContext->PSSetSamplers(0, 1, RenderStates::SSAnisotropicWrap.GetAddressOf());
    m_pd3dImmediateContext->OMSetBlendState(RenderStates::BSTransparent.Get(), nullptr, 0xFFFFFFFF);

    D3D11_MAPPED_SUBRESOURCE mappedData;
    m_CBDrawingStates.isReflection = false;
    HR(m_pd3dImmediateContext->Map(m_pCBDrawingStates.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
    memcpy_s(mappedData.pData, sizeof(CBDrawingStates), &m_CBDrawingStates, sizeof(CBDrawingStates));
    m_pd3dImmediateContext->Unmap(m_pCBDrawingStates.Get(), 0);
    m_pd3dImmediateContext->RSSetState(nullptr);
    m_pd3dImmediateContext->OMSetDepthStencilState(nullptr, 0);
    m_pd3dImmediateContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);

    for (auto &obj : Objects)
    {
        obj.Draw(m_pd3dImmediateContext.Get());
    }

    for (auto &light : PointLightsObjects)
    {
        light.Draw(m_pd3dImmediateContext.Get());
    }

    m_pd3dImmediateContext->PSSetShaderResources(0, 1, pSRV);

    // 屏幕渲染
    m_pd3dImmediateContext->RSSetViewports(1, &m_ScreenViewport);
    m_pd3dImmediateContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), nullptr);
    m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), ClearColor);
    m_pd3dImmediateContext->ClearDepthStencilView(
        m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL | D3D11_CLEAR_STENCIL, 1.0f, 0);

    stride = sizeof(VertexPosTex);
    offset = 0;

    m_pd3dImmediateContext->IASetInputLayout(m_pScreenInputLayout.Get());
    // 将着色器绑定到渲染管线
    m_pd3dImmediateContext->VSSetShader(m_pScreenVS.Get(), nullptr, 0);
    m_pd3dImmediateContext->PSSetShader(m_pScreenPS_None.Get(), nullptr, 0);
    m_pd3dImmediateContext->RSSetState(RenderStates::RSNoCull.Get());

    m_pd3dImmediateContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
    m_pd3dImmediateContext->PSSetShaderResources(0, 1, m_pTexture_Screen->GetAddressOfShaderResource());
    m_pd3dImmediateContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);

    m_pd3dImmediateContext->Draw(3, 0);

    m_pd3dImmediateContext->PSSetShaderResources(0, 1, pSRV);

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

    // 屏幕
    HR(CompileShaderFromFile(nullptr, L"HLSL\\Screen_VS.hlsl", "VS", "vs_5_0", blog.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreateVertexShader(blog->GetBufferPointer(), blog->GetBufferSize(), nullptr,
                                        m_pScreenVS.GetAddressOf()));

    HR(m_pd3dDevice->CreateInputLayout(VertexPosTex::inputLayout, ARRAYSIZE(VertexPosTex::inputLayout),
                                       blog->GetBufferPointer(), blog->GetBufferSize(),
                                       m_pScreenInputLayout.GetAddressOf()));

    HR(CompileShaderFromFile(nullptr, L"HLSL\\Screen_PS_None.hlsl", "PS", "ps_5_0", blog.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreatePixelShader(blog->GetBufferPointer(), blog->GetBufferSize(), nullptr,
                                       m_pScreenPS_None.GetAddressOf()));

    // Shadow Map
    HR(CompileShaderFromFile(nullptr, L"HLSL\\ShadowMap_VS.hlsl", "VS", "vs_5_0", blog.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreateVertexShader(blog->GetBufferPointer(), blog->GetBufferSize(), nullptr,
                                        m_pShadowVS.GetAddressOf()));

    HR(m_pd3dDevice->CreateInputLayout(VertexPosNormalTex::inputLayout, ARRAYSIZE(VertexPosNormalTex::inputLayout),
                                       blog->GetBufferPointer(), blog->GetBufferSize(),
                                       m_pShadowInputLayout.GetAddressOf()));

    HR(CompileShaderFromFile(nullptr, L"HLSL\\ShadowMap_PS.hlsl", "PS", "ps_5_0", blog.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreatePixelShader(blog->GetBufferPointer(), blog->GetBufferSize(), nullptr,
                                       m_pShadowPS.GetAddressOf()));
    return true;
}

bool GameApp::InitResource()
{

    // 模型
    Geometry::MeshData<VertexPosNormalTex> meshData = Geometry::CreateBox(1.0f, 1.0f, 1.0f);
    Geometry::MeshData<VertexPosNormalTex> meshData2 = Geometry::CreateSphere(0.5f, 10, 10);
    Objects.push_back(GameObject());

    // Object[0] - 地板
    Objects[0].SetBuffer(m_pd3dDevice.Get(), Geometry::CreatePlane(XMFLOAT2(50.0f, 50.0f), XMFLOAT2(5.0f, 5.0f)));

    int numBox = 10;
    float bet = 50.0f / numBox;
    for (int i = 0; i < numBox; ++i)
    {
        float xx = -25.0f + i * bet + 2.0f;
        for (int j = 0; j < numBox; ++j)
        {
            float zz = -25.0f + j * bet + 2.0f;
            Objects.push_back(GameObject());
            auto &obj = Objects[Objects.size() - 1];
            obj.SetBuffer(m_pd3dDevice.Get(), meshData);
            obj.GetTransform().SetPosition(XMFLOAT3(xx, 0.51f, zz));
            obj.GetMaterial().reflect.w = 0.0f;
        }
    }

    // Light模型
    PointLightsObjects.push_back(GameObject());
    PointLightsObjects[0].SetBuffer(m_pd3dDevice.Get(), meshData2);

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
    HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pShadowConstantBufferView.GetAddressOf()));

    cbd.ByteWidth = sizeof(VSConstantBufferOnResize);
    HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pVSConstantBufferOnResize.GetAddressOf()));
    HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pShadowConstantBufferPro.GetAddressOf()));

    cbd.ByteWidth = sizeof(PSConstantBuffer);
    HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pPSConstantBuffer.GetAddressOf()));

    cbd.ByteWidth = sizeof(CBDrawingStates);
    HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pCBDrawingStates.GetAddressOf()));

    // 相机
    auto camera = std::make_shared<FirstPersonCamera>();
    m_pCamera = camera;
    camera->SetPosition(-32.71f, 17.44f, -32.05f);
    camera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
    camera->LookAt(XMFLOAT3(-32.71f, 17.44f, -32.05f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));
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
    m_DirLight.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    m_DirLight.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    m_DirLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_DirLight.direction = XMFLOAT3(-1.0f, -1.0f, -1.0f);
    // 点光
    m_PointLight.position = XMFLOAT3(50.0f, 10.0f, 50.0f);
    m_PointLight.ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
    m_PointLight.diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
    m_PointLight.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    m_PointLight.att = XMFLOAT3(1.0f, 0.0f, 0.0f);
    m_PointLight.range = 100.0f;
    PointLightsObjects[0].GetTransform().SetPosition(m_PointLight.position);
    auto &mat = PointLightsObjects[0].GetMaterial();
    mat.reflect.w = 0.0f;
    mat.reflect.x = 1.0f;
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
    m_PSConstantBuffer.reflection = XMMatrixTranspose(XMMatrixReflect(XMVectorSet(0.0f, 0.0f, -1.0f, 10.0f)));
    m_PSConstantBuffer.dirLight[0] = m_DirLight;
    m_PSConstantBuffer.pointLight[0] = m_PointLight;
    m_PSConstantBuffer.spotLight[0] = m_SpotLight;
    m_PSConstantBuffer.NumDirLight = 1;
    m_PSConstantBuffer.NumPointLight = 1;
    m_PSConstantBuffer.NumSpotLight = 0;
    m_PSConstantBuffer.pad1 = 0.0023;

    // 更新PS常量缓冲区资源
    // D3D11_MAPPED_SUBRESOURCE mappedData;

    m_CBDrawingStates.isReflection = 0;
    HR(m_pd3dImmediateContext->Map(m_pCBDrawingStates.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
    memcpy_s(mappedData.pData, sizeof(CBDrawingStates), &m_CBDrawingStates, sizeof(CBDrawingStates));
    m_pd3dImmediateContext->Unmap(m_pCBDrawingStates.Get(), 0);

    RenderStates::InitAll(m_pd3dDevice.Get());

    // light相机
    camera = std::make_shared<FirstPersonCamera>();
    m_pLightCamera = camera;
    camera->SetPosition(m_PointLight.position);
    camera->SetViewPort(0.0f, 0.0f, 4096, 4096);
    camera->LookTo(m_PointLight.position, XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));
    camera->SetFrustum(XM_PI / 3, 1, 20.0f, 100.0f);
    camera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
    m_ShadowConstantBufferPro.projection = XMMatrixTranspose(camera->GetOrthoProjXM());
    HR(m_pd3dImmediateContext->Map(m_pShadowConstantBufferPro.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
    memcpy_s(mappedData.pData, sizeof(VSConstantBufferOnResize), &m_ShadowConstantBufferPro,
             sizeof(VSConstantBufferOnResize));
    m_pd3dImmediateContext->Unmap(m_pShadowConstantBufferPro.Get(), 0);

    m_ShadowConstantBufferView.view = XMMatrixTranspose(camera->GetViewXM());
    auto pos = camera->GetPosition();
    m_ShadowConstantBufferView.eyePos = XMFLOAT4(pos.x, pos.y, pos.z, 1.0f);
    HR(m_pd3dImmediateContext->Map(m_pShadowConstantBufferView.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
    memcpy_s(mappedData.pData, sizeof(VSConstantBufferEveryFrame), &m_ShadowConstantBufferView,
             sizeof(VSConstantBufferEveryFrame));
    m_pd3dImmediateContext->Unmap(m_pShadowConstantBufferView.Get(), 0);

    m_PSConstantBuffer.ShadowMatrix = XMMatrixTranspose(camera->GetViewXM() * camera->GetOrthoProjXM());
    HR(m_pd3dImmediateContext->Map(m_pPSConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
    memcpy_s(mappedData.pData, sizeof(PSConstantBuffer), &m_PSConstantBuffer, sizeof(PSConstantBuffer));
    m_pd3dImmediateContext->Unmap(m_pPSConstantBuffer.Get(), 0);

    // Texture
    HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\WoodCrate.dds", nullptr, m_pTexture.GetAddressOf()));
    HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\floor.dds", nullptr, m_pTexture_floor.GetAddressOf()));
    HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\WireFence.dds", nullptr,
                                m_pTexture_wireFence.GetAddressOf()));
    HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\water.dds", nullptr, m_pTexture_water.GetAddressOf()));
    HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\brick.dds", nullptr, m_pTexture_brick.GetAddressOf()));
    // m_pd3dImmediateContext->PSSetShaderResources(0, 1, m_pTexture.GetAddressOf());
    Objects[0].SetTexture(m_pTexture_floor.Get());
    // Objects[1].SetTexture(m_pTexture.Get());

    // 渲染纹理
    m_pTexture_Screen = std::make_shared<Texture2D>(m_pd3dDevice.Get(), 1280, 720, DXGI_FORMAT_R8G8B8A8_UNORM);
    m_pTexture_ScreenDepth = std::make_shared<Depth2D>(m_pd3dDevice.Get(), 1280, 720);
    m_pTexture_ShadowMap = std::make_shared<Texture2D>(m_pd3dDevice.Get(), 4096, 4096, DXGI_FORMAT_R8G8B8A8_UNORM);
    m_pTexture_ShadowMapDepth = std::make_shared<Depth2D>(m_pd3dDevice.Get(), 4096, 4096);

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

Material &GameApp::GameObject::GetMaterial()
{
    return m_Material;
}

const Material &GameApp::GameObject::GetMaterial() const
{
    return m_Material;
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