#ifndef TRANSFERLIST_H
#define TRANSFERLIST_H

#include "genericthread.h"
#include "transferitem.h"

#include "util/Queue.h"

#include <list>

using std::pair;
using std::list;

typedef pair<WGenericThread *, WTransferItem *> WTPair;
typedef Queue<WTPair> WTList;
//typedef WTList::iterator WTIter;

#endif
