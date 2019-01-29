#ifndef QGX_MATHSMETATYPES_H
#define QGX_MATHSMETATYPES_H

#include <GxMaths/GxVector.h>
#include <GxMaths/GxMatrix.h>
#include <GxMaths/GxQuaternion.h>

#include <QtCore/QMetaType>
#include <QtCore/QDataStream>

Q_DECLARE_METATYPE(Gx::Vec2)
Q_DECLARE_METATYPE(Gx::Vec3)
Q_DECLARE_METATYPE(Gx::Vec4)

Q_DECLARE_METATYPE(Gx::Matrix)

Q_DECLARE_METATYPE(Gx::Quaternion)

QDataStream &operator<<(QDataStream &ds, const Gx::Vec2 &v);
QDataStream &operator<<(QDataStream &ds, const Gx::Vec3 &v);
QDataStream &operator<<(QDataStream &ds, const Gx::Vec4 &v);

QDataStream &operator>>(QDataStream &ds, Gx::Vec2 &v);
QDataStream &operator>>(QDataStream &ds, Gx::Vec3 &v);
QDataStream &operator>>(QDataStream &ds, Gx::Vec4 &v);

namespace QGx
{

void registerMathStreamOperators();

}

#endif // QGX_MATHSMETATYPES_H
