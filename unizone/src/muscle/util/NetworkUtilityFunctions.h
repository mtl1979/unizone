/* This file is Copyright 2002 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifndef MuscleNetworkUtilityFunctions_h
#define MuscleNetworkUtilityFunctions_h

#include "util/TimeUtilityFunctions.h"
#include "util/String.h"

namespace muscle {

/** Numeric representation of 127.0.0.1, for convenience */
const uint32 localhostIP = ((((uint32)127)<<24)|((uint32)1));

// On these OS's, calls like accept() take an int* rather than a uint32*
// If you use net_length_t instead of specifying the type directly, you'll
// remain portable nonetheless.  Or better yet, call Accept() instead...
#if __BEOS__ || __APPLE__ || __CYGWIN__ || WIN32
# define net_length_t int
#else
# define net_length_t size_t
#endif

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
  */
status_t FinalizeAsyncConnect(int socket);

/** Closes the given socket.  
  * @param socket the socket to close.  (socket) may be negative, indicating a no-op.
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

/** Returns a string representation of the IP address (address).  Useful on OS's where inet_ntoa isn't available.
 *  @param address The 4-byte IP address to translate into text.
 *  @returns The ASCII representation of the IP address.
 */
String Inet_NtoA(uint32 address);

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

/** Calls fork(), setsid(), chdir(), umask(), etc, to fork an independent daemon process.
 *  Also closes all open file descriptors.
 *  Note that this function will call exit() on the parent process if successful,
 *  and thus won't ever return in that process. 
 *  @param optNewDir If specified, the daemon will chdir() to the directory specified here.
 *  @param optOutputTo Where to redirect stderr and stdout to.  Defaults to "/dev/null".
 *                     If set to NULL, or if the output device can't be opened, output
 *                     will not be rerouted.
 *  @param createOutputFileIfNecessary if set true, and (optOutputTo) can't be opened,
 *                                     (optOutputTo) will be created.
 *  @return B_NO_ERROR on success (the child process will see this), B_ERROR on failure.
 */
status_t BecomeDaemonProcess(const char * optNewDir = NULL, const char * optOutputTo = "/dev/null", bool createOutputFileIfNecessary = false);

/** Same as BecomeDaemonProcess(), except that the parent process returns as well as the child process.  
 *  @param returningAsParent Set to true on return of the parent process, or false on return of the child process.
 *  @param optNewDir If specified, the child will chdir() to the directory specified here.
 *  @param optOutputTo Where to redirect stderr and stdout to.  Defaults to "/dev/null".
 *                     If set to NULL, or if the output device can't be opened, output
 *                     will not be rerouted.
 *  @param createOutputFileIfNecessary if set true, and (optOutputTo) can't be opened,
 *                                     (optOutputTo) will be created.
 *  @return B_NO_ERROR (twice!) on success, B_ERROR on failure.
 */ 
status_t SpawnDaemonProcess(bool & returningAsParent, const char * optNewDir = NULL, const char * optOutputTo = "/dev/null", bool createOutputFileIfNecessary = false);

/** Creates a pair of sockets that are connected to each other,
 *  so that any bytes you pass into one socket come out the other socket.
 *  This is useful when you want to wake up a thread that is blocked in select()...
 *  you have it select() on one socket, and you send a byte on the other.
 *  @param retSocket1 On success, this value will be set to the socket ID of the first socket.  Set to -1 on failure.
 *  @param retSocket2 On success, this value will be set to the socket ID of the second socket.  Set to -1 on failure.
 *  @param blocking Whether the two sockets should use blocking I/O or not.  Defaults to false.
 *  @return B_NO_ERROR on success, or B_ERROR on failure.
 */
status_t CreateConnectedSocketPair(int & retSocket1, int & retSocket2, bool blocking = false);

/** Enables or disables blocking I/O on the given socket. 
 *  (Default for a socket is blocking mode enabled)
 *  @param socket the socket descriptor to act on.
 *  @param enabled Whether I/O on this socket should be enabled or not.
 *  @return B_NO_ERROR on success, or B_ERROR on failure.
 */
status_t SetSocketBlockingEnabled(int socket, bool enabled);

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

};  // end namespace muscle

#endif

