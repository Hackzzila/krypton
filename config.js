const fs = require('fs');
const { spawn } = require('child_process');

const config = {
  target_defaults: {
    defines: [],
    libraries: [],
  },
};

if (!process.env.npm_config_krypton_disable_opus && process.platform !== 'win32') {
  config.target_defaults.libraries.push('-lopus');
} else if (process.env.npm_config_krypton_disable_opus) {
  config.target_defaults.defines.push('KRYPTON_DISABLE_OPUS');
}

if (!process.env.npm_config_krypton_disable_lame && process.platform !== 'win32') {
  config.target_defaults.libraries.push('-lmp3lame');
} else if (process.env.npm_config_krypton_disable_lame) {
  config.target_defaults.defines.push('KRYPTON_DISABLE_LAME');
}

if (!process.env.npm_config_krypton_disable_sodium && process.platform !== 'win32') {
  config.target_defaults.libraries.push('-lsodium');
} else if (process.env.npm_config_krypton_disable_sodium) {
  config.target_defaults.defines.push('KRYPTON_DISABLE_SODIUM');
}

fs.writeFileSync('config.gypi', JSON.stringify(config, undefined, 2));

if (process.platform !== 'win32') {
  spawn('node-gyp', ['rebuild'], { stdio: 'inherit' });
} else {
  console.log('Not running "node-gyp rebuild", you must run this yourself.');
}
