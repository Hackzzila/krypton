const abi = require('node-abi');
const path = require('path');
const fs = require('fs');
const { execSync } = require('child_process');
const { get, post } = require('superagent');
const tar = require('tar');
const xml = require('xml-js');

const arches = {
  win32: ['x64', 'ia32'],
  linux: ['x64', 'ia32', 'arm', 'arm64'],
  darwin: ['x64'],
};

const targets = ['8.0.0', '9.0.0'];

const env = {
  arm: {
    CC: 'arm-linux-gnueabihf-gcc',
    CXX: 'arm-linux-gnueabihf-g++',
    LD: 'arm-linux-gnueabihf-ld',
    CFLAGS: '-fPIC',
    CXXFLAGS: '-fPIC',
  },
  arm64: {
    CC: 'aarch64-linux-gnu-gcc',
    CXX: 'aarch64-linux-gnu-g++',
    LD: 'aarch64-linux-gnu-ld',
    CFLAGS: '-fPIC',
    CXXFLAGS: '-fPIC',
  },
  x64: {
    CFLAGS: '-fPIC',
    CXXFLAGS: '-fPIC',
  },
  ia32: {
    CFLAGS: '-m32 -fPIC',
    CXXFLAGS: '-m32 -fPIC',
    LDFLAGS: '-m32',
  },
};

async function compileLibrary(name, arch, url) {
  if (fs.existsSync(path.resolve('deps', arch, name))) return;

  fs.mkdirSync(path.resolve('deps', arch, name));

  const stream = tar.x({
    strip: 1,
    cwd: path.resolve('deps', arch, name),
  });

  await get(url);

  get(url).pipe(stream);

  await new Promise(r => stream.on('finish', r));

  let cross = '';
  if (arch === 'ia32') {
    cross = '--build=x86_64-pc-linux-gnu --host=i686-pc-linux-gnu';
  } else if (arch === 'arm') {
    cross = '--build=x86_64-pc-linux-gnu --host=arm-linux-gnueabihf';
  } else if (arch === 'arm64') {
    cross = '--build=x86_64-pc-linux-gnu --host=aarch64-linux-gnu';
  }

  execSync(`./configure ${cross} --disable-shared --disable-pie --prefix=${path.resolve('deps', arch, 'install')}`, {
    stdio: 'inherit',
    cwd: path.resolve('deps', arch, name),
    env: {
      ...process.env,
      ...env[arch],
    },
  });

  execSync('make', {
    stdio: 'inherit',
    cwd: path.resolve('deps', arch, name),
    env: {
      ...process.env,
      ...env[arch],
    },
  });

  execSync('make install', {
    stdio: 'inherit',
    cwd: path.resolve('deps', arch, name),
    env: {
      ...process.env,
      ...env[arch],
    },
  });
}

if (!fs.existsSync('deps')) fs.mkdirSync('deps');
if (!fs.existsSync('builds')) fs.mkdirSync('builds');

async function linux() {
  for (const arch of arches[process.platform]) {
    if (!fs.existsSync(path.resolve('deps', arch))) fs.mkdirSync(path.resolve('deps', arch));
    if (!fs.existsSync(path.resolve('deps', arch, 'install'))) fs.mkdirSync(path.resolve('deps', arch, 'install'));

    await compileLibrary('opus', arch, 'https://archive.mozilla.org/pub/opus/opus-1.2.1.tar.gz');
    await compileLibrary('sodium', arch, 'https://github.com/jedisct1/libsodium/releases/download/1.0.16/libsodium-1.0.16.tar.gz');
    await compileLibrary('lame', arch, `https://downloads.sourceforge.net/project/lame/lame/3.100/lame-3.100.tar.gz?ts=${Math.floor(Date.now() / 1000)}`);

    const config = {
      target_defaults: {
        defines: [],
        libraries: [path.resolve('deps', arch, 'install', 'lib', 'libopus.a'), path.resolve('deps', arch, 'install', 'lib', 'libsodium.a'), path.resolve('deps', arch, 'install', 'lib', 'libmp3lame.a')],
        include_dirs: [path.resolve('deps', arch, 'install', 'include')],
      },
    };

    fs.writeFileSync('config.gypi', JSON.stringify(config, undefined, 2));

    for (const target of targets) {
      const id = abi.getAbi(target, 'node');

      execSync(`${path.resolve('node_modules', '.bin', 'node-gyp')} rebuild --target=v${target} --arch=${arch}`, {
        stdio: 'inherit',
        env: {
          ...process.env,
          ...env[arch],
        },
      });

      fs.copyFileSync(path.resolve('build', 'Release', 'krypton.node'), path.resolve('builds', `${process.platform}-${arch}-${id}.node`));
    }
  }
}

async function windows() {
  for (const arch of arches[process.platform]) {
    const config = {
      target_defaults: {
        defines: ['KRYPTON_DISABLE_LAME', 'SODIUM_STATIC'],
      },
    };

    if (arch === 'ia32') config.target_defaults.defines.push('KRYPTON_DISABLE_SODIUM');

    fs.writeFileSync('config.gypi', JSON.stringify(config, undefined, 2));

    for (const target of targets) {
      const id = abi.getAbi(target, 'node');

      execSync(`${path.resolve('node_modules', '.bin', 'node-gyp')} configure --target=v${target} --arch=${arch}`, {
        stdio: 'inherit',
        env: {
          ...process.env,
          ...env[arch],
        },
      });

      const vcxproj = xml.xml2js(fs.readFileSync(path.resolve('build', 'krypton.vcxproj'), 'utf8'));

      const globals = vcxproj.elements.find(x => x.name === 'Project').elements.find(x => x.name === 'PropertyGroup' && x.attributes.Label === 'Globals');

      globals.elements.push({
        type: 'element',
        name: 'VcpkgTriplet',
        elements: [{
          type: 'text',
          text: `${arch === 'x64' ? 'x64' : 'x86'}-windows-static`,
        }],
      });

      globals.elements.push({
        type: 'element',
        name: 'VcpkgEnabled',
        elements: [{
          type: 'text',
          text: 'true',
        }],
      });

      fs.writeFileSync(path.resolve('build', 'krypton.vcxproj'), xml.js2xml(vcxproj, { spaces: 2 }));

      execSync(`${path.resolve('node_modules', '.bin', 'node-gyp')} build --target=v${target} --arch=${arch}`, {
        stdio: 'inherit',
        env: {
          ...process.env,
          ...env[arch],
        },
      });

      fs.copyFileSync(path.resolve('build', 'Release', 'krypton.node'), path.resolve('builds', `${process.platform}-${arch}-${id}.node`));
    }
  }
}

if (process.platform === 'win32') {
  windows();
} else {
  linux();
}
