<!-- SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only) -->
<!-- Copyright (c) 2025 Kooijman Incorporate Holding B.V. -->

# Contributing to Genesis‑X

Thanks for taking the time to contribute! This guide explains how we work and how to get your changes merged smoothly.

---

## Project flow (branches)

We keep it simple:

- Default branch: **`main`** (stable, release-ready)
- Integration branch: **`staging`** (where features land first)
- Work happens on **feature branches** off `staging`, e.g. `feat/linux-notifications`

**Preferred merges:** fast‑forward (linear history). Use squash‑merge for multi‑commit features if it reads cleaner.

### Quick start
```bash
git checkout staging
git pull --ff-only
git checkout -b feat/your-change

# ... commit work ...

git push -u origin feat/your-change

# open MR/PR → staging
git checkout staging
git pull --ff-only
git merge --ff-only feat/your-change
git push origin staging
```

When ready to release:
```bash
git checkout main
git pull --ff-only
git merge --ff-only staging
git tag -a vX.Y.Z -m "Your summary"
git push origin main --follow-tags
```

---

## Commit style

Use **Conventional Commits** to make history and changelogs useful:

- `feat: add linux notifications`
- `fix: handle null token on android`
- `docs: add installation steps`
- `chore: bump firebase sdk script`

Small focused commits > giant “mixed” commits.

---

## Code style & scope

- C++20/23 with Qt 6 APIs.
- Keep headers clean and QML‑friendly where relevant.
- Platform‑specific code should live under clear folders (`core/src/notifications/…`).

Add/update unit or integration tests where it makes sense.

---

## Scripts you can use

All scripts live under `scripts/`.

### Bootstrap third‑party SDKs
Fetch and verify 3rd‑party packages outside Git history.

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

Package scripts reside in `scripts/packages/` (e.g., `firebase.(sh|bat)`), with checksum verification.

### Generate Android Gradle properties
The library generates `core/android-template/gradle.properties` from `gradle.properties.in` via qmake substitution. You generally don’t need to touch this in your app; rebuilding the library refreshes absolute paths.

### Release helpers
Create and push a tag (mirror to GitHub occurs automatically if configured).

- **Windows**
  ```bat
  scripts\release.bat v0.1.0 "Initial release"
  ```

- **macOS / Linux**
  ```bash
  ./scripts/release.sh v0.1.0 "Initial release"
  ```

### Add SPDX headers (dual‑license)
Adds missing headers with the project’s dual‑license expression and writes **UTF‑8 (no BOM)**.

- **Windows**
  ```bat
  scripts\add-headers.bat
  ```

- **macOS / Linux**
  ```bash
  ./scripts/add-headers.sh
  ```

Environment overrides:
```bash
COPYRIGHT_OWNER="Kooijman Incorporate Holding B.V." COPYRIGHT_YEAR="2025" LICENSE_EXPR="(LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)" ./scripts/add-headers.sh
```

### Fix UTF‑8 BOM (Windows qmake safety)
If you see **“Unexpected UTF‑8 BOM”** from qmake:

- **Windows**
  ```bat
  scripts\fix-bom.bat
  ```

- **PowerShell**
  ```powershell
  scripts\fix-bom.ps1
  ```

### Fetch GPL license text
Replace the placeholder with the official GPL v3 text:
```bash
scripts/fetch-gpl-license.sh
# or on Windows:
scripts\fetch-gpl-license.ps1
```

### Prevent pushing to protected branches
Local hook to block pushes to **`main`** and **`staging`** (override once with `ALLOW_PROTECTED_PUSH=1`).

- `.githooks/pre-push` (bash hook)
- Install helpers:
  - `install-prepush.sh` (macOS/Linux)
  - `install-prepush.bat` (Windows)

Usage:
```bash
# one-time in each clone
./install-prepush.sh
# Windows: install-prepush.bat
```

---

## Licensing of contributions

By submitting a contribution, you agree to license your work under the project’s **dual license**:

```
(LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
```

Please add an SPDX header to new files; otherwise the header scripts will do it during review.

> If your employer requires a contributor agreement or special crediting, please mention it in your PR description.

---

## Opening issues

When filing a bug or feature request, please include (as applicable):

- OS / platform, Qt version, toolchain
- For Android: output of Gradle error and the generated `core/android-template/gradle.properties`
- Repro steps or minimal example
- Expected vs actual result

---

## PR checklist

- [ ] Conventional commit messages
- [ ] SPDX headers present (or run header script)
- [ ] Builds locally (target platforms affected by the change)
- [ ] Tests/docs updated if relevant

Thanks for helping improve Genesis‑X!
