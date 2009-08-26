/*
 * This file was written by Bill Cox, originally in 1991, and maintained since.  It is hereby
 * placed into the public domain.
 */

#include <windows.h>
#include <direct.h>
#include <stdlib.h>
#include <io.h>
#include <time.h>
#include <sys/stat.h>
#include "ddutil.h"

jmp_buf utJmpBuf[UT_MAX_SETJMP_DEPTH];

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
  Default Error notification routine in absence of user-speified.
--------------------------------------------------------------------------------------------------*/
void ErrorNotify (
   char *string)
{
   printf("%s\n", string);
   fflush(stdout);
}

/*--------------------------------------------------------------------------------------------------
  Allocate a block of memory. Don't fill with 0's.
--------------------------------------------------------------------------------------------------*/
void *mtCalloc (
   uint32 numBlocks,
   uint16 size)
{
   return calloc((unsigned)numBlocks, (unsigned)size);
}

/*--------------------------------------------------------------------------------------------------
  Allocate a block of memory. Don't fill with 0's.
--------------------------------------------------------------------------------------------------*/
void *mtMalloc (
   uint32 numBlocks,
   uint16 size)
{
   return malloc(numBlocks*size);
}

/*--------------------------------------------------------------------------------------------------
  Reallocate and expand memory.
--------------------------------------------------------------------------------------------------*/
void * mtRealloc (
   void *mem,
   uint32 numBlocks,
   uint16 size)
{
   return realloc((char *)mem, (size_t)(numBlocks * size));
}

/*--------------------------------------------------------------------------------------------------
  Free a blo`ck of memory.  Return TRUE if sucess.
--------------------------------------------------------------------------------------------------*/
bool mtFree (
   void *p)
{
   free(p);
   return true;
}


/*--------------------------------------------------------------------------------------------------
   FUNCTION: utGetProfileString
   PURPOSE : Function to read a string from a .ini file
--------------------------------------------------------------------------------------------------*/
bool utGetProfileString(char *szToolName, char *szKeyName,
   char *szDefault, char *szBuffer, int16 nSize)
{
   bool ret;

   ret = (bool) GetProfileString(szToolName, szKeyName,
      szDefault, szBuffer, nSize);

   return ret;
}

/*--------------------------------------------------------------------------------------------------
   FUNCTION: utGetProfileString
   PURPOSE : Function to write a string to win.ini file
--------------------------------------------------------------------------------------------------*/
bool utWriteProfileString(char *szSection, char *szKeyName,
   char *szValue)
{
   bool ret;

   ret = (bool) WriteProfileString(szSection, szKeyName, szValue);
   return ret;
}

/*--------------------------------------------------------------------------------------------------
   FUNCTION: utGetPrivateProfileInt
   PURPOSE : Function to read a Int from a .ini file
--------------------------------------------------------------------------------------------------*/
int32 utGetPrivateProfileInt(char *szToolName,
   char *szKeyName, int32 nDefault, char *szIniFileName)
{
   int32 val;

   val = GetPrivateProfileInt(szToolName, szKeyName, nDefault, szIniFileName);
   return val;
}

/*--------------------------------------------------------------------------------------------------
   FUNCTION: utGetPrivateProfileString
   PURPOSE : Function to read a string from a .ini file
--------------------------------------------------------------------------------------------------*/
void utGetPrivateProfileString(char *szToolName,
   char *szKeyName, char *szDefault, char *szBuffer,
   int16 nSize, char *szIniFileName)
{
   GetPrivateProfileString(szToolName, szKeyName, szDefault,
      szBuffer, nSize, szIniFileName);
}

/*--------------------------------------------------------------------------------------------------
   FUNCTION: utWritePrivateProfileString
   PURPOSE : Function to write a string to a .ini file
--------------------------------------------------------------------------------------------------*/
void utWritePrivateProfileString(char *szToolName,
   char *szKeyName, char *szSetStr, char *szIniFileName)
{
   WritePrivateProfileString(szToolName, szKeyName, szSetStr, szIniFileName);
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
   PROCESS_INFORMATION processInfo;
   STARTUPINFO startupInfo;

   memset((void *)(&startupInfo), 0, sizeof(STARTUPINFO));
   startupInfo.cb = sizeof(STARTUPINFO);
   if (!CreateProcess(NULL, (char *)cmdLine, NULL, NULL, FALSE, 0, NULL, wkgDir, 
                      &startupInfo, &processInfo)) {
      utError("UT1000: Failed to launch program %s - Error %d", cmdLine,
               GetLastError());
      return false;
   }
   return true;
}

/*--------------------------------------------------------------------------------------------------
  Terminate. Unix optionally writes a core file for debug, NT just exits.
--------------------------------------------------------------------------------------------------*/
void utAbnormalProgramTermination(void)
{
    exit(-1);
}

/*--------------------------------------------------------------------------------------------------
   FUNCTION: utGetModuleDirectory
   PURPOSE : Function to get the path of the specified module (exe or dll)
             NOTE:  Module must be loaded already!
--------------------------------------------------------------------------------------------------*/
bool utGetModuleDirectory (
   char *moduleName,
   char *path,
   int16   max_path)
{
   HMODULE hModule;

   hModule = GetModuleHandle(moduleName);      /* get module instance */
   if (!hModule) {
      return false;
   }
   GetModuleFileName(hModule, path, max_path);
   return true;
}

/*--------------------------------------------------------------------------------------------------
  Set the environment variable.
--------------------------------------------------------------------------------------------------*/
void utSetEnvironmentVariable(
    char *name,
    char *value)
{
    SetEnvironmentVariable(name, value);
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
  Match the string to a file and return the matched file name.
--------------------------------------------------------------------------------------------------*/
char *utGlob(
    char *fileName)
{
    if(utAccess(fileName, NULL)) {
        return utCopyString(fileName);
    }
    return NULL;
}

/*--------------------------------------------------------------------------------------------------
  Find the current working directory name.
--------------------------------------------------------------------------------------------------*/
char *utGetcwd(void)
{
    char buffer[UTSTRLEN];

    _getcwd(buffer, UTSTRLEN);
    return utCopyString(buffer);
}

/*--------------------------------------------------------------------------------------------------
  Take the relative path, and return the full path.
--------------------------------------------------------------------------------------------------*/
char *utFullPath(
    char *relativePath)
{
    char fullPath[UTSTRLEN];

    _fullpath(fullPath, relativePath, UTSTRLEN);
    return utCopyString(fullPath);
}

/*--------------------------------------------------------------------------------------------------
  Determine the access to a file
--------------------------------------------------------------------------------------------------*/
bool utAccess(
    char *name,
    char *mode)
{
    if(mode == NULL) {
        return _access(name, 0) == 0;
    }
    if(!strcmp(mode, "r")) {
        return _access(name, 4) == 0;
    }
    if(!strcmp(mode, "w")) {
        return _access(name, 2) == 0;
    }
    if(!strcmp(mode, "x")) {
        return _access(name, 0) == 0;
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
    char *path, *p;
    char *fullPath;
        char actualPath[1024];
        char *fullPathProgram;

    if(strchr(program, UTDIRSEP) != NULL) {
        return utFullPath(program);
    } else {
                if(_getcwd(actualPath, 1024) != 0) {
                        fullPath = utMakeString(strlen(actualPath)+1+strlen(program)+1);
                        fullPath = utSprintf("%s%c%s", actualPath, UTDIRSEP, program);
                        fullPathProgram = utGlob(fullPath);
                        if(fullPathProgram != NULL) {
                                return fullPathProgram;
                        }
                        fullPathProgram = utGlob(utSprintf("%s.exe", fullPath));
                        if(fullPathProgram != NULL) {
                                return fullPathProgram;
                        }
                } else {
                        path = getenv ("PATH");
                        if(path != NULL) {
                                p = path;
                                do {
                                        path = p;
                                        p = strchr(path, UTPATHSEP);
                                        if(p == NULL) {
                                                p = strchr(path, '\0');
                                        }
                                        if(p == path) {
                                                fullPath = utCopyString(program);
                                        } else {
                                                fullPath = utMakeString((p - path) + 1);
                                                strncpy(fullPath, path, p - path);
                                                fullPath = utSprintf("%s%c%s", fullPath, UTDIRSEP, program);
                                        }
                                        fullPath = utGlob(fullPath);
                                        if(fullPath != NULL) {
                                                return fullPath;
                                        }
                                        fullPath = utGlob(utSprintf("%s.exe", fullPath));
                                        if(fullPath != NULL) {
                                                return fullPath;
                                        }
                                } while(*p++ != '\0');
                        }
                }
    }
    utError("could not find executable %s", program);
    return NULL;
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
    return (statbuf.st_mode & S_IFDIR) != 0;
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
