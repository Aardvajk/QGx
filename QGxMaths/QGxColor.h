#ifndef QGXCOLOR_H
#define QGXCOLOR_H

#include <GxMaths/GxColor.h>

#include <QtGui/QColor>

namespace QGx
{

class Color : public Gx::Color
{
public:
    using Gx::Color::Color;
    Color(const QColor &v);
};

}

#endif // QGXCOLOR_H
