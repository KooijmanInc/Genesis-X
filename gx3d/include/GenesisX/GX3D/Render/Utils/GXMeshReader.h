// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXMESHREADER_H
#define GXMESHREADER_H

#include <QString>

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

#include <GenesisX/GX3D/Render/Utils/GXMeshData.h>

namespace gx::gx3d::render {

class GENESISX_GX3D_EXPORT GXMeshReader
{
public:
    struct Options {
        bool strict = true;
        bool requireSubMeshes = true;
    };

    GXMeshReader() = default;

    bool read(const QString& filePath, GXMeshData& outMesh, QString* errorString = nullptr, const Options& opt = Options{true, true}) const;

    static bool readFile(const QString& filePath, GXMeshData& outMesh, QString* errorString = nullptr, const Options& opt = Options{true, true});
};

}

#endif // GXMESHREADER_H
