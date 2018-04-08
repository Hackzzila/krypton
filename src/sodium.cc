#ifndef KRYPTON_DISABLE_SODIUM

#include "main.h"
#include "sodium.h"

#include <sodium.h>

#include <stdlib.h>

namespace sodium {
  Pipe *Encrypt(void *req) {
    Args *args = static_cast<Args *>(req);
    EncryptArgs *encryptArgs = static_cast<EncryptArgs *>(args->args);
    Pipe *ret = new Pipe;

    if (encryptArgs->nonce == nullptr) {
      encryptArgs->nonce = static_cast<unsigned char *>(malloc(crypto_secretbox_NONCEBYTES));
      randombytes_buf(encryptArgs->nonce, crypto_secretbox_NONCEBYTES);
    }

    int length = encryptArgs->length;
    unsigned char *data = encryptArgs->data;

    if (args->from == "Pipe") {
      Pipe *pipe = static_cast<Pipe *>(args->pipe);

      length = pipe->length;
      data = reinterpret_cast<unsigned char *>(pipe->data);
    }

    ret->length = crypto_secretbox_MACBYTES + length;
    ret->data = static_cast<char *>(malloc(ret->length));
    crypto_secretbox_easy(reinterpret_cast<unsigned char *>(ret->data), data, length, encryptArgs->nonce, encryptArgs->key);

    if (args->from == "Pipe") {
      Pipe *pipe = static_cast<Pipe *>(args->pipe);

      free(pipe->data);
      delete pipe;
    }

    delete args;
    delete encryptArgs;

    return ret;
  }
}  // namespace sodium

#else  // KRYPTON_DISABLE_SODIUM

#pragma message("sodium support disabled")

#endif  // KRYPTON_DISABLE_SODIUM
