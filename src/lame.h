#ifndef KRYPTON_SRC_LAME_H_
#define KRYPTON_SRC_LAME_H_

#ifndef KRYPTON_DISABLE_LAME

#include "main.h"

#include <lame/lame.h>

#include <nan.h>
#include <node.h>
#include <v8.h>

namespace lame {

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

}  // namespace lame

#else  // KRYPTON_DISABLE_LAME

#include "main.h"

#include <nan.h>
#include <node.h>
#include <v8.h>

namespace lame {

typedef struct EncodeArgs {
  void *encoder;
  int length;
  int channels;
  int16_t *data;
} EncodeArgs;

typedef struct DecodeArgs {
  int decoder;
  size_t length;
  int channels;
  unsigned char *data;
} DecodeArgs;

inline Pipe *Decode(void *) {
  Nan::ThrowError("lame support is not enabled");
  return nullptr;
}

class KryptonLameEncoder : public node::ObjectWrap {
 public:
  void *encoder;
  int decoder;

  int rate;
  int channels;
  int quality;

  static inline void Dummy(const Nan::FunctionCallbackInfo< v8::Value >& info) {
    Nan::ThrowError("lame support is not enabled");
  }

  static inline void New(const Nan::FunctionCallbackInfo<v8::Value>& info) {
    if (!info.IsConstructCall()) {
      return Nan::ThrowTypeError("Use the new operator to construct the OpusEncoder.");
    }

    Nan::ThrowError("lame support is not enabled");
  }

  static inline void Init(v8::Local<v8::Object> exports) {
    Nan::HandleScope scope;

    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("LameEncoder").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetPrototypeMethod(tpl, "decode", Dummy);

    exports->Set(Nan::New("LameEncoder").ToLocalChecked(), tpl->GetFunction());
  }
};

}  // namespace lame

#endif  // KRYPTON_DISABLE_LAME

#endif  // KRYPTON_SRC_LAME_H_
