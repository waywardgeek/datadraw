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

#include <stdio.h>
#include "dvdatabase.h"

#define DV_MAX_INDENT 1024

bool dvGenerateCode(dvModule module, char *includeFile, char *sourceFile);
void dvWriteHeaderFile(dvModule module, char *includeFile);
void dvWriteCFile(dvModule module, char *sourceFile);
void dvGenerateAdminTool(dvModule module, char *sourceDir);
bool dvReadFile(char *fileName, bool loadModules);
extern void dvBindProperty(dvProperty property, dvModule module);
void dvBindTypes(dvModule module);

/* Constructors */
dvModpath dvModpathCreate(utSym sym, bool insertAtHeadOfList);
dvModule dvModuleCreate(utSym sym, utSym prefix);
dvEnum dvEnumCreate(dvModule module, utSym sym, utSym prefix);
dvEntry dvEntryCreate(dvEnum owningEnum, utSym sym, uint32 value);
dvTypedef dvTypedefCreate(dvModule module, utSym sym, char *initializer);
dvSchema dvSchemaCreate(dvModule module, utSym sym);
dvClass dvClassCreate(dvModule module, utSym sym, dvClass baseClass);
dvProperty dvPropertyCreate(dvClass owningClass, dvUnion owningUnion, dvPropertyType type, utSym sym);
dvUnion dvUnionCreate(dvClass owningClass, utSym propertySym, uint16 unionNumber);
dvRelationship dvRelationshipCreate(dvSchema schema, dvClass parent, dvClass child, dvRelationshipType type,
    utSym parentLabel, utSym childLabel);
dvLink dvLinkCreate(dvModule importModule, dvModule exportModule);
dvKey dvKeyCreate(dvRelationship relationship, dvProperty property);
dvKey dvUnboundKeyCreate(dvRelationship relationship, utSym propertySym, uint32 lineNum);
dvCase dvCaseCreate(dvProperty property, dvEntry entry);
dvSparsegroup dvSparsegroupCreate(dvClass theClass, utSym sym);
dvCache dvCacheCreate(dvClass theClass);
dvKeyproperty dvUnboundKeypropertyCreate(dvKey key, utSym sym);
dvKeyproperty dvKeypropertyCreate(dvKey key, dvProperty property);

/* Utility functions */
void prUtilStart(void);
void prUtilStop(void);
dvModule dvFindModuleFromPrefix(utSym prefix);
uint8 dvFindPropertySize(dvProperty property);
void dvWrtemp(FILE *file, char *temp, ...);
char *dvSwrtemp(char *temp, ...);
char *dvPropertyGetTypeName(dvProperty property);
char *dvPropertyGetFieldTypeName(dvProperty property);
uint32 dvComputeDatabaseHash(void);
char *dvClassGetPrefix(dvClass theClass);
char *dvPropertyGetPrefix(dvProperty property);
utSym dvUpperSym(utSym sym);
char *dvFindPropertyFormatString(dvProperty property);
char *dvClassGetReferenceTypeName(dvClass theClass);
bool dvModuleHasClassAttributes(dvModule module);
void dvAddDefaultKey(dvRelationship relationship);
bool dvRelationshipHashedByName(dvRelationship relationship);
char *dvPropertyFindInitializer(dvProperty property);
dvProperty dvClassLookupProperty(dvClass theClass, utSym sym);
char *dvKeyGetLengthMacro(dvKey key, bool useParamName, char *param);
char *dvKeyGetAccessMacro(dvKey key, bool useParamName, char * param);
char *dvPropertyGetLengthMacro(dvProperty property, bool useParamName, char * param);
char *dvPropertyGetAccessMacro(dvProperty property, bool useParamName, char * param);

/* Some shortcut macros */
#define dvModuleGetPrefix(module) utSymGetName(dvModuleGetPrefixSym(module))
#define dvRelationshipGetParentLabel(theClass) utSymGetName(dvRelationshipGetParentLabelSym(theClass))
#define dvRelationshipGetChildLabel(theClass) utSymGetName(dvRelationshipGetChildLabelSym(theClass))
#define dvClassPersistent(theClass) dvModulePersistent(dvClassGetModule(theClass))
#define dvPropertyGetID(property) utSprintf("%u", dvPropertyGetFieldNumber(property))
#define dvClassRedo(theClass) (dvModulePersistent(dvClassGetModule(theClass)) || \
    dvModuleUndoRedo(dvClassGetModule(theClass)))
#define dvClassUndo(theClass) (dvModuleUndoRedo(dvClassGetModule(theClass)))
#define dvUnionGetTypeName(theUnion) utSprintf("%s%sUnion%u", dvPrefix, \
    dvClassGetName(dvUnionGetClass(theUnion)), dvUnionGetNumber(theUnion))
#define dvUnionGetFieldName(theUnion) utSprintf("union%d", dvUnionGetNumber(theUnion))
#define dvCacheGetTypeName(cache) utSprintf("struct %s%sCache%u", dvPrefix, \
    dvClassGetName(dvCacheGetClass(cache)), dvCacheGetNumber(cache))
#define dvCacheGetFieldName(cache) utSprintf("cache%d", dvCacheGetNumber(cache))

/* Globals */
extern dvRoot dvTheRoot; /* Root of the database */
extern  dvModule dvCurrentModule; /* Module just read by dvparse */

/* Lex, Yacc stuff */
extern uint32 dvLineNum;
extern bool dvLoadModules;
extern uint16 dvNumEndsRemaining;
extern int dvparse(void);
extern int dvlex(void); /* A wrapper around dvlexlex */
extern int dvlexlex(void); /* The real lexer */
extern void dverror(char *message, ...);
extern char *dvlextext;
extern FILE *dvFile;
extern bool dvLastWasReturn;
extern bool dvEnding;
