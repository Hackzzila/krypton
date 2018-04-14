/**
 * Opus encoding/decoding
 * @namespace lame
 */

const native = require('./native');

/**
 * @class lame.LameEncoder
 * @memberof lame
 * @classdesc An Opus Encoder and Decoder
 * @constructor
 * @param {Number} rate - The sample rate
 * @param {Number} channels - The number of channels
 */

/**
 * @memberof lame.LameEncoder
 * @method encode
 * @param {Buffer<PCM>} data - The input pcm data, may be null if piping from another NativeFunction
 * @returns {NativeFunction<Buffer<Opus>>} - A native function to be run with a {@link Chain}
 * @instance
 */

module.exports = {
  /**
   * The version of libmp3lame, or null if the lame module is not enabled.
   * @memberof lame
   * @type {String?}
   */
  version: native.lameVersion,

  LameEncoder: native.LameEncoder,
};
