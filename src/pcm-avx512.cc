#ifdef __AVX512F__

#pragma message("using AVX-512 volume")

#include "main.h"
#include "pcm.h"

#include <cmath>
#include <cstdint>
#include <cstdlib>

#include <limits>

#include <immintrin.h>

__m512i max;
__m512i min;

namespace pcm {

void init() {
  min = _mm512_set1_epi32((std::numeric_limits<int16_t>::max)());
  max = _mm512_set1_epi32((std::numeric_limits<int16_t>::min)());

  _mm_setcsr(32675);
}

Pipe *Volume16(void *req) {
  Args *args = static_cast<Args *>(req);
  Volume16Args *volumeArgs = static_cast<Volume16Args *>(args->args);
  Pipe *ret = new Pipe;

  int length = volumeArgs->length / 2;
  int16_t *data = volumeArgs->data;
  if (args->pipe) {
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
      __m512i vol = _mm512_set1_epi32(static_cast<int32_t>(volumeArgs->volume));

      for (int i = 0; i < complete; i += 16) {
        __m512i d = _mm512_setr_epi32(data[i], data[i + 1], data[i + 2], data[i + 3], data[i + 4], data[i + 5], data[i + 6], data[i + 7], data[i + 8], data[i + 9], data[i + 10], data[i + 11], data[i + 12], data[i + 13], data[i + 14], data[i + 15]);

        __m512i out = _mm512_min_epi32(min, _mm512_max_epi32(max, _mm512_mullo_epi32(d, vol)));

        int32_t *out32 = static_cast<int32_t *>(malloc(64));

        _mm512_storeu_si512(reinterpret_cast<void *>(out32), out);

        for (int n = 0; n < 16; n++) {
          sdata[i + n] = static_cast<int16_t>(out32[n]);
        }

        free(out32);
      }
    } else if (volumeArgs->volume < 1) {
      __m512i vol = _mm512_set1_epi16(32768 * volumeArgs->volume);

      for (int i = 0; i < complete; i += 32) {
        __m512i d = _mm512_loadu_si512(reinterpret_cast<__m512i *>(data + i));

        __m512i out = _mm512_mulhrs_epi16(d, vol);

        _mm512_storeu_si512(reinterpret_cast<__m512i *>(sdata + i), out);
      }
    } else {
      __m512 vol = _mm512_set1_ps(static_cast<float>(volumeArgs->volume));

      for (int i = 0; i < complete; i += 16) {
        __m512 d = _mm512_setr_ps(data[i], data[i + 1], data[i + 2], data[i + 3], data[i + 4], data[i + 5], data[i + 6], data[i + 7], data[i + 8], data[i + 9], data[i + 10], data[i + 11], data[i + 12], data[i + 13], data[i + 14], data[i + 15]);

        __m512i out = _mm512_min_epi32(min, _mm512_max_epi32(max, _mm512_cvtps_epi32(_mm512_mul_ps(d, vol))));

        int32_t *out32 = static_cast<int32_t *>(malloc(64));

        _mm512_storeu_si512(reinterpret_cast<void *>(out32), out);

        for (int n = 0; n < 16; n++) {
          sdata[i + n] = static_cast<int16_t>(out32[n]);
        }

        free(out32);
      }
    }

    ret->data = reinterpret_cast<char *>(sdata);
  }

  if (args->pipe) {
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

#endif  // __AVX512F__
