#ifndef KRYPTON_SRC_LAME_H_
#define KRYPTON_SRC_LAME_H_

#include "main.h"

#include <lame/lame.h>

#include <nan.h>
#include <node.h>
#include <v8.h>

namespace Lame {

typedef struct EncodeArgs {
  lame_global_flags *encoder;
  int length;
  int channels;
  int16_t *data;
} EncodeArgs;

typedef struct DecodeArgs {
  hip_t decoder;
  size_t length;
  int channels;
  unsigned char *data;
} DecodeArgs;

Pipe *Decode(void *);

class KryptonLameEncoder : public node::ObjectWrap {
 public:
  lame_global_flags *encoder;
  hip_t decoder;

  int rate;
  int channels;
  int quality;

  KryptonLameEncoder(int rate, int channels, int quality);

  ~KryptonLameEncoder();

  static void Decode(const Nan::FunctionCallbackInfo<v8::Value> &info);

  static void New(const Nan::FunctionCallbackInfo<v8::Value> &info);
  static void Init(v8::Local<v8::Object> exports);
};

}  // namespace Lame

#endif  // KRYPTON_SRC_LAME_H_
