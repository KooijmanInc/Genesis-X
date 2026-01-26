// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Scene/Nodes/GXNode.h>

using namespace gx::gx3d::scene;

static inline bool fuzzyVec3(const QVector3D& a, const QVector3D& b, float eps = 1e-4)
{
    return (a - b).lengthSquared() <= eps * eps;
}

GXNode::GXNode(QObject *parent)
    : QObject{parent}
{
}

void GXNode::setX(float v)
{
    if (qFuzzyCompare(m_pos.x(), v)) return;
    m_pos.setX(v);

    emit xChanged();
    markTransformDirty();
}

void GXNode::setY(float v)
{
    if (qFuzzyCompare(m_pos.y(), v)) return;
    m_pos.setY(v);

    emit yChanged();
    markTransformDirty();
}

void GXNode::setZ(float v)
{
    if (qFuzzyCompare(m_pos.z(), v)) return;
    m_pos.setZ(v);

    emit zChanged();
    markTransformDirty();
}

void GXNode::setPosition(const QVector3D &pos)
{
    if (m_pos == pos) return;
    m_pos = pos;

    emit positionChanged();
    markTransformDirty();
}

void GXNode::setRotation(const QQuaternion &rot)
{
    if (m_rotation == rot) return;
    m_rotation = rot;

    m_eulerRotation = m_rotation.toEulerAngles();

    emit rotationChanged();
    emit eulerRotationChanged();
    markTransformDirty();
}

void GXNode::setScale(const QVector3D &s)
{
    if (m_scale == s) return;
    m_scale = s;

    emit scaleChanged();
    markTransformDirty();
}

void GXNode::setEulerRotation(const QVector3D &eRot)
{
    if (fuzzyVec3(eRot, m_eulerRotation)) return;

    m_eulerRotation = eRot;
    m_rotation = QQuaternion::fromEulerAngles(m_eulerRotation);

    emit eulerRotationChanged();
    emit rotationChanged();
    markTransformDirty();
}

QVector3D GXNode::forward() const
{
    return m_rotation.rotatedVector(QVector3D(0, 0, -1));
}

QVector3D GXNode::back() const
{
    return m_rotation.rotatedVector(QVector3D(0, 0, 1));
}

QVector3D GXNode::left() const
{
    return m_rotation.rotatedVector(QVector3D(-1, 0, 0));
}

QVector3D GXNode::right() const
{
    return m_rotation.rotatedVector(QVector3D(1, 0, 0));
}

void GXNode::addYaw(float degrees)
{
    m_eulerRotation.setY(m_eulerRotation.y() + degrees);
    m_rotation = QQuaternion::fromEulerAngles(m_eulerRotation);

    emit eulerRotationChanged();
    emit rotationChanged();
    markTransformDirty();
}

void GXNode::addPitch(float degrees)
{
    m_eulerRotation.setX(m_eulerRotation.x() + degrees);
    m_rotation = QQuaternion::fromEulerAngles(m_eulerRotation);

    emit eulerRotationChanged();
    emit rotationChanged();
    markTransformDirty();
}

QMatrix4x4 GXNode::localMatrix() const
{
    QMatrix4x4 m;
    m.translate(m_pos);
    m.rotate(m_rotation);
    m.scale(m_scale);

    return m;
}

QMatrix4x4 GXNode::worldMatrix() const
{
    if (auto* p = qobject_cast<GXNode*>(parent())) return p->worldMatrix() * localMatrix();

    return localMatrix();
}

QVector3D GXNode::worldPosition() const
{
    return worldMatrix().map(QVector3D(0,0,0));
}

QQmlListProperty<GXNode> GXNode::children()
{
    return QQmlListProperty<GXNode>(
        this,
        this,
        &GXNode::appendChild,
        &GXNode::childCount,
        &GXNode::childAt,
        &GXNode::clearChildren
    );
}

void GXNode::markTransformDirty()
{
    m_dirty = true;
}

void GXNode::appendChild(QQmlListProperty<GXNode> *prop, GXNode *child)
{
    auto* self = static_cast<GXNode*>(prop->data);
    if (!child || self->m_children.contains(child))
        return;

    child->setParent(self);   // QObject ownership
    self->m_children.append(child);
}

qsizetype GXNode::childCount(QQmlListProperty<GXNode> *prop)
{
    auto* self = static_cast<GXNode*>(prop->data);
    return self->m_children.size();
}

GXNode *GXNode::childAt(QQmlListProperty<GXNode> *prop, qsizetype index)
{
    auto* self = static_cast<GXNode*>(prop->data);
    return self->m_children.value(index);
}

void GXNode::clearChildren(QQmlListProperty<GXNode> *prop)
{
    auto* self = static_cast<GXNode*>(prop->data);
    self->m_children.clear();
}
