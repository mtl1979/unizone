#ifndef TRANSFERLIST_H
#define TRANSFERLIST_H

#include "downloadthread.h"
#include "uploadthread.h"
#include "transferitem.h"

#include "util/Queue.h"

#include <list>

using std::pair;
using std::list;

typedef pair<WDownloadThread *, WTransferItem *> DLPair;
typedef Queue<DLPair> DLList;

typedef pair<WUploadThread *, WTransferItem *> ULPair;
typedef Queue<ULPair> ULList;

#endif
