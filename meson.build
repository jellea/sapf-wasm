project(
  'sapf',
  ['cpp'],
  meson_version : '>=1.1'
)

cpp = meson.get_compiler('cpp')
sources = [
  'src/AudioToolboxBuffers.cpp',
  'src/AudioToolboxSoundFile.cpp',
  'src/CoreOps.cpp',
  'src/DelayUGens.cpp',
  'src/dsp.cpp',
  'src/elapsedTime.cpp',
  'src/ErrorCodes.cpp',
  'src/FilterUGens.cpp',
  'src/main.cpp',
  'src/MathFuns.cpp',
  'src/MathOps.cpp',
  'src/Midi.cpp',
  'src/MultichannelExpansion.cpp',
  'src/Object.cpp',
  'src/Opcode.cpp',
  'src/OscilUGens.cpp',
  'src/Parser.cpp',
  'src/Play.cpp',
  'src/PortableBuffers.cpp',
  'src/primes.cpp',
  'src/RCObj.cpp',
  'src/RandomOps.cpp',
  'src/SetOps.cpp',
  'src/SndfileSoundFile.cpp',
  'src/SoundFiles.cpp',
  'src/Spectrogram.cpp',
  'src/StreamOps.cpp',
  'src/symbol.cpp',
  'src/Types.cpp',
  'src/UGen.cpp',
  'src/VM.cpp',
]
deps = []
cpp_args = ['-std=c++17']
link_args = []

# libedit
if host_machine.system() == 'windows'
    # TODO: Not sure if we need both or if these are valid substitutes
    deps += dependency('readline', required: true)
    deps += dependency('history', required: true)
    cpp_args += ['-DUSE_LIBEDIT=0', '-D_POSIX_THREAD_SAFE_FUNCTIONS']
    # TODO: needed?
    #link_args += ['-lrtaudio']
else
  deps += dependency('libedit', required: true)
    cpp_args += ['-DUSE_LIBEDIT=1']
endif

if get_option('asan')
  cpp_args += ['-fsanitize=address', '-O1', '-fno-omit-frame-pointer', '-g']
  link_args += ['-fsanitize=address']
endif

if get_option('accelerate')
  add_project_arguments('-DSAPF_ACCELERATE', language: 'cpp')
else
  if host_machine.system() == 'windows'
    deps += dependency('fftw3', required: true, version: '>=3')
  else
    deps += dependency('fftw3', required: true, version: '>=3')
  endif

endif

if get_option('apple_lock')
  add_project_arguments('-DSAPF_APPLE_LOCK', language: 'cpp')
endif

if get_option('audiotoolbox')
  add_project_arguments('-DSAPF_AUDIOTOOLBOX', language: 'cpp')
else
  deps += dependency('rtaudio', required: true, version: '>=5.2')
  #TODO: check is not working on windows/mingw
  if not cpp.has_header('RtAudio.h') and cpp.has_header('rtaudio/RtAudio.h')
    add_project_arguments('-DSAPF_RTAUDIO_H=<rtaudio/RtAudio.h>', language: 'cpp')
  endif

  deps += dependency('sndfile', required: true)
endif

if get_option('carbon')
  add_project_arguments('-DSAPF_CARBON', language: 'cpp')
endif

if get_option('cocoa')
  add_project_arguments('-DSAPF_COCOA', language: 'cpp')
  sources += 'src/makeImage.mm'
  add_languages('objcpp', required : true)
else
  sources += 'src/makeImage.cpp'
endif

if get_option('corefoundation')
  add_project_arguments('-DSAPF_COREFOUNDATION', language: 'cpp')
endif

if get_option('coremidi')
  add_project_arguments('-DSAPF_COREMIDI', language: 'cpp')
endif

if get_option('dispatch')
  cpp_args += ['-fblocks']
  link_args += ['-lBlocksRuntime', '-ldispatch']
  add_project_arguments('-DSAPF_DISPATCH', language: 'cpp')
endif

if get_option('mach_time')
  add_project_arguments('-DSAPF_MACH_TIME', language: 'cpp')
endif

if get_option('manta')
  add_project_arguments('-DSAPF_MANTA', language: 'cpp')
endif

executable(
  'sapf',
  sources,
  include_directories: [include_directories('include')],
  dependencies: deps,
  cpp_args : cpp_args,
  link_args : link_args
)
