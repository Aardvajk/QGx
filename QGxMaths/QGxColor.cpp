#include "QGxColor.h"

QGx::Color::Color(const QColor &v) : Gx::Color(v.redF(), v.greenF(), v.blueF(), v.alphaF())
{
}
