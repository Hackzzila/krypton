const abi = require('node-abi');
const path = require('path');
const fs = require('fs');
const { execSync } = require('child_process');

const arches = {
  win32: ['x64', 'ia32'],
  linux: ['x64', 'ia32', 'arm', 'arm64'],
  darwin: ['x64'],
};

const targets = ['6.0.0', '7.0.0', '8.0.0', '9.0.0'];

const env = {
  arm: {
    CC: 'arm-linux-gnueabihf-gcc',
    CXX: 'arm-linux-gnueabihf-g++',
    LD: 'arm-linux-gnueabihf-ld',
  },
  arm64: {
    CC: 'aarch64-linux-gnu-gcc',
    CXX: 'aarch64-linux-gnu-g++',
    LD: 'aarch64-linux-gnu-ld',
  },
  x64: {},
  ia32: {},
};

for (const arch of arches[process.platform]) {
  for (const target of targets) {
    const id = abi.getAbi(target, 'node');

    execSync(`${path.join('node_modules', '.bin', 'node-gyp')} rebuild --target=v${target} --arch=${arch}`, {
      stdio: 'inherit',
      env: {
        ...process.env,
        ...env[arch],
      },
    });

    fs.copyFileSync(path.join('build', 'Release', 'krypton.node'), path.join('builds', `${process.platform}-${arch}-${id}.node`));
  }
}
