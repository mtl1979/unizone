#ifndef DEBUGIMPL_H
#define DEBUGIMPL_H

// Make sure _DEBUG is defined if DEBUG2 is
#ifdef DEBUG2
# ifndef _DEBUG
#  define _DEBUG
# endif
# define PRINT2 PRINT
#else
# define PRINT2 
#endif

#ifdef _DEBUG

void PRINT(const char *, ...);
void RedirectDebugOutput();
void CleanupDebug();

# define POPUP(X) WPopup(X)

#else

// Empty inline to eliminate debug output
inline void PRINT(const char *, ...) {}
# define RedirectDebugOutput()
# define CleanupDebug()
# define POPUP(X)

#endif


#if defined(_DEBUG) || defined(BETA)
void WPopup(const char *);
void WAssert(bool, const char *, int, const char *, const char *);
#define WASSERT(X, Y) WAssert(X, Y, __LINE__, __FILE__, __DATE__)
#else
#define WASSERT(X, Y)
#endif

#endif
