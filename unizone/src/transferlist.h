#ifndef TRANSFERLIST_H
#define TRANSFERLIST_H

#include "downloadthread.h"
#include "uploadthread.h"
#include "transferitem.h"

#include "util/Queue.h"

#include <list>

using std::pair;
using std::list;

typedef struct DLPair
{
	WDownloadThread * thread;
	WTransferItem * item;
} DLPair;
typedef Queue<DLPair> DLList;

typedef struct ULPai
{
	WUploadThread * thread;
	WTransferItem * item;
} ULPair;
typedef Queue<ULPair> ULList;

#endif
