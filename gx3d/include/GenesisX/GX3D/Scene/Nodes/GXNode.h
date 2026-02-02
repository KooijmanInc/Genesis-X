// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXNODE_H
#define GXNODE_H

#include <QUrl>
#include <QObject>
#include <QVector3D>
#include <QMatrix4x4>
#include <QQuaternion>
#include <QQmlListProperty>

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

#include <GenesisX/GX3D/Render/Materials/GXMaterial.h>

namespace gx::gx3d::scene {

class GENESISX_GX3D_EXPORT GXNode : public QObject
{
    Q_OBJECT

    Q_CLASSINFO("DefaultProperty", "data")

    Q_PROPERTY(float x READ x WRITE setX NOTIFY positionChanged)
    Q_PROPERTY(float y READ y WRITE setY NOTIFY positionChanged)
    Q_PROPERTY(float z READ z WRITE setZ NOTIFY positionChanged)
    Q_PROPERTY(QVector3D position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(QQuaternion rotation READ rotation WRITE setRotation NOTIFY rotationChanged)
    Q_PROPERTY(QVector3D scale READ scale WRITE setScale NOTIFY scaleChanged)
    Q_PROPERTY(QVector3D eulerRotation READ eulerRotation WRITE setEulerRotation NOTIFY eulerRotationChanged)
    Q_PROPERTY(QVector3D forward READ forward NOTIFY forwardChanged)
    Q_PROPERTY(QVector3D back READ back NOTIFY backChanged)
    Q_PROPERTY(QVector3D left READ left NOTIFY leftChanged)
    Q_PROPERTY(QVector3D right READ right NOTIFY rightChanged)

    Q_PROPERTY(QUrl sceneSource READ sceneSource WRITE setSceneSource NOTIFY sceneSourceChanged)

    Q_PROPERTY(QQmlListProperty<QObject> data READ data)
    Q_PROPERTY(QQmlListProperty<GXNode> children READ children NOTIFY childrenChanged)
    Q_PROPERTY(QQmlListProperty<render::GXMaterial> materials READ materials NOTIFY materialsChanged)

public:
    explicit GXNode(QObject* parent = nullptr);

    float x() const { return m_pos.x(); }
    void setX(float v);

    float y() const { return m_pos.y(); }
    void setY(float v);

    float z() const { return m_pos.z(); }
    void setZ(float v);

    QVector3D position() const { return m_pos; }
    void setPosition(const QVector3D& pos);

    QQuaternion rotation() const { return m_rotation; }
    void setRotation(const QQuaternion& rot);

    QVector3D scale() const { return m_scale; }
    void setScale(const QVector3D& s);

    QVector3D eulerRotation() const { return m_eulerRotation; }
    void setEulerRotation(const QVector3D& eRot);

    QVector3D forward() const;
    QVector3D back() const;
    QVector3D left() const;
    QVector3D right() const;

    QUrl sceneSource() const { return m_sceneSource; }
    void setSceneSource(const QUrl& url);

    Q_INVOKABLE void addYaw(float degrees);
    Q_INVOKABLE void addPitch(float degrees);

    QMatrix4x4 localMatrix() const;
    QMatrix4x4 worldMatrix() const;

    QVector3D worldPosition() const;

    QQmlListProperty<QObject> data();

    QQmlListProperty<GXNode> children();
    const QList<GXNode*>& childrenNodes() const { return m_children; }
    // const QVector<GXNode*> childrenNodes() const { return m_children; }

    QQmlListProperty<render::GXMaterial> materials();
    const QList<render::GXMaterial*>& materialsList() const { return m_materials; }

    void addChild(GXNode* child);

signals:
    void xChanged();
    void yChanged();
    void zChanged();
    void positionChanged();
    void rotationChanged();
    void scaleChanged();
    void eulerRotationChanged();
    void forwardChanged();
    void backChanged();
    void leftChanged();
    void rightChanged();
    void sceneSourceChanged();
    void childrenChanged();
    void materialsChanged();

protected:
    void markTransformDirty();

private:
    static void dataAppend(QQmlListProperty<QObject>* p, QObject* o);
    static qsizetype dataCount(QQmlListProperty<QObject>* p);
    static QObject* dataAt(QQmlListProperty<QObject>* p, qsizetype i);
    static void dataClear(QQmlListProperty<QObject>* p);

    static void appendChild(QQmlListProperty<GXNode>* prop, GXNode* child);
    static qsizetype childCount(QQmlListProperty<GXNode>* prop);
    static GXNode* childAt(QQmlListProperty<GXNode>* prop, qsizetype index);
    static void clearChildren(QQmlListProperty<GXNode>* prop);

    static void materialAppend(QQmlListProperty<render::GXMaterial>* p, render::GXMaterial* o);
    static qsizetype materialCount(QQmlListProperty<render::GXMaterial>* p);
    static render::GXMaterial* materialAt(QQmlListProperty<render::GXMaterial>* p, qsizetype i);
    static void materialClear(QQmlListProperty<render::GXMaterial>* p);

private:
    // QVector<GXNode*> m_children;
    QList<GXNode*> m_children;
    QList<render::GXMaterial*> m_materials;

    QVector3D m_pos {0,0,0};
    QQuaternion m_rotation;
    QVector3D m_scale {1,1,1};
    QVector3D m_eulerRotation {0,0,0};

    QUrl m_sceneSource;
    GXNode* m_sceneRoot = nullptr;

    bool m_dirty = true;
};

}

#endif // GXNODE_H
