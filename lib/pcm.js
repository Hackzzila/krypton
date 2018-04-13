const native = require('./native');

/**
 * PCM utilities
 * @namespace pcm
 */

/**
 * Arguments for {@link pcm.volume16}
 * @memberof pcm
 * @typedef {Object} VolumeArguments
 * @property {Buffer} [data] - The input data, may be left out if piping from another NativeFunction
 * @property {Buffer} [volume=1] - Volume to be used.
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
   * @see pcm.VolumeArguments
   * @returns {NativeFunction<Buffer<PCM>>} - A native function to be run with a {@link Chain}
   */
  volume16({ data, volume } = { volume: 1 }) {
    return {
      name: 'PCM::Volume16',
      args: {
        data,
        volume,
      },
    };
  },
};
