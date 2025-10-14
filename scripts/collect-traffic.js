#!/usr/bin/env node
// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

/**
 * Collects GitHub Traffic stats (clones, views, referrers, paths) and
 * stores a dated JSON snapshot in data/traffic/YYYY-MM-DD.json.
 *
 * Requires:
 *   - env TRAFFIC_TOKEN  (classic PAT with `repo`)
 *   - env REPO_FULL      ("owner/repo")
 */
const fs = require("fs");
const path = require("path");

const token = process.env.TRAFFIC_TOKEN;
const repoFull = process.env.REPO_FULL;

if (!token) {
  console.error("Missing TRAFFIC_TOKEN env var.");
  process.exit(1);
}
if (!repoFull || !repoFull.includes("/")) {
  console.error("Missing or invalid REPO_FULL env var (expected owner/repo).");
  process.exit(1);
}
const [owner, repo] = repoFull.split("/");

const headers = {
  "Accept": "application/vnd.github+json",
  "Authorization": `Bearer ${token}`,
  "X-GitHub-Api-Version": "2022-11-28"
};

async function gh(endpoint, params = "") {
  const url = `https://api.github.com${endpoint}${params ? "?" + params : ""}`;
  const res = await fetch(url, { headers });
  if (!res.ok) {
    const text = await res.text();
    throw new Error(`${res.status} ${res.statusText}: ${text}`);
  }
  return res.json();
}

(async () => {
  try {
    const today = new Date().toISOString().slice(0,10);
    const outDir = path.join("data", "traffic");
    fs.mkdirSync(outDir, { recursive: true });

    // 1) Clones (daily granularity, last 14 days)
    const clonesDaily = await gh(`/repos/${owner}/${repo}/traffic/clones`, "per=day");

    // 2) Views (daily)
    const viewsDaily  = await gh(`/repos/${owner}/${repo}/traffic/views`, "per=day");

    // 3) Popular referrers / paths (rolling 14 days)
    let referrers = [], paths = [];
    try {
      referrers = await gh(`/repos/${owner}/${repo}/traffic/popular/referrers`);
    } catch (_) { /* optional; ignore */ }
    try {
      paths = await gh(`/repos/${owner}/${repo}/traffic/popular/paths`);
    } catch (_) { /* optional; ignore */ }

    const snapshot = {
      repo: repoFull,
      collected_at: new Date().toISOString(),
      clones: clonesDaily,  // {count, uniques, clones:[{timestamp, count, uniques}]}
      views:  viewsDaily,   // {count, uniques, views:[{timestamp, count, uniques}]}
      referrers,            // [{referrer, count, uniques}]
      paths                 // [{path, title, count, uniques}]
    };

    const file = path.join(outDir, `${today}.json`);
    fs.writeFileSync(file, JSON.stringify(snapshot, null, 2));
    console.log(`Wrote ${file}`);

    // Optional: maintain a compact CSV for quick charting
    const toCsv = (rows, kind) =>
      rows.map(r => `${today},${kind},${r.timestamp || ""},${r.count},${r.uniques}`).join("\n");

    const csvPath = path.join(outDir, "traffic.csv");
    const rows = [];
    if (clonesDaily.clones) rows.push(toCsv(clonesDaily.clones, "clones"));
    if (viewsDaily.views)   rows.push(toCsv(viewsDaily.views,   "views"));
    const csvLine = rows.filter(Boolean).join("\n");
    if (csvLine) {
      if (!fs.existsSync(csvPath)) {
        fs.writeFileSync(csvPath, "date,metric,timestamp,count,uniques\n");
      }
      fs.appendFileSync(csvPath, csvLine + "\n");
      console.log(`Appended CSV rows to ${csvPath}`);
    }

  } catch (err) {
    console.error("Collector failed:", err.message);
    process.exit(1);
  }
})();
