BUILD_DIR = build
SRC_DIR = src
INCLUDE_DIR = include
EIGEN_DIR = ./third_party/eigen
XIMD_DIR = ./third_party/xsimd/include
FFTW_DIR = ./third_party/fftw3

# Project files - separate C and C++ sources
C_SOURCES = $(wildcard $(SRC_DIR)/*.c)
CPP_SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
C_OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SOURCES))
CPP_OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(CPP_SOURCES))
OBJECTS = $(C_OBJECTS) $(CPP_OBJECTS)

TARGET = sapf.html

# FFTW
# emconfigure ./configure --disable-fortran --enable-float --prefix=$EM_PATH CFLAGS="-s WASM=1" CXXFLAGS="-s WASM=1"  LDFLAGS="-s WASM=1"
#emmake make -j
# emmake make install
# -I$(FFTW_DIR)

# Compiler flags
EMCC = emcc
CFLAGS = -msimd128 -I$(INCLUDE_DIR) -I./third_party/fftw3/api -I$(XIMD_DIR) -DXTENSOR_USE_XSIMD -I$(EIGEN_DIR) -O3 -flto -DUSE_LIBEDIT=0 -pthread -s USE_PTHREADS=1 -sPTHREAD_POOL_SIZE=2 -sPROXY_TO_PTHREAD=0

WASM_FLAGS =   -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s ASSERTIONS=1 -s EXIT_RUNTIME=0 --preload-file ./sapf-prelude.txt

EXPORTED_FUNCTIONS = -s EXPORTED_FUNCTIONS='["_main"]' -s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]'

AUDIO_FLAGS = -s USE_WEBGPU=0 -s WASM_WORKERS=1 -s AUDIO_WORKLET=1 -s SHARED_MEMORY=1 -s WEBAUDIO_DEBUG=1

EMBIND_FLAGS = -lembind


.PHONY: all clean debug worklet

all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJECTS)
	$(EMCC) -O1 $(CFLAGS) -L./third_party/fftw3/.libs \
  -lfftw3 $(WASM_FLAGS) $(EXPORTED_FUNCTIONS) $(AUDIO_FLAGS) \
    $(EMBIND_FLAGS) \
		$(PRELOAD_FILES) \
		$^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(EMCC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(EMCC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
	emcc --clear-cache

# Debug target to show variables
debug:
	@echo "C_SOURCES: $(C_SOURCES)"
	@echo "CPP_SOURCES: $(CPP_SOURCES)"
	@echo "OBJECTS: $(OBJECTS)"

serve:
	emrun --port 8080 build