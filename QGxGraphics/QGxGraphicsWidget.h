#ifndef QGX_GRAPHICSWIDGET_H
#define QGX_GRAPHICSWIDGET_H

#include <QtWidgets/QWidget>

namespace QGx
{

class GraphicsDevice;

class GraphicsWidget : public QWidget
{
    Q_OBJECT
    
public:
    GraphicsWidget(GraphicsDevice &device, QWidget *parent = nullptr);
    virtual ~GraphicsWidget() override;

protected:
    GraphicsDevice &device();

    virtual QPaintEngine *paintEngine() const override;
    virtual void resizeEvent(QResizeEvent *event) override;

private:
    friend class GraphicsDevice;

    GraphicsDevice *d;
};

}

#endif // QGX_GRAPHICSWIDGET_H
