/* This file is Copyright 2003 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifndef ZLibCodec_h
#define ZLibCodec_h

#ifdef MUSCLE_ENABLE_ZLIB_ENCODING

# include "util/ByteBuffer.h"
# include "zlib/zlib/zlib.h"

namespace muscle {
 
/** This class is a handy wrapper around the zlib C functions.
  * It quickly and easily inflates and deflates data to/from independently compressed chunks.
  */
class ZLibCodec
{
public:
   /** Constructor.
     * @param compressionLevel how much to compress outgoing data.  0 is no
     *                         compression, 9 is maximum compression.  Default is 6.
     */
   ZLibCodec(int compressionLevel = 6);

   /** Destructor */
   ~ZLibCodec();

   /** Given a buffer of raw data, returns a reference to a Buffer containing
     * the matching compressed data.
     * @param rawData The raw data to compress
     * @param fullyIndependent If set true, all data will be dumped and the compressor's
     *                         state reset, so that the next buffer deflated after this one
     *                         will not depend on the decompressor having decompressed this
     *                         buffer previously in order to decompress that buffer.  Note that
     *                         setting this to true can severely degrade decompression efficiency!
     * @returna Reference to a buffer of compressed data on success, or a NULL reference on failure.
     */
   ByteBufferRef Deflate(const ByteBuffer & rawData, bool fullyIndependent);

   /** Given a buffer of compressed data, returns a reference to a Buffer containing
     * the matching raw data, or NULL on failure.
     * @param rawData The compressed data to expand.
     * @returna Reference to a buffer of decompressed data on success, or a NULL reference on failure.
     */
   ByteBufferRef Inflate(const ByteBuffer & compressedData);

   /** Given a ByteBuffer that was previously produced by Deflate(), returns the number of bytes
     * of raw data that the buffer represents, or -1 if the buffer isn't recognized as valid.
     * @param compressedData a ByteBuffer that was previously created by ZLibCodec::Deflate().
     */
   int32 GetInflatedSize(const ByteBuffer & compressedData) const;

   /** Returns this codec's compression level, as was specified in the constructor.
     * Note that this value only affects what we compress to -- we can compress any compression
     * level, although the compression level cannot change from one Inflate() call to another.
     */
   int GetCompressionLevel() const {return _compressionLevel;}

private:
   void InitStream(z_stream & stream);

   int _compressionLevel;

   bool _inflateOkay;
   z_stream _inflater;

   bool _deflateOkay;
   z_stream _deflater;
};

};  // end namespace muscle

#endif

#endif
