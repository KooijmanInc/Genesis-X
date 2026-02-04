// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Query/GXIntersect.h>

namespace gx::gx3d::query {

static inline float safeInv(float v)
{
    const float eps = 1e-8f;

    return (std::abs(v) < eps) ? std::numeric_limits<float>::infinity() : (1.0f / v);
}

bool intersectRayAabb(const GXRay &ray, const GXAabb &b, float &outT, QVector3D &outNormal)
{
    float tmin = 0.0f;
    float tmax = std::numeric_limits<float>::infinity();
    outNormal = QVector3D(0, 0, 0);

    const QVector3D invDir(safeInv(ray.dirWS.x()), safeInv(ray.dirWS.y()), safeInv(ray.dirWS.z()));

    int hitAxis = -1;
    float hitSign = 0.0f;

    auto testAxis = [&](float origin, float invD, float minB, float maxB, int axis) -> bool {
        float t1 = (minB - origin) * invD;
        float t2 = (maxB - origin) * invD;

        float enter = std::min(t1, t2);
        float exit = std::max(t1, t2);

        if (enter > tmin) {
            tmin = enter;
            hitAxis = axis;
            hitSign = (t1 < t2) ? -1.0f : +1.0f;
        }

        tmax = std::min(tmax, exit);
        return tmax >= tmin;
    };

    if (!testAxis(ray.originWS.x(), invDir.x(), b.minWS.x(), b.maxWS.x(), 0)) return false;
    if (!testAxis(ray.originWS.y(), invDir.y(), b.minWS.y(), b.maxWS.y(), 1)) return false;
    if (!testAxis(ray.originWS.z(), invDir.z(), b.minWS.z(), b.maxWS.z(), 2)) return false;

    // If tmin is behind origin, try tmax (ray starts inside the box)
    float tHit = (tmin >= 0.0f) ? tmin : tmax;
    if (tHit < 0.0f || !std::isfinite(tHit)) return false;

    outT = tHit;

    // Normal from axis
    if (hitAxis == 0) outNormal = QVector3D(hitSign, 0, 0);
    else if (hitAxis == 1) outNormal = QVector3D(0, hitSign, 0);
    else if (hitAxis == 2) outNormal = QVector3D(0, 0, hitSign);

    return true;
}

}
