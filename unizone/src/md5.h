#ifndef MD5_H
#define MD5_H

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include <qfile.h>
#include <sys/types.h>
#include <support/MuscleSupport.h>

#define md5byte unsigned char
#define UWORD32 unsigned int

struct MD5Context
{
	UWORD32 buf[4];
	UWORD32 bytes[2];
	UWORD32 in[16];
};

void MD5Init(struct MD5Context * context);
void MD5Update(struct MD5Context * context, md5byte const * buf, unsigned len);
void MD5Final(unsigned char digest[16], struct MD5Context * context);
void MD5Transform(UWORD32 buf[4], UWORD32 const in[16]);

/* Computes the hash code of the first (len) bytes of the given file.
 * (returnDigest) will have 16 bytes of MD5 hash data written into it.
 * If (len) is equal to zero, the file size will be detected, used,
 * and returned in (len).
 * If (optShutdownFlag) is specified, we will abort if we see it get set to true.
 */
status_t HashFileMD5(const QString & file, uint64 & len, uint64 offset, uint64 & retBytesHashed,
					 uint8 * returnDigest, volatile bool * optShutdownFlag);

#endif
