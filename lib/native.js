/* eslint-disable import/no-unresolved */

const fs = require('fs');
const path = require('path');

if (fs.existsSync(path.join(__dirname, '..', 'build'))) {
  module.exports = require('../build/Release/krypton.node');
} else {
  module.exports = require('../krypton.node');
}
