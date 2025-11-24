// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/Biometrics/Biometrics.h>

#include <QtGlobal>

#ifdef Q_OS_ANDROID
QVariant gx_app_biometrics_authenticate_android(const QString& reason, QObject* ctx);
bool gx_app_biometrics_available_android();
int gx_app_biometrics_status_android();
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

Biometrics::Biometrics(QObject *parent)
    : QObject{parent}
{
#ifdef Q_OS_ANDROID
    m_available = gx_app_biometrics_available_android();
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
    emit authenticated(/*code*/1, /*message*/QStringLiteral("Biometrics not available"));
    return gx_app_biometrics_authenticate_android(reason, this);
#else
    BiometricsResult r{BiometricsResult::NotAvailable, QStringLiteral("Biometrics not available on this platform.")};
    emit authenticated(2, reason);
    return {};
#endif
}
