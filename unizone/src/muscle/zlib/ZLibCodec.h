/* This file is Copyright 2000-2008 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

#ifndef ZLibCodec_h
#define ZLibCodec_h

#ifdef MUSCLE_ENABLE_ZLIB_ENCODING

# include "util/ByteBuffer.h"
# include "zlib/zlib/zlib.h"

BEGIN_NAMESPACE(muscle);
 
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
     * @param independent If true, the generated buffer will be decompressible on its
     *                    own, not depending on any previously decompressed data.
     *                    If false, the generated buffer will only be uncompressable
     *                    if the previously Deflate()'d buffers have been reinflated
     *                    before it.  Setting this value to true will reduce the
     *                    compression efficiency, but allows for more flexibility.
     * @returna Reference to a buffer of compressed data on success, or a NULL reference on failure.
     */
   ByteBufferRef Deflate(const ByteBuffer & rawData, bool independent);

   /** Given a buffer of compressed data, returns a reference to a Buffer containing
     * the matching raw data, or NULL on failure.
     * @param rawData The compressed data to expand.  This should be a buffer that was previously
     *                produced by the Deflate() method.
     * @returns Reference to a buffer of decompressed data on success, or a NULL reference on failure.
     */
   ByteBufferRef Inflate(const ByteBuffer & compressedData);

   /** Given a ByteBuffer that was previously produced by Deflate(), returns the number of bytes
     * of raw data that the buffer represents, or -1 if the buffer isn't recognized as valid.
     * @param compressedData a ByteBuffer that was previously created by ZLibCodec::Deflate().
     * @param optRetIsIndependent If non-NULL, the bool that this argument points to will have
     *                            the independent/non-independent state of this buffer written into it.
     *                            See Deflate()'s documentation for details.
     */
   int32 GetInflatedSize(const ByteBuffer & compressedData, bool * optRetIsIndependent = NULL) const;

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

END_NAMESPACE(muscle);

#endif

#endif
