#if !defined(PLATFORM_WIN32_H)
#define PLATFORM_WIN32_H

#define COBJMACROS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <combaseapi.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <time.h>

#define NK_D3D12_IMPLEMENTATION
#include "thirdparty/nuklear/nuklear_d3d12.h"

struct PlatformState {
    WNDCLASSW wc;
};

/* DXGI & Window related device objects */
static IDXGIFactory2 *dxgi_factory;
static IDXGISwapChain1 *swap_chain;
static ID3D12DescriptorHeap *rtv_descriptor_heap;
static D3D12_CPU_DESCRIPTOR_HANDLE rtv_handles[2];
static ID3D12Resource *rtv_buffers[2];
static UINT rtv_desc_increment;
static UINT rtv_index;
/* DirectX common device objects */
static ID3D12Device *device;
static ID3D12CommandQueue *command_queue;
static ID3D12Fence *queue_fence;
static UINT64 fence_value;
static ID3D12CommandAllocator *command_allocator;
static ID3D12GraphicsCommandList *command_list;

static void get_swap_chain_buffers()
{
    HRESULT hr;
    D3D12_CPU_DESCRIPTOR_HANDLE descriptor_handle;

    /* Get resource objects from swap chain */
    hr = IDXGISwapChain1_GetBuffer(swap_chain, 0, &IID_ID3D12Resource, &rtv_buffers[0]);
    assert(SUCCEEDED(hr));
    hr = IDXGISwapChain1_GetBuffer(swap_chain, 1, &IID_ID3D12Resource, &rtv_buffers[1]);
    assert(SUCCEEDED(hr));

    /* Recreate render target views */
    ID3D12DescriptorHeap_GetCPUDescriptorHandleForHeapStart(rtv_descriptor_heap, &descriptor_handle);
    ID3D12Device_CreateRenderTargetView(device, rtv_buffers[0], NULL, descriptor_handle);
    rtv_handles[0] = descriptor_handle;
    descriptor_handle.ptr += rtv_desc_increment;
    ID3D12Device_CreateRenderTargetView(device, rtv_buffers[1], NULL, descriptor_handle);
    rtv_handles[1] = descriptor_handle;
}

static void signal_and_wait()
{
    HRESULT hr;

    /* Signal fence when execution finishes */
    hr = ID3D12CommandQueue_Signal(command_queue, queue_fence, ++fence_value);
    assert(SUCCEEDED(hr));

    /* Wait for queue to finish */
    while(ID3D12Fence_GetCompletedValue(queue_fence) != fence_value)
    {
      SwitchToThread(); /* Allow windows to do other work */
    }
}

static void execute_commands()
{
    /* Prepare command list for execution */
    ID3D12GraphicsCommandList_Close(command_list);

    /* Execute on command queue */
    ID3D12CommandList* cmd_lists[] = { (ID3D12CommandList*)command_list};
    ID3D12CommandQueue_ExecuteCommandLists(command_queue, 1, cmd_lists);

    /* Wait for execution */
    signal_and_wait();

    /* Reset command allocator and list */
    ID3D12CommandAllocator_Reset(command_allocator);
    ID3D12GraphicsCommandList_Reset(command_list, command_allocator, NULL);
}

static void
set_swap_chain_size(int width, int height)
{
    HRESULT hr;

    /* Wait for pending work */
    signal_and_wait();
    signal_and_wait(); /* Two times because we have two buffers in flight */

    /* Release all open refereces to the buffers */
    ID3D12Resource_Release(rtv_buffers[0]);
    ID3D12Resource_Release(rtv_buffers[1]);

    /* DXGI can now perform resizing */
    hr = IDXGISwapChain1_ResizeBuffers(swap_chain, 2, width, height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
    assert(SUCCEEDED(hr));

    /* Get references for the new resized buffers */
    get_swap_chain_buffers();

    /* Reset RTV index */
    rtv_index = 0;
}



static LRESULT CALLBACK WindowProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_SIZE:
        if (swap_chain)
        {
            int width = LOWORD(lparam);
            int height = HIWORD(lparam);
            set_swap_chain_size(width, height);
            nk_d3d12_resize(width, height);
        }
        break;
    }

    if (nk_d3d12_handle_event(wnd, msg, wparam, lparam))
        return 0;

    return DefWindowProcW(wnd, msg, wparam, lparam);
}

LPWSTR cStringToLPWSTR(const char * input) {
    LPWSTR output = NULL;
    int lenA = lstrlenA(input);
    int lenW = MultiByteToWideChar(CP_ACP, 0, input, lenA, output, 0);
    return output;
}

// platform implementation
struct nk_context* platformCreateWindow(const char *title, int windowWidth, int windowHeight) {
    struct nk_context *ctx;
    WNDCLASSW wc;
    RECT rect = { 0, 0, windowWidth, windowHeight };
    DWORD style = WS_OVERLAPPEDWINDOW;
    DWORD exstyle = WS_EX_APPWINDOW;
    HWND wnd;
    int running = 1;
    HRESULT hr;

    D3D12_COMMAND_QUEUE_DESC command_queue_desc;
    DXGI_SWAP_CHAIN_DESC1 swap_chain_desc;
    D3D12_DESCRIPTOR_HEAP_DESC rtv_desc_heap_desc;

    /* Win32 */
    memset(&wc, 0, sizeof(wc));
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandleW(0);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"NuklearWindowClass";
    RegisterClassW(&wc);

    AdjustWindowRectEx(&rect, style, FALSE, exstyle);
    
    wnd = CreateWindowExW(exstyle, wc.lpszClassName, cStringToLPWSTR(title),
        style | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top,
        NULL, NULL, wc.hInstance, NULL);

    /* D3D12 setup */
    /* Create default Device */
    hr = D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_11_0, &IID_ID3D12Device, &device);
    assert(SUCCEEDED(hr));
    /* Create a command queue */
    command_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    command_queue_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    command_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    command_queue_desc.NodeMask = 0;
    hr = ID3D12Device_CreateCommandQueue(device, &command_queue_desc, &IID_ID3D12CommandQueue, &command_queue);
    assert(SUCCEEDED(hr));
    /* Create a fence for command queue executions */
    fence_value = 0;
    hr = ID3D12Device_CreateFence(device, fence_value, D3D12_FENCE_FLAG_NONE, &IID_ID3D12Fence, &queue_fence);
    assert(SUCCEEDED(hr));
    /* Create a command allocator */
    hr = ID3D12Device_CreateCommandAllocator(device, D3D12_COMMAND_LIST_TYPE_DIRECT, &IID_ID3D12CommandAllocator, &command_allocator);
    assert(SUCCEEDED(hr));
    /* Create a command list that will use our allocator */
    hr = ID3D12Device_CreateCommandList(device, 0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator, NULL,  &IID_ID3D12GraphicsCommandList1, &command_list);
    assert(SUCCEEDED(hr));

    /* DXGI Setup (Swap chain & resources) */
    /* Create a descriptor heap for the back buffers */
    rtv_desc_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtv_desc_heap_desc.NumDescriptors = 2;
    rtv_desc_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtv_desc_heap_desc.NodeMask = 0;
    hr = ID3D12Device_CreateDescriptorHeap(device, &rtv_desc_heap_desc, &IID_ID3D12DescriptorHeap, &rtv_descriptor_heap);
    assert(SUCCEEDED(hr));
    /* Get descriptor increment */
    rtv_desc_increment = ID3D12Device_GetDescriptorHandleIncrementSize(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    /* Get the DXGI factory */
    hr = CreateDXGIFactory1(&IID_IDXGIFactory2, &dxgi_factory);
    assert(SUCCEEDED(hr));
    /* Create the swap chain */
    swap_chain_desc.Width = windowWidth;
    swap_chain_desc.Height = windowHeight;
    swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_desc.Stereo = 0;
    swap_chain_desc.SampleDesc.Count = 1;
    swap_chain_desc.SampleDesc.Quality = 0;
    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.BufferCount = 2;
    swap_chain_desc.Scaling = DXGI_SCALING_STRETCH;
    swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL ;
    swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    hr = IDXGIFactory2_CreateSwapChainForHwnd(dxgi_factory, (IUnknown*)command_queue, wnd, &swap_chain_desc, NULL, NULL, &swap_chain);
    assert(SUCCEEDED(hr));
    get_swap_chain_buffers();

    /* GUI */
    ctx = nk_d3d12_init(device, windowWidth, windowHeight, MAX_VERTEX_BUFFER, MAX_INDEX_BUFFER, USER_TEXTURES);

     /* Load Fonts: if none of these are loaded a default font will be used  */
    /* Load Cursor: if you uncomment cursor loading please hide the cursor */
    {
    struct nk_font_atlas *atlas;
    nk_d3d12_font_stash_begin(&atlas);
    /*struct nk_font *droid = nk_font_atlas_add_from_file(atlas, "../../extra_font/DroidSans.ttf", 14, 0);*/
    /*struct nk_font *robot = nk_font_atlas_add_from_file(atlas, "../../extra_font/Roboto-Regular.ttf", 14, 0);*/
    /*struct nk_font *future = nk_font_atlas_add_from_file(atlas, "../../extra_font/kenvector_future_thin.ttf", 13, 0);*/
    /*struct nk_font *clean = nk_font_atlas_add_from_file(atlas, "../../extra_font/ProggyClean.ttf", 12, 0);*/
    /*struct nk_font *tiny = nk_font_atlas_add_from_file(atlas, "../../extra_font/ProggyTiny.ttf", 10, 0);*/
    /*struct nk_font *cousine = nk_font_atlas_add_from_file(atlas, "../../extra_font/Cousine-Regular.ttf", 13, 0);*/
    nk_d3d12_font_stash_end(command_list);
    /*nk_style_load_all_cursors(ctx, atlas->cursors);*/
    /*nk_style_set_font(ctx, &droid->handle)*/;
    }

    /* Execute the command list to make sure all texture (font) data has been uploaded */
    execute_commands();
    /* Now we can cleanup all resources consumed by font stashing that are no longer used */
    nk_d3d12_font_stash_cleanup();

    return ctx;
}

// platform implementation 
int platformRender(struct nk_colorf bg) {
    HRESULT hr;
    /* Set rtv resource state */
        D3D12_RESOURCE_BARRIER resource_barrier;
        resource_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        resource_barrier.Transition.pResource = rtv_buffers[rtv_index];
        resource_barrier.Transition.Subresource = 0;
        resource_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        resource_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        resource_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        ID3D12GraphicsCommandList_ResourceBarrier(command_list, 1, &resource_barrier);

        /* Clear and set the rtv */
        ID3D12GraphicsCommandList_ClearRenderTargetView(command_list, rtv_handles[rtv_index], &bg.r, 0, NULL);
        ID3D12GraphicsCommandList_OMSetRenderTargets(command_list, 1, &rtv_handles[rtv_index], FALSE, NULL);

        /* Draw */
        nk_d3d12_render(command_list, NK_ANTI_ALIASING_ON);

        /* Bring the rtv resource back to present state */
        resource_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        resource_barrier.Transition.pResource = rtv_buffers[rtv_index];
        resource_barrier.Transition.Subresource = 0;
        resource_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        resource_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        resource_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        ID3D12GraphicsCommandList_ResourceBarrier(command_list, 1, &resource_barrier);

        /* Execute command list and wait */
        execute_commands();

        /* Present frame */
        hr = IDXGISwapChain2_Present(swap_chain, 1, 0);
        rtv_index = (rtv_index + 1) % 2;
        if (hr == DXGI_ERROR_DEVICE_RESET || hr == DXGI_ERROR_DEVICE_REMOVED) {
            /* to recover from this, you'll need to recreate device and all the resources */
            MessageBoxW(NULL, L"D3D12 device is lost or removed!", L"Error", 0);
            return -1;
        } else if (hr == DXGI_STATUS_OCCLUDED) {
            /* window is not visible, so vsync won't work. Let's sleep a bit to reduce CPU usage */
            Sleep(10);
        }
        assert(SUCCEEDED(hr));
        return 0;
}


void platformProcessInput(ApplicationState *applicationState) {
    for(int i = 0; i < INPUT_EVENT_COUNT; i++) {
        applicationState->inputs[i] = 0;
    }

    MSG msg;
    while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                applicationState->inputs[INPUT_EVENT_QUIT] = 1;
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
    }
}
#endif // PLATFORM_WIN32_H
