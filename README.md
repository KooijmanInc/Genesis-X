<!-- SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only) -->
<!-- Copyright (c) 2025 Kooijman Incorporate Holding B.V. -->

# Genesis-X

[![CI – Quality checks](https://github.com/KooijmanInc/Genesis-X/actions/workflows/quality.yml/badge.svg?branch=main)](https://github.com/KooijmanInc/Genesis-X/actions/workflows/quality.yml)
[![CI – Quality checks (staging)](https://github.com/KooijmanInc/Genesis-X/actions/workflows/quality.yml/badge.svg?branch=staging)](https://github.com/KooijmanInc/Genesis-X/actions/workflows/quality.yml)
[![GitLab pipeline](https://gitlab.com/kooijmaninc2/genesisx/genesis-x/badges/main/pipeline.svg)](https://gitlab.com/kooijmaninc2/genesisx/genesis-x/-/pipelines)


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

## Features

- QML‑friendly Core API
- Notifications:
  - ✅ Android (Firebase C++ SDK)
  - ✅ Windows
  - ⏳ Linux, iOS/iPadOS, macOS (planned)
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
Genesis-X/
├─ core/
│  ├─ include/GenesisX/...
│  ├─ src/...
│  └─ android-template/
│     ├─ build.gradle
│     ├─ gradle.properties.in     # template with $$VARIABLE tokens
│     └─ (gradle.properties)      # generated; ignored by Git
├─ mkspecs/features/              # qmake feature: gx_app_root
├─ scripts/
│  ├─ bootstrap.sh                # macOS/Linux entry
│  ├─ bootstrap.bat               # Windows entry
│  ├─ add-headers.(sh|ps1|bat)    # SPDX header helper
│  ├─ fix-bom.(ps1|bat)           # strip UTF-8 BOMs (Windows/qmake safety)
│  ├─ install-prepush.(sh|bat)    # install pre-push hook that blocks main/staging
│  ├─ fetch-gpl-license.(sh|ps1|bat) # fetch official GPL-3.0-only text
│  └─ packages/
│     └─ firebase.(sh|bat)        # downloads + verifies Firebase C++ SDK
└─ 3rdparty/                      # ignored by Git; populated by scripts
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
