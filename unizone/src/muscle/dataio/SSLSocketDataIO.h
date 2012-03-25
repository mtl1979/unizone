#ifndef MuscleSSLSocketDataIO_h
#define MuscleSSLSocketDataIO_h

#include <openssl/ssl.h>
#include "dataio/TCPSocketDataIO.h"

namespace muscle {

/** This class lets you communicate over a TCP socket with SSL encryption enabled on it. */
class SSLSocketDataIO : public TCPSocketDataIO, private CountedObject<SSLSocketDataIO>
{
public:
   /**
    * Constructor.
    * @param sockfd The socket to use.
    * @param blocking If true, the socket will be set to blocking mode; otherwise non-blocking
    * @param accept If true, accept a connection at start.  Defaults to false.
    */
   SSLSocketDataIO(const ConstSocketRef & sockfd, bool blocking, bool accept = false);

   virtual ~SSLSocketDataIO();

   /** Adds a certification to use for this session.
     * @param certFilePath File path of the certificate file to use.
     * @returns B_NO_ERROR on success, or B_ERROR on failure (couldn't find file?)
     */ 
   status_t SetCertificate(const char * certFilePath);

   /** Adds a private key to use for this session.
     * @param privateKeyFilePath File path of the private key file to use.
     * @returns B_NO_ERROR on success, or B_ERROR on failure (couldn't find file?)
     */ 
   status_t SetKey(const char * privateKeyFilePath);

   virtual int32 Read(void *buffer, uint32 size);
   virtual int32 Write(const void *buffer, uint32 size);
   virtual void Shutdown();

private:
   SSL_CTX *_ctx;
   SSL * _ssl;
   BIO * _sbio;
};

}; // end namespace muscle

#endif
