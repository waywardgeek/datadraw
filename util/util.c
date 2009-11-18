/*
 * This file was written by Bill Cox, originally in 1991, and maintained since.  It is hereby
 * placed into the public domain.
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include "ddutil.h"
#include "utmem.h"

bool  _utInitialized = false;
utSymtab utTheSymtab;
uint32 utSetjmpLine[UT_MAX_SETJMP_DEPTH];
char *utSetjmpFile[UT_MAX_SETJMP_DEPTH];
char *utConfigDirectory = NULL;
static char *utExeDirectory = NULL;
static char *utExeFullPath = NULL;
static FILE *utLogFile = NULL;
static FILE *utReportFile = NULL;
static FILE *utDebugFile = NULL;
static char *utLogFileName = NULL;
static char *utReportFileName = NULL;
static char *utDebugFileName = NULL;
uint32 utDebugVal = 0;
uint32 utVerboseVal = 0;
#define UT_MAX_BUFFERS 42
static char *utBuffers[UT_MAX_BUFFERS];
static uint16 utNextBuffer;
static uint32 utBufferSizes[UT_MAX_BUFFERS];
static time_t utTimers[UT_MAX_BUFFERS];
static uint16 utTimerDepth = 0;
static utErrorProc utUserErrProc = NULL, utUserWarningProc = NULL, utUserStatusProc = NULL, utUserLogMessageProc = NULL;
static char *utVersion = NULL;
int16 utSetjmpDepth = 0;
static uint32 utSymNextIndex;
static bool utMessageHeaderEnabled = false;

/*--------------------------------------------------------------------------------------------------
  Like ansi sprintf.
--------------------------------------------------------------------------------------------------*/
char *utSprintf(
    char *format,
    ...)
{
    va_list ap;
    char *returnBuffer;

    va_start(ap, format);
    returnBuffer = utVsprintf(format, ap);
    va_end(ap);
    return returnBuffer;
}

/*--------------------------------------------------------------------------------------------------
    Enable the message header tags to add info to the messages
--------------------------------------------------------------------------------------------------*/
void utEnableMessageHeaders(
    bool value)
{
    utMessageHeaderEnabled = value;
}

/*--------------------------------------------------------------------------------------------------
    get the message prefix
--------------------------------------------------------------------------------------------------*/
static char* getMessageNameFromType(
    utMessageType msgType)
{
    switch(msgType) {
        case UT_MESSAGE_INFO: return "INFO:";
        case UT_MESSAGE_WARNING: return "WARNING:";
        case UT_MESSAGE_ERROR: return "ERROR:";
        case UT_MESSAGE_EXIT: return "EXIT:";
        case UT_MESSAGE_DETAILS: return "DETAILS:";
        case UT_MESSAGE_INTERNAL: return "DEBUG:";
        case UT_MESSAGE_REPORT: return "REPORT: ";
    }
    return "";
}

/*--------------------------------------------------------------------------------------------------
  Log a message to the debug file.
--------------------------------------------------------------------------------------------------*/
void utLogMessageType(
    utMessageType msgType,
    char *format,
    ...)
{
    va_list ap;
    char *buff;

    va_start(ap, format);
    buff = utVsprintf((char *)format, ap);
    va_end(ap);
    if(utMessageHeaderEnabled || msgType == UT_MESSAGE_ERROR || msgType == UT_MESSAGE_EXIT ||
            msgType == UT_MESSAGE_WARNING) {
        buff = utSprintf("%s%s", getMessageNameFromType(msgType), buff);
    }
    if(utUserLogMessageProc != NULL) {
        (utUserLogMessageProc)(buff);
    }
    if(utLogFile == NULL) {
        return;
    }
    fputs(buff, utLogFile);
    fflush(utLogFile);
}

/*--------------------------------------------------------------------------------------------------
  Function to report results
--------------------------------------------------------------------------------------------------*/
void utReport(
    char *format, ...)
{
    va_list ap;
    char *buff;

    va_start(ap, format);
    buff = utVsprintf(format, ap);
    va_end(ap);
    if(utReportFile == NULL) {
       utLogMessageType(UT_MESSAGE_REPORT, "%s", buff);
       return;
    }
    fputs(buff, utReportFile);
    fflush(utReportFile);
}

/*--------------------------------------------------------------------------------------------------
  Same as utLogDebug but without the newline & prefix.
--------------------------------------------------------------------------------------------------*/
bool utDebug(
    char *format,
    ...)
{
    va_list ap;
    char *buff;

    va_start(ap, format);
    buff = utVsprintf((char *)format, ap);
    va_end(ap);
    if(utDebugFile == NULL) {
        utLogMessageType(UT_MESSAGE_INTERNAL, "%s", buff);
        return true;
    }
    fputs(buff, utDebugFile);
    fflush(utDebugFile);
    return true; /* So utDebug can be used in ',' expressions */
}

/*--------------------------------------------------------------------------------------------------
  Log a message to the debug file.
--------------------------------------------------------------------------------------------------*/
void utLogDebug(
    char *format,
    ...)
{
    va_list ap;
    char *buff;

    va_start(ap, format);
    buff = utVsprintf((char *)format, ap);
    va_end(ap);
    if(utUserStatusProc != NULL) {
        utStatus("%s\n", buff);
    } else if(!utMessageHeaderEnabled) {
        printf("%s\n", buff);
        fflush(stdout);
    }
    utDebug("%s\n", buff);
}

/*--------------------------------------------------------------------------------------------------
  Log a message to the debug file.
--------------------------------------------------------------------------------------------------*/
void utLogString(
    char *format,
    ...)
{
    va_list ap;
    char *buff;

    va_start(ap, format);
    buff = utVsprintf((char *)format, ap);
    va_end(ap);
    if(utUserStatusProc != NULL) {
        utStatus("%s", buff);
    } else if(!utMessageHeaderEnabled) {
        printf("%s", buff);
        fflush(stdout);
    }
    utLogMessageType(UT_MESSAGE_INFO, "%s", buff);
}

/*--------------------------------------------------------------------------------------------------
  Log a message to the debug file.
--------------------------------------------------------------------------------------------------*/
void utLogMessage(
    char *format,
    ...)
{
    va_list ap;
    char *buff;

    va_start(ap, format);
    buff = utVsprintf((char *)format, ap);
    va_end(ap);
    if(utUserStatusProc != NULL) {
       utStatus("%s\n", buff);
    } else {
       printf("%s\n", buff);
       fflush(stdout);
    }
    utLogMessageType(UT_MESSAGE_INFO, "%s\n", buff);
}

/*--------------------------------------------------------------------------------------------------
  Print a message, and start recording the time since this message.
--------------------------------------------------------------------------------------------------*/
uint32 utStartTimer(
    char *format,
    ...)
{
    va_list ap;
    char *buff;
    uint16 timerID = utTimerDepth++;

    utTimers[timerID] = time(NULL);
    if(format != NULL) {
        va_start(ap, format);
        buff = utVsprintf((char *)format, ap);
        va_end(ap);
        utLogMessageType(UT_MESSAGE_INFO, "%s\n", buff);
        if(utUserStatusProc != NULL) {
            utLogMessage("%s", buff);
        } else {
            printf("%s\n", buff);
            fflush(stdout);
        }
    }
    return timerID;
}

/*--------------------------------------------------------------------------------------------------
  Print a message about the length of the current timer.
--------------------------------------------------------------------------------------------------*/
void utStopTimer(
    uint32 timerID,
    char *format,
    ...)
{
    va_list ap;
    char *buff;
    uint32 deltaTime = (uint32)difftime(time(NULL), utTimers[--utTimerDepth]);
    uint32 hours, minutes, seconds;

    if(timerID != utTimerDepth) {
        utWarning("Timer start/stop mismatch!");
    }
    hours = deltaTime/3600;
    deltaTime -= hours*3600;
    minutes = deltaTime/60;
    deltaTime -= minutes*60;
    seconds = deltaTime;
    va_start(ap, format);
    buff = utVsprintf((char *)format, ap);
    va_end(ap);
    utLogMessageType(UT_MESSAGE_INFO, "%s %u:%02u:%02u\n", buff, hours, minutes, seconds);
    if(utUserStatusProc != NULL) {
        utStatus("%s %u:%02u:%02u\n", buff, hours, minutes, seconds);
    } else {
        printf("%s %u:%02u:%02u\n", buff, hours, minutes, seconds);
        fflush(stdout);
    }
}

/*--------------------------------------------------------------------------------------------------
  Function to log errors without a message box to log file.
--------------------------------------------------------------------------------------------------*/
void utLogError(
    char *format,
    ...)
{
    va_list ap;
    char *buff;

    va_start(ap, format);
    buff = utVsprintf(format, ap);
    va_end(ap);
    utLogMessageType(UT_MESSAGE_ERROR, "%s\n", buff);
}

/*--------------------------------------------------------------------------------------------------
  Function to Send a message to the log file. Prefix with a time stamp line and a comment character.
--------------------------------------------------------------------------------------------------*/
void utLogTimeStamp(
    char *message,
    ...)
{
    time_t timeInt = time(NULL);
    char *timeStr = ctime(&timeInt);
    va_list ap;
    char *buff1;

    va_start(ap, message);
    buff1 = utVsprintf(message, ap);
    va_end(ap);
    utLogMessageType(UT_MESSAGE_INFO, "%s : %s", buff1, timeStr);
}

/*--------------------------------------------------------------------------------------------------
  Function to log the status of a tool to log file
--------------------------------------------------------------------------------------------------*/
void utStatus(
    char *format, ...)
{
    va_list ap;
    char *buff;

    va_start(ap, format);
    buff = utVsprintf(format, ap);
    va_end(ap);
    if(utUserStatusProc != NULL) {
        (utUserStatusProc)(buff);
    } else {
        printf("%s", buff);
        fflush(stdout);
    }
}

static char *utExitFileName;
static uint32 utExitLineNum;
uint32 utDummyCount = 0;
/*--------------------------------------------------------------------------------------------------
  Exit with a fatal error message.
--------------------------------------------------------------------------------------------------*/
void utExit_(
    char *format,
    ...)
{
    va_list ap;
    char *buff;

    va_start(ap, format);
    buff = utVsprintf((char *)format, ap);
    va_end(ap);
    if(utUserStatusProc != NULL) {
        utStatus("%s\n", buff);
    } else if(!utMessageHeaderEnabled) {
        printf("%s\n", buff);
        fflush(stdout);
    }
    if(utExitFileName == NULL) {
        utLogMessageType(UT_MESSAGE_EXIT, "%s\n", buff);
    } else {    
        utLogMessageType(UT_MESSAGE_EXIT, "%s:%u %s\n", utExitFileName, utExitLineNum, buff);
    }    
    utDummyCount++;
    if(utDummyCount == 0) {
        return; /* Foo the optimizer into having a return here which is never called.  It's useful for debugging. */
    }
    if(utUserErrProc != NULL) {
        (utUserErrProc)(buff);
    }
    exit(1);
}

/*--------------------------------------------------------------------------------------------------
  Set the file and line globals so that utExit_ can print them. Return utExit_ so it can be called
  with the user's parameters.
--------------------------------------------------------------------------------------------------*/
utExitProcType utSetFileAndLineAndReturnExitFunc(
    char *fileName,
    uint32 lineNum)
{
    utExitFileName = fileName;
    utExitLineNum = lineNum;
    return &utExit_;
}

/*--------------------------------------------------------------------------------------------------
  Post a warning message.
--------------------------------------------------------------------------------------------------*/
void utWarning(
    char *format,
    ...)
{
    va_list ap;
    char *buff;

    va_start(ap, format);
    buff = utVsprintf((char *)format, ap);
    va_end(ap);
    if(utUserStatusProc != NULL) {
       utStatus("%s\n", buff);
    } else if(!utMessageHeaderEnabled) {
        printf("%s\n", buff);
        fflush(stdout);
    }
    utLogMessageType(UT_MESSAGE_WARNING, "%s\n", buff);
    if(utUserWarningProc != NULL) {
        (utUserWarningProc)(buff);
    }
}

/*--------------------------------------------------------------------------------------------------
  Post a note.
--------------------------------------------------------------------------------------------------*/
void utNote(
    char *format,
    ...)
{
    va_list ap;
    char *buff;

    va_start(ap, format);
    buff = utVsprintf((char *)format, ap);
    va_end(ap);
    utReport("%s\n", buff);
}

/*--------------------------------------------------------------------------------------------------
  Post an error message, and longjump.
  This function just returns a value so that it can be used for inline macros as a return value.
--------------------------------------------------------------------------------------------------*/
void utError(
    char *format,
    ...)
{
    va_list ap;
    char *buff;

    va_start(ap, format);
    buff = utVsprintf((char *)format, ap);
    va_end(ap);
    if(utUserStatusProc != NULL) {
        utStatus("%s\n", buff);
    } else if(!utMessageHeaderEnabled) {
        printf("%s\n", buff);
        fflush(stdout);
    }
    utLogMessageType(UT_MESSAGE_ERROR, "%s\n", buff);
    utLongjmp();
}

/*--------------------------------------------------------------------------------------------------
  Post an error message, and exit.  Do not report file and line number, since this is a user error.
--------------------------------------------------------------------------------------------------*/
void utCriticalError(
    char *format,
    ...)
{
    va_list ap;
    char *buff;

    va_start(ap, format);
    buff = utVsprintf((char *)format, ap);
    va_end(ap);
    utLogMessageType(UT_MESSAGE_ERROR, "%s\n", buff);
    exit(1);
}

/*--------------------------------------------------------------------------------------------------
  Cry and die.
--------------------------------------------------------------------------------------------------*/
void utAssert_(
    char *fileName,
    uint32 line,
    char *text)
{
    utMemCheck();
    utExitFileName = NULL;  /* we don't want "util.c" and the utAssert line number to print */
    utExit_("Assertion failed in file %s on line %u: %s", fileName, line, text);
}

/*--------------------------------------------------------------------------------------------------
  Set the name of the Error callback.
--------------------------------------------------------------------------------------------------*/
void utSetErrorCallback(
    utErrorProc errorProc)
{
    utUserErrProc = errorProc;
}

/*--------------------------------------------------------------------------------------------------
  Set the name of the Warning callback.
--------------------------------------------------------------------------------------------------*/
void utSetWarningCallback(
    utErrorProc warningProc)
{
    utUserWarningProc = warningProc;
}

/*--------------------------------------------------------------------------------------------------
  Set the name of the status update callback.
--------------------------------------------------------------------------------------------------*/
void utSetStatusCallback(
    utErrorProc statusProc)
{
    utUserStatusProc = statusProc;
}

/*--------------------------------------------------------------------------------------------------
  Set the name of the log message callback.
--------------------------------------------------------------------------------------------------*/
void utSetLogMessageCallback(
    utErrorProc logMessageProc)
{
    utUserLogMessageProc = logMessageProc;
}

/*--------------------------------------------------------------------------------------------------
  Get the name of the Error callback.
--------------------------------------------------------------------------------------------------*/
utErrorProc utGetErrorCallback(void)
{
    return utUserErrProc;
}

/*--------------------------------------------------------------------------------------------------
  Get the name of the Warning callback.
--------------------------------------------------------------------------------------------------*/
utErrorProc utGetWarningCallback(void)
{
    return utUserWarningProc;
}

/*--------------------------------------------------------------------------------------------------
  Get the name of the status update callback.
--------------------------------------------------------------------------------------------------*/
utErrorProc utGetStatusCallback(void)
{
    return utUserStatusProc;
}

/*--------------------------------------------------------------------------------------------------
  Get the name of the log message callback.
--------------------------------------------------------------------------------------------------*/
utErrorProc utGetLogMessageCallback(void)
{
    return utUserLogMessageProc;
}

/*--------------------------------------------------------------------------------------------------
  Set the name of the logging file and reset it.
--------------------------------------------------------------------------------------------------*/
void utInitDebugFile(
    char *fileName)
{
    utSetDebugFile(fileName);
}

/*--------------------------------------------------------------------------------------------------
  Just return the logi file name.
--------------------------------------------------------------------------------------------------*/
FILE *utGetDebugFile(void)
{
    return utDebugFile;
}

/*--------------------------------------------------------------------------------------------------
  Just return the logi file name.
--------------------------------------------------------------------------------------------------*/
char *utGetDebugFileName(void)
{
    return utDebugFileName;
}

/*--------------------------------------------------------------------------------------------------
  Set the name of the logging file without resetting it.
--------------------------------------------------------------------------------------------------*/
void utSetDebugFile(
    char *fileName)
{
    if(utDebugFile != NULL) {
        fclose(utDebugFile);
        utDebugFile = NULL;
    }
    if(utDebugFileName != NULL) {
        utFree(utDebugFileName);
    }
    utDebugFileName = utNewA(char, strlen(fileName) + 1);
    strcpy(utDebugFileName, fileName);
    utDebugFile = fopen(fileName, "w");
}

/*--------------------------------------------------------------------------------------------------
  Set the name of the logging file and reset it.
--------------------------------------------------------------------------------------------------*/
void utInitReportFile(
    char *fileName)
{
    utSetReportFile(fileName);
}

/*--------------------------------------------------------------------------------------------------
  Just return the logi file name.
--------------------------------------------------------------------------------------------------*/
FILE *utGetReportFile(void)
{
    return utReportFile;
}

/*--------------------------------------------------------------------------------------------------
  Just return the logi file name.
--------------------------------------------------------------------------------------------------*/
char *utGetReportFileName(void)
{
    return utReportFileName;
}

/*--------------------------------------------------------------------------------------------------
  Set the name of the logging file without resetting it.
--------------------------------------------------------------------------------------------------*/
void utSetReportFile(
    char *fileName)
{
    if(utReportFile != NULL) {
        fclose(utReportFile);
    }
    if(utReportFileName != NULL) {
        utFree(utReportFileName);
    }
    utReportFileName = utNewA(char, strlen(fileName) + 1);
    strcpy(utReportFileName, fileName);
    utReportFile = fopen(fileName, "w");
}

/*--------------------------------------------------------------------------------------------------
  Set the name of the logging file and reset it.
--------------------------------------------------------------------------------------------------*/
void utInitLogFile(
    char *fileName)
{
    if(utLogFile != NULL) {
        fclose(utLogFile);
    }
    if(utLogFileName != NULL) {
        utFree(utLogFileName);
    }
    utLogFileName = utAllocString(fileName);
    utLogFile = fopen(fileName, "w");
}

/*--------------------------------------------------------------------------------------------------
  Just return the logi file name.
--------------------------------------------------------------------------------------------------*/
char *utGetLogFileName(void)
{
    return utLogFileName;
}

/*--------------------------------------------------------------------------------------------------
  Just return the logi file name.
--------------------------------------------------------------------------------------------------*/
FILE *utGetLogFile(void)
{
    return utLogFile;
}

/*--------------------------------------------------------------------------------------------------
  Set the name of the logging file without resetting it.
--------------------------------------------------------------------------------------------------*/
void utSetLogFile(
    char *fileName)
{
    if(utLogFile != NULL) {
        fclose(utLogFile);
    }
    if(utLogFileName != NULL) {
        utFree(utLogFileName);
    }
    utLogFileName = utAllocString(fileName);
    utLogFile = fopen(fileName, "a");
}

/*--------------------------------------------------------------------------------------------------
  Set the name of the configuration file directory.
--------------------------------------------------------------------------------------------------*/
void utSetConfigDirectory(
    char *dirName)
{
    if(utConfigDirectory != NULL) {
        utFree(utConfigDirectory);
    }
    utConfigDirectory = utNewA(char, strlen(dirName) + 1);
    strcpy(utConfigDirectory, dirName);
}

/*--------------------------------------------------------------------------------------------------
  Set the name of the executable file directory.
--------------------------------------------------------------------------------------------------*/
void utSetExeDirectory(
    char *dirName)
{
    if(utExeDirectory != NULL) {
        utFree(utExeDirectory);
    }
    utExeDirectory = utNewA(char, strlen(dirName) + 1);
    strcpy(utExeDirectory, dirName);
}

/*--------------------------------------------------------------------------------------------------
  Return the exec directory.
--------------------------------------------------------------------------------------------------*/
char *utGetExeDirectory(void)
{
    return utExeDirectory;
}

/*--------------------------------------------------------------------------------------------------
  Return the config directory.
--------------------------------------------------------------------------------------------------*/
char *utGetConfigDirectory(void)
{
    return utConfigDirectory;
}

/*--------------------------------------------------------------------------------------------------
  Set the complete path and name of the executable file (argv[0])
--------------------------------------------------------------------------------------------------*/
void utSetExeFullPath(
    char *fullName)
{
    if(utExeFullPath != NULL) {
        utFree(utExeFullPath);
    }
    utExeFullPath = utNewA(char, strlen(fullName) + 1);
    strcpy(utExeFullPath, fullName);
}

/*--------------------------------------------------------------------------------------------------
  Return the full path of the executable file.
--------------------------------------------------------------------------------------------------*/
char *utGetExeFullPath(void)
{
    return utExeFullPath;
}

/*--------------------------------------------------------------------------------------------------
  Just set the version variable.
--------------------------------------------------------------------------------------------------*/
void utSetVersion(
    char *version)
{
    if(utVersion != NULL) {
        utFree(utVersion);
    }
    utVersion = utNewA(char, strlen(version) + 1);
    strcpy(utVersion, version);
}

/*--------------------------------------------------------------------------------------------------
  Return the version variable.
--------------------------------------------------------------------------------------------------*/
char *utGetVersion(void)
{
    return utVersion;
}

/*--------------------------------------------------------------------------------------------------
  Initialize generic buffer memory.
--------------------------------------------------------------------------------------------------*/
static void initBuffers(void)
{
    uint16 xBuffer;

    utNextBuffer = 0;
    for(xBuffer = 0; xBuffer < UT_MAX_BUFFERS; xBuffer++) {
        utBufferSizes[xBuffer] = 42;
        utBuffers[xBuffer] = (char *)calloc(utBufferSizes[xBuffer], sizeof(char));
    }
}

/*--------------------------------------------------------------------------------------------------
  Free generic buffer memory.
--------------------------------------------------------------------------------------------------*/
static void freeBuffers(void)
{
    uint16 xBuffer;

    utNextBuffer = 0;
    for(xBuffer = 0; xBuffer < UT_MAX_BUFFERS; xBuffer++) {
        free(utBuffers[xBuffer]);
    }
}

/*--------------------------------------------------------------------------------------------------
  Free memory used by the utility module.
--------------------------------------------------------------------------------------------------*/
void utStop(
    bool reportTimeAndMemory)
{
    if(utInitialized()) {
        if(utSetjmpDepth > 0 && utSetjmpDepth < UT_MAX_SETJMP_DEPTH) {
            utWarning("utClose: utSetjmpDepth != 0 (file %s, line %u)",
               utSetjmpFile[utSetjmpDepth - 1],
               utSetjmpLine[utSetjmpDepth - 1]);
        } else if(utSetjmpDepth != 0) {
            utWarning("utClose: utSetjmpDepth has an invalid value");
            /* prevents crashes */
        }
        if(utTimerDepth != 1) {
          utWarning("utClose: timer started, but never stopped");
        } else if(reportTimeAndMemory) {
          utStopTimer(0, "Process completed in");
        }
        utDatabaseManagerStop();
        utDatabaseStop();
        utFreePersistenceObjects();
        utInitialized() = false;
        if(utReportFile != NULL) {
          fflush(utReportFile);
          fclose(utReportFile);
          utFree(utReportFileName);
          utReportFileName = NULL;
          utReportFile = NULL;
        }
        if(utDebugFile != NULL) {
          fflush(utDebugFile);
          fclose(utDebugFile);
          utFree(utDebugFileName);
          utDebugFileName = NULL;
          utDebugFile = NULL;
        }
        if(utLogFile != NULL) {
          fflush(utLogFile);
          fclose(utLogFile);
          utFree(utLogFileName);
          utLogFileName = NULL;
          utLogFile = NULL;
        }
        if(utConfigDirectory != NULL) {
          utFree(utConfigDirectory);
          utConfigDirectory = NULL;
        }
        if(utExeDirectory != NULL) {
          utFree(utExeDirectory);
          utExeDirectory = NULL;
        }
        if(utExeFullPath != NULL) {
          utFree(utExeFullPath);
          utExeFullPath = NULL;
        }
        if(utVersion != NULL) {
          utFree(utVersion);
          utVersion = NULL;
        }
        utMemStop(reportTimeAndMemory);
        freeBuffers();
    }
}

/*--------------------------------------------------------------------------------------------------
  Initialize symbol table entries to utSymNull.
--------------------------------------------------------------------------------------------------*/
void utInitSymTable(void)
{
    uint32 xSym;

    utTheSymtab = utSymtabAlloc();
    utSymtabAllocTables(utTheSymtab, 2);
    for(xSym = 0; xSym < 2; xSym++) {
        utSymtabSetiTable(utTheSymtab, xSym, utSymNull);
    }
}

/*--------------------------------------------------------------------------------------------------
  Initialize local memory.
--------------------------------------------------------------------------------------------------*/
void utStart(void)
{
    if(!utInitialized()) {
        utMemStart();
        utUserErrProc = 0;
        utInitialized() = true;
        utSetjmpDepth = 0;
        utTimerDepth = 0;
        initBuffers();
        utStartTimer(NULL);
        utInitSeed(4357);
        utAllocPersistenceObjects();
        utDatabaseStart();
        utInitSymTable();
        utDatabaseManagerStart();
    }
}

/*--------------------------------------------------------------------------------------------------
  Just like mtRealloc, but pass file name and line number.
--------------------------------------------------------------------------------------------------*/
void *utReallocTrace(
    void *memPtr,
    size_t num,
    size_t size,
    char *fileName,
    uint32 line)
{
    utMemRef mem;
    size_t newSize = (num + 1)*size;

    if(memPtr == NULL) {
       return utMallocTrace(num, size, fileName, line);
    }
    mem = utqMemPtr(memPtr);
    utMemCheckTrace(fileName, line);
    if(!uttMemExists(mem)) {
        utExit("utRealloc: Bad memory pointer in %s, line %u", fileName, line);
    }
    utdMem(mem, true, fileName, line);
    memPtr = realloc(memPtr, (num + 1)*size);
    if(memPtr != NULL) {
        utBuildMem(memPtr, newSize, true, fileName, line);
    } else {
        utLogMessage("utRealloc: unable to allocate memory %lu.  Total used: %lu",
            num*(size+1), utUsedMem);
    }
    return memPtr;

}

/*--------------------------------------------------------------------------------------------------
  Just like mtMalloc, but pass file name and line number.
--------------------------------------------------------------------------------------------------*/
void *utMallocTrace(
    size_t sStruct,
    size_t size,
    char *fileName,
    uint32 line)
{
    size_t sByte = sStruct*size;
    void *memPtr = malloc(sByte + 1);

    utMemCheckTrace(fileName, line);
    if(memPtr != NULL) {
        utBuildMem(memPtr, sByte + 1, true, fileName, line);
    } else {
        utLogMessage("utRealloc: unable to allocate memory %lu.  Total used: %lu",
            sByte, utUsedMem);
    }
    return memPtr;
}

/*--------------------------------------------------------------------------------------------------
  Just like calloc, but pass file name and line number.
--------------------------------------------------------------------------------------------------*/
void *utCallocTrace(
    size_t sStruct,
    size_t size,
    char *fileName,
    uint32 line)
{
    size_t sByte = sStruct*size;
    void *memPtr;

    memPtr = utMallocTrace(sStruct, size, fileName, line);
    if(memPtr) {
        memset((void *)memPtr, 0, sByte);
    }
    return memPtr;
}

/*--------------------------------------------------------------------------------------------------
  Just like calloc, but pass file name and line number.
--------------------------------------------------------------------------------------------------*/
void *utCallocTraceInitFirst(
    size_t sStruct,
    size_t size,
    char *fileName,
    uint32 line)
{
    size_t sByte = sStruct*size;
    void *memPtr;
    char *cptr;
    char c = (char)170; /*10101010*/
    uint8 x;

    memPtr = utMallocTrace(sStruct, size, fileName, line);
    if(memPtr) {
        memset((void *)memPtr, 0, sByte);
    }
    cptr = (char*)memPtr;
    for(x = 0;x < size;x++) {
        cptr[x] = c;
    }
    return memPtr;
}

/*--------------------------------------------------------------------------------------------------
  Just like mtFree, but pass file name and line number.
--------------------------------------------------------------------------------------------------*/
void utFreeTrace(
    void *memPtr,
    char *fileName,
    uint32 line)
{
    utMemRef mem = utqMemPtr(memPtr);

    utMemCheckTrace(fileName, line);
    if(!uttMemExists(mem)) {
        utExit("utFree: Bad memory pointer in %s, line %u", fileName, line);
    }
    utdMem(mem, true, fileName, line);
    free(memPtr);
}

/*--------------------------------------------------------------------------------------------------
  Just find a hash value for the symbol name.
--------------------------------------------------------------------------------------------------*/
uint32 utHashString(
    char *name)
{
    uint32 hashValue = 0;

    do {
        hashValue = utHashValues(hashValue, *name);
    } while(*name++);
    return hashValue;
}

/*--------------------------------------------------------------------------------------------------
  Just find a hash value for the symbol name.
--------------------------------------------------------------------------------------------------*/
uint32 utHashData(
    void *data,
    uint32 length)
{
    uint8 *dataPtr = (uint8 *)data;
    uint32 hashValue = 0;

    while(length--) {
        hashValue = utHashValues(hashValue, *dataPtr);
        dataPtr++;
    }
    return hashValue;
}

/*--------------------------------------------------------------------------------------------------
  Just find a hash value for the float value.
--------------------------------------------------------------------------------------------------*/
uint32 utHashFloat(
    float value)
{
    uint8 *data = (uint8 *)(void *)&value;

    return utHashData(data, sizeof(float));
}

/*--------------------------------------------------------------------------------------------------
  Just find a hash value for the double value.
--------------------------------------------------------------------------------------------------*/
uint32 utHashDouble(
    double value)
{
    uint8 *data = (uint8 *)(void *)&value;

    return utHashData(data, sizeof(double));
}

/*--------------------------------------------------------------------------------------------------
  Symbol table support.
--------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------
  Find a symbol in the symbol table.
--------------------------------------------------------------------------------------------------*/
static utSym symtabFindSym(
    char *name,
    uint32 hashValue)
{
    uint32 index = (utSymtabGetNumTable(utTheSymtab) - 1) & hashValue;
    utSym sym = utSymtabGetiTable(utTheSymtab, index);

    while(sym != utSymNull) {
        if(!strcmp(name, utSymGetName(sym))) {
            return sym;
        }
        sym = utSymGetNext(sym);
    }
    return utSymNull;
}

/*--------------------------------------------------------------------------------------------------
  Build a new symbol, or return an old one with the same name.
--------------------------------------------------------------------------------------------------*/
static void addSymTableSym(
    utSym sym)
{
    uint32 hashValue = utSymGetHashValue(sym);
    uint32 index = (utSymtabGetNumTable(utTheSymtab) - 1) & hashValue;
    utSym nextSym = utSymtabGetiTable(utTheSymtab, index);

    utSymSetNext(sym, nextSym);
    utSymtabSetiTable(utTheSymtab, index, sym);
    utSymtabSetNumSym(utTheSymtab, utSymtabGetNumSym(utTheSymtab) + 1);
}

/*--------------------------------------------------------------------------------------------------
  Resize the symbol table.  Make it twice as big.
--------------------------------------------------------------------------------------------------*/
static void resizeSymtab(void)
{
    utSym sym;
    uint32 numSyms = utSymtabGetNumTable(utTheSymtab) << 1;
    uint32 xSym;

    utSymtabResizeTables(utTheSymtab, numSyms);
    for(xSym = 0; xSym < numSyms; xSym++) {
        utSymtabSetiTable(utTheSymtab, xSym, utSymNull);
    }
    utSymtabSetNumSym(utTheSymtab, 0);
    utForeachSym(sym) {
        addSymTableSym(sym);
    } utEndSym;
}

/*--------------------------------------------------------------------------------------------------
  Build a new symbol, or return an old one with the same name.
--------------------------------------------------------------------------------------------------*/
utSym utSymCreate(
    char *name)
{
    uint32 hashValue = utHashString(name);
    utSym sym = symtabFindSym(name, hashValue);
    uint32 length;

    if(sym != utSymNull) {
        return sym;
    }
    if(utSymtabGetNumSym(utTheSymtab) == utSymtabGetNumTable(utTheSymtab)) {
        resizeSymtab();
    }
    sym = utSymAlloc();
    length = strlen(name) + 1;
    utSymSetName(sym, name, length);
    utSymSetHashValue(sym, hashValue); 
    addSymTableSym(sym);
    return sym;
}

/*--------------------------------------------------------------------------------------------------
  Build a new symbol, and add it to the symbol table.
--------------------------------------------------------------------------------------------------*/
utSym utSymCreateFormatted(char *format, ...)
{
    char *buff;
    va_list ap;

    va_start(ap, format);
    buff = utVsprintf(format, ap);
    va_end(ap);
    return utSymCreate(buff);
}

/*--------------------------------------------------------------------------------------------------
  Find the lower-case version of the symbol, and create it if it does not exist.
--------------------------------------------------------------------------------------------------*/
utSym utSymGetLowerSym(
    utSym sym)
{
    return utSymCreate(utStringToLowerCase(utSymGetName(sym)));
}

/*--------------------------------------------------------------------------------------------------
  Allocate a buffer for generic use.  There is only a queue of UT_MAX_BUFFERS which can be in use
  at any time.  This is primarily useful for returning strings from subroutines.
--------------------------------------------------------------------------------------------------*/
void *utMakeBuffer_(
    uint32 length)
{
    void *buffer;

    if(length > utBufferSizes[utNextBuffer]) {
        utBufferSizes[utNextBuffer] = length + length/2;
        utBuffers[utNextBuffer] = (char *)realloc(utBuffers[utNextBuffer],
                utBufferSizes[utNextBuffer]*sizeof(char));
    }
    buffer = utBuffers[utNextBuffer];
    if(++utNextBuffer == UT_MAX_BUFFERS) {
        utNextBuffer = 0;
    }
    return buffer;
}

/*--------------------------------------------------------------------------------------------------
  Print a formated string to the file.
--------------------------------------------------------------------------------------------------*/
char *utVsprintf(
    char *format,
    va_list ap)
{
    char buffer[UTSTRLEN];
    char *returnBuffer;

    vsprintf((char *)buffer, (char *)format, ap);
    returnBuffer = utMakeString(strlen(buffer) + 1);
    strcpy(returnBuffer, buffer);
    return returnBuffer;
}

/*--------------------------------------------------------------------------------------------------
  Create a new string which is the specified string with the specified
  suffix appended
  ------------------------------------------------------------------------------------------------*/
char* utAddSuffix(
    char* originalName,
    char* suffix)
{
    char* result = NULL;
    uint32 length = strlen(originalName) + strlen(suffix) + 1;

    result = (char*)utMalloc(1,length);
    strcpy(result, originalName);
    strcat(result, suffix);
    
    return result;
}

/*--------------------------------------------------------------------------------------------------
  Just replace the file suffix.
--------------------------------------------------------------------------------------------------*/
char *utReplaceSuffix(
    char *originalName,
    char *newSuffix)
{
    uint32 length = strlen(originalName);
    char *buffer = utMakeBuffer_(length + strlen(newSuffix) + 1);
    char *endPtr;

    strcpy(buffer, originalName);
    endPtr = buffer + length;
    while(endPtr > buffer && *endPtr != '.' && *endPtr != UTDIRSEP) {
        endPtr--;
    }
    if(*endPtr != '.') {
        endPtr = buffer + length;
    }
    strcpy(endPtr, newSuffix);
    return buffer;
}

/*--------------------------------------------------------------------------------------------------
  Return a temporary copy of a string.
--------------------------------------------------------------------------------------------------*/
char *utCopyString(
    char *string)
{
    char *buffer;

    if(string == NULL) {
        return NULL;
    }
    buffer = utMakeString(strlen(string) + 1);
    strcpy(buffer, string);
    return buffer;
}

/*--------------------------------------------------------------------------------------------------
  Make a new string by concatenating the two old ones.
--------------------------------------------------------------------------------------------------*/
char *utCatStrings(
    char *string1,
    char *string2)
{
    return utSprintf("%s%s", string1, string2);
}

/*--------------------------------------------------------------------------------------------------
  Convert a string to upper case.
--------------------------------------------------------------------------------------------------*/
char *utStringToUpperCase(
    char *string)
{
    char *buffer = utCopyString(string);
    char *p = buffer;
    
    while(*p != '\0') {
        *p = toupper(*p);
        p++;
    }
    return buffer;
}

/*--------------------------------------------------------------------------------------------------
  Convert a string to lower case.
--------------------------------------------------------------------------------------------------*/
char *utStringToLowerCase(
    char *string)
{
    char *buffer = utCopyString(string);
    char *p = buffer;
    
    while(*p != '\0') {
        *p = tolower(*p);
        p++;
    }
    return buffer;
}

/*--------------------------------------------------------------------------------------------------
  Return the compile time for the executable. This is in one place only for consistancy.
--------------------------------------------------------------------------------------------------*/
char *utGetCompileTime(void)
{
    return __DATE__ " " __TIME__;
}

/*--------------------------------------------------------------------------------------------------
  Return the date and time as an ASCII string.
--------------------------------------------------------------------------------------------------*/
char *utGetDateAndTime(void)
{
    time_t timeval = time(NULL);
    struct tm *theTime = localtime(&timeval);

    return utSprintf("%02u/%02u/%02u %02u:%02u:%02u", theTime->tm_mon + 1, theTime->tm_mday,
        theTime->tm_year % 100, theTime->tm_hour, theTime->tm_min, theTime->tm_sec);
}

/*--------------------------------------------------------------------------------------------------
  Return the base name of the path name.
--------------------------------------------------------------------------------------------------*/
char *utBaseName(
    char *name)
{
    char *left = strrchr(name, UTDIRSEP);
    char *right = name + strlen(name);
    char *buffer;

    if(left == NULL) {
        return utCopyString(name);
    }
    left++;
    buffer = utMakeString(right - left + 1);
    strncpy(buffer, left, right - left);
    buffer[right - left] = '\0';
    return buffer;
}

/*--------------------------------------------------------------------------------------------------
  Return the suffix of the filename.
--------------------------------------------------------------------------------------------------*/
char *utSuffix(
    char *name)
{
    uint32 i = 0;
    uint32 start = UINT32_MAX;
    char c = name[i];

    while(c != 0) {
        i++;
        if(c == '.') {
            start = i;
        } else if(c == UTDIRSEP) {
            start = UINT32_MAX;
        }
        c = name[i];
    }
    if(start == UINT32_MAX) {
        return NULL;
    }
    return utCopyString(name + start);
}

/*--------------------------------------------------------------------------------------------------
  Return the directory name of the path name.
--------------------------------------------------------------------------------------------------*/
char *utDirName(
    char *name)
{
    char *end = strrchr(name, UTDIRSEP);
    char *buffer;

    if(end == NULL) {
        return ".";
    }
    buffer = utMakeString(end - name + 1);
    strncpy(buffer, name, end - name);
    buffer[end - name] = '\0';
    return buffer;
}

/*--------------------------------------------------------------------------------------------------
  Just determine if the file exists.
--------------------------------------------------------------------------------------------------*/
bool utFileExists(
    char *fileName)
{
    FILE *file;

    if(fileName == NULL || *fileName == '\0') {
        return false;
    }
    file = fopen(fileName, "r");
    if(file != NULL) {
        fclose(file);
        return true;
    }
    return false;
}

/*--------------------------------------------------------------------------------------------------
  Find a file name in the directory and return the full path if it exists.  Otherwise return NULL.
--------------------------------------------------------------------------------------------------*/
static char *findFileInDirectory(
    char *fileName,
    char *dirName)
{
    char *name = utSprintf("%s%c%s", dirName, UTDIRSEP, fileName);

    if(utAccess(name, NULL)) {
        return name;
    }
    return NULL;
}

/*--------------------------------------------------------------------------------------------------
  Find a file in the path that has the mode.
--------------------------------------------------------------------------------------------------*/
char *utFindInPath(
    char *name,
    char *path)
{
    char *buf = utCopyString(path);
    char *p = buf;
    char *next, *fileName, *directory;

    while(*p != '\0') {
        next = strchr(p, UTPATHSEP);
        if(next != NULL) {
            *next++ = '\0';
        }
        directory = utExpandEnvVariables(p);
        fileName = findFileInDirectory(name, directory);
        if(fileName != NULL) {
            return fileName;
        }
        p = next;
    }
    return NULL;
}

/*--------------------------------------------------------------------------------------------------
  Find the matching '}' in the string.
--------------------------------------------------------------------------------------------------*/
static char *findMatchingBracket(
    char *string)
{
    uint32 numBraces = 0;
    char c;

    utDo {
        c = *string;
        if(c == '{') {
            numBraces++;
        } else if(c == '}') {
            numBraces--;
        }
    } utWhile(numBraces != 0) {
        string++;
    } utRepeat;
    return string;
}

/*--------------------------------------------------------------------------------------------------
  Find the first non alpha-numeric character.
--------------------------------------------------------------------------------------------------*/
static char *findFirstNonAlnumChar(
    char *string)
{
    while(isalnum(*string)) {
        string++;
    }
    return string;
}

/*--------------------------------------------------------------------------------------------------
  Expand the string's environment variable.
--------------------------------------------------------------------------------------------------*/
static char *expandString(
    char *string,
    char *varStart)
{
    char *ending;
    char *p;

    *varStart++ = '\0';
    if(*varStart == '{') {
        ending = findMatchingBracket(varStart);
        varStart++;
        if(ending == NULL) {
            utWarning("Variable %s does not have a matching '}'" , varStart);
            return string;
        }
        *ending++ = '\0';
    } else {
        ending = findFirstNonAlnumChar(varStart);
        if(ending == NULL) {
            ending = "";
        } else {
            p = ending;
            ending = utCopyString(ending);
            *p = '\0'; /* To terminate varStart */
        }
    }
    return utSprintf("%s%s%s", string, getenv(varStart), ending);
}

/*--------------------------------------------------------------------------------------------------
  Expand the string to replace environment variables with their values.
--------------------------------------------------------------------------------------------------*/
char *utExpandEnvVariables(
    char *string)
{
    char *p;
    bool changed;

    string = utCopyString(string); /* To make a writable copy */
    if(string == NULL) {
        return NULL;
    }
    do {
        changed = false;
        p = strchr(string, '$');
        if(p != NULL) {
            changed = true;
            string = expandString(string, p);
        }
    } while(changed);
    return string;
}

/*--------------------------------------------------------------------------------------------------
  Create a dynamic array.
--------------------------------------------------------------------------------------------------*/
utDynarray utDynarrayCreate_(
    uint16 valueSize)
{
    utDynarray dynarray = utDynarrayAlloc();

    utDynarraySetSize(dynarray, valueSize);
    return dynarray;
}

/*--------------------------------------------------------------------------------------------------
  Expand the string to replace environment variables with their values.
--------------------------------------------------------------------------------------------------*/
void utDynarrayResize(
    utDynarray dynarray,
    uint32 newSize)
{
    uint32 numValues = newSize*utDynarrayGetSize(dynarray);

    if(utDynarrayGetNumValue(dynarray) == 0) {
        utDynarrayAllocValues(dynarray, numValues);
    } else {
        utDynarrayResizeValues(dynarray, numValues);
    }
}

/*--------------------------------------------------------------------------------------------------
  Build a new symbol, and add it to the symbol table.
--------------------------------------------------------------------------------------------------*/
utSym utUniqueSymCreate(
    char *name,
    char *suffix)
{
    utSym baseSym, sym;
    char buf[UTSTRLEN];
    char *tail;
    uint32 count;
    uint32 hashValue;

    strcpy(buf, name);
    tail = buf + strlen(buf) - 1;
    while(isdigit(*tail)) {
        tail--;
    }
    tail++;
    strcpy(tail, suffix);
    hashValue = utHashString(buf);
    baseSym = symtabFindSym(buf, hashValue);
    if(baseSym == utSymNull) {
        return utSymCreate(buf);
    }
    tail += strlen(suffix);
    count = utSymNextIndex;
    do {
        sprintf(tail, "%u", count);
        count++;
        hashValue = utHashString(buf);
        sym = symtabFindSym(buf, hashValue);
    } while (sym != utSymNull);
    utSymNextIndex = count;
    return utSymCreate(buf);
}

/*--------------------------------------------------------------------------------------------------
  Convert directory path separation characters from either '\\' or '/' to UTDIRSEP.
--------------------------------------------------------------------------------------------------*/
char *utConvertDirSepChars(
    char *path)
{
    char *buffer = utCopyString(path);
    char *p = buffer;
    char c;

    utDo {
        c = *p;
    } utWhile(c != '\0') {
        if(c == '\\' || c == '/') {
            *p = UTDIRSEP;
        }
        p++;
    } utRepeat;
    return buffer;
}
