/*
 * This file was written by Bill Cox, originally in 1991, and maintained since.  It is hereby
 * placed into the public domain.
 */

/*==================================================================================================
   Module : Virtual Operating System
   Purpose: Hide system specific operations from other modules.  This should help in portability.
==================================================================================================*/

#ifndef DD_UTIL_H
#define DD_UTIL_H

#if __cplusplus
extern "C" {
#endif

#include <stdarg.h> /* For utVsprintf declaration */
#include <setjmp.h> /* For utSetjmp */
#include <stdio.h> /* Everyone uses it */
#include <string.h> /* Has memset, used in generated macros */

/*--------------------------------------------------------------------------------------------------
  Datadraw defined Data types
--------------------------------------------------------------------------------------------------*/

#ifndef UTTYPES_H
#include "uttypes.h"
#endif

/*--------------------------------------------------------------------------------------------------
  Basic symbol table support.  Note that user applications don't need to worry about which
  header gets included here, since they should only be reading from symbols.
--------------------------------------------------------------------------------------------------*/
/* These functions are used in generated header files, so declare them first */
/* Log a message to the log file and exit if false is passed. */
void utAssert_(char *fileName, uint32 line, char *text);
#define utAssert(assertion) ((void)(utLikely(assertion) || (utAssert_(__FILE__, __LINE__, #assertion), 0)))
void utRecordField(uint8 moduleID, uint16 fieldIndex, uint64 objectNumber, bool undo);
void utRecordArray(uint8 moduleID, uint16 fieldIndex, uint32 dataIndex, uint32 length, bool undo);
void utRecordGlobal(uint8 moduleID, uint8 numBytes, void *location, bool undo);
void utRecordResize(uint8 moduleID, uint16 fieldIndex, uint64 length, bool undo);
#if defined(UT_USE_UTDATABASEUP_H)
#include "utdatabaseup.h"
#elif defined(UT_USE_UTDATABASEP_H)
#include "utdatabasep.h"
#elif defined(UT_USE_UTDATABASEU_H)
#include "utdatabasep.h"
#else
#include "utdatabase.h"
#endif

utSym utSymCreate(char *name);
utSym utSymCreateFormatted(char *format, ...);
utSym utUniqueSymCreate(char *name, char *suffix);
utSym utSymGetLowerSym(utSym sym);

/*--------------------------------------------------------------------------------------------------
  Basic memory management support.
--------------------------------------------------------------------------------------------------*/
#ifndef UTMEM_H
#include "utmem.h"
#endif

#ifdef DMALLOC
#include "dmalloc.h"
#endif

/*--------------------------------------------------------------------------------------------------
  The max size of a string which the utFprintf, utExit, etc functions can print.
--------------------------------------------------------------------------------------------------*/
#define UTSTRLEN 4096

extern uint32 utDebugVal;
extern uint32 utVerboseVal;

/*--------------------------------------------------------------------------------------------------
  Initialization, cleaning up.
--------------------------------------------------------------------------------------------------*/
void utStop(bool reportTimeAndMemory);
void utStart(void);
extern bool _utInitialized;
#define utInitialized() _utInitialized


/*--------------------------------------------------------------------------------------------------
  Functions you may want to call after initialization.
--------------------------------------------------------------------------------------------------*/
void utSetConfigDirectory(char *dirName);
void utSetExeFullPath(char *fullName);
void utSetVersion(char *version);
char *utGetVersion(void);
char *utGetExeDirectory(void);
char *utGetConfigDirectory(void);
char *utGetExeFullPath(void);

/*--------------------------------------------------------------------------------------------------
  Error Callback function to be provided by the user.
--------------------------------------------------------------------------------------------------*/
typedef void(*utErrorProc)(char *message);
typedef bool(*utMessageYesNoProc)(char *message);
typedef void(*utExitProc)(void);

/*--------------------------------------------------------------------------------------------------
  Basic interface to file system.
--------------------------------------------------------------------------------------------------*/
char *utGetcwd(void);
bool utChdir(char *dirName);
bool utFileExists(char *fileName);
bool utDirectoryExists(char *dirName);
bool utAccess(char *name, char *mode);
uint64 utFindFileSize(char *fileName);
void utForeachDirectoryFile(char *dirName, void (*func)(char *dirName, char *fileName));
char *utExecPath(char *name);
char *utFullPath(char *relativePath);
char *utFindInPath(char *name, char *path);
void utTruncateFile(char *fileName, uint64 length);
bool utDeleteFile(char *fileName);
bool utMakeDirectory(char *dirName);

/*--------------------------------------------------------------------------------------------------
  Portable interface to launch an application.
--------------------------------------------------------------------------------------------------*/
bool utLaunchApp(char* cmdLine, char *wkgDir);

/*--------------------------------------------------------------------------------------------------
  Memory allocation.
  There are things called malloc, calloc and free, but they deal in the
  local heap, thus are to be avoided.
--------------------------------------------------------------------------------------------------*/
extern uint64 utUsedMem;
#define utMalloc(sStruct, size) utMallocTrace(sStruct, size, __FILE__, __LINE__)
#define utCalloc(sStruct, size) utCallocTrace(sStruct, size, \
      __FILE__, __LINE__)
#define utCallocInitFirst(sStruct, size) utCallocTraceInitFirst(sStruct, size, \
      __FILE__, __LINE__)
#define utRealloc(mem, numBlocks, size) \
      utReallocTrace((void *)mem, numBlocks, size, __FILE__, __LINE__)
#define utResizeArray(array, num) \
        ((array) = utRealloc((void *)(array), (num), sizeof(*(array))))
#define utNew(type) (type *)utCalloc(1, sizeof(type))
#define utNewA(type, num) (type *)utCalloc((num), sizeof(type))
#define utNewAInitFirst(type, num) (type *)utCallocInitFirst((num), sizeof(type))
#define utFree(p) utFreeTrace(p, __FILE__, __LINE__)
#define utAllocString(string) strcpy(utNewA(char, strlen(string) + 1), string)

void * utReallocTrace(void *memPtr, size_t numBlocks, size_t size, char *fileName,
    uint32 line);
void *utMallocTrace(size_t sStruct, size_t size, char *fileName, uint32 line);
void *utCallocTrace(size_t sStruct, size_t size, char *fileName, uint32 line);
void *utCallocTraceInitFirst(size_t sStruct, size_t size, char *fileName, uint32 line);
void utFreeTrace(void *memPtr, char *fileName, uint32 line);

/* maxmimum memory usage */
extern uint32 utmByte;

/*--------------------------------------------------------------------------------------------------
  Random number support.
--------------------------------------------------------------------------------------------------*/
void utInitSeed(uint32 seed);
uint32 utRand(void);
#define utRandN(n) (utRand() % (n))
#define utRandBool() ((bool) (utRand() & 1))
uint8 *utRealRandom(uint32 length);

/*--------------------------------------------------------------------------------------------------
  String and temporary buffer manipulation.
--------------------------------------------------------------------------------------------------*/
/* These use a queue of buffers */
void *utMakeBuffer_(uint32 length);
#define utNewBufA(type, num) (type *)utMakeBuffer_((num)*sizeof(type))
#define utNewBuf(type) (type *)utMakeBuffer_(sizeof(type))
#define utMakeString(length) (char *)utMakeBuffer_(length)
char *utCopyString(char *string);
char *utCatStrings(char *string1, char *string2);
char *utStringToUpperCase(char *string);
char *utStringToLowerCase(char *string);
char *utSprintf(char *format, ...);
char *utReplaceSuffix(char *originalName, char *newSuffix);
char *utAddSuffix(char *originalName, char *suffix);
char *utSuffix(char *name);
char *utBaseName(char *name);
char *utDirName(char *name);
char *utExpandEnvVariables(char *string);
char *utVsprintf(char *format, va_list ap);
void utSetEnvironmentVariable(char *name, char *value);
char *utGetEnvironmentVariable(char *name);
char *utFindHexString(uint8 *values, uint32 size);
bool utReadHex(uint8 *dest, char *value, uint32 size);
char *utMemoryUnits(uint64 memory);
bool utParseInteger(int64 *dest, char *string);
char *utConvertDirSepChars(char *path);

/*--------------------------------------------------------------------------------------------------
  Message loging, error reporting.
--------------------------------------------------------------------------------------------------*/
typedef enum {
    UT_MESSAGE_INFO,
    UT_MESSAGE_WARNING,
    UT_MESSAGE_ERROR,
    UT_MESSAGE_EXIT,
    UT_MESSAGE_DETAILS,
    UT_MESSAGE_INTERNAL,
    UT_MESSAGE_REPORT,
} utMessageType;
void utEnableMessageHeaders(bool value);
void utLogMessageType(utMessageType msgType, bool alsoLog, char *format, ...);
void utLogMessage(char *format, ...);
void utLogString(char *format, ...);
void utLogDebug(char *format, ...);
char *utGetCompileTime(void);
char *utCompileTime(void);
char *utGetDateAndTime(void);
bool utDebug(char *format,... );
void utLogError(char *format, ...);
void utLogTimeStamp(char *message, ...);
uint32 utStartTimer(char *message, ...);
void utStopTimer(uint32 timerID, char *message, ...);
void utStatus(char *format, ...);
void utExit_ (char *format, ...);
typedef void(*utExitProcType)(char *format, ...);
utExitProcType utSetFileAndLineAndReturnExitFunc(char *fileName, uint32 lineNum);
#define utExit utSetFileAndLineAndReturnExitFunc(__FILE__, __LINE__)
    void utWarning_(uint32 count, char *format, ...);
#define utMaxWarnings 10
#define  utWarning(format, ...) {               \
        static int32 count = 0;                 \
        if(count <= utMaxWarnings) {            \
            ++count;                            \
        }                                       \
        utWarning_(count, format, ## __VA_ARGS__);      \
    }
void utNote(char *format, ...);
void utError(char *format, ...);
void utCriticalError(char *format, ...);
void utReport(char *format, ...);
void utSetErrorCallback(utErrorProc errorProc);
void utSetWarningCallback(utErrorProc warningProc);
void utSetStatusCallback(utErrorProc statusProc);
void utSetLogMessageCallback(utErrorProc logMessageProc);
utErrorProc utGetErrorCallback(void);
utErrorProc utGetWarningCallback(void);
utErrorProc utGetStatusCallback(void);
utErrorProc utGetLogMessageCallback(void);
void utInitLogFile(char *fileName);
void utSetLogFile(char *fileName);
FILE *utGetLogFile(void);
char *utGetLogFileName(void);
void utInitReportFile(char *fileName);
void utSetReportFile(char *fileName);
FILE *utGetReportFile(void);
char *utGetReportFileName(void);
void utInitDebugFile(char *fileName);
void utSetDebugFile(char *fileName);
FILE *utGetDebugFile(void);
char *utGetDebugFileName(void);
#define utIfDebug(minVal) if(utUnlikely(utDebugVal >= minVal))
#define utIfVerbose(minVal) if(utUnlikely(utVerboseVal >= minVal))

#define UT_MAX_SETJMP_DEPTH 5
#if defined(_WINDOWS) && !defined(_WIN32)
#if !defined(CATCHBUF)
typedef int CATCHBUF[9];
#endif
extern CATCHBUF utCatchBuf[UT_MAX_SETJMP_DEPTH];
#else
extern jmp_buf utJmpBuf[UT_MAX_SETJMP_DEPTH];
#endif

/*--------------------------------------------------------------------------------------------------
  Macros for computing simple hashed signatures.  Hash signatures are uint32s.
--------------------------------------------------------------------------------------------------*/
#define utHashUint32(value) (((uint32)(value))*1103515245 + 12345)
#define utHashValues(hash1, hash2) (((uint32)(hash1) ^ (uint32)(hash2))*1103515245 + 12345)
uint32 utHashData(void *data, uint32 length);
uint32 utHashString(char *string);
uint32 utHashFloat(float value);
uint32 utHashDouble(double value);

/*--------------------------------------------------------------------------------------------------
  Setjmp/longjmp stack.
--------------------------------------------------------------------------------------------------*/
extern int16 utSetjmpDepth;
extern uint32 utSetjmpLine[UT_MAX_SETJMP_DEPTH];
extern char *utSetjmpFile[UT_MAX_SETJMP_DEPTH];
#define utUnsetjmp() (utSetjmpDepth--,\
      !strcmp(utSetjmpFile[utSetjmpDepth],__FILE__) ||\
      (utExit("Mismatched utUnsetjmp in file %s, line %u", __FILE__, __LINE__), 1))
#define utSetjmp() (++utSetjmpDepth,\
      utSetjmpFile[utSetjmpDepth - 1] = __FILE__,\
      utSetjmpLine[utSetjmpDepth - 1] = __LINE__,\
      setjmp(utJmpBuf[utSetjmpDepth - 1]))
void utLongjmp(void);

/*--------------------------------------------------------------------------------------------------
  Functions supporting database persistence.
--------------------------------------------------------------------------------------------------*/

#ifndef UTPERSIST_H
#include "utpersist.h"
#endif

/* These are used by DataDraw to manage the database, and can generally be ignored */
uint8 utRegisterModule(char *prefix, bool persistent, uint32 hashValue, uint16 numClasses, uint16 numFields,
    uint16 numEnums, uint16 globalSize, void *globalData, void (*start)(void), void (*stop)(void));
void utUnregisterModule(uint8 moduleID);
void utRegisterClass(char *name, uint16 numFields, void *numUsedPtr, void *numAllocatedPtr,
    void *firstFreePtr, uint16 nextFreeFieldIndex, uint8 referenceSize,
    uint64 (*constructor)(void), void (*destructor)(uint64 objectIndex));
void utRegisterBaseClass(char *baseModulePrefix, uint16 baseClassIndex);
void utRegisterField(char *name, void *arrayPtr, uint32 size, utFieldType type,
    char *destName);
void utSetFieldHidden(void);
void utRegisterArray(uint32 *numUsedPtr, uint32 *numAllocatedPtr,
    void *(*getValues)(uint64 objectNumber, uint32 *numValues),
    void *(*allocValues)(uint64 objectNumber, uint32 numValues),
    void (*compactArray)(void));
void utRegisterFixedArray(uint32 length, void *(*getValues)(uint64 objectNumber, uint32 *numValues));
void utRegisterEnum(char *name, uint16 numEntries);
void utRegisterEntry(char *name, uint32 value);
void utRegisterUnion(char *switchFieldName, uint16 numCases);
void utRegisterUnionCase(uint32 value, utFieldType type, uint32 size);
void utAllocPersistenceObjects(void);
void utFreePersistenceObjects(void);
void utInitSymTable(void);
void utDatabaseManagerStart(void);
void utDatabaseManagerStop(void);
void utDumpRecentChanges(void);
void utDumpCommand(uint8 *command);

/* These are the useful functions */
bool utStartPersistence(char *directory, bool useTextDatabaseFormat, bool keepBackup);
void utStopPersistence(void);
uint32 utUndo(uint32 numChanges);
uint32 utRedo(uint32 numChanges);
void utStartUndoRedo(void);
void utStopUndoRedo(void);
void utTransactionComplete(bool flushToDisk);
void utLoadBinaryDatabase(FILE *file);
void utSaveBinaryDatabase(FILE *file);
void utLoadTextDatabase(FILE *file);
void utSaveTextDatabase(FILE *file);
void utManager(void);
void utDatabaseShowObject(char *modulePrefix, char *className, uint64 objectNumber);
void utResetDatabase(void);
void utCompactDatabase(void);

/* Some dynamic array stuff */
#ifdef DD_DEBUG
/* this double-evaluates and might cause side-effects */
#define utDynarrayGetiValueType(Dynarray, type, x) (utAssert((uint32)(x) < utDynarrayGetSize( \
    Dynarray)), ((type *)(utDynarrayGetValues(Dynarray)))[x])
#define utDynarraySetiValueType(Dynarray, type, x, Value) (utAssert((uint32)(x) < \
    utDynarrayGetSize(Dynarray)), ((type *)(utDynarrayGetValues(Dynarray)))[x] = (Value))
#else
#define utDynarrayGetiValueType(Dynarray, type, x) (((type *)(utDynarrayGetValues(Dynarray)))[x])
#define utDynarraySetiValueType(Dynarray, type, x, Value) \
    (((type *)utDynarrayGetValues(Dynarray))[x] = (Value))
#endif
utDynarray utDynarrayCreate_(uint16 valueSize);
#define utDynarrayCreate(type) utDynarrayCreate_(sizeof(type))
void utDynarrayResize(utDynarray dynarray, uint32 newSize);
#define utDynarrayAppendValue(dynarray, type, value) \
    (utDynarrayGetUsedValue(dynarray) == utDynarrayGetSize(dynarray) && \
    (utDynarrayResize((dynarray), 1 + utDynarrayGetSize(dynarray) + \
    (utDynarrayGetSize(dynarray) >> 1)), true),\
    utDynarraySetiValueType(dynarray, type, utDynarrayGetUsedValue(dynarray), (value)),\
    utDynarraySetUsedValue(dynarray, utDynarrayGetUsedValue(dynarray) + 1))
#define utForeachDynarrayTypeValue(dynarray, type, value) \
{\
    uint32 _xValue;\
    for(_xValue = 0; _xValue < utDynarrayGetUsedValue(dynarray); _xValue++) {\
        (value) = utDynarrayGetiValueType((dynarray), type, _xValue);
#define utEndDynarrayTypeValue }}
#define utDynarrayCopy(destDynarray, sourceDynarray, type) { \
    uint32 _xValue; \
    if (utDynarrayGetSize(destDynarray) < utDynarrayGetSize(sourceDynarray)) { \
        utDynarrayResize(destDynarray, utDynarrayGetSize(sourceDynarray)); \
    } \
    utDynarraySetUsedValue(destDynarray, utDynarrayGetUsedValue(sourceDynarray)); \
    for(_xValue = 0; _xValue < utDynarrayGetUsedValue(sourceDynarray); _xValue++) { \
        utDynarraySetiValueType((destDynarray), type, _xValue, \
            utDynarrayGetiValueType((sourceDynarray), type, _xValue)); \
    } \
}

#if __cplusplus
}
#endif

#endif

