/**
 * Sodium encrypting/decrypting
 * @namespace sodium
 */

const native = require('./native');

/**
 * Arguments for {@link sodium.encrypt}
 * @memberof sodium
 * @typedef {Object} EncryptArguments
 * @property {Buffer} [data] - The input data, may be left out if piping from another NativeFunction
 * @property {Buffer} key - The encryption key
 * @property {Buffer} [nonce] - The nonce, will be generated if not provided
 */

module.exports = {
  /**
   * The version of libsodium, or null if the sodium module is not enabled.
   * @memberof sodium
   * @type {String?}
   */
  version: native.sodiumVersion,

  /**
   * Encrypts with sodium
   * @memberof sodium
   * @type {NativeFunction}
   * @see sodium.EncryptArguments
   * @returns {NativeFunction<Buffer>} - A native function to be run with a {@link Chain}
   */
  encrypt({ data, key, nonce }) {
    return {
      name: 'Sodium::Encrypt',
      args: {
        data,
        key,
        nonce,
      },
    };
  },
};
