/*
 * Copyright (C) 2006 Bill Cox
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111 USA
 */

/*--------------------------------------------------------------------------------------------------
  DataDraw is a CASE tool for generating data structures in C from simple descriptions.
--------------------------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "dv.h"

static char *dvExecutableName;
static char *dvIncludeFile = NULL;
static char *dvSourceFile = NULL;
static char *dvLogFile = NULL;
static bool dvMakeModulePersistent;
static bool dvMakeModuleUndoRedo;
static bool dvRunDatabaseManager;

/*--------------------------------------------------------------------------------------------------
  Process arguments, performing the requested actions.
--------------------------------------------------------------------------------------------------*/
static void usage(
   char *format,
   ...)
{
   va_list ap;
   char *buff;

   if(format != NULL) {
       va_start(ap, format);
       buff = utVsprintf((char *)format, ap);
       va_end(ap);
       utLogMessage("%s", buff);
   }
   utLogMessage("Usage: %s [options] file...\n"
      "    -h file    -- Use file as the output header file\n"
      "    -I path    -- Add a directory to the module search path\n"
      "    -l file    -- Use file as a debugging log file\n"
      "    -m         -- Start the database manager to examine datadraw's database\n"
      "    -p         -- Set the module as persistent.\n"
      "    -s file    -- Use file as the output for the source file\n"
      "    -u         -- Set the module as undo_redo\n"
      "This program generates C data structures from their DataDraw file description.\n"
      "This is version %s, compiled on %s\n"
      , dvExecutableName, utGetVersion(), utGetCompileTime());
   exit(1);
}

/*--------------------------------------------------------------------------------------------------
  Process arguments, performing the requested actions.
--------------------------------------------------------------------------------------------------*/
static uint32 processArguments(
    int argc,
    char **argv)
{
    int16 xArg;
    char *optionPtr;

    dvIncludeFile = NULL;
    dvSourceFile = NULL;
    dvLogFile = NULL;
    dvMakeModulePersistent = false;
    dvMakeModuleUndoRedo = false;
    dvRunDatabaseManager = false;
    for(xArg = 1; xArg < argc && argv[xArg][0] == '-'; xArg++) {
        optionPtr = argv[xArg] + 1;
        switch(*optionPtr) {
        case 'I':
            xArg++;
            if(xArg < argc) {
                dvModpathCreate(utSymCreate(argv[xArg]), false);
            } else {
                usage("Expecting module directory after '-%s'", optionPtr);
            }
            break;
        case 'h':
            xArg++;
            if(xArg < argc) {
                dvIncludeFile = argv[xArg];
            } else {
                usage("Expecting output header file name after '-%s'", optionPtr);
            }
            break;
        case 'm':
            dvRunDatabaseManager = true;
            break;
        case 'l':
            xArg++;
            if(xArg < argc) {
                dvLogFile = argv[xArg];
            } else {
                usage("Expecting log file name after '-%s'", optionPtr);
            }
            break;
        case 'p':
            dvMakeModulePersistent = true;
            break;
        case 's':
            xArg++;
            if(xArg < argc) {
                dvSourceFile = argv[xArg];
            } else {
                usage("Expecting output source file name after '-%s'", optionPtr);
            }
            break;
        case 'u':
            dvMakeModuleUndoRedo = true;
            break;
        default:
            usage("Unrecognised option '-%s'", optionPtr);
        }
    }
    return xArg;
}

/*--------------------------------------------------------------------------------------------------
  Print the status.
--------------------------------------------------------------------------------------------------*/
static void batchStatus(
    char *message)
{
    printf("%s", message);
    fflush(stdout);
}

/*--------------------------------------------------------------------------------------------------
  Set directories from command line
--------------------------------------------------------------------------------------------------*/
static bool setDirectories(
    char *executableName)
{
    char *execDirectory;

    utSetExeFullPath(utExecPath(executableName));
    execDirectory = utDirName(utGetExeFullPath());
    if(execDirectory == NULL) {
        fprintf(stderr, "cannot find executable path");
        return true;
    }
    return false;
}

/*--------------------------------------------------------------------------------------------------
  Initialize memory.
--------------------------------------------------------------------------------------------------*/
void dvStart(void)
{
    char *modPath, *p;

    dvDatabaseStart();
    prUtilStart();
    dvTheRoot = dvRootAlloc();
    modPath = utCopyString(utGetEnvironmentVariable("DD_MODPATH"));
    while(modPath != NULL) {
        p = strchr(modPath, ':');
        if(p != NULL) {
            *p++ = '\0';
        }
        dvModpathCreate(utSymCreate(modPath), false);
        modPath = p;
    }
}

/*--------------------------------------------------------------------------------------------------
  Free memory.
--------------------------------------------------------------------------------------------------*/
void dvStop(void)
{
    prUtilStop();
    dvDatabaseStop();
}

/*--------------------------------------------------------------------------------------------------
  This is the actual main routine.
--------------------------------------------------------------------------------------------------*/
int main(
    int argc,
    char **argv)
{
    dvModule module;
    int32 xArg;
    char *exeName;

    utStart();
    utSetVersion(VERSION);
    exeName = utReplaceSuffix(utBaseName(argv[0]), "");
    dvExecutableName = utNewA(char, strlen(exeName) + 1);
    strcpy(dvExecutableName, exeName);
    if(setDirectories(argv[0])) {
        utWarning("Unable to set directories");
        return 1;
    }
    utSetStatusCallback(batchStatus);
    if(utSetjmp()) {
        utWarning("Exiting due to errors");
        if(dvIncludeFile != NULL) {
            utFree(dvIncludeFile);
        }
        if(dvSourceFile != NULL) {
            utFree(dvSourceFile);
        }
        if(dvExecutableName != NULL) {
            utFree(dvExecutableName);
        }
        dvStop();
        utStop(false);
        return 1;
    }
    dvStart();
    xArg = processArguments(argc, argv);
    if(argc - xArg != 1) {
        usage("Expecting a database description file");
    }
    if(dvLogFile != NULL) {
        utInitLogFile(dvLogFile);
    }
    /* Create default module path . */
    dvModpathCreate(utSymCreate(utDirName(argv[xArg])), true);
    utLogMessage("Generating data structure for database description file %s", argv[xArg]);
    if(!dvReadFile(utReplaceSuffix(argv[xArg], ".dd"), true)) {
        utError("Exiting due to errors reading %s", argv[xArg]);
    }
    if(dvIncludeFile == NULL) {
        dvIncludeFile = utDirName(argv[xArg]);
    }
    if(dvSourceFile == NULL) {
        dvSourceFile = utDirName(argv[xArg]);
    }
    dvIncludeFile = utAllocString(dvIncludeFile);
    dvSourceFile = utAllocString(dvSourceFile);
    module = dvRootGetLastModule(dvTheRoot);
    if(dvMakeModulePersistent) {
        dvModuleSetPersistent(module, true);
    }
    if(dvMakeModuleUndoRedo) {
        dvModuleSetUndoRedo(module, true);
    }
    if(!dvGenerateCode(module, dvIncludeFile, dvSourceFile)) {
        utError("Exiting due to errors reading file %s", argv[xArg]);
    }
    if(dvRunDatabaseManager) {
        utManager();
    }
    dvStop();
    utFree(dvIncludeFile);
    utFree(dvSourceFile);
    utFree(dvExecutableName);
    utUnsetjmp();
    utStop(false);
    return 0;
}
