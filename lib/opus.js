/**
 * Opus encoding/decoding
 * @namespace opus
 */

const native = require('./native');

/**
 * @class opus.OpusEncoder
 * @memberof opus
 * @classdesc An Opus Encoder and Decoder
 * @constructor
 * @param {Number} rate - The sample rate
 * @param {Number} channels - The number of channels
 */

/**
 * @memberof opus.OpusEncoder
 * @method encode
 * @param {Buffer<PCM>} [data] - The input pcm data,
 * may be undefined if piping from another NativeFunction
 * @returns {NativeFunction<Buffer<Opus>>} - A native function to be run with a {@link Chain}
 * @instance
 */

/**
 * @memberof opus.OpusEncoder
 * @method decode
 * @param {Buffer<Opus>} [data] - The input opus data,
 * may be undefined if piping from another NativeFunction
 * @returns {NativeFunction<Buffer<PCM>>} - A native function to be run with a {@link Chain}
 * @instance
 */

module.exports = {
  /**
   * The version of libopus, or null if the opus module is not enabled.
   * @memberof opus
   * @type {Number?}
   */
  version: native.opusVersion,

  OpusEncoder: native.OpusEncoder,
};
