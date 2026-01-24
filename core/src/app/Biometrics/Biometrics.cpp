// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/Biometrics/Biometrics.h>

#include <QtGlobal>

#ifdef Q_OS_ANDROID
QVariant gx_app_biometrics_authenticate_android(const QString& reason, QObject* ctx);
bool gx_app_biometrics_available_android();
int gx_app_biometrics_status_android();

QVariant gx_app_biometrics_store_token_android(const QString& token, const QString& reason, QObject* ctx);
QVariant gx_app_biometrics_load_token_android(const QString& reason, QObject* ctx);
bool gx_app_biometrics_clear_token_android(QObject* ctx);
bool gx_app_biometrics_has_token_android(QObject* ctx);
#endif

#ifdef Q_OS_IOS
extern "C++" {
    QVariant gx_app_biometrics_authenticate_ios(const QString& reason, QObject* ctx);
    bool gx_app_biometrics_available_ios();
    int gx_app_biometrics_status_ios();

    QVariant gx_app_biometrics_store_token_ios(const QString& token, const QString& reason, QObject* ctx);
    QVariant gx_app_biometrics_load_token_ios(const QString& reason, QObject* ctx);
    bool gx_app_biometrics_clear_token_ios(QObject* ctx);
    bool gx_app_biometrics_has_token_ios(QObject* ctx);
}
#endif

/*!
    \class gx::app::biometrics::Biometrics
    \inmodule GenesisX
    \ingroup app-classes
    \title Mobile Biometrics
    \since Qt 6.10
    \brief Setting biometrics for mobile devices.

    \note Linked QML module: \c GenesisX\App\Biometrics
    \note Enabled when the app uses qmake flag \c genesisx_app_biometrics
 */

/*!
    \qmlmodule GenesisX.App.Biometrics
    \inqmlmodule io.genesisx.app
    \title Genesis-X Biometrics (QML)
    \brief QML APIs for biometrics.

    Import this module to use the \l Biometrics type:
    \code
    import GenesisX.App.Biometrics
    \endcode
 */

/*!
    \qmltype Biometrics
    \nativetype gx::app::biometrics::Biometrics
    \inqmlmodule io.genesisx.app
    \since Qt 6.10
    \brief QML APIs for biometrics.

    \section2 Example
    \qml
    import GenesisX.App.Biometrics

    Biometrics {
        onAuthenticated: (code, message) => {
            if (code === 0) console.log("OK biometrics there", code, message)
            else console.warn("Biometric error:", code, message)
        }
        Component.onCompleted: {
            if (bio.available) {
                bio.authenticate("Unlock app")
            }
        }
    }
    \endqml
 */

/*!
    \qmlsignal void Biometrics::availabilityChanged()
    \qmlsignal void Biometrics::authenticated(int code, string message)
 */

using namespace gx::app::biometrics;

static int mapCodeOrDefault(const QVariantMap& m, int defCode)
{
    const QVariant v = m.value(QStringLiteral("code"));
    return v.isValid() ? v.toInt() : defCode;
}

static QString mapMsgOrDefault(const QVariantMap& m, const QString& defMsg)
{
    const QVariant v = m.value(QStringLiteral("message"));
    return v.isValid() ? v.toString() : defMsg;
}

Biometrics::Biometrics(QObject *parent)
    : QObject{parent}
{
#ifdef Q_OS_ANDROID
    m_available = gx_app_biometrics_available_android();
#elif defined(Q_OS_IOS)
    m_available = gx_app_biometrics_available_ios();
#else
    m_available = false;
#endif

    emit availabilityChanged();
}

/*!
    \qmlproperty bool Biometrics::available

    True if biometrics is available
 */
bool Biometrics::available() const
{
    return m_available;
}

/*!
    \qmlmethod int Biometrics::status()
 */
int Biometrics::status() const
{
#ifdef Q_OS_ANDROID
    return gx_app_biometrics_status_android();
#elif defined(Q_OS_IOS)
    return gx_app_biometrics_status_ios();
#else
    return BiometricsResult::NotAvailable;
#endif
}

/*!
    \qmlmethod var Biometrics::authenticate(string reason)
    \a reason, why it isn't available
 */
QVariant Biometrics::authenticate(const QString &reason)
{
#ifdef Q_OS_ANDROID
    // emit authenticated(/*code*/1, /*message*/QStringLiteral("Biometrics not available"));
    return gx_app_biometrics_authenticate_android(reason, this);
#elif defined(Q_OS_IOS)
    return gx_app_biometrics_authenticate_ios(reason, this);
#else
    Q_UNUSED(reason);
    BiometricsResult r{BiometricsResult::NotAvailable, QStringLiteral("Biometrics not available on this platform.")};
    emit authenticated(r.code, r.message);
    return {};
#endif
}

bool Biometrics::hasLoginToken() const
{
#ifdef Q_OS_ANDROID
    return gx_app_biometrics_has_token_android(const_cast<Biometrics*>(this));
#elif defined(Q_OS_IOS)
    return gx_app_biometrics_has_token_ios(const_cast<Biometrics*>(this));
#else
    return false;
#endif
}

QVariant Biometrics::storeLoginToken(const QString &token, const QString &reason)
{qDebug() << " store token";
#ifdef Q_OS_ANDROID
    if (m_tokenOpInFlight) {
        return QVariantMap{
            { "code", BiometricsResult::TemporarilyUnavailable },
            { "message", "Token operation already running" }
        };
    }
    m_tokenOpInFlight = true;
    const QVariant res = gx_app_biometrics_store_token_android(token, reason, this);
    // const QVariantMap m = res.toMap();
    // emit loginTokenChanged();
    // emit authenticated(mapCodeOrDefault(m, BiometricsResult::Internal), mapMsgOrDefault(m, QStringLiteral("storeLoginToken finished")));

    return res;
#elif defined(Q_OS_IOS)
    if (m_tokenOpInFlight) {
        return QVariantMap{
            { "code", BiometricsResult::TemporarilyUnavailable },
            { "message", "Token operation already running" }
        };
    }
    m_tokenOpInFlight = true;
    return gx_app_biometrics_store_token_ios(token, reason, this);
#else
    Q_UNUSED(token);
    Q_UNUSED(reason);

    const QVariantMap m{
        {QStringLiteral("code"), BiometricsResult::NotAvailable},
        {QStringLiteral("message"), QStringLiteral("Secure token store not available on this platform")}
    };
    Q_UNUSED(mapCodeOrDefault(m, BiometricsResult::NotAvailable));
    Q_UNUSED(mapMsgOrDefault(m, ""));
    emit authenticated(BiometricsResult::NotAvailable, m.value(QStringLiteral("message")).toString());

    return m;
#endif
}

QVariant Biometrics::loadLoginToken(const QString &reason)
{
#ifdef Q_OS_ANDROID
    const QVariant res = gx_app_biometrics_load_token_android(reason, this);
    const QVariantMap m = res.toMap();
    const int code = mapCodeOrDefault(m, BiometricsResult::Internal);
    const QString msg = mapMsgOrDefault(m, QStringLiteral("loadLoginToken finished"));
    const QString token = m.value(QStringLiteral("token")).toString();

    emit authenticated(code, msg);
    emit loginTokenReady(code, msg, token);

    return res;
#elif defined(Q_OS_IOS)
    const QVariant res = gx_app_biometrics_load_token_ios(reason, this);
    const QVariantMap m = res.toMap();
    const int code = mapCodeOrDefault(m, BiometricsResult::Internal);
    const QString msg = mapMsgOrDefault(m, QStringLiteral("loadLoginToken finished"));
    const QString token = m.value(QStringLiteral("token")).toString();
    qDebug() << "loading token" << code << msg << token;
    emit authenticated(code, msg);
    emit loginTokenReady(code, msg, token);

    return res;
#else
    Q_UNUSED(reason);
    const QVariantMap m{
        {QStringLiteral("code"), BiometricsResult::NotAvailable},
        {QStringLiteral("message"), QStringLiteral("Secure token storage not available on this platform")},
        {QStringLiteral("token"), QString()}
    };
    emit authenticated(BiometricsResult::NotAvailable, m.value(QStringLiteral("message")).toString());
    emit loginTokenReady(BiometricsResult::NotAvailable, m.value(QStringLiteral("message")).toString(), QString());

    return m;
#endif
}

bool Biometrics::clearLoginToken()
{
#ifdef Q_OS_ANDROID
    const bool ok = gx_app_biometrics_clear_token_android(this);
    emit loginTokenChanged();

    return ok;
#elif defined(Q_OS_IOS)
    const bool ok = gx_app_biometrics_clear_token_ios(this);
    emit loginTokenChanged();

    return ok;
#else
    return false;
#endif
}
