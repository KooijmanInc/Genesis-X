// #include "include/GenesisX/Navigation/GxRouter.h"
#include <GenesisX/Navigation/GxRouter.h>

#include <QUrlQuery>

using namespace gx::navigation;

static GxRouter s_router;

static QString normalizePath(QString p)
{
    if (p.contains("://")) {
        QUrl u(p);
        if (u.isValid())
            p = u.path(); // keeps leading '/'
    } else if (p.startsWith("qrc:/")) {
        // qrc:/ has only one slash after the colon
        p = p.mid(4); // drop 'qrc:'
        if (!p.startsWith('/')) p.prepend('/');
    }

    if (!p.startsWith('/')) p.prepend('/');
    if (p.size() > 1 && p.endsWith('/')) p.chop(1);
    return p;
}

static QVariantMap toVariantMap(const QUrl& url)
{
    QVariantMap m;
    QUrlQuery q(url);
    for (auto it = q.queryItems(QUrl::FullyDecoded).cbegin(); it != q.queryItems().cend(); ++it)
        m[it->first] = it->second;

    return m;
}

GxRouter::GxRouter(QObject *parent)
    : QObject{parent}
{

}

void GxRouter::clearRoutes()
{
    m_routes.clear();
}

void GxRouter::registerRoute(const QString &name, const QString &path, const QUrl &comp)
{
    for (auto& r: m_routes) if (r.name == name) { r = makeSpec(name, path, comp); return; }
    m_routes.push_back(makeSpec(name, path, comp));
}

bool GxRouter::navigate(const QString &pathOrName, const QVariantMap &params)
{
    return doNavigate(pathOrName, params, /*replaceTop*/false);
}

bool GxRouter::replace(const QString &pathOrName, const QVariantMap &params)
{
    return doNavigate(pathOrName, params, /*replaceTop*/true);
}

bool GxRouter::back(int steps)
{
    if (steps < 1 || steps >= m_hist.size()) return false;
    while (steps--) m_hist.removeLast();
    emit currentChanged();

    return true;
}

void GxRouter::reset()
{
    m_hist.clear();
    emit currentChanged();
}

bool GxRouter::openDeepLink(const QUrl &url)
{
    QString p = url.path();
    QVariantMap qp = toVariantMap(url);

    return doNavigate(p, qp, false);
}

bool GxRouter::openDeepLinkString(const QString &s)
{
    return openDeepLink(QUrl(s));
}

QString GxRouter::currentPath() const
{
    return m_hist.isEmpty() ? QString() : m_hist.last().path;
}

QUrl GxRouter::currentComponent() const
{
    return m_hist.isEmpty() ? QUrl() : m_hist.last().componentUrl;
}

QVariantMap GxRouter::currentParams() const
{
    return m_hist.isEmpty() ? QVariantMap{} : m_hist.last().params;
}

QVariantList GxRouter::history() const
{
    QVariantList v;
    for (const auto& h: m_hist) {
        QVariantMap m;
        m["path"] = h.path;
        m["component"] = h.componentUrl;
        m["params"] = h.params;
        v << m;
    }

    return v;
}

bool GxRouter::doNavigate(const QString &pathLike, const QVariantMap &params, bool replaceTop)
{
    QString resolvedPath;
    QUrl comp;
    QVariantMap captured;

    if (!resolve(pathLike, resolvedPath, comp, captured)) return false;
    QVariantMap finalParams = mergeParams(captured, params);

    emit beforeNavigate(resolvedPath, finalParams);

    if (replaceTop && !m_hist.isEmpty()) m_hist.removeLast();
    m_hist.push_back({resolvedPath, comp, finalParams});
    emit currentChanged();

    return true;
}

bool GxRouter::resolve(const QString &pathLike, QString &outPath, QUrl &outComp, QVariantMap &outParams)
{
    QString path = pathLike.contains('/') ? normalizePath(pathLike) : QString();
    const GxRouteSpec* match = nullptr;
    QVariantMap captured;
    if (path.isEmpty()){
        // lookup by name first
        for (const auto& r : m_routes) if (r.name==pathLike){ match=&r; break;}
        if (!match) return false;
        outPath = match->path; outComp = match->componentUrl; // path may still be dynamic; params must fill it in later
        return true;
    }
    // concrete path: find first matching route by regex
    for (const auto& r : m_routes){
        auto m = r.regex.match(path);
        if (m.hasMatch()){
            match = &r;
            for (int i=0;i<r.paramNames.size();++i)
                captured[r.paramNames[i]] = m.captured(i+1);
            break;
        }
    }
    if (!match) return false;
    outPath = path;
    outComp = match->componentUrl;
    outParams = captured;

    return true;
}

GxRouteSpec GxRouter::makeSpec(QString name, QString path, QUrl comp)
{
    GxRouteSpec s;
    s.name = name;
    s.path = normalizePath(path);
    s.componentUrl = comp;
    s.regex = compilePattern(s.path, s.paramNames);

    return s;
}

QRegularExpression GxRouter::compilePattern(const QString &path, QStringList &paramNames)
{
    const QString norm = normalizePath(path);

    QString rx = "^";
    const QStringList parts = norm.split('/', Qt::SkipEmptyParts);
    for (const QString& part : parts) {
        rx += "/";
        if (part.startsWith(':')) {          // dynamic segment
            paramNames << part.mid(1);
            rx += "([^/]+)";
        } else if (part.startsWith('*')) {   // splat
            paramNames << part.mid(1);
            rx += "(.*)";
        } else {                             // literal
            rx += QRegularExpression::escape(part);
        }
    }
    rx += "$";

    QRegularExpression re(rx /*, QRegularExpression::NoPatternOption */);
    if (!re.isValid()) {
        qWarning() << "GxRouter regex error:" << re.errorString() << "pattern:" << rx;
    }
    return re;
}

QVariantMap GxRouter::mergeParams(const QVariantMap &a, const QVariantMap &b)
{
    QVariantMap r = a;
    for (auto it = b.begin(); it != b.end(); ++it) r[it.key()] = it.value(); return r;
}

GxRouter *gx::navigation::router()
{
    return &s_router;
}
