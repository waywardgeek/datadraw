/*
 * This file was written by Bill Cox, originally in 1991, and maintained since.  It is hereby
 * placed into the public domain.
 */

/*============================================================================
   Module : Datadraw
   Purpose: Includes UNIX dependent functions.
============================================================================*/

#include <sys/stat.h>
#include <sys/types.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <dirent.h>
#include <unistd.h>
#include "ddutil.h"

jmp_buf utJmpBuf[UT_MAX_SETJMP_DEPTH];

/*--------------------------------------------------------------------------------------------------
  Generate a real randome number string.
--------------------------------------------------------------------------------------------------*/
uint8 *utRealRandom(
    uint32 length)
{
    uint8 *randBuff = utNewBufA(uint8, length);
    uint32 xByte;
    uint32 seed = (uint32)time(NULL);

    utInitSeed(seed);
    for(xByte = 0; xByte < length; xByte++) {
        randBuff[xByte] = utRandN(0x100);
    }
    return randBuff;
}

/*--------------------------------------------------------------------------------------------------
  Decrement the setjmp stack, and longjmp.
--------------------------------------------------------------------------------------------------*/
void utLongjmp (void)
{
   utSetjmpDepth--;
   if (utSetjmpDepth < 0 || utSetjmpDepth > UT_MAX_SETJMP_DEPTH) {
      utExit("utLongjmp: No utSetjmp for utLongjmp");
   } else {
      longjmp(utJmpBuf[utSetjmpDepth], 1);
   }
}

/*--------------------------------------------------------------------------------------------------
  Take the relative path, and return the full path.
--------------------------------------------------------------------------------------------------*/
char *utFullPath(
    char *relativePath)
{ 
    char *fileName = relativePath;
    char *dirName = utGetcwd();

    if(*relativePath == UTDIRSEP || (*relativePath != '\0' && relativePath[1] == ':')) {
        return utCopyString(relativePath);
    }
    if(fileName == NULL) {
        return utSprintf("%s%c%s", dirName, UTDIRSEP, relativePath);
    }
    while(!strncmp(fileName, ".." UTDIRSEP_STRING, 3)) {
        fileName = fileName + 3;
        dirName = utDirName(dirName);
        if(dirName == NULL) {
            return utSprintf("%s%c%s", dirName, UTDIRSEP, relativePath);
        }
    }
    return utSprintf("%s%c%s", dirName, UTDIRSEP, fileName);
}

/*--------------------------------------------------------------------------------------------------
  Allocate a block of memory. Don't fill with 0's.
--------------------------------------------------------------------------------------------------*/
void *mtCalloc(
   uint32 numBlocks,
   uint16 size)
{
   return calloc(numBlocks, size);
}

/*--------------------------------------------------------------------------------------------------
  Allocate a block of memory. Don't fill with 0's.
--------------------------------------------------------------------------------------------------*/
void *mtMalloc(
   uint32 numBlocks,
   uint16 size)
{
   return malloc(numBlocks*size);
}

/*--------------------------------------------------------------------------------------------------
  Reallocate and expand memory.
--------------------------------------------------------------------------------------------------*/
void *mtRealloc(
   void *mem,
   uint32 numBlocks,
   uint16 size)
{
   return realloc((char *)mem, (size_t)(numBlocks * size));
}

/*--------------------------------------------------------------------------------------------------
  Free a block of memory.  Return TRUE if sucess.
--------------------------------------------------------------------------------------------------*/
bool mtFree(
   void *p)
{
   free((char *)p);
   return true;
}

/*--------------------------------------------------------------------------------------------------
  Launches an application. Simple interface - no params except
             command line & wkgDir. Command line can include args. Wkg dir
             can be NULL.
--------------------------------------------------------------------------------------------------*/
bool utLaunchApp (
   char* cmdLine,
   char* wkgDir)
{
   return (bool) system(cmdLine);
}

/*--------------------------------------------------------------------------------------------------
  Find the current working directory name.
--------------------------------------------------------------------------------------------------*/
char *utGetcwd(void)
{
    char buffer[UTSTRLEN];

    getcwd(buffer, UTSTRLEN);
    return utConvertDirSepChars(buffer);
}

/*--------------------------------------------------------------------------------------------------
  Change to the directory.
--------------------------------------------------------------------------------------------------*/
bool utChdir(
    char *dirName)
{
    if(!chdir(dirName)) {
        return false;
    }
    return true;
}

/*--------------------------------------------------------------------------------------------------
  Determine the access to a file
--------------------------------------------------------------------------------------------------*/
bool utAccess(
    char *name,
    char *mode)
{
    if(mode == NULL) {
        return access(name, 0) == 0;
    }
    if(!strcmp(mode, "r")) {
        return access(name, R_OK) == 0;
    }
    if(!strcmp(mode, "w")) {
        return access(name, W_OK) == 0;
    }
    if(!strcmp(mode, "x")) {
        return access(name, X_OK) == 0;
    }
    utExit("Unknown mode passed to utAccess");
    return false; /* Dummy return */
}

/*--------------------------------------------------------------------------------------------------
  Return the path name which an executable resides.
--------------------------------------------------------------------------------------------------*/
char *utExecPath(
    char *program)
{
    return program; /* On Windows, we have the real path */
}

/*--------------------------------------------------------------------------------------------------
  Set the environment variable.
--------------------------------------------------------------------------------------------------*/
void utSetEnvironmentVariable(
    char *name,
    char *value)
{
    putenv(utSprintf("%s=%s", name, value));
}

/*--------------------------------------------------------------------------------------------------
  Get the environment variable.  Return NULL if it is not set.
--------------------------------------------------------------------------------------------------*/
char *utGetEnvironmentVariable(
    char *name)
{
    return getenv(name);
}

/*--------------------------------------------------------------------------------------------------
  Find the size of the file.
--------------------------------------------------------------------------------------------------*/
uint64 utFindFileSize(
    char *fileName)
{
    struct stat statbuf;

    if(stat(fileName, &statbuf)) {
        return 0;
    }
    return statbuf.st_size;
}

/*--------------------------------------------------------------------------------------------------
  Find the size of the file.
--------------------------------------------------------------------------------------------------*/
bool utDirectoryExists(
    char *dirName)
{
    struct stat statbuf;

    if(stat(dirName, &statbuf)) {
        return false;
    }
    return S_ISDIR(statbuf.st_mode);
}

/*--------------------------------------------------------------------------------------------------
  Truncate the file.
--------------------------------------------------------------------------------------------------*/
void utTruncateFile(
    char *fileName,
    uint64 length)
{
    int file = open(fileName, 0);

    if(file == -1) {
        utWarning("utTruncateFile: could not open file %s", fileName);
        return;
    }
    chsize(file, length);
    close(file);
}

/*--------------------------------------------------------------------------------------------------
  Delete the file.
--------------------------------------------------------------------------------------------------*/
bool utDeleteFile(
    char *fileName)
{
    return !unlink(fileName);
}
