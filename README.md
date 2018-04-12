<div align="center">
  <br />
  <p>
    <img src="https://i.imgur.com/OvtSGky.png" width="300" />
  </p>

[![Build Status](https://travis-ci.org/Hackzzila/krypton.svg?branch=master)](https://travis-ci.org/Hackzzila/krypton)
[![Build Status](https://ci.appveyor.com/api/projects/status/github/Hackzzila/krypton?svg=true)](https://ci.appveyor.com/project/Hackzzila/krypton)
[![dependencies](https://david-dm.org/Hackzzila/krypton/status.svg)](https://david-dm.org/Hackzzila/krypton)
[![devDependencies](https://david-dm.org/Hackzzila/krypton/dev-status.svg)](https://david-dm.org/Hackzzila/krypton?type=dev)
[![npm](https://img.shields.io/npm/dt/krypton.svg)]()

<p><b>Multithreaded audio library for Node.js. Provides behind-the-scenes audio support for <a href="https://discord.js.org">Discord.js</a></b></p>

`npm install krypton`

</div>

------

## Installation

If you are on a machine with AVX-512 support (such as Google Cloud Compute), enable AVX-512 by appending `--krypton-enable-avx-512` when installing with npm.

If you are on an old machine without AVX-2 support, append `--krypton-disable-avx2` when installing with npm.

If you would like to disable Neon on arm64, append `--krypton-disable-neon` when installing with npm.

You can disable any module when installing by appending `--krypton-disable-MODULENAME` (e.g. `--krypton-disable-opus`).

**Note:** Passing `--krypton-` flags will **NOT** work with Yarn.

### Linux

First install opus, sodium, and lame. This can be done on Ubuntu with the following command.
```bash
sudo apt-get install libopus-dev libsodium-dev libmp3lame-dev
```

Then you can install krypton with npm.
```bash
npm i krypton
```

### Windows

First install [vcpkg](https://github.com/Microsoft/vcpkg) and setup user-wide integration.

Then install opus and sodium with vcpkg. If you are on a 32 bit system, remove `:x64-windows`.
```bash
.\vcpkg install opus:x64-windows libsodium:x64-windows
```

Then you can install with npm.
```bash
npm i krypton --krypton-disable-lame
```

Then you can build krypton if needed.
```bash
cd node_modules/krypton
node-gyp rebuild
cd ../..
```

### macOS

First install [Homebrew](https://brew.sh)

Then install opus, sodium, and lame with Homebrew.
```bash
brew install opus libsodium lame
```

Then you can install krypton with npm.
```bash
npm i krypton
```