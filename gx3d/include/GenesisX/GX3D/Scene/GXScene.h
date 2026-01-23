// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXSCENE_H
#define GXSCENE_H

#include <QObject>
#include <QQmlListProperty>

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

namespace gx::gx3d::scene {

class GXNode;

class GENESISX_GX3D_EXPORT GXScene : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QQmlListProperty<GXNode> roots READ roots)

public:
    explicit GXScene(QObject* parent = nullptr);

    QQmlListProperty<GXNode> roots();

    Q_INVOKABLE void addRoot(GXNode* node);
    Q_INVOKABLE void removeRoot(GXNode* node);
    Q_INVOKABLE void clear();

    void traverse(const std::function<void(GXNode*)> &visitor) const;

signals:
    void nodeAdded(gx::gx3d::scene::GXNode* node);

private:
    QList<GXNode*> m_roots;

    static void qmlAppendRoot(QQmlListProperty<GXNode>* list, GXNode* node);
    static qsizetype qmlRootCount(QQmlListProperty<GXNode>* list);
    static GXNode* qmlRootAt(QQmlListProperty<GXNode>* list, qsizetype index);
    static void qmlClearRoots(QQmlListProperty<GXNode>* list);

    void traverseNode(GXNode* node, const std::function<void(GXNode*)>& visitor) const;
};

}

#endif // GXSCENE_H
