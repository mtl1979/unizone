#ifndef VERSION_H
#define VERSION_H

class QString;

QString WinShareVersionString();

const QString & GetUnizoneYears();

const int & UZ_MajorVersion();
const int & UZ_MinorVersion();
const int & UZ_Patch();
const int & UZ_Build();

#endif
