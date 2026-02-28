'use strict';

const express = require('express');
const path = require('path');

const app = express();
const PORT = process.env.PORT || 3000;

// Serve everything under public/ as static files
app.use(express.static(path.join(__dirname, 'public')));

// Fallback: serve index.html for any unmatched route
app.get('*', (_req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

app.listen(PORT, () => {
  console.log(`6G RAN Viz server running at http://localhost:${PORT}`);
});
