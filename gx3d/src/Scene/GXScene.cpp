// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Scene/GXScene.h>
#include <GenesisX/GX3D/Scene/Nodes/GXNode.h>

using namespace gx::gx3d::scene;

GXScene::GXScene(QObject *parent)
    : QObject{parent}
{
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

void GXScene::addRoot(GXNode *node)
{
    if (!node) return;
    qWarning() << "GXScene addRoot" << node << node->metaObject()->className() << "name=" << node->objectName();

    if (m_roots.contains(node)) return;

    if (!node->parent()) node->setParent(this);

    m_roots.append(node);
    emit nodeAdded(node);
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

void GXScene::qmlAppendRoot(QQmlListProperty<GXNode> *list, GXNode *node)
{
    auto* self = static_cast<GXScene*>(list->data);
    self->addRoot(node);
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
        // if (auto* childNode = qobject_cast<GXNode*>(obj))
        traverseNode(child, visitor);
    }
}

