/* This file is Copyright 2007 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

#ifndef ZipFileUtilityFunctions_h
#define ZipFileUtilityFunctions_h

#include "message/Message.h"

BEGIN_NAMESPACE(muscle);

/** Given a Message object, writes an .zip file to disk containing  the B_RAW_TYPE data in that Message.
  * Each B_RAW_TYPE field in the Message object will be written to the .zip file as a contained file,
  * except for Message fields, which will be written recursively to the .zip file as sub-directories.
  * @param fileName Name of the file to create and write to.  This name will typically end in ".zip".
  * @param msg Reference to the Message to write to the file.
  * @param compressionLevel A number between 0 (no compression) and 9 (maximum compression).  Default value is 9.
  * @param fileCreationTime the file creation time (in microseconds since 1970) to assign to all of the 
  *                         file-records in the .zip file.  If left as MUSCLE_TIME_NEVER (the default) then
  *                         the current time will be used.
  * @note This function is useful only when you need to be compatible with the .ZIP file format. 
  *       In particular, it does NOT save all the information in the Message -- only the contents
  *       of the B_RAW_TYPE fields (as "files") and B_MESSAGE_TYPE fields as ("folders").
  *       If you need to store Message objects to disk and read them back verbatim later, you should
  *       use Message::Flatten() and Message::Unflatten() to do so, not this function.
  * @returns B_NO_ERROR on success, or B_ERROR on failure.
  */
status_t WriteZipFile(const char * fileName, const Message & msg, int compressionLevel = 9, uint64 fileCreationTime = MUSCLE_TIME_NEVER);

/** Given the name of a .zip file on disk, reads the file and creates and
  * returns an equivalent Message object.  Each contained file in the .zip file
  * will appear in the Message object as a B_RAW_DATA field, and each directory
  * in the .zip file will appear in the Message object as a Message field.
  * @param fileName Name of the file to read from.
  * @returns B_NO_ERROR on success, or B_ERROR on failure.
  */
MessageRef ReadZipFile(const char * fileName);

END_NAMESPACE(muscle);

#endif
