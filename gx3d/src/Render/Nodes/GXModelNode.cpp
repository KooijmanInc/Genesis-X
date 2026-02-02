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

using namespace gx::gx3d::render;

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

    if (m_rhi == rhi && m_ps && m_vsUbuf && m_fsUbuf && !m_pipelineDirty && m_boundLightingUbo == m_frameLightingUbo) return;

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
        QRhiVertexInputAttribute(0, 2, QRhiVertexInputAttribute::Float2,  offsetof(GXMesh::Vertex, u))
    });

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
}

void GXModel::recordRender(QRhiCommandBuffer *cb, QRhiRenderTarget *rt)
{
    if (!cb || !rt) return;

    QRhi* rhi = cb->rhi();
    ensureResources(rhi, rt, cb);

    if (m_mesh) m_mesh->uploadIfNeeded(rhi, cb);

    if (!m_ps) return;

    if (!m_vsUbuf || !m_fsUbuf) {
        qWarning() << "GXModel::recordRender: missing UBO buffers"
                   << "vsUbuf=" << (void*)m_vsUbuf
                   << "fsUbuf=" << (void*)m_fsUbuf;
        return;
    }

    QMatrix4x4 model = worldMatrix();

    QMatrix4x4 mvp = rhi->clipSpaceCorrMatrix() * (m_viewProj * model);

    auto updateUbosFor = [&](GXMaterial* m, const QMatrix4x4& mvpIn, const QMatrix4x4& modelIn) -> QVector4D {

        QByteArray vsData;
        vsData.resize(m->vsUboSize());
        if (!vsData.isEmpty())
            m->fillVS(vsData.data(), mvpIn, modelIn);

        QByteArray fsData;
        fsData.resize(m->fsUboSize());
        if (!fsData.isEmpty())
            m->fillFS(fsData.data());

        QVector4D preview(0, 0, 0, 0);
        if (fsData.size() >= 16) {
            const float *f = reinterpret_cast<const float*>(fsData.constData());
            preview = QVector4D(f[0], f[1], f[2], f[3]);
        }

        QRhiBuffer* vsBuf = m_vsUbufPerMat.value(m, nullptr);
        QRhiBuffer* fsBuf = m_fsUbufPerMat.value(m, nullptr);

        if (!vsBuf || !fsBuf) {
            qWarning() << "GXModel: missing per-mat UBO for" << m;
            return preview;
        }

        {
            QRhiResourceUpdateBatch* u = rhi->nextResourceUpdateBatch();
            if (!vsData.isEmpty())
                u->updateDynamicBuffer(vsBuf, 0, vsData.size(), vsData.constData());
            if (!fsData.isEmpty())
                u->updateDynamicBuffer(fsBuf, 0, fsData.size(), fsData.constData());
            cb->resourceUpdate(u);
        }

        return preview;
    };

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

    const QSize ps = rt->pixelSize();
    cb->setViewport(QRhiViewport(0, 0, float(ps.width()), float(ps.height())));
    cb->setScissor(QRhiScissor(0, 0, ps.width(), ps.height()));

    const auto &subs = m_mesh->subMeshes();
    const auto& mats = materialsVector();
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

        updateUbosFor(useMat, mvp, model);

        cb->setShaderResources(useSrb);
        cb->drawIndexed(int(sm.indexCount), 1, int(sm.indexOffset), int(sm.baseVertex), 0);
    }

    if (subs.isEmpty()) {
        GXMaterial *useMat0 = nullptr;
        if (!mats.isEmpty()) useMat0 = mats[0];
        if (!useMat0) useMat0 = material();

        if (!useMat0) {
            qWarning() << "[GXModel] no material (fallback draw)";
            return;
        }
        if (useMat0->fsUboSize() > int(m_fsUbuf->size()))
            qWarning() << "FS UBO too large for buffer";

        updateUbosFor(useMat0, mvp, model);
        QRhiShaderResourceBindings* useSrb0 = srbForMaterial(useMat0, cb);
        if (!useSrb0) return;
        cb->setShaderResources(useSrb0);
        cb->drawIndexed(m_mesh->indexCount());
    }
}

void GXModel::releaseResources()
{
    destroyRhiResources();
    GXRenderableNode::releaseResources();
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

    auto* srb = m_rhi->newShaderResourceBindings();
    QVector<QRhiShaderResourceBinding> bindings;
    bindings.reserve(4);

    bindings.append(QRhiShaderResourceBinding::uniformBuffer(mat->vsBinding(), QRhiShaderResourceBinding::VertexStage, vsBuf));
    bindings.append(QRhiShaderResourceBinding::uniformBuffer(mat->fsBinding(), QRhiShaderResourceBinding::FragmentStage, fsBuf));

    if (m_frameLightingUbo) {
        bindings.append(QRhiShaderResourceBinding::uniformBuffer(2, QRhiShaderResourceBinding::FragmentStage, m_frameLightingUbo));
    }

    bindings.append(QRhiShaderResourceBinding::sampledTexture(3, QRhiShaderResourceBinding::FragmentStage, mat->baseColorTex(), mat->baseColorSampler()));

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
