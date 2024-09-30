#include "GameApp.h"
#include "DXTrace.h"
#include "d3dUtil.h"
using namespace DirectX;

/** */
const D3D11_INPUT_ELEMENT_DESC GameApp::VertexPosColor::inputLayout[2] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}};

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
}

void GameApp::UpdateScene(float dt)
{

    static float phi = 0.0f, theta = 0.0f;
    phi += 0.3f * dt, theta += 0.37f * dt;
    m_cbData.mWorld = DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationX(phi) * DirectX::XMMatrixRotationY(theta));
    m_cbData.mProjection =
        DirectX::XMMatrixTranspose(DirectX::XMMatrixPerspectiveFovLH(XM_PIDIV2, AspectRatio(), 1.0f, 1000.0f));

    D3D11_MAPPED_SUBRESOURCE mappedData;
    HR(m_pd3dImmediateContext->Map(m_pConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
    memcpy_s(mappedData.pData, sizeof(ConstantBuffer), &m_cbData, sizeof(ConstantBuffer));
    m_pd3dImmediateContext->Unmap(m_pConstantBuffer.Get(), 0);
}

void GameApp::DrawScene()
{
    assert(m_pd3dImmediateContext);
    assert(m_pSwapChain);
    static float ClearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), ClearColor);
    m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                                  1.0f, 0);

    m_pd3dImmediateContext->DrawIndexed(36, 0, 0);
    HR(m_pSwapChain->Present(0, 0));
}

bool GameApp::InitEffect()
{
    ComPtr<ID3DBlob> blog = nullptr;

    // 顶点着色器
    HR(CompileShaderFromFile(L"HLSL\\Quad_VS.cso", L"HLSL\\Quad_VS.hlsl", "VS", "vs_5_0",
                             blog.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreateVertexShader(blog->GetBufferPointer(), blog->GetBufferSize(), nullptr,
                                        m_pVertexShader.GetAddressOf()));

    // 输入布局
    HR(m_pd3dDevice->CreateInputLayout(VertexPosColor::inputLayout, ARRAYSIZE(VertexPosColor::inputLayout),
                                       blog->GetBufferPointer(), blog->GetBufferSize(), m_pInputLayout.GetAddressOf()));

    // 片段着色器
    HR(CompileShaderFromFile(L"HLSL\\Quad_PS.cso", L"HLSL\\Quad_PS.hlsl", "PS", "ps_5_0",
                             blog.ReleaseAndGetAddressOf()));

    HR(m_pd3dDevice->CreatePixelShader(blog->GetBufferPointer(), blog->GetBufferSize(), nullptr,
                                       m_pPixelShader.GetAddressOf()));

    return true;
}

bool GameApp::InitResource()
{
    // 设置三角形顶点
    VertexPosColor TriangleVertices[] = {

        {XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)},
        {XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)},
        {XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f)},
        {XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f)},
        {XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f)},
        {XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f)},
        {XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)},
        {XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f)}

    };
    // 创建顶点缓冲区 ： 1. 创建顶点缓冲描述 2. 创建顶点缓冲
    D3D11_BUFFER_DESC vbd;
    ZeroMemory(&vbd, sizeof(vbd));
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(TriangleVertices);
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = TriangleVertices;

    HR(m_pd3dDevice->CreateBuffer(&vbd, &InitData, m_pVertexBuffer.GetAddressOf()));

    // 创建索引缓冲区
    DWORD indices[] = {// 正面
                       0, 1, 2, 2, 3, 0,
                       // 左面
                       4, 5, 1, 1, 0, 4,
                       // 顶面
                       1, 5, 6, 6, 2, 1,
                       // 背面
                       7, 6, 5, 5, 4, 7,
                       // 右面
                       3, 2, 6, 6, 7, 3,
                       // 底面
                       4, 0, 3, 3, 7, 4};

    D3D11_BUFFER_DESC ibd;
    ZeroMemory(&ibd, sizeof(ibd));
    // ZeroMemory(&InitData, sizeof(InitData));
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(indices);
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;

    InitData.pSysMem = indices;

    HR(m_pd3dDevice->CreateBuffer(&ibd, &InitData, m_pIndexBuffer.GetAddressOf()));

    // 常量缓冲区
    D3D11_BUFFER_DESC cbd;
    ZeroMemory(&cbd, sizeof(cbd));
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.ByteWidth = sizeof(ConstantBuffer);
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffer.GetAddressOf()));

    // 初始化常量缓冲区的值
    m_cbData.mWorld = DirectX::XMMatrixIdentity();
    m_cbData.mView = XMMatrixTranspose(XMMatrixLookAtLH(DirectX::XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f),
                                                        DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
                                                        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));
    m_cbData.mProjection =
        XMMatrixTranspose(DirectX::XMMatrixPerspectiveFovLH(XM_PIDIV2, AspectRatio(), 1.0f, 1000.0f));

    UINT stride = sizeof(VertexPosColor);
    UINT offset = 0;

    m_pd3dImmediateContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);
    m_pd3dImmediateContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pd3dImmediateContext->IASetInputLayout(m_pInputLayout.Get());

    m_pd3dImmediateContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
    m_pd3dImmediateContext->VSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf());

    m_pd3dImmediateContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);

    // ******************
    // 设置调试对象名
    //
    D3D11SetDebugObjectName(m_pInputLayout.Get(), "VertexPosColorLayout");
    D3D11SetDebugObjectName(m_pVertexBuffer.Get(), "VertexBuffer");
    D3D11SetDebugObjectName(m_pVertexShader.Get(), "Trangle_VS");
    D3D11SetDebugObjectName(m_pPixelShader.Get(), "Trangle_PS");
    return true;
}
