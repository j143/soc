# Roadmap

## 6G RAN Silicon Hierarchy — Die-Level Visualizer

This document tracks the planned features and milestones for the project.

---

## ✅ v1.0 — Current (Die Visualizer + WASM Simulation)

- Interactive SVG die-level visualizer for RU, DU, CU, and IO chips
- WebAssembly circuit simulation engine (compiled from C, no libm dependency)
- REST API backed by `chips.json` configuration (Express / Node.js)
- Per-block simulation: power, thermal, signal integrity, beamforming, LDPC, AI throughput
- GitHub Pages static deployment via GitHub Actions

---

## 🔨 v1.1 — Static Analysis & Data Export

- [ ] Export simulation results as JSON / CSV from the inspector panel
- [ ] Add a collapsible block-detail drawer with raw simulation parameters
- [ ] Keyboard navigation: arrow keys to cycle through die blocks
- [ ] Print / screenshot mode (high-contrast white background variant)

---

## 🔨 v1.2 — Extended Chip Library

- [ ] Add Open RAN O-RU, O-DU, O-CU reference configurations
- [ ] Support user-supplied `chips.json` loaded via drag-and-drop
- [ ] Versioned chip configurations (semver tag in `chips.json`)
- [ ] Compare view: side-by-side diff of two chip layouts

---

## 🔨 v2.0 — AI-Assisted Design Analysis

- [ ] Natural-language chip query interface (agent-ready REST endpoint)
- [ ] LLM-driven design-space exploration via the `/api/chips` endpoint
- [ ] Anomaly detection: flag blocks with unusually high power density
- [ ] Auto-generate `agents.md`-compatible tool schema from simulation exports
- [ ] Streaming simulation results for long-running multi-block sweeps

---

## 🔨 v2.1 — Multi-Layer Die View

- [ ] Toggle between metal layers (METAL_1 … METAL_12)
- [ ] Render power-domain boundaries and clock distribution trees
- [ ] Animated signal-path trace for selected block pairs

---

## 🔨 v3.0 — Collaborative & Real-Time

- [ ] WebSocket push for live simulation updates
- [ ] Multi-user session: shared cursor & annotation overlay
- [ ] Integration with standard EDA netlist formats (DEF / LEF import)

---

## Contributing

See [`agents.md`](agents.md) for the AI agent developer guide, and the project
[README](README.md) for setup instructions.
