#ifndef UTILS_CODEC_RTMPCODEC_H

#include <cstdint>
#include <cstring>

constexpr int MOVIE_CSID = 4;

enum RtmpPackFmt
{
	FMT0,
	FMT1,
	FMT2,
	FMT3
};

enum RtmpPackCode : uint8_t
{
	AUDIO = 8,
	VIDEO = 9

};

class RtmpPack
{
public:
	RtmpPack() = default;
	~RtmpPack() = default;

private:

	RtmpPackFmt fmt_;
	uint32_t timestamp_;
	uint32_t msg_length_;
	RtmpPackCode code_;
	uint32_t stream_id_;
};

class RtmpCodec
{
public:

	RtmpCodec() = default;
	~RtmpCodec() = default;

	int Decode(const char* data, size_t length);

private:

};

#endif // !UTILS_CODEC_RTMPCODEC_H
