#if defined(__ARM_NEON) && !defined(KRYPTON_DISABLE_NEON)

#pragma message("using NEON volume")

#include "main.h"
#include "pcm.h"

#include <cmath>
#include <cstdint>
#include <cstdlib>

#include <limits>

#include <arm_neon.h>

int32x4_t max;
int32x4_t min;

namespace pcm {

void init() {
  min = vdupq_n_s32((std::numeric_limits<int16_t>::max)());
  max = vdupq_n_s32((std::numeric_limits<int16_t>::min)());
}

Pipe *Volume16(void *req) {
  Args *args = static_cast<Args *>(req);
  Volume16Args *volumeArgs = static_cast<Volume16Args *>(args->args);
  Pipe *ret = new Pipe;

  int length = volumeArgs->length / 2;
  int16_t *data = volumeArgs->data;
  if (args->from == "Pipe") {
    Pipe *pipe = static_cast<Pipe *>(args->pipe);

    length = pipe->length / 2;
    data = reinterpret_cast<int16_t *>(pipe->data);
  }

  ret->length = length * 2;

  if (volumeArgs->volume == 1) {
    ret->data = reinterpret_cast<char *>(data);
  } else {
    int bytes = 2;
    int complete = static_cast<int>(floor(length / bytes)) * bytes;
    int16_t *sdata = static_cast<int16_t *>(malloc(length * sizeof(int16_t)));

    if (volumeArgs->volume == floor(volumeArgs->volume)) {
      int32x4_t vol = vdupq_n_s32(static_cast<int32_t>(volumeArgs->volume));

      for (int i = 0; i < complete; i += 4) {
        int32_t tmp[4];
        tmp[0] = data[i];
        tmp[1] = data[i + 1];
        tmp[2] = data[i + 2];
        tmp[3] = data[i + 3];

        int32x4_t d = vld1q_s32(tmp);

        int32x4_t out = vminq_s32(min, vmaxq_s32(max, vmulq_s32(d, vol)));

        vst1q_s32(tmp, out);

        for (int n = 0; n < 4; n++) {
          sdata[i + n] = static_cast<int16_t>(tmp[n]);
        }
      }
    } else {
      float32x4_t vol = vdupq_n_f32(static_cast<float>(volumeArgs->volume));

      for (int i = 0; i < complete; i += 4) {
        float32_t tmp[4];
        tmp[0] = data[i];
        tmp[1] = data[i + 1];
        tmp[2] = data[i + 2];
        tmp[3] = data[i + 3];

        float32x4_t d = vld1q_f32(tmp);

        int32x4_t out = vminq_s32(min, vmaxq_s32(max, vcvtq_s32_f32(vmulq_f32(d, vol))));

        int32_t out32[4];

        vst1q_s32(out32, out);

        for (int n = 0; n < 4; n++) {
          sdata[i + n] = static_cast<int16_t>(out32[n]);
        }
      }
    }

    ret->data = reinterpret_cast<char *>(sdata);
  }

  if (args->from == "Pipe") {
    Pipe *pipe = static_cast<Pipe *>(args->pipe);

    if (volumeArgs->volume != 1) {
      free(pipe->data);
    }

    delete pipe;
  }

  delete args;
  delete volumeArgs;

  return ret;
}

}  // namespace pcm

#endif  // __ARM_NEON
