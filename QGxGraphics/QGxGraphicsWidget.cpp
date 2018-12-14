#include "QGxGraphics/QGxGraphicsWidget.h"

#include "GxMaths/GxSize.h"

#include "QGxGraphics/QGxGraphicsDevice.h"

QGx::GraphicsWidget::GraphicsWidget(GraphicsDevice &device, QWidget *parent) : QWidget(parent), d(&device)
{
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);

    setMinimumSize(64, 64);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    
    d->registerWidget(this);
}

QGx::GraphicsWidget::~GraphicsWidget()
{
    d->unregisterWidget(this);
}

Gx::SizeF QGx::GraphicsWidget::sizeF() const
{
    return { static_cast<float>(width()), static_cast<float>(height()) };
}

QGx::GraphicsDevice &QGx::GraphicsWidget::device()
{
    return *d;
}

QPaintEngine *QGx::GraphicsWidget::paintEngine() const
{
    return nullptr;
}

void QGx::GraphicsWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    d->scheduleReset();
}
