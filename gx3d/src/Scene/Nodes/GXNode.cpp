// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Scene/Nodes/GXNode.h>

using namespace gx::gx3d::scene;

GXNode::GXNode(QObject *parent)
    : QObject{parent}
{
}

void GXNode::setX(float v)
{
    if (qFuzzyCompare(m_pos.x(), v)) return;
    m_pos.setX(v);

    emit xChanged();
}

void GXNode::setY(float v)
{
    if (qFuzzyCompare(m_pos.y(), v)) return;
    m_pos.setY(v);

    emit yChanged();
}

void GXNode::setZ(float v)
{
    if (qFuzzyCompare(m_pos.z(), v)) return;
    m_pos.setZ(v);

    emit zChanged();
}

void GXNode::setPosition(const QVector3D &pos)
{
    if (m_pos == pos) return;
    m_pos = pos;

    emit positionChanged();
}

void GXNode::setRotation(const QQuaternion &rot)
{
    if (m_rotation == rot) return;
    m_rotation = rot;

    emit rotationChanged();
}

void GXNode::setScale(const QVector3D &s)
{
    if (m_scale == s) return;
    m_scale = s;

    emit scaleChanged();
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
