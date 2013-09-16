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
  Random support functions.
--------------------------------------------------------------------------------------------------*/
#include <ctype.h>
#include "dv.h"

/*--------------------------------------------------------------------------------------------------
  Find the module from the prefix.
--------------------------------------------------------------------------------------------------*/
dvModule dvFindModuleFromPrefix(
    utSym prefix)
{
    dvModule module;

    dvForeachRootModule(dvTheRoot, module) {
        if(dvModuleGetPrefixSym(module) == prefix) {
            return module;
        }
    } dvEndRootModule;
    return dvModuleNull;
}

/*--------------------------------------------------------------------------------------------------
  Find out how many args are used in a template.
--------------------------------------------------------------------------------------------------*/
static uint32 countArgs(
    char *temp)
{
    char c;
    uint32 maxArg = 0, xArg;

    while (*temp) {
        c = *temp++;
        if (c == '%') {
            c = *temp;
            if (c == 'l' || c == 'u' || c == 'c') {
                temp++;
                c = *temp;
            }
            if(isdigit((unsigned char)c)) {
                temp++;
                xArg = c - '0';
                if (xArg >= maxArg) {
                    maxArg = xArg + 1;
                }
            } else if (c == '%') {
                temp++;
            }
        }
    }
    return maxArg;
}

/*--------------------------------------------------------------------------------------------------
  This code manages a simple string buffer.
--------------------------------------------------------------------------------------------------*/
static char *prStringBuffer;
static uint32 prStringBufferSize;
static uint32 prStringBufferPosition;

/*--------------------------------------------------------------------------------------------------
  Append a string to the string buffer.
--------------------------------------------------------------------------------------------------*/
static void appendString(
    char *string)
{
    uint32 length = strlen(string);

    if(prStringBufferPosition + length + 1 >= prStringBufferSize) {
        prStringBufferSize = ((prStringBufferPosition + length + 1)*3) >> 1;
        utResizeArray(prStringBuffer, prStringBufferSize);
    }
    strcpy(prStringBuffer + prStringBufferPosition, string);
    prStringBufferPosition += length;
}

/*--------------------------------------------------------------------------------------------------
  Append a character to the string buffer.
--------------------------------------------------------------------------------------------------*/
static void appendChar(
    char c)
{
    char string[2];

    string[0] = c;
    string[1] = '\0';
    appendString(string);
}

/*--------------------------------------------------------------------------------------------------
  Initialize the utility module, allocating buffers.
--------------------------------------------------------------------------------------------------*/
void prUtilStart(void)
{
    prStringBufferSize = 42;
    prStringBuffer = utNewA(char, prStringBufferSize);
    prStringBufferPosition = 0;
}

/*--------------------------------------------------------------------------------------------------
  Free memory used by the utility module.
--------------------------------------------------------------------------------------------------*/
void prUtilStop(void)
{
    utFree(prStringBuffer);
}

/*--------------------------------------------------------------------------------------------------
  Write a template to a string.
--------------------------------------------------------------------------------------------------*/
static void wrtemp(
   char *temp,
   va_list ap)
{
    uint32 sArg = countArgs(temp), xArg;
    char *(args[10]);
    char *string, *arg;
    char c;
    bool lowerCase = false, upperCase = false, caps = false;

    prStringBufferPosition = 0;
    prStringBuffer[0] = '\0';
    for (xArg = 0; xArg < sArg; xArg++) {
        args[xArg] = va_arg(ap, char *);
    }
    string = temp;
    while (*string) {
        c = *string++;
        if (c == '%') {
            c = *string;
            if(c == 'l') {
                lowerCase = true;
                c = *++string;
            } else if(c == 'u') {
                upperCase = true;
                c = *++string;
            } else if(c == 'c') {
                caps = true;
                c = *++string;
            }
            if(isdigit((unsigned char)c)) {
                string++;
                xArg = c - '0';
                if(xArg >= sArg) {
                    utExit("exWrtemp: not enough args");
                }
                if (*args[xArg]) {
                    if(lowerCase) {
                        appendChar((char)tolower((unsigned char)*(args[xArg])));
                        appendString((args[xArg]) + 1);
                        lowerCase = false;
                    } else if(upperCase) {
                        appendChar((char)toupper((unsigned char)*(args[xArg])));
                        appendString((args[xArg]) + 1);
                        upperCase = false;
                    } else if(caps) {
                        arg = args[xArg];
                        while(*arg) {
                            appendChar((char)toupper((unsigned char)*arg));
                            arg++;
                        }
                        caps = false;
                    } else {
                        appendString(args[xArg]);
                    }
                }
            } else if (c == '%') {
                string++;
                appendChar('%');
            }
        } else {
            appendChar(c);
        }
    }
}

/*--------------------------------------------------------------------------------------------------
  Write a template to a string.
--------------------------------------------------------------------------------------------------*/
char *dvSwrtemp(
    char *temp,
    ...)
{
    va_list ap;

    va_start(ap, temp);
    wrtemp(temp, ap);
    va_end(ap);
    return utCopyString(prStringBuffer);
}

/*--------------------------------------------------------------------------------------------------
  Write a template to a string.
--------------------------------------------------------------------------------------------------*/
void dvWrtemp(
    FILE *file,
    char *temp,
    ...)
{
    va_list ap;

    va_start(ap, temp);
    wrtemp(temp, ap);
    va_end(ap);
    fputs(prStringBuffer, file);
}

/*--------------------------------------------------------------------------------------------------
  Get the property type name.
--------------------------------------------------------------------------------------------------*/
char *dvPropertyGetTypeName(
    dvProperty property)
{
    dvClass theClass;
    dvEnum theEnum;
    dvTypedef theTypedef;

    switch(dvPropertyGetType(property)) {
    case PROP_INT: return utSprintf("int%u", dvPropertyGetWidth(property));
    case PROP_UINT: return utSprintf("uint%u", dvPropertyGetWidth(property));
    case PROP_FLOAT: return "float";
    case PROP_DOUBLE: return "double";
    case PROP_BIT: case PROP_BOOL:
        return "uint8";
    case PROP_CHAR: return "char";
    case PROP_SYM: return "utSym";
    case PROP_ENUM:
        theEnum = dvPropertyGetEnumProp(property);
        return utSprintf("%s%s", dvModuleGetPrefix(dvEnumGetModule(theEnum)),
            dvEnumGetName(theEnum));
    case PROP_TYPEDEF: return dvTypedefGetName(dvPropertyGetTypedefProp(property));
        theTypedef = dvPropertyGetTypedefProp(property);
        return utSprintf("%s%s", dvModuleGetPrefix(dvTypedefGetModule(theTypedef)),
            dvTypedefGetName(theTypedef));
    case PROP_POINTER:
        theClass = dvPropertyGetClassProp(property);
        return utSprintf("%s%s", dvClassGetPrefix(theClass), dvClassGetName(theClass));
    default:
        utExit("Unknown property type");
    }
    return NULL; /* Dummy return */
}

/*--------------------------------------------------------------------------------------------------
  Get the property type name as a utility library field type.
--------------------------------------------------------------------------------------------------*/
char *dvPropertyGetFieldTypeName(
    dvProperty property)
{
    switch(dvPropertyGetType(property)) {
    case PROP_INT: return "UT_INT";
    case PROP_UINT: return "UT_UINT";
    case PROP_FLOAT: return "UT_FLOAT";
    case PROP_DOUBLE: return "UT_DOUBLE";
    case PROP_BIT: return "UT_BIT";
    case PROP_BOOL: return "UT_BOOL";
    case PROP_CHAR: return "UT_CHAR";
    case PROP_SYM: return "UT_SYM";
    case PROP_ENUM: return "UT_ENUM";
    case PROP_TYPEDEF: return "UT_TYPEDEF";
    case PROP_POINTER: return "UT_POINTER";
    default:
        utExit("Unknown property type");
    }
    return NULL; /* Dummy return */
}

/*--------------------------------------------------------------------------------------------------
  Find a hash summary of the enumerated type.
--------------------------------------------------------------------------------------------------*/
static uint32 findEnumHash(
    dvEnum theEnum)
{
    dvEntry entry;
    uint32 hash = utSymGetHashValue(dvEnumGetSym(theEnum));

    hash = utHashValues(hash, utSymGetHashValue(dvEnumGetPrefixSym(theEnum)));
    dvForeachEnumEntry(theEnum, entry) {
        hash ^= utSymGetHashValue(dvEntryGetSym(entry));
    } dvEndEnumEntry;
    return hash;
}

/*--------------------------------------------------------------------------------------------------
  Find a hash signature for the class.
--------------------------------------------------------------------------------------------------*/
static uint32 findPropertyHash(
    dvProperty property)
{
    uint32 hash = utSymGetHashValue(dvPropertyGetSym(property));

    hash = utHashValues(hash, dvPropertyGetType(property));
    hash = utHashValues(hash, dvPropertyArray(property));
    switch(dvPropertyGetType(property)) {
    case PROP_INT:
    case PROP_UINT:
        hash = utHashValues(hash, dvPropertyGetWidth(property));
        break;
    case PROP_FLOAT:
    case PROP_DOUBLE:
    case PROP_BIT:
    case PROP_BOOL:
    case PROP_CHAR:
    case PROP_SYM:
        break;
    case PROP_ENUM:
        hash = utHashValues(hash, utSymGetHashValue(dvEnumGetSym(dvPropertyGetEnumProp(property))));
        break;
    case PROP_TYPEDEF:
        hash = utHashValues(hash, utSymGetHashValue(dvTypedefGetSym(dvPropertyGetTypedefProp(property))));
        break;
    case PROP_POINTER:
        hash = utHashValues(hash, utSymGetHashValue(dvClassGetSym(dvPropertyGetClassProp(property))));
        break;
    default:
        utExit("Unknown property type");
    }
    return hash;
}

/*--------------------------------------------------------------------------------------------------
  Find a hash signature for the union.
--------------------------------------------------------------------------------------------------*/
static uint32 findUnionHash(
    dvUnion theUnion)
{
    dvProperty property;
    uint32 hash = 0;

    dvForeachUnionProperty(theUnion, property) {
        hash ^= utSymGetHashValue(dvPropertyGetSym(property));
    } dvEndUnionProperty;
    return hash;
}

/*--------------------------------------------------------------------------------------------------
  Find a hash signature for the relationship.
--------------------------------------------------------------------------------------------------*/
static uint32 findRelationshipHash(
    dvRelationship relationship)
{
    uint32 hash = utSymGetHashValue(dvClassGetSym(dvRelationshipGetParentClass(relationship)));

    hash = utHashValues(hash, utSymGetHashValue(dvClassGetSym(dvRelationshipGetChildClass(relationship))));
    hash = utHashValues(hash, dvRelationshipGetType(relationship));
    hash = utHashValues(hash, utSymGetHashValue(dvRelationshipGetParentLabelSym(relationship)));
    hash = utHashValues(hash, utSymGetHashValue(dvRelationshipGetChildLabelSym(relationship)));
    hash = utHashValues(hash, dvRelationshipMandatory(relationship));
    hash = utHashValues(hash, dvRelationshipCascade(relationship));
    hash = utHashValues(hash, dvRelationshipAccessChild(relationship));
    hash = utHashValues(hash, dvRelationshipAccessParent(relationship));
    return hash;
}

/*--------------------------------------------------------------------------------------------------
  Find a hash signature for the class.
--------------------------------------------------------------------------------------------------*/
static uint32 findClassHash(
    dvClass theClass)
{
    dvProperty property;
    dvUnion theUnion;
    dvRelationship relationship;
    uint32 hash = utSymGetHashValue(dvClassGetSym(theClass));

    if(dvClassGetBaseClass(theClass) != dvClassNull) {
        hash = utHashValues(hash, utSymGetHashValue(dvClassGetSym(dvClassGetBaseClass(theClass))));
    }
    hash = utHashValues(hash, dvClassGetMemoryStyle(theClass));
    hash = utHashValues(hash, dvClassGetReferenceSize(theClass));
    dvForeachClassProperty(theClass, property) {
        hash ^= findPropertyHash(property);
    } dvEndClassProperty;
    dvForeachClassUnion(theClass, theUnion) {
        hash ^= findUnionHash(theUnion);
    } dvEndClassUnion;
    dvForeachClassChildRelationship(theClass, relationship) {
        hash ^= findRelationshipHash(relationship);
    } dvEndClassChildRelationship;
    return hash;
}

/*--------------------------------------------------------------------------------------------------
  Find a hash summary of the entire module.
--------------------------------------------------------------------------------------------------*/
static uint32 findModuleHash(
    dvModule module)
{
    dvEnum theEnum;
    dvClass theClass;
    dvTypedef theTypedef;
    uint32 hash = utSymGetHashValue(dvModuleGetSym(module));

    hash = utHashValues(hash, dvModulePersistent(module));
    hash = utHashValues(hash, utSymGetHashValue(dvModuleGetPrefixSym(module)));
    dvForeachModuleEnum(module, theEnum) {
        hash ^= findEnumHash(theEnum);
    } dvEndModuleEnum;
    dvForeachModuleTypedef(module, theTypedef) {
        hash ^= utSymGetHashValue(dvTypedefGetSym(theTypedef));
    } dvEndModuleTypedef;
    dvForeachModuleClass(module, theClass) {
        hash ^= findClassHash(theClass);
    } dvEndModuleClass;
    return hash;
}

/*--------------------------------------------------------------------------------------------------
  Compute a hash value that depends on every aspect of the database.  Two databases with the same
  hash value should be 100% compatible.
--------------------------------------------------------------------------------------------------*/
uint32 dvComputeDatabaseHash(void)
{
    dvModule module;
    uint32 hash = 0;

    dvForeachRootModule(dvTheRoot, module) {
        hash ^= findModuleHash(module);
    } dvEndRootModule;
    return hash;
}

/*--------------------------------------------------------------------------------------------------
  Find the module prefix to use for the class.  It is the prefix of the base class, if defined,
  otherwise the prefix of the owning module.
--------------------------------------------------------------------------------------------------*/
char *dvClassGetPrefix(
    dvClass theClass)
{
    dvClass baseClass;

    utDo {
        baseClass = dvClassGetBaseClass(theClass);
    } utWhile(baseClass != dvClassNull) {
        theClass = baseClass;
    } utRepeat;
    return dvModuleGetPrefix(dvClassGetModule(theClass));
}


/*--------------------------------------------------------------------------------------------------
  Find the module prefix to use for the property.  It is the prefix of the owning module which 
  can be different from the prefix of the owningClass
--------------------------------------------------------------------------------------------------*/
char *dvPropertyGetPrefix(
    dvProperty property)
{

    return dvModuleGetPrefix(dvClassGetModule(dvPropertyGetClass(property)));
}

/*--------------------------------------------------------------------------------------------------
  Return a symbol with the same name, but the first letter capitalized.
--------------------------------------------------------------------------------------------------*/
utSym dvUpperSym(
    utSym sym)
{
    char *name;

    if(sym == utSymNull) {
        return utSymNull;
    }
    name = utCopyString(utSymGetName(sym));
    *name = toupper((unsigned char)*name);
    return utSymCreate(name);
}

/*--------------------------------------------------------------------------------------------------
  Return the format string for printing a property value.
--------------------------------------------------------------------------------------------------*/
char *dvFindPropertyFormatString(
    dvProperty property)
{
    switch(dvPropertyGetType(property)) {
    case PROP_INT: return "%d";
    case PROP_UINT: return "%u";
    case PROP_FLOAT: return "%g";
    case PROP_DOUBLE: return "%g";
    case PROP_BIT: return "%u";
    case PROP_BOOL: return "%u";
    case PROP_CHAR:
        if(dvPropertyArray(property)) {
            return "\\\"%s\\\"";
        }
        return "%c";
    case PROP_SYM: return "0x%x \\\"%s\\\"";
    case PROP_ENUM: return "%u";
    case PROP_TYPEDEF:
        return "???";
    case PROP_POINTER: return "0x%x";
    default:
        utExit("Unknown property type");
    }
    return NULL; /* Dummy return */
}

/*--------------------------------------------------------------------------------------------------
  Just return a string representing the object reference type.
--------------------------------------------------------------------------------------------------*/
char *dvClassGetReferenceTypeName(
    dvClass theClass)
{
    return utSprintf("uint%u", dvClassGetReferenceSize(theClass));
}

/*--------------------------------------------------------------------------------------------------
  Just determine if any class in the module was declared with the "attributes" class option.
--------------------------------------------------------------------------------------------------*/
bool dvModuleHasClassAttributes(
    dvModule module)
{
    dvClass theClass;

    dvForeachModuleClass(module, theClass) {
        if(dvClassGenerateAttributes(theClass)) {
            return true;
        }
    } dvEndModuleClass;
    return false;
}

/*--------------------------------------------------------------------------------------------------
  Determine if this is a name-based hash relationship.
--------------------------------------------------------------------------------------------------*/
bool dvRelationshipHashedByName(
    dvRelationship relationship)
{
    dvProperty property;
    dvKey key = dvRelationshipGetFirstKey(relationship);
    dvKeyproperty keyproperty = dvKeyGetFirstKeyproperty(key);

    if(dvKeyGetNextRelationshipKey(key) != dvKeyNull) {
        return false;
    }
    if(dvKeypropertyGetNextKeyKeyproperty(keyproperty) != dvKeypropertyNull) {
        return false;
    }
    property = dvKeypropertyGetProperty(key);
    if(strcmp(dvPropertyGetName(property), utSprintf("%sSym", dvRelationshipGetChildLabel(relationship)))) {
        return false;
    }
    return dvPropertyGetType(property) == PROP_SYM;
}

/*--------------------------------------------------------------------------------------------------
  Find the initial value for a property.
--------------------------------------------------------------------------------------------------*/
char *dvPropertyFindInitializer(
    dvProperty property)
{
    dvPropertyType type = dvPropertyGetType(property);
    dvTypedef theTypedef;
    dvEnum theEnum;

    if(dvPropertyGetNumInitializer(property) != 0) {
        return dvPropertyGetInitializer(property);
    }
    if(type == PROP_POINTER) {
        return utSprintf("%s%sNull", dvClassGetPrefix(dvPropertyGetClassProp(property)),
               dvClassGetName(dvPropertyGetClassProp(property)));
    } else if(type == PROP_SYM) {
        return "utSymNull";
    } else if(type == PROP_TYPEDEF) {
        theTypedef = dvPropertyGetTypedefProp(property);
        if(dvTypedefGetNumInitializer(theTypedef) != 0) {
            return dvTypedefGetInitializer(theTypedef);
        }
    } else if(type == PROP_ENUM) {
        theEnum = dvPropertyGetEnumProp(property);
        return utSprintf("%s%s", utSymGetName(dvEnumGetPrefixSym(theEnum)), dvEntryGetName(dvEnumGetFirstEntry(theEnum)));
    }
    return "0";
}

/*--------------------------------------------------------------------------------------------------
  Format a access function for just this property.
--------------------------------------------------------------------------------------------------*/
char *dvPropertyGetAccessMacro(
    dvProperty property,
    bool useParamName,
    char * param)
{
    dvClass childClass = dvPropertyGetClass(property);
    dvPropertyType type = dvPropertyGetType(property);
    char *accessMacro;
    if(useParamName) {
        accessMacro = dvPropertyGetName(property);
    } else {
        if(!dvPropertyArray(property) && (type == PROP_BOOL || type == PROP_BIT)) {
            accessMacro = utSprintf("%s%s%s(%s)", dvPropertyGetPrefix(property), dvClassGetName(childClass),
                dvPropertyGetName(property), param);
        } else {
            accessMacro = utSprintf("%s%sGet%s(%s)", dvPropertyGetPrefix(property), dvClassGetName(childClass),
                dvPropertyGetName(property), param);
        }
    }
    return accessMacro;
}


/*--------------------------------------------------------------------------------------------------
  Format a length function for just this property.
--------------------------------------------------------------------------------------------------*/
char *dvPropertyGetLengthMacro(
    dvProperty property,
    bool useParamName,
    char * param)
{
    dvClass childClass = dvPropertyGetClass(property);
    char *lengthMacro;
    if(useParamName) {
        if(!dvPropertyFixedSize(property)) {
            lengthMacro = utSprintf("%sLength", dvPropertyGetName(property));
        } else {
            lengthMacro = utSprintf("(%s)", dvPropertyGetIndex(property));
        }
    } else {
        if(!dvPropertyFixedSize(property)) {
            lengthMacro = utSprintf("%s%sGetNum%s(%s)", dvPropertyGetPrefix(property), dvClassGetName(childClass),
                dvPropertyGetName(property), param);
        } else {
            lengthMacro = utSprintf("(%s)", dvPropertyGetIndex(property));
        }
    }
    return lengthMacro;
}

/*--------------------------------------------------------------------------------------------------
  Format a access function for just this key.
--------------------------------------------------------------------------------------------------*/
char *dvKeyGetAccessMacro(
    dvKey key,
    bool useParamName,
    char * param)
{
    dvKeyproperty keyproperty = dvKeyGetLastKeyproperty(key);
    dvProperty property = dvKeypropertyGetProperty(keyproperty);

    char *accessMacro = param;

    dvForeachKeyKeyproperty(key, keyproperty) {
        property = dvKeypropertyGetProperty(keyproperty);
        accessMacro = dvPropertyGetAccessMacro(property, useParamName, accessMacro);
    } dvEndKeyKeyproperty;
    
    return accessMacro;
}

/*--------------------------------------------------------------------------------------------------
  Format a length function for just this key.
--------------------------------------------------------------------------------------------------*/
char *dvKeyGetLengthMacro(
    dvKey key,
    bool useParamName,
    char *param)
{
    dvKeyproperty keyproperty = dvKeyGetLastKeyproperty(key);
    dvProperty property = dvKeypropertyGetProperty(keyproperty);

    char *lengthMacro = param;

    dvForeachKeyKeyproperty(key, keyproperty) {
        property = dvKeypropertyGetProperty(keyproperty);
        if(dvKeypropertyGetNextKeyKeyproperty(keyproperty) != dvKeypropertyNull) {
            lengthMacro = dvPropertyGetAccessMacro(property, useParamName, lengthMacro);
        }
        else {
            lengthMacro = dvPropertyGetLengthMacro(property, useParamName, lengthMacro);
        }
    } dvEndKeyKeyproperty;

    return lengthMacro;
}


