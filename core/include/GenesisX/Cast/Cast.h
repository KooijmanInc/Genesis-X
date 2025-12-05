#ifndef CAST_H
#define CAST_H

#include <QTimer>
#include <QDebug>
#include <QObject>
#include <QUdpSocket>
#include <QStringList>
#include <QHostAddress>
#include <QNetworkAccessManager>

#include <GenesisX/genesisx_global.h>

namespace gx::app::cast {

class GENESISX_CORE_EXPORT Cast : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool scanning READ scanning NOTIFY scanningChanged)
    Q_PROPERTY(bool devicesAvailable READ devicesAvailable NOTIFY devicesAvailableChanged)
    Q_PROPERTY(QStringList devices READ devices NOTIFY devicesChanged)
    Q_PROPERTY(QString selectedDevice READ selectedDevice NOTIFY selectedDeviceChanged)

public:
    explicit Cast(QObject *parent = nullptr);

    Q_INVOKABLE void startScan();
    Q_INVOKABLE void stopScan();

    Q_INVOKABLE bool selectDevice(const QString& id);
    Q_INVOKABLE void clearSelection();
    Q_INVOKABLE void castConnect(const QString& from = "");

    Q_INVOKABLE QString deviceDisplay(const QString& id) const;
    Q_INVOKABLE QString deviceIp(const QString& id) const;

    bool scanning() const { return _scanning; }
    bool devicesAvailable() const { return !_devices.isEmpty(); }
    QStringList devices() const { return _devices; }
    QString selectedDevice() const { return _selectedDevice; }

signals:
    void scanningChanged();
    void devicesAvailableChanged();
    void devicesChanged();
    void selectedDeviceChanged();

private slots:
    void onSsdpReadyRead();
    void onScanTimeout();
    void onHttpFinished(QNetworkReply* reply);

private:
    void sendSsdpQuery(const QByteArray& st, int mxSeconds = 3);
    void scheduleResends();
    void requestFriendlyName(const QString& deviceId, const QUrl& descUrl);
    void updateDeviceDisplay(const QString& deviceId, const QString& display);

    int _resendsLeft = 2;

    bool _scanning = false;
    QStringList _devices;
    QString _selectedDevice;

    QUdpSocket _ssdp;
    QTimer _scanTimeout;

    QSet<QString> _seenIds;

    QNetworkAccessManager _nam;
    struct Pending { QString id; QUrl url; };
    QHash<QNetworkReply*, Pending> _pending;
    QHash<QString, QString> _displayById;
    QHash<QString, QString> _ipById;
};

}

#endif // CAST_H
