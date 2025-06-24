{
  lib,
  stdenv,
  meson,
  ninja,
  doctest,
  eigen,
  fftw,
  libedit,
  libsndfile,
  pkg-config,
  rtaudio_6,
  xsimd,
}:

stdenv.mkDerivation (finalAttrs: {
  pname = "sapf";
  version = "0.0";

  src = ./.;

  buildInputs = [
    eigen
    fftw
    libedit
    libsndfile
    rtaudio_6
    xsimd
  ];

  nativeBuildInputs = [
    doctest
    meson
    ninja
    pkg-config
  ];

  doCheck = true;
  
  meta = {
    homepage = "https://github.com/ahihi/sapf";
    description = "Sound As Pure Form, cross-platform edition";
    license = lib.licenses.gpl3;
    mainProgram = "sapf";
    platforms = lib.platforms.unix;
  };
})
