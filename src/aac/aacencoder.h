/* 
 * File:   aacencoder.h
 * Author: Sergio
 *
 * Created on 20 de junio de 2013, 10:46
 */

#ifndef AACENCODER_H
#define	AACENCODER_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
}
#include "config.h"
#include "codecs.h"
#include "audio.h"

class AACEncoder : public AudioEncoder
{
public:
	AACEncoder(const Properties &properties);
	virtual ~AACEncoder();
	virtual int Encode(SWORD *in,int inLen,BYTE* out,int outLen);
	virtual DWORD TrySetRate(DWORD rate, DWORD numChannels)	{ return GetRate();				}
	virtual DWORD GetRate()					{ return ctx->sample_rate?ctx->sample_rate:8000;}
	virtual DWORD GetClockRate()				{ return GetRate();				}
private:
	AVCodec 	*codec;
	AVCodecContext	*ctx;
	SwrContext *swr;
	AVFrame         *frame;
	BYTE *samples;
	int samplesSize;
	int samplesNum;
};

#endif	/* AACENCODER_H */

