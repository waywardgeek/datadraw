/*
 * This file was written by Bill Cox, originally in 1991, and maintained since.  It is hereby
 * placed into the public domain.
 */

/*============================================================================
   Module : Datadraw
   Purpose: Includes UNIX dependent functions.
============================================================================*/

#include <setjmp.h>
#ifndef __USE_XOPEN
#define __USE_XOPEN /* So stdlib.h will include putenv */
#endif
#include <stdlib.h>
#include <string.h>
#ifndef __USE_BSD
#define __USE_BSD /* So unistd.h will include readlink */
#endif
#include <unistd.h>
#include <glob.h>
#include <ctype.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <time.h>
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
    FILE *randFile = fopen("/dev/urandom", "r");

    if(randFile == NULL) {
        utExit("Unable to open random number source.");
    }
    for(xByte = 0; xByte < length; xByte++) {
        randBuff[xByte] = getc(randFile);
    }
    fclose(randFile);
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

    if(*relativePath == UTDIRSEP) {
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
    return utCopyString(buffer);
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
    char *buf;
    char *execpath = utMakeString(UTSTRLEN);
    int numChars;

    buf = utSprintf("/proc/%d/exe", getpid());
    memset(execpath, 0, UTSTRLEN);
    numChars = readlink(buf, execpath, UTSTRLEN - 1);
    execpath[numChars] = '\0';
    return execpath;
}

/*--------------------------------------------------------------------------------------------------
  Set the environment variable.
--------------------------------------------------------------------------------------------------*/
void utSetEnvironmentVariable(
    char *name,
    char *value)
{
    setenv(name, value, 1);
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
  Determine if a directory exists.
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
  Determine if a directory exists.
--------------------------------------------------------------------------------------------------*/
bool utMakeDirectory(char *dirName)
{
    return !mkdir(dirName, 0777);
}

/*--------------------------------------------------------------------------------------------------
  Truncate the file.
--------------------------------------------------------------------------------------------------*/
void utTruncateFile(
    char *fileName,
    uint64 length)
{
    truncate(fileName, length);
}

/*--------------------------------------------------------------------------------------------------
  Delete the file.
--------------------------------------------------------------------------------------------------*/
bool utDeleteFile(
    char *fileName)
{
    return !unlink(fileName);
}
