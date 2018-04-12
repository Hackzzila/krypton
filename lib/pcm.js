const native = require('./native');

/**
 * PCM utilities
 * @namespace pcm
 */

module.exports = {
  /**
   * The intrinsics used for the volume transform. May be `AVX2`, `AVX-512`, `NEON`, or null.
   * @memberof pcm
   * @type {String?}
   */
  simd: native.volume,

  /**
   * Reduces/increases the volume
   * @memberof pcm
   * @param {Buffer<PCM>} data - Input PCM data
   * @param {Number} [volume = 1] - Volume level, if set to 1 this will act as a passthrough
   * @returns {NativeFunction<Buffer<PCM>>} - A native function to be run with a {@link Chain}
   */
  volume16(data, volume = 1) {
    return {
      name: 'PCM::Volume16',
      args: {
        data,
        volume,
      },
    };
  },
};
