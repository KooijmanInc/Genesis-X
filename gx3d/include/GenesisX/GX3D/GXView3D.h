// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXVIEW3D_H
#define GXVIEW3D_H

#include <QColor>
#include <QTimer>
#include <QQuickItem>

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

namespace gx::gx3d {

class GENESISX_GX3D_EXPORT GXView3D : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QColor clearColor READ clearColor WRITE setClearColor NOTIFY clearColorChanged)

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

signals:
    void clearColorChanged();
    void renderModeChanged();
    void targetFpsChanged();

protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override;
    void itemChange(ItemChange change, const ItemChangeData& data) override;

private:
    void applyRenderPolicy();
    bool canRenderContinuously() const;

private:
    QColor m_clearColor = Qt::black;

    RenderMode m_renderMode = OnDemand;
    int m_targetFps = 30;

    QTimer m_tickTimer;
};

}

#endif // GXVIEW3D_H
