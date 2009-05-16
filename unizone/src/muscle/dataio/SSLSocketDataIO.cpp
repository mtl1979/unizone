#include <openssl/err.h>
#include "dataio/SSLSocketDataIO.h"

namespace muscle {

static void SetupOpenSSL() 
{
   static Mutex _sslInitMutex;
   if (_sslInitMutex.Lock() == B_NO_ERROR)
   {
      static bool _isSSLSetup = false;
      if (_isSSLSetup == false)
      {
         _isSSLSetup = true;

         SSL_load_error_strings();
         SSL_library_init();

         /* If we want to run on Windows, should seed PRNG */
      }
      _sslInitMutex.Unlock();
   }
}

SSLSocketDataIO::SSLSocketDataIO(const ConstSocketRef & sockfd, bool blocking, bool accept) : TCPSocketDataIO(sockfd, blocking) 
{
   SetupOpenSSL();

   _ctx = SSL_CTX_new(SSLv23_method());
   if (!blocking) SSL_CTX_set_mode(_ctx, SSL_MODE_ENABLE_PARTIAL_WRITE);

   _ssl = SSL_new(_ctx);
   _sbio = BIO_new_socket(sockfd, BIO_NOCLOSE);

   SSL_set_bio(_ssl, _sbio, _sbio);

   if (accept) SSL_set_accept_state(_ssl);
          else SSL_set_connect_state(_ssl);

   BIO_set_nbio(_sbio,!blocking);

   ERR_print_errors_fp(stderr);
}

SSLSocketDataIO::~SSLSocketDataIO() 
{
   Shutdown();
}

void SSLSocketDataIO::Shutdown() 
{
   if (_ssl)  {SSL_shutdown(_ssl);  _ssl  = NULL;}
   if (_ctx)  {SSL_CTX_free(_ctx);  _ctx  = NULL;}
   if (_sbio) {BIO_free_all(_sbio); _sbio = NULL;}
   TCPSocketDataIO::Shutdown();
}

status_t SSLSocketDataIO::SetCertificate(const char *path) 
{
   return (SSL_use_certificate_file(_ssl, path, SSL_FILETYPE_PEM) == 1) ? B_NO_ERROR : B_ERROR;
}

status_t SSLSocketDataIO::SetKey(const char *pem) 
{
   return (SSL_use_PrivateKey_file(_ssl, pem, SSL_FILETYPE_PEM) == 1) ? B_NO_ERROR : B_ERROR;
}

int32 SSLSocketDataIO::Read(void *buffer, uint32 size) 
{
   int32 bytes = SSL_read(_ssl, buffer, size);
   if (bytes == 0) return -1;

   /* Confusingly, a zero return from SSL_read() means the connection has
      been terminated, whereas a negative return usually indicates that
      we should try again */
   int err = SSL_get_error(_ssl, bytes);
   if ((bytes < 0)&&((err == SSL_ERROR_WANT_READ)||(err == SSL_ERROR_WANT_WRITE))) bytes = 0;
   if (bytes < 0) 
   {
      fprintf(stderr,"SSL ERROR!");
      ERR_print_errors_fp(stderr);
   }
   return bytes;
}

int32 SSLSocketDataIO::Write(const void *buffer, uint32 size) 
{
   int32 bytes = SSL_write(_ssl, buffer, size);
   if (bytes == 0) return -1;

   int err = SSL_get_error(_ssl, bytes);
   if ((bytes < 0)&&((err == SSL_ERROR_WANT_READ)||(err == SSL_ERROR_WANT_WRITE))) bytes = 0;
   if (bytes < 0) 
   {
      fprintf(stderr,"SSL ERROR!");
      ERR_print_errors_fp(stderr);
   }
   return bytes;
}

}; // end namespace muscle
