#ifndef KRYPTON_SRC_OPUS_H_
#define KRYPTON_SRC_OPUS_H_

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

#endif  // KRYPTON_SRC_OPUS_H_
