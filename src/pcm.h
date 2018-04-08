#ifndef KRYPTON_SRC_PCM_H_
#define KRYPTON_SRC_PCM_H_

#include "main.h"

#include <cstdint>

namespace pcm {

typedef struct Volume16Args {
  int16_t *data;
  double volume;
  int length;
} Volume16Args;

void init();

Pipe *Volume16(void *);

}  // namespace pcm

#endif  // KRYPTON_SRC_PCM_H_
