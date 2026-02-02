// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXSCENE_H
#define GXSCENE_H

#include <QObject>
#include <QQmlListProperty>

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

namespace gx::gx3d::scene {

class GXNode;
class GXLight;

class GENESISX_GX3D_EXPORT GXScene : public QObject
{
    Q_OBJECT

    Q_CLASSINFO("DefaultProperty", "items")

    Q_PROPERTY(bool debugLighting READ debugLighting WRITE setDebugLighting NOTIFY debugLightingChanged)
    Q_PROPERTY(int maxLights READ maxLights WRITE setMaxLights NOTIFY maxLightsChanged)

    Q_PROPERTY(QQmlListProperty<GXNode> roots READ roots)
    Q_PROPERTY(QQmlListProperty<QObject> items READ items)
    // Q_PROPERTY(QQmlListProperty<render::GXMaterial> materials READ materials NOTIFY materialsChanged)

public:
    explicit GXScene(QObject* parent = nullptr);

    bool debugLighting() const { return m_debugLighting; }
    void setDebugLighting(bool on);

    int maxLights() const { return m_maxLights; }
    void setMaxLights(int v);

    const QList<GXNode*>& nodes() const { return m_nodes; }
    const QList<GXLight*>& lights() const { return m_lights; }
    // const QList<render::GXMaterial*>& materialsList() const { return m_materials; }

    QQmlListProperty<GXNode> roots();
    QQmlListProperty<QObject> items();
    // QQmlListProperty<render::GXMaterial> materials();

    Q_INVOKABLE void addRootNode(GXNode* node);
    Q_INVOKABLE void addRootLight(GXLight* light);
    // Q_INVOKABLE void addRootMaterial(render::GXMaterial* material);
    Q_INVOKABLE void removeRoot(GXNode* node);
    Q_INVOKABLE void clear();

    void traverse(const std::function<void(GXNode*)> &visitor) const;

signals:
    void sceneChanged();
    void debugLightingChanged();
    void maxLightsChanged();
    // void materialsChanged();
    void nodeAdded(gx::gx3d::scene::GXNode* node);

private:
    static void appendItems(QQmlListProperty<QObject>* prop, QObject* obj);
    static qsizetype itemsCount(QQmlListProperty<QObject>* prop);
    static QObject* itemsAt(QQmlListProperty<QObject>* prop, qsizetype index);
    static void clearItems(QQmlListProperty<QObject>* prop);

private:
    bool m_debugLighting = false;
    int m_maxLights = 8;
    QList<GXNode*> m_roots;
    QList<GXNode*> m_nodes;
    QList<GXLight*> m_lights;
    // QList<render::GXMaterial*> m_materials;
    QList<QObject*> m_items;

    static void qmlAppendRoot(QQmlListProperty<GXNode>* list, GXNode* node);
    static qsizetype qmlRootCount(QQmlListProperty<GXNode>* list);
    static GXNode* qmlRootAt(QQmlListProperty<GXNode>* list, qsizetype index);
    static void qmlClearRoots(QQmlListProperty<GXNode>* list);

    // static void materialsAppend(QQmlListProperty<gx::gx3d::render::GXMaterial>* p, gx::gx3d::render::GXMaterial* m);
    // static qsizetype materialsCount(QQmlListProperty<gx::gx3d::render::GXMaterial>* p);
    // static gx::gx3d::render::GXMaterial* materialsAt(QQmlListProperty<gx::gx3d::render::GXMaterial>* p, qsizetype i);
    // static void materialsClear(QQmlListProperty<gx::gx3d::render::GXMaterial>* p);

    void traverseNode(GXNode* node, const std::function<void(GXNode*)>& visitor) const;
    // void traverseLight(GXLight* node, const std::function<void(GXLight*)>& visitor) const;
};

}

#endif // GXSCENE_H
