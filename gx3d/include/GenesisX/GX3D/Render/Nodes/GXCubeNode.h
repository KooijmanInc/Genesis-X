// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXCUBENODE_H
#define GXCUBENODE_H

#include <QColor>
#include <QMatrix4x4>
#include <rhi/qrhi.h>

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

#include <GenesisX/GX3D/Render/Resources/GXMesh.h>
#include <GenesisX/GX3D/Render/Nodes/GXRenderableNode.h>

class QRhiBuffer;
class QRhiGraphicsPipeline;
class QRhiShaderResourceBindings;

namespace gx::gx3d::render {

class GENESISX_GX3D_EXPORT GXCubeNode : public GXRenderableNode
{
    Q_OBJECT

    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QVector3D position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(QQuaternion rotation READ rotation WRITE setRotation NOTIFY rotationChanged)
    Q_PROPERTY(QVector3D scale READ scale WRITE setScale NOTIFY scaleChanged)

public:
    explicit GXCubeNode(QObject* parent = nullptr);
    ~GXCubeNode() override;

    QColor color() const { return m_color; }
    void setColor(const QColor& c);
    void setMvp(const QMatrix4x4& mvp);

    QVector3D position() const { return m_position; }
    void setPosition(const QVector3D& pos);

    QQuaternion rotation() const { return m_rotation; }
    void setRotation(const QQuaternion& rot);

    QVector3D scale() const { return m_scale; }
    void setScale(const QVector3D& s);

    void ensureResources(QRhi* rhi, QRhiRenderTarget* rt, QRhiCommandBuffer* cb) override;
    void recordRender(QRhiCommandBuffer* cb, QRhiRenderTarget* rt) override;
    void releaseResources() override;

signals:
    void colorChanged();
    void positionChanged();
    void rotationChanged();
    void scaleChanged();

private:
    void destroyRhiResources();

    GXMesh* m_mesh = nullptr;

    QColor m_color = Qt::white;
    QVector3D m_position;
    QQuaternion m_rotation;
    QVector3D m_scale;
    QMatrix4x4 m_mvp;

    QShader m_vs, m_fs;

    QRhiBuffer* m_vbuf = nullptr;
    QRhiBuffer* m_ibuf = nullptr;
    QRhiBuffer* m_ubuf = nullptr;

    QRhiShaderResourceBindings* m_srb = nullptr;
    QRhiGraphicsPipeline* m_ps = nullptr;

    int m_indexCount = 0;
    bool m_uploaded = false;

};

}

#endif // GXCUBENODE_H
