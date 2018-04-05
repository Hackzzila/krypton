#ifndef KRYPTON_SRC_SODIUM_H_
#define KRYPTON_SRC_SODIUM_H_

#ifndef KRYPTON_DISABLE_SODIUM

#include "main.h"

namespace Sodium {

typedef struct EncryptArgs {
  int length;
  unsigned char *data;
  unsigned char *key;
  unsigned char *nonce = nullptr;
} EncryptArgs;

Pipe *Encrypt(void *);

}  // namespace Sodium

#else  // KRYPTON_DISABLE_SODIUM

#include "main.h"

namespace Sodium {

typedef struct EncryptArgs {
  int length;
  unsigned char *data;
  unsigned char *key;
  unsigned char *nonce = nullptr;
} EncryptArgs;

inline Pipe *Encrypt(void *) {
  Nan::ThrowError("sodium support is not enabled");
  return nullptr;
}

}  // namespace Sodium

#endif  // KRYPTON_DISABLE_SODIUM

#endif  // KRYPTON_SRC_SODIUM_H_
