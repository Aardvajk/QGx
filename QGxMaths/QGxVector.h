#ifndef QGXVECTOR_H
#define QGXVECTOR_H

#include <GxMaths/GxVector.h>

#include <QtCore/QPointF>

#include <QtGui/QVector2D>
#include <QtGui/QVector3D>
#include <QtGui/QVector4D>

namespace QGx
{

class Vec2 : public Gx::Vec2
{
public:
    using Gx::Vec2::Vec2;
    
    Vec2(){ }
    Vec2(const Gx::Vec2 &v) : Gx::Vec2(v) { }
    Vec2(const QPointF &v) : Gx::Vec2(v.x(), v.y()) { }
    Vec2(const QVector2D &v) : Gx::Vec2(v.x(), v.y()) { }

    operator QPointF() const { return { x, y }; }
    operator QVector2D() const { return { x, y }; }
};

class Vec3 : public Gx::Vec3
{
public:
    using Gx::Vec3::Vec3;
    
    Vec3(){ }
    Vec3(const Gx::Vec3 &v) : Gx::Vec3(v) { }
    Vec3(const QVector3D &v) : Gx::Vec3(v.x(), v.y(), v.z()) { }

    operator QVector3D() const { return { x, y, z }; }
};

class Vec4 : public Gx::Vec4
{
public:
    using Gx::Vec4::Vec4;
    
    Vec4(){ }
    Vec4(const Gx::Vec4 &v) : Gx::Vec4(v) { }
    Vec4(const QVector4D &v) : Gx::Vec4(v.x(), v.y(), v.z(), v.w()) { }

    operator QVector4D() const { return { x, y, z, w }; }
};

}

#endif // QGXVECTOR_H
