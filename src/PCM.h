#ifndef KRYPTON_SRC_PCM_H_
#define KRYPTON_SRC_PCM_H_

#include "main.h"

#include <cstdint>

namespace PCM {

typedef struct Volume16Args {
  int16_t *data;
  double volume;
  int length;
} Volume16Args;

Pipe *Volume16(void *);

}  // namespace PCM

#endif  // KRYPTON_SRC_PCM_H_
