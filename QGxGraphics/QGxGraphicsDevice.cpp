#include "QGxGraphics/QGxGraphicsDevice.h"

#include "QGxGraphics/QGxGraphicsWidget.h"

#include "internal/qgx_common.h"

#include <QtCore/QSize>

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
    Cache() : direct3d(nullptr), device(nullptr), back(nullptr), reset(true) { }

    IDirect3D9 *direct3d;
    IDirect3DDevice9 *device;
    IDirect3DSurface9 *back;
    std::unordered_map<QGx::GraphicsWidget*, SwapChain*> widgets;
    bool reset;
};

void createBasicDevice(Cache &cache, HWND hw, const QSize &size)
{
    D3DCAPS9 caps;
    HRESULT r = cache.direct3d->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);
    if(FAILED(r))
    {
        qgx_detail_com_ptr_release(cache.direct3d);
        throw std::runtime_error("unable to get device caps");
    }

    auto params = createParams(hw, size);

    r = cache.direct3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, NULL, vertexProcessing(caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT), &params, &(cache.device));
    if(FAILED(r))
    {
        qgx_detail_com_ptr_release(cache.device);
        qgx_detail_com_ptr_release(cache.direct3d);

        throw std::runtime_error("unable to create device");
    }

    setGlobalDeviceSettings(cache.device);
}

void releaseDevice(Cache &cache)
{
    for(auto widget: cache.widgets)
    {
        delete cache.widgets[widget.first];
    }

    qgx_detail_com_ptr_release(cache.device);
}

bool contains(Cache &cache, QGx::GraphicsWidget *widget)
{
    return cache.widgets.find(widget) != cache.widgets.end();
}

}

QGx::GraphicsDevice::GraphicsDevice()
{
    cache.alloc<Cache>();

    cache.get<Cache>().direct3d = Direct3DCreate9(D3D_SDK_VERSION);
    if(!cache.get<Cache>().direct3d)
    {
        throw std::runtime_error("unable to create graphics device");
    }

    reset();
}

QGx::GraphicsDevice::~GraphicsDevice()
{
    for(auto widget: cache.get<Cache>().widgets)
    {
        widget.first->d = 0;
        unregisterWidget(widget.first);
    }

    releaseDevice(cache.get<Cache>());

    qgx_detail_com_ptr_release(cache.get<Cache>().direct3d);
}

void QGx::GraphicsDevice::reset()
{
    if(cache.get<Cache>().widgets.empty())
    {
        releaseDevice(cache.get<Cache>());

        cache.get<Cache>().reset = false;
        createBasicDevice(cache.get<Cache>(), NULL, QSize(0, 0));
        
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
        createBasicDevice(cache.get<Cache>(), reinterpret_cast<HWND>(widgets.front()->winId()), widgets.front()->size());
    }
    else
    {
        auto params = createParams(reinterpret_cast<HWND>(widgets.front()->winId()), widgets.front()->size());

        HRESULT r = cache.get<Cache>().device->Reset(&params);
        if(FAILED(r))
        {
            releaseDevice(cache.get<Cache>());
            throw std::runtime_error("unable to reset graphics device");
        }

        setGlobalDeviceSettings(cache.get<Cache>().device);
    }

    HRESULT r = cache.get<Cache>().device->GetSwapChain(0, &(cache.get<Cache>().widgets[widgets[0]]->p));
    if(FAILED(r))
    {
        releaseDevice(cache.get<Cache>());
        throw std::runtime_error("unable to create swap chain");
    }

    for(unsigned i = 1; i < widgets.size(); ++i)
    {
        auto params = createParams(reinterpret_cast<HWND>(widgets[i]->winId()), widgets[i]->size());

        r = cache.get<Cache>().device->CreateAdditionalSwapChain(&params, &(cache.get<Cache>().widgets[widgets[i]]->p));
        if(FAILED(r))
        {
            releaseDevice(cache.get<Cache>());
            throw std::runtime_error("unable to create swap chain");
        }

        r = cache.get<Cache>().device->CreateDepthStencilSurface(widgets[i]->size().width(), widgets[i]->size().height(), D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0, false, &(cache.get<Cache>().widgets[widgets[i]]->ds), NULL);
        if(FAILED(r))
        {
            releaseDevice(cache.get<Cache>());
            throw std::runtime_error("unable to create depth stencil surface");
        }
    }

    cache.get<Cache>().reset = false;
}

void QGx::GraphicsDevice::begin(GraphicsWidget *widget)
{
    cache.get<Cache>().widgets[widget]->p->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &cache.get<Cache>().back);
    cache.get<Cache>().device->SetRenderTarget(0, cache.get<Cache>().back);

    cache.get<Cache>().device->SetDepthStencilSurface(cache.get<Cache>().widgets[widget]->ds);

    cache.get<Cache>().device->BeginScene();
}

void QGx::GraphicsDevice::end(GraphicsWidget *widget)
{
    cache.get<Cache>().device->EndScene();
    cache.get<Cache>().widgets[widget]->p->Present(NULL, NULL, reinterpret_cast<HWND>(widget->winId()), NULL, 0);

    cache.get<Cache>().device->SetDepthStencilSurface(nullptr);

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
