#pragma once

#ifndef SAPF_AUDIOTOOLBOX
#include "PortableBuffers.hpp"

#include <memory>
#include <vector>
#include <sndfile.h>
#include "AsyncAudioFileWriter.hpp"

class SndfileSoundFile {
public:
	SndfileSoundFile(std::string path, std::unique_ptr<AsyncAudioFileWriter> writer,
		SNDFILE *inSndfile, int inNumChannels);
	~SndfileSoundFile();

	uint32_t numChannels() const;
	int pull(uint32_t *framesRead, PortableBuffers& buffers);
	// write to file synchronously (blocking). Only functions if create was called with async=false
	// bufs is expected to contain only a single buffer with the specified number of channels
	// and the buffer data already interleaved (for wav output), as floats. It should have
	// the exact amount of frames as indicated by numFrames.
	void write(int numFrames, const PortableBuffers& bufs) const;

	// write to file asynchronously (non-blocking). Only functions if create was called with async=true
	// captures the current data in the buffers and submits it to be written asynchronously.
	// will be flushed when this object is destructed.
	void writeAsync(const RtBuffers& buffers, unsigned int nBufferFrames) const;

	static std::unique_ptr<SndfileSoundFile> open(const char *path);
	// async parameter determines whether async writing should be supported (writeAsync) or not (write).
	// they are mutually exclusive.
	static std::unique_ptr<SndfileSoundFile> create(const char *path, int numChannels, double threadSampleRate,
		double fileSampleRate, bool async);
private:
	// file path
	const std::string mPath;

	// for async output (recording)
	const std::unique_ptr<AsyncAudioFileWriter> mWriter;

	// for synchronous input / output
	SNDFILE* const mSndfile;
	// TODO: actually used for output?
	const int mNumChannels;
};
#endif // SAPF_AUDIOTOOLBOX
