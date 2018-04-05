#ifndef KRYPTON_SRC_OPUS_H_
#define KRYPTON_SRC_OPUS_H_

#ifndef KRYPTON_DISABLE_OPUS

#include "main.h"

#include <opus/opus.h>

#include <nan.h>
#include <node.h>
#include <v8.h>

namespace Opus {

typedef struct EncodeArgs {
  OpusEncoder *encoder;
  int length;
  int channels;
  const opus_int16 *pcm;
} EncodeArgs;

typedef struct DecodeArgs {
  OpusDecoder *decoder;
  int length;
  int channels;
  unsigned char *data;
} DecodeArgs;

Pipe *Encode(void *);
Pipe *Decode(void *);

class KryptonOpusEncoder : public node::ObjectWrap {
 protected:
  int EnsureEncoder();
  int EnsureDecoder();

 public:
  OpusEncoder* encoder;
  OpusDecoder* decoder;

  opus_int32 rate;
  int channels;
  int application;

  KryptonOpusEncoder(opus_int32 rate, int channels, int application);

  ~KryptonOpusEncoder();

  static void Encode(const Nan::FunctionCallbackInfo<v8::Value> &info);
  static void Decode(const Nan::FunctionCallbackInfo<v8::Value> &info);

  static void ApplyEncoderCTL(const Nan::FunctionCallbackInfo<v8::Value> &info);
  static void ApplyDecoderCTL(const Nan::FunctionCallbackInfo<v8::Value> &info);

  static void SetBitrate(const Nan::FunctionCallbackInfo<v8::Value> &info);
  static void GetBitrate(const Nan::FunctionCallbackInfo<v8::Value> &info);

  static void New(const Nan::FunctionCallbackInfo<v8::Value> &info);
  static void Init(v8::Local<v8::Object> exports);
};

}  // namespace Opus

#else  // KRYPTON_DISABLE_OPUS

#include "main.h"

#include <nan.h>
#include <node.h>
#include <v8.h>

namespace Opus {

typedef struct EncodeArgs {
  void *encoder;
  int length;
  int channels;
  const void *pcm;
} EncodeArgs;

typedef struct DecodeArgs {
  void *decoder;
  int length;
  int channels;
  unsigned char *data;
} DecodeArgs;

inline Pipe *Encode(void *) {
  Nan::ThrowError("opus support is not enabled");
  return nullptr;
}

inline Pipe *Decode(void *) {
  Nan::ThrowError("opus support is not enabled");
  return nullptr;
}

class KryptonOpusEncoder : public node::ObjectWrap {
 public:
  void* encoder;
  void* decoder;

  int rate;
  int channels;
  int application;

  static inline void Dummy(const Nan::FunctionCallbackInfo< v8::Value >& info) {
    Nan::ThrowError("opus support is not enabled");
  }

  static inline void New(const Nan::FunctionCallbackInfo< v8::Value >& info) {
    if (!info.IsConstructCall()) {
      return Nan::ThrowTypeError("Use the new operator to construct the OpusEncoder.");
    }

    Nan::ThrowError("opus support is not enabled");
  }

  static inline void Init(v8::Local<v8::Object> exports) {
    Nan::HandleScope scope;

    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("OpusEncoder").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetPrototypeMethod(tpl, "encode", Dummy);
    Nan::SetPrototypeMethod(tpl, "decode", Dummy);
    Nan::SetPrototypeMethod(tpl, "applyEncoderCTL", Dummy);
    Nan::SetPrototypeMethod(tpl, "applyDecoderCTL", Dummy);
    Nan::SetPrototypeMethod(tpl, "setBitrate", Dummy);
    Nan::SetPrototypeMethod(tpl, "getBitrate", Dummy);

    exports->Set(Nan::New("OpusEncoder").ToLocalChecked(), tpl->GetFunction());
  }
};


}  // namespace Opus

#endif  // KRYPTON_DISABLE_OPUS

#endif  // KRYPTON_SRC_OPUS_H_
