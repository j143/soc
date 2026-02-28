'use strict';

const express    = require('express');
const path       = require('path');
const rateLimit  = require('express-rate-limit');

const app  = express();
const PORT = process.env.PORT || 3000;

// Chip configuration data (single source of truth for the frontend)
const chipData = require('./public/chips.json');

// ── Rate limiting ───────────────────────────────────────────────────────
const limiter = rateLimit({
  windowMs: 15 * 60 * 1000,  // 15 minutes
  max: 300,                   // max requests per window per IP
  standardHeaders: true,
  legacyHeaders: false,
});
app.use(limiter);

// ── REST API ────────────────────────────────────────────────────────────

// GET /api/chips  →  all chip configurations
app.get('/api/chips', (_req, res) => {
  res.json(chipData);
});

// GET /api/chips/:id  →  single chip configuration
app.get('/api/chips/:id', (req, res) => {
  const chip = chipData[req.params.id];
  if (!chip) return res.status(404).json({
    error: 'Chip not found',
    requested: req.params.id,
    available: Object.keys(chipData)
  });
  res.json(chip);
});

// ── Static files (public/) ──────────────────────────────────────────────
app.use(express.static(path.join(__dirname, 'public')));

// Fallback: serve index.html for any unmatched route
app.get('*', (_req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

app.listen(PORT, () => {
  console.log(`6G RAN Viz server running at http://localhost:${PORT}`);
});
