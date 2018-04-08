const fs = require('fs');
const { spawn } = require('child_process');

const config = {
  target_defaults: {
    defines: [],
    libraries: [],
    cflags: [],
    xcode_settings: {
      OTHER_CFLAGS: [],
    },
    msvs_settings: {
      VCCLCompilerTool: {
        AdditionalOptions: [],
      },
    },
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

if (!process.env.npm_config_krypton_disable_avx2 && ['x64', 'ia32'].includes(process.arch)) {
  config.target_defaults.cflags.push('-mavx2');
  config.target_defaults.xcode_settings.OTHER_CFLAGS.push('-mavx2');
  config.target_defaults.msvs_settings.VCCLCompilerTool.AdditionalOptions.push('/arch:AVX2');
}

if (process.env.npm_config_krypton_enable_avx_512) {
  config.target_defaults.cflags.push('-mavx512f');
  config.target_defaults.cflags.push('-mavx512bw');
}

if (process.env.npm_config_krypton_disable_neon) {
  config.target_defaults.defines.push('KRYPTON_DISABLE_NEON');
}

fs.writeFileSync('config.gypi', JSON.stringify(config, undefined, 2));

if (process.platform !== 'win32') {
  spawn('node-gyp', ['rebuild'], { stdio: 'inherit' });
} else {
  console.log('Not running "node-gyp rebuild", you must run this yourself.');
}
