// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Render/Nodes/GXSceneRenderNode.h>
#include <GenesisX/GX3D/Render/Nodes/GXRenderableNode.h>
#include <GenesisX/GX3D/Render/Nodes/GXModelNode.h>
#include <GenesisX/GX3D/Render/Materials/GXPrincipledMaterial.h>
#include <GenesisX/GX3D/Scene/GXScene.h>
#include <GenesisX/GX3D/Core/GXCamera.h>
#include <GenesisX/GX3D/Scene/Lights/GXDirectionalLight.h>
#include <GenesisX/GX3D/QtQuick/GXEnvironmentBuildJob.h>

#include <GenesisX/GX3D/Query/GXWorldQuery.h>
#include <GenesisX/GX3D/Query/GXSceneQueryBackend.h>

#include <QObject>
#include <QSGRendererInterface>
#include <QElapsedTimer>

#include <QByteArray>
#include <QVector3D>
#include <QtMath>
#include <QtCore/qfloat16.h>

#include <rhi/qrhi_platform.h>
#include <rhi/qrhi.h>

using namespace gx::gx3d::render;

struct ScopeUs {
    qint64 *acc;
    QElapsedTimer t;
    ScopeUs(qint64 *a) : acc(a) { t.start(); }
    ~ScopeUs() { *acc += t.nsecsElapsed() / 1000; }
};

struct ExtCmdGuard {
    QQuickWindow *w = nullptr;
    bool active = false;
    ~ExtCmdGuard() { if (active) w->endExternalCommands(); }
};

struct GizmoV { float x,y,z; };

// 36 vertices (12 triangles) unit cube centered at origin, size 1
static const GizmoV kCubeVerts[36] = {
    // +Z
    {-0.5f,-0.5f, 0.5f}, { 0.5f,-0.5f, 0.5f}, { 0.5f, 0.5f, 0.5f},
    {-0.5f,-0.5f, 0.5f}, { 0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f},
    // -Z
    { 0.5f,-0.5f,-0.5f}, {-0.5f,-0.5f,-0.5f}, {-0.5f, 0.5f,-0.5f},
    { 0.5f,-0.5f,-0.5f}, {-0.5f, 0.5f,-0.5f}, { 0.5f, 0.5f,-0.5f},
    // +X
    { 0.5f,-0.5f, 0.5f}, { 0.5f,-0.5f,-0.5f}, { 0.5f, 0.5f,-0.5f},
    { 0.5f,-0.5f, 0.5f}, { 0.5f, 0.5f,-0.5f}, { 0.5f, 0.5f, 0.5f},
    // -X
    {-0.5f,-0.5f,-0.5f}, {-0.5f,-0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f},
    {-0.5f,-0.5f,-0.5f}, {-0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f,-0.5f},
    // +Y
    {-0.5f, 0.5f, 0.5f}, { 0.5f, 0.5f, 0.5f}, { 0.5f, 0.5f,-0.5f},
    {-0.5f, 0.5f, 0.5f}, { 0.5f, 0.5f,-0.5f}, {-0.5f, 0.5f,-0.5f},
    // -Y
    {-0.5f,-0.5f,-0.5f}, { 0.5f,-0.5f,-0.5f}, { 0.5f,-0.5f, 0.5f},
    {-0.5f,-0.5f,-0.5f}, { 0.5f,-0.5f, 0.5f}, {-0.5f,-0.5f, 0.5f},
};

struct alignas(16) GizmoUBO {
    float mvp[16];
    float color[4];
};

static constexpr int GX_MAX_LIGHTS = 15;

struct alignas(16) GXLightGPU
{
    QVector4D pos;
    QVector4D dir;
    QVector4D color;
    QVector4D params;
};

struct alignas(16) FrameLightingUBO
{
    QVector4D frameParams;
    QVector4D cameraWorldPos;
    GXLightGPU lights[GX_MAX_LIGHTS];
};
static_assert(sizeof(FrameLightingUBO) == sizeof(QVector4D) * 2 + sizeof(GXLightGPU) * GX_MAX_LIGHTS);
static_assert(sizeof(FrameLightingUBO) % 16 == 0, "FrameLightingUBO must be 16-byte aligned");

struct alignas(16) FrameEnvironmentUBO {
    QVector4D ambient_ao;
    QVector4D aoParams;
    QVector4D envSkyDirStr;
    QVector4D envSpecularParams;
    QVector4D brdfLutInvSize;
};
static_assert(sizeof(FrameEnvironmentUBO) % 16 == 0, "FrameEnvironmentUBO must be 16-byte aligned");

static QSize surfacePixelSize(QRhiRenderTarget* rt)
{
    if (auto *swrt = dynamic_cast<QRhiSwapChainRenderTarget *>(rt)) {
        if (auto* sc = swrt->swapChain()) return sc->currentPixelSize();
    }
    return rt? rt->pixelSize() : QSize();
}

static QImage loadFace(const char *path)
{
    QImage img(QString::fromUtf8(path));
    if (img.isNull())
        return img;

    img = img.convertToFormat(QImage::Format_RGBA8888);

    return img;
}

// static inline float saturate(float x) { return x < 0.f ? 0.f : (x > 1.f ? 1.f : x); }
// static inline float smoothstep(float e0, float e1, float x) {
//     float t = saturate((x - e0) / (e1 - e0));
//     return t * t * (3.f - 2.f * t);
// }

// Map (face, u, v) -> direction (OpenGL cubemap convention)
// static QVector3D cubeDir(int face, float u, float v)
// {
//     // u,v in [-1..1]
//     switch (face) {
//     case 0: return QVector3D(+1.f, -v, -u); // +X
//     case 1: return QVector3D(-1.f, -v, +u); // -X
//     case 2: return QVector3D(+u, +1.f, +v); // +Y
//     case 3: return QVector3D(+u, -1.f, -v); // -Y
//     case 4: return QVector3D(+u, -v, +1.f); // +Z
//     case 5: return QVector3D(-u, -v, -1.f); // -Z
//     default: return QVector3D(0, 0, 1);
//     }
// }

// Simple “HDR-ish” sky with a bright sun hotspot.
// Returns linear RGB, can exceed 1.0.
// static QVector3D sampleProceduralEnv(const QVector3D &dirN)
// {
//     // Sky gradient based on "up"
//     float t = saturate(dirN.y() * 0.5f + 0.5f);

//     QVector3D skyBottom(0.02f, 0.02f, 0.03f);
//     QVector3D skyTop   (0.65f, 0.55f, 0.85f); // purple-ish top
//     QVector3D sky = skyBottom * (1.f - t) + skyTop * t;

//     // “Sun” direction and intensity (bright, > 1.0)
//     QVector3D sunDir = QVector3D(-0.25f, 0.75f, 0.60f).normalized();
//     float cosA = QVector3D::dotProduct(dirN, sunDir); // [-1..1]

//     // Tight hotspot + wider bloom
//     float sunCore  = smoothstep(0.995f, 1.0f, cosA);
//     float sunBloom = smoothstep(0.90f, 1.0f, cosA);

//     QVector3D sunColor(8.0f, 6.5f, 7.5f);     // HDR-ish “white-pink”
//     sky += sunColor * (sunCore * 2.0f + sunBloom * 0.25f);

//     return sky;
// }

struct PrefilterParamsUBO
{
    float roughness;   // 0..1
    int faceIndex;     // 0..5
    int sampleCount;   // e.g. 256/64
    int pad;           // alignment
};
static_assert(sizeof(PrefilterParamsUBO) == 16, "PrefilterParamsUBO must be 16 bytes");


// static gx::gx3d::scene::GXNode* pickOwner(gx::gx3d::scene::GXNode* n)
// {
//     for (auto* cur = n; cur; cur = qobject_cast<gx::gx3d::scene::GXNode*>(cur->parent())) {
//         if (cur->pickingId() != 0)
//             return cur;
//     }
//     return n;
// }


// static bool gxExtractFakeEmissionLight(GXMaterial* mat, QColor& outColor, float& outIntensity, float& outRadius)
// {
//     using PM = GXPrincipledMaterial;

//     auto* pm = qobject_cast<PM*>(mat);
//     if (!pm) return false;

//     if (pm->emissionLight() != PM::FakeLight) return false;

//     const QColor c = pm->emissionColor();

//     float strength = pm->emissionStrength();
//     if (strength <= 0.0f) strength = 0.5f;

//     float intensity = pm->emissionLightIntensity();
//     if (intensity <= 0.0f) intensity = strength;

//     float radius = pm->emissionLightRadius();
//     if (radius <= 0.0f) radius = 6.0f;

//     if (intensity <= 0.0f || radius <= 0.0f) return false;

//     outColor = c;
//     outIntensity = intensity;
//     outRadius = radius;

//     return true;
// }

GXSceneRenderNode::GXSceneRenderNode() = default;

GXSceneRenderNode::~GXSceneRenderNode()
{
    forEachRenderable([](GXRenderableNode* r) {
        r->releaseResources();
    });
}

void GXSceneRenderNode::setScene(scene::GXScene *scene)
{
    if (m_scene == scene) return;

    if (m_scene) {
        forEachRenderable([&](GXRenderableNode* r) {
            QObject::disconnect(r, &GXRenderableNode::renderDirty, m_window, nullptr);
        });
    }

    gx::gx3d::query::GXWorldQuery q;
    q.setBackend(std::make_unique<gx::gx3d::query::GXSceneQueryBackend>(scene));

    gx::gx3d::query::GXRay ray;
    ray.originWS = QVector3D(0, 2, 5);
    ray.dirWS    = QVector3D(0, -0.2f, -1).normalized();

    // auto hit = q.raycast(ray);
    // qDebug() << "hit?" << hit.hit << "t=" << hit.t << "node=" << (hit.node ? hit.node->objectName() : QString());


    m_scene = scene;

    if (m_scene && m_window) {
        forEachRenderable([&](GXRenderableNode* r) {
            QObject::connect(r, &GXRenderableNode::renderDirty, m_window, [this]() {
                if (m_window) m_window->update();
            }, Qt::QueuedConnection);
            // QObject::connect(m_scene, &scene::GXScene::nodeAdded, this, [this](scene::GXNode* n) {
            //     m_scene->traverse(n, [&](scene::GXNode* x) {
            //         if (auto* r = qobject_cast<GXRenderableNode*>(x)) {
            //             QObject::connect(r, &GXRenderableNode::renderDirty(), this, &GXSceneRenderNode::requestRender, Qt::QueuedConnection);
            //         }
            //     });
            // });
        });
    }

    requestRender();
}

void GXSceneRenderNode::setQuickWindow(QQuickWindow *w)
{
    if (m_window == w) return;

    // if (m_beforeRenderingConn) QObject::disconnect(m_beforeRenderingConn);
    // if (m_sceneGraphInvalidatedConn) QObject::disconnect(m_sceneGraphInvalidatedConn);

    if (m_beforeRenderingConn) {
        QObject::disconnect(m_beforeRenderingConn);
        m_beforeRenderingConn = {};
    }

    m_window = w;

    if (!m_window) return;

    auto markDirty = [this]() {
        m_depthDirty.store(true, std::memory_order_relaxed);

        // m_brdfLutUploaded = false;
        // m_envCubeUploaded = false;
        // m_prefilterSpecCubeBuilt = false;

        // m_envBuildRequested.store(true, std::memory_order_relaxed);
        // m_envBuilt.store(false, std::memory_order_relaxed);
    };

    // m_sceneGraphInvalidatedConn = QObject::connect(
    //     m_window,
    //     &QQuickWindow::sceneGraphInvalidated,
    //     m_window,
    //     markDirty,
    //     Qt::DirectConnection
    // );

    // m_beforeRenderingConn = QObject::connect(
    //     m_window,
    //     &QQuickWindow::beforeRendering,
    //     m_window,
    //     [this]() {
    //         if (!m_window) return;
    //         qDebug() << "connect after window";

    //         // Only do work when requested (or when not built yet)
    //         if (!m_envBuildRequested.load(std::memory_order_relaxed))
    //             return;
    //         qDebug() << "connect after envbuildrequested";

    //         // IMPORTANT: no beginExternalCommands/endExternalCommands here.
    //         // We are already in the render thread, inside Qt’s frame recording.
    //         buildEnvironment();

    //         m_envBuildRequested.store(false, std::memory_order_relaxed);
    //         m_envBuilt.store(true, std::memory_order_relaxed);
    //     },
    //     Qt::DirectConnection
    // );

    // QObject::connect(m_window, &QQuickWindow::widthChanged, m_window, markDirty, Qt::DirectConnection);
    // QObject::connect(m_window, &QQuickWindow::heightChanged, m_window, markDirty, Qt::DirectConnection);

    QObject::connect(m_window, &QQuickWindow::sceneGraphInvalidated, m_window, markDirty, Qt::DirectConnection);

    // if (!m_prefilterSpecCubeBuilt) {
    // QObject::connect(m_window, &QQuickWindow::beforeRendering, m_window, [this]() {
    //     if (!m_window) return;
    //     buildEnvironment();
    // }, Qt::DirectConnection);
    // }

    // QObject::connect(m_window, &QQuickWindow::sceneGraphInitialized, m_window, markDirty, Qt::DirectConnection);

    if (m_window && m_scene) {
        forEachRenderable([&](GXRenderableNode* r) {
            QObject::disconnect(r, &GXRenderableNode::renderDirty, m_window, nullptr);
            QObject::connect(r, &GXRenderableNode::renderDirty, m_window, [this]() {
                if (m_window) m_window->update();
            }, Qt::QueuedConnection);
        });
        m_window->update();
    }

    // m_window->scheduleRenderJob(new GXEnvironmentBuildJob(this), QQuickWindow::BeforeRenderingStage);
}

void GXSceneRenderNode::render(const RenderState */*state*/)
{
    // static qint64 s_us_total = 0;
    // static qint64 s_us_envUbo = 0;
    // static qint64 s_us_lightTraverse = 0;
    // static qint64 s_us_lightUbo = 0;
    // static qint64 s_us_perRenderableSetters = 0;
    // static qint64 s_us_perRenderableEnsure = 0;
    // static qint64 s_us_perRenderableRecord = 0;
    // static int s_samples = 0;

    // static QElapsedTimer s_printT;
    // static bool s_started = false;
    // if (!s_started) { s_printT.start(); s_started = true; }

    // QElapsedTimer tTotal;
    // tTotal.start();

    m_stats = {};
    if (!m_window || !m_scene) return;

    QRhi* rhi = m_window->rhi();
    if (!rhi) return;

    if (m_lastRhi != rhi) {
        destroyFrameLightUbo();
        m_lastRhi = rhi;
        m_frameLightDirty = true;
        m_environmentDirty = true;
    }

    if (!m_frameLightUbo) {
        m_frameLightUbo = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(FrameLightingUBO));
        if (!m_frameLightUbo->create()) qWarning() << "GXSceneRenderNode: frame light UBO create failed";
        m_frameLightDirty = true;
    }

    if (!m_environmentUbo) {
        m_environmentUbo = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(FrameEnvironmentUBO));
        if (!m_environmentUbo->create()) qWarning() << "GXSceneRenderNode: frame environment UBO create failed";
        m_environmentDirty = true;
    }

    QRhiCommandBuffer *cb = commandBuffer();
    QRhiRenderTarget *rt  = renderTarget();

    if (!cb || !rt)
        return;

    if (m_depthDirty.exchange(false, std::memory_order_relaxed)) {
        destroyDepthTarget();

        forEachRenderable([&](GXRenderableNode *r) {
            r->markForRelease();
        });
        // if (m_window) m_window->update();
        return;
    }

    const bool isSwapchain = dynamic_cast<QRhiSwapChainRenderTarget *>(rt) != nullptr;
    const bool ownsDepth = !isSwapchain; // we only own depth for texture RT path

    if (ownsDepth) {
        const QSize surfaceSz = surfacePixelSize(rt);
        if (m_depthBuffer && m_depthBuffer->pixelSize() != surfaceSz) {
            destroyDepthTarget();
            forEachRenderable([](GXRenderableNode* r) { r->markForRelease(); });
            // if (m_window) m_window->update();
            return;
        }
    }

    // QElapsedTimer t;
    // t.start();

    // auto ns0 = t.nsecsElapsed();

    // auto ns1 = t.nsecsElapsed();

    // qint64 worstObjNs = 0;
    // GXRenderableNode* worstObj = nullptr;


    // using clock = std::chrono::steady_clock;

    // static auto last = clock::now();
    // static auto t0 = clock::now();
    // static qint64 sumUs = 0;
    // static qint64 maxUs = 0;
    // static int count1 = 0;

    // auto now = clock::now();
    // auto gapUs = std::chrono::duration_cast<std::chrono::microseconds>(now - last).count();
    // last = now;

    // sumUs += gapUs;
    // maxUs = std::max<qint64>(maxUs, gapUs);
    // count1++;

    // auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - t0).count();
    // if (elapsedMs >= 1000) {
    //     qDebug() << "Render gap avg" << (count1 ? sumUs / count1 : 0)
    //     << "us max" << maxUs
    //     << "us samples" << count1;
    //     sumUs = 0;
    //     maxUs = 0;
    //     count1 = 0;
    //     t0 = now;
    // }



    // using clock = std::chrono::steady_clock;
    // static clock::time_point last = clock::now();
    // auto now = clock::now();
    // auto gapUs = std::chrono::duration_cast<std::chrono::microseconds>(now - last).count();
    // last = now;

    // if (gapUs > 30000)
    //     qDebug() << "Render thread gap" << gapUs << "us";

    // static qint64 sumUs = 0;
    // static int count1 = 0;
    // static qint64 maxUs = 0;
    // static auto t0 = std::chrono::steady_clock::now();

    // sumUs += gapUs;
    // count1++;
    // maxUs = std::max<qint64>(maxUs, gapUs);

    // auto t1 = std::chrono::steady_clock::now();
    // auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    // if (elapsedMs >= 1000) {
    //     const qint64 avgUs = count1 ? (sumUs / count1) : 0;
    //     qDebug() << "Render gap avg" << avgUs << "us"
    //              << "max" << maxUs << "us"
    //              << "samples" << count1;
    //     sumUs = 0;
    //     count1 = 0;
    //     maxUs = 0;
    //     t0 = t1;
    // }


    ensureDepthTarget(rhi, rt);

    // QRhiRenderTarget *useRt = m_rtWithDepth ? static_cast<QRhiRenderTarget*>(m_rtWithDepth) : rt;
    QRhiRenderTarget *useRt = rt;
    if (!isSwapchain && m_rtWithDepth) useRt = m_rtWithDepth;

    // QRect r = m_windowRectPx;

    // // Fallback to full RT if something is off
    // const QSize ps = useRt ? useRt->pixelSize() : rt->pixelSize();
    // if (!r.isValid() || r.isEmpty())
    //     r = QRect(0, 0, ps.width(), ps.height());

    // // Clamp to RT bounds (important!)
    // r = r.intersected(QRect(0, 0, ps.width(), ps.height()));

    // cb->setViewport(QRhiViewport(float(r.x()), float(r.y()),
    //                              float(r.width()), float(r.height())));
    // cb->setScissor(QRhiScissor(r.x(), r.y(), r.width(), r.height()));

    QRect r = m_windowRectPx;
    const QSize ps = useRt->pixelSize();
    if (!r.isValid() || r.isEmpty())
        r = QRect(0, 0, ps.width(), ps.height());

    r = r.intersected(QRect(0, 0, ps.width(), ps.height()));

    // --- IMPORTANT: flip Y for QRhi coordinate space when needed ---
    QRect rRhi = r;

    // Qt Quick scene coordinates are top-left origin.
    // Some QRhi backends use bottom-left origin for viewport/scissor.
    const bool yUp = rhi->isYUpInFramebuffer();   // <--- THIS exists in QRhi
    if (yUp) {
        rRhi.setY(ps.height() - (r.y() + r.height()));
    }

    // cb->setViewport(QRhiViewport(float(rRhi.x()), float(rRhi.y()), float(rRhi.width()), float(rRhi.height())));
    cb->setViewport(QRhiViewport(float(r.x()), float(r.y()), float(r.width()), float(r.height())));
    // cb->setScissor(QRhiScissor(rRhi.x(), rRhi.y(), rRhi.width(), rRhi.height()));
    cb->setScissor(QRhiScissor(r.x(), r.y(), r.width(), r.height()));

    // qDebug() << "node" << this
    //          << "winRectPx" << m_windowRectPx
    //          << "ps" << ps
    //          << "yUp" << rhi->isYUpInFramebuffer()
    //          << "rRhi" << rRhi;



    const QRect sc = m_windowRectPx;
    // QRect r = m_windowRectPx;

    // if (!r.isValid() || r.isEmpty()) {
    //     const QSize ps = (useRt ? useRt : rt)->pixelSize();
    //     r = QRect(0, 0, ps.width(), ps.height());
    // }

    // cb->setViewport(QRhiViewport(float(r.x()), float(r.y()),
    //                              float(r.width()), float(r.height())));
    // cb->setScissor(QRhiScissor(r.x(), r.y(), r.width(), r.height()));

    // qDebug() << "GXSceneRenderNode" << this
    //          << "scissorEnabled" << state->scissorEnabled()
    //          << "scissorRect" << state->scissorRect();


    if (ownsDepth) {
        const QSize surfaceSz2 = surfacePixelSize(useRt);
        if (m_depthBuffer && m_depthBuffer->pixelSize() != surfaceSz2) {
            qWarning() << "Depth still mismatched after rebuild:"
                       << "depth=" << m_depthBuffer->pixelSize()
                       << "surface=" << surfaceSz2
                       << "-> skipping frame";
            destroyDepthTarget();
            forEachRenderable([](GXRenderableNode* r) { r->markForRelease(); });
            // if (m_window) m_window->update();
            return;
        }
    }

    FrameEnvironmentUBO fe{};
    if (m_environment) {
        fe.ambient_ao = QVector4D(
            m_environment->ambientIntensity(),
            m_environment->aoStrength(),
            0.0f,
            0.0f
        );

        fe.aoParams = QVector4D(
            m_environment->aoRadius(),
            m_environment->aoSoftness(),
            0.0f,
            0.0f
        );

        fe.envSkyDirStr = QVector4D(
            m_environment->skySpecularDirection(),
            0.0f
        );

        fe.envSpecularParams = QVector4D(
            float(m_environment->specularSource()),
            m_environment->skySpecularIntensity(),
            0.0f,
            m_maxMip
        );

        fe.brdfLutInvSize = QVector4D(
            1.0f / m_brdfInvSize,
            1.0f / m_brdfInvSize,
            0.0f,
            0.0f
        );
    } else {
        fe.ambient_ao     = QVector4D(0.03f, 0.0f, 0.0f, 0.0f);
        fe.aoParams       = QVector4D(0.0f,  0.0f, 0.0f, 0.0f);
        fe.envSkyDirStr  = QVector4D(-0.4f, 0.7f, 0.0f, 0.35f);
    }

    if (m_environmentDirty) {
        // ScopeUs _(&s_us_envUbo);

        QRhiResourceUpdateBatch* u = rhi->nextResourceUpdateBatch();
        u->updateDynamicBuffer(m_environmentUbo, 0, sizeof(FrameEnvironmentUBO), &fe);
        cb->resourceUpdate(u);
        m_environmentDirty = false;
    }

    FrameLightingUBO fl{};
    const int requested = m_scene ? m_scene->maxLights() : 8;
    const int used = qMin(requested, GX_MAX_LIGHTS);

    if (requested > GX_MAX_LIGHTS) {
        qWarning() << "GXScene maxLights=" << requested
                   << "exceeds shader cap" << GX_MAX_LIGHTS
                   << "clamping.";
    }



    {
        // ScopeUs _(&s_us_lightTraverse);
        int count = 0;
    m_scene->traverse([&](scene::GXNode* n) {
        if (count >= used) return;

        // point light
        if (auto* p = qobject_cast<scene::GXPointLight*>(n)) {
            const QVector3D posWS = p->worldMatrix().map(QVector3D(0, 0, 0));
            const QVector3D col = QVector3D(p->color().redF(), p->color().greenF(), p->color().blueF());

            auto &L = fl.lights[count++];
            L.pos = QVector4D(posWS, 0.0f);
            L.dir = QVector4D(0.0f, 0.0f, 0.0f, 0.0f);
            L.color = QVector4D(col, p->intensity());
            L.params = QVector4D(p->range(), -1.0f, 1.0f, 0.0f);

            return;
        }

        // spot light
        if (auto* s = qobject_cast<scene::GXSpotLight*>(n)) {
            const QVector3D posWS = s->worldMatrix().map(QVector3D(0,0,0));
            const QVector3D dirWS = s->directionWS().normalized();
            const QVector3D col = QVector3D(s->color().redF(), s->color().greenF(), s->color().blueF());

            // angles are degrees in API
            const float inner = std::cos(qDegreesToRadians(s->innerConeAngle()));
            const float outer = std::cos(qDegreesToRadians(s->outerConeAngle()));
            const float cIn = qMax(inner, outer);
            const float cOut = qMin(inner, outer);

            auto &L = fl.lights[count++];
            L.pos = QVector4D(posWS, 1.0f);
            L.dir = QVector4D(dirWS, 0.0f);
            L.color = QVector4D(col, s->intensity());
            L.params = QVector4D(s->range(), cIn, cOut, 0.0f);

            return;
        }

        // directional light
        if (auto* d = qobject_cast<scene::GXDirectionalLight*>(n)) {
            if (count >= used) return;

            QVector3D dirWS = d->direction().normalized();
            QVector3D col(
                d->color().redF(),
                d->color().greenF(),
                d->color().blueF()
                );

            auto& L = fl.lights[count++];
            L.pos    = QVector4D(0, 0, 0, 2.0f);           // type = 2 (directional)
            L.dir    = QVector4D(dirWS, 0.0f);
            L.color  = QVector4D(col, d->intensity());
            L.params = QVector4D(0, 0, 0, 0);

            return;
        }


        // qDebug() << n->objectName();
        // check emission materials
        // if (auto* m = qobject_cast<GXModel*>(n)) {
        //     auto mats = m->materials();

        // }
    });


    fl.cameraWorldPos = QVector4D(
        m_cameraWorldPos.x(),
        m_cameraWorldPos.y(),
        m_cameraWorldPos.z(),
        1.0f
        );

    fl.frameParams = QVector4D(float(count), (m_scene && m_scene->debugLighting()) ? 0.15f : 0.0f, 0.0f, 0.0f);
    }

    {
        // ScopeUs _(&s_us_lightUbo);
        QRhiResourceUpdateBatch* u = rhi->nextResourceUpdateBatch();
        const quint32 bytes = sizeof(FrameLightingUBO);
        u->updateDynamicBuffer(m_frameLightUbo, 0, bytes, &fl);
        cb->resourceUpdate(u);
    }

    // static qint64 worstNs = 0;
    // QElapsedTimer t; t.start();

    // // ... your render work ...

    // const qint64 ns = t.nsecsElapsed();
    // qDebug() << "GX3D render CPU" << (ns / 1000) << "us";

    // worstNs = std::max(worstNs, ns);
    // if (ns > 2'000'000) // >2ms
    //     qDebug() << "GX3D render CPU" << (ns/1e6) << "ms worst" << (worstNs/1e6) << "ms";
    // auto us = t.nsecsElapsed() / 1000;
    // worstUs = std::max(worstUs, us);
    // // if (us > 20000) // >20ms ~ misses 60fps
    // qDebug() << "GX3D render took" << us/1000.0 << "ms (worst" << worstUs/1000.0 << "ms)";



    forEachRenderable([&](GXRenderableNode *r) {
        // QElapsedTimer tt; tt.start();
        {
            // ScopeUs _(&s_us_perRenderableSetters);

            r->setViewProj(m_viewProj);

            r->setEnvCubeTex(m_envCubeTex);
            r->setEnvCubeSampler(m_envCubeSampler);

            r->setPrefilterSpecCubeTex(m_prefilterSpecCubeTex);
            r->setPrefilterSpecCubeSampler(m_prefilterSpecCubeSampler);

            r->setIrradianceCubeTex(m_irradianceCubeTex);
            r->setIrradianceCubeSampler(m_irradianceCubeSampler);

            r->setBrdfLutTex(m_brdfLutTex);
            r->setBrdfLutSampler(m_brdfLutSampler);

            r->setFrameEnvironmentUbo(m_environmentUbo);
            r->setFrameLightingUbo(m_frameLightUbo);
        // r->syncFromScene();
        }
        {
            // ScopeUs _(&s_us_perRenderableEnsure);
            r->ensureResources(rhi, useRt, cb);
        }
        {
            // ScopeUs _(&s_us_perRenderableRecord);
            r->recordRender(cb, useRt, sc);
        }
        // qint64 ns = tt.nsecsElapsed();
        // if (ns > worstObjNs) { worstObjNs = ns; worstObj = r; }


    });

    // s_us_total += tTotal.nsecsElapsed() / 1000;
    // s_samples++;

    // if (s_printT.elapsed() >= 1000) {
    //     const double inv = s_samples ? 1.0 / double(s_samples) : 0.0;

    //     qDebug().noquote()
    //         << QString("[GXSceneRenderNode] avg(us): total=%1 envUbo=%2 lightTraverse=%3 lightUbo=%4 set=%5 ensure=%6 record=%7 samples=%8")
    //                .arg(qint64(s_us_total * inv))
    //                .arg(qint64(s_us_envUbo * inv))
    //                .arg(qint64(s_us_lightTraverse * inv))
    //                .arg(qint64(s_us_lightUbo * inv))
    //                .arg(qint64(s_us_perRenderableSetters * inv))
    //                .arg(qint64(s_us_perRenderableEnsure * inv))
    //                .arg(qint64(s_us_perRenderableRecord * inv))
    //                .arg(s_samples);

    //     s_us_total = s_us_envUbo = s_us_lightTraverse = s_us_lightUbo = 0;
    //     s_us_perRenderableSetters = s_us_perRenderableEnsure = s_us_perRenderableRecord = 0;
    //     s_samples = 0;
    //     s_printT.restart();
    // }

    // qDebug() << "pipeline count" << m_stats.pipelinesCreated;

    // if (worstObjNs > 2'000'000) { // >2ms
    //     qDebug() << "Worst renderable" << worstObj
    //              << "took" << (worstObjNs/1000) << "us";
    // }

    // auto ns2 = t.nsecsElapsed();

    // // (C) end-of-pass / submit (if you do it here)
    // // ...

    // auto ns3 = t.nsecsElapsed();

    // qDebug() << "GX3D"
    //          << "setup" << (ns1-ns0)/1000 << "us"
    //          << "draw"  << (ns2-ns1)/1000 << "us"
    //          << "end"   << (ns3-ns2)/1000 << "us"
    //          << "total" << (ns3-ns0)/1000 << "us";


    // cb->endPass();

    // if (m_pickRt && m_pickPs && m_pickSrb && m_pickUbuf) {
    //     // QRhiResourceUpdateBatch* u = rhi->nextResourceUpdateBatch();
    //     m_window->beginExternalCommands();
    //     cb->beginPass(m_pickRt, Qt::transparent, { 1.0f, 0}, nullptr);

    //     const QSize ps = m_pickRt->pixelSize();
    //     cb->setViewport(QRhiViewport(0, 0, float(ps.width()), float(ps.height())));
    //     cb->setScissor(QRhiScissor(0, 0, ps.width(), ps.height()));

    //     cb->setGraphicsPipeline(m_pickPs);
    //     cb->setShaderResources(m_pickSrb);

    //     forEachRenderable([&](GXRenderableNode* rn) {
    //         auto* model = qobject_cast<GXModel*>(rn);
    //         if (!model) return;

    //         auto* owner = pickOwner(rn);
    //         qDebug() << "pick rn" << owner->objectName() << owner->pickingId();

    //         QMatrix4x4 mvp = m_viewProj * model->worldMatrix();

    //         struct PickU {
    //             float mvp[16];
    //             quint32 id[4];
    //         } pu;

    //         const float* mm = mvp.constData();
    //         for (int i = 0; i < 16; ++i) pu.mvp[i] = mm[i];
    //         pu.id[0] = owner->pickingId();
    //         pu.id[1] = 0;
    //         pu.id[2] = 0;
    //         pu.id[3] = 0;

    //         QRhiResourceUpdateBatch* ru = rhi->nextResourceUpdateBatch();
    //         ru->updateDynamicBuffer(m_pickUbuf, 0, sizeof(PickU), &pu);
    //         cb->resourceUpdate(ru);

    //         cb->setGraphicsPipeline(m_pickPs);
    //         cb->setShaderResources(m_pickSrb);

    //         model->recordPick(cb);
    //         qDebug() << "end traverse";
    //     });
    //     cb->endPass();
    //     m_window->endExternalCommands();
    // }

    m_frameLightDirty = false;
    m_environmentDirty = false;

    // if (firstLight) {
    //     if (firstLight) {
    //         qDebug() << "PointLight pos =" << firstLight->position()
    //         << "intensity =" << firstLight->intensity()
    //         << "range =" << firstLight->range();
    //     }
    //     ensureLightGizmo(rhi, cb, useRt);
    //     if (m_gizmoPs && m_gizmoVbuf && m_gizmoUbuf) {

    //         GizmoUBO g{};
    //         QMatrix4x4 gizmoModel;
    //         gizmoModel.translate(light.positionWS);

    //         QMatrix4x4 gizmoMvp = rhi->clipSpaceCorrMatrix() * (m_viewProj * gizmoModel);
    //         memcpy(g.mvp, gizmoMvp.constData(), 16 * sizeof(float));

    //         // bright yellow
    //         g.color[0] = 1.0f; g.color[1] = 1.0f; g.color[2] = 0.0f; g.color[3] = 1.0f;

    //         QRhiResourceUpdateBatch* u = rhi->nextResourceUpdateBatch();
    //         u->updateDynamicBuffer(m_gizmoUbuf, 0, sizeof(GizmoUBO), &g);
    //         cb->resourceUpdate(u);

    //         cb->setGraphicsPipeline(m_gizmoPs);
    //         cb->setShaderResources(m_gizmoSrb);

    //         const QRhiCommandBuffer::VertexInput vb(m_gizmoVbuf, 0);
    //         cb->setVertexInput(0, 1, &vb);

    //         const QSize ps = useRt->pixelSize();
    //         cb->setViewport(QRhiViewport(0, 0, float(ps.width()), float(ps.height())));
    //         cb->setScissor(QRhiScissor(0, 0, ps.width(), ps.height()));

    //         cb->draw(6); // 6 vertices => 3 line segments
    //     }
    // }

}

void GXSceneRenderNode::requestRender()
{
    if (m_window) m_window->update();
}

void GXSceneRenderNode::forEachRenderable(const std::function<void (GXRenderableNode *)> &fn) const
{
    auto& ren = m_renderables;
    for (const auto& pr : ren) {
        GXRenderableNode* r = pr.data();
        if (pr.isNull()) return;
        if (!r) continue;
        fn(r);
    }
    // if (!m_scene) return;

    // m_scene->traverse([&](scene::GXNode* n){
    //     if (auto* r = qobject_cast<GXRenderableNode*>(n)) fn(r);
    // });
}

void GXSceneRenderNode::destroyDepthTarget()
{
    if (m_rpDesc) { m_rpDesc->destroy(); delete m_rpDesc; m_rpDesc = nullptr; }
    if (m_rtWithDepth) { m_rtWithDepth->destroy(); delete m_rtWithDepth; m_rtWithDepth = nullptr; }
    if (m_depthBuffer) { m_depthBuffer->destroy(); delete m_depthBuffer; m_depthBuffer = nullptr; }
    m_lastSize = {};
    m_lastSampleCount = 1;
    m_lastSwapChain = nullptr;
}

void GXSceneRenderNode::ensureLightGizmo(QRhi *rhi, QRhiCommandBuffer* cb, QRhiRenderTarget *rt)
{
    if (!rhi || !rt) return;

    if (m_gizmoRhi == rhi && m_gizmoPs && !m_gizmoDirty)
        return;

    destroyLightGizmo();
    m_gizmoRhi = rhi;

    // A little 3D cross (3 axes), drawn as lines (6 vertices).
    // struct V { float x,y,z; };
    // const float s = 0.1f; // gizmo size in world units (tweak)
    // const V verts[] = {
    //     {-s, 0, 0}, {+s, 0, 0},  // X axis
    //     {0, -s, 0}, {0, +s, 0},  // Y axis
    //     {0, 0, -s}, {0, 0, +s},  // Z axis
    // };

    m_gizmoVbuf = rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(kCubeVerts));
    m_gizmoVbuf->create();

    {
        QRhiResourceUpdateBatch* u = rhi->nextResourceUpdateBatch();
        u->uploadStaticBuffer(m_gizmoVbuf, kCubeVerts);
        cb->resourceUpdate(u);
    }

    // {
    //     QRhiResourceUpdateBatch* u = rhi->nextResourceUpdateBatch();
    //     u->uploadStaticBuffer(m_gizmoVbuf, verts);
    //     // We'll submit this batch in render() using cb->resourceUpdate(...)
    //     // (same pattern you use elsewhere)
    //     // We'll store it on the side by returning it to the caller.
    //     // To keep this function simple, we’ll just do it later in render() the first time.
    //     // If you prefer, I can show the "immediate upload" pattern too.
    //     // For now: mark dirty, and upload in render() right after ensureLightGizmo().
    //     // (We’ll do it below.)
    // }

    m_gizmoUbuf = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(GizmoUBO));
    m_gizmoUbuf->create();

    m_gizmoSrb = rhi->newShaderResourceBindings();
    m_gizmoSrb->setBindings({
        QRhiShaderResourceBinding::uniformBuffer(0,
                                                 QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage,
                                                 m_gizmoUbuf)
    });
    m_gizmoSrb->create();

    // Load/compile your gizmo shaders the same way you already do for GXMaterial.
    // I’ll assume you can get QRhiShaderStage::Vertex/Fragment blobs as:
    //   loadShader(":/shaders/light_gizmo.vert.qsb") etc.
    // Replace these with your actual shader-loading calls.
    qDebug() << "and gismo??";
    const QShader vs = m_shaderUtils.gxLoadShader(":/gx3d/shaders/light_gizmo.vert.qsb");
    const QShader fs = m_shaderUtils.gxLoadShader(":/gx3d/shaders/light_gizmo.frag.qsb");

    m_gizmoPs = rhi->newGraphicsPipeline();
    m_gizmoPs->setShaderStages({
        { QRhiShaderStage::Vertex, vs },
        { QRhiShaderStage::Fragment, fs }
    });

    QRhiVertexInputLayout inputLayout;
    inputLayout.setBindings({ QRhiVertexInputBinding(sizeof(GizmoV)) });
    inputLayout.setAttributes({
        QRhiVertexInputAttribute(0, 0, QRhiVertexInputAttribute::Float3, 0)
    });

    m_gizmoPs->setVertexInputLayout(inputLayout);
    m_gizmoPs->setShaderResourceBindings(m_gizmoSrb);
    m_gizmoPs->setTopology(QRhiGraphicsPipeline::Lines);
    m_gizmoPs->setRenderPassDescriptor(rt->renderPassDescriptor());
    m_gizmoPs->setSampleCount(rt->sampleCount());

    // Depth test ON so it disappears behind objects; depth write OFF so it doesn't mess with depth.
    m_gizmoPs->setDepthTest(true);
    m_gizmoPs->setDepthWrite(false);
    m_gizmoPs->setDepthOp(QRhiGraphicsPipeline::LessOrEqual);

    if (!m_gizmoPs->create())
        qWarning() << "Light gizmo pipeline create failed";

    m_gizmoDirty = false;
}

void GXSceneRenderNode::destroyLightGizmo()
{
    if (m_gizmoPs) { m_gizmoPs->destroy(); delete m_gizmoPs; m_gizmoPs = nullptr; }
    if (m_gizmoSrb) { m_gizmoSrb->destroy(); delete m_gizmoSrb; m_gizmoSrb = nullptr; }
    if (m_gizmoUbuf) { m_gizmoUbuf->destroy(); delete m_gizmoUbuf; m_gizmoUbuf = nullptr; }
    if (m_gizmoVbuf) { m_gizmoVbuf->destroy(); delete m_gizmoVbuf; m_gizmoVbuf = nullptr; }
    m_gizmoRhi = nullptr;
    m_gizmoDirty = true;
}

void GXSceneRenderNode::ensureBrdfLut(QRhi *rhi, QRhiCommandBuffer* cb)
{
    if (m_brdfLutTex && m_brdfLutSampler)
        return;

    QImage img(":/gx3d/assets/brdf_lut_64.png");
    if (img.isNull()) {
        qWarning() << "BRDF LUT image missing";
        return;
    }

    img = img.convertToFormat(QImage::Format_RGBA8888);

    m_brdfLutTex = rhi->newTexture(QRhiTexture::RGBA8, img.size(), 1);
    m_brdfLutTex->create();

    m_brdfLutSampler = rhi->newSampler(QRhiSampler::Linear, QRhiSampler::Linear,
                                       QRhiSampler::None,
                                       QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge);
    m_brdfLutSampler->create();

    // upload
    QRhiResourceUpdateBatch *u = rhi->nextResourceUpdateBatch();
    u->uploadTexture(m_brdfLutTex, img);

    cb->resourceUpdate(u);

    m_brdfLutUploaded = true;
}

void GXSceneRenderNode::ensureEnvCube(QRhi *rhi, QRhiCommandBuffer *cb)
{
    if (m_envCubeTex && m_envCubeSampler && m_envCubeUploaded)
        return;

    // +X, -X, +Y, -Y, +Z, -Z
    QImage faces[6] = {
        loadFace(":/gx3d/assets/envcube/posx.png"),
        loadFace(":/gx3d/assets/envcube/negx.png"),
        loadFace(":/gx3d/assets/envcube/posy.png"),
        loadFace(":/gx3d/assets/envcube/negy.png"),
        loadFace(":/gx3d/assets/envcube/posz.png"),
        loadFace(":/gx3d/assets/envcube/negz.png"),
    };

    for (int i = 0; i < 6; ++i) {
        if (faces[i].isNull())
            return;
    }

    const QSize sz = faces[0].size();
    for (int i = 1; i < 6; ++i) {
        if (faces[i].size() != sz) {
            qWarning() << "EnvCube face size mismatch" << i << faces[i].size() << "expected" << sz;
            return;
        }
    }

    if (!m_envCubeTex) {
        QRhiTexture::Flags tf;
        tf |= QRhiTexture::CubeMap;
        tf |= QRhiTexture::MipMapped;              // <-- THIS is what you were missing
        tf |= QRhiTexture::UsedWithGenerateMips;

        m_envCubeTex = rhi->newTexture(QRhiTexture::RGBA8, sz, 1, tf);
        if (!m_envCubeTex->create()) {
            qWarning() << "Failed to create env cubemap texture";
            m_envCubeTex = nullptr;
            return;
        }
    }

    if (!m_envCubeSampler) {
        m_envCubeSampler = rhi->newSampler(
            QRhiSampler::Linear,   // mag
            QRhiSampler::Linear,   // min
            QRhiSampler::Linear,   // mip  (important)
            QRhiSampler::ClampToEdge,
            QRhiSampler::ClampToEdge,
            QRhiSampler::ClampToEdge
            );
        if (!m_envCubeSampler->create()) {
            qWarning() << "Failed to create env cubemap sampler";
            m_envCubeSampler = nullptr;
            return;
        }
    }

    // Upload all 6 faces as layers, mip level 0
    QRhiTextureUploadDescription uploadDesc;
    QVector<QRhiTextureUploadEntry> entries;
    entries.reserve(6);

    for (int face = 0; face < 6; ++face) {
        QRhiTextureSubresourceUploadDescription sub(faces[face]);
        entries.push_back(QRhiTextureUploadEntry(face /*layer*/, 0 /*level*/, sub));
    }
    uploadDesc.setEntries(entries.cbegin(), entries.cend());

    QRhiResourceUpdateBatch *u = rhi->nextResourceUpdateBatch();
    u->uploadTexture(m_envCubeTex, uploadDesc);

    // Generate mip chain on GPU
    u->generateMips(m_envCubeTex);

    cb->resourceUpdate(u);
    m_envCubeUploaded = true;

}

void GXSceneRenderNode::ensurePrefilterSpecCube(QRhi *rhi, QRhiCommandBuffer *cb)
{
    if (m_prefilterSpecCubeTex && m_prefilterSpecCubeSampler && m_prefilterSpecCubeBuilt)
        return;

    // ensurePrefilterPipeline(rhi);
    ensureEnvCube(rhi, cb);
    ensurePrefilterParamsUbo(rhi);
    ensurePrefilterSrb(rhi);

#ifdef Q_OS_WINDOWS
    m_brdfInvSize = 256;
#else
    m_brdfInvSize = 128;
#endif

    if (!m_prefilterSpecCubeTex) {
        QRhiTexture::Flags tf;
        tf |= QRhiTexture::CubeMap;
        tf |= QRhiTexture::MipMapped;
        tf |= QRhiTexture::RenderTarget;
        tf |= QRhiTexture::UsedWithLoadStore;


        m_prefilterSpecCubeTex = rhi->newTexture(QRhiTexture::RGBA16F, QSize(m_brdfInvSize, m_brdfInvSize), 1, tf);
        if (!m_prefilterSpecCubeTex->create()) {
            qWarning() << "Failed to create prefilterSpec cubemap texture";
            m_prefilterSpecCubeTex = nullptr;
            return;
        }
    }

    if (!m_prefilterSpecCubeSampler) {
        m_prefilterSpecCubeSampler = rhi->newSampler(
            QRhiSampler::Linear,   // mag
            QRhiSampler::Linear,   // min
            QRhiSampler::Linear,   // mip  (important)
            QRhiSampler::ClampToEdge,
            QRhiSampler::ClampToEdge,
            QRhiSampler::ClampToEdge
            );
        if (!m_prefilterSpecCubeSampler->create()) {
            qWarning() << "Failed to create prefilterSpec cubemap sampler";
            m_prefilterSpecCubeSampler = nullptr;
            return;
        }
    }

    int mipCount = 1;
    for (int s = m_brdfInvSize; s > 1; s >>= 1) ++mipCount;
    m_maxMip = float(mipCount - 1);

    int mip = 0;
    int face = 0;

    for (mip = 0; mip < mipCount; ++mip) {
        int mipSize = m_brdfInvSize >> mip;
        float roughness = float(mip) / float(mipCount - 1);

        for (face = 0; face < 6; ++face) {
            renderIntoCubeFaceMip(rhi, cb, face, mip, mipSize, roughness);
        }
    }

    m_prefilterSpecCubeBuilt = true;
}

void GXSceneRenderNode::prepare()
{
    if (m_envBuilt) return;

    if (!m_window || !m_scene) return;

    QRhi *rhi = m_window->rhi();
    if (!rhi) return;

    QRhiCommandBuffer *cb = commandBuffer();
    if (!cb) return;

    if (!m_brdfLutUploaded) ensureBrdfLut(rhi, cb);
    if (!m_envCubeUploaded) ensureEnvCube(rhi, cb);
    if (!m_prefilterSpecCubeBuilt) ensurePrefilterSpecCube(rhi, cb);
    if (!m_irradianceCubeUploaded) ensureIrradianceCube(rhi, cb);

    m_envBuilt = true;
}

void GXSceneRenderNode::ensurePrefilterPipeline(QRhi *rhi, QRhiRenderPassDescriptor *rp)
{
    if (m_prefilterPipeline && m_prefilterRp == rp) return;

    if (!m_prefilterSrb) {
        qWarning() << "Prefilter SRB missing before pipeline creation";
        return;
    }

    if (!m_prefilterPipeline) m_prefilterPipeline = rhi->newGraphicsPipeline();

    m_prefilterRp = rp;

    m_prefilterPipeline->setTopology(QRhiGraphicsPipeline::Triangles);

    QRhiShaderStage stages[] = {
        { QRhiShaderStage::Vertex,  m_shaderUtils.gxLoadShader(":/gx3d/shaders/prefilter_spec.vert.qsb") },
        { QRhiShaderStage::Fragment, m_shaderUtils.gxLoadShader(":/gx3d/shaders/prefilter_spec.frag.qsb") }
    };

    m_prefilterPipeline->setShaderStages(std::begin(stages), std::end(stages));

    QRhiVertexInputLayout inputLayout; // no VBO
    m_prefilterPipeline->setVertexInputLayout(inputLayout);

    m_prefilterPipeline->setCullMode(QRhiGraphicsPipeline::None);
    m_prefilterPipeline->setDepthTest(false);
    m_prefilterPipeline->setDepthWrite(false);

    m_prefilterPipeline->setShaderResourceBindings(m_prefilterSrb);

    m_prefilterPipeline->setRenderPassDescriptor(rp);

    if (!m_prefilterPipeline->create())
        qWarning() << "Failed to create prefilter pipeline";
}

void GXSceneRenderNode::ensurePrefilterSrb(QRhi *rhi)
{
    if (m_prefilterSrb)
        return;

    m_prefilterSrb = rhi->newShaderResourceBindings();

    m_prefilterSrb->setBindings({
        QRhiShaderResourceBinding::sampledTexture(
            0, QRhiShaderResourceBinding::FragmentStage,
            m_envCubeTex, m_envCubeSampler
            ),
        QRhiShaderResourceBinding::uniformBuffer(
            1, QRhiShaderResourceBinding::FragmentStage,
            m_prefilterParamsUbo
            )
    });

    m_prefilterSrb->create();
}

void GXSceneRenderNode::ensurePrefilterParamsUbo(QRhi *rhi)
{
    if (m_prefilterParamsUbo)
        return;

    m_prefilterParamsUbo = rhi->newBuffer(QRhiBuffer::Dynamic,
                                          QRhiBuffer::UniformBuffer,
                                          sizeof(PrefilterParamsUBO));
    if (!m_prefilterParamsUbo->create()) {
        qWarning() << "Failed to create prefilter params UBO";
        delete m_prefilterParamsUbo;
        m_prefilterParamsUbo = nullptr;
    }
}

void GXSceneRenderNode::renderIntoCubeFaceMip(QRhi *rhi, QRhiCommandBuffer *cb, int face, int mip, int mipSize, float roughness)
{
    if (!m_prefilterSpecCubeTex)
        return;

    // Attach output cubemap face+mip as render target
    QRhiColorAttachment ca(m_prefilterSpecCubeTex);
    ca.setLayer(face);
    ca.setLevel(mip);

    QRhiTextureRenderTargetDescription rtDesc(ca);
    QRhiTextureRenderTarget *rt = rhi->newTextureRenderTarget(rtDesc);
    QRhiRenderPassDescriptor *rp = rt->newCompatibleRenderPassDescriptor();
    rt->setRenderPassDescriptor(rp);

    if (!rt->create()) {
        qWarning() << "Failed to create RT for prefilter face/mip" << face << mip;
        delete rp;
        delete rt;
        return;
    }
    ensurePrefilterPipeline(rhi, rt->renderPassDescriptor());

    // Update uniforms for this face/mip (roughness + faceIndex + sampleCount)
    // (We’ll define the UBO layout below)
    updatePrefilterParams(rhi, cb, face, roughness);

    const QColor clearColor(0, 0, 0, 255);
    cb->beginPass(rt, clearColor, {1.0f, 0}, nullptr);

    cb->setGraphicsPipeline(m_prefilterPipeline);
    cb->setShaderResources(m_prefilterSrb);
    cb->setViewport({0, 0, float(mipSize), float(mipSize)});
    cb->draw(3); // fullscreen triangle is better than draw(4)

    cb->endPass();
}

void GXSceneRenderNode::updatePrefilterParams(QRhi *rhi, QRhiCommandBuffer *cb, int face, float roughness)
{
    if (!m_prefilterParamsUbo) {
        // Dynamic is fine: we update it per face+mip
        m_prefilterParamsUbo = rhi->newBuffer(QRhiBuffer::Dynamic,
                                              QRhiBuffer::UniformBuffer,
                                              sizeof(PrefilterParamsUBO));
        if (!m_prefilterParamsUbo->create()) {
            qWarning() << "Failed to create prefilter params UBO";
            delete m_prefilterParamsUbo;
            m_prefilterParamsUbo = nullptr;
            return;
        }
    }

    PrefilterParamsUBO p {};
    p.roughness = roughness;
    p.faceIndex = face;

#ifdef Q_OS_WINDOWS
    p.sampleCount = m_prefilterSamplesDesktop; // e.g. 256
#else
    p.sampleCount = m_prefilterSamplesMobile;  // e.g. 64
#endif

    // Upload via resource update batch
    QRhiResourceUpdateBatch *u = rhi->nextResourceUpdateBatch();
    u->updateDynamicBuffer(m_prefilterParamsUbo, 0, sizeof(PrefilterParamsUBO), &p);
    cb->resourceUpdate(u);
}

void GXSceneRenderNode::ensureIrradianceCube(QRhi *rhi, QRhiCommandBuffer *cb)
{
    if (m_irradianceCubeTex && m_irradianceCubeSampler && m_irradianceCubeUploaded)
        return;

    // +X, -X, +Y, -Y, +Z, -Z
    QImage faces[6] = {
        loadFace(":/gx3d/assets/envcube/irr_posx.png"),
        loadFace(":/gx3d/assets/envcube/irr_negx.png"),
        loadFace(":/gx3d/assets/envcube/irr_posy.png"),
        loadFace(":/gx3d/assets/envcube/irr_negy.png"),
        loadFace(":/gx3d/assets/envcube/irr_posz.png"),
        loadFace(":/gx3d/assets/envcube/irr_negz.png"),
    };

    for (int i = 0; i < 6; ++i) {
        if (faces[i].isNull()) return;
        faces[i] = faces[i].convertToFormat(QImage::Format_RGBA8888);
    }

    const QSize sz = faces[0].size();
    for (int i = 1; i < 6; ++i) {
        if (faces[i].size() != sz) {
            qWarning() << "IrradianceCube face size mismatch" << i << faces[i].size() << "expected" << sz;
            return;
        }
    }

    if (!m_irradianceCubeTex) {
        QRhiTexture::Flags tf = QRhiTexture::CubeMap;

        m_irradianceCubeTex = rhi->newTexture(QRhiTexture::RGBA8, sz, 1, tf);
        if (!m_irradianceCubeTex->create()) {
            qWarning() << "Failed to create irradiance cubemap texture";
            m_irradianceCubeTex = nullptr;
            return;
        }
    }

    if (!m_irradianceCubeSampler) {
        m_irradianceCubeSampler = rhi->newSampler(
            QRhiSampler::Linear,   // mag
            QRhiSampler::Linear,   // min
            QRhiSampler::None,   // mip  (important)
            QRhiSampler::ClampToEdge,
            QRhiSampler::ClampToEdge,
            QRhiSampler::ClampToEdge
            );
        if (!m_irradianceCubeSampler->create()) {
            qWarning() << "Failed to create irradiance cubemap sampler";
            m_irradianceCubeSampler = nullptr;
            return;
        }
    }

    // Upload all 6 faces as layers, mip level 0
    QRhiTextureUploadDescription uploadDesc;
    QVector<QRhiTextureUploadEntry> entries;
    entries.reserve(6);

    for (int face = 0; face < 6; ++face) {
        QRhiTextureSubresourceUploadDescription sub(faces[face]);
        entries.push_back(QRhiTextureUploadEntry(face /*layer*/, 0 /*level*/, sub));
    }
    uploadDesc.setEntries(entries.cbegin(), entries.cend());

    QRhiResourceUpdateBatch *u = rhi->nextResourceUpdateBatch();
    u->uploadTexture(m_irradianceCubeTex, uploadDesc);

    cb->resourceUpdate(u);
    m_irradianceCubeUploaded = true;
}

void GXSceneRenderNode::destroyFrameLightUbo()
{
    if (m_frameLightUbo) {
        m_frameLightUbo->destroy();
        delete m_frameLightUbo;
        m_frameLightUbo = nullptr;
        m_frameLightDirty = true;
    }
}

void GXSceneRenderNode::destroyEnvironmentUbo()
{
    if (m_environmentUbo) {
        m_environmentUbo->destroy();
        delete m_environmentUbo;
        m_environmentUbo = nullptr;
        m_environmentDirty = true;
    }
}

void GXSceneRenderNode::syncRenderables()
{
    m_renderables.clear();
    if (!m_scene) return;

    m_scene->traverse([&](scene::GXNode* n) {
        if (auto* r = qobject_cast<GXRenderableNode*>(n)) m_renderables.push_back(QPointer<GXRenderableNode>(r));
    });
}

// void GXSceneRenderNode::ensurePickTarget(QRhi *rhi, QRhiRenderTarget *rt)
// {
//     if (!rhi || !rt) return;

//     const QSize sz = surfacePixelSize(rt);
//     const int sampleCount = rt->sampleCount();

//     if (sz.isEmpty()) return;

//     const bool sizeChanged = (m_lastPickSize != sz);
//     const bool sampleChanged = (m_lastPickSampleCount != sampleCount);

//     const int pickSamples = 1;

//     const bool needRebuild = (!m_pickTex) || (!m_pickRt) || sizeChanged || (m_lastPickSampleCount != pickSamples);

//     if (!needRebuild) return;

//     destroyPickTarget();

//     m_lastPickSize = sz;
//     m_lastPickSampleCount = pickSamples;

//     m_pickTex = rhi->newTexture(QRhiTexture::RGBA8, sz, 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource);
//     if (!m_pickTex->create()) {
//         qWarning() << "[GXSceneRenderNode] failed to create pick texture";
//         destroyPickTarget();
//         return;
//     }

//     QRhiTextureRenderTargetDescription rtDesc((QRhiColorAttachment(m_pickTex)));
//     m_pickRt = rhi->newTextureRenderTarget(rtDesc);

//     m_pickRp = m_pickRt->newCompatibleRenderPassDescriptor();
//     m_pickRt->setRenderPassDescriptor(m_pickRp);

//     if (!m_pickRt->create()) {
//         qWarning() << "[GXSceneRenderNode] failed to create pick RT";
//         destroyPickTarget();
//         return;
//     }
// }

// void GXSceneRenderNode::destroyPickTarget()
// {
//     destroyPickPassResources();

//     if (m_pickRt) { m_pickRt->destroy(); delete m_pickRt; m_pickRt = nullptr; }
//     if (m_pickRp) { m_pickRp->destroy(); delete m_pickRp; m_pickRp = nullptr; }
//     if (m_pickTex) { m_pickTex->destroy(); delete m_pickTex; m_pickTex = nullptr; }

//     m_lastPickSize = {};
//     m_lastPickSampleCount = 1;
// }

// void GXSceneRenderNode::ensurePickPassResources(QRhi *rhi)
// {
//     if (!rhi || !m_pickRt || !m_pickRp)
//         return;

//     if (m_pickPs && m_pickRhi == rhi)
//         return;

//     destroyPickPassResources();
//     m_pickRhi = rhi;

//     // --- UBO: mat4 + uvec4 (std140)
//     const int ubufSize = 64 + 16; // mat4 = 64 bytes, uvec4 = 16 bytes
//     m_pickUbuf = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, ubufSize);
//     if (!m_pickUbuf->create()) {
//         qWarning() << "PickPass: failed to create uniform buffer";
//         destroyPickPassResources();
//         return;
//     }

//     // --- SRB: binding=0 uniform buffer
//     m_pickSrb = rhi->newShaderResourceBindings();
//     m_pickSrb->setBindings({
//         QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage, m_pickUbuf)
//     });
//     if (!m_pickSrb->create()) {
//         qWarning() << "PickPass: failed to create SRB";
//         destroyPickPassResources();
//         return;
//     }

//     // --- Shaders
//     const QRhiShaderStage vs(QRhiShaderStage::Vertex, renderUtils.gxLoadShader(":/gx3d/shaders/pick.vert.qsb")); // adapt your loader
//     const QRhiShaderStage fs(QRhiShaderStage::Fragment, renderUtils.gxLoadShader(":/gx3d/shaders/pick.frag.qsb"));

//     // --- Pipeline
//     m_pickPs = rhi->newGraphicsPipeline();
//     m_pickPs->setShaderStages({ vs, fs });

//     // Vertex layout: MUST match your mesh position attribute location=0.
//     // If your meshes use a different layout, we adjust.
//     QRhiVertexInputLayout inputLayout;
//     inputLayout.setBindings({
//         QRhiVertexInputBinding(sizeof(float) * 8) // Example stride (pos+normal+uv). Adjust to your actual mesh stride.
//     });
//     inputLayout.setAttributes({
//         // location=0: position
//         QRhiVertexInputAttribute(0, 0, QRhiVertexInputAttribute::Float3, 0)
//     });

//     m_pickPs->setVertexInputLayout(inputLayout);
//     m_pickPs->setShaderResourceBindings(m_pickSrb);
//     m_pickPs->setRenderPassDescriptor(m_pickRp);

//     // No blending, depth test optional:
//     // For picking you usually WANT depth test so the front-most object wins.
//     QRhiGraphicsPipeline::TargetBlend tb;
//     tb.enable = false;
//     m_pickPs->setTargetBlends({ tb });

//     m_pickPs->setDepthTest(false);
//     m_pickPs->setDepthWrite(false);
//     m_pickPs->setCullMode(QRhiGraphicsPipeline::Back);

//     if (!m_pickPs->create()) {
//         qWarning() << "PickPass: failed to create pipeline";
//         destroyPickPassResources();
//         return;
//     }
// }

// void GXSceneRenderNode::destroyPickPassResources()
// {
//     delete m_pickPs; m_pickPs = nullptr;
//     delete m_pickSrb; m_pickSrb = nullptr;
//     delete m_pickUbuf; m_pickUbuf = nullptr;
//     m_pickRhi = nullptr;
// }

void GXSceneRenderNode::ensureDepthTarget(QRhi *rhi, QRhiRenderTarget *windowRt)
{
    if (!rhi || !windowRt) return;

    if (auto *swrt = dynamic_cast<QRhiSwapChainRenderTarget *>(windowRt)) {
        // Swapchain target: let QQuickWindow / QRhiSwapChain manage depth+rpDesc.
        // We must not override sc->setDepthStencil() or sc->setRenderPassDescriptor().
        m_rtWithDepth = nullptr;      // ensure we render to swapchain RT
        m_rpDesc = nullptr;
        m_depthBuffer = nullptr;      // we no longer own one for swapchain
        m_lastSwapChain = swrt->swapChain();
        m_lastSize = surfacePixelSize(windowRt);
        m_lastSampleCount = m_lastSwapChain ? m_lastSwapChain->sampleCount() : 1;
        return;
    }


    // if (auto *swrt = dynamic_cast<QRhiSwapChainRenderTarget *>(windowRt)) {
    //     QRhiSwapChain *sc = swrt->swapChain();
    //     if (!sc) return;

    //     // const QSize sz = sc->currentPixelSize();
    //     const QSize sz = surfacePixelSize(windowRt);
    //     const int sampleCount = sc->sampleCount();
    //     if (sz.isEmpty()) return;

    //     const bool swapChanged   = (m_lastSwapChain != sc);
    //     const bool sizeChanged   = (m_lastSize != sz);
    //     const bool samplesChanged = (m_lastSampleCount != sampleCount);
    //     const bool needRebuild = !m_depthBuffer || !m_rpDesc || swapChanged || sizeChanged || samplesChanged;

    //     if (!needRebuild)
    //         return;

    //     destroyDepthTarget();

    //     m_lastSwapChain = sc;
    //     m_lastSize = sz;
    //     m_lastSampleCount = sampleCount;

    //     m_depthBuffer = rhi->newRenderBuffer(QRhiRenderBuffer::DepthStencil, sz, sampleCount);
    //     if (!m_depthBuffer->create()) {
    //         qWarning() << "GXSceneRenderNode: swapchain depth buffer create failed";
    //         destroyDepthTarget();
    //         return;
    //     }

    //     sc->setDepthStencil(m_depthBuffer);

    //     m_rpDesc = sc->newCompatibleRenderPassDescriptor();
    //     sc->setRenderPassDescriptor(m_rpDesc);

    //     // Important: renderables need to rebuild pipelines once
    //     forEachRenderable([](GXRenderableNode* r) { r->markForRelease(); });
    //     return;
    // }

    if (auto *wtrt = dynamic_cast<QRhiTextureRenderTarget *>(windowRt)) {
        const QSize sz = windowRt->pixelSize();
        const int sampleCount = windowRt->sampleCount();

        if (sz.isEmpty())
            return;

        const bool sizeChanged    = (m_lastSize != sz);
        const bool samplesChanged = (m_lastSampleCount != sampleCount);
        const bool needRebuild    = (!m_rtWithDepth) || (!m_depthBuffer) || sizeChanged || samplesChanged;

        if (!needRebuild)
            return;

        destroyDepthTarget();

        m_lastSize = sz;
        m_lastSampleCount = sampleCount;

        // m_depthBuffer = rhi->newRenderBuffer(QRhiRenderBuffer::DepthStencil, sz, sampleCount, QRhiRenderBuffer::UsedWithSwapChainOnly, QRhiTexture::D32F);
        m_depthBuffer = rhi->newRenderBuffer(QRhiRenderBuffer::DepthStencil, sz, sampleCount);

        if (!m_depthBuffer->create()) {
            qWarning() << "GXSceneRenderNode: depth buffer create failed";
            destroyDepthTarget();
            return;
        }

        const QRhiTextureRenderTargetDescription winDesc = wtrt->description();
        QVector<QRhiColorAttachment> colors;
        colors.reserve(winDesc.colorAttachmentCount());
        for (int i = 0; i < winDesc.colorAttachmentCount(); ++i)
            colors.append(*winDesc.colorAttachmentAt(i));

        if (colors.isEmpty()) {
            qWarning() << "GXSceneRenderNode: window texture RT has no color attachments";
            destroyDepthTarget();
            return;
        }

        QRhiTextureRenderTargetDescription desc;
        desc.setColorAttachments(colors.cbegin(), colors.cend());
        desc.setDepthStencilBuffer(m_depthBuffer);

        m_rtWithDepth = rhi->newTextureRenderTarget(desc);
        m_rpDesc = m_rtWithDepth->newCompatibleRenderPassDescriptor();
        m_rtWithDepth->setRenderPassDescriptor(m_rpDesc);

        if (!m_rtWithDepth->create()) {
            qWarning() << "GXSceneRenderNode: RT-with-depth create failed";
            destroyDepthTarget();
            return;
        }

        forEachRenderable([](GXRenderableNode* r) { r->markForRelease(); });
        return;
    }

    destroyDepthTarget();
}
