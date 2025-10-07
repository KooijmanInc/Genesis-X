# Genesis-X

Qt library with a physics-engine foundation and cross-platform notifications.  
Tested with **Qt 6.10** (Qt Creator **17.0.2**).

---

## donations

[![Sponsor](https://img.shields.io/badge/Sponsor-GitHub%20Sponsors-ff69b4)](<your GH Sponsors URL>)
[![Donate](https://img.shields.io/badge/Donate-Buy%20Me%20a%20Coffee-yellow)](<your BMC URL>)
[![OpenCollective](https://img.shields.io/badge/Backers-Open%20Collective-3385ff)](<your OC URL>)
[![PayPal](https://img.shields.io/badge/Donate-PayPal-blue)](<your PayPal/Stripe URL>)

---

## 💖 Support Genesis-X

If Genesis-X saved you time, consider supporting development:

- GitHub Sponsors: <link>
- Buy Me a Coffee / Ko-fi: <link>
- Open Collective: <link>
- Company sponsorships (invoice/VAT): email sponsors@kooijman-inc.com

See also: **[BACKERS.md](BACKERS.md)** and **.github/FUNDING.yml**.

---

## Features

- QML-friendly Core API
- Notifications:
  - ✅ Android (Firebase C++ SDK)
  - ✅ Windows
  - ⏳ Linux, iOS/iPadOS, macOS (planned)
- Android Firebase integration auto-wired via generated `gradle.properties`
- Scripted 3rd-party setup (kept out of Git)

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
│  └─ packages/
│     └─ firebase.sh / .bat       # downloads + verifies Firebase C++ SDK
└─ 3rdparty/                      # ignored by Git; populated by scripts
```

---

## Install / Setup

### 1) Clone
```bash
git clone <your-repo-url> Genesis-X
cd Genesis-X
```

### 2) Fetch 3rd-party packages (Firebase)
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
- Verifies SHA-256
- Unzips to `3rdparty/firebase_cpp_sdk/` (Git-ignored)

---

## Integrate into Your App

> Goal: zero per-app hardcoding of Firebase paths.

### A) App root: copy feature + `.qmake.conf`
Copy **`mkspecs/features`** from Genesis-X into your **app root**, then create **`.qmake.conf`** in your app root:

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

### C) Link Genesis-X modules in your app
Add the modules you want to your app `.pro`:
```qmake
# Core
QT += genesisx

# Optional modules
QT += genesisx-physics
```

### D) (Soon Optional, now mandatory) Clean output structure in your app
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
**Solution:** Genesis-X generates them for you—no app changes needed.

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

> If you move Genesis-X, simply **rebuild the library**; paths are refreshed automatically.

---

## Scripts Reference

### Bootstrap (downloads 3rd-party deps)

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

Notes:
- Each package script is self-contained (`scripts/packages/<name>.sh|.bat`).
- Add new packages by creating a new pair of scripts and wiring them in `bootstrap`.

---

## google-services.json

- Place your **real** `google-services.json` in your app’s `ANDROID_PACKAGE_SOURCE_DIR` (e.g., `your-app/android/`).
- Genesis-X will copy a **template** if missing so the first build succeeds, but you must replace it.

---

## Troubleshooting

**Gradle: “Firebase C++ SDK not found”**
- Run bootstrap (`firebase` or `all`) so `3rdparty/firebase_cpp_sdk/` exists.
- Ensure your app sets `ANDROID_PACKAGE_SOURCE_DIR`.
- Rebuild the **library** (regenerates `core/android-template/gradle.properties`), then rebuild the **app**.

**google-services.json missing**
- Replace the template with the real file from Firebase Console.

**Windows path quirks**
- Genesis-X normalizes paths to forward slashes for Gradle; you don’t need to change anything.

---

## Git Hygiene

Recommended `.gitignore` excerpts (already configured):

```
# 3rd-party SDKs live locally
3rdparty/
!3rdparty/.keep

# Generated Android file
core/android-template/gradle.properties

# Builds & IDE
build-*/
build/
*.o
*.so
*.dll
*.user
.idea/
.vscode/
```

### Push to GitHub (public) and GitLab (private) from one working copy
```bash
git remote add origin  <github-url>
git remote add gitlab  <gitlab-url>

git push --all --follow-tags origin
git push --all --follow-tags gitlab
```

---

## File Headers / License

Project licensing: **Apache-2.0** — see [`LICENSE`](LICENSE) and [`NOTICE`](NOTICE).  
Add SPDX headers to sources so notices stay consistent.

**C/C++/Gradle/QML:**
```cpp
// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.
```

**qmake (.pro/.pri), bash:**
```bash
# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.
```

**batch (.bat):**
```bat
:: SPDX-License-Identifier: Apache-2.0
:: Copyright (c) 2025 Kooijman Incorporate Holding B.V.
```

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
