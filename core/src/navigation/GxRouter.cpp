// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/Navigation/GxRouter.h>

#include <QUrlQuery>

/*!
    \class gx::navigation::GxRouter
    \inmodule io.genesisx.core
    \ingroup core-classes
    \title Navigation for QML routes
    \since Qt 6.10
    \brief Handles navigation for QML views.
 */

/*!
    \qmlmodule GenesisX.Navigation
    \inqmlmodule io.genesisx.core
    \title Genesis-X Navigation (QML)
    \since Qt 6.10
    \nativetype gx::navigation::GxRouter
    \brief Handles navigation for QML views.

    Import this module to use the \l Router type:

    \code
    import GenesisX.Navigation 1.0
    \endcode
 */

/*!
    \qmltype Router
    \inqmlmodule io.genesisx.core
    \since Qt 6.10
    \nativetype gx::navigation::GxRouter
    \brief Handles routing for QML views.

    \section2 Example
    \qml
    import GenesisX.Navigation 1.0

    Window {
        id: root

        Component.onCompleted: {
            Router.clearRoutes()
            Router.registerRoute("welcomeIntro", "/welcome-intro", "qrc:/views/WelcomeIntroView.qml")
        }
    }
    \endqml
 */

/*!
    \qmlsignal Router::currentChanged()

    Emitted whenever the current router is changed.
*/

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

/*!
    \qmlmethod void Router::clearRoutes()

    Clears all routes from memory
 */
void GxRouter::clearRoutes()
{
    m_routes.clear();
}

/*!
    \qmlmethod void Router::registerRoute(string name, string path, url comp)

    Registeres all the different routes by setting a \a name in combination with
    a \a path and a \a comp (componentUrl)

    \code
    Router.registerRoute("welcomeIntro", "/welcome-intro", "qrc:/views/WelcomeIntroView.qml")
    \endcode
 */
void GxRouter::registerRoute(const QString &name, const QString &path, const QUrl &comp)
{
    for (auto& r: m_routes) if (r.name == name) { r = makeSpec(name, path, comp); return; }
    m_routes.push_back(makeSpec(name, path, comp));
}

/*!
    \qmlmethod bool Router::navigate(string pathOrName, var params)

    Navigate to a specific component via \a pathOrName and optionally with \a params and returns a boolean

    \code
    Router.navigate("/auth")
    \endcode
 */
bool GxRouter::navigate(const QString &pathOrName, const QVariantMap &params)
{
    return doNavigate(pathOrName, params, /*replaceTop*/false);
}

/*!
    \qmlmethod bool Router::replace(string pathOrName, var params)

    Replace the specific component via \a pathOrName and optionally with \a params and returns a boolean

    \code
    Router.replace("/auth")
    \endcode
 */
bool GxRouter::replace(const QString &pathOrName, const QVariantMap &params)
{
    return doNavigate(pathOrName, params, /*replaceTop*/true);
}

/*!
    \qmlmethod bool Router::back(int step)

    Go back in history, \a step is standard 1 but can have any value you choose
    and returns a boolean
 */
bool GxRouter::back(int steps)
{
    if (steps < 1 || steps >= m_hist.size()) return false;
    while (steps--) m_hist.removeLast();
    emit currentChanged();

    return true;
}

/*!
    \qmlmethod void Router::reset()

    This will reset the current history
 */
void GxRouter::reset()
{
    m_hist.clear();
    emit currentChanged();
}

/*!
    \qmlmethod bool Router::openDeepLink(url url)

    Ability to cache \a url from e.g. notifications and go to component
    on opening the app and returns a boolean
 */
bool GxRouter::openDeepLink(const QUrl &url)
{
    QString p = url.path();
    QVariantMap qp = toVariantMap(url);

    return doNavigate(p, qp, false);
}

/*!
    \qmlmethod bool Router::openDeepLink(string s)

    Ability to cache \a s string from e.g. notifications and go to component
    on opening the app, same as openDeepLink but in stead of QUrl a string
    and returns a boolean
 */
bool GxRouter::openDeepLinkString(const QString &s)
{
    return openDeepLink(QUrl(s));
}

/*!
    \qmlproperty string Router::currentPath

    Holds the value of the current path
 */
QString GxRouter::currentPath() const
{
    return m_hist.isEmpty() ? QString() : m_hist.last().path;
}

/*!
    \qmlproperty url Router::currentComponent

    Holds the url of the current component
 */
QUrl GxRouter::currentComponent() const
{
    return m_hist.isEmpty() ? QUrl() : m_hist.last().componentUrl;
}

/*!
    \qmlproperty var Router::currentParams

    Holds the variables of the current parameters
 */
QVariantMap GxRouter::currentParams() const
{
    return m_hist.isEmpty() ? QVariantMap{} : m_hist.last().params;
}

/*!
    \qmlmethod variantlist Router::history()

    Returns the complete history as a VariantList
 */
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
