/* This file is Copyright 2002 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifndef MuscleMessageIOGateway_h
#define MuscleMessageIOGateway_h

#include "iogateway/AbstractMessageIOGateway.h"

namespace muscle {

/**
 * Encoding IDs.  Only MUSCLE_MESSAGE_ENCODING_DEFAULT is supported for now!
 */
enum {
   MUSCLE_MESSAGE_ENCODING_DEFAULT = 1164862256  // 'Enc0',  /**< just plain ol' flattened Message objects, with no special encoding */
};

/** Callback function type for flatten/unflatten notification callbacks */
typedef void (*MessageFlattenedCallback)(MessageRef msgRef, void * userData);

/**
 * A "gateway" object that knows how to send/receive Messages over a wire
 * (via a provided DataIO object). 
 * May be subclassed to change the byte-level protocol, or used as is if the default protocol is desired.
 *
 * The default protocol format used by this class is:
 *   -# 4 bytes (uint32) indicating the flattened size of the message
 *   -# 4 bytes (uint32) indicating the encoding type (should always be MUSCLE_MESSAGE_ENCODING_DEFAULT for now)
 *   -# n bytes of flattened Message (where n is the value specified in 1)
 *   -# goto 1 ...
 *
 * An example flattened Message byte structure is provided at the bottom of the
 * MessageIOGateway.h header file.
 */
class MessageIOGateway : public AbstractMessageIOGateway
{
public:
   /** 
    *  Constructor.
    *  @param encoding The byte-stream format the message should be encoded into.
    *                  The only supported format is MUSCLE_MESSAGE_ENCODING_DEFAULT, for now.
    */
   MessageIOGateway(int32 encoding = MUSCLE_MESSAGE_ENCODING_DEFAULT);

   /**
    *  Destructor.
    *  Deletes the held DataIO object.
    */
   virtual ~MessageIOGateway();

   virtual int32 DoOutput(uint32 maxBytes = MUSCLE_NO_LIMIT);
   virtual int32 DoInput(uint32 maxBytes = MUSCLE_NO_LIMIT);
   virtual bool HasBytesToOutput() const;

   /**
    * Lets you specify a function that will be called every time an outgoing
    * Message is about to be flattened by this gateway.  You may alter the
    * Message at this time, if you need to.
    * @param cb Callback function to call.
    * @param ud User data; set this to any value you like.
    */
   void SetAboutToFlattenMessageCallback(MessageFlattenedCallback cb, void * ud) {_aboutToFlattenCallback = cb; _aboutToFlattenCallbackData = ud;}

   /**
    * Lets you specify a function that will be called every time an outgoing
    * Message has been flattened by this gateway.
    * @param cb Callback function to call.
    * @param ud User data; set this to any value you like.
    */
   void SetMessageFlattenedCallback(MessageFlattenedCallback cb, void * ud) {_flattenedCallback = cb; _flattenedCallbackData = ud;}

   /**
    * Lets you specify a function that will be called every time an incoming
    * Message has been unflattened by this gateway.
    * @param cb Callback function to call.
    * @param ud User data; set this to any value you like.
    */
   void SetMessageUnflattenedCallback(MessageFlattenedCallback cb, void * ud) {_unflattenedCallback = cb; _unflattenedCallbackData = ud;}

   /**
    * Lets you specify the maximum allowable size for an incoming flattened Message.
    * Doing so lets you limit the amount of memory a remote computer can cause your
    * computer to attempt to allocate.  Default max message size is MUSCLE_NO_LIMIT 
    * (or about 4 gigabytes)
    * @param maxBytes New incoming message size limit, in bytes.
    */
   void SetMaxIncomingMessageSize(uint32 maxBytes) {_maxIncomingMessageSize = maxBytes;}

   /** Returns the current maximum incoming message size, as was set above. */
   uint32 GetMaxIncomingMessageSize() const {return _maxIncomingMessageSize;}

protected:
   /**
    * Should return the total number of bytes to use for the body buffer for flattening the given message.
    * @param msg The Message in question.
    * @return Default implementation returns msg.FlattenedSize().
    */
   virtual uint32 GetFlattenedMessageBodySize(const Message & msg) const;

   /**
    * Flattens (msg) into (buffer) and (header).
    * @param msg The Message to flatten into a byte array.
    * @param header A byte array that is GetHeaderSize() bytes long.  FlattenMessage() must write the
    *               proper header bytes into this array.
    * @param buffer A byte array that is GetFlattenedMessageBodySize() bytes long.  FlattenMessage()
    *               must write the proper message body bytes into this array.
    * @return B_NO_ERROR if the flattening succeeded and the produced bytes should be sent, 
    *         B_ERROR if flattening failed and/or the produced bytes should not be sent.
    * The default implementation always returns B_NO_ERROR.
    */
   virtual status_t FlattenMessage(const Message & msg, uint8 * header, uint8 * buffer) const;

   /**
    * Unflattens (msg) from (buffer) and (header).
    * @param setMsg An empty Message.  On success, the restored Message should be written into this object.
    * @param header The buffer of message header bytes.  It is GetHeaderSize() bytes long.
    * @param buf The buffer of message body bytes.  It is (bufSize) bytes long.
    * @param bufSize The number of bytes pointed to by (buf)
    * @return B_NO_ERROR on success, B_ERROR if the Message couldn't be restored.
    */
   virtual status_t UnflattenMessage(Message & setMsg, const uint8 * header, const uint8 * buf, uint32 bufSize) const;
 
   /**
    * Returns the size of the pre-flattened-message header section, in bytes.
    * The default encoding has an 8-byte header (4 bytes for encoding ID, 4 bytes for message size)
    */
   virtual uint32 GetHeaderSize() const;

   /**
    * Must Extract and returns the buffer body size from the given header.
    * Note that the returned size should NOT include the header bytes!
    * @param header Points to the header of the message.  The header is GetHeaderSize() bytes long.
    * @param setSize On success, the number of bytes to expect in the body of the message should be written here.
    * @return B_NO_ERROR if successful, B_ERROR if the header format was incorrect or for some
    *         other reason the message body size could not be determined.
    */
   virtual status_t GetBodySize(const uint8 * header, uint32 & setSize) const;

   /**
    * This method is called after GetBodySize() returns an error code.  (i.e.
    * when the header bytes we read in were not valid)
    * It should return B_NO_ERROR if the gateway should try to recover from
    * the invalid header, or B_ERROR if it should just give up and close the connection.
    * If it returns B_NO_ERROR, it may set the (setFirstValidByte) parameter to the
    * index of the first byte in the header buffer to start reading the next header
    * from, or leave (setFirstValidByte) unchanged to throw out all the bytes in the 
    * current header buffer and start afresh.
    * Default implementation just returns B_ERROR.
    * @param headerBuf the header bytes that were read in.
    * @param setFirstValidByte the length of the header.  May be set as described above.
    * @return B_NO_ERROR if recovery was successful, else B_ERROR.  (see above)
    */
   virtual status_t RecoverFromInvalidHeader(const uint8 * headerBuf, uint32 & setFirstValidByte) const;
   
   /**
    * This method is called after UnflattenMessage() returns an error code (i.e.
    * when the body bytes we read in were not valid).
    * It should return B_NO_ERROR if the gateway should try to recover from the
    * invalid message, or B_ERROR if it should just give up and close the connection.
    * If B_NO_ERROR is returned, the the gateway will begin trying to read the
    * next header from the bytes immediately after these.
    * @param messageBuf Pointer to the message bytes.
    * @param messageBufSize Number of bytes pointed to by (messageBuf)
    * @return B_NO_ERROR to continue parsing, B_ERROR to give up (see above).
    */
   virtual status_t RecoverFromInvalidMessage(const uint8 * messageBuf, uint32 messageBufSize) const;

private:
   uint8 * _sendBuffer;
   uint32  _sendBufferSize;
   int32   _sendBufferOffset;
   uint32  _currentSendMessageSize;  // includes header bytes

   uint8 * _recvBuffer;
   uint32  _recvBufferSize;
   int32   _recvBufferOffset;
   uint32  _currentRecvMessageSize;  // includes header bytes
   uint32  _maxIncomingMessageSize;

   int32   _encoding;
  
   MessageFlattenedCallback _aboutToFlattenCallback;
   void * _aboutToFlattenCallbackData;

   MessageFlattenedCallback _flattenedCallback;
   void * _flattenedCallbackData;

   MessageFlattenedCallback _unflattenedCallback;
   void * _unflattenedCallbackData;
};

//////////////////////////////////////////////////////////////////////////////////
//
// Here is a commented example of a flattened Message's byte structure.
// When one uses a MessageIOGateway to send Messages, it will
// send out series of bytes that looks like this.
//
// Note that this information is only helpful if you are trying to implement
// your own MessageIOGateway-compatible serialization/deserialization code.
// C++ and Java programmers will have a much easier time if they use the 
// MessageIOGateway class provided in the MUSCLE archive, rather than 
// coding at the byte-stream level.
//
// The Message used in this example has a 'what' code value of 2 and the 
// following name/value pairs placed in it:
//
//  String field, name="!SnKy"   value="/*/*/beshare"
//  String field, name="session" value="123"
//  String field, name="text"    value="Hi!"
//
// Bytes in single quotes represent ASCII characters, bytes without quotes
// means literal decimal byte values.  (E.g. '2' means 50 decimal, 2 means 2 decimal)
//
// All occurences of '0' here indicate the ASCII digit zero (decimal 48), not the letter O.
//
// The bytes shown here should be sent across the TCP socket in 
// 'normal reading order': left to right, top to bottom.
//
// 88   0   0   0   (int32, indicates that total message body size is 88 bytes) (***)
// '0' 'c' 'n' 'E'  ('Enc0' == MUSCLE_MESSAGE_ENCODING_DEFAULT) (***)
//
// '0' '0' 'M' 'P'  ('PM00' == CURRENT_PROTOCOL_VERSION)
//  2   0   0   0   (2      == NET_CLIENT_NEW_CHAT_TEXT, the message's 'what' code)
//  3   0   0   0   (3      == Number of name/value pairs in this message)
//  6   0   0   0   (6      == Length of first name, "!SnKy", include NUL byte) 
// '!' 'S' 'n' 'K'  (Field name ASCII bytes.... "!SnKy")
// 'y'  0           (last field name ASCII byte and the NUL terminator byte) 
// 'R' 'T' 'S' 'C'  ('CSTR' == B_STRING_TYPE; i.e. this value is a string)
// 13   0   0   0   (13     == Length of value string including NUL byte)
// '/' '*' '/' '*'  (Field value ASCII bytes.... "/*/*/beshare")
// '/' 'b' 'e' 's'  (....)
// 'h' 'a' 'r' 'e'  (....)
//  0               (NUL terminator byte for the ASCII value)
//  8   0   0   0   (8      == Length of second name, "session", including NUL)
// 's' 'e' 's' 's'  (Field name ASCII Bytes.... "session")
// 'i' 'o' 'n'  0   (rest of field name ASCII bytes and NUL terminator)
// 'R' 'T' 'S' 'C'  ('CSTR' == B_STRING_TYPE; i.e. this value is a string)
//  4   0   0   0   (4      == Length of value string including NUL byte)
// '1' '2' '3'  0   (Field value ASCII bytes... "123" plus NUL byte)
//  5   0   0   0   (5      == Length of third name, "text", including NUL)
// 't' 'e' 'x' 't'  (Field name ASCII bytes... "text")
//  0               (NUL byte terminator for field name)
// 'R' 'T' 'S' 'C'  ('CSTR' == B_STRING_TYPE; i.e. this value is a string)
//  4   0   0   0   (3      == Length of value string including NUL byte)
// 'H' 'i' '!'  0   (Field value ASCII Bytes.... "Hi!" plus NUL byte)
//
// [that's the complete byte sequence; to transmit another message, 
//  you would start again at the top, with the next message's 
//  message-body-length-count]
//
// (***) The bytes in this field should not be included when tallying the message body size!
//
//////////////////////////////////////////////////////////////////////////////////

};  // end namespace muscle

#endif
