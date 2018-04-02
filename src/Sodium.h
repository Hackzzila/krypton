#ifndef KRYPTON_SRC_SODIUM_H_
#define KRYPTON_SRC_SODIUM_H_

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

#endif  // KRYPTON_SRC_SODIUM_H_
