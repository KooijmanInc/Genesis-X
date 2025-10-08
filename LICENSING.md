<!-- SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only) -->
<!-- Copyright (c) 2025 Kooijman Incorporate Holding B.V. -->

# Licensing

Genesis-X is dual‑licensed under:

- **Commercial**: `LicenseRef-KooijmanInc-Commercial`
- **Open source**: **GPL-3.0-only**

You may choose **either** license for any copy of the library.

---

## Which license should I choose?

| Scenario | Recommended |
| --- | --- |
| You are building **closed-source** or **proprietary** software, or you want private redistribution terms | **Commercial** (`LicenseRef-KooijmanInc-Commercial`) |
| You are building **open source** software that is compatible with **GPL-3.0-only** and you agree to GPL terms (copyleft) | **GPL-3.0-only** |

> Commercial terms are in `LICENSES/LicenseRef-KooijmanInc-Commercial.txt`.  
> GPL text is in `LICENSES/GPL-3.0-only.txt` (use the helper script to fetch the official text if you see a placeholder).

---

## SPDX headers

All source files should begin with an SPDX header so tools and CI can detect the chosen license:

**C/C++/QML/JS/Gradle/Java/Kotlin**:
```cpp
// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.
```

**qmake (.pro/.pri), Bash, YAML**:
```bash
# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.
```

**Batch (.bat/.cmd)**:
```bat
:: SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
:: Copyright (c) 2025 Kooijman Incorporate Holding B.V.
```

**XML/QRC/UI/HTML/Markdown**:
```xml
<!-- SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only) -->
<!-- Copyright (c) 2025 Kooijman Incorporate Holding B.V. -->
```

> Do **not** add headers to files that cannot contain comments (e.g., JSON) or to third‑party vendored sources.

We ship scripts to add headers automatically:
- `scripts/add-headers.sh` (macOS/Linux)
- `scripts/add-headers.ps1` + `scripts/add-headers.bat` (Windows)

These write **UTF‑8 without BOM** and respect shebang lines. See `CONTRIBUTING.md` for usage.

---

## Included license files

- `LICENSES/LicenseRef-KooijmanInc-Commercial.txt` — commercial terms.  
- `LICENSES/GPL-3.0-only.txt` — GPL v3 only.
  - If this file contains a placeholder, fetch the official text with one of:
    ```bash
    scripts/fetch-gpl-license.sh
    # or on Windows:
    scripts\fetch-gpl-license.ps1
    ```

---

## Third‑party software

Genesis‑X may depend on third‑party software that is licensed under its own terms. Those terms **continue to apply** to those components. Please refer to notices bundled with the respective packages and our `NOTICE` file if present.

---

## Trademarks

“Genesis‑X” is used as a project name. Company names and product names are trademarks of their respective owners.

---

## Contact

For commercial licensing or support, email **sponsors@kooijman-inc.com**.
