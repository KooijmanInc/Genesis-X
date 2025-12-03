<!-- SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only) -->
<!-- Copyright (c) 2025 Kooijman Incorporate Holding B.V. -->

# Genesis-X

[![CI – Quality checks](https://github.com/KooijmanInc/Genesis-X/actions/workflows/quality.yml/badge.svg?branch=main)](https://github.com/KooijmanInc/Genesis-X/actions/workflows/quality.yml)
[![CI – Quality checks (staging)](https://github.com/KooijmanInc/Genesis-X/actions/workflows/quality.yml/badge.svg?branch=staging)](https://github.com/KooijmanInc/Genesis-X/actions/workflows/quality.yml)


Qt library with a physics-engine foundation and cross-platform notifications.  
Tested with **Qt 6.10** (Qt Creator **17.0.2**).

---

## 💖 Donations & Sponsorships

[![Sponsor](https://img.shields.io/badge/Sponsor-GitHub%20Sponsors-ff69b4)](https://github.com/sponsors/KooijmanInc)
[![Stripe](https://img.shields.io/badge/Donate-Stripe-635bff)](https://buy.stripe.com/3cIaEXeEt1n66BybcjaAw00)
[![Stripe Monthly](https://img.shields.io/badge/Sponsor-Stripe%20Monthly-635bff)](https://buy.stripe.com/6oUfZh67X1n67FC4NVaAw01)
[![PayPal](https://img.shields.io/badge/Donate-PayPal-blue)](https://paypal.me/kooijmaninc)

If Genesis‑X saved you time, consider supporting development:  
- GitHub Sponsors: https://github.com/sponsors/KooijmanInc  
- Stripe (one‑time): https://buy.stripe.com/3cIaEXeEt1n66BybcjaAw00  
- Stripe (monthly): https://buy.stripe.com/6oUfZh67X1n67FC4NVaAw01  
- Company sponsorships (invoice/VAT): sponsors@kooijman-inc.com

See also **[BACKERS.md](BACKERS.md)** and **.github/FUNDING.yml**.

---

## New in this release
- **Biometrics**: C++ `BiometricsAuth` and QML `Biometrics` with `authenticate()`, `isAvailable`.
- **Permissions**: C++ `PermissionManager` and QML `Permissions` to check/request runtime permissions.
- **CommandController**: C++ Connecting to an api server. (we created a template api package in symfony for you to download at [https://github.com/KooijmanInc/gx-template-api](https://github.com/KooijmanInc/gx-template-api))
- **qdocs**: Written qdocs for easy access via F1 and help (usally it's autodetected, otherwise add the docs manually)

### QML quick start
```qml
import GenesisX.App.Biometrics 1.0
import GenesisX.App.Permissions 1.0

Biometrics {
    id: bio
    onResult: (ok, error) => console.log("Auth:", ok, error)
}

Permissions {
    id: perms
}

Button {
    text: "Unlock"
    onClicked: bio.authenticate("Confirm your identity")
}
Button {
    text: "Allow Notifications"
    onClicked: perms.request(["notifications"])
}
```

```qmake
QT += genesisx_orm genesisx_app_notifications genesisx_app_biometrics genesisx_app_permissions

GX_LOADED_MODULES = $$QT
GX_LOADED_MODULES_CSV = $$join(GX_LOADED_MODULES, ",")
DEFINES += GX_LOADED_MODULES=\\\"$$GX_LOADED_MODULES_CSV\\\"

# ---------- Genesis-X pass app root path ----------
GENESISX_APP_ROOT = $$PWD

# ---------- Build type sets library path ----------
debug {
    LOCAL_BUILD_PATH = debug
} else: profile {
    LOCAL_BUILD_PATH = profile
} else {
    LOCAL_BUILD_PATH = release
    DEFINES += QT_NO_DEBUG_OUTPUT    # disables qDebug()
    #DEFINES += QT_NO_INFO_OUTPUT    # uncomment to also disable qInfo()
    #DEFINES += QT_NO_WARNING_OUTPUT # (usually keep warnings on)
}

# ---------- Set same build path optional ----------
DESTDIR = $$PWD/../$$LOCAL_DESTINATION_PATH/$$LOCAL_BUILD_PATH
DEST_LIBDIRS = $$DESTDIR
```

### C++ quick start
```cpp
#include <GenesisX/CoreQml.h>
#include <GenesisX/Orm/OrmQml.h>
#include <GenesisX/Orm/ConfigIO.h>
#include <GenesisX/Orm/Controllers.h>
#include <GenesisX/utils/SystemInfo.h>
#include <GenesisX/Orm/TransportConfig.h>

int main (int argc, char* argv[])
{
  GXOrm::TransportConfig cfg;
  GXOrm::loadTransportConfig(":/config/config.json", cfg, u"dev");
  
  auto* conn = GXOrm::gxOrmConnectionController();
  conn->applyTransport(cfg);
  
  auto* cmd = GXOrm::gxOrmCommandController();
  cmd->cmdLogin("user@example.localhost", "your-password");
  cmd->cmdPing();
  
  QQmlApplicationEngine engine;
  QQmlContext *rootContext = engine.rootContext();
  GXCore::registerEnabledQmlModules(&engine, GX_LOADED_MODULES);
  GXOrm::registerEnabledQmlModules(&engine);
}
```

```json
{
    "backend": "http",
    "api": {
        "baseUrl": "https://api.example.com",
        "appVersion": "1.0.0",
        "userLanguage": "en",
        "timeoutMs": 8000,
        "retryCount": 0,
        "allowInsecureDev": false,
        "appKey": "app-key",
        "apiToken": "2Y&1N+yBmJxgDp%D-you-token_%UJNHOUIuc0+!yFWW%OmJ",
        "bearerToken": "",
        "headers": {
            "Accept": "application/json"
        }
    },
    "sql": {
        "driver": "",
        "host": "",
        "port": 3306,
        "database": "",
        "user": "",
        "password": "",
        "options": {}
    },
    "overrides": {
        "dev": {
            "api": {
                "baseUrl": "https://api.example.localhost",
                "allowInsecureDev": true
            },
            "sql": {
                "host": "127.0.0.1",
                "port": 3306,
                "user": "dev",
                "password": "secret"
            }
        },
        "staging": {
            "api": {
                "baseUrl": "https://staging.api.example.com",
                "allowInsecureDev": false
            },
            "sql": {
                "host": "https://www.example.com",
                "port": 3306,
                "user": "staging",
                "password": "secret"
            }
        }
    }
}

```

---

## Features

- QML‑friendly Core API
- Notifications:
  - ✅ Android (Firebase C++ SDK)
  - ✅ Windows
  - ✅ Linux, iOS/iPadOS, macOS
- Permissions:
  - ✅ Android
- Biometrics:
  - ✅ Android
- Android Firebase integration auto‑wired via generated `gradle.properties`
- Scripted 3rd‑party setup (kept out of Git)

---

## Requirements

- Qt 6.10 toolchains (Android kit if targeting Android)
- Android SDK + NDK (for Android builds)
- Git, PowerShell (Windows) or bash (macOS/Linux)

---

## Repo Layout (excerpt)

```
Genesis-X /
├─ common/
│  ├─ qmake-destination-path.pri
│  └─ qmake-target-platform.pri
├─ config/
│  └─ deps.json
├─ core/
│  ├─ android-template/
│  │  ├─ build.gradle
│  │  ├─ google-services.json
│  │  ├─ gradle.properties
│  │  └─ gradle.properties.in
│  ├─ include/
│  │  └─ GenesisX/
│  │     ├─ Auth/
│  │     │  └─ Auth.h
│  │     ├─ Biometrics/
│  │     │  └─ Biometrics.h
│  │     ├─ Permissions/
│  │     │  └─ Permissions.h
│  │     ├─ utils/
│  │     │  └─ SystemInfo.h
│  │     ├─ CoreQml.h
│  │     └─ genesisx_global.h
│  ├─ qml/
│  │  ├─ GenesisX/
│  │  │  ├─ App/
│  │  │  │  ├─ Biometrics/
│  │  │  │  │  ├─ biometrics.qmltypes
│  │  │  │  │  └─ qmldir
│  │  │  │  └─ Permissions/
│  │  │  │     ├─ permissions.qmltypes
│  │  │  │     └─ qmldir
│  │  │  └─ Core/
│  │  │     ├─ Navigation/
│  │  │     │  ├─ Link.qml
│  │  │     │  ├─ NavHost.qml
│  │  │     │  ├─ navigation.qmltypes
│  │  │     │  └─ qmldir
│  │  │     ├─ Notifications/
│  │  │     │  ├─ notifications.qmltypes
│  │  │     │  └─ qmldir
│  │  │     └─ SystemInfo/
│  │  │        ├─ qmldir
│  │  │        └─ systeminfo.qmltypes
│  │  └─ core_modules.qrc
│  ├─ resources/
│  │  ├─ core.qrc
│  │  └─ logo.ico
│  ├─ src/
│  │  ├─ app/
│  │  │  ├─ ab/
│  │  │  │  ├─ ABTesting.cpp
│  │  │  │  └─ ABTesting.h
│  │  │  ├─ analytics/
│  │  │  │  ├─ analytics.cpp
│  │  │  │  └─ analytics.h
│  │  │  ├─ AudioRecorder/
│  │  │  │  ├─ AudioRecorder.cpp
│  │  │  │  └─ AudioRecorder.h
│  │  │  ├─ Auth/
│  │  │  │  └─ Auth.cpp
│  │  │  ├─ Biometrics/
│  │  │  │  ├─ android/
│  │  │  │  │  └─ src/
│  │  │  │  │     └─ main/
│  │  │  │  │        └─ java/
│  │  │  │  │           ├─ biometrics/
│  │  │  │  │           │  └─ GxBiometrics.java
│  │  │  │  │           └─ org/
│  │  │  │  │              └─ qtproject/
│  │  │  │  │                 └─ qt/
│  │  │  │  │                    └─ android/
│  │  │  │  │                       └─ QtActivityUtils.java
│  │  │  │  ├─ Biometrics.cpp
│  │  │  │  ├─ BiometricsAndroid.cpp
│  │  │  │  ├─ BiometricsQml.cpp
│  │  │  │  └─ BiometricsQml.h
│  │  │  ├─ notifications/
│  │  │  │  ├─ fcm_android.cpp
│  │  │  │  ├─ fcm_android.h
│  │  │  │  ├─ GXAppDelegate+Push_ios.mm
│  │  │  │  ├─ GXAppDelegate+Push_ios_old.mm
│  │  │  │  ├─ GXAppDelegate+Push_macos.mm
│  │  │  │  ├─ GXAppDelegate+Push_macos_old.mm
│  │  │  │  ├─ GXPush_macos.mm
│  │  │  │  ├─ GXPushBridge.mm
│  │  │  │  ├─ NotificationHandler.cpp
│  │  │  │  ├─ NotificationHandler.h
│  │  │  │  ├─ NotificationHandler_apple.mm
│  │  │  │  ├─ NotificationHandler_apple_bridge.h
│  │  │  │  ├─ NotificationHandler_apple_old.mm
│  │  │  │  ├─ NotificationsQml.cpp
│  │  │  │  └─ NotificationsQml.h
│  │  │  └─ Permissions/
│  │  │     ├─ android/
│  │  │     │  └─ src/
│  │  │     │     └─ main/
│  │  │     │        └─ java/
│  │  │     │           └─ permissions/
│  │  │     │              └─ GxPermissions.java
│  │  │     ├─ Permissions.cpp
│  │  │     ├─ PermissionsAndroid.cpp
│  │  │     ├─ PermissionsQml.cpp
│  │  │     └─ PermissionsQml.h
│  │  ├─ core/
│  │  │  └─ CoreQml.cpp
│  │  ├─ navigation/
│  │  │  ├─ GxRouter.cpp
│  │  │  ├─ GxRouter.h
│  │  │  ├─ NavigationQml.cpp
│  │  │  └─ NavigationQml.h
│  │  └─ utils/
│  │     ├─ SystemInfo.cpp
│  │     ├─ SystemInfoQml.cpp
│  │     └─ SystemInfoQml.h
│  └─ core.pro
├─ docs/
│  ├─ images/
│  │  └─ arrow_bc.png
│  ├─ out/
│  │  ├─ classes.html
│  │  ├─ codegen-h.html
│  │  ├─ coreqml-h.html
│  │  ├─ genesisx-app-biometrics-qmlmodule.html
│  │  ├─ genesisx-app-permissions-qmlmodule.html
│  │  ├─ genesisx-core.html
│  │  ├─ genesisx-guides-module.html
│  │  ├─ genesisx-notifications-qmlmodule.html
│  │  ├─ genesisx-orm-module.html
│  │  ├─ genesisx-orm.html
│  │  ├─ genesisx.index
│  │  ├─ GenesisX.qch
│  │  ├─ GenesisX.qhp
│  │  ├─ getting-started.html
│  │  ├─ gx-app-ab-abtesting.html
│  │  ├─ gx-app-ab.html
│  │  ├─ gx-app-analytics-analytics.html
│  │  ├─ gx-app-analytics.html
│  │  ├─ gx-app-audiorecorder-audiorecorder.html
│  │  ├─ gx-app-audiorecorder.html
│  │  ├─ gx-app-auth-auth.html
│  │  ├─ gx-app-auth.html
│  │  ├─ gx-app-biometrics-biometrics.html
│  │  ├─ gx-app-biometrics.html
│  │  ├─ gx-app-notifications-classes.html
│  │  ├─ gx-app-notifications-notificationhandler.html
│  │  ├─ gx-app-notifications.html
│  │  ├─ gx-app-permissions-permissions.html
│  │  ├─ gx-app-permissions.html
│  │  ├─ gx-app.html
│  │  ├─ gx-orm-codegen.html
│  │  ├─ gx-orm-commandcontroller.html
│  │  ├─ gx-orm-connectioncontroller.html
│  │  ├─ gx-orm.html
│  │  ├─ gx.html
│  │  ├─ index.html
│  │  ├─ namespaces-orm.html
│  │  ├─ namespaces.html
│  │  ├─ qml-genesisx-app-biometrics-biometrics-members.html
│  │  ├─ qml-genesisx-app-biometrics-biometrics.html
│  │  ├─ qml-genesisx-app-permissions-permissions-members.html
│  │  ├─ qml-genesisx-app-permissions-permissions.html
│  │  ├─ qml-genesisx-notifications-notificationhandler-members.html
│  │  ├─ qml-genesisx-notifications-notificationhandler.html
│  │  └─ systeminfo-h.html
│  ├─ style/
│  │  └─ genesisx-dark.css
│  ├─ topics/
│  │  ├─ classes.qdoc
│  │  ├─ genesisx-core.qdoc
│  │  ├─ getting-started.qdoc
│  │  ├─ group_notifications.qdoc
│  │  ├─ guides.qdoc
│  │  ├─ gx-namespace.qdoc
│  │  ├─ gx-orm.qdoc
│  │  ├─ index.qdoc
│  │  └─ namespaces.qdoc
│  ├─ .gitignore
│  ├─ .qmake.stash
│  ├─ docs.pro
│  ├─ genesisx.qdocconf
│  ├─ Makefile
│  ├─ Makefile.Debug
│  ├─ Makefile.Release
│  └─ qt-includes.qdocconf.in
├─ LICENSES/
│  ├─ GPL-3.0-only.txt
│  └─ LicenseRef-KooijmanInc-Commercial.txt
├─ mkspecs/
│  ├─ features/
│  │  ├─ conf/
│  │  │  ├─ gx_core.prf
│  │  │  ├─ gx_core_sub.prf
│  │  │  ├─ gx_orm.prf
│  │  │  └─ gx_physics.prf
│  │  ├─ genesisx_app_core.prf
│  │  ├─ gx_app_ab.prf
│  │  ├─ gx_app_analytics.prf
│  │  ├─ gx_app_audiorecorder.prf
│  │  ├─ gx_app_auth.prf
│  │  ├─ gx_app_background.prf
│  │  ├─ gx_app_billing.prf
│  │  ├─ gx_app_biometrics.prf
│  │  ├─ gx_app_calendar.prf
│  │  ├─ gx_app_camera.prf
│  │  ├─ gx_app_cast.prf
│  │  ├─ gx_app_clipboard.prf
│  │  ├─ gx_app_config.prf
│  │  ├─ gx_app_connectivity.prf
│  │  ├─ gx_app_contacts.prf
│  │  ├─ gx_app_crash.prf
│  │  ├─ gx_app_deeplinks.prf
│  │  ├─ gx_app_files.prf
│  │  ├─ gx_app_haptics.prf
│  │  ├─ gx_app_imagepicker.prf
│  │  ├─ gx_app_intents.prf
│  │  ├─ gx_app_location.prf
│  │  ├─ gx_app_logging.prf
│  │  ├─ gx_app_media.prf
│  │  ├─ gx_app_notifications.prf
│  │  ├─ gx_app_permissions.prf
│  │  ├─ gx_app_remoteconfig.prf
│  │  ├─ gx_app_review.prf
│  │  ├─ gx_app_root.prf
│  │  ├─ gx_app_securestore.prf
│  │  ├─ gx_app_sensors.prf
│  │  ├─ gx_app_share.prf
│  │  ├─ gx_app_updater.prf
│  │  └─ gx_runtime.prf
│  └─ modules/
│     ├─ qt_lib_genesisx.pri
│     ├─ qt_lib_genesisx_app_ab.pri
│     ├─ qt_lib_genesisx_app_analytics.pri
│     ├─ qt_lib_genesisx_app_audiorecorder.pri
│     ├─ qt_lib_genesisx_app_auth.pri
│     ├─ qt_lib_genesisx_app_background.pri
│     ├─ qt_lib_genesisx_app_billing.pri
│     ├─ qt_lib_genesisx_app_biometrics.pri
│     ├─ qt_lib_genesisx_app_calendar.pri
│     ├─ qt_lib_genesisx_app_camera.pri
│     ├─ qt_lib_genesisx_app_cast.pri
│     ├─ qt_lib_genesisx_app_clipboard.pri
│     ├─ qt_lib_genesisx_app_config.pri
│     ├─ qt_lib_genesisx_app_connectivity.pri
│     ├─ qt_lib_genesisx_app_contacts.pri
│     ├─ qt_lib_genesisx_app_core.pri
│     ├─ qt_lib_genesisx_app_crash.pri
│     ├─ qt_lib_genesisx_app_deeplinks.pri
│     ├─ qt_lib_genesisx_app_files.pri
│     ├─ qt_lib_genesisx_app_haptics.pri
│     ├─ qt_lib_genesisx_app_imagepicker.pri
│     ├─ qt_lib_genesisx_app_intents.pri
│     ├─ qt_lib_genesisx_app_location.pri
│     ├─ qt_lib_genesisx_app_logging.pri
│     ├─ qt_lib_genesisx_app_media.pri
│     ├─ qt_lib_genesisx_app_notifications.pri
│     ├─ qt_lib_genesisx_app_permissions.pri
│     ├─ qt_lib_genesisx_app_preferences.pri
│     ├─ qt_lib_genesisx_app_remoteconfig.pri
│     ├─ qt_lib_genesisx_app_review.pri
│     ├─ qt_lib_genesisx_app_securestore.pri
│     ├─ qt_lib_genesisx_app_sensors.pri
│     ├─ qt_lib_genesisx_app_share.pri
│     ├─ qt_lib_genesisx_app_updater.pri
│     ├─ qt_lib_genesisx_orm.pri
│     └─ qt_lib_genesisx_physics.pri
├─ orm/
│  ├─ include/
│  │  └─ GenesisX/
│  │     └─ Orm/
│  │        ├─ AuthCredentials.h
│  │        ├─ Codegen.h
│  │        ├─ CommandController.h
│  │        ├─ ConfigIO.h
│  │        ├─ ConnectionCheck.h
│  │        ├─ ConnectionController.h
│  │        ├─ Controllers.h
│  │        ├─ DataAccess.h
│  │        ├─ genesisx_orm_global.h
│  │        ├─ HttpConfig.h
│  │        ├─ HttpResponse.h
│  │        ├─ Json.h
│  │        ├─ JsonAdapter.h
│  │        ├─ OrmQml.h
│  │        ├─ Repository.h
│  │        ├─ RepositoryOld.h
│  │        ├─ SqlConfig.h
│  │        └─ TransportConfig.h
│  ├─ src/
│  │  ├─ core/
│  │  │  ├─ CommandController.cpp
│  │  │  ├─ CommandControllerQml.cpp
│  │  │  ├─ CommandControllerQml.h
│  │  │  ├─ ConfigIO.cpp
│  │  │  ├─ ConnectionController.cpp
│  │  │  ├─ Controllers.cpp
│  │  │  ├─ HttpConnectionChecker.cpp
│  │  │  ├─ HttpResponse.cpp
│  │  │  └─ OrmQml.cpp
│  │  └─ tools/
│  │     └─ Codegen.cpp
│  └─ orm.pro
├─ physics/
│  ├─ include/
│  │  └─ GenesisX/
│  │     ├─ vehicles/
│  │     │  └─ vehicle4w.h
│  │     └─ genesisx_physics_global.h
│  ├─ plugin/
│  │  ├─ physics_plugin.cpp
│  │  └─ physics_plugin.pro
│  ├─ src/
│  │  └─ vehicles/
│  │     └─ vehicle4w.cpp
│  ├─ physics.json
│  └─ physics.pro
├─ scripts/
│  ├─ ci/
│  │  ├─ check-spdx-and-bom.bat
│  │  ├─ check-spdx-and-bom.ps1
│  │  └─ check-spdx-and-bom.sh
│  ├─ packages/
│  │  ├─ firebase.bat
│  │  └─ firebase.sh
│  ├─ add-headers.bat
│  ├─ add-headers.ps1
│  ├─ add-headers.sh
│  ├─ bootstrap.bat
│  ├─ bootstrap.sh
│  ├─ cleanup-merged-branches.bat
│  ├─ cleanup-merged-branches.ps1
│  ├─ cleanup-merged-branches.sh
│  ├─ collect-traffic.js
│  ├─ fetch-gpl-license.bat
│  ├─ fetch-gpl-license.ps1
│  ├─ fetch-gpl-license.sh
│  ├─ fix-bom.bat
│  ├─ fix-bom.ps1
│  ├─ install-prepush.bat
│  ├─ install-prepush.sh
│  ├─ release.bat
│  └─ release.sh
├─ tools/
│  ├─ gxgen/
│  │  ├─ debug/
│  │  │  ├─ config.json
│  │  │  ├─ genesisx.dll
│  │  │  ├─ genesisx_orm.dll
│  │  │  ├─ gxgen.exe
│  │  │  ├─ libgenesisx.a
│  │  │  └─ libgenesisx_orm.a
│  │  ├─ .qmake.stash
│  │  ├─ gxgen.pro
│  │  ├─ main.cpp
│  │  ├─ Makefile
│  │  ├─ Makefile.Debug
│  │  └─ Makefile.Release
│  ├─ qtcreator-snippets/
│  │  └─ snippets.xml
│  ├─ qtcreator-wizard/
│  │  ├─ projects/
│  │  │  ├─ GenesisXApiDatabase/
│  │  │  │  └─ templates/
│  │  │  │     └─ config.json.tmpl
│  │  │  ├─ GenesisXApp/
│  │  │  │  ├─ templates/
│  │  │  │  │  ├─ features/
│  │  │  │  │  │  └─ gx_app_root.prf.tmpl
│  │  │  │  │  ├─ project-ui/
│  │  │  │  │  │  ├─ android/
│  │  │  │  │  │  │  ├─ AndroidManifest.xml.tmpl
│  │  │  │  │  │  │  └─ google-services.json.tmpl
│  │  │  │  │  │  ├─ apple/
│  │  │  │  │  │  │  ├─ ios/
│  │  │  │  │  │  │  │  ├─ Entitlements.plist.tmpl
│  │  │  │  │  │  │  │  └─ Info.plist.tmpl
│  │  │  │  │  │  │  ├─ macos/
│  │  │  │  │  │  │  │  ├─ Entitlements.plist.tmpl
│  │  │  │  │  │  │  │  └─ Info.plist.tmpl
│  │  │  │  │  │  │  └─ GoogleService-Info.plist.tmpl
│  │  │  │  │  │  ├─ assets/
│  │  │  │  │  │  │  ├─ genesisx-xcore.svg
│  │  │  │  │  │  │  ├─ qmldir.tmpl
│  │  │  │  │  │  │  └─ Style.qml.tmpl
│  │  │  │  │  │  ├─ components/
│  │  │  │  │  │  │  ├─ qmldir.tmpl
│  │  │  │  │  │  │  └─ SplashWobble.qml.tmpl
│  │  │  │  │  │  ├─ src/
│  │  │  │  │  │  │  └─ main.cpp.tmpl
│  │  │  │  │  │  ├─ views/
│  │  │  │  │  │  │  └─ MasterView.qml.tmpl
│  │  │  │  │  │  ├─ assets.qrc.tmpl
│  │  │  │  │  │  ├─ components.qrc.tmpl
│  │  │  │  │  │  ├─ icons.qrc.tmpl
│  │  │  │  │  │  ├─ ui.pro.tmpl
│  │  │  │  │  │  └─ views.qrc.tmpl
│  │  │  │  │  ├─ .gitignore.tmpl
│  │  │  │  │  ├─ .qmake.conf.tmpl
│  │  │  │  │  └─ project.pro
│  │  │  │  └─ wizard.json
│  │  │  ├─ GenesisXPhysicsApp/
│  │  │  │  └─ wizard.json
│  │  │  └─ genesisx-xcore.png
│  │  └─ install-wizard.bat
│  └─ update_gradle_props.ps1.in
├─ zdev/
│  ├─ readyFordeletion/
│  │  ├─ ApiClient.cpp
│  │  └─ ApiClient.h
│  └─ zdev.pro
├─ .gitattributes
├─ .gitignore
├─ .gitlab-ci.yml
├─ BACKERS.md
├─ CODE_OF_CONDUCT.md
├─ CONTRIBUTING.md
├─ dev.pri
├─ GenesisX.pro
├─ GenesisX.pro.user
├─ LICENSE
├─ LICENSING.md
├─ README.md
├─ SECURITY.md
└─ SUPPORT.md
```

---

## Install / Setup

### 1) Clone
```bash
git clone <your-repo-url> Genesis-X
cd Genesis-X
```

### 2) Fetch 3rd‑party packages (Firebase)
**Windows (PowerShell/cmd):**
```bat
scripts\bootstrap.bat firebase
:: or:
scripts\bootstrap.bat all
```

**macOS / Linux:**
```bash
./scripts/bootstrap.sh firebase
# or:
./scripts/bootstrap.sh all
```

What it does:
- Downloads `firebase_cpp_sdk_13.1.0.zip`
- Verifies SHA‑256
- Unzips to `3rdparty/firebase_cpp_sdk/` (Git‑ignored)

---

## Integrate into Your App

> Goal: zero per‑app hardcoding of Firebase paths.

### A) App root: copy feature + `.qmake.conf`
Copy **`mkspecs/features`** from Genesis‑X into your **app root**, then create **`.qmake.conf`** in your app root:

```ini
# .qmake.conf (in your APP root)
load(gx_app_root)
QMAKEPATH += <absolute-or-relative-path-to>/Genesis-X
```

### B) App `.pro`: set Android package dir
```qmake
# Where Qt copies Android template files from:
ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
```

### C) Link Genesis‑X modules in your app
```qmake
# Core
QT += genesisx

# Optional modules
QT += genesisx-physics
```

### D) (Soon optional, now mandatory) Clean output structure in your app
```qmake
# ---------- Build type ----------
CONFIG(debug, debug|release|profile) {
    BUILD_PATH = debug
} else: CONFIG(profile, debug|release|profile) {
    BUILD_PATH = profile
} else {
    BUILD_PATH = release
}

DESTDIR = $$PWD/binaries/$$LOCAL_DESTINATION_PATH/$$BUILD_PATH
```

---

## How Android Path Resolution Works

**Problem:** Gradle needs absolute paths to Firebase C++ SDK.  
**Solution:** Genesis‑X generates them for you—no app changes needed.

1. `core/android-template/gradle.properties.in` contains tokens:
   ```properties
   firebase_cpp_sdk_dir=$$FIREBASE_CPP_SDK_DIR
   baseDir=$$FIREBASE_DEPENDENCIES_GRADLE
   ```
2. During **Run qmake**, the library computes:
   - `$$FIREBASE_CPP_SDK_DIR` → `<Genesis-X>/3rdparty/firebase_cpp_sdk`
   - `$$FIREBASE_DEPENDENCIES_GRADLE` → `<...>/Android/firebase_dependencies.gradle`
3. qmake writes the resolved file to the shadow build tree and the library build copies it back to:
   ```
   core/android-template/gradle.properties
   ```
4. `qt_lib_genesisx.pri` copies `build.gradle` and the **generated** `gradle.properties` into your app’s `ANDROID_PACKAGE_SOURCE_DIR`.  
   Qt then copies those into `android-build/` and Gradle uses them.

> If you move Genesis‑X, simply **rebuild the library**; paths are refreshed automatically.

---

## Scripts Reference

### Bootstrap (downloads 3rd‑party deps)
- **Windows**
  ```bat
  scripts\bootstrap.bat firebase
  scripts\bootstrap.bat all
  ```
- **macOS / Linux**
  ```bash
  ./scripts/bootstrap.sh firebase
  ./scripts/bootstrap.sh all
  ```

### SPDX headers (dual license)
```bash
# macOS/Linux
./scripts/add-headers.sh
# Windows
scripts\add-headers.bat
```
Writes **UTF‑8 (no BOM)** and respects shebangs. Override with env vars:
`COPYRIGHT_OWNER`, `COPYRIGHT_YEAR`, `LICENSE_EXPR`.

### Fix UTF‑8 BOM (Windows/qmake)
```bat
scripts\fix-bom.bat
```

### Pre‑push protection (main/staging)
```bash
./install-prepush.sh
# Windows: install-prepush.bat
```
Blocks direct pushes to `main`/`staging`. One‑off override:
`ALLOW_PROTECTED_PUSH=1 git push …`.

### Fetch GPL text
```bash
scripts/fetch-gpl-license.sh
# or Windows:
scripts\fetch-gpl-license.ps1
```

---

## Governance & Policies

- **Contributing Guide:** [CONTRIBUTING.md](CONTRIBUTING.md)  
- **Code of Conduct:** [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md)  
- **Security Policy:** [SECURITY.md](SECURITY.md)  
- **Support:** [SUPPORT.md](SUPPORT.md)

---

## Licensing

Genesis‑X is **dual‑licensed**:  
**(LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)**

See:
- `LICENSES/LicenseRef-KooijmanInc-Commercial.txt` (commercial terms)
- `LICENSES/GPL-3.0-only.txt` (GPL v3 only; use fetch script if placeholder)

All source files should include an SPDX header. See **LICENSING.md** for examples.

---

## Roadmap

- Linux notifications
- iOS/iPadOS/macOS notifications (APNs)
- Physics modules exposed to QML
- CI samples (bootstrap + Android matrix)

---

## Support

Open an issue/discussion and include:
- your app’s `ANDROID_PACKAGE_SOURCE_DIR`
- `core/android-template/gradle.properties` contents
- the Gradle error snippet
