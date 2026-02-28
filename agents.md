# Agent Developer Guide

This document describes the project structure, available interfaces, and
conventions for AI agents (LLM-based coding assistants, automation scripts,
or autonomous agents) working on this repository.

---

## Repository Layout

```
soc/
├── public/               # Static assets served by Express & GitHub Pages
│   ├── index.html        # Main UI — die visualizer + WASM simulation panel
│   ├── chips.json        # Chip configuration data (single source of truth)
│   └── sim.wasm          # Compiled WebAssembly simulation engine
├── wasm/
│   ├── sim.c             # C source for the simulation engine
│   └── Makefile          # clang wasm32 build (outputs public/sim.wasm)
├── server.js             # Express server (REST API + static file serving)
├── package.json
├── roadmap.md            # Project milestones
└── agents.md             # This file
```

---

## Data Model

### `public/chips.json`

The canonical source for all chip configurations.  
Shape of each entry:

```jsonc
{
  "<chip_id>": {           // "ru" | "du" | "cu" | "io"
    "name":     "string",  // Marketing / part name
    "tag":      "string",  // Short category label shown in UI badge
    "tagColor": "string",  // CSS color value for the badge
    "desc":     "string",  // One-sentence description
    "specs": {             // Key/value pairs for the inspector panel
      "<label>": "<value>"
    },
    "layout": [            // Die block list (rendered as SVG rects)
      {
        "id":        "string",   // Block ID referenced by simParams in index.html
        "x":         number,     // SVG x position (0–1000 coordinate space)
        "y":         number,
        "w":         number,     // Width
        "h":         number,     // Height
        "label":     "string",   // Displayed on die
        "highlight": boolean     // Optional — pulse animation + accent stroke
      }
    ]
  }
}
```

Agents that add new chips or blocks **must** update both `chips.json` and
the `simParams` object inside `public/index.html`.

---

## REST API (server.js)

| Method | Path              | Description                                     |
|--------|-------------------|-------------------------------------------------|
| GET    | `/api/chips`      | Full `chips.json` payload                       |
| GET    | `/api/chips/:id`  | Single chip config — 404 if `id` not found      |

The static middleware also serves every file under `public/` at the
corresponding path (e.g. `GET /sim.wasm`, `GET /chips.json`).

When running under GitHub Pages (no server), the UI fetches
`./chips.json` and `./sim.wasm` directly as static assets.

---

## WASM Simulation API (`wasm/sim.c`)

All functions accept and return `float` (f32).  
Rebuild after changing `sim.c`:

```bash
npm run build:wasm   # requires clang ≥ 14 with wasm32 target + wasm-ld
```

| Exported function | Signature | Returns |
|---|---|---|
| `compute_block_power` | `(block_type: i32, freq_ghz, vdd, active_factor)` | mW |
| `compute_power_density` | `(total_power_mw, area_mm2)` | mW/mm² |
| `compute_thermal` | `(power_mw, r_thermal_c_per_w, ambient_c)` | °C |
| `compute_rc_delay` | `(length_um, rc_product_fs_per_um2)` | fs |
| `compute_signal_integrity` | `(freq_ghz, trace_length_mm)` | dB |
| `compute_beamforming_gain` | `(num_antennas: i32, element_gain_dbi)` | dBi |
| `compute_snr` | `(tx_power_dbm, path_loss_db, noise_figure_db)` | dB |
| `compute_spectral_efficiency` | `(snr_db, bandwidth_ghz)` | Gbps |
| `compute_ldpc_throughput` | `(code_rate, clock_ghz, parallelism: i32)` | Gbps |
| `compute_fft_latency` | `(fft_size: i32, clock_ghz, parallel_units: i32)` | ns |
| `compute_ai_throughput` | `(tops, utilization, precision_factor)` | TOPS |
| `compute_switching_latency` | `(fabric_ports: i32, clock_ghz)` | ns |
| `compute_link_power` | `(data_rate_gbps, energy_per_bit_pj)` | mW |

`block_type` codes: `0` unknown · `1` AFE · `2` digital logic ·
`3` IO/SerDes · `4` compute core · `5` switching fabric

---

## Development Workflow

```bash
npm install          # install Express + rate-limiter
npm run build:wasm   # (re)compile sim.c → public/sim.wasm
npm start            # http://localhost:3000
```

The GitHub Actions workflow (`.github/workflows/pages.yml`) automatically
deploys the `public/` directory to GitHub Pages on every push to `main`.

---

## Agent Conventions

1. **Chip data changes** — edit `public/chips.json`; keep `simParams` in
   `public/index.html` in sync (same block IDs, realistic parameter values).

2. **New simulation functions** — add the C implementation to `wasm/sim.c`,
   re-run `npm run build:wasm`, then call the new export from `index.html`.

3. **Style** — the UI uses CSS custom properties defined in `:root` in
   `public/index.html`. Prefer those variables over hard-coded hex values.

4. **No build step for JS/CSS** — all frontend assets are plain files;
   do not introduce a bundler without updating this guide and the CI workflow.

5. **Security** — the Express server applies rate-limiting via
   `express-rate-limit`. Do not remove or weaken that middleware.
