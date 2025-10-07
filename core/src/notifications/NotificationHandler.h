#ifndef NOTIFICATION_HANDLER_H
#define NOTIFICATION_HANDLER_H

#include <QObject>
#include <QVariantMap>

class QSystemTrayIcon;

namespace gx {

class NotificationHandler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool initialized READ initialized NOTIFY initializedChanged)

public:
    explicit NotificationHandler(QObject *parent = nullptr) : QObject(parent) {}

    Q_INVOKABLE void show(const QString& title, const QString& body, int msec = 5000);

    Q_INVOKABLE void initialize(const QVariantMap& options = {});
    Q_INVOKABLE void subscribe(const QString& topic);
    Q_INVOKABLE void unsubscribe(const QString& topic);
    Q_INVOKABLE QString fcmToken() const;

    bool initialized() const { return m_initialized; }

signals:
    void initializedChanged();
    void notificationReceived(const QString& title, const QString& body, const QVariantMap& data);
    void tokenChanged(const QString& token);

private:
    bool m_initialized = false;

#ifdef Q_OS_WIN
    QSystemTrayIcon* m_tray = nullptr;
    void ensureTray();
    bool eventFilter(QObject*, QEvent* e);
#endif
};

}

#endif // NOTIFICATION_HANDLER_H
