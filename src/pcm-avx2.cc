#if defined(__AVX2__) && !defined(__AVX512F__)

#pragma message("using AVX2 volume")

#include "main.h"
#include "pcm.h"

#include <cmath>
#include <cstdint>
#include <cstdlib>

#include <limits>

#include <immintrin.h>

__m256i max;
__m256i min;

namespace pcm {

void init() {
  min = _mm256_set1_epi32((std::numeric_limits<int16_t>::max)());
  max = _mm256_set1_epi32((std::numeric_limits<int16_t>::min)());

  _mm_setcsr(32675);
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
      __m256i vol = _mm256_set1_epi32(static_cast<int32_t>(volumeArgs->volume));

      for (int i = 0; i < complete; i += 8) {
        __m256i d = _mm256_setr_epi32(data[i], data[i + 1], data[i + 2], data[i + 3], data[i + 4], data[i + 5], data[i + 6], data[i + 7]);

        __m256i out = _mm256_min_epi32(min, _mm256_max_epi32(max, _mm256_mullo_epi32(d, vol)));

        int32_t out32[8];
        _mm256_storeu_si256(reinterpret_cast<__m256i *>(out32), out);

        for (int n = 0; n < 8; n++) {
          sdata[i + n] = static_cast<int16_t>(out32[n]);
        }
      }
    } else if (volumeArgs->volume < 1) {
      __m256i vol = _mm256_set1_epi16(32768 * volumeArgs->volume);

      for (int i = 0; i < complete; i += 16) {
        __m256i d = _mm256_loadu_si256(reinterpret_cast<__m256i *>(data + i));

		__m256i out = _mm256_mulhrs_epi16(d, vol);

        _mm256_storeu_si256(reinterpret_cast<__m256i *>(sdata + i), out);
      }
    } else {
      __m256 vol = _mm256_set1_ps(static_cast<float>(volumeArgs->volume));

      for (int i = 0; i < complete; i += 8) {
        __m256 d = _mm256_setr_ps(data[i], data[i + 1], data[i + 2], data[i + 3], data[i + 4], data[i + 5], data[i + 6], data[i + 7]);

        __m256i out = _mm256_min_epi32(min, _mm256_max_epi32(max, _mm256_cvtps_epi32(_mm256_mul_ps(d, vol))));

        int32_t out32[8];
        _mm256_storeu_si256(reinterpret_cast<__m256i *>(out32), out);

        for (int n = 0; n < 8; n++) {
          sdata[i + n] = static_cast<int16_t>(out32[n]);
        }
      }
    }

    ret->data = reinterpret_cast<char *>(sdata);

    free(data);
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

#endif  // __AVX2__
