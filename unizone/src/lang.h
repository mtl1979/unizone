#ifndef UNIZONE_LANG_H
#define UNIZONE_LANG_H

/*
**
** To add a new language:
**
** 1. Copy the lang/english.h as template
** 2. Add conditional to include the new file if MSG_LANG is correct
** 3. Update project configuration to include new translation
** 4. MSG_LANG is international access code for the primary country that uses the language,
**    if more languages than one is used in the country, number starting from 1 will be appended.
**    Primary language will be plain IAC. F.ex in Finland MSG_LANG=358 will be Finnish and MSG_LANG=3581 will be
**    finlandssvenska (Swedish dialect spoken in Western Finland), Lapp would be 3582.
**
*/

// Set default language to Finnish

#ifndef MSG_LANG
#define MSG_LANG 358
#endif

//
// English Strings
//

#if (MSG_LANG == 1)

#include "lang/english.h"

#endif

//
// Finnish Strings
//

#if (MSG_LANG == 358)

#include "lang/finnish.h"

#endif

//
// Portuguese Strings -- Francois(BR) (fvincent@freeshell.org)
//

#if (MSG_LANG == 351)

#include "lang/portuguese.h"

#endif

//
// German Strings -- leprOSy (leprosy@freenet.de)
//

#if (MSG_LANG == 49)

#include "lang/german.h"

#endif

//
// French Strings  -- LoLL Addon -- (beosland@free.fr)
//

#if (MSG_LANG == 33)

#include "lang/french.h"

#endif

//
// Russian Strings -- Sauron_RU (wblaze@yandex.ru)
//

#if (MSG_LANG == 7)

#include "lang/russian.h"

#endif

//
// Swedish Strings -- Ripped from BeShare and invented some of my own ;)
//

#if (MSG_LANG == 46)

#include "lang/swedish.h"

#endif

//
// Spanish Strings -- bencer (bencer@e-milio.com)

#if (MSG_LANG == 34)

#include "lang/spanish.h"

#endif

//
// End of language translations
//

#endif // UNIZONE_LANG_H