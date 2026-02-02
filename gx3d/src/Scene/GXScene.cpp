// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Scene/GXScene.h>
#include <GenesisX/GX3D/Scene/Nodes/GXNode.h>
#include <GenesisX/GX3D/Scene/Lights/GXLight.h>
#include <GenesisX/GX3D/Scene/Lights/GXPointLight.h>

using namespace gx::gx3d::scene;

GXScene::GXScene(QObject *parent)
    : QObject{parent}
{
}

void GXScene::setDebugLighting(bool on)
{
    if (m_debugLighting == on) return;
    m_debugLighting = on;

    emit debugLightingChanged();
}

void GXScene::setMaxLights(int v)
{
    v = qBound(0, v, 1024);
    if (m_maxLights == v) return;

    m_maxLights = v;

    emit maxLightsChanged();
}

QQmlListProperty<GXNode> GXScene::roots()
{
    return QQmlListProperty<GXNode>(
        this,
        this,
        &GXScene::qmlAppendRoot,
        &GXScene::qmlRootCount,
        &GXScene::qmlRootAt,
        &GXScene::qmlClearRoots
    );
}

QQmlListProperty<QObject> GXScene::items()
{
    return QQmlListProperty<QObject>(
        this,
        this,
        &GXScene::appendItems,
        &GXScene::itemsCount,
        &GXScene::itemsAt,
        &GXScene::clearItems
        );
}

void GXScene::addRootNode(GXNode *node)
{
    if (!node) return;

    if (m_nodes.contains(node)) return;

    if (!node->parent()) node->setParent(this);

    m_roots.append(node);
    m_nodes.append(node);

    emit nodeAdded(node);
}

void GXScene::addRootLight(GXLight *obj)
{
    if (!obj) return;

    if (m_lights.contains(obj)) return;

    if (!obj->parent()) obj->setParent(this);

    m_items.append(obj);
    m_lights.append(obj);
    m_roots.append(obj);

    emit sceneChanged();
}

void GXScene::removeRoot(GXNode *node)
{
    if (!node) return;

    m_roots.removeAll(node);
}

void GXScene::clear()
{
    m_roots.clear();
}

void GXScene::traverse(const std::function<void (GXNode *)> &visitor) const
{
    for (GXNode* root : m_roots) traverseNode(root, visitor);
}

void GXScene::appendItems(QQmlListProperty<QObject> *prop, QObject *obj)
{
    auto* self = static_cast<GXScene*>(prop->data);

    if (auto *n = qobject_cast<GXLight*>(obj)) {
        self->addRootLight(n);
    } else if (auto *n = qobject_cast<GXNode*>(obj)) {
        self->addRootNode(n);
    } else  {
        qWarning() << "[GXScene] unsupported scene object";
    }
}

qsizetype GXScene::itemsCount(QQmlListProperty<QObject> *prop)
{
    auto* self = static_cast<GXScene*>(prop->data);
    return self->m_items.size();
}

QObject *GXScene::itemsAt(QQmlListProperty<QObject> *prop, qsizetype index)
{
    auto* self = static_cast<GXScene*>(prop->data);
    return (index >= 0 && index < self->m_items.size()) ? self->m_items.at(index) : nullptr;
}

void GXScene::clearItems(QQmlListProperty<QObject> *prop)
{
    auto* self = static_cast<GXScene*>(prop->data);
    self->m_items.clear();
    self->m_nodes.clear();
    self->m_lights.clear();

    emit self->sceneChanged();
}

void GXScene::qmlAppendRoot(QQmlListProperty<GXNode> *list, GXNode *node)
{
    auto* self = static_cast<GXScene*>(list->data);
    self->addRootNode(node);
}

qsizetype GXScene::qmlRootCount(QQmlListProperty<GXNode> *list)
{
    auto* self = static_cast<GXScene*>(list->data);
    return self->m_roots.size();
}

GXNode *GXScene::qmlRootAt(QQmlListProperty<GXNode> *list, qsizetype index)
{
    auto *self = static_cast<GXScene*>(list->data);
    if (index < 0 || index >= self->m_roots.size()) return nullptr;

    return self->m_roots.at(index);
}

void GXScene::qmlClearRoots(QQmlListProperty<GXNode> *list)
{
    auto *self = static_cast<GXScene*>(list->data);
    self->clear();
}

void GXScene::traverseNode(GXNode *node, const std::function<void (GXNode *)> &visitor) const
{
    if (!node) return;

    visitor(node);

    const auto& kids = node->childrenNodes();
    for (GXNode* child : kids) {
        traverseNode(child, visitor);
    }
}
