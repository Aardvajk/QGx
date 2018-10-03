#ifndef QGX_GRAPHICSDEVICE_H
#define QGX_GRAPHICSDEVICE_H

#include <GxGraphics/GxGraphicsDevice.h>

#include <pcx/aligned_store.h>

namespace QGx
{

class GraphicsWidget;

class GraphicsDevice : public Gx::GraphicsDevice
{
public:
    GraphicsDevice();
    virtual ~GraphicsDevice() override;
    
    void reset();

    void begin(GraphicsWidget *widget);
    void end(GraphicsWidget *widget);

    bool needsResetting() const;

private:
    friend class GraphicsWidget;

    void scheduleReset();

    void registerWidget(GraphicsWidget *widget);
    void unregisterWidget(GraphicsWidget *widget);

    pcx::aligned_store<88> cache;
};

}

#endif // QGX_GRAPHICSDEVICE_H
