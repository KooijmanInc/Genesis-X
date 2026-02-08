// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/QtQuick/GXView3D.h>
#include <GenesisX/GX3D/Render/Nodes/GXSceneRenderNode.h>
#include <GenesisX/GX3D/Render/Nodes/GXClearNode.h>
#include <GenesisX/GX3D/Query/GXSceneQueryBackend.h>

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
        connect(m_camera, &GXCamera::lookAtChanged, this, &GXView3D::update);
        connect(m_camera, &GXCamera::upChanged, this, &GXView3D::update);
        connect(m_camera, &GXCamera::changed, this, [this]() { update(); });
    }

    emit cameraChanged();

    update();
}

void GXView3D::setScene(scene::GXScene *s)
{
    if (m_scene == s) return;

    if (m_scene) disconnect(m_scene, nullptr, this, nullptr);

    m_scene = s;

    rebuildQueryBackend();

    if (m_scene) {
        connect(m_scene, &QObject::destroyed, this, [this]() {
            m_scene = nullptr;
            update();
        });
    }

    emit sceneChanged();
    update();
}

QObject *GXView3D::pick(float x, float y)
{
    if (!m_scene || !m_camera)
        return nullptr;

    if (m_scene) m_scene->updateWorldMatrices();

    const auto ray = makeRayFromItemPos(x, y);
    auto hit = m_worldQuery.raycast(ray, /*minPriority=*/1);

    if (!hit.hit) {
        hit = m_worldQuery.raycast(ray);
    }

    if (hit.hit && hit.node) {
        emit picked(hit.node, hit.positionWS, hit.t);
        return hit.node; // QML can use it as QObject*
    }
    return nullptr;
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

query::GXRay GXView3D::makeRayFromItemPos(float x, float y) const
{
    query::GXRay out;

    const float w = float(width());
    const float h = float(height());
    if (w <= 1.0f || h <= 1.0f || !m_camera) {
        out.originWS = QVector3D(0,0,0);
        out.dirWS = QVector3D(0,0,-1);
        return out;
    }

    const float aspect = w / h;

    const QMatrix4x4 view = m_camera->viewMatrix();
    const QMatrix4x4 proj = m_camera->projectionMatrix(aspect);

    bool ok = false;
    const QMatrix4x4 invVP = (proj * view).inverted(&ok);
    if (!ok) {
        out.originWS = QVector3D(0,0,0);
        out.dirWS = QVector3D(0,0,-1);
        return out;
    }

    // Camera world position (robust, doesn't assume local position API)
    const QVector3D camPos = m_camera->worldMatrix().map(QVector3D(0,0,0));

    // Camera forward (prefer target if usable)
    QVector3D camFwd = (m_camera->lookAt() - camPos);
    if (camFwd.lengthSquared() < 1e-8f) {
        // Fallback: derive from view matrix (camera looks down -Z in view space)
        // Inverse(view) * (0,0,-1,0) gives forward direction
        bool ok2 = false;
        const QMatrix4x4 invView = view.inverted(&ok2);
        if (ok2) {
            const QVector4D f = invView * QVector4D(0, 0, -1, 0);
            camFwd = f.toVector3D();
        } else {
            camFwd = QVector3D(0,0,-1);
        }
    }
    camFwd.normalize();

    auto unproject = [&](float ndcX, float ndcY, float zClip) -> QVector3D {
        QVector4D pClip(ndcX, ndcY, zClip, 1.0f);
        QVector4D pWorld = invVP * pClip;
        if (!qFuzzyIsNull(pWorld.w()))
            pWorld /= pWorld.w();
        return pWorld.toVector3D();
    };

    auto makeCandidate = [&](bool flipY, bool z01) -> query::GXRay {
        const float ndcX = (2.0f * x / w) - 1.0f;
        const float ndcY = flipY ? (1.0f - (2.0f * y / h))
                                 : ((2.0f * y / h) - 1.0f);

        const float zNear = z01 ? 0.0f : -1.0f;
        const float zFar  = 1.0f;

        const QVector3D pNear = unproject(ndcX, ndcY, zNear);
        const QVector3D pFar  = unproject(ndcX, ndcY, zFar);

        query::GXRay r;
        r.originWS = camPos;                 // key change: origin = camera
        r.dirWS = (pFar - camPos).normalized();
        return r;
    };

    // Try 4 combos and pick the one most aligned with camera forward
    query::GXRay best = makeCandidate(true,  false);
    float bestDot = QVector3D::dotProduct(best.dirWS, camFwd);

    const query::GXRay c2 = makeCandidate(true,  true);
    const float d2 = QVector3D::dotProduct(c2.dirWS, camFwd);
    if (d2 > bestDot) { bestDot = d2; best = c2; }

    const query::GXRay c3 = makeCandidate(false, false);
    const float d3 = QVector3D::dotProduct(c3.dirWS, camFwd);
    if (d3 > bestDot) { bestDot = d3; best = c3; }

    const query::GXRay c4 = makeCandidate(false, true);
    const float d4 = QVector3D::dotProduct(c4.dirWS, camFwd);
    if (d4 > bestDot) { bestDot = d4; best = c4; }

    // (Optional) debug: you can keep this for one run
    // qDebug() << "pickRay bestDot=" << bestDot << "dir=" << best.dirWS << "camFwd=" << camFwd;

    return best;
    // query::GXRay ray;

    // const float w = float(width());
    // const float h = float(height());
    // if (w <= 1.0f || h <= 1.0f || !m_camera) {
    //     ray.originWS = QVector3D(0,0,0);
    //     ray.dirWS = QVector3D(0,0,-1);
    //     return ray;
    // }

    // // Item coords -> NDC
    // const float ndcX = (2.0f * x / w) - 1.0f;
    // const float ndcY = 1.0f - (2.0f * y / h);

    // const float aspect = w / h;

    // const QMatrix4x4 view = m_camera->viewMatrix();
    // const QMatrix4x4 proj = m_camera->projectionMatrix(aspect);

    // bool ok = false;
    // const QMatrix4x4 invVP = (proj * view).inverted(&ok);
    // if (!ok) {
    //     ray.originWS = QVector3D(0,0,0);
    //     ray.dirWS = QVector3D(0,0,-1);
    //     return ray;
    // }

    // auto unproject = [&](float zClip) -> QVector3D {
    //     const QVector4D pClip(ndcX, ndcY, zClip, 1.0f);
    //     QVector4D pWorld = invVP * pClip;
    //     if (!qFuzzyIsNull(pWorld.w()))
    //         pWorld /= pWorld.w();
    //     return pWorld.toVector3D();
    // };

    // // With Qt's perspective matrix, clip z in [-1, +1] works for unproject
    // const QVector3D pNear = unproject(-1.0f);
    // const QVector3D pFar  = unproject(+1.0f);

    // ray.originWS = pNear;
    // ray.dirWS = (pFar - pNear).normalized();
    // return ray;



    // query::GXRay ray;

    // const float w = float(width());
    // const float h = float(height());
    // if (w <= 1.0f || h <= 1.0f || !m_camera) {
    //     ray.originWS = QVector3D(0,0,0);
    //     ray.dirWS = QVector3D(0,0,-1);
    //     return ray;
    // }

    // // NDC: x in [-1,1], y in [-1,1] (note Qt y-down, NDC y-up)
    // const float ndcX = (2.0f * x / w) - 1.0f;
    // const float ndcY = 1.0f - (2.0f * y / h);

    // // Build view/proj matrices.
    // // If your GXCamera already has view/projection, replace this block with that.
    // const QVector3D camPos = m_camera->position();
    // const QVector3D camTarget = m_camera->target();
    // const QVector3D camUp = m_camera->up();

    // QMatrix4x4 view;
    // view.lookAt(camPos, camTarget, camUp);

    // QMatrix4x4 proj;
    // const float aspect = w / h;
    // proj.perspective(60.0f, aspect, 0.1f, 1000.0f);

    // const QMatrix4x4 invVP = (proj * view).inverted();

    // auto unproject = [&](float zClip) -> QVector3D {
    //     const QVector4D pClip(ndcX, ndcY, zClip, 1.0f);
    //     QVector4D pWorld = invVP * pClip;
    //     if (!qFuzzyIsNull(pWorld.w()))
    //         pWorld /= pWorld.w();
    //     return pWorld.toVector3D();
    // };

    // // Clip-space z: -1 near, +1 far (Qt uses OpenGL-style clip coords in QMatrix4x4)
    // const QVector3D pNear = unproject(-1.0f);
    // const QVector3D pFar  = unproject(+1.0f);

    // ray.originWS = pNear;
    // ray.dirWS = (pFar - pNear).normalized();

    // return ray;
}

void GXView3D::rebuildQueryBackend()
{
    if (!m_scene) {
        m_worldQuery.setBackend(nullptr);
        return;
    }
    m_worldQuery.setBackend(std::make_unique<query::GXSceneQueryBackend>(m_scene));
}
