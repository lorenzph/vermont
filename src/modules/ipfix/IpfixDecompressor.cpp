#include "IpfixDecompressor.h"
#ifdef WITH_DECOMPRESSION
#ifdef WITH_ZLIB
#include <zlib.h>

class DeflateDecompressor : public Decompressor {
public:
	int decompress(const boost::shared_array<uint8_t> source,
				   uint16_t sourceLength,
				   boost::shared_array<uint8_t> &destination,
				   uint16_t &destinationLength);
};

int DeflateDecompressor::decompress(const boost::shared_array<uint8_t> source,
									uint16_t sourceLength,
									boost::shared_array<uint8_t> &destination,
									uint16_t &destinationLength) {
	int ret;
	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	ret = inflateInit2(&strm, -15);

	if (ret != Z_OK)
		return -1;

	boost::shared_array<uint8_t> buffer(new uint8_t[MAX_MSG_LEN]);

	strm.avail_in = sourceLength;
	strm.next_in = source.get();
	strm.avail_out = MAX_MSG_LEN;
	strm.next_out = buffer.get();

	ret = inflate(&strm, Z_NO_FLUSH);

	if (ret != Z_STREAM_END) {
		DPRINTF("Failed to inflate packet: %d", ret);
		inflateEnd(&strm);

		return -1;
	}

	destination.swap(buffer);
	destinationLength = MAX_MSG_LEN - strm.avail_out;

	inflateEnd(&strm);

	return 0;
}
#endif

#ifdef WITH_BZIP2
#include <bzlib.h>
class Bzip2Decompressor : public Decompressor {
public:
	int decompress(const boost::shared_array<uint8_t> source,
				   uint16_t sourceLength,
				   boost::shared_array<uint8_t> &destination,
				   uint16_t &destinationLength);

};

int Bzip2Decompressor::decompress(const boost::shared_array<uint8_t> source,
									uint16_t sourceLength,
									boost::shared_array<uint8_t> &destination,
									uint16_t &destinationLength) {
	int ret;
	bz_stream strm;
	strm.bzalloc = NULL;
	strm.bzfree = NULL;
	strm.opaque = NULL;

	ret = BZ2_bzDecompressInit(&strm, 0, 0);

	if (ret != BZ_OK)
		return -1;

	boost::shared_array<uint8_t> buffer(new uint8_t[MAX_MSG_LEN]);

	strm.avail_in = sourceLength;
	strm.next_in = (char *) source.get();
	strm.avail_out = MAX_MSG_LEN;
	strm.next_out = (char *) buffer.get();

	ret = BZ2_bzDecompress(&strm);

	if (ret != BZ_STREAM_END) {
		DPRINTF("Failed to decompress packet: %d", ret);
		BZ2_bzDecompressEnd(&strm);

		return -1;
	}

	destination.swap(buffer);
	destinationLength = MAX_MSG_LEN - strm.avail_out;

	BZ2_bzDecompressEnd(&strm);

	return 0;
}
#endif

#include "../../common/quicklz/quicklz.h"

class QuickLZDecompressor : public Decompressor {
public:
	QuickLZDecompressor();

	int decompress(const boost::shared_array<uint8_t> source,
				   uint16_t sourceLength,
				   boost::shared_array<uint8_t> &destination,
				   uint16_t &destinationLength);
private:
	qlz_state_decompress state;
};

QuickLZDecompressor::QuickLZDecompressor() {
	// Sanity checks to ensure that QuickLZ has been compiled with the correct
	// defines.

        ASSERT(qlz_get_setting(0) == 1, "Unexpected QLZ_COMPRESSION_LEVEL");
	ASSERT(qlz_get_setting(3) == MAX_MSG_LEN, "Unexpected QLZ_STREAMING_BUFFER");
	ASSERT(qlz_get_setting(6) == 1, "Unexpected QLZ_MEMORY_SAFE");
}

int QuickLZDecompressor::decompress(const boost::shared_array<uint8_t> source,
									uint16_t sourceLength,
									boost::shared_array<uint8_t> &destination,
									uint16_t &destinationLength) {
	memset(&state, 0, sizeof(qlz_state_decompress));

	if (sourceLength <= 9) {
		DPRINTF("Need at lest 9 bytes to determine compressed size.");
		return -1;
	}

	size_t len = qlz_size_compressed((const char *) source.get());
	if (len > MAX_MSG_LEN) {
		DPRINTF("Decompressed packet would be larger than MAX_MSG_SIZE - discarding it.");
		return -1;
	}

	boost::shared_array<uint8_t> buffer(new uint8_t[len]);
	size_t decompressedLength =
			qlz_decompress((const char *) source.get(),
						   (char *) buffer.get(), &state);

	if (decompressedLength != len) {
		DPRINTF("Decompressed length != expected decompressed length.");
		return -1;
	}

	destinationLength = len;
	destination.swap(buffer);

	return 0;
}

IpfixDecompressor::IpfixDecompressor(const std::string &algorithm) :
	decompressor(NULL)
{
#ifdef WITH_BZIP2
	if (algorithm == "bzip2")
		decompressor = new Bzip2Decompressor();
#endif
#ifdef WITH_ZLIB
	if (algorithm == "deflate")
		decompressor = new DeflateDecompressor();
#endif
	if (algorithm == "quicklz")
		decompressor = new QuickLZDecompressor();

	if (decompressor == NULL)
		THROWEXCEPTION("Unsupported decompression algorithm: %s", algorithm.c_str());

}

IpfixDecompressor::~IpfixDecompressor() {
	if (decompressor)
		delete decompressor;
}

int IpfixDecompressor::processPacket(boost::shared_array<uint8_t> &message,
									 uint16_t &length,
									 boost::shared_ptr<IpfixRecord::SourceID> sourceId) {
	return decompressor->decompress(message, length, message, length);
}
#endif
