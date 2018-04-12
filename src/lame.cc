#ifndef KRYPTON_DISABLE_LAME

#include "common.h"
#include "main.h"
#include "lame.h"

#include <lame/lame.h>

#include <nan.h>
#include <node.h>
#include <v8.h>

#include <cstdlib>

namespace lame {

Pipe *Decode(void *req) {
  Args *args = static_cast<Args *>(req);
  DecodeArgs *decodeArgs = static_cast<DecodeArgs *>(args->args);
  Pipe *ret = new Pipe;

  unsigned char *data = decodeArgs->data;
  size_t length = decodeArgs->length;
  if (args->from == "Pipe") {
    Pipe *pipe = static_cast<Pipe *>(args->pipe);

    data = reinterpret_cast<unsigned char *>(pipe->data);
    length = pipe->length;
  }

  uint16_t pcmlen = 576;
  if ((data[1] & 8) >> 3) {
    pcmlen = 1152;
  }

  int16_t *lpcm = static_cast<int16_t *>(malloc(pcmlen * 2 * sizeof(int16_t)));
  int16_t *rpcm = static_cast<int16_t *>(malloc(pcmlen * 2 * sizeof(int16_t)));
  int samples = hip_decode(decodeArgs->decoder, data, length, lpcm, rpcm);

  ret->length = samples * 2 * sizeof(int16_t);
  int16_t *sdata = static_cast<int16_t *>(malloc(ret->length));

  int i = 0;
  for (int s = 0; s < samples; s++) {
    sdata[i++] = lpcm[s];
    sdata[i++] = rpcm[s];
  }

  free(lpcm);
  free(rpcm);

  ret->data = reinterpret_cast<char *>(sdata);

  if (args->from == "Pipe") {
    Pipe *pipe = static_cast<Pipe *>(args->pipe);

    free(pipe->data);
    delete pipe;
  }

  delete args;
  delete decodeArgs;

  return ret;
}

KryptonLameEncoder::KryptonLameEncoder(int rate, int channels, int quality):
    rate(rate), channels(channels), quality(quality), encoder(nullptr) {
  decoder = hip_decode_init();
}

KryptonLameEncoder::~KryptonLameEncoder() {
  hip_decode_exit(decoder);
}

void KryptonLameEncoder::Decode(const Nan::FunctionCallbackInfo< v8::Value >& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  v8::Local<v8::Object> obj = Nan::New<v8::Object>();
  v8::Local<v8::Object> args = Nan::New<v8::Object>();

  obj->Set(context, v8::String::NewFromUtf8(isolate, "name"), v8::String::NewFromUtf8(isolate, "Lame::Decode"));
  obj->Set(context, v8::String::NewFromUtf8(isolate, "args"), args);

  args->Set(context, v8::String::NewFromUtf8(isolate, "decoder"), info.This());
  args->Set(context, v8::String::NewFromUtf8(isolate, "data"), info[0]);

  info.GetReturnValue().Set(obj);
}

void KryptonLameEncoder::New(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  if (!info.IsConstructCall()) {
    return Nan::ThrowTypeError("Use the new operator to construct the LameEncoder.");
  }

  OPT_INT_ARG(0, rate, 48000);
  OPT_INT_ARG(1, channels, 1);
  OPT_INT_ARG(2, quality, 2);

  KryptonLameEncoder* encoder = new KryptonLameEncoder(rate, channels, quality);

  encoder->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

void KryptonLameEncoder::Init(v8::Local<v8::Object> exports) {
  Nan::HandleScope scope;

  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("LameEncoder").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "decode", Decode);

  exports->Set(Nan::New("LameEncoder").ToLocalChecked(), tpl->GetFunction());
}

}  // namespace lame

#else  // KRYPTON_DISABLE_LAME

#pragma message("lame support disabled")

#endif  // KRYPTON_DISABLE_LAME
