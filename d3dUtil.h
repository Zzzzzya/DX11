//***************************************************************************************
// d3dUtil.h by X_Jun(MKXJun) (C) 2018-2022 All Rights Reserved.
// Licensed under the MIT License.
//
// D3D实用工具集
// Direct3D utility tools.
//***************************************************************************************

#ifndef D3DUTIL_H
#define D3DUTIL_H

#include <DirectXCollision.h> // 已包含DirectXMath.h
#include <DirectXColors.h>
#include <DirectXPackedVector.h>
#include <d3d11_1.h> // 已包含Windows.h
#include <d3dcompiler.h>
#include <string>
#include <vector>

//
// 宏相关
//

// 默认开启图形调试器具名化
// 如果不需要该项功能，可通过全局文本替换将其值设置为0
#ifndef GRAPHICS_DEBUGGER_OBJECT_NAME
#define GRAPHICS_DEBUGGER_OBJECT_NAME (1)
#endif

// 安全COM组件释放宏
#define SAFE_RELEASE(p)                                                                                                \
    {                                                                                                                  \
        if ((p)) {                                                                                                     \
            (p)->Release();                                                                                            \
            (p) = nullptr;                                                                                             \
        }                                                                                                              \
    }

//
// 辅助调试相关函数
//

// ------------------------------
// D3D11SetDebugObjectName函数
// ------------------------------
// 为D3D设备创建出来的对象在图形调试器中设置对象名
// [In]resource				D3D11设备创建出的对象
// [In]name					对象名
template <UINT TNameLength>
inline void D3D11SetDebugObjectName(_In_ ID3D11DeviceChild *resource, _In_ const char (&name)[TNameLength]) {
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
    resource->SetPrivateData(WKPDID_D3DDebugObjectName, TNameLength - 1, name);
#else
    UNREFERENCED_PARAMETER(resource);
    UNREFERENCED_PARAMETER(name);
#endif
}

// ------------------------------
// D3D11SetDebugObjectName函数
// ------------------------------
// 为D3D设备创建出来的对象在图形调试器中设置对象名
// [In]resource				D3D11设备创建出的对象
// [In]name					对象名
// [In]length				字符串长度
inline void D3D11SetDebugObjectName(_In_ ID3D11DeviceChild *resource, _In_ LPCSTR name, _In_ UINT length) {
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
    resource->SetPrivateData(WKPDID_D3DDebugObjectName, length, name);
#else
    UNREFERENCED_PARAMETER(resource);
    UNREFERENCED_PARAMETER(name);
    UNREFERENCED_PARAMETER(length);
#endif
}

// ------------------------------
// D3D11SetDebugObjectName函数
// ------------------------------
// 为D3D设备创建出来的对象在图形调试器中设置对象名
// [In]resource				D3D11设备创建出的对象
// [In]name					对象名
inline void D3D11SetDebugObjectName(_In_ ID3D11DeviceChild *resource, _In_ const std::string &name) {
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
    resource->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)name.length(), name.c_str());
#else
    UNREFERENCED_PARAMETER(resource);
    UNREFERENCED_PARAMETER(name);
#endif
}

// ------------------------------
// D3D11SetDebugObjectName函数
// ------------------------------
// 为D3D设备创建出来的对象在图形调试器中清空对象名
// [In]resource				D3D11设备创建出的对象
inline void D3D11SetDebugObjectName(_In_ ID3D11DeviceChild *resource, _In_ std::nullptr_t) {
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
    resource->SetPrivateData(WKPDID_D3DDebugObjectName, 0, nullptr);
#else
    UNREFERENCED_PARAMETER(resource);
#endif
}

// ------------------------------
// DXGISetDebugObjectName函数
// ------------------------------
// 为DXGI对象在图形调试器中设置对象名
// [In]object				DXGI对象
// [In]name					对象名
template <UINT TNameLength>
inline void DXGISetDebugObjectName(_In_ IDXGIObject *object, _In_ const char (&name)[TNameLength]) {
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
    object->SetPrivateData(WKPDID_D3DDebugObjectName, TNameLength - 1, name);
#else
    UNREFERENCED_PARAMETER(object);
    UNREFERENCED_PARAMETER(name);
#endif
}

// ------------------------------
// DXGISetDebugObjectName函数
// ------------------------------
// 为DXGI对象在图形调试器中设置对象名
// [In]object				DXGI对象
// [In]name					对象名
// [In]length				字符串长度
inline void DXGISetDebugObjectName(_In_ IDXGIObject *object, _In_ LPCSTR name, _In_ UINT length) {
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
    object->SetPrivateData(WKPDID_D3DDebugObjectName, length, name);
#else
    UNREFERENCED_PARAMETER(object);
    UNREFERENCED_PARAMETER(name);
    UNREFERENCED_PARAMETER(length);
#endif
}

// ------------------------------
// DXGISetDebugObjectName函数
// ------------------------------
// 为DXGI对象在图形调试器中设置对象名
// [In]object				DXGI对象
// [In]name					对象名
inline void DXGISetDebugObjectName(_In_ IDXGIObject *object, _In_ const std::string &name) {
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
    object->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)name.length(), name.c_str());
#else
    UNREFERENCED_PARAMETER(object);
    UNREFERENCED_PARAMETER(name);
#endif
}

// ------------------------------
// DXGISetDebugObjectName函数
// ------------------------------
// 为DXGI对象在图形调试器中清空对象名
// [In]object				DXGI对象
inline void DXGISetDebugObjectName(_In_ IDXGIObject *object, _In_ std::nullptr_t) {
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
    object->SetPrivateData(WKPDID_D3DDebugObjectName, 0, nullptr);
#else
    UNREFERENCED_PARAMETER(object);
#endif
}

/* 着色器相关  Shader */
/**
 * @brief
 *
 * @param csoFileNameInOut [In] 编译好的二进制着色器文件名 - 检查是否存在，如果存在则直接读取
 * @param hlslFileNmae [In] HLSL源文件名
 * @param entryPoint   [In] 入口点 - 指定开始的函数
 * @param shaderModel  [In] 着色器模型 - 格式为"*s_5_0"，*可以为c,d,g,h,p,v之一
 * @param ppBlobOut    [Out] 输出着色器二进制信息
 * @return HRESULT
 */
HRESULT CompileShaderFromFile(const WCHAR *csoFileNameInOut,
                              const WCHAR *hlslFileName,
                              LPCSTR entryPoint,
                              LPCSTR shaderModel,
                              ID3DBlob **ppBlobOut);

#endif
