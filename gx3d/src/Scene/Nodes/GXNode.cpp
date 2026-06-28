// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Scene/Nodes/GXNode.h>
#include <GenesisX/GX3D/Render/Utils/GXGltfLoader.h>

using namespace gx::gx3d::scene;

static inline bool fuzzyVec3(const QVector3D& a, const QVector3D& b, float eps = 1e-4)
{
    return (a - b).lengthSquared() <= eps * eps;
}

GXNode::GXNode(QObject *parent)
    : QObject{parent}
{
    m_worldMatrix = localMatrix();
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

void GXNode::setSceneSource(const QUrl &url)
{
    if (m_sceneSource == url) return;

    m_sceneSource = url;
    emit sceneSourceChanged();

    if (m_sceneRoot) {
        m_children.removeOne(m_sceneRoot);
        delete m_sceneRoot;
        m_sceneRoot = nullptr;
    }

    if (!m_sceneSource.isValid()) return;

    GXNode* loadedRoot = render::GXGltfLoader::loadSceneRoot(m_sceneSource);
    if (!loadedRoot) return;

    addChild(loadedRoot);
    m_sceneRoot = loadedRoot;
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
    if (!m_dirty)
        return m_worldMatrix;

    QMatrix4x4 parentWorld;
    if (auto* p = qobject_cast<GXNode*>(parent())) {
        parentWorld = p->worldMatrix(); // parent will lazily update itself too
    } else {
        parentWorld.setToIdentity();
    }

    m_worldMatrix = parentWorld * localMatrix();
    m_dirty = false;
    return m_worldMatrix;
    // if (auto* p = qobject_cast<GXNode*>(parent())) return p->worldMatrix() * localMatrix();

    // return localMatrix();
}

QVector3D GXNode::worldPosition() const
{
    return m_worldMatrix.map(QVector3D(0, 0, 0));
    // return worldMatrix().map(QVector3D(0,0,0));
}

QQmlListProperty<QObject> GXNode::data()
{
    return QQmlListProperty<QObject>(
        this,
        this,
        &GXNode::dataAppend,
        &GXNode::dataCount,
        &GXNode::dataAt,
        &GXNode::dataClear
        );
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

QQmlListProperty<gx::gx3d::render::GXMaterial> GXNode::materials()
{
    return QQmlListProperty<render::GXMaterial>(
        this,
        this,
        &GXNode::materialAppend,
        &GXNode::materialCount,
        &GXNode::materialAt,
        &GXNode::materialClear
        );
}

void GXNode::addChild(GXNode *child)
{
    if (!child || m_children.contains(child)) return;

    child->setParent(this);
    m_children.append(child);
}

void GXNode::updateWorldRecursive(const QMatrix4x4 &parentWorld)
{
    m_worldMatrix = parentWorld * localMatrix();
    m_dirty = false;

    for (GXNode* c : childrenNodes()) {
        if (c) c->updateWorldRecursive(m_worldMatrix);
    }
    // const QMatrix4x4 local = localMatrix();
    // m_worldMatrix = parentWorld * local;

    // const auto& kids = childrenNodes();
    // for (GXNode* c : kids) {
    //     if (c) c->updateWorldRecursive(m_worldMatrix);
    // }
}

void GXNode::markTransformDirty()
{
    m_dirty = true;
    auto& child = m_children;
    for (GXNode* c : child) {
        if (c) c->markTransformDirty();
    }
    emit changed();
}

void GXNode::dataAppend(QQmlListProperty<QObject> *p, QObject *o)
{
    auto* self = static_cast<GXNode*>(p->data);
    if (!self || !o) return;

    // Route by type
    if (auto* n = qobject_cast<GXNode*>(o)) {
        if (!self->m_children.contains(n)) {
            self->m_children.append(n);
            if (!n->parent()) n->setParent(self);
            emit self->childrenChanged();
        }
        return;
    }

    if (auto* m = qobject_cast<gx::gx3d::render::GXMaterial*>(o)) {
        if (!self->m_materials.contains(m)) {
            self->m_materials.append(m);
            if (!m->parent()) m->setParent(self);
            emit self->materialsChanged();
        }
        return;
    }

    // Unknown object type: keep ownership to avoid leaks, but otherwise ignore
    if (!o->parent()) o->setParent(self);
}

qsizetype GXNode::dataCount(QQmlListProperty<QObject> *p)
{
    auto* self = static_cast<GXNode*>(p->data);
    if (!self) return 0;
    return self->m_children.size() + self->m_materials.size();
}

QObject *GXNode::dataAt(QQmlListProperty<QObject> *p, qsizetype i)
{
    auto* self = static_cast<GXNode*>(p->data);
    if (!self) return nullptr;

    if (i < self->m_children.size())
        return self->m_children.at(i);

    const qsizetype j = i - self->m_children.size();
    if (j < self->m_materials.size())
        return self->m_materials.at(j);

    return nullptr;
}

void GXNode::dataClear(QQmlListProperty<QObject> *p)
{
    auto* self = static_cast<GXNode*>(p->data);
    if (!self) return;

    self->m_children.clear();
    self->m_materials.clear();

    emit self->childrenChanged();
    emit self->materialsChanged();
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

void GXNode::materialAppend(QQmlListProperty<render::GXMaterial> *p, render::GXMaterial *m)
{
    auto* self = static_cast<GXNode*>(p->data);
    if (!self || !m) return;

    if (self->m_materials.contains(m))
        return;

    self->m_materials.append(m);

    if (!m->parent())
        m->setParent(self);

    emit self->materialsChanged();
}

qsizetype GXNode::materialCount(QQmlListProperty<render::GXMaterial> *p)
{
    auto* self = static_cast<GXNode*>(p->data);
    return self ? self->m_materials.size() : 0;
}

gx::gx3d::render::GXMaterial *GXNode::materialAt(QQmlListProperty<render::GXMaterial> *p, qsizetype i)
{
    auto* self = static_cast<GXNode*>(p->data);
    if (!self) return nullptr;
    if (i < 0 || i >= self->m_materials.size()) return nullptr;
    return self->m_materials.at(i);
}

void GXNode::materialClear(QQmlListProperty<render::GXMaterial> *p)
{
    auto* self = static_cast<GXNode*>(p->data);
    if (!self) return;

    self->m_materials.clear();
    emit self->materialsChanged();
}
