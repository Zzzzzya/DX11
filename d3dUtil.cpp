#include "d3dUtil.h"

HRESULT CompileShaderFromFile(const WCHAR *csoFileNameInOut,
                              const WCHAR *hlslFileName,
                              LPCSTR entryPoint,
                              LPCSTR shaderModel,
                              ID3DBlob **ppBlobOut)
{
    /**
     * Step 1 : 检查是否有编译好的二进制着色器文件 - 有的话直接返回
     * Step 2 : 针对Debug环境，设置编译标志
     * Step 3 : 编译着色器
     * Step 4 : 如果设置了输出路径，则将编译好的二进制着色器文件保存到指定路径
     */

    HRESULT hr = S_OK;

    // Step 1
    if (csoFileNameInOut && D3DReadFileToBlob(csoFileNameInOut, ppBlobOut) == S_OK)
    {
        return hr;
    }

    // Step 2
    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
    // 设置 D3DCOMPILE_DEBUG 标志用于获取着色器调试信息。该标志可以提升调试体验，
    // 但仍然允许着色器进行优化操作
    dwShaderFlags |= D3DCOMPILE_DEBUG;

    // 在Debug环境下禁用优化以避免出现一些不合理的情况
    dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    // Step 3
    ID3DBlob *pErrorBlob = nullptr;
    hr = D3DCompileFromFile(hlslFileName, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, shaderModel,
                            dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
    if (FAILED(hr))
    {
        if (pErrorBlob)
        {
            OutputDebugStringA((char *)pErrorBlob->GetBufferPointer());
        }
        SAFE_RELEASE(pErrorBlob);
        return hr;
    }

    // Step 4
    if (csoFileNameInOut)
    {
        hr = D3DWriteBlobToFile(*ppBlobOut, csoFileNameInOut, TRUE);
    }

    return hr;
}
