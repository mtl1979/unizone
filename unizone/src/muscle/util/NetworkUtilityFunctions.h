/* This file is Copyright 2005 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifndef MuscleNetworkUtilityFunctions_h
#define MuscleNetworkUtilityFunctions_h

#include "support/MuscleSupport.h"
#include "util/TimeUtilityFunctions.h"

// These includes are here so that people can use select() without having to #include the proper
// things themselves all the time.
#ifndef WIN32
# include <sys/types.h>
# include <sys/socket.h>
#endif

#ifdef BONE
# include <sys/select.h>  // sikosis at bebits.com says this is necessary... hmm.
#endif

BEGIN_NAMESPACE(muscle);

/** Numeric representation of 127.0.0.1, for convenience */
const uint32 localhostIP = ((((uint32)127)<<24)|((uint32)1));

/** Numeric representation of 255.255.255.255, for convenience */
const uint32 broadcastIP = ((uint32)-1);

/** Given a hostname or IP address (e.g. "mycomputer.be.com" or "192.168.0.1"),
  * performs a hostname lookup and returns the 4-byte IP address that corresponds
  * with that name.
  * @param name ASCII IP address or hostname to look up.
  * @return The 4-byte IP address (local endianness), or 0 on failure.
  */
uint32 GetHostByName(const char * name);

/** Convenience function for connecting with TCP to a given hostName/port.
 * @param hostName The ASCII host name or ASCII IP address of the computer to connect to.
 * @param port The port number to connect to.
 * @param debugTitle If non-NULL, debug output to stdout will be enabled and debug output will be prefaced by this string.
 * @param debugOutputOnErrorsOnly if true, debug output will be printed only if an error condition occurs.
 * @return the non-negative sockfd if the connection is successful, -1 if the connection attempt failed.
 */
int Connect(const char * hostName, uint16 port, const char * debugTitle = NULL, bool debugOutputOnErrorsOnly = true);

/** Mostly as above, only with the target IP address specified numerically, rather than as an ASCII string. 
 *  This version of connect will never do a DNS lookup.
 * @param hostIP The numeric host IP address to connect to.
 * @param port The port number to connect to.
 * @param debugHostName If non-NULL, we'll print this host name out when reporting errors.  It isn't used for networking purposes, though.
 * @param debugTitle If non-NULL, debug output to stdout will be enabled and debug output will be prefaced by this string.
 * @param debugOutputOnErrorsOnly if true, debug output will be printed only if an error condition occurs.
 * @return the non-negative sockfd if the connection is successful, -1 if the connection attempt failed.
 */
int Connect(uint32 hostIP, uint16 port, const char * debugHostName = NULL, const char * debugTitle = NULL, bool debugOutputOnErrorsOnly = true);

/** Convenience function for accepting a TCP connection on a given socket.  Has the same semantics as accept().
 *  (This is somewhat better than calling accept() directly, as certain cross-platform issues get transparently taken care of)
 * @param socket socket FD to accept from (e.g. a socket that was previously returned by CreateAcceptingSocket())
 * @return the non-negative sockfd if the accept is successful, or -1 if the accept attempt failed.
 */
int Accept(int socket);

/** Reads as many bytes as possible from the given socket and places them into (buffer).
 *  @param socket The socket to read from.
 *  @param buffer Buffer to write the bytes into.
 *  @param bufferSizeBytes Number of bytes in the buffer.
 *  @param socketIsBlockingIO Pass in true if the given socket is set to use blocking I/O, or false otherwise.
 *  @return The number of bytes read into (buffer), or a negative value if there was an error.
 *          Note that this value may be smaller than (bufferSizeBytes).
 */
int32 ReceiveData(int socket, void * buffer, uint32 bufferSizeBytes, bool socketIsBlockingIO);

/** Identical to ReceiveData(), except that this function's logic is adjusted to handle UDP semantics properly. 
 *  @param socket The socket to read from.
 *  @param buffer Buffer to write the bytes into.
 *  @param bufferSizeBytes Number of bytes in the buffer.
 *  @param socketIsBlockingIO Pass in true if the given socket is set to use blocking I/O, or false otherwise.
 *  @param optRetFromIP If set to non-NULL, then on success the uint32 this parameter points to will be filled in
 *                      with the IP address that the received data came from.  Defaults to NULL.
 *  @param optRetFromPort If set to non-NULL, then on success the uint16 this parameter points to will be filled in
 *                      with the source port that the received data came from.  Defaults to NULL.
 *  @return The number of bytes read into (buffer), or a negative value if there was an error.
 *          Note that this value may be smaller than (bufferSizeBytes).
 */
int32 ReceiveDataUDP(int socket, void * buffer, uint32 bufferSizeBytes, bool socketIsBlockingIO, uint32 * optRetFromIP = NULL, uint16 * optRetFromPort = NULL);

/** Transmits as many bytes as possible from the given buffer over the given socket.
 *  @param socket The socket to transmit over.
 *  @param buffer Buffer to read the outgoing bytes from.
 *  @param bufferSizeBytes Number of bytes to send.
 *  @param socketIsBlockingIO Pass in true if the given socket is set to use blocking I/O, or false otherwise.
 *  @return The number of bytes sent from (buffer), or a negative value if there was an error.
 *          Note that this value may be smaller than (bufferSizeBytes).
 */
int32 SendData(int socket, const void * buffer, uint32 bufferSizeBytes, bool socketIsBlockingIO);

/** Similar to SendData(), except that this function's logic is adjusted to handle UDP semantics properly.
 *  @param socket The socket to transmit over.
 *  @param buffer Buffer to read the outgoing bytes from.
 *  @param bufferSizeBytes Number of bytes to send.
 *  @param socketIsBlockingIO Pass in true if the given socket is set to use blocking I/O, or false otherwise.
 *  @param optDestIP If set to non-zero, the data will be sent to the given IP address.  Otherwise it will
 *                   be sent to the socket's current IP address (see SetUDPSocketTarget()).  Defaults to zero.
 *  @param destPort If set to non-zero, the data will be sent to the specified port.  Otherwise it will
 *                  be sent to the socket's current destination port (see SetUDPSocketTarget()).  Defaults to zero.
 *  @return The number of bytes sent from (buffer), or a negative value if there was an error.
 *          Note that this value may be smaller than (bufferSizeBytes).
 */
int32 SendDataUDP(int socket, const void * buffer, uint32 bufferSizeBytes, bool socketIsBlockingIO, uint32 optDestIP = 0, uint16 destPort = 0);

/** This function initiates a non-blocking connection to the given host IP address and port.
  * It will return the created socket, which may or may not be fully connected yet.
  * If it is connected, (retIsReady) will be to true, otherwise it will be set to false.
  * If (retIsReady) is false, then you can use select() to find out when the state of the
  * socket has changed:  select() will return ready-to-write on the socket when it is 
  * fully connected (or when the connection fails), at which point you can call 
  * FinalizeAsyncConnect() on the socket:  if FinalizeAsyncConnect() succeeds, the connection 
  * succeeded; if not, the connection failed.
  * @param hostIP 32-bit IP address to connect to (hostname isn't used as hostname lookups can't be made asynchronous AFAIK)
  * @param port Port to connect to.
  * @param retIsReady On success, this bool is set to true iff the socket is ready to use, or 
  *                   false to indicate that an asynchronous connection is now in progress.
  * @return a non-negative sockfd which is in the process of connecting on success, or -1 on error.
  */
int ConnectAsync(uint32 hostIP, uint16 port, bool & retIsReady);

/** When a socket that was connecting asynchronously finally
  * selects ready-for-write to indicate that the asynchronous connect 
  * attempt has reached a conclusion, call this method.  It will finalize
  * the connection and make it ready for use.
  * @param socket The socket that was connecting asynchronously
  * @returns B_NO_ERROR if the connection is ready to use, or B_ERROR if the connect failed.
  * @note Under Windows, select() won't return ready-for-write if the connection fails... instead
  *       it will select-notify for your socket on the exceptions fd_set (if you provided one).
  *       Once this happens, there is no need to call FinalizeAsyncConnect() -- the fact that the
  *       socket notified on the exceptions fd_set is enough for you to know the asynchronous connection
  *       failed.  Successful asynchronous connect()s do exhibit the standard (select ready-for-write)
  *       behaviour, though.
  */
status_t FinalizeAsyncConnect(int socket);

/** Shuts the given socket down.  (Note that you don't generally need to call this function, as CloseSocket() will suffice)
 *  @param socket The socket to shutdown communication on. 
 *  @param disableReception If true, further reception of data will be disabled on this socket.
 *  @param disableTransmission If true, further transmission of data will be disabled on this socket.
 *  @return B_NO_ERROR on success, or B_ERROR on failure.
 */
status_t ShutdownSocket(int socket, bool disableReception = true, bool disableTransmission = true);

/** Closes the given socket.  
  * @param socket The socket to close.  (socket) may be negative, indicating a no-op.
  */
void CloseSocket(int socket);

/** Convenience function for creating a TCP socket that is listening on a given local port for incoming TCP connections.
 *  @param port Which port to listen on, or 0 to have the system should choose a port for you
 *  @param maxbacklog Maximum connection backlog to allow for (defaults to 20)
 *  @param optRetPort If non-NULL, the uint16 this value points to will be set to the actual port bound to (useful when you want the system to choose a port for you)
 *  @param optFrom Optional IP address to accept values from only.  Defaults to zero, meaning TCP connections
 *                 will be accepted from any IP address.
 *  @return the non-negative sockfd if the port was bound successfully, or -1 if the attempt failed.
 */
int CreateAcceptingSocket(uint16 port, int maxbacklog = 20, uint16 * optRetPort = NULL, uint32 optFrom = 0); 

/** Translates the given 4-byte IP address into a string representation.
 *  @param address The 4-byte IP address to translate into text.
 *  @param outBuf Buffer where the NUL-terminated ASCII representation of the string will be placed.
 *                This buffer must be at least 16 bytes long.
 */
void Inet_NtoA(uint32 address, char * outBuf);

/** Given a dotted-quad IP address in ASCII format (e.g. "192.168.0.1"), returns
  * the equivalent IP address in uint32 (packet binary) form. 
  * @param buf numeric IP address in ASCII.
  * @returns IP address as a uint32, or 0 on failure.
  */
uint32 Inet_AtoN(const char * buf);

/** Reurns the IP address that the given socket is connected to.
 *  @param socket The socket descriptor to find out info about.
 *  @return The IP address on success, or 0 on failure (such as if the socket isn't valid and connected).
 */
uint32 GetPeerIPAddress(int socket);

/** Creates a pair of sockets that are connected to each other,
 *  so that any bytes you pass into one socket come out the other socket.
 *  This is useful when you want to wake up a thread that is blocked in select()...
 *  you have it select() on one socket, and you send a byte on the other.
 *  @param retSocket1 On success, this value will be set to the socket ID of the first socket.  Set to -1 on failure.
 *  @param retSocket2 On success, this value will be set to the socket ID of the second socket.  Set to -1 on failure.
 *  @param blocking Whether the two sockets should use blocking I/O or not.  Defaults to false.
 *  @param nagles Whether the two sockets should use Nagle's algorithm or not.  Defaults to false, for lower latency.
 *  @return B_NO_ERROR on success, or B_ERROR on failure.
 */
status_t CreateConnectedSocketPair(int & retSocket1, int & retSocket2, bool blocking = false, bool nagles=false);

/** Enables or disables blocking I/O on the given socket. 
 *  (Default for a socket is blocking mode enabled)
 *  @param socket the socket descriptor to act on.
 *  @param enabled Whether I/O on this socket should be enabled or not.
 *  @return B_NO_ERROR on success, or B_ERROR on failure.
 */
status_t SetSocketBlockingEnabled(int socket, bool enabled);

/**
  * Turns Nagle's algorithm (output packet buffering/coalescing) on or off.
  * @param socket the socket descriptor to act on.
  * @param enabled If true, data will be held momentarily before sending, 
  *                to allow for bigger packets.  If false, each Write() 
  *                call will cause a new packet to be sent immediately.
  * @return B_NO_ERROR on success, B_ERROR on error.
  */
status_t SetSocketNaglesAlgorithmEnabled(int socket, bool enabled);

/**
  * Sets the size of the given socket's TCP send buffer to the specified
  * size (or as close to that size as is possible).
  * @param socket the socket descriptor to act on.
  * @param sendBufferSizeBytes New size of the TCP send buffer, in bytes.
  * @returns B_NO_ERROR on success, or B_ERROR on failure.
  */
status_t SetSocketSendBufferSize(int socket, uint32 sendBufferSizeBytes);

/**
  * Sets the size of the given socket's TCP receive buffer to the specified
  * size (or as close to that size as is possible).
  * @param socket the socket descriptor to act on.
  * @param receiveBufferSizeBytes New size of the TCP receive buffer, in bytes.
  * @returns B_NO_ERROR on success, or B_ERROR on failure.
  */
status_t SetSocketReceiveBufferSize(int socket, uint32 receiveBufferSizeBytes);

/** Convenience function:  Won't return for a given number of microsends.
 *  @param micros The number of microseconds to wait for.
 *  @return B_NO_ERROR on success, or B_ERROR on failure.
 */
status_t Snooze64(uint64 microseconds);

/** Set a user-specified IP address to return from GetHostByName() and GetPeerAddress() instead of 127.0.0.1.
  * Note that this function <b>does not</b> change the computer's IP address -- it merely changes what
  * the aforementioned functions will report.
  * @param ip New IP address to return instead of 127.0.0.1, or 0 to disable this override.
  */
void SetLocalHostIPOverride(uint32 ip);

/** Returns the user-specified IP address that was previously set by SetLocalHostIPOverride(), or 0
  * if none was set.  Note that this function <b>does not</b> report the local computer's IP address,
  * unless you previously called SetLocalHostIPOverride() with that address.
  */
uint32 GetLocalHostIPOverride();

/** Creates and returns a socket that can be used for UDP communications.
 *  Returns a negative value on error, or a non-negative socket handle on
 *  success.  You'll probably want to call BindUDPSocket() or SetUDPSocketTarget()
 *  after calling this method.  When you are done with the socket, be sure to
 *  call CloseSocket() on it.
 */
int CreateUDPSocket();

/** Attempts to given UDP socket to the given port.  
 *  @param sock The UDP socket (previously created by CreateUDPSocket())
 *  @param port UDP port ID to bind the socket to.  If zero, the system will choose a port ID.
 *  @param optRetPort if non-NULL, then on successful return the value this pointer points to will contain
 *                    the port ID that the socket was bound to.  Defaults to NULL.
 *  @param optFrom If non-zero, then the socket will be bound in such a way that only data
 *                 packets from this IP address will be accepted.  Defaults to zero.
 *  @returns B_NO_ERROR on success, or B_ERROR on failure.
 */
status_t BindUDPSocket(int sock, uint16 port, uint16 * optRetPort = NULL, uint32 optFrom = 0);

/** Set the target/destination address for a UDP socket.  After successful return
 *  of this function, any data that is written to the UDP socket will be sent to this
 *  IP address and port.  This function is guaranteed to return quickly.
 *  @param sock The UDP socket to send to (previously created by CreateUDPSocket()).
 *  @param remoteIP Remote IP address that data should be sent to.
 *  @param remotePort Remote UDP port ID that data should be sent to.
 *  @returns B_NO_ERROR on success, or B_ERROR on failure.
 */
status_t SetUDPSocketTarget(int sock, uint32 remoteIP, uint16 remotePort);

/** Enable/disable sending of broadcast packets on the given UDP socket.
 *  @param sock UDP socket to enable or disable the sending of broadcast UDP packets with.
 *              (Note that the default state of newly created UDP sockets is broadcast-disabled)
 *  @param broadcast True if broadcasting should be enabled, false if broadcasting should be disabled.
 *  @returns B_NO_ERROR on success, or B_ERROR on failure.
 */
status_t SetUDPSocketBroadcastEnabled(int sock, bool broadcast);

/** As above, except that the remote host is specified by hostname instead of IP address.
 *  Note that this function may take involve a DNS lookup, and so may take a significant
 *  amount of time to complete.
 *  @param sock The UDP socket to send to (previously created by CreateUDPSocket()).
 *  @param remoteHostName Name of remote host (e.g. "www.mycomputer.com" or "132.239.50.8")
 *  @param remotePort Remote UDP port ID that data should be sent to.
 *  @returns B_NO_ERROR on success, or B_ERROR on failure.
 */
status_t SetUDPSocketTarget(int sock, const char * remoteHostName, uint16 remotePort);

END_NAMESPACE(muscle);

#endif

