// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/Cast/Cast.h>

#include <QRegularExpression>
#include <QCoreApplication>
#include <QXmlStreamReader>
#include <QNetworkDatagram>
#include <QNetworkReply>
#include <QHostAddress>
#include <QtQml/qqml.h>
#include <QByteArray>


#ifdef Q_OS_ANDROID
#include <QJniEnvironment>
#include <QJniObject>
#endif

/*!
    \class gx::app::cast::Cast
    \inmodule io.genesisx.app
    \ingroup app-classes
    \title Cast ability to connect to ChromeCast.
    \since Qt 6.10
    \brief Connect to ChromeCast and play your media.
 */

/*!
    \qmlmodule GenesisX.Cast
    \inqmlmodule io.genesisx.app
    \title Genesis-X Cast (QML)
    \since Qt 6.10
    \nativetype gx::app::cast::Cast
    \brief Handles the ability to cast.

    Import this module to use the \l Cast type:

    \code
    import GenesisX.Cast 1.0
    \endcode
 */

/*!
    \qmltype Cast
    \inqmlmodule io.genesisx.app
    \since Qt 6.10
    \nativetype gx::app::cast::Cast
    \brief Initialize and search for cast.

    \section2 Example
    \qml
    Item {
        MouseArea {
            anchors.fill: parent
            onClicked: {
                cast.startScan()
            }
        }
    }
    \endqml
 */

/*!
    \qmlsignal Cast::scanningChanged()

    Emitted when scanning is changed.
 */

/*!
    \qmlsignal Cast::devicesAvailableChanged()

    Emitted when scanned devices is changed.
 */

/*!
    \qmlsignal Cast::devicesChanged()

    Emitted when devices are changed.
 */

/*!
    \qmlsignal Cast::selectedDeviceChanged()

    Emitted when selected device is changed.
 */

using namespace gx::app::cast;

static QString headerValue(const QByteArray& headers, const QByteArray& key)
{
    QList<QByteArray> lines = headers.split('\n');
    for (QByteArray line : lines) {
        line = line.trimmed();
        if (line.isEmpty() || line.startsWith("HTTP/")) continue;
        int colon = line.indexOf(':');
        if (colon <= 0) continue;
        QByteArray k = line.left(colon).trimmed();
        if (k.compare(key, Qt::CaseInsensitive) == 0) {
            return QString::fromUtf8(line.mid(colon + 1).trimmed());
        }
    }
    return {};
}

Cast::Cast(QObject *parent)
    : QObject{parent}
{
#ifdef Q_OS_ANDROID
    connect(&_ssdp, &QUdpSocket::readyRead, this, &Cast::onSsdpReadyRead);

    _scanTimeout.setSingleShot(true);
    connect(&_scanTimeout, &QTimer::timeout, this, &Cast::onScanTimeout);

    connect(&_nam, &QNetworkAccessManager::finished, this, &Cast::onHttpFinished);
#endif
}

void Cast::startScan()
{
    if (_scanning) { qInfo() << "[Cast] startScan(): already scanning"; return; }
    _scanning = true;
    emit scanningChanged();

    _devices.clear();
    emit devicesChanged();
    emit devicesAvailableChanged();
    _seenIds.clear();

    if (!_ssdp.bind(QHostAddress::AnyIPv4, 0, QUdpSocket::ShareAddress|QUdpSocket::ReuseAddressHint)) {
        qWarning() << "[Cast] SSDP bind failed:" << _ssdp.errorString();
    }

    sendSsdpQuery("urn:dial-multiscreen-org:service:dial:1", 3);
    sendSsdpQuery("ssdp:all", 3);
    sendSsdpQuery("urn:schemas-upnp-org:device:MediaRenderer:1", 3);

    _resendsLeft = 3;
    scheduleResends();

    _scanTimeout.start(7000);

    qInfo() << "[Cast] SSDP scan started";
}

void Cast::stopScan()
{
    if (!_scanning) return;
    _scanning = false;
    emit scanningChanged();
    _scanTimeout.stop();
    _ssdp.close();
    qInfo() << "[Cast] SSDP scan stopped";
}

bool Cast::selectDevice(const QString &id)
{
    if (!_displayById.contains(id)) {
        qWarning().noquote() << "[Cast] selectDevice: unknown id" << id;
        return false;
    }
    if (_selectedDevice == id) return true;
    _selectedDevice = id;
    emit selectedDeviceChanged();
    qInfo().noquote() << "[Cast] selected id=" << id
                      << "name=" << _displayById.value(id)
                      << "ip=" << _ipById.value(id);

    return true;
}

void Cast::clearSelection()
{
    if (_selectedDevice.isEmpty()) return;
    _selectedDevice.clear();
    emit selectedDeviceChanged();
    qInfo() << "[Cast] selection cleared";
}

void Cast::castConnect(const QString& from)
{
    if (from.isEmpty())
        qInfo().noquote() << "[Cast] castConnect()";
    else
        qInfo().noquote() << "[Cast] castConnect() from:" << from;

#ifdef Q_OS_ANDROID
    QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=] {
        QJniObject ctx = QNativeInterface::QAndroidApplication::context();

        QJniObject::callStaticMethod<void>(
            "com/genesisx/cast/GXCastManager",
            "init",
            "(Landroid/content/Context;)V",
            ctx.object<jobject>());

        QJniObject::callStaticMethod<void>(
            "com/genesisx/cast/GXCastManager",
            "showChooser",
            "(Landroid/content/Context;)V",
            ctx.object<jobject>());
    });
#else
    if (_selectedDevice.isEmpty()) {
        qWarning() << "[Cast] castConnect(): no device selected";
        return;
    }
    const QString name = _displayById.value(_selectedDevice);
    const QString ip = _ipById.value(_selectedDevice);
    qInfo().noquote() << "[Cast] (stub) connect to" << name << "ip:" << ip
                      << "id:" << _selectedDevice;
#endif
}

QString Cast::deviceDisplay(const QString &id) const
{
    return _displayById.value(id);
}

QString Cast::deviceIp(const QString &id) const
{
    return _ipById.value(id);
}

void Cast::sendSsdpQuery(const QByteArray& st, int mxSeconds)
{
    static const QHostAddress multicast("239.255.255.250");
    static const quint16 port = 1900;

    QByteArray req;
    req = "M-SEARCH * HTTP/1.1\r\n";
    req += "HOST: 239.255.255.250:1900\r\n";
    req += "MAN: \"ssdp:discover\"\r\n";
    req += QByteArray("MX: ") + QByteArray::number(mxSeconds) + "\r\n";
    req += "ST: " + st + "\r\n";
    req += "USER_AGENT: Qt/6 UPnP/1.1 SSDP-Discover\r\n";
    req += "\r\n";

    auto sent = _ssdp.writeDatagram(req, multicast, port);
    if (sent < 0) {
        qWarning() << "[Cast] SSDP send failed:" << _ssdp.errorString();
    } else {
        qInfo() << "[Cast] SSDP query sent for ST =" << st;
    }
}

void Cast::onSsdpReadyRead()
{
    while (_ssdp.hasPendingDatagrams()) {
        QNetworkDatagram d = _ssdp.receiveDatagram();
        const QByteArray data = d.data();

        // Basic sanity: must look like an SSDP response
        if (!data.startsWith("HTTP/1.1 200")) continue;

        const QString st = headerValue(data, "ST");
        const QString usn = headerValue(data, "USN");
        const QString loc = headerValue(data, "LOCATION");
        const QString server = headerValue(data, "SERVER");
        const QString appUrl = headerValue(data, "APPLICATION-URL"); // Chromecast often includes this

        // Filter for our targets quickly
        if (!(st.contains("dial", Qt::CaseInsensitive) ||
              st.contains("MediaRenderer", Qt::CaseInsensitive)))
            continue;

        QString deviceId = !usn.isEmpty() ? usn : (!loc.isEmpty() ? loc : d.senderAddress().toString());
        if (deviceId.startsWith("uuid:")) {
            const int sep = deviceId.indexOf("::");
            if (sep > 0) deviceId = deviceId.left(sep);
        }
        if (_seenIds.contains(deviceId)) continue;
        _seenIds.insert(deviceId);

        const QString ip = d.senderAddress().toString();
        _ipById.insert(deviceId, ip);

        QString display = "Chromecast @ " + d.senderAddress().toString();

        if (!appUrl.isEmpty()) {
            QUrl url(appUrl);
            if (url.isValid() && !url.host().isEmpty())
                display = "Chromecast @ " + url.host();
        }

        _displayById[deviceId] = display;
        updateDeviceDisplay(deviceId, display);

        QUrl descUrl;
        if (!loc.isEmpty()) {
            descUrl = QUrl(loc);
        } else if (!appUrl.isEmpty()) {
            QUrl base(appUrl);
            descUrl = base.resolved(QUrl("/ssdp/device-desc.xml"));
        }
        if (descUrl.isValid())
            requestFriendlyName(deviceId, descUrl);

        qInfo().noquote() << "[Cast] SSDP hit:"
                          << "ST=" << st << "USN=" << usn
                          << "LOCATION=" << loc << "SERVER=" << server
                          << "FROM=" << d.senderAddress().toString();
    }
}

void Cast::onScanTimeout()
{
    if (!_scanning) return;
    stopScan();
    qInfo() << "[Cast] SSDP scan finished. devices=" << _devices;
}

void Cast::onHttpFinished(QNetworkReply* reply)
{
    if (!reply) return;
    // auto it = qobject_cast<QNetworkReply*>(sender());
    // if (!reply) return;

    auto it = _pending.find(reply);
    if (it == _pending.end()) { reply->deleteLater(); return; }

    const QString deviceId = it->id;
    const QUrl url = it->url;
    _pending.erase(it);

    if (reply->error() == QNetworkReply::NoError) {
        const QByteArray xmlBytes = reply->readAll();

        // Quick-and-safe parse with QXmlStreamReader
        QXmlStreamReader xml(xmlBytes);
        QString friendly;
        while (!xml.atEnd()) {
            xml.readNext();
            if (xml.isStartElement() && xml.name() == QLatin1String("friendlyName")) {
                friendly = xml.readElementText();
                break;
            }
        }
        if (!xml.hasError() && !friendly.isEmpty()) {
            updateDeviceDisplay(deviceId, friendly);
            qInfo().noquote() << "[Cast] friendlyName(" << url.toString() << ") =" << friendly;
        } else {
            qInfo().noquote() << "[Cast] no friendlyName in" << url.toString();
        }
    } else {
        qWarning().noquote() << "[Cast] HTTP error for" << url.toString()
        << ":" << reply->errorString();
    }

    reply->deleteLater();
}

void Cast::scheduleResends()
{
    if (_resendsLeft <= 0) return;
    QTimer::singleShot(1500, this, [this]{
        sendSsdpQuery("urn:dial-multiscreen-org:service:dial:1", 3);
        sendSsdpQuery("ssdp:all", 3);
        sendSsdpQuery("urn:schemas-upnp-org:device:MediaRenderer:1", 3);
        _resendsLeft--;
        scheduleResends();
    });
}

void Cast::requestFriendlyName(const QString &deviceId, const QUrl &descUrl)
{
    qInfo().noquote() << "[Cast] fetching device-desc for" << deviceId << "->" << descUrl.toString();
    QNetworkRequest req(descUrl);
    req.setHeader(QNetworkRequest::UserAgentHeader, "GenesisX/1.0 (Qt)");
#if QT_VERSION >= QT_VERSION_CHECK(6,7,0)
    req.setTransferTimeout(3000);
#endif
    auto* reply = _nam.get(req);
    _pending.insert(reply, Pending{deviceId, descUrl});
}

void Cast::updateDeviceDisplay(const QString &deviceId, const QString &display)
{
    bool changed = false;

    QString finalDisplay = display;
    const bool looksProvisional = display.startsWith("Chromecast @");
    if (!looksProvisional) {
        const QString ip = _ipById.value(deviceId);
        if (!ip.isEmpty() && !display.contains(" @ "))
            finalDisplay = display + " @ " + ip;
    }

    auto old = _displayById.value(deviceId);
    if (old != finalDisplay) {
        _displayById[deviceId] = finalDisplay;
        changed = true;
    }

    QStringList newList = _displayById.values();
    std::sort(newList.begin(), newList.end(), [](const QString& a, const QString& b){
        return a.localeAwareCompare(b) < 0;
    });

    if (newList != _devices) {
        _devices = newList;
        emit devicesChanged();
        emit devicesAvailableChanged();
        changed = true;
    }

    if (changed) {
        qInfo() << "[Cast] devices =" << _devices;
    }
}
