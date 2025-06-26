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

#include "Play.hpp"

#include <emscripten/webaudio.h>
#include <pthread.h>

#include <atomic>
#include <thread>

#include "Buffers.hpp"
#include "VM.hpp"

class Player;
static bool fillBufferList(Player* player, int inNumberFrames,
                           Buffers* buffers);

bool inputCallback(int numInputs, const AudioSampleFrame* inputs,
                   int numOutputs, AudioSampleFrame* outputs, int numParams,
                   const AudioParamFrame* params, void* userData);

struct WAPlayerBackend {
  WAPlayerBackend(int inNumChannels, EMSCRIPTEN_WEBAUDIO_T ctx);
  ~WAPlayerBackend();
  int32_t createGraph();
  void stop();
  void* player;
  int numChannels;
  EMSCRIPTEN_WEBAUDIO_T context;
};

typedef WAPlayerBackend PlayerBackend;

const int kMaxChannels = 32;
struct Player {
  Player(const Thread& inThread, int numChannels, EMSCRIPTEN_WEBAUDIO_T ctx);
  ~Player();
  int numChannels();
  int32_t createGraph();
  void stop();
  Thread th;
  int count;  // unused?
  bool done;
  Player* prev;
  Player* next;
  PlayerBackend backend;
  ZIn in[kMaxChannels];
};

WAPlayerBackend::WAPlayerBackend(int inNumChannels, EMSCRIPTEN_WEBAUDIO_T ctx)
    : player(nullptr), numChannels(inNumChannels), context(ctx) {}

WAPlayerBackend::~WAPlayerBackend() {}

bool inputCallback(int numInputs, const AudioSampleFrame* inputs,
                   int numOutputs, AudioSampleFrame* outputs, int numParams,
                   const AudioParamFrame* params, void* userData) {
  int numChannels = outputs[0].numberOfChannels;
  int numFrames = outputs[0].samplesPerChannel;
  WasmBuffers buffers(outputs[0].data, numChannels, numFrames);
  Player* player = static_cast<Player*>(userData);
  assert(player != nullptr);  // should not throw now

  // for (int ch = 0; ch < numChannels; ++ch) {
  //   float* buf = buffers.data(ch);
  //   for (int s = 0; s < numFrames; ++s) {
  //     buf[s] = emscripten_random() * 0.2 - 0.1;  // white noise
  //   }
  // }

  fillBufferList(player, numFrames, &buffers);
  for (int ch = 0; ch < numChannels; ++ch) {
    float* buf = buffers.data(ch);
    for (int s = 0; s < numFrames; ++s) {
      outputs[0].data[s * numChannels + ch] = buf[s];
    }
  }
  return true;
}

int32_t WAPlayerBackend::createGraph() {
  post("create DSP graph!\n");
  int outputChannelCounts[1] = {2};
  EmscriptenAudioWorkletNodeCreateOptions options = {
      .numberOfInputs = 0,
      .numberOfOutputs = 1,
      .outputChannelCounts = outputChannelCounts};
  auto inputProc = inputCallback;
  auto inputProcRefCon = this->player;
  EMSCRIPTEN_AUDIO_WORKLET_NODE_T wasmAudioWorklet =
      emscripten_create_wasm_audio_worklet_node(
          context, "sapf-processor", &options, &inputCallback, inputProcRefCon);
  emscripten_audio_node_connect(wasmAudioWorklet, context, 0, 0);
  return 1;
}

void WAPlayerBackend::stop() {}

struct Player* gAllPlayers = nullptr;

Player::Player(const Thread& inThread, int numChannels,
               EMSCRIPTEN_WEBAUDIO_T ctx)
    : th(inThread),
      count(0),
      done(false),
      prev(nullptr),
      next(gAllPlayers),
      backend(numChannels, ctx) {
  gAllPlayers = this;
  if (next) next->prev = this;
  backend.player = this;
}

Player::~Player() {
  if (next) next->prev = prev;
  if (prev)
    prev->next = next;
  else
    gAllPlayers = next;
}

int Player::numChannels() { return this->backend.numChannels; }
int32_t Player::createGraph() { return this->backend.createGraph(); }
void Player::stop() { this->backend.stop(); }

pthread_mutex_t gPlayerMutex = PTHREAD_MUTEX_INITIALIZER;

static void stopPlayer(Player* player) {
  player->stop();
  delete player;
}

void stopPlaying() {}
void stopPlayingIfDone() {}

static bool fillBufferList(Player* player, int inNumberFrames,
                           Buffers* buffers) {
  if (player->done) {
  zeroAll:
    for (int i = 0; i < (int)buffers->count(); ++i) {
      memset(buffers->data(i), 0, inNumberFrames * sizeof(float));
    }
    return true;
  }
  ZIn* in = player->in;
  bool done = true;
  for (int i = 0; i < (int)buffers->count(); ++i) {
    int n = inNumberFrames;
    if (i >= player->numChannels()) {
      memset(buffers->data(i), 0, buffers->size(i));
    } else {
      try {
        float* buf = buffers->data(i);
        bool imdone = in[i].fill(player->th, n, buf, 1);
        if (n < inNumberFrames) {
          memset(buffers->data(i) + n, 0, (inNumberFrames - n) * sizeof(float));
        }
        done = done && imdone;
      } catch (int err) {
        if (err <= -1000 && err > -1000 - kNumErrors) {
          post("\nerror: %s\n", errString[-1000 - err]);
        } else {
          post("\nerror: %d\n", err);
        }
        post("exception in real time. stopping player.\n");
        done = true;
        goto zeroAll;
      } catch (std::bad_alloc& xerr) {
        post("\nnot enough memory\n");
        post("exception in real time. stopping player.\n");
        done = true;
        goto zeroAll;
      } catch (...) {
        post("\nunknown error\n");
        post("exception in real time. stopping player.\n");
        done = true;
        goto zeroAll;
      }
    }
  }
  return done;
}

bool gWatchdogRunning = false;
pthread_t watchdog;

static void* stopDonePlayers(void* x) {
  using namespace std::chrono_literals;
  while (1) {
    std::this_thread::sleep_for(1s);
    stopPlayingIfDone();
  }
}

void playWithPlayer(Thread& th, V& v) {
  if (!v.isList()) wrongType("play : s", "List", v);
  extern EMSCRIPTEN_WEBAUDIO_T context;
  Player* player;
  if (v.isZList()) {
    player = new Player(th, 1, context);
    player->in[0].set(v);
  } else {
    if (!v.isFinite()) indefiniteOp("play : s", "");
    P<List> s = (List*)v.o();
    s = s->pack(th, kMaxChannels);
    if (!s()) {
      post("Too many channels. Max is %d.\n", kMaxChannels);
      return;
    }
    Array* a = s->mArray();
    int asize = (int)a->size();
    player = new Player(th, asize, context);
    for (int i = 0; i < asize; ++i) {
      player->in[i].set(a->at(i));
    }
    s = nullptr;
    a = nullptr;
  }
  v.o = nullptr;  // try to prevent leak.
  player->createGraph();
}
