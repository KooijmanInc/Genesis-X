// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXFILEWRITER_H
#define GXFILEWRITER_H

#include <GenesisX/genesisx_global.h>

// NOTE:
// GXFileWriter is a low-level IO utility.
// It must not depend on GX3D, QML, or asset semantics.
// Only file mechanics live here.

#include <QFile>
#include <QString>
#include <QSaveFile>
#include <QByteArray>

namespace gx::utils::io {

class GENESISX_CORE_EXPORT GXFileWriter
{
public:
    enum class Mode {
        Binary,
        Text
    };

    enum class Endian {
        Little,
        Big
    };

    // For chunked binary formats
    struct ChunkRef {
        qint64 sizePos = -1;   // where the size field lives
        qint64 dataStart = -1; // start of payload
        quint32 id = 0;
        bool valid() const { return sizePos >= 0 && dataStart >= 0; }
    };

    explicit GXFileWriter(const QString& filePath, Mode mode = Mode::Binary, Endian endian = Endian::Little);
    ~GXFileWriter();

    GXFileWriter(const GXFileWriter&) = delete;
    GXFileWriter& operator=(const GXFileWriter&) = delete;

    // ─────────────────────────────
    // Lifecycle
    // ─────────────────────────────
    bool open();
    void close();

    // Atomic write: writes to temp file and commits on close()
    // Recommended for converter outputs.
    void setAtomic(bool on);
    bool isAtomic() const { return m_atomic; }

    bool isOpen() const;
    bool hasError() const {return !m_errorString.isEmpty(); }
    QString errorString() const { return m_errorString; }
    QString filePath() const { return m_filePath; }

    // ─────────────────────────────
    // Positioning
    // ─────────────────────────────
    qint64 pos() const;
    bool seek(qint64 p);

    // ─────────────────────────────
    // Text helpers (Mode::Text)
    // ─────────────────────────────
    void setIndentWidth(int spaces);
    void pushIndent();
    void popIndent();
    int indentLevel() const { return m_indentLevel; }

    bool write(const QString& text);                 // raw text (no newline)
    bool writeLine(const QString& line = QString()); // line + '\n'
    bool writeIndent();                              // writes indent spaces

    // ─────────────────────────────
    // Raw bytes (both modes)
    // ─────────────────────────────
    bool writeBytes(const void* data, qsizetype size);
    bool writeBytes(const QByteArray& bytes);

    // ─────────────────────────────
    // Binary primitives (Mode::Binary)
    // Always written using configured endianness
    // ─────────────────────────────
    bool writeU8(quint8 v);
    bool writeI8(qint8 v);

    bool writeU16(quint16 v);
    bool writeI16(qint16 v);

    bool writeU32(quint32 v);
    bool writeI32(qint32 v);

    bool writeU64(quint64 v);
    bool writeI64(qint64 v);

    bool writeF32(float v);
    bool writeF64(double v);

    // Common binary string patterns
    bool writeStringUtf8Z(const QString& s);   // null-terminated utf-8
    bool writeStringUtf8U32(const QString& s); // length (u32) + utf-8 bytes

    // ─────────────────────────────
    // Chunked binary helpers
    // Layout:
    //   [u32 chunkId][u32 chunkSize][payload...]
    // chunkSize is payload size in bytes (not including header)
    // ─────────────────────────────
    static constexpr quint32 fourcc(char a, char b, char c, char d)
    {
        return (quint32(quint8(a)) << 24) |
               (quint32(quint8(b)) << 16) |
               (quint32(quint8(c)) << 8)  |
               (quint32(quint8(d)) << 8);
    }

    ChunkRef beginChunk(quint32 chunkId);
    bool endChunk(const ChunkRef& ref);

private:
    bool setError(const QString& msg) const;
    bool ensureOpenFor(Mode expected) const;

    //endian helpers
    quint16 swap16(quint16 v) const;
    quint32 swap32(quint32 v) const;
    quint64 swap64(quint64 v) const;

    template <typename T>
    bool writePOD(const T& pod);

private:
    QString m_filePath;
    Mode m_mode = Mode::Binary;
    Endian m_endian = Endian::Little;
    bool m_atomic = false;

    QSaveFile* m_save = nullptr;
    QFile* m_file = nullptr;

    int m_indentWidth = 4;
    int m_indentLevel = 0;

    mutable QString m_errorString;
};

}

#endif // GXFILEWRITER_H
