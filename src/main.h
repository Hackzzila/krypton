#ifndef KRYPTON_SRC_MAIN_H_
#define KRYPTON_SRC_MAIN_H_

#include <nan.h>
#include <v8.h>

#include <string.h>

#define FRAME_SIZE 960
#define MAX_FRAME_SIZE 6*960
#define MAX_PACKET_SIZE (3*1276)
#define BITRATE 64000

typedef struct NativeFunction {
  std::string name;
  void *args;
} NativeFunction;

typedef struct Pipe {
  char *data;
  int length;
  std::string error;
} Pipe;

typedef struct ThreadData {
  Nan::Global<v8::Promise::Resolver> promise;
  std::vector<NativeFunction> functions;
  std::string name;
  Pipe *pipe = nullptr;
} ThreadData;

typedef struct Args {
  std::string from;
  void *args;
  void *pipe;
  bool last = false;
} Args;

#endif  // KRYPTON_SRC_MAIN_H_
