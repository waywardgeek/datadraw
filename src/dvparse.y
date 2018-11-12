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

%{

/* Substitute the variable and function names.  */
#define yyparse         dvparse
#define yylex           dvlex
#define yyerror         dverror
#define yydebug         dvdebug
#define yynerrs         dvnerrs

#define yylval          dvlval
#define yychar          dvchar
#include "dv.h"

dvModule dvCurrentModule;
static dvSchema dvCurrentSchema;
static dvEnum dvCurrentEnum;
static dvClass dvCurrentClass;
static dvUnion dvCurrentUnion;
static dvProperty dvCurrentProperty;
static dvRelationship dvCurrentRelationship;
static dvKey dvCurrentKey;
static dvCache dvCurrentCache;
static uint32 dvCacheNumber;
static uint32 dvCurrentEnumValue;
static uint8 dvIntWidth;
static uint32 dvUnionNumber;

/*--------------------------------------------------------------------------------------------------
  Provide yyerror function capability.
--------------------------------------------------------------------------------------------------*/
void dverror(
    char *message,
    ...)
{
    char *buff;
    va_list ap;

    va_start(ap, message);
    buff = utVsprintf(message, ap);
    va_end(ap);
    utError("Line %d, token \"%s\": %s", dvLineNum, dvlextext, buff);
}

/*--------------------------------------------------------------------------------------------------
  Build a user-defined type property, either an enum or typedef.
--------------------------------------------------------------------------------------------------*/
static dvProperty buildUserTypeProperty(
    dvModule module,
    utSym typeSym,
    utSym sym)
{
    dvProperty property = dvPropertyCreate(dvCurrentClass, dvCurrentUnion, PROP_UNBOUND, sym);
 
    dvPropertySetTypeSym(property, typeSym);
    dvPropertySetLine(property, dvLineNum);
    if(module != dvCurrentModule && dvLoadModules) {
        /* Bind it now while we know the module */
        dvBindProperty(property, module);
    }
    return property;
}

/*--------------------------------------------------------------------------------------------------
  Default keys are a utSym field called just "Sym".  If no key is provided, create it.  Also create
  the Sym property on the child if it does not exist.
--------------------------------------------------------------------------------------------------*/
void dvAddDefaultKey(
    dvRelationship relationship)
{
    dvClass childClass = dvRelationshipGetChildClass(relationship);
    utSym symName = utSymCreateFormatted("%sSym", dvRelationshipGetChildLabel(relationship));
    dvProperty symProp = dvClassFindProperty(childClass, symName);

    if(symProp != dvPropertyNull) {
        if(dvPropertyGetType(symProp) != PROP_SYM) {
            dverror("The default key \"Sym\" in hashed relationships must be a symbol");
        }
    } else {
        symProp = dvPropertyCreate(childClass, dvUnionNull, PROP_SYM, symName);
    }
    /* Move symbol to front, just to improve reading fields */
    dvClassRemoveProperty(childClass, symProp);
    dvClassInsertProperty(childClass, symProp);
    dvPropertySetRelationship(symProp, relationship);
    dvKeyCreate(relationship, symProp);
}

%}

%union {
    utSym symVal;
    utSym stringVal;
    utSym indexVal;
    uint32 intVal;
    dvPropertyType propTypeVal;
    dvModule moduleVal;
    dvRelationshipType relationshipType;
};

%token <symVal> IDENT
%token <stringVal> STRING
%token <indexVal> INDEX
%token <intVal> INTEGER INTTYPE UINTTYPE

%type <propTypeVal> propertyType
%type <moduleVal> moduleSpec
%type <relationshipType> relationshipType
%type <symVal> optLabel optIdent upperIdent

%token KWARRAY
%token KWATTRIBUTES
%token KWBEGIN
%token KWBIT
%token KWBOOL
%token KWCACHE_TOGETHER
%token KWCASCADE
%token KWCHAR
%token KWCHILD_ONLY
%token KWCLASS
%token KWCREATE_ONLY
%token KWDOUBLE
%token KWDOUBLY_LINKED
%token KWEND
%token KWENUM
%token KWFLOAT
%token KWFREE_LIST
%token KWHASHED
%token KWHEAP
%token KWIMPORT
%token KWINHERITANCE
%token KWLINKED_LIST
%token KWMANDATORY
%token KWMODULE
%token KWORDERED_LIST
%token KWPARENT_ONLY
%token KWPERSISTENT
%token KWREFERENCE_SIZE
%token KWRELATIONSHIP
%token KWSCHEMA
%token KWSPARSE
%token KWSYM
%token KWTAIL_LINKED
%token KWTYPEDEF
%token KWUNDO_REDO
%token KWUNION
%token KWUNORDERED
%token KWVIEW
%token KWVOLATILE

%%

goal: initialize module
;

initialize: /* Empty */
{
    dvCurrentUnion = dvUnionNull;
    dvCurrentSchema = dvSchemaNull;
}

module: moduleHeader moduleParameters '\n' moduleStuff
;

moduleParameters: /* Empty */
| moduleParameters moduleParameter
;

moduleParameter: KWVOLATILE
| KWPERSISTENT
{
    dvModuleSetPersistent(dvCurrentModule, true);
}
| KWUNDO_REDO
{
    dvModuleSetUndoRedo(dvCurrentModule, true);
}
;

moduleHeader: KWMODULE upperIdent optIdent
{
    dvCurrentModule = dvModuleCreate($2, $3);
    dvCurrentSchema = dvSchemaNull;
}
;

optIdent: /* Empty */
{
    $$ = utSymNull;
}
| IDENT
{
    $$ = $1;
}
;

moduleStuff: /* Empty */
| moduleStuff moduleElement
;

moduleElement: import
| enum
| typedef
| schema
| class
| relationship
;

import: KWIMPORT upperIdent '\n'
{
    dvModule importModule = dvRootFindModule(dvTheRoot, $2);
    if(importModule != dvModuleNull) {
        dvLinkCreate(dvCurrentModule, importModule);
    }
}
;

enum: enumHeader KWBEGIN entries KWEND
;

enumHeader: KWENUM upperIdent optIdent '\n'
{
    dvCurrentEnum = dvEnumCreate(dvCurrentModule, $2, $3);
    dvCurrentEnumValue = 0;
}
;

entries: entry
| entries entry
;

entry: IDENT '\n'
{
    dvEntryCreate(dvCurrentEnum, $1, dvCurrentEnumValue);
    dvCurrentEnumValue++;
}
| IDENT '=' INTEGER '\n'
{
    dvCurrentEnumValue = $3;
    dvEntryCreate(dvCurrentEnum, $1, dvCurrentEnumValue);
    dvCurrentEnumValue++;
}
;

typedef: KWTYPEDEF IDENT '\n'
{
    dvTypedefCreate(dvCurrentModule, $2, "0");
}
| KWTYPEDEF IDENT '=' STRING '\n'
{
    dvTypedefCreate(dvCurrentModule, $2, utSymGetName($4));
}
;

schema: KWSCHEMA upperIdent '\n'
{
    dvCurrentSchema = dvSchemaCreate(dvCurrentModule, $2);
}
;

class: classHeader classOptions '\n' KWBEGIN properties KWEND
| classHeader classOptions '\n'
;

classHeader: KWCLASS upperIdent optLabel
{
    dvClass baseClass = dvClassNull;
    dvModule baseModule = dvModuleNull;
    if($3 != utSymNull && dvLoadModules) {
        baseModule = dvRootFindModule(dvTheRoot, dvUpperSym($3));
        if(baseModule == dvModuleNull) {
            baseModule = dvFindModuleFromPrefix($3);
        }
        if(baseModule == dvModuleNull) {
            dverror("Undefined module %s", utSymGetName($3));
        }
        baseClass = dvModuleFindClass(baseModule, $2);
        if(baseClass == dvClassNull) {
            dverror("Base class %s not defined in module %s", utSymGetName($2), utSymGetName($3));
        }
    }
    dvCurrentClass = dvClassCreate(dvCurrentModule, $2, baseClass);
    dvClassSetBaseClassSym(dvCurrentClass, $3);
    dvUnionNumber = 1;
    dvCacheNumber = 1;
}
;

classOptions: /* Empty */
| classOptions classOption
;

classOption: KWREFERENCE_SIZE INTEGER
{
    if($2 & 7) {
        dverror("Reference sizes may only be 8, 16, 32, or 64");
    }
    dvClassSetReferenceSize(dvCurrentClass, (uint8)$2);
}
| KWFREE_LIST
{
    dvClassSetMemoryStyle(dvCurrentClass, MEM_FREE_LIST);
}
| KWCREATE_ONLY
{
    dvClassSetMemoryStyle(dvCurrentClass, MEM_CREATE_ONLY);
}
| KWARRAY
{
    dvClassSetGenerateArrayClass(dvCurrentClass, true);
}
| KWATTRIBUTES
{
    dvClassSetGenerateAttributes(dvCurrentClass, true);
}
| KWSPARSE
{
    dvClassSetSparse(dvCurrentClass, true);
}
;

properties: property '\n'
| union
| cacheTogether '\n'
| properties property '\n'
| properties union
| properties cacheTogether '\n'
;

property: baseProperty propertyAttributes
| KWARRAY baseProperty propertyAttributes
{
    if(dvCurrentUnion != dvUnionNull) {
        dverror("Arrays are not allowed in unions");
    }
    dvPropertySetArray(dvCurrentProperty, true);
}
| KWARRAY baseProperty INDEX propertyAttributes
{
    char *index;
    if(dvCurrentUnion != dvUnionNull) {
        dverror("Arrays are not allowed in unions");
    }
    if(dvPropertySparse(dvCurrentProperty)) {
        dverror("Fixed sized arrays cannot be sparse");
    }
    dvPropertySetArray(dvCurrentProperty, true);
    dvPropertySetFixedSize(dvCurrentProperty, true);
    index = utSymGetName($3);
    dvPropertySetIndex(dvCurrentProperty, index, strlen(index) + 1);
}
;

propertyAttributes: /* Empty */
| propertyAttributes propertyAttribute
;

propertyAttribute: KWCASCADE
{
    dvPropertySetCascade(dvCurrentProperty, true);
}
| KWVIEW
{
    dvPropertySetView(dvCurrentProperty, true);
}
| KWSPARSE
{
    if(dvCurrentUnion != dvUnionNull) {
        dverror("Union fields cannot be sparse");
    }
    dvPropertySetSparse(dvCurrentProperty, true);
}
| '=' STRING
{
    char *value = utSymGetName($2);
    dvPropertySetInitializer(dvCurrentProperty, value, strlen(value) + 1);
}
;

baseProperty: IDENT upperIdent
{
    dvCurrentProperty = buildUserTypeProperty(dvCurrentModule, $1, $2);
}
| moduleSpec IDENT upperIdent
{
    dvCurrentProperty = buildUserTypeProperty($1, $2, $3);
}
| propertyType upperIdent
{
    dvCurrentProperty = dvPropertyCreate(dvCurrentClass, dvCurrentUnion, $1, $2);
    if($1 == PROP_INT || $1 == PROP_UINT) {
        if(dvIntWidth != 8 && dvIntWidth != 16 && dvIntWidth != 32 && dvIntWidth != 64) {
            dverror("Valid integer widths are 8, 16, 32, and 64");
        }
        dvPropertySetWidth(dvCurrentProperty, dvIntWidth);
    }
}
;

cacheTogether: cacheTogetherHeader cacheIdents
;

cacheTogetherHeader: KWCACHE_TOGETHER
{
    if(!dvClassRedo(dvCurrentClass)) {
        dvCurrentCache = dvCacheCreate(dvCurrentClass);
        dvCacheSetLine(dvCurrentCache, dvLineNum);
        dvCacheSetNumber(dvCurrentCache, dvCacheNumber);
        dvCacheNumber++;
    }
}

cacheIdents: /* Empty */
| cacheIdents upperIdent
{
    if(!dvClassRedo(dvCurrentClass)) {
        dvPropident propident = dvPropidentAlloc();
        dvPropidentSetSym(propident, $2);
        dvCacheAppendPropident(dvCurrentCache, propident);
    }
}
;

moduleSpec: IDENT ':'
{
    dvModule module = dvRootFindModule(dvTheRoot, $1);
    if(module == dvModuleNull) {
        module = dvFindModuleFromPrefix($1);
    }
    if(module == dvModuleNull && dvLoadModules) {
        dverror("Module %s not found", utSymGetName($1));
    }
    $$ = module;
}

propertyType: INTTYPE
{
    dvIntWidth = (uint8)$1;
    $$ = PROP_INT;
}
| UINTTYPE
{
    dvIntWidth = (uint8)$1;
    $$ = PROP_UINT;
}
| KWFLOAT
{
    $$ = PROP_FLOAT;
}
| KWDOUBLE
{
    $$ = PROP_DOUBLE;
}
| KWBIT
{
    if(dvCurrentUnion == dvUnionNull) {
        $$ = PROP_BIT;
    } else {
        $$ = PROP_BOOL;
    }
}
| KWBOOL
{
    $$ = PROP_BOOL;
}
| KWCHAR
{
    $$ = PROP_CHAR;
}
| KWENUM
{
    $$ = PROP_ENUM;
}
| KWTYPEDEF
{
    $$ = PROP_TYPEDEF;
}
| KWSYM
{
    $$ = PROP_SYM;
}
;

union: unionHeader KWBEGIN nonUnionProperties KWEND
{
    if(dvUnionGetFirstProperty(dvCurrentUnion) == dvPropertyNull) {
        dvUnionDestroy(dvCurrentUnion);
    }
    dvCurrentUnion = dvUnionNull;
}
;

unionHeader: KWUNION upperIdent '\n'
{
    dvCurrentUnion = dvUnionCreate(dvCurrentClass, $2, dvUnionNumber++);
}
;

nonUnionProperties: property ':' unionCases '\n'
| nonUnionProperties property ':' unionCases '\n'
;

unionCases: unionCase
| unionCases unionCase
;

unionCase: IDENT
{
    dvCase theCase = dvCaseAlloc();
    dvCaseSetEntrySym(theCase, $1);
    dvPropertyAppendCase(dvCurrentProperty, theCase);
}

relationship: relationshipHeader relationshipType relationshipOptions '\n'
{
    dvRelationshipSetType(dvCurrentRelationship, $2);
}
;

relationshipHeader: KWRELATIONSHIP upperIdent optLabel upperIdent optLabel
{
    dvClass parent = dvModuleFindClass(dvCurrentModule, $2);
    dvClass child = dvModuleFindClass(dvCurrentModule, $4);
    if(parent == dvClassNull) {
        dverror("Unknown class %s", utSymGetName($2));
    }
    if(child == dvClassNull) {
        dverror("Unknown class %s", utSymGetName($4));
    }
    if(dvCurrentSchema == dvSchemaNull) {
        dvCurrentSchema = dvSchemaCreate(dvCurrentModule, dvModuleGetSym(dvCurrentModule));
    }
    /* Note that the type is just a place-holder.  We fill in the real type after parsing the type */
    dvCurrentRelationship = dvRelationshipCreate(dvCurrentSchema, parent, child, REL_UNBOUND,
        dvUpperSym($3), dvUpperSym($5));
}
;

optLabel: /* Empty */
{
    $$ = utSymNull;
}
| ':' IDENT
{
    $$ = $2;
}
;

relationshipOptions: /* Empty */
| relationshipOptions relationshipOption
;

relationshipType: /* Empty */
{
    $$ = REL_POINTER;
}
| KWLINKED_LIST
{
    $$ = REL_LINKED_LIST;
}
| KWDOUBLY_LINKED
{
    $$ = REL_DOUBLY_LINKED;
}
| KWTAIL_LINKED
{
    $$ = REL_TAIL_LINKED;
}
| KWARRAY
{
    $$ = REL_ARRAY;
}
| KWHEAP key
{
    $$ = REL_HEAP;
}
| KWHASHED key
{
    $$ = REL_HASHED;
    if(dvRelationshipGetFirstKey(dvCurrentRelationship) == dvKeyNull) {
        dvAddDefaultKey(dvCurrentRelationship);
    }
}
| KWORDERED_LIST key
{
    $$ = REL_ORDERED_LIST;
}
;

key: /* empty */
| key keyproperties
;

keyproperties : upperIdent
{
    dvCurrentKey = dvUnboundKeyCreate(dvCurrentRelationship, $1, dvLineNum);
}
| keyproperties '.' upperIdent
{
    dvUnboundKeypropertyCreate(dvCurrentKey, $3);
}
;

relationshipOption: KWCASCADE
{
    dvRelationshipSetCascade(dvCurrentRelationship, true);
}
| KWMANDATORY
{
    dvRelationshipSetMandatory(dvCurrentRelationship, true);
    dvRelationshipSetCascade(dvCurrentRelationship, true);
}
| KWPARENT_ONLY
{
    dvRelationshipSetAccessChild(dvCurrentRelationship, false);
}
| KWCHILD_ONLY
{
    dvRelationshipSetAccessParent(dvCurrentRelationship, false);
}
| KWSPARSE
{
    dvRelationshipSetSparse(dvCurrentRelationship, true);
}
| KWUNORDERED
{
    dvRelationshipSetUnordered(dvCurrentRelationship, true);
}
;

upperIdent: IDENT
{
    $$ = dvUpperSym($1);
}

%%
