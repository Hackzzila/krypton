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
 * @param {Buffer<PCM>} data - The input pcm data, may be null if piping from another NativeFunction
 * @returns {NativeFunction<Buffer<Opus>>} - A native function to be run with a {@link Chain}
 */

module.exports = {
  version: native.opusVersion,
  OpusEncoder: native.OpusEncoder,
};
