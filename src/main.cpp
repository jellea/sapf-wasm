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

/* issue:

[These comments are very old and I have not checked if they are still relevant.]

TableData alloc should use new

bugs:

itd should have a tail time. currently the ugen stops as soon as its input,
cutting off the delayed signal.

+ should not stop until both inputs stop?
other additive binops: - avg2 sumsq

no, use a  operator

---

adsrg (gate a d s r --> out) envelope generator with gate.
adsr (dur a d s r --> out) envelope generator with duration.
evgg - (gate levels times curves suspt --> out) envelope generator with gate.
suspt is the index of the sustain level. evg - (dur levels times curves suspt
--> out) envelope generator with duration. suspt is the index of the sustain
level.

blip (freq phase nharm --> out) band limited impulse oscillator.
dsf1 (freq phase nharm lharm hmul --> out) sum of sines oscillator.

formant (freq formfreq bwfreq --> out) formant oscillator

svf (in freq rq --> [lp hp bp bs]) state variable filter.
moogf (in freq rq --> out) moog ladder low pass filter.

*/

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

Thread th;

static void sendToRepl(std::string inString) {
  char* example = "800 0 sinosc .3 * play";

  std::thread replThread([&]() { compileSmth(th, example); });
  replThread.join();
}

EMSCRIPTEN_BINDINGS(my_module) {
  emscripten::function("sendToRepl", &sendToRepl);
}

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

  loadFile(th, "./sapf-prelude.txt");

  return 0;
}
