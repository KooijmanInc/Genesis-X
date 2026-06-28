// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXVIEW3D_H
#define GXVIEW3D_H

#include <QColor>
#include <QTimer>
#include <QPointer>
#include <QVector3D>
#include <QQuickItem>
#include <QElapsedTimer>

#include <GenesisX/GX3D/Core/GXCamera.h>
#include <GenesisX/GX3D/Scene/GXScene.h>
#include <GenesisX/GX3D/Render/Nodes/GXSceneRenderNode.h>
#include <GenesisX/GX3D/Query/GXWorldQuery.h>
#include <GenesisX/GX3D/Query/GXRay.h>
#include <GenesisX/GX3D/Render/Environment/GXEnvironment.h>

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

namespace gx::gx3d {

class GENESISX_GX3D_EXPORT GXView3D : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QColor clearColor READ clearColor WRITE setClearColor NOTIFY clearColorChanged)
    Q_PROPERTY(GXCamera* camera READ camera WRITE setCamera NOTIFY cameraChanged)
    Q_PROPERTY(scene::GXScene* scene READ scene WRITE setScene NOTIFY sceneChanged)
    Q_PROPERTY(render::GXEnvironment* environment READ environment WRITE setEnvironment NOTIFY environmentChanged)

public:
    /**
     * @brief The RenderMode enum
     * OnDemand is default
     * Continuous for game mode
     * Throttled for mobile/battery
     */
    enum RenderMode {
        OnDemand = 0,
        Continuous,
        Throttled
    };
    Q_ENUM(RenderMode)

    Q_PROPERTY(RenderMode renderMode READ renderMode WRITE setRenderMode NOTIFY renderModeChanged)
    Q_PROPERTY(int targetFps READ targetFps WRITE setTargetFps NOTIFY targetFpsChanged)

public:
    explicit GXView3D(QQuickItem* parent = nullptr);

    QColor clearColor() const { return m_clearColor; }
    void setClearColor(const QColor &c);

    RenderMode renderMode() const { return m_renderMode; }
    void setRenderMode(RenderMode m);

    int targetFps() const { return m_targetFps; }
    void setTargetFps(int fps);

    GXCamera* camera() const { return m_camera; }
    QVector3D camPos() const { return m_camPos; }
    void setCamera(GXCamera* cam);

    scene::GXScene* scene() const { return m_scene; }
    void setScene(scene::GXScene* s);

    render::GXEnvironment* environment() const { return m_environment; }
    void setEnvironment(render::GXEnvironment* e);

    Q_INVOKABLE QObject* pick(float x, float y);
    Q_INVOKABLE QVector3D projectToPlane(float x, float y, float planeY) const;

signals:
    void clearColorChanged();
    void renderModeChanged();
    void targetFpsChanged();
    void cameraChanged();
    void sceneChanged();
    void environmentChanged();
    void windowRectPxChanged();
    void picked(QObject* node, QVector3D positionWS, float distance);

protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override;
    void itemChange(ItemChange change, const ItemChangeData& data) override;
    void geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) override;
    void componentComplete() override;
    void syncFrameData(render::GXSceneRenderNode* rn);

private:
    void applyRenderPolicy();
    void onFrameSwapped();
    bool canRenderContinuously() const;
    query::GXWorldQuery m_worldQuery;

    query::GXRay makeRayFromItemPos(float x, float y) const;
    void rebuildQueryBackend();

private:
    QColor m_clearColor = Qt::black;
    QVector3D m_camPos = {0,0,0};
    QRect m_windowRectPx;

    RenderMode m_renderMode = Continuous;
    int m_targetFps = 30;

    QTimer m_tickTimer;
    QMetaObject::Connection m_frameSwappedConn;
    QPointer<QQuickWindow> m_connectedWindow;
    QElapsedTimer m_frameLimiter;

    QPointer<GXCamera> m_camera;
    scene::GXScene* m_scene = nullptr;
    QPointer<render::GXEnvironment> m_environment;
};

}

#endif // GXVIEW3D_H
