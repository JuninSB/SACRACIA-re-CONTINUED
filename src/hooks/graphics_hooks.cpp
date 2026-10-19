#include <src/pch.h>

#include <src/cheat.h>
#include <src/overlay/overlay.h>

namespace Cheat
{
    namespace
    {
        struct GpuTimestampSlot
        {
            ID3D11Query* Disjoint = nullptr;
            ID3D11Query* Start = nullptr;
            ID3D11Query* End = nullptr;
            bool Active = false;
            bool Pending = false;
        };
        static GpuTimestampSlot s_GpuTimestampSlots[4]{};
        static int s_CurrentGpuTimestampSlot = -1;
        static bool s_GpuTimestampUnavailable = false;

        static bool EnsureGpuTimestampQueries(GpuTimestampSlot& slot)
        {
            if (slot.Disjoint && slot.Start && slot.End) return true;
            if (s_GpuTimestampUnavailable || !G || !G->Device) return false;
            D3D11_QUERY_DESC desc{};
            desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
            if (FAILED(G->Device->CreateQuery(&desc, &slot.Disjoint))) { s_GpuTimestampUnavailable = true; return false; }
            desc.Query = D3D11_QUERY_TIMESTAMP;
            if (FAILED(G->Device->CreateQuery(&desc, &slot.Start)) || FAILED(G->Device->CreateQuery(&desc, &slot.End)))
            {
                if (slot.Disjoint) { slot.Disjoint->Release(); slot.Disjoint = nullptr; }
                if (slot.Start) { slot.Start->Release(); slot.Start = nullptr; }
                if (slot.End) { slot.End->Release(); slot.End = nullptr; }
                s_GpuTimestampUnavailable = true;
                return false;
            }
            return true;
        }
    }

    static HRESULT WINAPI Hooked_Present(IDXGISwapChain* swapChain, UINT a1, UINT a2);
    static HRESULT WINAPI Hooked_ResizeBuffers(IDXGISwapChain* swapChain, UINT a1, UINT a2, UINT a3, DXGI_FORMAT a4, UINT a5);
    static void CreateRenderTarget(IDXGISwapChain* swapChain);

    void HookGraphics()
    {
        Hax::LogDebug(G->Logger, L"Hooking graphics...");

        WNDCLASSEX dummyWindow =
        {
            .cbSize = sizeof(WNDCLASSEX),
            .style = CS_HREDRAW | CS_VREDRAW,
            .lpfnWndProc = ::DefWindowProcW,
            .cbClsExtra = 0,
            .cbWndExtra = 0,
            .hInstance = ::GetModuleHandleW(NULL),
            .hIcon = NULL,
            .hCursor = NULL,
            .hbrBackground = NULL,
            .lpszMenuName = nullptr,
            .lpszClassName = L"Dummy window",
            .hIconSm = NULL
        };
        ::RegisterClassEx(&dummyWindow);
        HWND hWnd = ::CreateWindow(dummyWindow.lpszClassName, L"Dummy window", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, nullptr, nullptr, dummyWindow.hInstance, nullptr);

        DXGI_SWAP_CHAIN_DESC swapChainDesc =
        {
            .BufferDesc = {.Format = DXGI_FORMAT_R8G8B8A8_UNORM},
            .SampleDesc = {.Count = 1},
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = 2,
            .OutputWindow = hWnd,
            .Windowed = TRUE
        };

        const D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };

        ID3D11Device* device = nullptr;
        ID3D11DeviceContext* context = nullptr;
        IDXGISwapChain* swapChain = nullptr;

        HRESULT result = ::D3D11CreateDeviceAndSwapChain
        (
            NULL, D3D_DRIVER_TYPE_NULL, NULL, 0, featureLevels, 2,
            D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &device, nullptr, &context
        );
        HAX_PANIC(result == S_OK, &G->Logger, L"D3D11CreateDeviceAndSwapChain %d", result);

        void** vtable = *(void***)(swapChain);
        {
            DWORD oldProtect;
            BOOL ok = ::VirtualProtect(&vtable[8], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
            HAX_PANIC(ok == TRUE, &G->Logger, L"Present hook not installed");
            G->PresentOrig = vtable[8];
            vtable[8] = Hooked_Present;
            VirtualProtect(&vtable[8], sizeof(void*), oldProtect, &oldProtect);
        }
        {
            DWORD oldProtect;
            BOOL ok = VirtualProtect(&vtable[13], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
            HAX_PANIC(ok == TRUE, &G->Logger, L"ResizeBuffers hook not installed");
            G->ResizeBuffersOrig = vtable[13];
            vtable[13] = Hooked_ResizeBuffers;
            VirtualProtect(&vtable[13], sizeof(void*), oldProtect, &oldProtect);
        }

        swapChain->Release();
        context->Release();
        device->Release();

        ::DestroyWindow(hWnd);
        ::UnregisterClass(dummyWindow.lpszClassName, dummyWindow.hInstance);
    }

    void BenchmarkGpuFrameBegin()
    {
        if (!IsBenchmarkCaptureEnabled() || s_GpuTimestampUnavailable || !G || !G->DeviceContext || s_CurrentGpuTimestampSlot >= 0)
            return;
        for (int i = 0; i < (int)_countof(s_GpuTimestampSlots); ++i)
        {
            auto& slot = s_GpuTimestampSlots[i];
            if (slot.Active || slot.Pending || !EnsureGpuTimestampQueries(slot)) continue;
            G->DeviceContext->Begin(slot.Disjoint);
            G->DeviceContext->End(slot.Start);
            slot.Active = true;
            s_CurrentGpuTimestampSlot = i;
            return;
        }
    }

    void BenchmarkGpuFrameEndAndPoll()
    {
        if (!G || !G->DeviceContext) return;
        if (s_CurrentGpuTimestampSlot >= 0)
        {
            auto& slot = s_GpuTimestampSlots[s_CurrentGpuTimestampSlot];
            G->DeviceContext->End(slot.End);
            G->DeviceContext->End(slot.Disjoint);
            slot.Active = false;
            slot.Pending = true;
            s_CurrentGpuTimestampSlot = -1;
        }
        for (auto& slot : s_GpuTimestampSlots)
        {
            if (!slot.Pending) continue;
            D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjoint{};
            UINT64 start = 0, end = 0;
            const HRESULT disjointReady = G->DeviceContext->GetData(slot.Disjoint, &disjoint, sizeof(disjoint), D3D11_ASYNC_GETDATA_DONOTFLUSH);
            if (disjointReady != S_OK) continue;
            const HRESULT startReady = G->DeviceContext->GetData(slot.Start, &start, sizeof(start), D3D11_ASYNC_GETDATA_DONOTFLUSH);
            const HRESULT endReady = G->DeviceContext->GetData(slot.End, &end, sizeof(end), D3D11_ASYNC_GETDATA_DONOTFLUSH);
            if (startReady == S_OK && endReady == S_OK && !disjoint.Disjoint && disjoint.Frequency && end >= start)
                BenchmarkRecordGpuTimeUs((unsigned long long)((end - start) * 1000000ULL / disjoint.Frequency));
            slot.Pending = false;
        }
    }

    static HRESULT WINAPI Hooked_Present(IDXGISwapChain* swapChain, UINT a1, UINT a2)
    {
        BenchmarkGpuFrameEndAndPoll();
        LARGE_INTEGER benchStart{}, overlayEnd{}, presentEnd{};
        const bool bench = IsBenchmarkCaptureEnabled();
        if (bench) ::QueryPerformanceCounter(&benchStart);
        if (!Hax::Gui::Initialized())
        {
            swapChain->GetDevice(IID_PPV_ARGS(&G->Device));
            G->Device->GetImmediateContext(&G->DeviceContext);

            DXGI_SWAP_CHAIN_DESC sd;
            swapChain->GetDesc(&sd);
            G->GameWndHandle = sd.OutputWindow;

            Hax::Gui::Initialize((Hax::Handle)sd.OutputWindow, G->Device);
            PrepareMenu();
            ToggleMenuVisibility();
            G->UvmThread = Hax::Unity::AttachThread();
        }

        if (G->RenderTarget == nullptr)
            CreateRenderTarget(swapChain);

        //!
        G->NoGrabMaxTime &= !G->IsClient;
        G->Unbreakable &= !G->IsClient;
        G->EasyGrab &= !G->IsClient;
        G->InfBattery &= !G->IsClient;

        try
        {
            ID3D11RenderTargetView* gameRTV = nullptr;
            ID3D11DepthStencilView* backupDSV = nullptr;
            G->DeviceContext->OMGetRenderTargets(1, &gameRTV, &backupDSV);
            G->DeviceContext->OMSetRenderTargets(1, &G->RenderTarget, nullptr);

            Hax::Gui::BeginFrame();
            RenderOverlay();
            Hax::Gui::EndFrame();

            G->DeviceContext->OMSetRenderTargets(1, &gameRTV, backupDSV);
        }
        catch (System::Exception& ex)
        {
            System::String message = ex.Message();
            Hax::LogError(G->Logger, L"%d: %s", __LINE__, message != null ? message.ToString().GetRawStringData() : L"Exception without message");
        }

        if (bench) { ::QueryPerformanceCounter(&overlayEnd); BenchmarkRecordSection(7, (unsigned long long)(overlayEnd.QuadPart - benchStart.QuadPart)); }
        const HRESULT result = ((HRESULT(WINAPI*)(IDXGISwapChain*, UINT, UINT))G->PresentOrig)(swapChain, a1, a2);
        if (bench) { ::QueryPerformanceCounter(&presentEnd); BenchmarkRecordSection(8, (unsigned long long)(presentEnd.QuadPart - overlayEnd.QuadPart)); }
        return result;
    }

    static HRESULT WINAPI Hooked_ResizeBuffers(IDXGISwapChain* swapChain, UINT a1, UINT a2, UINT a3, DXGI_FORMAT a4, UINT a5)
    {
        if (G->RenderTarget != nullptr)
        {
            G->RenderTarget->Release();
            G->RenderTarget = nullptr;
        }

        return ((HRESULT(WINAPI*)(IDXGISwapChain*,UINT,UINT,UINT,DXGI_FORMAT,UINT))G->ResizeBuffersOrig)(swapChain, a1, a2, a3, a4, a5);
    }

    static void CreateRenderTarget(IDXGISwapChain* swapChain)
    {
        ID3D11Texture2D* backBuffer = nullptr;
        swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
        if (backBuffer != nullptr && G->Device != nullptr)
        {
            D3D11_RENDER_TARGET_VIEW_DESC desc = {.Format = DXGI_FORMAT_R8G8B8A8_UNORM, .ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D};
            G->Device->CreateRenderTargetView(backBuffer, &desc, &G->RenderTarget);
            backBuffer->Release();
        }
    }
}
