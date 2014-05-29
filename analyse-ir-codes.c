/*
    @file analyse-ir-codes.c
    
    Copyright 2011, Paul Chambers. All Rights Reserved.
*/

#include "common.h"
#include "analyse-ir-codes.h"
#include "timestamp.h"

#include "import.h"
#include "analyse.h"
#include "export.h"

#define DEBUG   1
#define VERSION "0.1"

tGlobals globals = {
    NULL,
    VERSION,
    { BUILD_DATE, EXPIRY_DATE },
    EXPIRY_TIMESTAMP
};


tIRCodeSet  *gIRCodeSets = NULL;	/* linked list of IR code sets */
tIRCode     *gIRCodes    = NULL;        /* master list of all IR codes */

static const char *usageString = 
{
"    -i <file>    input file (defaults to stdin, if not a tty)\n"
"    -o <file>    output file (defaults to stdout)\n"
"    -l <file>    input file (defaults to stderr)\n"
"    -d <level>   debug level (0+)\n"
"    -q           'quiet' - suppress everything except fatal and error messages.\n"
};


int main(int argc, const char *argv[])
{
    int     i;
    int     debugLevel;
    char    *p;
    time_t  now;
    FILE    *inputFile, *outputFile, *logFile;

    enum {
        kInputFile  = 'i',
        kOutputFile = 'o',
        kLogFile    = 'l',
        kDebugLevel = 'd',
        kQuiet      = 'q',
        kNormal     = 'n'
    } optState;

    initLogging(LOG_WARNING, stderr);

    inputFile = stdin;
    outputFile = stdout;

    globals.myName = argv[0];
    p = strrchr( globals.myName, '/' );
    if ( p != NULL)
        globals.myName = p + 1;

    logprintf("%s, version %s\n   built on %s\n",
                globals.myName,
                globals.version, 
                globals.date.built );

    now = time(NULL);
    if (now == -1 || now > globals.expiryTimestamp)
    {
        logprintf(" expired on %s - exiting\n", globals.date.expiries );
        exit(-1);
    } else {
        logprintf("    expires %s\n", globals.date.expiries );
    }

    optState = kNormal;
    
    for (i = 1; i < argc; ++i)
    {
        logDebug(0,"argv[%d] = \'%s\'\n", i, argv[i]);
        if (argv[i][0] == '-')
        {
            const char *p = &argv[i][1];
            while (*p != '\0')
            {
                switch (*p)
                {
                case kDebugLevel:
                    ++p;
                    if (!isdigit(*p))
                    {
                        if (optState == kNormal)
                            optState = kDebugLevel;
                        else
                            fatalExit(-5, "bad combination of options");
                    }
                    else 
                    {
                        debugLevel = 0;
                        do {
                            /* accumulate digits */
                            debugLevel = (debugLevel * 10) + (*p++ - '0');
                        } while (isdigit(*p));
                        setLogThreshold(LOG_DEBUG + debugLevel);
                    }
                    --p; /* pre-compensate for ++p after switch */
                    break;
                    
                case kQuiet:
                    setLogThreshold( LOG_ERR );
                    break;

                case kInputFile:
                    if (optState == kNormal)
                        optState = kInputFile;
                    else
                        fatalExit(-5, "bad combination of options");
                    break;

                case kOutputFile:
                    if (optState == kNormal)
                        optState = kOutputFile;
                    else
                        fatalExit(-5, "bad combination of options");
                    break;

                case kLogFile:
                    if (optState == kNormal)
                        optState = kLogFile;
                    else
                        fatalExit(-5, "bad combination of options");
                    break;

                default:
                    fatalExit(-2, "don't understand option \'%s\'", argv[i]);
                    break;
                }
                ++p;
            }
        }
        else
        {
            switch (optState)
            {
            case kNormal:
                break;

            case kDebugLevel:
                setLogThreshold( LOG_DEBUG + atoi(argv[i]) );
                optState = kNormal;
                break;

            case kInputFile:
                inputFile = fopen( argv[i], "r" );
                if (inputFile == NULL)
                {
                    inputFile = stdin;
                    fatalExitErrno( -3, "unable to open input file \"%s\"", argv[i] );
                }
                optState = kNormal;
                break;

            case kOutputFile:
                outputFile = fopen( argv[i], "w" );
                if (outputFile == NULL)
                {
                    outputFile = stdout;
                    fatalExitErrno( -3, "unable to open output file \"%s\"", argv[i] );
                }
                optState = kNormal;
                break;

            case kLogFile:
                logFile = fopen( argv[i], "w" );
                if (logFile != NULL)
                {
                    logTo(logFile);
                }
                else
                {
                    logFile = stderr;
                    fatalExitErrno( -3, "unable to open log file \"%s\"", argv[i] );
                }
                optState = kNormal;
                break;

            default:
                fatalExit(-99, "internal error, unknown optState");  
                break;
            }
        }
    }
    
    if (optState != kNormal)
    {
        fatalExit(-1, "-%c option is missing a parameter", (char)optState);
    }

    if ( isatty(fileno(inputFile)) )
    {
        fatalExit(-1, "Usage: not enough arguments provided.\n\n%s", usageString);
    }

    importDB(inputFile);

    analyzeIRCodeSets();

    exportDB(outputFile);

    if (inputFile != stdin)
        fclose(inputFile);

    if (outputFile != stdout)
        fclose(outputFile);

    exit(0);
}
