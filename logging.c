/*
    @file logging.c

    Copyright 2011, Paul Chambers. All Rights Reserved.
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "logging.h"

int     gLogThreshold = LOG_INFO;
FILE *  gLogFile      = NULL;

static const char *msgLevelStr[] = {
    "### Emergency: ",  /*  LOG_EMERG   0   system is unusable */
    "### Alert:   ",    /*  LOG_ALERT   1   action must be taken immediately */
    "### Fatal:   ",    /*  LOG_CRIT    2   critical conditions */
    "### Error:   ",    /*  LOG_ERR     3   error conditions */
    "#   Warning: ",    /*  LOG_WARNING 4   warning conditions */
    "#   Notice:  ",    /*  LOG_NOTICE  5   normal but significant condition */
    "#   Info:    ",    /*  LOG_INFO    6   informational */
    DEBUG_LINE_PREFIX   /*  LOG_DEBUG   7   debug-level messages */
};

/* this will be necessary when syslog logging is added */
void initLogging(int logLevel, FILE *logFile)
{
    setLogThreshold( logLevel );
    logTo( logFile );
}

void setLogThreshold(int logLevel)
{
    gLogThreshold = logLevel;
}

void logTo(FILE *logFile)
{
    if (gLogFile != NULL && gLogFile != stderr)
        fclose(gLogFile);

    gLogFile = logFile;
}

void _logHelper( int level, const char *file, const char * UNUSED(function), const int line, const int error, const char *format, ...)
{
    va_list args;
    int msgLevel;
    
    if (gLogFile != NULL)
    {
        va_start(args, format);
     
        msgLevel = level;
        if (msgLevel > LOG_DEBUG)
            msgLevel = LOG_DEBUG;
        
        fprintf(gLogFile, "%s%s, line %d: ", msgLevelStr[msgLevel], file, line);

        vfprintf(gLogFile, format, args);

        if (error != 0)
            fprintf(gLogFile, " (%d: %s)", error, strerror(error));

        fputc('\n', gLogFile);

        va_end(args);
    }
}

void _fatalExit( int exitcode, const char *file, const char * UNUSED(function), const int line, const int error, const char *format, ...)
{
    va_list args;
    
    if (gLogFile != NULL)
    {
        va_start(args, format);
     
        fprintf(stderr, "%s%s, line %d: ", msgLevelStr[LOG_CRIT], file, line);
        vfprintf(stderr, format, args);

        if (error != 0)
            fprintf(stderr, " (%d: %s)", error, strerror(error));

        fputc('\n', stderr);

        va_end(args);
    }

    exit(exitcode);
}   
