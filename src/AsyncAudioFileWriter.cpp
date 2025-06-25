// #include "AsyncAudioFileWriter.hpp"

// AsyncAudioFileWriter::AsyncAudioFileWriter(int samplerate, int numChannels)
//     : mSampleRate(samplerate),
//       mNumChannels(numChannels),
//       mChunkSize(
//           std::min(maxChunkSize, (size_t)(4096 / numChannels) *
//           numChannels)),
//       mRingWriteBuffer(mChunkSize),
//       mWriteBuffer(mChunkSize),
//       mRunning(true) {
//   mWriterThread = std::thread(&AsyncAudioFileWriter::writeLoop, this);
// }

// AsyncAudioFileWriter::~AsyncAudioFileWriter() { finalize(); }

// void AsyncAudioFileWriter::writeAsync(const std::vector<float>& data,
//                                       size_t frames) {
//   const size_t valuesToWrite = frames * mNumChannels;
//   if (data.size() < valuesToWrite) return;

//   size_t written = 0;
//   while (written < valuesToWrite) {
//     const size_t chunk = std::min(mChunkSize, valuesToWrite - written);

//     std::unique_lock<std::mutex> lock(mBufferMutex);
//     mSpaceAvailableCondition.wait(lock, [this, chunk] {
//       return mRingBuffer.write_available() >= chunk || !mRunning;
//     });

//     if (!mRunning) break;

//     mRingBuffer.write(data.data() + written, chunk);
//     written += chunk;

//     mDataAvailableCondition.notify_one();
//   }
// }

// void AsyncAudioFileWriter::finalize() {
//   if (!mRunning) return;

//   mRunning = false;
//   mDataAvailableCondition.notify_one();

//   if (mWriterThread.joinable()) {
//     mWriterThread.join();
//   }

//   generateAndDownloadWAV();
// }

// void AsyncAudioFileWriter::writeLoop() {
//   while (mRunning || mRingBuffer.read_available() > 0) {
//     std::unique_lock<std::mutex> lock(mBufferMutex);
//     mDataAvailableCondition.wait(lock, [this] {
//       return !mRunning || mRingBuffer.read_available() >= mChunkSize;
//     });

//     size_t available = mRingBuffer.read_available();
//     if (available > 0) {
//       size_t toRead = std::min(available, mChunkSize);
//       mRingBuffer.read(mWriteBuffer.data(), toRead);
//       mSpaceAvailableCondition.notify_one();
//     }
//   }
// }