#include "QGxMaths/QGxMathsMetatypes.h"

QDataStream &operator<<(QDataStream &ds, const Gx::Vec2 &v)
{
    return ds << v.x << v.y;
}

QDataStream &operator<<(QDataStream &ds, const Gx::Vec3 &v)
{
    return ds << v.x << v.y << v.z;
}

QDataStream &operator<<(QDataStream &ds, const Gx::Vec4 &v)
{
    return ds << v.x << v.y << v.z << v.w;
}

QDataStream &operator>>(QDataStream &ds, Gx::Vec2 &v)
{
    return ds >> v.x >> v.y;
}

QDataStream &operator>>(QDataStream &ds, Gx::Vec3 &v)
{
    return ds >> v.x >> v.y >> v.z;
}

QDataStream &operator>>(QDataStream &ds, Gx::Vec4 &v)
{
    return ds >> v.x >> v.y >> v.z >> v.w;
}

void QGx::registerMathStreamOperators()
{
    qRegisterMetaTypeStreamOperators<Gx::Vec2>("Gx::Vec2");
    qRegisterMetaTypeStreamOperators<Gx::Vec3>("Gx::Vec3");
    qRegisterMetaTypeStreamOperators<Gx::Vec4>("Gx::Vec4");
}
