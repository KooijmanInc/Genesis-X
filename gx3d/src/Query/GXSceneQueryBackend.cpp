// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Query/GXSceneQueryBackend.h>
#include <GenesisX/GX3D/Query/GXIntersect.h>

#include <GenesisX/GX3D/Scene/GXScene.h>
#include <GenesisX/GX3D/Scene/Nodes/GXNode.h>

#include <GenesisX/GX3D/Render/Nodes/GXModelNode.h>

namespace gx::gx3d::query {

static GXAabb worldAabbFromLocalBounds(const QMatrix4x4& world, QVector3D& minLS, QVector3D& maxLS)
{
    QVector3D corners[8] = {
                            {minLS.x(), minLS.y(), minLS.z()},
                            {maxLS.x(), minLS.y(), minLS.z()},
                            {minLS.x(), maxLS.y(), minLS.z()},
                            {maxLS.x(), maxLS.y(), minLS.z()},
                            {minLS.x(), minLS.y(), maxLS.z()},
                            {maxLS.x(), minLS.y(), maxLS.z()},
                            {minLS.x(), maxLS.y(), maxLS.z()},
                            {maxLS.x(), maxLS.y(), maxLS.z()},
                            };

    QVector3D minW(+std::numeric_limits<float>::infinity(),
                   +std::numeric_limits<float>::infinity(),
                   +std::numeric_limits<float>::infinity());
    QVector3D maxW(-std::numeric_limits<float>::infinity(),
                   -std::numeric_limits<float>::infinity(),
                   -std::numeric_limits<float>::infinity());

    for (const auto& c : corners) {
        const QVector3D p = world.map(c);
        minW.setX(std::min(minW.x(), p.x()));
        minW.setY(std::min(minW.y(), p.y()));
        minW.setZ(std::min(minW.z(), p.z()));
        maxW.setX(std::max(maxW.x(), p.x()));
        maxW.setY(std::max(maxW.y(), p.y()));
        maxW.setZ(std::max(maxW.z(), p.z()));
    }

    return { minW, maxW };
}

GXSceneQueryBackend::GXSceneQueryBackend(scene::GXScene *scene)
    : m_scene{scene}
{
}

GXRayHit GXSceneQueryBackend::raycast(const GXRay &ray, int minPriority) const
{
    GXRayHit best;
    float bestT = std::numeric_limits<float>::infinity();
    // float bestVol = std::numeric_limits<float>::infinity();
    // int bestPriority = std::numeric_limits<int>::min();

    if (!m_scene) return best;

    m_scene->traverse([&](gx::gx3d::scene::GXNode* n) {
        auto* model = qobject_cast<gx::gx3d::render::GXModel*>(n);
        if (!model) return;
        // qDebug() << "[raycast] node:" << n->metaObject()->className()
        //          << "name:" << n->objectName();

        if (!model->pickable()) return;
        if (model->pickPriority() < minPriority) return;
        // qDebug() << "[raycast]   model pickable=" << model->pickable()
        //          << "source=" << model->source();

        QVector3D minLS, maxLS;
        model->localBounds(minLS, maxLS);
        const GXAabb box = worldAabbFromLocalBounds(model->worldMatrix(), minLS, maxLS);

        float tHit = 0.0f;
        QVector3D nrm;
        if (!intersectRayAabb(ray, box, tHit, nrm)) return;

        if (!best.hit || tHit < bestT) {
            best.hit = true;
            bestT = tHit;
            best.t = tHit;
            best.positionWS = ray.originWS + ray.dirWS * tHit;
            best.normalWS = nrm;
            best.node = model;
        }


        // QVector3D minLS, maxLS;
        // model->localBounds(minLS, maxLS);

        // const QMatrix4x4 world = model->worldMatrix();
        // const GXAabb box = worldAabbFromLocalBounds(world, minLS, maxLS);

        // float tHit = 0.0f;
        // QVector3D nrm;
        // if (!intersectRayAabb(ray, box, tHit, nrm)) return;

        // auto volume = [](const GXAabb& b) {
        //     const QVector3D s = b.maxWS - b.minWS;
        //     return s.x() * s.y() * s.z();
        // };

        // const float eps = 0.1f; // 2 cm; tune per your scale
        // const int prio = model->pickPriority();
        // // qDebug() << (std::abs(tHit - bestT) <= eps && prio > bestPriority);
        // if (!best.hit || tHit < bestT - eps || (std::abs(tHit - bestT) <= eps && prio > bestPriority)) {
        //     bestT = tHit;
        //     best.hit = true;
        //     best.t = tHit;
        //     best.positionWS = ray.originWS + ray.dirWS * tHit;
        //     best.normalWS = nrm;
        //     best.node = model;
        //     bestPriority = prio;
        //     qDebug() << "have prio" << prio;
        //     bestVol = volume(box);
        // } else if (std::abs(tHit - bestT) <= eps) {
        //     const float v = volume(box);
        //     if (v < bestVol) {
        //         bestT = tHit;
        //         best.t = tHit;
        //         best.positionWS = ray.originWS + ray.dirWS * tHit;
        //         best.normalWS = nrm;
        //         best.node = model;
        //         bestVol = v;
        //     }
        // }

        // if (tHit < bestT) {
        //     bestT = tHit;
        //     best.hit = true;
        //     best.t = tHit;
        //     best.positionWS = ray.originWS + ray.dirWS * tHit;
        //     best.normalWS = nrm;
        //     best.node = model;
        // }
    });

    return best;
}

}
