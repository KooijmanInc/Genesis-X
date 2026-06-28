// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Render/Nodes/GXModelNode.h>
#include <GenesisX/GX3D/Render/Materials/GXDefaultLitMaterial.h>
#include <GenesisX/GX3D/Render/Materials/GXPrincipledMaterial.h>
#include <GenesisX/GX3D/Render/Materials/GXMaterial.h>
#include <GenesisX/GX3D/Render/Meshes/GXConeMesh.h>
#include <GenesisX/GX3D/Render/Meshes/GXCylinderMesh.h>
#include <GenesisX/GX3D/Render/Meshes/GXSphereMesh.h>
#include <GenesisX/GX3D/Render/Meshes/GXTorusMesh.h>
#include <GenesisX/GX3D/Render/Utils/GXGltfLoader.h>
#include <GenesisX/GX3D/Render/Utils/GXMeshReader.h>

#include <QFileInfo>
#include <QVector4D>

#include <QElapsedTimer>

#include <limits>
#include <algorithm>
// #include <atomic>


using namespace gx::gx3d::render;

// static std::atomic<int> g_pipelinesCreatedThisSecond{0};
// static std::atomic<int> g_srbsCreatedThisSecond{0};
// static std::atomic<int>   g_ruBatchesThisSecond{0};
// static std::atomic<int>   g_dynUpdatesThisSecond{0};
// static std::atomic<qint64> g_dynBytesThisSecond{0};

static inline bool boundsValid(const QVector3D& bmin, const QVector3D& bmax)
{
    return !qIsNaN(bmin.x()) && !qIsNaN(bmin.y()) && !qIsNaN(bmin.z()) &&
           !qIsNaN(bmax.x()) && !qIsNaN(bmax.y()) && !qIsNaN(bmax.z());
}

static inline void computeBoundsIfMissing(GXMeshData& cpu)
{
    if (boundsValid(cpu.boundsMin, cpu.boundsMax))
        return;

    if (cpu.vertices.isEmpty()) {
        cpu.boundsMin = QVector3D(-0.5f, -0.5f, -0.5f);
        cpu.boundsMax = QVector3D( 0.5f,  0.5f,  0.5f);
        return;
    }

    QVector3D minV(+std::numeric_limits<float>::infinity(),
                   +std::numeric_limits<float>::infinity(),
                   +std::numeric_limits<float>::infinity());
    QVector3D maxV(-std::numeric_limits<float>::infinity(),
                   -std::numeric_limits<float>::infinity(),
                   -std::numeric_limits<float>::infinity());

    auto& ve = cpu.vertices;
    for (const auto& v : ve) {
        const QVector3D p = v.position;
        minV.setX(std::min(minV.x(), p.x()));
        minV.setY(std::min(minV.y(), p.y()));
        minV.setZ(std::min(minV.z(), p.z()));
        maxV.setX(std::max(maxV.x(), p.x()));
        maxV.setY(std::max(maxV.y(), p.y()));
        maxV.setZ(std::max(maxV.z(), p.z()));
    }

    cpu.boundsMin = minV;
    cpu.boundsMax = maxV;
}

static void ensureUboForMat(QRhi* rhi, QHash<GXMaterial*, QRhiBuffer*>& map, GXMaterial* mat, int size)
{
    if (!rhi || !mat || size <= 0) return;

    QRhiBuffer*& buf = map[mat];
    const quint32 usize = quint32(size);

    if (buf && buf->size() == usize) return;

    if (buf) { buf->destroy(); delete buf; buf = nullptr; }

    buf = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, usize);
    if (!buf || !buf->create()) {
        qWarning() << "GXModel: per-mat ubuf create failed";
        delete buf;
        buf = nullptr;
    }
}

static void ensureUbo(QRhi* rhi, QRhiBuffer*& buf, int size)
{
    if (!rhi || size <= 0) {
        qWarning() << "GXModel: ensureUbo invalid args" << "rhi=" << (void*)rhi << "size=" << size;
        return;
    }

    const quint32 usize = quint32(size);

    if (buf && buf->size() == usize) return;

    if (buf) {
        buf->destroy();
        delete buf;
        buf = nullptr;
    }

    buf = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, usize);
    if (!buf || !buf->create()) {
        qWarning() << "GXModel: ubuf Uniform buffer failed";
        delete buf;
        buf = nullptr;
    }
}

GXModel::GXModel(QObject *parent)
    : GXRenderableNode{parent}
{
    if (!m_material) setMaterial(new GXDefaultLitMaterial(this));
}

GXModel::~GXModel()
{
    destroyRhiResources();
    delete m_mesh;
    m_mesh = nullptr;
}

void GXModel::setMesh(GXMesh *mesh)
{
    if (m_mesh == mesh) return;
    delete m_mesh;
    m_mesh = mesh;

    emit meshChanged();
}

void GXModel::setSource(QString src)
{
    if (m_source == src) return;
    m_source = src;
    delete m_mesh;
    m_mesh = nullptr;

    m_meshLoaded = false;
    m_loadedMeshSource.clear();
    m_meshCpu = GXMeshData{};

    if (src == "#Cone") {
        m_mesh = GXConeMesh::create();
    } else if (src == "#Cube") {
        m_mesh = GXCubeMesh::create();
    } else if (src == "#Cylinder") {
        m_mesh = GXCylinderMesh::create();
    } else if (src == "#Plane") {
        m_mesh = GXPlaneMesh::create();
    } else if (src == "#Sphere") {
        m_mesh = GXSphereMesh::create();
    } else if (src == "#Torus") {
        m_mesh = GXTorusMesh::create();
    } else if (src.endsWith(".glb")) {
        const auto meshes = GXGltfLoader::loadMesh(QUrl(src));
        m_mesh = meshes.isEmpty() ? nullptr : meshes.first();
    } else if (src.endsWith(".mesh")) {
        // m_source = src;
    }

    emit sourceChanged();
}

QQmlListProperty<GXMaterial> GXModel::materials()
{
    return QQmlListProperty<GXMaterial>(
        this,
        this,
        [](QQmlListProperty<GXMaterial>* p, GXMaterial* v) {
            auto* self = static_cast<GXModel*>(p->data);
            if (!v) return;
            self->m_materials.append(v);
            emit self->materialsChanged();
        },
        [](QQmlListProperty<GXMaterial>* p) -> qsizetype {
            auto* self = static_cast<GXModel*>(p->data);
            return self->m_materials.size();
        },
        [](QQmlListProperty<GXMaterial>* p, qsizetype i) -> GXMaterial* {
            auto* self = static_cast<GXModel*>(p->data);
            return (i >= 0 && i < self->m_materials.size()) ? self->m_materials[int(i)] : nullptr;
        },
        [](QQmlListProperty<GXMaterial>* p) {
            auto* self = static_cast<GXModel*>(p->data);
            self->m_materials.clear();
            emit self->materialsChanged();
        }
    );
}

void GXModel::setPickable(bool on)
{
    if (m_pickable == on) return;
    m_pickable = on;

    emit pickableChanged();
}

void GXModel::setPickPriority(int v)
{
    if (m_pickPriority == v) return;
    m_pickPriority = v;

    emit pickPriorityChanged();
}

bool GXModel::localBounds(QVector3D &outMinLS, QVector3D &outMaxLS) const
{
    // If mesh CPU bounds are valid, use them.
    // Your GXMeshData uses qQNaN() as "unset".
    const auto& bmin = m_meshCpu.boundsMin;
    const auto& bmax = m_meshCpu.boundsMax;

    const bool valid =
        !qIsNaN(bmin.x()) && !qIsNaN(bmin.y()) && !qIsNaN(bmin.z()) &&
        !qIsNaN(bmax.x()) && !qIsNaN(bmax.y()) && !qIsNaN(bmax.z());

    if (valid) {
        outMinLS = bmin;
        outMaxLS = bmax;
        return true;
    }

    // Fallback: unit cube centered at origin (safe for “mesh not loaded yet”)
    outMinLS = QVector3D(-0.5f, -0.5f, -0.5f);
    outMaxLS = QVector3D( 0.5f,  0.5f,  0.5f);
    return false;
}

void GXModel::addMaterial(GXMaterial *m)
{
    if (!m) return;
    m_materials.append(m);

    emit materialsChanged();
    invalidPipeline();
}

void GXModel::clearMaterials()
{
    if (m_materials.isEmpty()) return;
    m_materials.clear();

    emit materialsChanged();
    invalidPipeline();
}

void GXModel::ensureResources(QRhi *rhi, QRhiRenderTarget *rt, QRhiCommandBuffer* cb)
{
    QString err;
    QRhiRenderPassDescriptor* rp = rt->renderPassDescriptor();
    const int sc = rt->sampleCount();

    if (!ensureMeshLoaded(&err)) {
        qWarning() << "[GXModel] failed to load mesh:" << err;
        return;
    }

    if (m_pendingRelease) {
        releaseResources();
        m_pendingRelease = false;
    }
    const bool haveLightingNow = (m_frameLightingUbo != nullptr);
    if (m_boundHadLighting != haveLightingNow || m_boundLightingUbo != m_frameLightingUbo) {
        m_pipelineDirty = true;

        qDeleteAll(m_materialSrbs);
        m_materialSrbs.clear();
    }

    GXRenderableNode::ensureResources(rhi, rt, cb);

    if (m_rhi == rhi && m_ps && m_vsUbuf && m_fsUbuf && !m_pipelineDirty && m_boundLightingUbo == m_frameLightingUbo && m_lastRp == rp && m_lastSampleCount == sc) return;

    destroyPipelineResources();
    m_rhi = rhi;

    if (m_mesh) m_mesh->ensureResources(rhi);

    GXMaterial* defaultMat = nullptr;
    if (!m_materials.isEmpty()) defaultMat = m_materials.first();
    else defaultMat = material();

    if (!defaultMat) {
        qWarning() << "GXModel: no default material";
        return;
    }

    int maxVsSize = 0;
    int maxFsSize = 0;

    auto considerMat = [&](GXMaterial* m) {
        if (!m) return;
        maxVsSize = qMax(maxVsSize, m->vsUboSize());
        maxFsSize = qMax(maxFsSize, m->fsUboSize());
    };

    for (GXMaterial* m : std::as_const(m_materials)) considerMat(m);

    considerMat(material());

    ensureUbo(rhi, m_vsUbuf, maxVsSize);
    ensureUbo(rhi, m_fsUbuf, maxFsSize);

    defaultMat->ensureRhi(rhi, cb);

    m_ps = m_rhi->newGraphicsPipeline();
    // g_pipelinesCreatedThisSecond.fetch_add(1, std::memory_order_relaxed);

    GXMaterial* matPtr = defaultMat;
    m_ps->setShaderStages({
        { QRhiShaderStage::Vertex, matPtr->vertexShader() },
        { QRhiShaderStage::Fragment, matPtr->fragmentShader() }
    });

    QRhiVertexInputLayout inputLayout;
    inputLayout.setBindings({ QRhiVertexInputBinding(sizeof(GXMesh::Vertex)) });
    inputLayout.setAttributes({
                               QRhiVertexInputAttribute(0, 0, QRhiVertexInputAttribute::Float3, offsetof(GXMesh::Vertex, px)),
        QRhiVertexInputAttribute(0, 1, QRhiVertexInputAttribute::Float3, offsetof(GXMesh::Vertex, nx)),
        QRhiVertexInputAttribute(0, 2, QRhiVertexInputAttribute::Float2,  offsetof(GXMesh::Vertex, u)),
        QRhiVertexInputAttribute(0, 3, QRhiVertexInputAttribute::Float4,  offsetof(GXMesh::Vertex, tx))
    });

    // qDebug() << "Vertex sizeof =" << sizeof(GXMesh::Vertex)
    //          << "px" << offsetof(GXMesh::Vertex, px)
    //          << "nx" << offsetof(GXMesh::Vertex, nx)
    //          << "u"  << offsetof(GXMesh::Vertex, u)
    //          << "tx" << offsetof(GXMesh::Vertex, tx);


    QRhiShaderResourceBindings* layoutSrb = srbForMaterial(defaultMat, cb);
    if (!layoutSrb) {
        qWarning() << "GXModel: failed to build SRB for default material";
        return;
    }

    m_ps->setVertexInputLayout(inputLayout);
    m_ps->setShaderResourceBindings(layoutSrb);
    m_ps->setTopology(QRhiGraphicsPipeline::Triangles);
    m_ps->setRenderPassDescriptor(rt->renderPassDescriptor());
    m_ps->setSampleCount(rt->sampleCount());

    if (defaultMat) {
        defaultMat->applyTo(m_ps);
    } else {
        m_ps->setCullMode(QRhiGraphicsPipeline::Back);
        m_ps->setFrontFace(QRhiGraphicsPipeline::CCW);
        m_ps->setDepthOp(QRhiGraphicsPipeline::LessOrEqual);
        m_ps->setDepthTest(true);
        m_ps->setDepthWrite(true);
    }

    if (!m_ps->create()) {
        qWarning() << "GXModel: pipeline create failed";
        return;
    }

    m_pipelineDirty = false;
    m_boundHadLighting = (m_frameLightingUbo != nullptr);
    m_boundLightingUbo = m_frameLightingUbo;
    m_lastRp = rp;
    m_lastSampleCount = sc;
}

void GXModel::recordRender(QRhiCommandBuffer *cb, QRhiRenderTarget *rt, const QRect &scissor)
{
    if (!cb || !rt) return;

    QRhi* rhi = cb->rhi();
    QRhiResourceUpdateBatch* u = rhi->nextResourceUpdateBatch();
    // bool anyUboUpdates = false;
    // auto t0 = GXClock::now();
    ensureResources(rhi, rt, cb);
    // auto t1 = GXClock::now();
    if (m_mesh) m_mesh->uploadIfNeeded(rhi, cb);

    if (!m_ps) return;

    if (!m_vsUbuf || !m_fsUbuf) {
        qWarning() << "GXModel::recordRender: missing UBO buffers"
                   << "vsUbuf=" << (void*)m_vsUbuf
                   << "fsUbuf=" << (void*)m_fsUbuf;
        return;
    }

    QMatrix4x4 model = worldMatrix();

    // QMatrix4x4 mvp = rhi->clipSpaceCorrMatrix() * (m_viewProj * model);

    auto updateUbosFor = [&](GXMaterial* m, const QMatrix4x4& mvpIn, const QMatrix4x4& modelIn, QRhiResourceUpdateBatch* u) -> bool {

        bool wrote = false;

        m_vsScratch.resize(m->vsUboSize());
        if (!m_vsScratch.isEmpty()) {
            m->fillVS(m_vsScratch.data(), mvpIn, modelIn);
            if (auto* vsBuf = m_vsUbufPerMat.value(m, nullptr))
                u->updateDynamicBuffer(vsBuf, 0, m_vsScratch.size(), m_vsScratch.constData()),
                    wrote = true;
        }

        m_fsScratch.resize(m->fsUboSize());
        if (!m_fsScratch.isEmpty()) {
            m->fillFS(m_fsScratch.data());
            if (auto* fsBuf = m_fsUbufPerMat.value(m, nullptr))
                u->updateDynamicBuffer(fsBuf, 0, m_fsScratch.size(), m_fsScratch.constData()),
                    wrote = true;
        }

        // QByteArray vsData;
        // m_vsScratch.resize(m->vsUboSize());
        // if (!m_vsScratch.isEmpty())
        //     m->fillVS(m_vsScratch.data(), mvpIn, modelIn);

        // // QByteArray fsData;
        // m_fsScratch.resize(m->fsUboSize());
        // if (!m_fsScratch.isEmpty())
        //     m->fillFS(m_fsScratch.data());

        // QVector4D preview(0, 0, 0, 0);
        // if (m_fsScratch.size() >= 16) {
        //     const float *f = reinterpret_cast<const float*>(m_fsScratch.constData());
        //     preview = QVector4D(f[0], f[1], f[2], f[3]);
        // }

        // QRhiBuffer* vsBuf = m_vsUbufPerMat.value(m, nullptr);
        // QRhiBuffer* fsBuf = m_fsUbufPerMat.value(m, nullptr);

        // if (!vsBuf || !fsBuf) {
        //     qWarning() << "GXModel: missing per-mat UBO for" << m;
        //     return false;
        // }

        // {
        //     // QRhiResourceUpdateBatch* u = rhi->nextResourceUpdateBatch();
        //     // g_ruBatchesThisSecond.fetch_add(1, std::memory_order_relaxed);
        //     if (!m_vsScratch.isEmpty()) {
        //         u->updateDynamicBuffer(vsBuf, 0, m_vsScratch.size(), m_vsScratch.constData());
        //         // anyUboUpdates = true;
        //         // g_dynUpdatesThisSecond.fetch_add(1, std::memory_order_relaxed);
        //         // g_dynBytesThisSecond.fetch_add(vsData.size(), std::memory_order_relaxed);
        //     }
        //     if (!m_fsScratch.isEmpty()) {
        //         u->updateDynamicBuffer(fsBuf, 0, m_fsScratch.size(), m_fsScratch.constData());
        //         // anyUboUpdates = true;
        //         // g_dynUpdatesThisSecond.fetch_add(1, std::memory_order_relaxed);
        //         // g_dynBytesThisSecond.fetch_add(fsData.size(), std::memory_order_relaxed);
        //     }
        //     // cb->resourceUpdate(u);
        // }

        return wrote;
    };

    const QSize ps = rt->pixelSize();
    if (ps.isEmpty()) return;
    // cb->setViewport(QRhiViewport(0, 0, float(ps.width()), float(ps.height())));
    // cb->setScissor(QRhiScissor(0, 0, ps.width(), ps.height()));

    // QRect r = scissor;
    // if (!r.isValid() || r.isEmpty()) {
    //     r = QRect(0, 0, ps.width(), ps.height());
    // }
    // r = r.intersected(QRect(0, 0, ps.width(), ps.height()));
    // const bool yUp = rhi->isYUpInFramebuffer();
    // if (yUp) {
    //     r.setY(ps.height() - (r.y() + r.height()));
    // }

    QRect r = scissor.intersected(QRect(0, 0, ps.width(), ps.height()));

    const int yFlipped = ps.height() - (r.y() + r.height());

    cb->setViewport(QRhiViewport(float(r.x()), float(yFlipped),
                                 float(r.width()), float(r.height())));
    cb->setScissor(QRhiScissor(r.x(), yFlipped, r.width(), r.height()));


    // r.x() = r.width();
    // cb->setViewport(QRhiViewport(float(r.x()), float(r.y()), float(r.width()), float(r.height())));
    // cb->setScissor(QRhiScissor(r.x(), r.y(), r.width(), r.height()));

    // const float sx = float(r.width())  / float(ps.width());
    // const float sy = float(r.height()) / float(ps.height());

    // // yUp is false in your logs, so NDC Y is top-to-bottom in window coords.
    // // This formula places the viewport correctly.
    // const float tx = (2.0f * float(r.x()) + float(r.width()))  / float(ps.width())  - 1.0f;
    // float ty;
    // // const float ty = 1.0f - (2.0f * float(r.y()) + float(r.height())) / float(ps.height());
    // if (yUp) {
    //     // r.y is now bottom-left based because we flipped it
    //     ty = (2.0f * float(r.y()) + float(r.height())) / float(ps.height()) - 1.0f;
    // } else {
    //     // r.y is top-left based
    //     ty = 1.0f - (2.0f * float(r.y()) + float(r.height())) / float(ps.height());
    // }

    // QMatrix4x4 viewportNdc;
    // viewportNdc.setToIdentity();
    // viewportNdc(0,0) = sx;
    // viewportNdc(1,1) = sy;
    // viewportNdc(0,3) = tx;
    // viewportNdc(1,3) = ty;

    QMatrix4x4 mvp = rhi->clipSpaceCorrMatrix() * (m_viewProj * model);

    cb->setGraphicsPipeline(m_ps);

    if (!m_mesh || !m_mesh->isReady()) {
        qWarning() << "GXModel: mesh or buffers not ready"
                   << "mesh" << (void*)m_mesh
                   << "vbuf" << (m_mesh ? (void*)m_mesh->vertexBuffer() : nullptr)
                   << "ibuf" << (m_mesh ? (void*)m_mesh->indexBuffer() : nullptr);
        return;
    }

    const int icount = m_mesh->indexCount();
    if (icount <= 0) {
        qWarning() << "GXModel: indexCount is 0";
        return;
    }

    const QRhiCommandBuffer::VertexInput vbufBinding(m_mesh->vertexBuffer(), 0);

    const auto idxFmt = (m_mesh->indexType() == GXMesh::IndexUInt32)
        ? QRhiCommandBuffer::IndexUInt32
        : QRhiCommandBuffer::IndexUInt16;

    cb->setVertexInput(0, 1, &vbufBinding, m_mesh->indexBuffer(), 0, idxFmt);

    const auto &subs = m_mesh->subMeshes();
    const auto& mats = materialsVector();

    if (!subs.isEmpty()) {

        struct DrawCmd { const GXSubMesh* sm; GXMaterial* mat; QRhiShaderResourceBindings* srb; };
        QVector<DrawCmd> draws;
        draws.reserve(subs.size());

        bool anyUboUpdates = false;

    for (int i = 0; i < subs.size(); ++i) {
        const GXSubMesh &sm = subs[i];

        int slot = sm.materialSlot;
        if (slot < 0) {
            slot = i;
        } else if (slot >= 0 && slot < mats.size()) {
            // qDebug() << "[GXModel] slot" << slot << "mat" << mats[slot];
        } else {
            qDebug() << "[GXModel] slot out of range" << slot << "mats.size" << mats.size();
        }

        GXMaterial* useMat = nullptr;

        if (slot >= 0 && slot < mats.size()) useMat = mats[slot];

        if (!useMat) useMat = material();

        if (slot < 0 || slot >= mats.size()) {
            qWarning() << "[GXModel] slot out of range"
                       << "slot=" << slot
                       << "mats.size=" << mats.size()
                       << "submesh=" << i;
        }

        if (!useMat) {
            qWarning() << "[GXModel] no material for submesh" << i;
            continue;
        }
        if (useMat->vsUboSize() > int(m_vsUbuf->size()))
            qWarning() << "VS UBO too large for buffer";
        if (useMat->fsUboSize() > int(m_fsUbuf->size()))
            qWarning() << "FS UBO too large for buffer";

        QRhiShaderResourceBindings* useSrb = srbForMaterial(useMat, cb);
        if (!useSrb) continue;

        anyUboUpdates |= updateUbosFor(useMat, mvp, model, u);


        draws.push_back({ &sm, useMat, useSrb });

        // cb->setShaderResources(useSrb);
        // cb->resourceUpdate(u);
        // cb->drawIndexed(int(sm.indexCount), 1, int(sm.indexOffset), int(sm.baseVertex), 0);

        // static QElapsedTimer s_timer;
        // static bool s_started = false;

        // if (!s_started) {
        //     s_timer.start();
        //     s_started = true;
        // }

        // if (s_timer.elapsed() >= 1000) {
        //     qDebug() << "[GX3D created/sec]"
        //              << "pipelines" << g_pipelinesCreatedThisSecond.exchange(0)
        //              << "srbs"      << g_srbsCreatedThisSecond.exchange(0);
        // //     const int created = g_pipelinesCreatedThisSecond.exchange(0, std::memory_order_relaxed);
        // //     qDebug() << "[GX3D] subs pipelinesCreated/sec =" << created;
        // //     const int batches = g_ruBatchesThisSecond.exchange(0, std::memory_order_relaxed);
        // //     const int updates = g_dynUpdatesThisSecond.exchange(0, std::memory_order_relaxed);
        // //     const qint64 bytes = g_dynBytesThisSecond.exchange(0, std::memory_order_relaxed);

        // //     qDebug() << "[GX3D] subs UBO updates/sec:"
        // //              << "batches=" << batches
        // //              << "updates=" << updates
        // //              << "bytes=" << bytes;

        // //     // keep your pipeline print too if you want
        // //     s_timer.restart();
        // //     // s_timer.restart();

        // }
    }
    if (anyUboUpdates)
        cb->resourceUpdate(u);

    for (const auto& dc : draws) {
        cb->setShaderResources(dc.srb);
        cb->drawIndexed(int(dc.sm->indexCount), 1,
                        int(dc.sm->indexOffset), int(dc.sm->baseVertex), 0);
    }

    } else {
        GXMaterial *useMat0 = nullptr;
        if (!mats.isEmpty()) useMat0 = mats[0];
        if (!useMat0) useMat0 = material();

        if (!useMat0) {
            qWarning() << "[GXModel] no material (fallback draw)";
            return;
        }
        if (useMat0->fsUboSize() > int(m_fsUbuf->size()))
            qWarning() << "FS UBO too large for buffer";

        updateUbosFor(useMat0, mvp, model, u);
        cb->resourceUpdate(u);

        QRhiShaderResourceBindings* useSrb0 = srbForMaterial(useMat0, cb);
        if (!useSrb0) return;

        cb->setShaderResources(useSrb0);
        cb->drawIndexed(m_mesh->indexCount());

        // static QElapsedTimer s_timer;
        // static bool s_started = false;

        // if (!s_started) {
        //     s_timer.start();
        //     s_started = true;
        // }

        // if (s_timer.elapsed() >= 1000) {
        //     const int created = g_pipelinesCreatedThisSecond.exchange(0, std::memory_order_relaxed);
        //     qDebug() << "[GX3D] no subs pipelinesCreated/sec =" << created;
        //     const int batches = g_ruBatchesThisSecond.exchange(0, std::memory_order_relaxed);
        //     const int updates = g_dynUpdatesThisSecond.exchange(0, std::memory_order_relaxed);
        //     const qint64 bytes = g_dynBytesThisSecond.exchange(0, std::memory_order_relaxed);

        //     qDebug() << "[GX3D] no subs UBO updates/sec:"
        //              << "batches=" << batches
        //              << "updates=" << updates
        //              << "bytes=" << bytes;

        //     // keep your pipeline print too if you want
        //     s_timer.restart();
        //     // s_timer.restart();
        // }
    }

    // const int created1 = g_pipelinesCreatedThisSecond.exchange(0, std::memory_order_relaxed);
    // qDebug() << "[GX3D] outside no subs pipelinesCreated/sec =" << created1;
    // const int batches1 = g_ruBatchesThisSecond.exchange(0, std::memory_order_relaxed);
    // const int updates1 = g_dynUpdatesThisSecond.exchange(0, std::memory_order_relaxed);
    // const qint64 bytes1 = g_dynBytesThisSecond.exchange(0, std::memory_order_relaxed);

    // qDebug() << "[GX3D] no subs UBO updates/sec:"
    //          << "batches=" << batches1
    //          << "updates=" << updates1
    //          << "bytes=" << bytes1;
}

void GXModel::releaseResources()
{
    destroyRhiResources();
    GXRenderableNode::releaseResources();
}

void GXModel::recordPick(QRhiCommandBuffer *cb)
{
    if (!cb || !m_mesh) return;

    QRhiBuffer* vb = m_mesh->vertexBuffer();
    QRhiBuffer* ib = m_mesh->indexBuffer();   // <-- MUST exist / be correct
    if (!vb || !ib) return;

    const auto idxFmt = (m_mesh->indexType() == GXMesh::IndexUInt32)
                            ? QRhiCommandBuffer::IndexUInt32
                            : QRhiCommandBuffer::IndexUInt16;

    const QRhiCommandBuffer::VertexInput vInput(vb, 0);
    cb->setVertexInput(0, 1, &vInput, ib, 0, idxFmt);

    cb->drawIndexed(m_mesh->indexCount(), 1, 0, 0, 0);

    // static QElapsedTimer s_timer;
    // static bool s_started = false;

    // if (!s_started) {
    //     s_timer.start();
    //     s_started = true;
    // }

    // if (s_timer.elapsed() >= 1000) {
    //     // const int created = g_pipelinesCreatedThisSecond.exchange(0, std::memory_order_relaxed);
    //     // qDebug() << "[GX3D] record pick pipelinesCreated/sec =" << created;
    //     // const int batches = g_ruBatchesThisSecond.exchange(0, std::memory_order_relaxed);
    //     // const int updates = g_dynUpdatesThisSecond.exchange(0, std::memory_order_relaxed);
    //     // const qint64 bytes = g_dynBytesThisSecond.exchange(0, std::memory_order_relaxed);

    //     // qDebug() << "[GX3D] record pick UBO updates/sec:"
    //     //          << "batches=" << batches
    //     //          << "updates=" << updates
    //     //          << "bytes=" << bytes;

    //     // keep your pipeline print too if you want
    //     s_timer.restart();
    //     s_timer.restart();
    // }
}

QRhiShaderResourceBindings *GXModel::srbForMaterial(GXMaterial *mat, QRhiCommandBuffer *cb)
{
    if (!m_rhi || ! mat) return nullptr;

    if (auto* existing = m_materialSrbs.value(mat, nullptr)) return existing;

    mat->ensureRhi(m_rhi, cb);

    ensureUboForMat(m_rhi, m_vsUbufPerMat, mat, mat->vsUboSize());
    ensureUboForMat(m_rhi, m_fsUbufPerMat, mat, mat->fsUboSize());

    QRhiBuffer* vsBuf = m_vsUbufPerMat.value(mat, nullptr);
    QRhiBuffer* fsBuf = m_fsUbufPerMat.value(mat, nullptr);

    // g_srbsCreatedThisSecond.fetch_add(1, std::memory_order_relaxed);
    auto* srb = m_rhi->newShaderResourceBindings();
    QVector<QRhiShaderResourceBinding> bindings;
    bindings.reserve(4);

    bindings.append(QRhiShaderResourceBinding::uniformBuffer(mat->vsBinding(), QRhiShaderResourceBinding::VertexStage, vsBuf));
    bindings.append(QRhiShaderResourceBinding::uniformBuffer(mat->fsBinding(), QRhiShaderResourceBinding::FragmentStage, fsBuf));

    if (m_frameLightingUbo) {
        bindings.append(QRhiShaderResourceBinding::uniformBuffer(2, QRhiShaderResourceBinding::FragmentStage, m_frameLightingUbo));
    }

    bindings.append(QRhiShaderResourceBinding::sampledTexture(3, QRhiShaderResourceBinding::FragmentStage, mat->baseColorTex(), mat->baseColorSampler()));

    if (mat->normalTex() && mat->normalSampler()) {
        bindings.append(QRhiShaderResourceBinding::sampledTexture(4, QRhiShaderResourceBinding::FragmentStage, mat->normalTex(), mat->normalSampler()));
    }

    if (m_environmentUbo) {
        bindings.append(QRhiShaderResourceBinding::uniformBuffer(5, QRhiShaderResourceBinding::FragmentStage, m_environmentUbo));
    }

    if (m_brdfLutTex && m_brdfLutSampler) {
        bindings.append(QRhiShaderResourceBinding::sampledTexture(6, QRhiShaderResourceBinding::FragmentStage, m_brdfLutTex, m_brdfLutSampler));
    }

    if (m_envCubeTex && m_envCubeSampler) {
        bindings.append(QRhiShaderResourceBinding::sampledTexture(7, QRhiShaderResourceBinding::FragmentStage, m_envCubeTex, m_envCubeSampler));
    }

    if (m_prefilterSpecCubeTex && m_prefilterSpecCubeSampler) {
        bindings.append(QRhiShaderResourceBinding::sampledTexture(8, QRhiShaderResourceBinding::FragmentStage, m_prefilterSpecCubeTex, m_prefilterSpecCubeSampler));
    }

    if (m_irradianceCubeTex && m_irradianceCubeSampler) {
        bindings.append(QRhiShaderResourceBinding::sampledTexture(9, QRhiShaderResourceBinding::FragmentStage, m_irradianceCubeTex, m_irradianceCubeSampler));
    }

    srb->setBindings(bindings.cbegin(), bindings.cend());
    if (!srb->create()) {
        qWarning() << "GXModel: per-material SRB create failed";
        delete srb;
        return nullptr;
    }

    m_materialSrbs.insert(mat, srb);
    return srb;
}

bool GXModel::ensureMeshLoaded(QString *err)
{
    if (!m_source.endsWith(".mesh")) return true;

    if (m_meshLoaded && m_loadedMeshSource == m_source) return true;

    QString e;
    GXMeshData cpu;
    GXMeshReader::Options opt{ true, true };

    const QString filePath = m_source;

    if (!GXMeshReader().read(filePath, cpu, &e, opt)) {
        if (err) *err = e;
        return false;
    }

    computeBoundsIfMissing(cpu);

    m_meshCpu = std::move(cpu);
    m_loadedMeshSource = m_source;
    m_meshLoaded = true;

    if (!m_mesh) {
        m_mesh = new GXMesh();
        emit meshChanged();
    }

    m_mesh->setCpuData(m_meshCpu);

    return true;
}

void GXModel::destroyRhiResources()
{
    if (m_ps) { m_ps->destroy(); delete m_ps; m_ps = nullptr; }
    if (m_vsUbuf) { m_vsUbuf->destroy(); delete m_vsUbuf; m_vsUbuf = nullptr; }
    if (m_fsUbuf) { m_fsUbuf->destroy(); delete m_fsUbuf; m_fsUbuf = nullptr; }
    for (auto* srb : std::as_const(m_materialSrbs)) {
        if (srb) { srb->destroy(); delete srb; }
    }
    m_materialSrbs.clear();
    m_pipelineDirty = true;
}

void GXModel::destroyPipelineResources()
{
    if (m_ps) { m_ps->destroy(); delete m_ps; m_ps = nullptr; }
    if (m_vsUbuf) { m_vsUbuf->destroy(); delete m_vsUbuf; m_vsUbuf = nullptr; }
    if (m_fsUbuf) { m_fsUbuf->destroy(); delete m_fsUbuf; m_fsUbuf = nullptr; }
    for (auto* srb : std::as_const(m_materialSrbs)) {
        if (srb) { srb->destroy(); delete srb; }
    }
    m_materialSrbs.clear();
}
