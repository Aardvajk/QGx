#ifndef QGX_COLOR_H
#define QGX_COLOR_H

#include <GxMaths/GxColor.h>

#include <QtGui/QColor>

namespace QGx
{

class Color : public Gx::Color
{
public:
    using Gx::Color::Color;

    Color(){ }
    Color(const Gx::Color &v) : Gx::Color(v) { }    
    Color(const QColor &v) : Gx::Color(v.redF(), v.greenF(), v.blueF(), v.alphaF()) { }
    
    operator QColor() const { return QColor::fromRgbF(r, g, b, a); }
};

}

#endif // QGX_COLOR_H
