// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/QtQuick/GXView3D.h>
#include <GenesisX/GX3D/Render/Nodes/GXSceneRenderNode.h>
#include <GenesisX/GX3D/Render/Nodes/GXClearNode.h>

#include <QSGSimpleRectNode>

#include <QtGlobal>

using namespace gx::gx3d;
using namespace gx::gx3d::render;

GXView3D::GXView3D(QQuickItem *parent)
    : QQuickItem{parent}
{
    setFlag(ItemHasContents, true);

    m_tickTimer.setTimerType(Qt::PreciseTimer);
    connect(&m_tickTimer, &QTimer::timeout, this, [this]() {
        update();
    });
}

void GXView3D::setClearColor(const QColor &c)
{
    if (m_clearColor == c) return;
    m_clearColor = c;

    emit clearColorChanged();

    update();
}

void GXView3D::setRenderMode(RenderMode m)
{
    if (m_renderMode == m) return;
    m_renderMode = m;

    emit renderModeChanged();
    applyRenderPolicy();
}

void GXView3D::setTargetFps(int fps)
{
    fps = qBound(1, fps, 240);
    if (m_targetFps == fps) return;
    m_targetFps = fps;

    emit targetFpsChanged();
    applyRenderPolicy();
}

void GXView3D::setCamera(GXCamera *cam)
{
    if (m_camera == cam) return;

    if (m_camera) disconnect(m_camera, nullptr, this, nullptr);

    m_camera = cam;

    if (m_camera) {
        connect(m_camera, &GXCamera::positionChanged, this, &GXView3D::update);
        connect(m_camera, &GXCamera::targetChanged, this, &GXView3D::update);
        connect(m_camera, &GXCamera::upChanged, this, &GXView3D::update);
    }

    emit cameraChanged();

    update();
}

void GXView3D::setScene(scene::GXScene *s)
{
    if (m_scene == s) return;
    // qWarning() << "GXView3D scene set s=" << s << "objectName" << s->objectName() << "classname" << (s ? s->metaObject()->className() : "null");
    if (m_scene) disconnect(m_scene, nullptr, this, nullptr);

    m_scene = s;

    // qWarning() << "GXView3D scene set to" << m_scene << "objectName" << m_scene->objectName();

    if (m_scene) {
        connect(m_scene, &QObject::destroyed, this, [this]() {
            m_scene = nullptr;
            update();
        });
    }

    emit sceneChanged();
    update();
}

QSGNode *GXView3D::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    QSGNode *root = oldNode;
    if (!root) {
        root = new QSGNode();
        root->appendChildNode(new GXClearNode());
        root->appendChildNode(new GXSceneRenderNode());
    }

    auto *clearNode = static_cast<GXClearNode *>(root->firstChild());
    auto *sceneNode   = static_cast<GXSceneRenderNode *>(clearNode->nextSibling());

    clearNode->setRect(QRectF(0, 0, width(), height()));
    clearNode->setColor(m_clearColor);

    // Compute camera MVP
    const float aspect = (height() > 0.0f) ? float(width()) / float(height()) : 1.0f;

    QMatrix4x4 view, proj;
    if (m_camera) {
        view = m_camera->viewMatrix();
        proj = m_camera->projectionMatrix(aspect);
    } else {
        view.lookAt({0,0,5}, {0,0,0}, {0,1,0});
        proj.perspective(60.0f, aspect, 0.1f, 1000.0f);
    }

    sceneNode->setRect(QRectF(0, 0, width(), height()));
    sceneNode->setScene(m_scene);
    sceneNode->setQuickWindow(window());
    sceneNode->setViewProj(proj * view);

    if (m_renderMode == Continuous && canRenderContinuously())
        update();

    return root;
}

void GXView3D::itemChange(ItemChange change, const ItemChangeData &data)
{
    if (window()) {
        const QSurfaceFormat fmt = window()->format();
        if (fmt.depthBufferSize() <= 0) {
            qWarning()
            << "GenesisX3D WARNING:"
            << "QQuickWindow has no depth buffer."
            << "Depth testing will not work correctly."
            << "Host application must enable depth via QSurfaceFormat.";
        }
    }

    if (change == ItemSceneChange && window()) {
        QSurfaceFormat fmt = window()->format();
        fmt.setDepthBufferSize(24);
        fmt.setStencilBufferSize(8);
        window()->setFormat(fmt);
    }
    QQuickItem::itemChange(change, data);

    if (change == ItemVisibleHasChanged || change == ItemSceneChange || change == ItemOpacityHasChanged) {
        Q_UNUSED(data);
        applyRenderPolicy();
    }
}

void GXView3D::applyRenderPolicy()
{
    m_tickTimer.stop();

    if (!canRenderContinuously()) {
        return;
    }

    switch (m_renderMode) {
    case OnDemand:
        break;
    case Continuous:
        m_tickTimer.start(0);
        break;
    case Throttled: {
        const int intervalMs = qMax(1, 1000 / qMax(1, m_targetFps));
        m_tickTimer.start(intervalMs);
        break;
    }
    }
}

bool GXView3D::canRenderContinuously() const
{
    if (!window()) return false;
    if (!isVisible()) return false;
    if (width() <= 0 || height() <= 0) return false;
    if (opacity() <= 0.0) return false;

    return true;
}
