#include "GameApp.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE prevInstance, _In_ LPSTR cmdLine, _In_ int showCmd) {
    // 这些参数不使用
    UNREFERENCED_PARAMETER(prevInstance);
    UNREFERENCED_PARAMETER(cmdLine);
    UNREFERENCED_PARAMETER(showCmd);
    // 允许在Debug版本进行运行时内存分配和泄漏检测
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    // 驱动类型数组
    D3D_DRIVER_TYPE driverTypes[] = {
        D3D_DRIVER_TYPE_HARDWARE,  // 硬件驱动
        D3D_DRIVER_TYPE_WARP,      // WARP驱动
        D3D_DRIVER_TYPE_REFERENCE, // 软件驱动
    };
    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    GameApp theApp(hInstance, L"DirectX11 Initialization", 1280, 720);

    if (!theApp.Init())
        return 0;

    return theApp.Run();
}
