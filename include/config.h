#ifndef _CONFIG_H_
#define _CONFIG_H_
#include <stdint.h>
#include <stdlib.h>
#include <cstddef>  // size_t
#include <cstdint> 
#include <limits>
#include <map>
#include <vector>
#include <sstream>
#include <string>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include "version.h"


#define	QCIF	0	// 176  x 144 	AR:	1,222222222
#define	CIF	1	// 352  x 288	AR:	1,222222222
#define	VGA	2	// 640  x 480	AR:	1,333333333
#define	PAL	3	// 768  x 576	AR:	1,333333333
#define	HVGA	4	// 480  x 320	AR:	1,5
#define	QVGA	5	// 320  x 240	AR:	1,333333333
#define	HD720P	6	// 1280 x 720	AR:	1,777777778
#define	WQVGA	7	// 400  x 240	AR:	1,666666667
#define	W448P	8	// 768  x 448	AR:	1,714285714
#define	SD448P	9	// 576  x 448	AR:	1,285714286
#define	W288P	10	// 512  x 288	AR:	1,777777778
#define	W576	11	// 1024 x 576	AR:	1,777777778
#define	FOURCIF	12	// 704  x 576	AR:	1,222222222
#define	FOURSIF	13	// 704  x 480	AR:	1,466666667
#define	XGA	14	// 1024 x 768	AR:	1,333333333
#define	WVGA	15	// 800  x 480	AR:	1,666666667
#define	DCIF	16	// 528  x 384	AR:	1,375
#define	SIF	17	// 352  x 240	AR:	1,466666667
#define	QSIF	18	// 176  x 120	AR:	1,466666667
#define	SD480P	19	// 480  x 360	AR:	1,333333333
#define	SQCIF	20	// 128  x 96	AR:	1,333333333
#define	SCIF	21	// 256  x 192	AR:	1,333333333
#define	HD1080P	22	// 1920 x 1080  AR:     1,777777778
#define UW720P  23	// 1680 x 720   AR:	2,333333333




#define FD_INVALID	(int)-1
#define MTU		1500u
#define UDPPAYLOADSIZE  1350u
#define RTPPAYLOADSIZE	1200u

#define QWORD		uint64_t
#define DWORD		uint32_t
#define WORD		uint16_t
#define SWORD		int16_t
#define BYTE		uint8_t
#define SBYTE		char
#define ALIGNTO32(x)	(BYTE*)(((((QWORD)x)+31)>>5)<<5);
#define SIZE2MUL(x)	((x)&0xFFFFFFFE)
#define SIZE4MUL(x)	((x)&0xFFFFFFFC)
#define MAXKBITS 	300

#if __APPLE__
#define ALIGNEDTO32
#define ZEROALIGNEDTO32
#else
#define ALIGNEDTO32        __attribute__ ((aligned (32)))
#define ZEROALIGNEDTO32    __attribute__ ((aligned (32))) = {0}
#endif

inline DWORD GetWidth(DWORD size)
{
	//Depending on size
	switch(size)
	{
		case QCIF:	return 176;
		case CIF:	return 352;
		case VGA:	return 640;
		case PAL:	return 768;
		case HVGA:	return 480;
		case QVGA:	return 320;
		case HD720P:	return 1280;
		case WQVGA:	return 400;
		case W448P:	return 768;
		case SD448P:	return 576;
		case W288P:	return 512;
		case W576:	return 1024;
		case FOURCIF:	return 704;
		case FOURSIF:	return 704;
		case XGA:	return 1024;
		case WVGA:	return 800;
		case DCIF:	return 528;
		case SIF:	return 352;
		case QSIF:	return 176;
		case SD480P:	return 480;
		case SQCIF:	return 128;
		case SCIF:	return 256;
		case HD1080P:	return 1920;
		case UW720P:	return 1680;
	}
	//Nothing
	return 0;
}

inline DWORD GetHeight(DWORD size)
{
	//Depending on size
	switch(size)
	{
		case QCIF:	return 144;
		case CIF:	return 288;
		case VGA:	return 480;
		case PAL:	return 576;
		case HVGA:	return 320;
		case QVGA:	return 240;
		case HD720P:	return 720;
		case WQVGA:	return 240;
		case W448P:	return 448;
		case SD448P:	return 448;
		case W288P:	return 288;
		case W576:	return 576;
		case FOURCIF:	return 576;
		case FOURSIF:	return 480;
		case XGA:	return 768;
		case WVGA:	return 480;
		case DCIF:	return 384;
		case SIF:	return 240;
		case QSIF:	return 120;
		case SD480P:	return 360;
		case SQCIF:	return 96;
		case SCIF:	return 192;
		case HD1080P:	return 1080;
		case UW720P:	return 720;
	}
	//Nothing
	return 0;
}

#include "Properties.h"

inline void* malloc32(size_t size)
{
	void* ptr;
	if(posix_memalign(&ptr,32,size))
		return NULL;
	return ptr;
}

struct VideoOrientation
{
	bool facing = 0;
	bool flip = 0;
	BYTE rotation = 0;
};

class ByteBuffer
{
public:
	ByteBuffer()
	{
		//Set buffer size
		size = 0;
		//Allocate memory
		buffer = NULL;
		//NO length
		length = 0;
	}
	
	ByteBuffer(const DWORD size)
	{
		//NO length
		length = 0;
		//Calculate new size
		this->size = size;
		//Realloc
		buffer = (BYTE*) malloc32(size);
	}
	
	ByteBuffer(const BYTE* data,const DWORD size)
	{
		//Calculate new size
		this->size = size;
		//Realloc
		buffer = (BYTE*) malloc32(size);
		//Copy
		memcpy(buffer,data,size);
		//Increase length
		length=size;
	}
	
	ByteBuffer(const ByteBuffer* bytes)
	{
		//Calculate new size
		size = bytes->GetLength();
		//Realloc
		buffer = (BYTE*) malloc32(size);
		//Copy
		memcpy(buffer,bytes->GetData(),size);
		//Increase length
		length=size;
	}

	ByteBuffer(const ByteBuffer& bytes)
	{
		//Calculate new size
		size = bytes.GetLength();
		//Realloc
		buffer = (BYTE*) malloc32(size);
		//Copy
		memcpy(buffer,bytes.GetData(),size);
		//Increase length
		length=size;
	}
	
	ByteBuffer* Clone() const {
		return new ByteBuffer(buffer,length);
	}

	virtual ~ByteBuffer()
	{
		//Clear memory
		if(buffer) free(buffer);
	}


	void Alloc(const DWORD size)
	{
		//Calculate new size
		this->size = size;
		//Realloc
		buffer = (BYTE*) realloc(buffer,size);
	}

	void Set(const BYTE* data,const DWORD size)
	{
		//Check size
		if (size>this->size)
			//Allocate new size
			Alloc(size*3/2);
		//Copy
		memcpy(buffer,data,size);
		//Increase length
		length=size;
	}

	DWORD Append(const BYTE* data,const DWORD size)
	{
		DWORD pos = length;
		//Check size
		if (size+length>this->size)
			//Allocate new size
			Alloc((size+length)*3/2);
		//Copy
		memcpy(buffer+length,data,size);
		//Increase length
		length+=size;
		//Return previous pos
		return pos;
	}
	
	const BYTE* GetData() const
	{
		return buffer;
	}
	
	DWORD GetSize() const
	{
		return size;
	}
	
	DWORD GetLength() const
	{
		return length;
	}
	
protected:
	BYTE	*buffer;
	DWORD	length;
	DWORD	size;
};
#endif
