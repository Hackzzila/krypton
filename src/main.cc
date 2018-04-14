#include "main.h"
#include "lame.h"
#include "opus.h"
#include "pcm.h"
#include "sodium.h"

#ifndef KRYPTON_DISABLE_OPUS
#include <opus/opus.h>
#else  // KRYPTON_DISABLE_OPUS
typedef void opus_int16;
#endif  // KRYPTON_DISABLE_OPUS

#ifndef KRYPTON_DISABLE_LAME
#include <lame/lame.h>
#endif  // KRYPTON_DISABLE_LAME

#ifndef KRYPTON_DISABLE_SODIUM
#include <sodium.h>
#endif  // KRYPTON_DISABLE_LAME

#include <nan.h>
#include <node.h>
#include <node_buffer.h>
#include <node_object_wrap.h>
#include <v8.h>

#include <cstdint>
#include <cstdlib>
#include <string.h>

void doWork(uv_work_t *req) {
  ThreadData *thread = static_cast<ThreadData *>(req->data);

  std::string name;
  void *pipe = nullptr;

  if (thread->pipe != nullptr) {
    name = "Pipe";
    pipe = static_cast<void *>(thread->pipe);
  }

  for (size_t i = 0; i < thread->functions.size(); i++) {
    NativeFunction function = thread->functions[i];
    Args *args = new Args;
    args->args = function.args;
    args->pipe = pipe;

    Pipe *ret = nullptr;
    if (function.name == "Opus::Encode") {
      ret = opus::Encode(static_cast<void *>(args));
    } else if (function.name == "Opus::Decode") {
      ret = opus::Decode(static_cast<void *>(args));
    } else if (function.name == "PCM::Volume16") {
      ret = pcm::Volume16(static_cast<void *>(args));
    } else if (function.name == "Sodium::Encrypt") {
      ret = sodium::Encrypt(static_cast<void *>(args));
    } else if (function.name == "Lame::Decode") {
      ret = lame::Decode(static_cast<void *>(args));
    }

    thread->name = function.name;
    name = function.name;
    pipe = static_cast<void *>(ret);
    thread->pipe = ret;
  }
}

void workCallback(uv_work_t *req, int status) {
  ThreadData *thread = static_cast<ThreadData *>(req->data);

  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  Nan::HandleScope scope;
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  v8::Local<v8::Promise::Resolver> promise = thread->promise.Get(isolate);

  if (thread->pipe->length > -1) {
    Nan::MaybeLocal<v8::Object> actualBuffer = Nan::CopyBuffer(thread->pipe->data, thread->pipe->length);
    if (!actualBuffer.IsEmpty()) {
      promise->Resolve(context, actualBuffer.ToLocalChecked());
    }
  } else {
    promise->Reject(context, Nan::Error(thread->pipe->error.c_str()));
  }

  thread->promise.Reset();

  free(thread->pipe->data);
  delete thread->pipe;
  delete thread;
  delete req;
}

void Run(const Nan::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  v8::Local<v8::Array> arr = v8::Local<v8::Array>::Cast(args[0]);

  bool threaded = true;
  if (args[1] == v8::False(isolate)) {
    threaded = false;
  }

  ThreadData *thread = new ThreadData;
  if (threaded) {
    v8::Local<v8::Promise::Resolver> resolver = v8::Promise::Resolver::New(context).ToLocalChecked();
    thread->promise = Nan::Global<v8::Promise::Resolver>(resolver);

    args.GetReturnValue().Set(resolver->GetPromise());
  }

  if (args[2]->IsUndefined() == false) {
    Pipe *pipe = new Pipe;

    pipe->length = node::Buffer::Length(args[2]);
    pipe->data = static_cast<char *>(malloc(pipe->length * sizeof(char)));

    memcpy(pipe->data, node::Buffer::Data(args[2]), pipe->length);

    thread->pipe = pipe;
  }

  for (int i = 0; i < static_cast<int>(arr->Length()); i++) {
    v8::Local<v8::Object> obj = v8::Local<v8::Object>::Cast(arr->Get(i));

    v8::String::Utf8Value utf8(v8::Local<v8::String>::Cast(obj->Get(v8::String::NewFromUtf8(isolate, "name"))));
    v8::Local<v8::Object> oargs = v8::Local<v8::Object>::Cast(obj->Get(v8::String::NewFromUtf8(isolate, "args")));

    NativeFunction function;
    std::string name(*utf8);
    function.name = name;

    if (name == "Opus::Encode") {
      opus::EncodeArgs *args = new opus::EncodeArgs;

      opus::KryptonOpusEncoder *encoder = node::ObjectWrap::Unwrap<opus::KryptonOpusEncoder>(Nan::To<v8::Object>(oargs->Get(v8::String::NewFromUtf8(isolate, "encoder"))).ToLocalChecked());

      args->encoder = encoder->encoder;
      args->channels = encoder->channels;

      if (oargs->Get(v8::String::NewFromUtf8(isolate, "data"))->IsUndefined() == false) {
        char* pcmData = node::Buffer::Data(oargs->Get(v8::String::NewFromUtf8(isolate, "data")));
        args->pcm = reinterpret_cast<opus_int16 *>(pcmData);
        args->length = node::Buffer::Length(oargs->Get(v8::String::NewFromUtf8(isolate, "data")));
      }

      function.args = static_cast<void *>(args);

      thread->functions.push_back(function);
    } else if (name == "Opus::Decode") {
      opus::DecodeArgs *args = new opus::DecodeArgs;

      opus::KryptonOpusEncoder *decoder = node::ObjectWrap::Unwrap<opus::KryptonOpusEncoder>(Nan::To<v8::Object>(oargs->Get(v8::String::NewFromUtf8(isolate, "decoder"))).ToLocalChecked());

      args->decoder = decoder->decoder;
      args->channels = decoder->channels;

      if (oargs->Get(v8::String::NewFromUtf8(isolate, "data"))->IsUndefined() == false) {
        char* opusData = node::Buffer::Data(oargs->Get(v8::String::NewFromUtf8(isolate, "data")));
        args->data = reinterpret_cast<unsigned char *>(opusData);
        args->length = node::Buffer::Length(oargs->Get(v8::String::NewFromUtf8(isolate, "data")));
      }

      function.args = static_cast<void *>(args);

      thread->functions.push_back(function);
    } else if (name == "PCM::Volume16") {
      pcm::Volume16Args *args = new pcm::Volume16Args;

      args->volume = v8::Local<v8::Number>::Cast(oargs->Get(v8::String::NewFromUtf8(isolate, "volume")))->Value();

      if (oargs->Get(v8::String::NewFromUtf8(isolate, "data"))->IsUndefined() == false) {
        args->length = node::Buffer::Length(oargs->Get(v8::String::NewFromUtf8(isolate, "data")));

        char* pcmData = node::Buffer::Data(oargs->Get(v8::String::NewFromUtf8(isolate, "data")));
        args->data = static_cast<int16_t *>(malloc(args->length * sizeof(int16_t)));
        memcpy(args->data, reinterpret_cast<int16_t *>(pcmData), args->length);
      }

      function.args = static_cast<void *>(args);

      thread->functions.push_back(function);
    } else if (name == "Sodium::Encrypt") {
      sodium::EncryptArgs *args = new sodium::EncryptArgs;

      if (oargs->Get(v8::String::NewFromUtf8(isolate, "data"))->IsUndefined() == false) {
        args->data = reinterpret_cast<unsigned char *>(node::Buffer::Data(oargs->Get(v8::String::NewFromUtf8(isolate, "data"))));
        args->length = static_cast<int>(node::Buffer::Length(oargs->Get(v8::String::NewFromUtf8(isolate, "data"))));
      }

      if (oargs->Get(v8::String::NewFromUtf8(isolate, "nonce"))->IsUndefined() == false) {
        args->nonce = reinterpret_cast<unsigned char *>(node::Buffer::Data(oargs->Get(v8::String::NewFromUtf8(isolate, "nonce"))));
      }

      args->key = reinterpret_cast<unsigned char *>(node::Buffer::Data(oargs->Get(v8::String::NewFromUtf8(isolate, "key"))));

      function.args = static_cast<void *>(args);

      thread->functions.push_back(function);
    } else if (name == "Lame::Decode") {
      lame::DecodeArgs *args = new lame::DecodeArgs;

      lame::KryptonLameEncoder *decoder = node::ObjectWrap::Unwrap<lame::KryptonLameEncoder>(Nan::To<v8::Object>(oargs->Get(v8::String::NewFromUtf8(isolate, "decoder"))).ToLocalChecked());

      args->decoder = decoder->decoder;
      args->channels = decoder->channels;

      if (oargs->Get(v8::String::NewFromUtf8(isolate, "data"))->IsUndefined() == false) {
        char* opusData = node::Buffer::Data(oargs->Get(v8::String::NewFromUtf8(isolate, "data")));
        args->data = reinterpret_cast<unsigned char*>(opusData);
        args->length = node::Buffer::Length(oargs->Get(v8::String::NewFromUtf8(isolate, "data")));
      }

      function.args = static_cast<void *>(args);

      thread->functions.push_back(function);
    }
  }

  uv_work_t *req = new uv_work_t;
  req->data = static_cast<void *>(thread);

  if (threaded) {
    uv_queue_work(uv_default_loop(), req, doWork, workCallback);
  } else {
    doWork(req);

    if (thread->pipe->length > -1) {
      Nan::MaybeLocal<v8::Object> actualBuffer = Nan::CopyBuffer(thread->pipe->data, thread->pipe->length);
      if (!actualBuffer.IsEmpty()) {
        args.GetReturnValue().Set(actualBuffer.ToLocalChecked());
      }
    } else {
      Nan::ThrowError(thread->pipe->error.c_str());
    }

    free(thread->pipe->data);
    delete thread->pipe;
    delete thread;
    delete req;
  }
}

void NodeInit(v8::Local<v8::Object> exports) {
  #ifndef KRYPTON_DISABLE_SODIUM
  if (sodium_init() == -1) {
    Nan::ThrowError("Failed to initialize sodium");
  }
  #endif  // KRYPTON_DISABLE_SODIUM

  pcm::init();

  lame::KryptonLameEncoder::Init(exports);
  opus::KryptonOpusEncoder::Init(exports);
  Nan::SetMethod(exports, "run", Run);

  #ifndef KRYPTON_DISABLE_OPUS
  Nan::Set(exports, Nan::New<v8::String>("opusVersion").ToLocalChecked(), Nan::New<v8::String>(opus_get_version_string()).ToLocalChecked());
  #else  // KRYPTON_DISABLE_OPUS
  Nan::Set(exports, Nan::New<v8::String>("opusVersion").ToLocalChecked(), Nan::Null());
  #endif  // KRYPTON_DISABLE_OPUS

  #ifndef KRYPTON_DISABLE_LAME
  Nan::Set(exports, Nan::New<v8::String>("lameVersion").ToLocalChecked(), Nan::New<v8::String>(get_lame_version()).ToLocalChecked());
  #else  // KRYPTON_DISABLE_LAME
  Nan::Set(exports, Nan::New<v8::String>("lameVersion").ToLocalChecked(), Nan::Null());
  #endif  // KRYPTON_DISABLE_LAME

  #ifndef KRYPTON_DISABLE_SODIUM
  Nan::Set(exports, Nan::New<v8::String>("sodiumVersion").ToLocalChecked(), Nan::New<v8::String>(sodium_version_string()).ToLocalChecked());
  #else  // KRYPTON_DISABLE_SODIUM
  Nan::Set(exports, Nan::New<v8::String>("sodiumVersion").ToLocalChecked(), Nan::Null());
  #endif  // KRYPTON_DISABLE_SODIUM

  #ifdef __AVX512F__
  Nan::Set(exports, Nan::New<v8::String>("volume").ToLocalChecked(), Nan::New<v8::String>("AVX-512").ToLocalChecked());
  #endif  // __AVX512F__

  #if defined(__AVX2__) && !defined(__AVX512F__)
  Nan::Set(exports, Nan::New<v8::String>("volume").ToLocalChecked(), Nan::New<v8::String>("AVX2").ToLocalChecked());
  #endif

  #if defined(__ARM_NEON) && !defined(KRYPTON_DISABLE_NEON)
  Nan::Set(exports, Nan::New<v8::String>("volume").ToLocalChecked(), Nan::New<v8::String>("NEON").ToLocalChecked());
  #endif

  #if !defined(__AVX2__) && !defined(__AVX512F__) && (!defined(__ARM_NEON) || defined(KRYPTON_DISABLE_NEON))
  Nan::Set(exports, Nan::New<v8::String>("volume").ToLocalChecked(), Nan::Null());
  #endif
}

NODE_MODULE(krypton, NodeInit)
