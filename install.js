const fs = require('fs');
const url = require('url');
const https = require('https');
const readline = require('readline');
const { version } = require('./package.json');
const { spawn } = require('child_process');

function build() {
  console.log('Download failed, building...');
  spawn('npm', ['run', 'build'], { stdio: 'inherit' });
}

function download(res, asset) {
  console.log(`Downloading binary from ${asset.browser_download_url}`);

  let last;
  let complete = 0;
  const total = res.headers['content-length'];

  res.on('data', (chunk) => {
    complete += chunk.length;
    const progress = Math.round((complete / total) * 20);

    if (progress !== last) {
      readline.cursorTo(process.stdout, 0, null);

      process.stdout.write(`Downloading binary: [${'='.repeat(progress)}${[' '.repeat(20 - progress)]}] ${Math.round((complete / total) * 100)}%`);

      last = progress;
    }
  });

  const write = fs.createWriteStream('krypton.node');

  write.on('finish', () => {
    console.log('\nDownload complete');
  });

  res.pipe(write);
}


https.get({
  headers: {
    'User-Agent': 'krypton-install',
  },
  ...url.parse(`https://api.github.com/repos/hackzzila/krypton/releases/tags/v${version}`),
}, (res) => {
  if (res.statusCode !== 200) {
    console.log(`Download failed: ${res.statusCode}`);
    build();
    return;
  }

  res.setEncoding('utf8');
  let data = '';
  res.on('data', (chunk) => { data += chunk; });
  res.on('end', () => {
    const json = JSON.parse(data);

    const asset = json.assets.find(x => x.name === `${process.platform}-${process.arch}-${process.versions.modules}.node`);

    if (!asset) {
      console.log('Could not find a binary for your system. If you would like support for your system, please create an issue at https://github.com/Hackzzila/krypton/issues');
      build();
      return;
    }

    https.get({
      headers: {
        'User-Agent': 'krypton-install',
        Accept: 'application/octet-stream',
      },
      ...url.parse(asset.url),
    }, (res2) => {
      if (res2.statusCode === 200) {
        download(res2, asset);
      } else if (res2.statusCode === 302) {
        https.get({
          headers: {
            'User-Agent': 'krypton-install',
            Accept: 'application/octet-stream',
          },
          ...url.parse(res2.headers.location),
        }, (res3) => {
          if (res3.statusCode !== 200) {
            console.log(`Download failed: ${res3.statusCode}`);
            build();
            return;
          }

          download(res3, asset);
        });
      } else {
        console.log(`Download failed: ${res2.statusCode}`);
        build();
      }
    });
  });
});
