/*
 * 6G RAN Circuit Simulation — WebAssembly Module
 *
 * Build:
 *   clang --target=wasm32 -nostdlib -O2 \
 *         -Wl,--no-entry -Wl,--export-dynamic \
 *         -o ../public/sim.wasm sim.c
 *
 * All arithmetic is self-contained; no libc/libm required.
 */

/* ── Portable bit helpers (safe type-pun via memcpy) ─────────────────── */

static unsigned int f2bits(float x) {
    unsigned int b;
    __builtin_memcpy(&b, &x, 4);
    return b;
}

static float bits2f(unsigned int b) {
    float x;
    __builtin_memcpy(&x, &b, 4);
    return x;
}

/* ── Fast math (no libm) ─────────────────────────────────────────────── */

/* log2(x) — Minmax polynomial on mantissa in [1,2), error < 1e-3 */
static float fast_log2f(float x) {
    if (x <= 0.0f) return -100.0f;
    unsigned int bits = f2bits(x);
    int exp = (int)((bits >> 23) & 0xFFu) - 127;
    float m = bits2f((bits & 0x007FFFFFu) | 0x3F800000u);
    float a = -1.7417939f
            + m * ( 2.8212026f
            + m * (-1.4699568f
            + m * ( 0.44717955f
            + m * (-0.056570851f))));
    return (float)exp + a;
}

static float fast_log10f(float x) { return fast_log2f(x) * 0.30103f; }

/* 2^x — Taylor series for fractional part; repeated multiply for integer part.
 * Saturates at ±50 to prevent infinite loops for extreme inputs. */
static float fast_pow2f(float x) {
    if (x >  50.0f) return 1125899906842624.0f; /* 2^50 – saturate */
    if (x < -50.0f) return 8.88e-16f;           /* 2^-50 – saturate */
    int   xi = (int)x;
    float xf = x - (float)xi;
    float t  = xf * 0.693147f;
    float r  = 1.0f + t * (1.0f + t * (0.5f + t * (0.166667f + t * 0.041667f)));
    if (xi >= 0) { for (int i = 0; i < xi; i++) r *= 2.0f; }
    else         { for (int i = 0; i > xi; i--) r *= 0.5f; }
    return r;
}

/* ── Power Analysis ──────────────────────────────────────────────────── */
/*
 * block_type codes:
 *   0 = unknown   1 = AFE (ADC/DAC)   2 = digital logic
 *   3 = IO/SerDes   4 = compute core   5 = switching fabric
 */
__attribute__((visibility("default")))
float compute_block_power(int block_type, float freq_ghz,
                          float vdd, float active_factor) {
    /* Representative switching capacitance per block category (pF) */
    static const float cap_pf[6] = { 0.0f, 2.5f, 8.0f, 1.2f, 15.0f, 4.0f };
    /* Reset to type 0 (unknown, 0 pF) for any invalid input */
    if (block_type < 0 || block_type > 5) block_type = 0;
    float cap      = cap_pf[block_type];
    float p_dyn    = active_factor * cap * 1e-12f * vdd * vdd * freq_ghz * 1e9f * 1e3f; /* mW */
    float p_static = vdd * (cap * 0.01f);  /* leakage: ~1 % of cap */
    return p_dyn + p_static;
}

__attribute__((visibility("default")))
float compute_power_density(float total_power_mw, float area_mm2) {
    if (area_mm2 <= 0.0f) return 0.0f;
    return total_power_mw / area_mm2;   /* mW/mm² */
}

__attribute__((visibility("default")))
float compute_thermal(float power_mw, float r_thermal_c_per_w, float ambient_c) {
    return ambient_c + (power_mw * 0.001f) * r_thermal_c_per_w;  /* °C */
}

/* ── Signal Integrity ────────────────────────────────────────────────── */

__attribute__((visibility("default")))
float compute_rc_delay(float length_um, float rc_product_fs_per_um2) {
    /* Elmore RC delay: t = 0.38 × R × C × L² */
    return 0.38f * rc_product_fs_per_um2 * length_um * length_um;  /* fs */
}

__attribute__((visibility("default")))
float compute_signal_integrity(float freq_ghz, float trace_length_mm) {
    /* Skin-effect + dielectric loss — simplified empirical model (dB) */
    float skin = 0.04f * freq_ghz * trace_length_mm;
    float diel = 0.02f * freq_ghz * trace_length_mm;
    return skin + diel;
}

/* ── 6G RF / Beamforming (RU) ────────────────────────────────────────── */

__attribute__((visibility("default")))
float compute_beamforming_gain(int num_antennas, float element_gain_dbi) {
    if (num_antennas <= 0) return 0.0f;
    return 10.0f * fast_log10f((float)num_antennas) + element_gain_dbi;  /* dBi */
}

__attribute__((visibility("default")))
float compute_snr(float tx_power_dbm, float path_loss_db, float noise_figure_db) {
    /* Noise floor at 400 MHz BW: -174 + 10×log10(400e6) ≈ -88 dBm */
    float noise_floor_dbm = -88.0f;
    return tx_power_dbm - path_loss_db - noise_floor_dbm - noise_figure_db;
}

__attribute__((visibility("default")))
float compute_spectral_efficiency(float snr_db, float bandwidth_ghz) {
    /* Shannon: C [Gbps] = B [GHz] × log2(1 + SNR_lin),  SNR_lin = 10^(snr_db/10) */
    float snr_lin = fast_pow2f(snr_db * 0.332193f);   /* ≡ 10^(snr_db/10) */
    return bandwidth_ghz * fast_log2f(1.0f + snr_lin);  /* Gbps */
}

/* ── Digital Processing (DU) ─────────────────────────────────────────── */

__attribute__((visibility("default")))
float compute_ldpc_throughput(float code_rate, float clock_ghz, int parallelism) {
    /* Throughput = clock × parallelism × code_rate × HW_efficiency */
    return clock_ghz * (float)parallelism * code_rate * 0.85f;  /* Gbps */
}

__attribute__((visibility("default")))
float compute_fft_latency(int fft_size, float clock_ghz, int parallel_units) {
    if (parallel_units <= 0 || fft_size <= 0 || clock_ghz <= 0.0f) return 0.0f;
    float stages = fast_log2f((float)fft_size);
    float cycles = stages * ((float)fft_size / (float)parallel_units);
    return cycles / (clock_ghz * 1e9f) * 1e9f;   /* ns */
}

/* ── AI / Compute (CU) ───────────────────────────────────────────────── */

__attribute__((visibility("default")))
float compute_ai_throughput(float tops, float utilization, float precision_factor) {
    return tops * utilization * precision_factor;  /* effective TOPS */
}

/* ── Switching Fabric (IO) ───────────────────────────────────────────── */

__attribute__((visibility("default")))
float compute_switching_latency(int fabric_ports, float clock_ghz) {
    /* Cut-through: SerDes header latency + log2(ports) pipeline stages */
    float serdes_latency  = 128.0f / 112.0f;                       /* ns  (PAM4 112G) */
    float pipeline_stages = fast_log2f((float)fabric_ports) * 2.0f;
    float fabric_latency  = pipeline_stages / clock_ghz;            /* ns */
    return serdes_latency + fabric_latency;
}

__attribute__((visibility("default")))
float compute_link_power(float data_rate_gbps, float energy_per_bit_pj) {
    return data_rate_gbps * energy_per_bit_pj;  /* mW  (Gbps × pJ/bit = mW) */
}
