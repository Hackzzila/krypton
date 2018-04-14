#ifndef KRYPTON_DISABLE_OPUS

#include "common.h"
#include "main.h"
#include "opus.h"

#include <opus/opus.h>

#include <nan.h>
#include <node.h>
#include <v8.h>

#include <cstdlib>

namespace opus {

Pipe *Encode(void *req) {
  Args *args = static_cast<Args *>(req);
  EncodeArgs *encodeArgs = static_cast<EncodeArgs *>(args->args);
  Pipe *ret = new Pipe;

  int frameSize = encodeArgs->length / encodeArgs->channels / sizeof(opus_int16);

  ret->data = static_cast<char *>(malloc(MAX_PACKET_SIZE));

  const opus_int16 *pcm = encodeArgs->pcm;
  if (args->pipe) {
    Pipe *pipe = static_cast<Pipe *>(args->pipe);

    pcm = reinterpret_cast<opus_int16 *>(pipe->data);
    frameSize = pipe->length / encodeArgs->channels / sizeof(opus_int16);
  }

  opus_int32 length = opus_encode(encodeArgs->encoder, pcm, frameSize, reinterpret_cast<unsigned char *>(ret->data), MAX_PACKET_SIZE);

  ret->length = static_cast<int>(length);

  if (args->pipe) {
    Pipe *pipe = static_cast<Pipe *>(args->pipe);

    free(pipe->data);
    delete pipe;
  }

  delete args;
  delete encodeArgs;

  return ret;
}

Pipe *Decode(void *req) {
  Args *args = static_cast<Args *>(req);
  DecodeArgs *decodeArgs = static_cast<DecodeArgs *>(args->args);
  Pipe *ret = new Pipe;

  ret->data = static_cast<char *>(malloc(decodeArgs->channels * MAX_FRAME_SIZE * sizeof(opus_int16)));

  unsigned char *data = decodeArgs->data;
  opus_int32 length = decodeArgs->length;
  if (args->pipe) {
    Pipe *pipe = static_cast<Pipe *>(args->pipe);

    data = reinterpret_cast<unsigned char *>(pipe->data);
    length = pipe->length;
  }

  ret->length = opus_decode(decodeArgs->decoder, data, length, reinterpret_cast<opus_int16 *>(ret->data), MAX_FRAME_SIZE, 0) * decodeArgs->channels * sizeof(opus_int16);

  if (args->pipe) {
    Pipe *pipe = static_cast<Pipe *>(args->pipe);

    free(pipe->data);
    delete pipe;
  }

  delete args;
  delete decodeArgs;

  return ret;
}

int KryptonOpusEncoder::EnsureEncoder() {
  if (encoder != nullptr) return 0;
  int error;
  encoder = opus_encoder_create(rate, channels, application, &error);
  return error;
}

int KryptonOpusEncoder::EnsureDecoder() {
  if (decoder != nullptr) return 0;
  int error;
  decoder = opus_decoder_create(rate, channels, &error);
  return error;
}

KryptonOpusEncoder::KryptonOpusEncoder(opus_int32 rate, int channels, int application):
    encoder(nullptr), decoder(nullptr),
  rate(rate), channels(channels), application(application) {
}

KryptonOpusEncoder::~KryptonOpusEncoder() {
  if (encoder != nullptr) opus_encoder_destroy(encoder);
  if (decoder != nullptr) opus_decoder_destroy(decoder);

  encoder = nullptr;
  decoder = nullptr;
}

void KryptonOpusEncoder::Encode(const Nan::FunctionCallbackInfo< v8::Value >& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  // Unwrap the encoder.
  KryptonOpusEncoder* self = ObjectWrap::Unwrap<KryptonOpusEncoder>(info.This());
  if (self->EnsureEncoder() != OPUS_OK) {
    Nan::ThrowError("Could not create encoder. Check the encoder parameters");
    return;
  }

  v8::Local<v8::Object> obj = Nan::New<v8::Object>();
  v8::Local<v8::Object> args = Nan::New<v8::Object>();

  obj->Set(context, v8::String::NewFromUtf8(isolate, "name"), v8::String::NewFromUtf8(isolate, "Opus::Encode"));
  obj->Set(context, v8::String::NewFromUtf8(isolate, "args"), args);

  args->Set(context, v8::String::NewFromUtf8(isolate, "encoder"), info.This());
  args->Set(context, v8::String::NewFromUtf8(isolate, "data"), info[0]);

  info.GetReturnValue().Set(obj);
}

void KryptonOpusEncoder::Decode(const Nan::FunctionCallbackInfo< v8::Value >& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  // Unwrap the encoder.
  KryptonOpusEncoder* self = ObjectWrap::Unwrap<KryptonOpusEncoder>(info.This());
  if (self->EnsureDecoder() != OPUS_OK) {
    Nan::ThrowError("Could not create decoder. Check the decoder parameters");
    return;
  }

  v8::Local<v8::Object> obj = Nan::New<v8::Object>();
  v8::Local<v8::Object> args = Nan::New<v8::Object>();

  obj->Set(context, v8::String::NewFromUtf8(isolate, "name"), v8::String::NewFromUtf8(isolate, "Opus::Decode"));
  obj->Set(context, v8::String::NewFromUtf8(isolate, "args"), args);

  args->Set(context, v8::String::NewFromUtf8(isolate, "decoder"), info.This());
  args->Set(context, v8::String::NewFromUtf8(isolate, "data"), info[0]);

  info.GetReturnValue().Set(obj);
}

void KryptonOpusEncoder::ApplyEncoderCTL(const Nan::FunctionCallbackInfo< v8::Value >&info) {
  REQ_INT_ARG(0, ctl);
  REQ_INT_ARG(1, value);

  KryptonOpusEncoder* self = ObjectWrap::Unwrap<KryptonOpusEncoder>(info.This());
  if (self->EnsureEncoder() != OPUS_OK) {
    Nan::ThrowError("Could not create encoder. Check the encoder parameters");
    return;
  }

  if (opus_encoder_ctl(self->encoder, ctl, value) != OPUS_OK) {
    return Nan::ThrowError("Invalid ctl/value");
  }
}

void KryptonOpusEncoder::ApplyDecoderCTL(const Nan::FunctionCallbackInfo< v8::Value >&info) {
  REQ_INT_ARG(0, ctl);
  REQ_INT_ARG(1, value);

  KryptonOpusEncoder* self = ObjectWrap::Unwrap<KryptonOpusEncoder>(info.This());
  if (self->EnsureDecoder() != OPUS_OK) {
    Nan::ThrowError("Could not create decoder. Check the decoder parameters");
    return;
  }

  if (opus_decoder_ctl(self->decoder, ctl, value) != OPUS_OK) {
    return Nan::ThrowError("Invalid ctl/value");
  }
}

void KryptonOpusEncoder::SetBitrate(const Nan::FunctionCallbackInfo< v8::Value >& info) {
  REQ_INT_ARG(0, bitrate);

  KryptonOpusEncoder* self = ObjectWrap::Unwrap<KryptonOpusEncoder>(info.This());
  if (self->EnsureEncoder() != OPUS_OK) {
    Nan::ThrowError("Could not create encoder. Check the encoder parameters");
    return;
  }

  if (opus_encoder_ctl(self->encoder, OPUS_SET_BITRATE(bitrate)) != OPUS_OK) {
    return Nan::ThrowError("Invalid bitrate");
  }
}

void KryptonOpusEncoder::GetBitrate(const Nan::FunctionCallbackInfo< v8::Value >& info) {
  KryptonOpusEncoder* self = ObjectWrap::Unwrap<KryptonOpusEncoder>(info.This());
  if (self->EnsureEncoder() != OPUS_OK) {
    Nan::ThrowError("Could not create encoder. Check the encoder parameters");
    return;
  }

  opus_int32 bitrate;
  opus_encoder_ctl(self->encoder, OPUS_GET_BITRATE(&bitrate));

  info.GetReturnValue().Set(Nan::New<v8::Integer>(bitrate));
}

void KryptonOpusEncoder::New(const Nan::FunctionCallbackInfo< v8::Value >& info) {
  if (!info.IsConstructCall()) {
    return Nan::ThrowTypeError("Use the new operator to construct the OpusEncoder.");
  }

  OPT_INT_ARG(0, rate, 48000);
  OPT_INT_ARG(1, channels, 1);
  OPT_INT_ARG(2, application, OPUS_APPLICATION_AUDIO);

  KryptonOpusEncoder* encoder = new KryptonOpusEncoder(rate, channels, application);

  encoder->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

void KryptonOpusEncoder::Init(v8::Local<v8::Object> exports) {
  Nan::HandleScope scope;

  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("OpusEncoder").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "encode", Encode);
  Nan::SetPrototypeMethod(tpl, "decode", Decode);
  Nan::SetPrototypeMethod(tpl, "applyEncoderCTL", ApplyEncoderCTL);
  Nan::SetPrototypeMethod(tpl, "applyDecoderCTL", ApplyDecoderCTL);
  Nan::SetPrototypeMethod(tpl, "setBitrate", SetBitrate);
  Nan::SetPrototypeMethod(tpl, "getBitrate", GetBitrate);

  exports->Set(Nan::New("OpusEncoder").ToLocalChecked(), tpl->GetFunction());
}

}  // namespace opus

#else  // KRYPTON_DISABLE_OPUS

#pragma message("opus support disabled")

#endif  // KRYPTON_DISABLE_OPUS
