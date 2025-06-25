#pragma once

#if defined(SAPF_AUDIOTOOLBOX)
#include <AudioToolbox/AudioToolbox.h>
#elif defined(SAPF_RTAUDIO_H)
#include SAPF_RTAUDIO_H
#else
#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#endif
#include <cstdint>

#ifdef SAPF_AUDIOTOOLBOX
class AUBuffers {
 public:
  AUBuffers(AudioBufferList *inIoData);
  uint32_t count() const;
  float *data(int channel) const;
  uint32_t size(int channel) const;
  AudioBufferList *ioData;
};
typedef AUBuffers Buffers;
#elif defined(SAPF_RTAUDIO_H)
// class RtBuffers {
//  public:
//   RtBuffers(float *inOut, uint32_t inCount, uint32_t inSize);
//   RtBuffers(uint32_t inCount, uint32_t inSize);
//   uint32_t count() const;
//   float *data(int channel) const;
//   // despite what this may imply, size isn't actually variable per buffer.
//   // This is just for compatibility with AUBuffers
//   uint32_t size(int channel) const;
//   float *out;
//   uint32_t theCount;
//   uint32_t theSize;
// };
// typedef RtBuffers Buffers;

#else
class WasmBuffers {
 public:
  // Constructor that takes a pointer to WASM memory and buffer info
  WasmBuffers(float* inOut, uint32_t inCount, uint32_t inSize)
      : out(inOut), theCount(inCount), theSize(inSize) {}

  // Constructor for output-only buffers
  WasmBuffers(uint32_t inCount, uint32_t inSize)
      : out(nullptr), theCount(inCount), theSize(inSize) {}

  uint32_t count() const { return theCount; }

  float* data(int channel) const {
    return out ? (out + (channel * theSize)) : nullptr;
  }

  uint32_t size(int channel) const { return theSize; }

  emscripten::val getData(int index) const {
    float* ptr = data(index);  // Your original function
    return emscripten::val(emscripten::typed_memory_view(size(index), ptr));
  }

  float* out;
  uint32_t theCount;
  uint32_t theSize;
};

typedef WasmBuffers RtBuffers;

EMSCRIPTEN_BINDINGS(WasmBuffers) {
  emscripten::class_<WasmBuffers>("WasmBuffers")
      .constructor<uint32_t, uint32_t>()
      .constructor<float*, uint32_t, uint32_t>()
      .function("count", &WasmBuffers::count)
      .function("data", &WasmBuffers::getData)
      .function("size", &WasmBuffers::size);
}
#endif