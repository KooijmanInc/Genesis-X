// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/GXView3D.h>
#include "render/GXClearNode.h"

#include <QSGSimpleRectNode>

#include <QtGlobal>

using namespace gx::gx3d;
using namespace gx::render;


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

QSGNode *GXView3D::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    auto* node = static_cast<GXClearNode*>(oldNode);
    if (!node) {
        node = new GXClearNode();
    }

    node->setRect(QRectF(0, 0, width(), height()));
    node->setColor(m_clearColor);

    if (m_renderMode == Continuous && canRenderContinuously()) {
        update();
    }

    return node;
}

void GXView3D::itemChange(ItemChange change, const ItemChangeData &data)
{
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
