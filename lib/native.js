/* eslint-disable import/no-unresolved */

const fs = require('fs');

if (fs.existsSync('build')) {
  module.exports = require('../build/Release/krypton.node');
} else {
  module.exports = require('../krypton.node');
}
