#ifndef GXROUTER_H
#define GXROUTER_H

#include <QRegularExpression>
#include <QObject>
#include <QUrl>

#include <GenesisX/genesisx_global.h>

namespace gx::navigation {

struct GENESISX_CORE_EXPORT GxRouteSpec {
    QString name;
    QString path;
    QUrl componentUrl;
    QRegularExpression regex;
    QStringList paramNames;
};

struct GENESISX_CORE_EXPORT GxHistoryEntry {
    QString path;
    QUrl componentUrl;
    QVariantMap params;
};

class GENESISX_CORE_EXPORT GxRouter: public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString currentPath READ currentPath NOTIFY currentChanged)
    Q_PROPERTY(QUrl currentComponent READ currentComponent NOTIFY currentChanged)
    Q_PROPERTY(QVariantMap currentParams READ currentParams NOTIFY currentChanged)

public:
    explicit GxRouter(QObject* parent = nullptr);

    Q_INVOKABLE void clearRoutes();
    Q_INVOKABLE void registerRoute(const QString& name, const QString& path, const QUrl& componentUrl);

    Q_INVOKABLE bool navigate(const QString& pathOrName, const QVariantMap& params = {});
    Q_INVOKABLE bool replace(const QString& pathOrName, const QVariantMap& params = {});
    Q_INVOKABLE bool back(int steps = 1);
    Q_INVOKABLE void reset();

    Q_INVOKABLE bool openDeepLink(const QUrl& url);
    Q_INVOKABLE bool openDeepLinkString(const QString&);

    QString currentPath() const;
    QUrl currentComponent() const;
    QVariantMap currentParams() const;
    Q_INVOKABLE QVariantList history() const;

signals:
    void beforeNavigate(QString toPath, QVariantMap params);
    void currentChanged();

private:
    bool doNavigate(const QString& pathLike, const QVariantMap& params, bool replaceTop);
    bool resolve(const QString& pathLike, QString& outPath, QUrl& outComp, QVariantMap& outParams);
    static GxRouteSpec makeSpec(QString name, QString path, QUrl comp);
    static QRegularExpression compilePattern(const QString& path, QStringList& paramNames);
    static QVariantMap mergeParams(const QVariantMap& a, const QVariantMap& b);

    QVector<GxRouteSpec> m_routes;
    QVector<GxHistoryEntry> m_hist;
};

GENESISX_CORE_EXPORT GxRouter* router();

}

#endif // GXROUTER_H
