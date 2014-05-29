/*
    @file logging.h
    
    Yet another set of logging macros.
    
    Everyone has their own, it seems :)
    
    Copyright 2011, Paul Chambers. All Rights Reserved.
*/
#ifndef __MY_LOGGING__
#define __MY_LOGGING__

#include <syslog.h>

#ifdef UNUSED 
#elif defined(__GNUC__) 
# define UNUSED(x) UNUSED_ ## x __attribute__((unused)) 
#elif defined(__LCLINT__) 
# define UNUSED(x) /*@unused@*/ x 
#else 
# define UNUSED(x) x 
#endif

/* private - don't set this directly - use setLogThreshold() */
/* higher the number, more gets logged. logFatal and logError always log. */
extern int gLogThreshold;
/* also private - set using logTo() */
extern FILE *gLogFile;

void initLogging(int logLevel, FILE *logFile);
void setLogThreshold(int logLevel);
void logTo(FILE *logFile);

/* loggging macros */

#define logFatal(...) \
            { _logHelper(LOG_CRIT, __FILE__, __func__, __LINE__, 0, __VA_ARGS__); }
#define logFatalErrno(...) \
            { _logHelper(LOG_CRIT, __FILE__, __func__, __LINE__, errno, __VA_ARGS__); }

#define logError(...) \
            { _logHelper(LOG_ERR, __FILE__, __func__, __LINE__, 0, __VA_ARGS__); }
#define logErrorErrno(...) \
            { _logHelper(LOG_ERR, __FILE__, __func__, __LINE__, errno, __VA_ARGS__); }

#define logWarning(...) \
            { if (LOG_WARNING <= gLogThreshold) _logHelper(LOG_WARNING, __FILE__, __func__, __LINE__, 0, __VA_ARGS__); }
#define logWarningErrno(...) \
            { if (LOG_WARNING <= gLogThreshold) _logHelper(LOG_WARNING, __FILE__, __func__, __LINE__, errno, __VA_ARGS__); }

#define logInfo(...) \
            { if (LOG_INFO <= gLogThreshold) _logHelper(LOG_INFO, __FILE__, __func__, __LINE__, 0, __VA_ARGS__); }
#define logInfoErrno(...) \
            { if (LOG_INFO <= gLogThreshold) _logHelper(LOG_INFO, __FILE__, __func__, __LINE__, errno, __VA_ARGS__); }

#define logDebug(level, ...) \
            { if ((LOG_DEBUG + level) <= gLogThreshold) _logHelper((LOG_DEBUG + level), __FILE__, __func__, __LINE__, 0, __VA_ARGS__); }
#define logDebugErrno(level, ...) \
            { if ((LOG_DEBUG + level) <= gLogThreshold) _logHelper((LOG_DEBUG + level), __FILE__, __func__, __LINE__, errno, __VA_ARGS__); }

void _logHelper( int level, const char *file, const char *function, const int line, const int error, const char *format, ...)
        __attribute__ ((format (printf, 6, 7)));

#define DEBUG_LINE_PREFIX   "dbg: "
#define logDebugEnabled(level)  ((LOG_DEBUG + level) <= gLogThreshold)
#define logprintf(...)          { fprintf(gLogFile, __VA_ARGS__); }

/* fatal error macros */

#define fatalExit(exitcode, ...)        _fatalExit(exitcode, __FILE__, __func__, __LINE__, 0, __VA_ARGS__)
#define fatalExitErrno(exitcode, ...)   _fatalExit(exitcode, __FILE__, __func__, __LINE__, errno, __VA_ARGS__)

void _fatalExit( int exitcode, const char *file, const char *function, const int line, const int error, const char *format, ...)
        __attribute__ ((format (printf, 6, 7),noreturn));

#endif
