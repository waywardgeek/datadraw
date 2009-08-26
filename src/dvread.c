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
   This module reads and writes gate-level Verilog designs.
--------------------------------------------------------------------------------------------------*/
#include <ctype.h>
#include "dv.h"

FILE *dvFile;
uint32 dvLineNum;
bool dvLoadModules;

/* Remember not to have any static variables!  dvReadFile is recursive! */

/*--------------------------------------------------------------------------------------------------
  Bind the property type.
--------------------------------------------------------------------------------------------------*/
void dvBindProperty(
    dvProperty property,
    dvModule module)
{
    utSym typeSym = dvPropertyGetTypeSym(property);
    dvClass pointerClass = dvModuleFindClass(module, typeSym);
    dvEnum theEnum;
    dvTypedef theTypedef;

    if(pointerClass != dvClassNull) {
        dvPropertySetType(property, PROP_POINTER);
        dvPropertySetClassProp(property, pointerClass);
        return;
    }
    theEnum = dvModuleFindEnum(module, typeSym);
    if(theEnum != dvEnumNull) {
        dvPropertySetType(property, PROP_ENUM);
        dvPropertySetEnumProp(property, theEnum);
        return;
    }
    theTypedef = dvModuleFindTypedef(module, typeSym);
    if(theTypedef == dvTypedefNull) {
        utError("Line %u: Undefined type %s.%s", dvPropertyGetLine(property),
            dvModuleGetName(module), utSymGetName(typeSym));
    }
    dvPropertySetType(property, PROP_TYPEDEF);
    dvPropertySetTypedefProp(property, theTypedef);
}

/*--------------------------------------------------------------------------------------------------
  Bind all property types in the module.
--------------------------------------------------------------------------------------------------*/
static void bindUnion(
    dvUnion theUnion)
{
    dvClass theClass = dvUnionGetClass(theUnion);
    dvEnum theEnum;
    dvEntry entry;
    utSym propertySym = dvUnionGetPropertySym(theUnion);
    dvProperty property = dvClassFindProperty(theClass, propertySym);
    dvCase theCase;

    if(property == dvPropertyNull) {
        utError("Line %u: Undefined property %s", dvUnionGetLine(theUnion),
            utSymGetName(propertySym));
    }
    if(dvPropertyGetType(property) != PROP_ENUM) {
        utError("Line %u: Expected enum property rather than %s", dvUnionGetLine(theUnion),
            utSymGetName(propertySym));
    }
    dvUnionSetTypeProperty(theUnion, property);
    theEnum = dvPropertyGetEnumProp(property);
    dvForeachUnionProperty(theUnion, property) {
        dvForeachPropertyCase(property, theCase) {
            entry = dvEnumFindEntry(theEnum, dvCaseGetEntrySym(theCase));
            if(entry == dvEntryNull) {
                utError("Line %u: Unknown entry %s", dvPropertyGetLine(property),
                    utSymGetName(dvCaseGetEntrySym(theCase)));
            }
            dvEntryAppendCase(entry, theCase);
        } dvEndPropertyCase;
    } dvEndUnionProperty;
}

/*--------------------------------------------------------------------------------------------------
  Bind all property types in the module.
--------------------------------------------------------------------------------------------------*/
void dvBindTypes(
    dvModule module)
{
    dvUnion theUnion;
    dvClass theClass;
    dvProperty property;
    dvPropertyType type;

    dvForeachModuleClass(module, theClass) {
        dvForeachClassProperty(theClass, property) {
            type = dvPropertyGetType(property);
            if(type == PROP_UNBOUND) {
                dvBindProperty(property, module);
            }
            if(dvPropertyGetType(property) != PROP_POINTER && dvPropertyCascade(property)) {
                utError("Line %u: Only pointer properties can be cascade",
                    dvPropertyGetLine(property));
            }
        } dvEndClassProperty;
        dvForeachClassUnion(theClass, theUnion) {
            bindUnion(theUnion);
        } dvEndClassUnion;
    } dvEndModuleClass;
}

/*--------------------------------------------------------------------------------------------------
  Just read one line from the input file.
--------------------------------------------------------------------------------------------------*/
static bool readLine(
    FILE *inFile,
    char **lineBufferPtr,
    uint32 *bufferSizePtr)
{
    char *lineBuffer = *lineBufferPtr;
    uint32 bufferSize = *bufferSizePtr;
    uint32 bufferLength = 0;
    int c;

    utDo {
        c = getc(inFile);
    } utWhile(c != EOF && c != '\n') {
        if(c != '\r') {
            if(bufferLength == bufferSize) {
                bufferSize <<= 1;
                utResizeArray(lineBuffer, bufferSize);
            }
            lineBuffer[bufferLength++] = c;
        }
    } utRepeat;
    if(bufferLength == bufferSize) {
        bufferSize <<= 1;
        utResizeArray(lineBuffer, bufferSize);
    }
    lineBuffer[bufferLength] = '\0';
    *lineBufferPtr = lineBuffer;
    *bufferSizePtr = bufferSize;
    return c != EOF || bufferLength > 0;
}

/*--------------------------------------------------------------------------------------------------
  Find the module in the module path.  Return the file name.
--------------------------------------------------------------------------------------------------*/
static char *findModuleFileInModpath(
    char *moduleName)
{
    dvModpath modpath;
    char *fileName;

    dvForeachRootModpath(dvTheRoot, modpath) {
        fileName = utSprintf("%s%c%s.dd", dvModpathGetName(modpath), UTDIRSEP, moduleName);
        if(utFileExists(fileName)) {
            return fileName;
        }
    } dvEndRootModpath;
    return NULL;
}

/*--------------------------------------------------------------------------------------------------
  Find the first letter after the identifier.
--------------------------------------------------------------------------------------------------*/
static char *findIdentifierEnd(
    char *identPtr)
{
    int c;

    utDo {
        c = *identPtr;
    } utWhile(isalnum(c) || c == '_') {
        identPtr++;
    } utRepeat;
    return identPtr;
}

/*--------------------------------------------------------------------------------------------------
  Check the current line to see if it's an import statement.
--------------------------------------------------------------------------------------------------*/
static void checkForModuleImport(
    char *lineBuffer)
{
    utSym moduleSym;
    char *moduleName, *fileName, *end;
    uint32 linePosition = 6;
    int c;

    if(strncmp(lineBuffer, "import", 6)) {
        return;
    }
    utDo {
        c = lineBuffer[linePosition];
    } utWhile(c == ' ' || c == '\t') {
        linePosition++;
    } utRepeat;
    moduleName = lineBuffer + linePosition;
    end = findIdentifierEnd(moduleName);
    *end = '\0';
    moduleSym = utSymCreate(moduleName);
    if(dvRootFindModule(dvTheRoot, moduleSym) != dvModuleNull) {
        /* Already loaded the module. */
        return;
    }
    fileName = findModuleFileInModpath(moduleName);
    if(fileName == NULL) {
        utError("Unable to find module %s in the module path", moduleName);
    }
    dvReadFile(fileName, true);
}

/*--------------------------------------------------------------------------------------------------
  Just scan the file once, looking for import statements.  Load the imported modules.
--------------------------------------------------------------------------------------------------*/
static void loadImportedModules(
    char *fileName)
{
    uint32 bufferSize = 256;
    char *lineBuffer = utNewA(char, bufferSize);
    FILE *inFile = fopen(fileName, "r");

    if(!inFile) {
        utExit("Could not open file %s for reading", fileName);
    }
    while(readLine(inFile, &lineBuffer, &bufferSize)) {
        checkForModuleImport(lineBuffer);
    }
    utFree(lineBuffer);
    fclose(inFile);
}

/*--------------------------------------------------------------------------------------------------
  Read a DataDraw file.  Note that this function is recursive due to "import" statements!  No
  static data!
--------------------------------------------------------------------------------------------------*/
bool dvReadFile(
    char *fileName,
    bool loadModules)
{
    fileName = utAllocString(fileName);
    if(loadModules) {
        loadImportedModules(fileName);
    }
    utLogMessage("Reading DataDraw file %s", fileName);
    dvFile = fopen(fileName, "r");
    if(!dvFile) {
        utExit("Could not open file %s for reading", fileName);
    }
    utFree(fileName);
    dvLineNum = 1;
    dvLoadModules = loadModules;
    dvLastWasReturn = true;
    dvEnding = false;
    if(dvparse()) {
        fclose(dvFile);
        return false;
    }
    if(dvLoadModules) {
        dvBindTypes(dvCurrentModule);
    }
    fclose(dvFile);
    return true;
}

