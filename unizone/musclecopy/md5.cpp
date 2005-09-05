#include "md5.h"
#include "wfile.h"

#include <stdio.h>
#include <string.h>
#include <qstring.h>
#include <qfile.h>

#if BYTE_ORDER == LITTLE_ENDIAN
#define byteSwap(buf, words)	// we're little endian (linux & windows on intel hardware)
								// so we don't need this
#else
void byteSwap(unsigned int *buf, unsigned int words)
{
	for (unsigned int x = 0; x < words; x++)
	{
		buf[x] = B_SWAP_INT32(buf[x]);
	}
}
#endif

/*
 * Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious initialization constants.
 */
void
MD5Init(struct MD5Context *ctx)
{
   ctx->buf[0] = 0x67452301;
   ctx->buf[1] = 0xefcdab89;
   ctx->buf[2] = 0x98badcfe;
   ctx->buf[3] = 0x10325476;

   ctx->bytes[0] = 0;
   ctx->bytes[1] = 0;

   // bugfix (dammit) -- jaf
   for (size_t i=0; i<sizeof(ctx->in)/sizeof(ctx->in[0]); i++) ctx->in[i] = 0;
}

/*
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
void
MD5Update(struct MD5Context *ctx, md5byte const *buf, unsigned len)
{
   /* Update byte count */
   UWORD32 t = ctx->bytes[0];
   if ((ctx->bytes[0] = t + len) < t) ctx->bytes[1]++;   /* Carry from low to high */

   t = 64 - (t & 0x3f);   /* Space available in ctx->in (at least 1) */
   if (t > len) {
      memcpy((md5byte *)ctx->in + 64 - t, buf, len);
      return;
   }

   /* First chunk is an odd size */
   memcpy((md5byte *)ctx->in + 64 - t, buf, t);
   byteSwap(ctx->in, 16);
   MD5Transform(ctx->buf, ctx->in);
   buf += t;
   len -= t;

   /* Process data in 64-byte chunks */
   while (len >= 64) {
      memcpy(ctx->in, buf, 64);
      byteSwap(ctx->in, 16);
      MD5Transform(ctx->buf, ctx->in);
      buf += 64;
      len -= 64;
   }

   /* Handle any remaining bytes of data. */
   memcpy(ctx->in, buf, len);
}

/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern 
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
void
MD5Final(md5byte digest[16], struct MD5Context *ctx)
{
   int count = ctx->bytes[0] & 0x3f;   /* Number of bytes in ctx->in */
   md5byte *p = (md5byte *)ctx->in + count;

   /* Set the first char of padding to 0x80.  There is always room. */
   *p++ = 0x80;

   /* Bytes of padding needed to make 56 bytes (-8..55) */
   count = 56 - 1 - count;

   if (count < 0) {   /* Padding forces an extra block */
      memset(p, 0, count + 8);
      byteSwap(ctx->in, 16);
      MD5Transform(ctx->buf, ctx->in);
      p = (md5byte *)ctx->in;
      count = 56;
   }
   memset(p, 0, count);
   byteSwap(ctx->in, 14);

   /* Append length in bits and transform */
   ctx->in[14] = ctx->bytes[0] << 3;
   ctx->in[15] = ctx->bytes[1] << 3 | ctx->bytes[0] >> 29;
   MD5Transform(ctx->buf, ctx->in);

   byteSwap(ctx->buf, 4);
   memcpy(digest, ctx->buf, 16);
   memset(ctx, 0, sizeof(ctx));   /* In case it's sensitive */
}

/* The four core functions - F1 is optimized somewhat */

/* #define F1(x, y, z) (x & y | ~x & z) */
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#define MD5STEP(f,w,x,y,z,in,s) (w += f(x,y,z) + in, w = (w<<s | w>>(32-s)) + x)

/*
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  MD5Update blocks
 * the data and converts bytes into longwords for this routine.
 */
void
MD5Transform(UWORD32 buf[4], UWORD32 const in[16])
{
   register UWORD32 a, b, c, d;

   a = buf[0];
   b = buf[1];
   c = buf[2];
   d = buf[3];

   MD5STEP(F1, a, b, c, d, in[0] + 0xd76aa478, 7);
   MD5STEP(F1, d, a, b, c, in[1] + 0xe8c7b756, 12);
   MD5STEP(F1, c, d, a, b, in[2] + 0x242070db, 17);
   MD5STEP(F1, b, c, d, a, in[3] + 0xc1bdceee, 22);
   MD5STEP(F1, a, b, c, d, in[4] + 0xf57c0faf, 7);
   MD5STEP(F1, d, a, b, c, in[5] + 0x4787c62a, 12);
   MD5STEP(F1, c, d, a, b, in[6] + 0xa8304613, 17);
   MD5STEP(F1, b, c, d, a, in[7] + 0xfd469501, 22);
   MD5STEP(F1, a, b, c, d, in[8] + 0x698098d8, 7);
   MD5STEP(F1, d, a, b, c, in[9] + 0x8b44f7af, 12);
   MD5STEP(F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
   MD5STEP(F1, b, c, d, a, in[11] + 0x895cd7be, 22);
   MD5STEP(F1, a, b, c, d, in[12] + 0x6b901122, 7);
   MD5STEP(F1, d, a, b, c, in[13] + 0xfd987193, 12);
   MD5STEP(F1, c, d, a, b, in[14] + 0xa679438e, 17);
   MD5STEP(F1, b, c, d, a, in[15] + 0x49b40821, 22);

   MD5STEP(F2, a, b, c, d, in[1] + 0xf61e2562, 5);
   MD5STEP(F2, d, a, b, c, in[6] + 0xc040b340, 9);
   MD5STEP(F2, c, d, a, b, in[11] + 0x265e5a51, 14);
   MD5STEP(F2, b, c, d, a, in[0] + 0xe9b6c7aa, 20);
   MD5STEP(F2, a, b, c, d, in[5] + 0xd62f105d, 5);
   MD5STEP(F2, d, a, b, c, in[10] + 0x02441453, 9);
   MD5STEP(F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
   MD5STEP(F2, b, c, d, a, in[4] + 0xe7d3fbc8, 20);
   MD5STEP(F2, a, b, c, d, in[9] + 0x21e1cde6, 5);
   MD5STEP(F2, d, a, b, c, in[14] + 0xc33707d6, 9);
   MD5STEP(F2, c, d, a, b, in[3] + 0xf4d50d87, 14);
   MD5STEP(F2, b, c, d, a, in[8] + 0x455a14ed, 20);
   MD5STEP(F2, a, b, c, d, in[13] + 0xa9e3e905, 5);
   MD5STEP(F2, d, a, b, c, in[2] + 0xfcefa3f8, 9);
   MD5STEP(F2, c, d, a, b, in[7] + 0x676f02d9, 14);
   MD5STEP(F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

   MD5STEP(F3, a, b, c, d, in[5] + 0xfffa3942, 4);
   MD5STEP(F3, d, a, b, c, in[8] + 0x8771f681, 11);
   MD5STEP(F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
   MD5STEP(F3, b, c, d, a, in[14] + 0xfde5380c, 23);
   MD5STEP(F3, a, b, c, d, in[1] + 0xa4beea44, 4);
   MD5STEP(F3, d, a, b, c, in[4] + 0x4bdecfa9, 11);
   MD5STEP(F3, c, d, a, b, in[7] + 0xf6bb4b60, 16);
   MD5STEP(F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
   MD5STEP(F3, a, b, c, d, in[13] + 0x289b7ec6, 4);
   MD5STEP(F3, d, a, b, c, in[0] + 0xeaa127fa, 11);
   MD5STEP(F3, c, d, a, b, in[3] + 0xd4ef3085, 16);
   MD5STEP(F3, b, c, d, a, in[6] + 0x04881d05, 23);
   MD5STEP(F3, a, b, c, d, in[9] + 0xd9d4d039, 4);
   MD5STEP(F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
   MD5STEP(F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
   MD5STEP(F3, b, c, d, a, in[2] + 0xc4ac5665, 23);

   MD5STEP(F4, a, b, c, d, in[0] + 0xf4292244, 6);
   MD5STEP(F4, d, a, b, c, in[7] + 0x432aff97, 10);
   MD5STEP(F4, c, d, a, b, in[14] + 0xab9423a7, 15);
   MD5STEP(F4, b, c, d, a, in[5] + 0xfc93a039, 21);
   MD5STEP(F4, a, b, c, d, in[12] + 0x655b59c3, 6);
   MD5STEP(F4, d, a, b, c, in[3] + 0x8f0ccc92, 10);
   MD5STEP(F4, c, d, a, b, in[10] + 0xffeff47d, 15);
   MD5STEP(F4, b, c, d, a, in[1] + 0x85845dd1, 21);
   MD5STEP(F4, a, b, c, d, in[8] + 0x6fa87e4f, 6);
   MD5STEP(F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
   MD5STEP(F4, c, d, a, b, in[6] + 0xa3014314, 15);
   MD5STEP(F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
   MD5STEP(F4, a, b, c, d, in[4] + 0xf7537e82, 6);
   MD5STEP(F4, d, a, b, c, in[11] + 0xbd3af235, 10);
   MD5STEP(F4, c, d, a, b, in[2] + 0x2ad7d2bb, 15);
   MD5STEP(F4, b, c, d, a, in[9] + 0xeb86d391, 21);

   buf[0] += a;
   buf[1] += b;
   buf[2] += c;
   buf[3] += d;
}


status_t 
HashFileMD5(QString entry, uint64 & len, uint64 offset, uint64 & retBytesHashed,
			uint8 * returnDigest, volatile bool * optShutdownFlag)
{
	WFile file; // UNICODE !!!
	if (!file.Open(entry, IO_ReadOnly))
		return B_ERROR;
	uint64 size = file.Size();
	if (len > 0 && size < len)
		return B_ERROR;

	if (len == 0 || len == size)
	{
		uint8 buf[24];
		// see if we've cached this MD5 yet
		QString cf(entry);
		cf += ".md5";
		WFile cache;
		if (cache.Open(cf, IO_ReadOnly))	// open will fail if we have not yet cached the item
		{
			if (cache.ReadBlock((char *)buf, sizeof(buf)) == sizeof(buf))
			{
				cache.Close();
				uint64 checkSize = B_LENDIAN_TO_HOST_INT64(*((int64 *)buf));
				if (checkSize == size)
				{
					memcpy(returnDigest, &buf[8], 16);
					if (len == 0)
						len = size;
					return B_OK;
				}
			}
		}
	}

	for (int i = 0; i < 16; i++)
		returnDigest[i] = 0;

	const uint32 bufSize = 256 * 1024;
	struct MD5Context ctx;
	unsigned char * buf = new unsigned char[bufSize];
	CHECK_PTR(buf);
	uint64 bytesLeft = (len > 0) ? len : size;

	if (offset > 0)
        {
                if (len > 0)
                {
                        if ((offset + len) > size)
                                return B_ERROR;
                        file.Seek(offset);
                }
                else
                {
                        if (offset < size)
                                file.Seek(size - offset);
                        bytesLeft = (offset < size) ? offset : size;
                }
        }

	MD5Init(&ctx);
	uint64 numRead;
	retBytesHashed = 0;
	while ((numRead = file.ReadBlock((char *)buf, (bytesLeft < bufSize) ? bytesLeft : bufSize)) > 0)
	{
		retBytesHashed += numRead;

		if (optShutdownFlag && *optShutdownFlag)
		{
			delete [] buf;
			file.Close();
			return B_ERROR;
		}

		if (numRead < bytesLeft)
		{
			bytesLeft -= numRead;
			MD5Update(&ctx, buf, numRead);
		}
		else
		{
			MD5Update(&ctx, buf, bytesLeft);
			bytesLeft = 0;
			break;
		}
	}
	delete [] buf;
	file.Close();

	if (bytesLeft > 0)
		return B_ERROR;		// file not long enough?

	MD5Final(returnDigest, &ctx);

	if (len == 0)
		len = size;
	if (len == size)
	{
		// cache the results
		uint8 buf[24];
		*((int64 *)buf) = B_HOST_TO_LENDIAN_INT64(size);
		memcpy(&buf[8], returnDigest, 16);
		
		QString cf(entry);
		cf += ".md5";
		WFile cache;
		if (cache.Open(cf, IO_WriteOnly))
		{
			cache.WriteBlock((const char *)buf, sizeof(buf));
			cache.Close();
		}
	}
	return B_OK;
}
