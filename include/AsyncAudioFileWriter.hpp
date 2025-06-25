// #pragma once

// #include <emscripten.h>
// #include <emscripten/bind.h>

// #include <atomic>
// #include <condition_variable>
// #include <cstring>
// #include <mutex>
// #include <thread>
// #include <vector>

// constexpr size_t ringBufferSize{1024 * 1024};
// constexpr size_t maxChunkSize{4096};

// template <typename T, size_t N>
// class RingBuffer {
//  public:
//   bool write(const T* data, size_t count) {
//     if (write_available() < count) return false;

//     for (size_t i = 0; i < count; ++i) {
//       buffer[(writePos + i) % N] = data[i];
//     }
//     writePos = (writePos + count) % N;
//     return true;
//   }

//   bool read(T* data, size_t count) {
//     if (read_available() < count) return false;

//     for (size_t i = 0; i < count; ++i) {
//       data[i] = buffer[(readPos + i) % N];
//     }
//     readPos = (readPos + count) % N;
//     return true;
//   }

//   size_t write_available() const {
//     if (writePos >= readPos) {
//       return N - writePos + readPos;
//     }
//     return readPos - writePos;
//   }

//   size_t read_available() const {
//     if (writePos >= readPos) {
//       return writePos - readPos;
//     }
//     return N - readPos + writePos;
//   }

//  private:
//   T buffer[N];
//   size_t readPos = 0;
//   size_t writePos = 0;
// };

// class AsyncAudioFileWriter {
//  public:
//   AsyncAudioFileWriter(int samplerate, int numChannels);
//   ~AsyncAudioFileWriter();

//   // Changed to use vector reference instead of raw pointer
//   void writeAsync(const std::vector<float>& data, size_t frames);
//   void finalize();

//  private:
//   void writeLoop();
//   void generateAndDownloadWAV();

//   int mSampleRate;
//   int mNumChannels;
//   size_t mChunkSize;

//   RingBuffer<float, ringBufferSize> mRingBuffer;
//   std::vector<float> mRingWriteBuffer;
//   std::vector<float> mWriteBuffer;

//   std::mutex mBufferMutex;
//   std::condition_variable mDataAvailableCondition;
//   std::condition_variable mSpaceAvailableCondition;

//   std::atomic<bool> mRunning;
//   std::thread mWriterThread;
// };

// // Binding declarations
// EMSCRIPTEN_BINDINGS(AsyncAudioFileWriter) {
//   emscripten::class_<AsyncAudioFileWriter>("AsyncAudioFileWriter")
//       .constructor<int, int>()
//       .function("writeAsync", &AsyncAudioFileWriter::writeAsync)
//       .function("finalize", &AsyncAudioFileWriter::finalize);

//   emscripten::register_vector<float>("FloatVector");
// }