const fs = require('fs');

const config = {
  target_defaults: {
    defines: [],
    libraries: [],
  },
};

if (!process.env.npm_config_krypton_disable_opus) {
  config.target_defaults.libraries.push('-lopus');
} else {
  config.target_defaults.defines.push('KRYPTON_DISABLE_OPUS');
}

if (!process.env.npm_config_krypton_disable_lame) {
  config.target_defaults.libraries.push('-lmp3lame');
} else {
  config.target_defaults.defines.push('KRYPTON_DISABLE_LAME');
}

if (!process.env.npm_config_krypton_disable_sodium) {
  config.target_defaults.libraries.push('-lsodium');
} else {
  config.target_defaults.defines.push('KRYPTON_DISABLE_SODIUM');
}

fs.writeFileSync('config.gypi', JSON.stringify(config, undefined, 2));
