#include "common.h"
#include "main.h"
#include "Lame.h"
#include "Opus.h"
#include "PCM.h"
#include "Sodium.h"

#include <opus/opus.h>
#include <lame/lame.h>
#include <sodium.h>

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

    if (i == thread->functions.size() - 1) {
      args->last = true;
    }

    if (pipe != nullptr) {
      args->from = "Pipe";
      args->pipe = pipe;
    }

    Pipe *ret = nullptr;
    if (function.name == "Opus::Encode") {
      ret = Opus::Encode(static_cast<void *>(args));
    } else if (function.name == "Opus::Decode") {
      ret = Opus::Decode(static_cast<void *>(args));
    } else if (function.name == "PCM::Volume16") {
      ret = PCM::Volume16(static_cast<void *>(args));
    } else if (function.name == "Sodium::Encrypt") {
      ret = Sodium::Encrypt(static_cast<void *>(args));
    } else if (function.name == "Lame::Decode") {
      ret = Lame::Decode(static_cast<void *>(args));
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
    // Create a new result buffer.
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
      Opus::EncodeArgs *args = new Opus::EncodeArgs;

      Opus::KryptonOpusEncoder *encoder = node::ObjectWrap::Unwrap<Opus::KryptonOpusEncoder>(Nan::To<v8::Object>(oargs->Get(v8::String::NewFromUtf8(isolate, "encoder"))).ToLocalChecked());

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
      Opus::DecodeArgs *args = new Opus::DecodeArgs;

      Opus::KryptonOpusEncoder *decoder = node::ObjectWrap::Unwrap<Opus::KryptonOpusEncoder>(Nan::To<v8::Object>(oargs->Get(v8::String::NewFromUtf8(isolate, "decoder"))).ToLocalChecked());

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
      PCM::Volume16Args *args = new PCM::Volume16Args;

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
      Sodium::EncryptArgs *args = new Sodium::EncryptArgs;

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
      Lame::DecodeArgs *args = new Lame::DecodeArgs;

      Lame::KryptonLameEncoder *decoder = node::ObjectWrap::Unwrap<Lame::KryptonLameEncoder>(Nan::To<v8::Object>(oargs->Get(v8::String::NewFromUtf8(isolate, "decoder"))).ToLocalChecked());

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
      // Create a new result buffer.
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

void OpusVersion(v8::Local<v8::String> property, const Nan::PropertyCallbackInfo<v8::Value>& info) {
  info.GetReturnValue().Set(Nan::New<v8::String>(opus_get_version_string()).ToLocalChecked());
}

void LameVersion(v8::Local<v8::String> property, const Nan::PropertyCallbackInfo<v8::Value>& info) {
  info.GetReturnValue().Set(Nan::New<v8::String>(get_lame_version()).ToLocalChecked());
}

void SodiumVersion(v8::Local<v8::String> property, const Nan::PropertyCallbackInfo<v8::Value>& info) {
  info.GetReturnValue().Set(Nan::New<v8::String>(sodium_version_string()).ToLocalChecked());
}

void NodeInit(v8::Local<v8::Object> exports) {
  if (sodium_init() == -1) {
    Nan::ThrowError("Failed to initialize sodium");
  }

  Lame::KryptonLameEncoder::Init(exports);
  Opus::KryptonOpusEncoder::Init(exports);
  Nan::SetMethod(exports, "run", Run);

  Nan::SetAccessor(exports, Nan::New<v8::String>("opusVersion").ToLocalChecked(), OpusVersion);
  Nan::SetAccessor(exports, Nan::New<v8::String>("lameVersion").ToLocalChecked(), LameVersion);
  Nan::SetAccessor(exports, Nan::New<v8::String>("sodiumVersion").ToLocalChecked(), SodiumVersion);
}

NODE_MODULE(krypton, NodeInit)
