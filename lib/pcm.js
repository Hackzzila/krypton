/**
 * PCM utilities
 * @namespace pcm
 */

module.exports = {
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
