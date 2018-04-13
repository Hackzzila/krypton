const { Transform } = require('stream');
const native = require('./native');
const lame = require('./lame');
const opus = require('./opus');
const pcm = require('./pcm');
const sodium = require('./sodium');

/**
 * A C++ function that can be passed into {@link do}
 * @typedef NativeFunction
 */

/**
 * A chain of functions
 */
class Chain extends Transform {
  /**
   * Creates a chain
   * @hideconstructor
   */
  constructor() {
    super();

    this.functions = [];
    this.buffer = Buffer.alloc(0);
  }

  /**
   * Queues a {@link NativeFunction} to be ran.
   * @param {NativeFunction<T>} func - The function
   * @returns {Chain<T>} - The Chain
   */
  do(func) {
    const chain = new Chain();

    if (func.chunker === true && this.functions.length === 0) {
      chain.chunker = func;
    } else {
      chain.functions = this.functions.concat(func);
    }

    return chain;
  }

  /**
   * Runs the chain of functions
   * @param {bool} [async = true] - Whether or not to run asynchronously in a thread
   * @returns {Promise<T>|T} - The unwrap>ed return value of the last function in the chain
   */
  run(async, data) {
    if (async === undefined) {
      if (module.exports.count === 1) {
        return Promise.resolve(native.run(this.functions, false, data));
      }

      return native.run(this.functions, true, data);
    }

    return native.run(this.functions, async, data);
  }

  async _transform(chunk, encoding, callback) {
    if (this.chunker) {
      if (this.chunker.name === 'Chunker::Opus') {
        this.buffer = Buffer.concat([this.buffer, chunk]);
        const required = this.chunker.args.frameSize * this.chunker.args.channels * 2;
        // console.log(required);
        let n = 0;
        while (this.buffer.length >= required * (n + 1)) {
          this.push(await this.run(undefined, this.buffer.slice(n * required, (n + 1) * required)));
          n += 1;
        }
        if (n > 0) this.buffer = this.buffer.slice(n * required);
        return callback();
      }
    } else {
      return callback(null, await this.run(undefined, chunk));
    }

    return null;
  }
}

module.exports = {
  lame,
  opus,
  pcm,
  sodium,

  /**
   * The number of operations running in parallel, you must increase and decrease this yourself.
   * @type {Number}
   */
  count: 0,

  chunker: {
    opus(channels, frameSize) {
      return {
        chunker: true,
        name: 'Chunker::Opus',
        args: {
          channels,
          frameSize,
        },
      };
    },
  },

  /**
   * Queues a {@link NativeFunction} to be ran.
   * @function do
   * @param {NativeFunction<T>} func - The function
   * @returns {Chain<T>} - The Chain
   */
  do(func) {
    return new Chain().do(func);
  },
};
