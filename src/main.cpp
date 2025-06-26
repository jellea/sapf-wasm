//    SAPF - Sound As Pure Form
//    Copyright (C) 2019 James McCartney
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <stdio.h>
;
#include <emscripten/webaudio.h>

uint8_t audioThreadStack[4096];
EMSCRIPTEN_WEBAUDIO_T context;
;
#include "VM.hpp"
// TODO: Is this even needed in this file?
#if USE_LIBEDIT
#include <histedit.h>
#endif
#include <emscripten/bind.h>
#include <sys/stat.h>

#include <algorithm>
#include <complex>

#include "primes.hpp"
#ifdef SAPF_DISPATCH
#include <dispatch/dispatch.h>
#else
#include <thread>
#endif  // SAPF_DISPATCH
#ifdef SAPF_COREFOUNDATION
#include <CoreFoundation/CoreFoundation.h>
#endif  // SAPF_COREFOUNDATION

extern void AddCoreOps();
extern void AddMathOps();
extern void AddStreamOps();
extern void AddLFOps();
extern void AddUGenOps();
extern void AddSetOps();
extern void AddRandomOps();
extern void AddMidiOps();

const char* gVersionString = "0.1.21";

static void replLoop(Thread th) {
  th.repl(stdin, vm.log_file);
  exit(0);
}

static char* inString;

void AudioWorkletProcessorCreated(EMSCRIPTEN_WEBAUDIO_T audioContext,
                                  bool success, void* userData) {
  if (!success)
    return;  // Check browser console in a debug build for detailed errors
             // compile

  Thread th;
  loadFile(th, "./sapf-prelude.txt");

  char* inString = static_cast<char*>(userData);

  post("code eval = %s\n", inString);

  compileSmth(th, inString);
}

void AudioThreadInitialized(EMSCRIPTEN_WEBAUDIO_T audioContext, bool success,
                            void* userData) {
  if (!success)
    return;  // Check browser console in a debug build for detailed errors
  WebAudioWorkletProcessorCreateOptions opts = {
      .name = "sapf-processor",
  };

  emscripten_create_wasm_audio_worklet_processor_async(
      audioContext, &opts, &AudioWorkletProcessorCreated, userData);
}

static void sendToRepl(std::string code) {
  char* strCopy = strdup(code.c_str());

  context = emscripten_create_audio_context(0);

  emscripten_start_wasm_audio_worklet_thread_async(
      context, audioThreadStack, sizeof(audioThreadStack),
      &AudioThreadInitialized, strCopy);
}

static void stopAudio() { context::suspend(); }

int main(int argc, const char* argv[]) {
  post("------------------------------------------------\n");
  post("A tool for the expression of sound as pure form.\n");
  post("------------------------------------------------\n");
  post("--- version %s\n", gVersionString);

  AddCoreOps();
  AddMathOps();
  AddStreamOps();
  AddRandomOps();
  AddUGenOps();
  // AddMidiOps();
  AddSetOps();

  return 0;
}

EMSCRIPTEN_BINDINGS(my_module) {
  emscripten::function("sendToRepl", &sendToRepl);
  emscripten::function("stopAudio", &stopAudio);
}