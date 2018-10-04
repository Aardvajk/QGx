#include "QGxGraphics/QGxGraphicsDevice.h"

#include "QGxGraphics/QGxGraphicsWidget.h"

#include "internal/qgx_common.h"

#include <QtCore/QSize>
#include <QtCore/QDebug>

#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <d3d9.h>

namespace
{

D3DPRESENT_PARAMETERS createParams(HWND hw, const QSize &size)
{
    D3DPRESENT_PARAMETERS p;
    ZeroMemory(&p, sizeof(D3DPRESENT_PARAMETERS));

    p.BackBufferWidth = (size.width() > 0 ? size.width() : 1);
    p.BackBufferHeight = (size.height() > 0 ? size.height() : 1);

    p.BackBufferFormat = D3DFMT_A8R8G8B8;
    p.BackBufferCount = 1;

    p.MultiSampleType = D3DMULTISAMPLE_NONE;

    p.SwapEffect = D3DSWAPEFFECT_DISCARD;
    p.hDeviceWindow = hw;
    p.Windowed = true;
    p.EnableAutoDepthStencil = false;
    p.AutoDepthStencilFormat = D3DFMT_D24S8;
    p.Flags = 0;

    p.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    p.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

    return p;
}

void setGlobalDeviceSettings(IDirect3DDevice9 *device)
{
    DWORD filter = D3DTEXF_LINEAR;

    device->SetSamplerState(0, D3DSAMP_MAGFILTER, filter);
    device->SetSamplerState(0, D3DSAMP_MINFILTER, filter);

    device->SetSamplerState(0, D3DSAMP_MIPFILTER, filter);

    device->SetSamplerState(1, D3DSAMP_MAGFILTER, filter);
    device->SetSamplerState(1, D3DSAMP_MINFILTER, filter);

    device->SetSamplerState(1, D3DSAMP_MIPFILTER, filter);

    device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

    device->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    device->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

    device->SetRenderState(D3DRS_ZENABLE, true);
    device->SetRenderState(D3DRS_LIGHTING, false);

    device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
    device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

    device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
}

DWORD vertexProcessing(bool hardware)
{
    return hardware ? D3DCREATE_HARDWARE_VERTEXPROCESSING : D3DCREATE_SOFTWARE_VERTEXPROCESSING;
}

class SwapChain
{
public:
    SwapChain() : p(0), ds(0) { }
    ~SwapChain(){ release(); }

    void release();

    IDirect3DSwapChain9 *p;
    IDirect3DSurface9 *ds;
};

void SwapChain::release()
{
    qgx_detail_com_ptr_release(p);
    qgx_detail_com_ptr_release(ds);
}

class Cache
{
public:
    Cache() : back(nullptr), reset(true) { }

    IDirect3DSurface9 *back;
    std::unordered_map<QGx::GraphicsWidget*, SwapChain*> widgets;
    bool reset;
};

void createBasicDevice(IDirect3D9 *direct3d, HWND hw, const QSize &size, IDirect3DDevice9 *&device)
{
    D3DCAPS9 caps;
    HRESULT r = direct3d->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);
    if(FAILED(r))
    {
        qgx_detail_com_ptr_release(direct3d);
        throw std::runtime_error("unable to get device caps");
    }

    auto params = createParams(hw, size);

    r = direct3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, NULL, vertexProcessing(caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT), &params, &(device));
    if(FAILED(r))
    {
        qgx_detail_com_ptr_release(device);
        qgx_detail_com_ptr_release(direct3d);

        throw std::runtime_error("unable to create device");
    }

    setGlobalDeviceSettings(device);
}

void releaseDevice(Cache &cache, IDirect3DDevice9 *&device)
{
    for(auto widget: cache.widgets)
    {
        delete cache.widgets[widget.first];
    }

    qgx_detail_com_ptr_release(device);
}

bool contains(Cache &cache, QGx::GraphicsWidget *widget)
{
    return cache.widgets.find(widget) != cache.widgets.end();
}

}

QGx::GraphicsDevice::GraphicsDevice()
{
    cache.alloc<Cache>();

    direct3d = Direct3DCreate9(D3D_SDK_VERSION);
    if(!direct3d)
    {
        throw std::runtime_error("unable to create graphics device");
    }

    reset();
}

QGx::GraphicsDevice::~GraphicsDevice()
{
    for(auto widget: cache.get<Cache>().widgets)
    {
        delete cache.get<Cache>().widgets[widget.first];
    }

    cache.get<Cache>().widgets.clear();

    releaseDevice(cache.get<Cache>(), device);
    qgx_detail_com_ptr_release(direct3d);
}

void QGx::GraphicsDevice::reset()
{
    if(cache.get<Cache>().widgets.empty())
    {
        releaseDevice(cache.get<Cache>(), device);

        cache.get<Cache>().reset = false;
        createBasicDevice(direct3d, NULL, QSize(0, 0), device);
        
        return;
    }

    for(auto widget: cache.get<Cache>().widgets)
    {
        widget.second->release();
    }

    clearCache();

    std::vector<GraphicsWidget*> widgets;
    for(auto widget: cache.get<Cache>().widgets)
    {
        widgets.push_back(widget.first);
    }    

    if(!device)
    {
        createBasicDevice(direct3d, reinterpret_cast<HWND>(widgets.front()->winId()), widgets.front()->size(), device);
    }
    else
    {
        auto params = createParams(reinterpret_cast<HWND>(widgets.front()->winId()), widgets.front()->size());

        HRESULT r = device->Reset(&params);
        if(FAILED(r))
        {
            releaseDevice(cache.get<Cache>(), device);
            throw std::runtime_error("unable to reset graphics device");
        }

        setGlobalDeviceSettings(device);
    }

    HRESULT r = device->GetSwapChain(0, &(cache.get<Cache>().widgets[widgets[0]]->p));
    if(FAILED(r))
    {
        releaseDevice(cache.get<Cache>(), device);
        throw std::runtime_error("unable to create swap chain");
    }

    r = device->CreateDepthStencilSurface(widgets[0]->size().width(), widgets[0]->size().height(), D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0, false, &(cache.get<Cache>().widgets[widgets[0]]->ds), NULL);
    if(FAILED(r))
    {
        releaseDevice(cache.get<Cache>(), device);
        throw std::runtime_error("unable to create depth stencil surface");
    }

    for(unsigned i = 1; i < widgets.size(); ++i)
    {
        auto params = createParams(reinterpret_cast<HWND>(widgets[i]->winId()), widgets[i]->size());

        r = device->CreateAdditionalSwapChain(&params, &(cache.get<Cache>().widgets[widgets[i]]->p));
        if(FAILED(r))
        {
            releaseDevice(cache.get<Cache>(), device);
            throw std::runtime_error("unable to create swap chain");
        }

        r = device->CreateDepthStencilSurface(widgets[i]->size().width(), widgets[i]->size().height(), D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0, false, &(cache.get<Cache>().widgets[widgets[i]]->ds), NULL);
        if(FAILED(r))
        {
            releaseDevice(cache.get<Cache>(), device);
            throw std::runtime_error("unable to create depth stencil surface");
        }
    }

    cache.get<Cache>().reset = false;
}

void QGx::GraphicsDevice::begin(GraphicsWidget *widget)
{
    cache.get<Cache>().widgets[widget]->p->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &cache.get<Cache>().back);
    device->SetRenderTarget(0, cache.get<Cache>().back);

    device->SetDepthStencilSurface(cache.get<Cache>().widgets[widget]->ds);

    device->BeginScene();
}

void QGx::GraphicsDevice::end(GraphicsWidget *widget)
{
    device->EndScene();
    cache.get<Cache>().widgets[widget]->p->Present(NULL, NULL, reinterpret_cast<HWND>(widget->winId()), NULL, 0);

    device->SetDepthStencilSurface(nullptr);

    qgx_detail_com_ptr_release(cache.get<Cache>().back);
}

bool QGx::GraphicsDevice::needsResetting() const
{
    return cache.get<Cache>().reset;
}

void QGx::GraphicsDevice::scheduleReset()
{
    cache.get<Cache>().reset = true;
}

void QGx::GraphicsDevice::registerWidget(GraphicsWidget *widget)
{
    if(!contains(cache.get<Cache>(), widget))
    {
        cache.get<Cache>().widgets[widget] = new SwapChain();
        cache.get<Cache>().reset = true;
    }
}

void QGx::GraphicsDevice::unregisterWidget(GraphicsWidget *widget)
{
    if(contains(cache.get<Cache>(), widget))
    {
        delete cache.get<Cache>().widgets[widget];
        cache.get<Cache>().widgets.erase(widget);

        cache.get<Cache>().reset = true;
    }
}
