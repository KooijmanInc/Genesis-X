// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/utils/io/GXFileWriter.h>

#include <QIODevice>

namespace gx::utils::io {

static QIODevice::OpenMode openModeFor(GXFileWriter::Mode mode)
{
    QIODevice::OpenMode m = QIODevice::WriteOnly | QIODevice::Truncate;
    if (mode == GXFileWriter::Mode::Text) m |= QIODevice::Text;

    return m;
}

GXFileWriter::GXFileWriter(const QString &filePath, Mode mode, Endian endian)
    : m_filePath(filePath)
    , m_mode(mode)
    , m_endian(endian)
{
}

GXFileWriter::~GXFileWriter()
{
    close();
}

bool GXFileWriter::open()
{
    close();
    m_errorString.clear();

    const QIODevice::OpenMode om = openModeFor(m_mode);

    if (m_atomic) {
        m_save = new QSaveFile(m_filePath);
        if (!m_save->open(om)) {
            const QString err = m_save->errorString();
            delete m_save;
            m_save = nullptr;
            return setError(QStringLiteral("[GXFileWriter] Failed to open (atomic) '%1': %2").arg(m_filePath, err));
        }
    } else {
        m_file = new QFile(m_filePath);
        if (!m_file->open(om)) {
            const QString err = m_file->errorString();
            delete m_file;
            m_file = nullptr;
            return setError(QStringLiteral("[GXFileWriter] Failed to open '%1': %2").arg(m_filePath, err));
        }
    }

    return true;
}

void GXFileWriter::close()
{
    if (m_save) {
        // commit() writes temp file to final destination atomically
        if (m_save->isOpen()) {
            if (!m_save->commit()) {
                setError(QStringLiteral("[GXFileWriter] Failed to commit '%1': %2").arg(m_filePath, m_save->errorString()));
            }
        }
        delete m_save;
        m_save = nullptr;
    }

    if (m_file) {
        if (m_file->isOpen()) m_file->close();
        delete m_file;
        m_file = nullptr;
    }
}

void GXFileWriter::setAtomic(bool on)
{
    m_atomic = on;
}

bool GXFileWriter::isOpen() const
{
    if (m_save) return m_save->isOpen();
    if (m_file) return m_file->isOpen();

    return false;
}

qint64 GXFileWriter::pos() const
{
    if (m_save) return m_save->pos();
    if (m_file) return m_file->pos();

    return -1;
}

bool GXFileWriter::seek(qint64 p)
{
    if (m_save) return m_save->seek(p);
    if (m_file) return m_file->seek(p);
    return setError(QStringLiteral("[GXFileWriter] seek() called while file not open"));
}

void GXFileWriter::setIndentWidth(int spaces)
{
    if (spaces < 0) spaces = 0;
    m_indentWidth = spaces;
}

void GXFileWriter::pushIndent()
{
    ++m_indentLevel;
}

void GXFileWriter::popIndent()
{
    if (m_indentLevel > 0) --m_indentLevel;
}

bool GXFileWriter::write(const QString &text)
{
    if (!ensureOpenFor(Mode::Text)) return false;

    const QByteArray bytes = text.toUtf8();

    return writeBytes(bytes);
}

bool GXFileWriter::writeLine(const QString &line)
{
    if (!ensureOpenFor(Mode::Text)) return false;

    if (!line.isEmpty()) {
        if (!write(line)) return false;
    }

    return writeBytes("\n", 1);
}

bool GXFileWriter::writeIndent()
{
    if (!ensureOpenFor(Mode::Text)) return false;

    const int count = m_indentLevel * m_indentWidth;
    if (count <= 0) return true;

    QByteArray spaces(count, ' ');

    return writeBytes(spaces);
}

bool GXFileWriter::writeBytes(const void *data, qsizetype size)
{
    if (!isOpen()) return setError(QStringLiteral("[GXFileWriter] writeBytes() called while file not open"));

    if (size <= 0) return true;

    QIODevice* dev = m_save ? static_cast<QIODevice*>(m_save) : static_cast<QIODevice*>(m_file);
    const qint64 written = dev->write(static_cast<const char*>(data), qint64(size));
    if (written != qint64(size)) {
        const QString err = dev->errorString();
        return setError(QStringLiteral("[GXFileWriter] Write failed for '%1': %2").arg(m_filePath, err));
    }
    return true;
}

bool GXFileWriter::writeBytes(const QByteArray &bytes)
{
    return writeBytes(bytes.constData(), bytes.size());
}

bool GXFileWriter::writeU8(quint8 v)
{
    return writeBytes(&v, 1);
}

bool GXFileWriter::writeI8(qint8 v)
{
    return writeBytes(&v, 1);
}

bool GXFileWriter::writeU16(quint16 v)
{
    if (!ensureOpenFor(Mode::Binary)) return false;

    if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
        v = (m_endian == Endian::Little) ? swap16(v) : v;
    } else {
        v = (m_endian == Endian::Big) ? swap16(v) : v;
    }

    return writeBytes(&v, 2);
}

bool GXFileWriter::writeI16(qint16 v)
{
    return writeU16(quint16(v));
}

bool GXFileWriter::writeU32(quint32 v)
{
    if (!ensureOpenFor(Mode::Binary)) return false;

    if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
        v = (m_endian == Endian::Little) ? swap32(v) : v;
    } else {
        v = (m_endian == Endian::Big) ? swap32(v) : v;
    }

    return writeBytes(&v, 4);
}

bool GXFileWriter::writeI32(qint32 v)
{
    return writeU32(quint32(v));
}

bool GXFileWriter::writeU64(quint64 v)
{
    if (!ensureOpenFor(Mode::Binary)) return false;

    if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
        v = (m_endian == Endian::Little) ? swap64(v) : v;
    } else {
        v = (m_endian == Endian::Big) ? swap64(v) : v;
    }

    return writeBytes(&v, 8);
}

bool GXFileWriter::writeI64(qint64 v)
{
    return writeU64(quint64(v));
}

bool GXFileWriter::writeF32(float v)
{
    static_assert(sizeof(float) == 4, "[GXFileWriter] float must be 32-bit");
    quint32 bits;
    memcpy(&bits, &v, 4);

    return writeU32(bits);
}

bool GXFileWriter::writeF64(double v)
{
    static_assert(sizeof(double) == 8, "[GXFileWriter] float must be 64-bit");
    quint64 bits;
    memcpy(&bits, &v, 8);

    return writeU64(bits);
}

bool GXFileWriter::writeStringUtf8Z(const QString &s)
{
    if (!ensureOpenFor(Mode::Binary)) return false;

    QByteArray b = s.toUtf8();
    if (!writeBytes(b)) return false;
    const char zero = '\0';

    return writeBytes(&zero, 1);
}

bool GXFileWriter::writeStringUtf8U32(const QString &s)
{
    if (!ensureOpenFor(Mode::Binary)) return false;

    const QByteArray b = s.toUtf8();
    if (!writeU32(quint32(b.size()))) return false;

    return writeBytes(b);
}

GXFileWriter::ChunkRef GXFileWriter::beginChunk(quint32 chunkId)
{
    ChunkRef ref;
    if (!ensureOpenFor(Mode::Binary)) return ref;

    // [u32 id][u32 size][payload...]
    if (!writeU32(chunkId)) return ref;

    ref.id = chunkId;
    ref.sizePos = pos();
    if (ref.sizePos < 0) return {};

    // placeholder size
    if (!writeU32(0)) return {};

    ref.dataStart = pos();

    return ref;
}

bool GXFileWriter::endChunk(const ChunkRef &ref)
{
    if (!ensureOpenFor(Mode::Binary)) return false;

    if (!ref.valid()) return setError(QStringLiteral("[GXFileWriter] endChunk() called with invalid ChunkRef"));

    const qint64 endPos = pos();
    if (endPos < 0) return setError(QStringLiteral("[GXFileWriter] endChunk() failed: invalid file position"));

    const qint64 payloadSize = endPos - ref.dataStart;
    if (payloadSize < 0 || payloadSize > std::numeric_limits<quint32>::max()) return setError(QStringLiteral("[GXFileWriter] Chunk too large or invalid size"));

    // write size
    if (!seek(ref.sizePos)) return false;

    if (!writeU32(quint32(payloadSize))) return false;

    // return to end
    if (!seek(endPos)) return false;

    return true;
}

bool GXFileWriter::setError(const QString &msg) const
{
    if (m_errorString.isEmpty()) m_errorString = msg;

    return false;
}

bool GXFileWriter::ensureOpenFor(Mode expected) const
{
    if (!isOpen()) return setError(QStringLiteral("[GXFileWriter] file not open: %1").arg(m_filePath));

    // Allow using text helpers only in Text mode, binary helpers only in Binary mode
    if (expected != m_mode) {
        const char *exp = (expected == Mode::Binary) ? "Binary" : "Text";
        const char *act = (m_mode == Mode::Binary) ? "Binary" : "Text";
        return setError(QStringLiteral("Writer mode mismatch for '%1': expected %2, is %3")
                            .arg(m_filePath, QString::fromLatin1(exp), QString::fromLatin1(act)));
    }

    return true;
}

quint16 GXFileWriter::swap16(quint16 v) const
{
    return (v >> 8) | (v << 8);
}

quint32 GXFileWriter::swap32(quint32 v) const
{
    return ((v & 0x000000FFu) << 24) |
           ((v & 0x0000FF00u) << 8)  |
           ((v & 0x00FF0000u) << 8)  |
           ((v & 0xFF000000u) << 24);
}

quint64 GXFileWriter::swap64(quint64 v) const
{
    return ((v & 0x00000000000000FFull) << 56) |
           ((v & 0x000000000000FF00ull) << 40) |
           ((v & 0x0000000000FF0000ull) << 24) |
           ((v & 0x00000000FF000000ull) << 8)  |
           ((v & 0x000000FF00000000ull) << 8)  |
           ((v & 0x0000FF0000000000ull) << 24) |
           ((v & 0x00FF000000000000ull) << 40) |
           ((v & 0xFF00000000000000ull) << 56);
}

template<typename T>
bool GXFileWriter::writePOD(const T &pod)
{
    return writeBytes(&pod, qsizetype(sizeof(T)));
}

}
