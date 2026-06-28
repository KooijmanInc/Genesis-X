// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXENVIRONMENTBUILDJOB_H
#define GXENVIRONMENTBUILDJOB_H

#include <QRunnable>

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

#include <GenesisX/GX3D/Render/Nodes/GXSceneRenderNode.h>

namespace gx::gx3d {

class GENESISX_GX3D_EXPORT GXEnvironmentBuildJob : public QRunnable
{
public:
    GXEnvironmentBuildJob(render::GXSceneRenderNode */*node*/) {}
        // : m_node(node) {}

    void run() override
    {
        // if (!m_node || !m_node->m_window)
        //     return;
        // qDebug() << "run activated got node and window";

        // QRhi *rhi = m_node->m_window->rhi();
        // if (!rhi)
        //     return;
        // qDebug() << "run activated got rhi";

        // // Safe here — Qt has a current command buffer internally
        // QRhiCommandBuffer *cb =
        //     static_cast<QRhiCommandBuffer*>(
        //         m_node->m_window->rendererInterface()
        //             ->getResource(m_node->m_window,
        //                           QSGRendererInterface::RhiRedirectCommandBuffer)
        //         );
        // m_node->commandBuffer();

        // if (!cb) {
        //     cb = m_node->commandBuffer();
        //     qDebug() << cb;
        //     if (!cb) return;
        // }
        // qDebug() << "run activated got cb";

        // m_node->m_window->beginExternalCommands();

        // if (!m_node->m_brdfLutUploaded)
        //     m_node->ensureBrdfLut(rhi, cb);

        // if (!m_node->m_envCubeUploaded)
        //     m_node->ensureEnvCube(rhi, cb);

        // if (!m_node->m_prefilterSpecCubeBuilt)
        //     m_node->ensurePrefilterSpecCube(rhi, cb);

        // m_node->m_window->endExternalCommands();
    }

private:
    // render::GXSceneRenderNode *m_node;
};

}

#endif // GXENVIRONMENTBUILDJOB_H
