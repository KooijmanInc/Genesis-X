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

// struct ScopedMicros {
//     const char* label = nullptr;
//     QElapsedTimer t;
//     qint64* sink = nullptr; // optional accumulator

//     ScopedMicros(const char* l, qint64* s = nullptr) : label(l), sink(s) { t.start(); }
//     ~ScopedMicros() {
//         const qint64 us = t.nsecsElapsed() / 1000;
//         if (sink) *sink += us;
//         else qDebug() << label << us << "us";
//     }
// };

GXView3D::GXView3D(QQuickItem *parent)
    : QQuickItem{parent}
{
    setFlag(ItemHasContents, true);

    // m_tickTimer.setTimerType(Qt::PreciseTimer);
    // connect(&m_tickTimer, &QTimer::timeout, this, [this]() {
    //     update();
    //     if (window()) window()->update();
    // });
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
        connect(m_camera, &GXCamera::positionChanged, this, &GXView3D::update, Qt::QueuedConnection);
        connect(m_camera, &GXCamera::lookAtChanged, this, &GXView3D::update, Qt::QueuedConnection);
        connect(m_camera, &GXCamera::upChanged, this, &GXView3D::update, Qt::QueuedConnection);
        connect(m_camera, &GXCamera::changed, this, &GXView3D::update, Qt::QueuedConnection);
        connect(m_camera, &GXCamera::changed, this, [this]() { update(); }, Qt::QueuedConnection);
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
        }, Qt::QueuedConnection);
        connect(m_scene, &scene::GXScene::sceneChanged, this, &QQuickItem::update, Qt::QueuedConnection);
    }

    emit sceneChanged();
    update();
}

void GXView3D::setEnvironment(render::GXEnvironment *e)
{
    if (m_environment == e) return;
    m_environment = e;

    emit environmentChanged();
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

QVector3D GXView3D::projectToPlane(float x, float y, float planeY) const
{
    const auto ray = makeRayFromItemPos(x, y);

    const float dy = ray.dirWS.y();
    if (qFuzzyIsNull(dy)) return ray.originWS;

    const float t = (planeY - ray.originWS.y()) / dy;

    return ray.originWS + ray.dirWS * t;
}

QSGNode *GXView3D::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    // QElapsedTimer totalT;
    // totalT.start();
    // static QElapsedTimer s_aggTimer;
    // static bool s_aggStarted = false;

    // static qint64 s_us_total = 0;
    // static qint64 s_us_sync = 0;
    // static qint64 s_us_rect = 0;
    // static qint64 s_us_setters = 0;
    // static int    s_samples = 0;

    // if (!s_aggStarted) {
    //     s_aggTimer.start();
    //     s_aggStarted = true;
    // }

    QSGNode *root = oldNode;
    if (!root) {
        root = new QSGNode();
        root->appendChildNode(new GXClearNode());
        root->appendChildNode(new GXSceneRenderNode());
    }

    auto *clearNode = static_cast<GXClearNode *>(root->firstChild());
    auto *sceneNode   = static_cast<GXSceneRenderNode *>(clearNode->nextSibling());

    {
        // ScopedMicros tm("clearNode", &s_us_rect);
        clearNode->setRect(QRectF(0, 0, width(), height()));
        clearNode->setColor(m_clearColor);
    }

    // Compute camera MVP
    const float aspect = (height() > 0.0f) ? float(width()) / float(height()) : 1.0f;

    QMatrix4x4 view, proj;
    if (m_camera) {
        (void)m_camera->worldMatrix();
        view = m_camera->viewMatrix();
        proj = m_camera->projectionMatrix(aspect);

        // const auto world = m_camera->worldMatrix();
        // qDebug() << "[cam] lookAt" << m_camera->lookAt()
        //          << "posWS" << world.map(QVector3D(0,0,0))
        //          << "view m03/m13/m23"
        //          << view(0,3) << view(1,3) << view(2,3)
        //          << "view rot row2"
        //          << view(2,0) << view(2,1) << view(2,2);
    } else {
        view.lookAt({0,0,5}, {0,0,0}, {0,1,0});
        proj.perspective(60.0f, aspect, 0.1f, 1000.0f);
    }

    // sceneNode->setRect(QRectF(0, 0, width(), height()));
    // const qreal dpr = window() ? window()->effectiveDevicePixelRatio() : 1.0;

    // Item's top-left in the QQuickItem scene coordinate system
    // QQuickWindow *W = window();
    // QQuickItem *rootW = W ? W->contentItem() : nullptr;
    // const QPointF tl = mapToItem(rootW, QPointF(0, 0));


    QRect winRectPx;
    {
        // ScopedMicros tm("winRectPx compute", &s_us_rect);

        const qreal dpr = window() ? window()->effectiveDevicePixelRatio() : 1.0;

        QPointF tl;
        if (window() && window()->contentItem()) {
            tl = mapToItem(window()->contentItem(), QPointF(0, 0)); // <-- window coords
        } else {
            tl = QPointF(0, 0);
        }

        const qreal x = tl.x() * dpr;
        const qreal y = tl.y() * dpr;
        const qreal w = width()  * dpr;
        const qreal h = height() * dpr;

        winRectPx = QRect(
            int(std::floor(x)),
            int(std::floor(y)),
            int(std::ceil(w)),
            int(std::ceil(h))
        );
    }

    // QRect winRectPx(
    //     int(std::round(tl.x() * dpr)),
    //     int(std::round(tl.y() * dpr)),
    //     int(std::round(width()  * dpr)),
    //     int(std::round(height() * dpr))
    //     );

    // if (window() && window()->screen())
    //     qDebug() << "screen refreshRate" << window()->screen()->refreshRate();

    // qDebug() << "swapInterval" << window()->format().swapInterval();

    {
        // ScopedMicros tm("sceneNode setters", &s_us_setters);

        sceneNode->setWindowRectPx(winRectPx);
        sceneNode->setScene(m_scene);
        sceneNode->setQuickWindow(window());
        sceneNode->setViewProj(proj * view);
        sceneNode->setCameraWorldPos(m_camPos);
        sceneNode->setEnvironment(m_environment);
    }

    {
        // ScopedMicros tm("syncRenderables", &s_us_sync);
        sceneNode->syncRenderables();
    }

    // s_us_total += totalT.nsecsElapsed() / 1000;
    // s_samples++;

    // // print once per second
    // if (s_aggTimer.elapsed() >= 1000) {
    //     const double inv = (s_samples > 0) ? (1.0 / double(s_samples)) : 0.0;

    //     qDebug().noquote()
    //         << QString("[GXView3D::updatePaintNode] avg(us): total=%1 sync=%2 setters=%3 rect=%4 samples=%5")
    //                .arg(qint64(s_us_total * inv))
    //                .arg(qint64(s_us_sync  * inv))
    //                .arg(qint64(s_us_setters * inv))
    //                .arg(qint64(s_us_rect * inv))
    //                .arg(s_samples);

    //     // reset
    //     s_us_total = s_us_sync = s_us_setters = s_us_rect = 0;
    //     s_samples = 0;
    //     s_aggTimer.restart();
    // }

    // if (m_renderMode == Continuous && canRenderContinuously())
        // update();

    return root;
}

void GXView3D::itemChange(ItemChange change, const ItemChangeData &data)
{
    // First call base so window() is up-to-date
    QQuickItem::itemChange(change, data);

    if (change == ItemSceneChange && data.window) {
        if (m_connectedWindow && m_frameSwappedConn) QObject::disconnect(m_frameSwappedConn);

        m_connectedWindow = data.window;
        if (data.window) {
        // auto fmt = data.window->format();
        // qDebug() << "before swapInterval" << fmt.swapInterval();
        // fmt.setSwapInterval(1);
        // data.window->setFormat(fmt);
        // qDebug() << "after swapInterval" << data.window->format().swapInterval();
        connect(data.window, &QQuickWindow::frameSwapped,
                this, &GXView3D::onFrameSwapped,
                Qt::QueuedConnection);
        }
        update();
        applyRenderPolicy(); // ok to call here
    }

    // if (change == ItemSceneChange && data.window) {
    //     auto *w = data.window;

    //     if (m_frameConn)
    //         disconnect(m_frameConn);

    //     m_frameLimiter.invalidate();

    //     m_frameConn = connect(w, &QQuickWindow::frameSwapped, this, [this]() {
    //         if (!canRenderContinuously())
    //             return;

    //         if (m_renderMode == Continuous) {
    //             update(); // request next frame immediately (vsync paced)
    //             return;
    //         }

    //         if (m_renderMode == Throttled) {
    //             if (!m_frameLimiter.isValid())
    //                 m_frameLimiter.start();

    //             const int intervalMs = qMax(1, 1000 / qMax(1, m_targetFps));
    //             if (m_frameLimiter.elapsed() >= intervalMs) {
    //                 m_frameLimiter.restart();
    //                 update();
    //             }
    //         }
    //     }, Qt::QueuedConnection);
    // }

    // if (change == ItemSceneChange && data.window) {
    //     // connect once per window
    //     connect(data.window, &QQuickWindow::frameSwapped,
    //             this, &GXView3D::onFrameSwapped, Qt::QueuedConnection);

    //     // applyRenderPolicy();
    // }

    // if (change == ItemSceneChange ||
    //     change == ItemVisibleHasChanged ||
    //     change == ItemOpacityHasChanged)
    // {
    //     applyRenderPolicy();
    // }

    // QQuickWindow* w = nullptr;

    // if (change == ItemSceneChange) {
    //     w = data.window;              // reliable only here
    //     if (w) {
    //         QSurfaceFormat fmt = w->format();
    //         fmt.setDepthBufferSize(24);
    //         fmt.setStencilBufferSize(8);
    //         w->setFormat(fmt);
    //     }
    // } else {
    //     w = window();                 // for visibility/opacity changes, use current window()
    // }

    // if (!w)
    //     return;

    // if (change == ItemVisibleHasChanged || change == ItemSceneChange || change == ItemOpacityHasChanged) {
    //     connect(w, &QQuickWindow::beforeSynchronizing,
    //             this, &GXView3D::syncFrameData,
    //             Qt::UniqueConnection);

    //     applyRenderPolicy();
    // }
    // if (window()) {
    //     const QSurfaceFormat fmt = window()->format();
    //     if (fmt.depthBufferSize() <= 0) {
    //         qWarning()
    //         << "GenesisX3D WARNING:"
    //         << "QQuickWindow has no depth buffer."
    //         << "Depth testing will not work correctly."
    //         << "Host application must enable depth via QSurfaceFormat.";
    //     }
    // }

    // if (change == ItemSceneChange && window()) {
    //     QSurfaceFormat fmt = window()->format();
    //     fmt.setDepthBufferSize(24);
    //     fmt.setStencilBufferSize(8);
    //     window()->setFormat(fmt);
    // }
    // QQuickItem::itemChange(change, data);

    // if (change == ItemVisibleHasChanged || change == ItemSceneChange || change == ItemOpacityHasChanged) {
    //     // Q_UNUSED(data);
    //     connect(data.window, &QQuickWindow::beforeSynchronizing, this, &GXView3D::syncFrameData, Qt::DirectConnection);
    //     applyRenderPolicy();
    // }
}

void GXView3D::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);

    QQuickWindow *w = window();
    if (!w) return;

    const qreal dpr = w->effectiveDevicePixelRatio();

    // Window-content coordinates (same space as window()->contentItem())
    const QPointF tl = mapToScene(QPointF(0, 0));   // <-- IMPORTANT (no nullptr)

    const qreal x = tl.x() * dpr;
    const qreal y = tl.y() * dpr;
    const qreal ww = width()  * dpr;
    const qreal hh = height() * dpr;

    const QRect winRectPx(int(std::floor(x)), int(std::floor(y)),
                          int(std::ceil(ww)), int(std::ceil(hh)));

    m_windowRectPx = winRectPx;        // store member
    emit windowRectPxChanged();        // or push to render node in sync step
    update();                          // request redraw
    applyRenderPolicy();
}

void GXView3D::componentComplete()
{
    QQuickItem::componentComplete();
    applyRenderPolicy();
}

void GXView3D::syncFrameData(render::GXSceneRenderNode* rn)
{
    // if (!m_camera) {
    //     m_camPos = QVector3D(0,0,0);
    //     return;
    // }
    // m_camPos = m_camera->worldPosition();
    if (m_camera) {
        m_camPos = m_camera->worldPosition();
        rn->setCameraWorldPos(m_camPos);
        rn->setViewProj(m_camera->viewMatrix());
    } else {
        m_camPos = {};
        rn->setCameraWorldPos(m_camPos);
        rn->setViewProj(QMatrix4x4());
    }

    rn->setScene(m_scene);

    rn->syncRenderables();
}

void GXView3D::applyRenderPolicy()
{
    m_tickTimer.stop();
    m_frameLimiter.invalidate();

    if (!window() || m_renderMode == OnDemand)
        return;

    update();

    // switch (m_renderMode) {
    // case OnDemand:
    //     break;
    // case Continuous: {
    //     const int intervalMs = qMax(1, 1000 / qMax(1, m_targetFps));
    //     qDebug() << "interval" << intervalMs << "targetFps" << m_targetFps;
    //     m_tickTimer.start(intervalMs);
    //     break;
    // }
    // case Throttled: {
    //     const int intervalMs1 = qMax(1, 1000 / qMax(1, m_targetFps));
    //     qDebug() << "interval" << intervalMs1 << "targetFps" << m_targetFps;
    //     m_tickTimer.start(intervalMs1);
    //     break;
    // }
    // }
}

void GXView3D::onFrameSwapped()
{
    // static qint64 lastMs = 0;
    // static qint64 sumDt = 0;
    // static qint64 maxDt = 0;
    // static int samples = 0;

    // const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    // if (lastMs != 0) {
    //     const qint64 dt = nowMs - lastMs;
    //     sumDt += dt;
    //     if (dt > maxDt) maxDt = dt;
    //     samples++;
    // }
    // lastMs = nowMs;

    // static QElapsedTimer printT;
    // if (!printT.isValid()) printT.start();

    // if (printT.elapsed() >= 1000) {
    //     if (samples > 0) {
    //         qDebug() << "frameSwapped avg"
    //                  << (double(sumDt) / double(samples)) << "ms"
    //                  << "max" << maxDt << "ms"
    //                  << "samples" << samples
    //                  << "swapInterval" << window()->format().swapInterval()
    //                  << "refreshRate" << (window()->screen() ? window()->screen()->refreshRate() : -1);
    //     }

    //     sumDt = 0;
    //     maxDt = 0;
    //     samples = 0;
    //     printT.restart();
    // }

    // static QElapsedTimer ft;
    // static bool started = false;
    // static qint64 sum = 0, mx = 0;
    // static int n = 0;

    // if (!started) { ft.start(); started = true; }

    // static qint64 lastMs1 = 0;
    // qint64 nowMs1 = ft.elapsed();
    // qint64 dt = nowMs1 - lastMs1;
    // lastMs1 = nowMs1;

    // sum += dt;
    // mx = qMax(mx, dt);
    // n++;

    // if (n >= 60) {
    //     qDebug() << "frameSwapped avg second debug" << (double(sum)/n) << "ms max" << mx;
    //     sum = 0; mx = 0; n = 0;
    // }

    if (!canRenderContinuously())
        return;

    if (m_renderMode == Continuous) {
        QMetaObject::invokeMethod(this, "update", Qt::QueuedConnection);
        return;
    }

    if (m_renderMode == Throttled) {
        if (!m_frameLimiter.isValid()) m_frameLimiter.start();

        // static QElapsedTimer t;
        // if (!t.isValid()) t.start();

        const int intervalMs = qMax(1, 1000 / qMax(1, m_targetFps));
        if (m_frameLimiter.elapsed() >= intervalMs) {
            m_frameLimiter.restart();
            QMetaObject::invokeMethod(this, "update", Qt::QueuedConnection);
        }
    }

    // if (m_renderMode == Continuous) {
    //     // qDebug() << "tick continuous" << QDateTime::currentMSecsSinceEpoch();
    //     update();
    // } else if (m_renderMode == Throttled) {
    //     // qDebug() << "tick throttled" << QDateTime::currentMSecsSinceEpoch();
    //     // limiter logic here (elapsed >= interval)
    //     update();
    // }
}

bool GXView3D::canRenderContinuously() const
{
    if (!window()) {
        qDebug() << "no window";
        return false;
    }
    if (!window()->isExposed()) {
        return false;
    }
    if (!window()->isVisible()) {
        return false;
    }
    if (!isVisible()) {
        qDebug() << "not visible";
        return false;
    }
    if (width() <= 0 || height() <= 0) {
        qDebug() << "no width or height";
        return false;
    }
    if (opacity() <= 0.0) {
        qDebug() << "no opacity";
        return false;
    }

    return true;
}

query::GXRay GXView3D::makeRayFromItemPos(float x, float y) const
{
    query::GXRay ray;

    const float w = float(width());
    const float h = float(height());
    if (w <= 1.0f || h <= 1.0f || !m_camera) {
        ray.originWS = QVector3D(0,0,0);
        ray.dirWS = QVector3D(0,0,-1);
        return ray;
    }

    const float aspect = w / h;

    // bool okView = false;
    // const QMatrix4x4 view = m_camera->worldMatrix().inverted(&okView);
    const QMatrix4x4 view = m_camera->viewMatrix();
    const QMatrix4x4 proj = m_camera->projectionMatrix(aspect);

    // if (!okView) {
    //     ray.originWS = QVector3D(0,0,0);
    //     ray.dirWS = QVector3D(0,0,-1);
    //     return ray;
    // }

    bool ok = false;
    const QMatrix4x4 invVP = (proj * view).inverted(&ok);
    if (!ok) {
        ray.originWS = QVector3D(0,0,0);
        ray.dirWS = QVector3D(0,0,-1);
        return ray;
    }

    // Qt item coords: (0,0) top-left. NDC: y-up.
    const float ndcX = (2.0f * x / w) - 1.0f;
    const float ndcY = 1.0f - (2.0f * y / h);

    auto unproject = [&](float zClip) -> QVector3D {
        QVector4D pClip(ndcX, ndcY, zClip, 1.0f);
        QVector4D pWorld = invVP * pClip;
        if (!qFuzzyIsNull(pWorld.w()))
            pWorld /= pWorld.w();
        return pWorld.toVector3D();
    };

    // QMatrix4x4 perspective uses OpenGL-style clip Z: -1 near, +1 far
    const QVector3D pNear = unproject(-1.0f);
    const QVector3D pFar  = unproject(+1.0f);

    ray.originWS = pNear;
    ray.dirWS = (pFar - pNear).normalized();
    return ray;
}

// query::GXRay GXView3D::makeRayFromItemPos(float x, float y) const
// {
//     query::GXRay out;

//     const float w = float(width());
//     const float h = float(height());
//     if (w <= 1.0f || h <= 1.0f || !m_camera) {
//         out.originWS = QVector3D(0,0,0);
//         out.dirWS = QVector3D(0,0,-1);
//         return out;
//     }

//     const float aspect = w / h;

//     // const QMatrix4x4 view = m_camera->viewMatrix();
//     bool okView = false;
//     const QMatrix4x4 view = m_camera->worldMatrix().inverted(&okView);
//     const QMatrix4x4 proj = m_camera->projectionMatrix(aspect);

//     bool ok = false;
//     const QMatrix4x4 invVP = (proj * view).inverted(&ok);
//     if (!okView) {
//         out.originWS = QVector3D(0,0,0);
//         out.dirWS = QVector3D(0,0,-1);
//         return out;
//     }

//     // Camera world position (robust, doesn't assume local position API)
//     const QVector3D camPos = m_camera->worldMatrix().map(QVector3D(0,0,0));

//     // Camera forward (prefer target if usable)
//     QVector3D camFwd = (m_camera->lookAt() - camPos);
//     if (camFwd.lengthSquared() < 1e-8f) {
//         // Fallback: derive from view matrix (camera looks down -Z in view space)
//         // Inverse(view) * (0,0,-1,0) gives forward direction
//         bool ok2 = false;
//         const QMatrix4x4 invView = view.inverted(&ok2);
//         if (ok2) {
//             const QVector4D f = invView * QVector4D(0, 0, -1, 0);
//             camFwd = f.toVector3D();
//         } else {
//             camFwd = QVector3D(0,0,-1);
//         }
//     }
//     camFwd.normalize();

//     auto unproject = [&](float ndcX, float ndcY, float zClip) -> QVector3D {
//         QVector4D pClip(ndcX, ndcY, zClip, 1.0f);
//         QVector4D pWorld = invVP * pClip;
//         if (!qFuzzyIsNull(pWorld.w()))
//             pWorld /= pWorld.w();
//         return pWorld.toVector3D();
//     };

//     auto makeCandidate = [&](bool flipY, bool z01) -> query::GXRay {
//         const float ndcX = (2.0f * x / w) - 1.0f;
//         const float ndcY = flipY ? (1.0f - (2.0f * y / h))
//                                  : ((2.0f * y / h) - 1.0f);

//         const float zNear = z01 ? 0.0f : -1.0f;
//         const float zFar  = 1.0f;

//         const QVector3D pNear = unproject(ndcX, ndcY, zNear);
//         Q_UNUSED(pNear);
//         const QVector3D pFar  = unproject(ndcX, ndcY, zFar);

//         query::GXRay r;
//         r.originWS = camPos;                 // key change: origin = camera
//         r.dirWS = (pFar - camPos).normalized();
//         return r;
//     };

//     // Try 4 combos and pick the one most aligned with camera forward
//     query::GXRay best = makeCandidate(true,  false);
//     float bestDot = QVector3D::dotProduct(best.dirWS, camFwd);

//     const query::GXRay c2 = makeCandidate(true,  true);
//     const float d2 = QVector3D::dotProduct(c2.dirWS, camFwd);
//     if (d2 > bestDot) { bestDot = d2; best = c2; }

//     const query::GXRay c3 = makeCandidate(false, false);
//     const float d3 = QVector3D::dotProduct(c3.dirWS, camFwd);
//     if (d3 > bestDot) { bestDot = d3; best = c3; }

//     const query::GXRay c4 = makeCandidate(false, true);
//     const float d4 = QVector3D::dotProduct(c4.dirWS, camFwd);
//     if (d4 > bestDot) { bestDot = d4; best = c4; }

//     // (Optional) debug: you can keep this for one run
//     // qDebug() << "pickRay bestDot=" << bestDot << "dir=" << best.dirWS << "camFwd=" << camFwd;

//     return best;
//     // query::GXRay ray;

//     // const float w = float(width());
//     // const float h = float(height());
//     // if (w <= 1.0f || h <= 1.0f || !m_camera) {
//     //     ray.originWS = QVector3D(0,0,0);
//     //     ray.dirWS = QVector3D(0,0,-1);
//     //     return ray;
//     // }

//     // // Item coords -> NDC
//     // const float ndcX = (2.0f * x / w) - 1.0f;
//     // const float ndcY = 1.0f - (2.0f * y / h);

//     // const float aspect = w / h;

//     // const QMatrix4x4 view = m_camera->viewMatrix();
//     // const QMatrix4x4 proj = m_camera->projectionMatrix(aspect);

//     // bool ok = false;
//     // const QMatrix4x4 invVP = (proj * view).inverted(&ok);
//     // if (!ok) {
//     //     ray.originWS = QVector3D(0,0,0);
//     //     ray.dirWS = QVector3D(0,0,-1);
//     //     return ray;
//     // }

//     // auto unproject = [&](float zClip) -> QVector3D {
//     //     const QVector4D pClip(ndcX, ndcY, zClip, 1.0f);
//     //     QVector4D pWorld = invVP * pClip;
//     //     if (!qFuzzyIsNull(pWorld.w()))
//     //         pWorld /= pWorld.w();
//     //     return pWorld.toVector3D();
//     // };

//     // // With Qt's perspective matrix, clip z in [-1, +1] works for unproject
//     // const QVector3D pNear = unproject(-1.0f);
//     // const QVector3D pFar  = unproject(+1.0f);

//     // ray.originWS = pNear;
//     // ray.dirWS = (pFar - pNear).normalized();
//     // return ray;



//     // query::GXRay ray;

//     // const float w = float(width());
//     // const float h = float(height());
//     // if (w <= 1.0f || h <= 1.0f || !m_camera) {
//     //     ray.originWS = QVector3D(0,0,0);
//     //     ray.dirWS = QVector3D(0,0,-1);
//     //     return ray;
//     // }

//     // // NDC: x in [-1,1], y in [-1,1] (note Qt y-down, NDC y-up)
//     // const float ndcX = (2.0f * x / w) - 1.0f;
//     // const float ndcY = 1.0f - (2.0f * y / h);

//     // // Build view/proj matrices.
//     // // If your GXCamera already has view/projection, replace this block with that.
//     // const QVector3D camPos = m_camera->position();
//     // const QVector3D camTarget = m_camera->target();
//     // const QVector3D camUp = m_camera->up();

//     // QMatrix4x4 view;
//     // view.lookAt(camPos, camTarget, camUp);

//     // QMatrix4x4 proj;
//     // const float aspect = w / h;
//     // proj.perspective(60.0f, aspect, 0.1f, 1000.0f);

//     // const QMatrix4x4 invVP = (proj * view).inverted();

//     // auto unproject = [&](float zClip) -> QVector3D {
//     //     const QVector4D pClip(ndcX, ndcY, zClip, 1.0f);
//     //     QVector4D pWorld = invVP * pClip;
//     //     if (!qFuzzyIsNull(pWorld.w()))
//     //         pWorld /= pWorld.w();
//     //     return pWorld.toVector3D();
//     // };

//     // // Clip-space z: -1 near, +1 far (Qt uses OpenGL-style clip coords in QMatrix4x4)
//     // const QVector3D pNear = unproject(-1.0f);
//     // const QVector3D pFar  = unproject(+1.0f);

//     // ray.originWS = pNear;
//     // ray.dirWS = (pFar - pNear).normalized();

//     // return ray;
// }

void GXView3D::rebuildQueryBackend()
{
    if (!m_scene) {
        m_worldQuery.setBackend(nullptr);
        return;
    }
    m_worldQuery.setBackend(std::make_unique<query::GXSceneQueryBackend>(m_scene));
}
