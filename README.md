# soc
soc visualizations

## 6G RAN Silicon Hierarchy — Die-Level Visualizer

A high-fidelity interactive app for understanding 6G RAN deployment design at the silicon level.
The circuit simulation engine runs entirely in **WebAssembly** (compiled from C) inside the browser.

### Architecture

```
Browser
  └── public/index.html      ← die visualizer UI
  └── public/sim.wasm        ← circuit simulation engine (C → wasm32)

Server (Node.js / Express)
  └── server.js              ← static files + REST API
  └── public/chips.json      ← chip configuration data source

Simulation source
  └── wasm/sim.c             ← C simulation module
  └── wasm/Makefile          ← clang wasm32 build
```

### REST API

| Method | Path              | Description                       |
|--------|-------------------|-----------------------------------|
| GET    | `/api/chips`      | All chip configurations (JSON)    |
| GET    | `/api/chips/:id`  | Single chip config (`ru/du/cu/io`)|

### WASM Simulation Functions

The `sim.wasm` module exposes these exported functions (all `f32`):

| Function | Description |
|---|---|
| `compute_block_power(type, freq, vdd, active)` | Dynamic + static power (mW) |
| `compute_power_density(power, area)` | Power density (mW/mm²) |
| `compute_thermal(power, Rth, T_amb)` | Junction temperature (°C) |
| `compute_rc_delay(length, RC)` | Elmore RC delay (fs) |
| `compute_signal_integrity(freq, length)` | Insertion loss (dB) |
| `compute_beamforming_gain(N, g_elem)` | Array gain (dBi) |
| `compute_snr(Tx, path_loss, NF)` | Link SNR (dB) |
| `compute_spectral_efficiency(SNR, BW)` | Shannon capacity (Gbps) |
| `compute_ldpc_throughput(rate, clk, N)` | LDPC decode throughput (Gbps) |
| `compute_fft_latency(N, clk, par)` | FFT pipeline latency (ns) |
| `compute_ai_throughput(TOPS, util, prec)` | Effective AI throughput (TOPS) |
| `compute_switching_latency(ports, clk)` | Cut-through latency (ns) |
| `compute_link_power(rate, E_bit)` | SerDes link power (mW) |

### Prerequisites

- **Node.js** ≥ 18
- **Clang** ≥ 14 with `wasm32` target (`clang --print-targets | grep wasm`)
- **wasm-ld** (`apt install lld` on Ubuntu)

### Quick Start

```bash
# 1. Install Node dependencies
npm install

# 2. (Re)compile the WASM simulation engine
npm run build:wasm

# 3. Start the server
npm start
# → http://localhost:3000
```

### Usage

1. Open `http://localhost:3000`
2. Click a tab to switch between **RU**, **DU**, **CU**, or **IO** chips
3. Click any coloured block on the die diagram
4. The **CIRCUIT_SIMULATION** panel on the right runs the WASM engine and
   displays power density, junction temperature, signal loss, RC delay,
   and block-specific metrics (beamforming gain, LDPC throughput, etc.)

