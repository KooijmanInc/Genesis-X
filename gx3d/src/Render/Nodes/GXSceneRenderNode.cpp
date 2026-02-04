// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Render/Nodes/GXSceneRenderNode.h>
#include <GenesisX/GX3D/Render/Nodes/GXRenderableNode.h>
#include <GenesisX/GX3D/Render/Nodes/GXModelNode.h>
#include <GenesisX/GX3D/Render/Materials/GXPrincipledMaterial.h>
#include <GenesisX/GX3D/Scene/GXScene.h>

#include <GenesisX/GX3D/Query/GXWorldQuery.h>
#include <GenesisX/GX3D/Query/GXSceneQueryBackend.h>

#include <QObject>
#include <QSGRendererInterface>

#include <rhi/qrhi_platform.h>
#include <rhi/qrhi.h>

using namespace gx::gx3d::render;

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

// struct alignas(16) FrameLightingUBO {
//     float lightPos[4];
//     float lightDir[4];
//     float lightColor[4];
//     float lightParams[4];

//     float cosInner;
//     float cosOuter;
//     float _pad0;
//     float _pad1;
// };
// static_assert(sizeof(FrameLightingUBO) % 16 == 0);

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
    GXLightGPU lights[GX_MAX_LIGHTS];
};
static_assert(sizeof(GXLightGPU) == 64);
static_assert(sizeof(FrameLightingUBO) == 16 + 64 * GX_MAX_LIGHTS);
static_assert(sizeof(FrameLightingUBO) % 16 == 0);

static QSize surfacePixelSize(QRhiRenderTarget* rt)
{
    if (auto *swrt = dynamic_cast<QRhiSwapChainRenderTarget *>(rt)) {
        if (auto* sc = swrt->swapChain()) return sc->currentPixelSize();
    }
    return rt? rt->pixelSize() : QSize();
}

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
    // if (m_beforeRenderingConn) QObject::disconnect(m_beforeRenderingConn);

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

    // if (m_beforeRenderingConn) {
    //     QObject::disconnect(m_beforeRenderingConn);
    //     m_beforeRenderingConn = {};
    // }

    m_window = w;

    if (!m_window) return;

    auto markDirty = [this]() {
        m_depthDirty.store(true, std::memory_order_relaxed);
    };

    // QObject::connect(m_window, &QQuickWindow::widthChanged, m_window, markDirty, Qt::DirectConnection);
    // QObject::connect(m_window, &QQuickWindow::heightChanged, m_window, markDirty, Qt::DirectConnection);

    QObject::connect(m_window, &QQuickWindow::sceneGraphInvalidated, m_window, markDirty, Qt::DirectConnection);
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
}

void GXSceneRenderNode::render(const RenderState */*state*/)
{
    if (!m_window || !m_scene) return;

    QRhi* rhi = m_window->rhi();
    if (!rhi) return;

    if (m_lastRhi != rhi) {
        destroyFrameLightUbo();
        m_lastRhi = rhi;
        m_frameLightDirty = true;
    }

    if (!m_frameLightUbo) {
        m_frameLightUbo = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(FrameLightingUBO));
        if (!m_frameLightUbo->create()) qWarning() << "GXSceneRenderNode: frame light UBO create failed";
        m_frameLightDirty = true;
    }

    // For QRhiRenderTarget + command buffer:
    // QSGRenderNode exposes these through internal render state; the easiest practical approach
    // is to follow the same pattern you used in GXTestTriangleNode (since it already works).
    //
    // So: copy the exact "get cb + rt" approach from GXTestTriangleNode here.

    QRhiCommandBuffer *cb = commandBuffer();
    QRhiRenderTarget *rt  = renderTarget();

    if (!cb || !rt)
        return;

    if (m_depthDirty.exchange(false, std::memory_order_relaxed)) {
        destroyDepthTarget();

        forEachRenderable([&](GXRenderableNode *r) {
            r->markForRelease();
        });
        if (m_window) m_window->update();
        return;
    }
    // if (m_depthDirty.load(std::memory_order_relaxed))
    //     qWarning() << "depthDirty is true";

    // if (auto *swrt = dynamic_cast<QRhiSwapChainRenderTarget *>(rt)) {
    const bool isSwapchain = dynamic_cast<QRhiSwapChainRenderTarget *>(rt) != nullptr;
    const bool ownsDepth = !isSwapchain; // we only own depth for texture RT path

    if (ownsDepth) {
        const QSize surfaceSz = surfacePixelSize(rt);
        if (m_depthBuffer && m_depthBuffer->pixelSize() != surfaceSz) {
            destroyDepthTarget();
            forEachRenderable([](GXRenderableNode* r) { r->markForRelease(); });
            if (m_window) m_window->update();
            return;
        }
    }

        // auto *sc = swrt->swapChain();
        // if (sc && m_depthBuffer && m_depthBuffer->pixelSize() != sc->currentPixelSize()) {
        //     // qWarning() << "Depth mismatch detected (forcing rebuild):"
        //     //            << "depth" << m_depthBuffer->pixelSize()
        //     //            << "swap" << sc->currentPixelSize();
        //     destroyDepthTarget();
        //     forEachRenderable([](GXRenderableNode* r) { r->markForRelease(); });
        //     m_window->update();
        //     return;
        // }
    // }

    ensureDepthTarget(rhi, rt);
    // ensurePickTarget(rhi, rt);
    // ensurePickPassResources(rhi);

    QRhiRenderTarget *useRt = m_rtWithDepth ? static_cast<QRhiRenderTarget*>(m_rtWithDepth) : rt;

    if (ownsDepth) {
        const QSize surfaceSz2 = surfacePixelSize(useRt); // <-- validate against the RT we actually render to
        if (m_depthBuffer && m_depthBuffer->pixelSize() != surfaceSz2) {
            qWarning() << "Depth still mismatched after rebuild:"
                       << "depth=" << m_depthBuffer->pixelSize()
                       << "surface=" << surfaceSz2
                       << "-> skipping frame";
            destroyDepthTarget();
            forEachRenderable([](GXRenderableNode* r) { r->markForRelease(); });
            if (m_window) m_window->update();
            return;
        }
    }

    FrameLightingUBO fl{};
    const int requested = m_scene ? m_scene->maxLights() : 8;
    const int used = qMin(requested, GX_MAX_LIGHTS);

    if (requested > GX_MAX_LIGHTS) {
        qWarning() << "GXScene maxLights=" << requested
                   << "exceeds shader cap" << GX_MAX_LIGHTS
                   << "clamping.";
    }

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

        // qDebug() << n->objectName();
        // check emission materials
        // if (auto* m = qobject_cast<GXModel*>(n)) {
        //     auto mats = m->materials();

        // }
    });

    fl.frameParams = QVector4D(float(count), (m_scene && m_scene->debugLighting()) ? 0.15f : 0.0f, 0.0f, 0.0f);

    QRhiResourceUpdateBatch* u = rhi->nextResourceUpdateBatch();
    const quint32 bytes = sizeof(FrameLightingUBO);
    u->updateDynamicBuffer(m_frameLightUbo, 0, bytes, &fl);
    cb->resourceUpdate(u);

    forEachRenderable([&](GXRenderableNode *r) {
        r->setViewProj(m_viewProj);
        r->setFrameLightingUbo(m_frameLightUbo);
        // r->syncFromScene();
        r->ensureResources(rhi, useRt, cb);
        r->recordRender(cb, useRt);
    });

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
    if (!m_scene) return;

    m_scene->traverse([&](scene::GXNode* n){
        if (auto* r = qobject_cast<GXRenderableNode*>(n)) fn(r);
    });
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

void GXSceneRenderNode::destroyFrameLightUbo()
{
    if (m_frameLightUbo) {
        m_frameLightUbo->destroy();
        delete m_frameLightUbo;
        m_frameLightUbo = nullptr;
        m_frameLightDirty = true;
    }
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

        m_depthBuffer = rhi->newRenderBuffer(QRhiRenderBuffer::DepthStencil, sz, sampleCount, QRhiRenderBuffer::UsedWithSwapChainOnly, QRhiTexture::D32F);

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
