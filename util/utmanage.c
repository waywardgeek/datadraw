/*
 * This file was written by Bill Cox.  It is hereby placed into the public domain.
 */

/*--------------------------------------------------------------------------------------------------
  This module provides basic database administration capabilities.
--------------------------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <ctype.h>
#include "ddutil.h"
#include "utpersist.h"

static char *utLineBuffer;
static uint32 utLineSize, utLinePosition;
static uint64 utLineNum;
static bool utAtEndOfFile;
static char *utLastToken;
static bool utShowHidden;
static FILE *utInputFile, *utOutputFile;
static utField *utFieldTranslationTable;
static uint32 utNumFields;

/*--------------------------------------------------------------------------------------------------
  Read a module header.
--------------------------------------------------------------------------------------------------*/
static void reportError(
    char *format,
    ...)
{
    char *buff;
    va_list ap;

    va_start(ap, format);
    buff = utVsprintf(format, ap);
    va_end(ap);
    if(utInputFile != stdin) {
        utWarning("Line %lld, token \"%s\": %s", utLineNum, utLastToken, buff);
    } else {
        utWarning("Token \"%s\": %s", utLastToken, buff);
    }
}

/*--------------------------------------------------------------------------------------------------
  Return an integer of the given size.
--------------------------------------------------------------------------------------------------*/
uint64 utFindIntValue(
    void *values,
    uint8 size)
{
    switch(size) {
    case 1: return *(uint8 *)values;
    case 2: return *(uint16 *)values;
    case 4: return *(uint32 *)values;
    case 8: return *(uint64 *)values;
    }
    utExit("Illegal integer size");
    return 0;
}

/*--------------------------------------------------------------------------------------------------
  Print out help.
--------------------------------------------------------------------------------------------------*/
static void printHelp(void)
{
    fprintf(utOutputFile, 
        "create <module> <class> - allocate a new object, and return it's object number\n"
        "compact - Compact the database, and delete the recent_changes file\n"
        "destroy <module> <class> <object number> - Destroy an object\n"
        "help - this command\n"
        "list - list the modules in the database, their object counts and memory usage\n"
        "list <module> - list classes in the module, their object counts and memory usage\n"
        "list <module> <class> - list fields of a class\n"
        "quit - quit the database manager\n"
        "set <module> <class> <object number> = comma separated values – \n"
        "    set all fields of an object\n"
        "set <module> <class> <object number> <field> = <value> - set a value\n"
        "show <module> <class> - show all field values of all objects of the class\n"
        "show <module> <class> <object number> - show an object's fields\n"
        "show_hidden <true or false> - Enable/disable listing of DataDraw internal fields\n"
        "load_binary <file> - Read the data from the binary database file into the database\n"
        "save_binary <file> - Write out the database in binary format to the file\n"
        "load_text <file> - Read the data from the text database file into the database\n"
        "save_text <file> - Write out the database in text format to the file\n");
}

/*--------------------------------------------------------------------------------------------------
  Return the number of objects allocated for the module.
--------------------------------------------------------------------------------------------------*/
static uint64 countModuleObjects(
    utModule module)
{
    utClass theClass;
    uint64 numObjects = 0;

    utForeachModuleClass(module, theClass) {
        numObjects += utFindIntValue(utClassGetNumUsedPtr(theClass), utClassGetReferenceSize(theClass));
    } utEndModuleClass;
    return numObjects;
}

/*--------------------------------------------------------------------------------------------------
  Return the total memory size used by the class.
--------------------------------------------------------------------------------------------------*/
static uint64 findClassMemory(
    utClass theClass)
{
    utField field;
    uint64 memory = 0;
    uint64 numUsed;

    utForeachClassField(theClass, field) {
        if(utFieldArray(field)) {
            if(!utFieldFixedSize(field)) {
                numUsed = *utFieldGetNumUsedPtr(field);
            } else {
                numUsed = utFindIntValue(utClassGetNumUsedPtr(theClass), utClassGetReferenceSize(theClass));
                numUsed *= utFieldGetLength(field);
            }
        } else {
            numUsed = utFindIntValue(utClassGetNumUsedPtr(theClass), utClassGetReferenceSize(theClass));
        }
        memory += utFieldGetSize(field)*numUsed;
    } utEndClassField;
    return memory;
}

/*--------------------------------------------------------------------------------------------------
  List the modules in the database.
--------------------------------------------------------------------------------------------------*/
static uint64 findModuleMemory(
    utModule module)
{
    utClass theClass;
    uint64 memory = 0;

    utForeachModuleClass(module, theClass) {
        memory += findClassMemory(theClass);
    } utEndModuleClass;
    return memory;
}

/*--------------------------------------------------------------------------------------------------
  Find the ammount in memory units.
--------------------------------------------------------------------------------------------------*/
char *utMemoryUnits(
    uint64 memory)
{
#if (_MSC_VER >= 1200) && (_MSC_VER < 1300)
   //for Visual Studio 6 use a signed __int64
   signed __int64 tempMem = memory;
   if(memory > 1024*1024) {
      return utSprintf("%.5g MB", tempMem/(1024.0*1024.0));
   } else if(memory > 1024) {
      return utSprintf("%.5g KB", tempMem/1024.0);
   }
   return utSprintf("%lu bytes", tempMem);
#else
   if(memory > 1024*1024) {
       return utSprintf("%.5g MB", memory/(1024.0*1024.0));
   } else if(memory > 1024) {
       return utSprintf("%.5g KB", memory/1024.0);
   }
   return utSprintf("%lu bytes", memory);
#endif
}

/*--------------------------------------------------------------------------------------------------
  List the modules in the database.
--------------------------------------------------------------------------------------------------*/
static void listModules(void)
{
    utModule module;

    fprintf(utOutputFile, "modules:\n");
    utForeachModule(module) {
        if(utModuleInitialized(module)) {
            fprintf(utOutputFile, "    %s - %llu objects, %s memory\n", utModuleGetPrefix(module),
                countModuleObjects(module), utMemoryUnits(findModuleMemory(module)));
        }
    } utEndModule;
}

/*--------------------------------------------------------------------------------------------------
  List the classes in the module.
--------------------------------------------------------------------------------------------------*/
static void listClasses(
    utModule module)
{
    utClass theClass;
    uint64 numUsed;

    fprintf(utOutputFile, "Module %s classes:\n", utModuleGetPrefix(module));
    utForeachModuleClass(module, theClass) {
        numUsed = utFindIntValue(utClassGetNumUsedPtr(theClass), utClassGetReferenceSize(theClass));
        fprintf(utOutputFile, "    %s - %llu objects, %s memory\n", utClassGetName(theClass), numUsed,
            utMemoryUnits(findClassMemory(theClass)));
    } utEndModuleClass;
}

/*--------------------------------------------------------------------------------------------------
  Just return a name for the field type.
--------------------------------------------------------------------------------------------------*/
static char *findBaseFieldTypeName(
    utField field)
{
    switch(utFieldGetType(field)) {
    case UT_INT: return utSprintf("int%u", utFieldGetSize(field) << 3);
    case UT_UINT: return utSprintf("uint%u", utFieldGetSize(field) << 3);
    case UT_FLOAT: return "float";
    case UT_DOUBLE: return "double";
    case UT_BIT: return "bit";
    case UT_BOOL: return "bool";
    case UT_CHAR: return "char";
    case UT_SYM: return "sym";
    case UT_ENUM: return "enum";
    case UT_TYPEDEF: return "typedef";
    case UT_POINTER: return utSprintf("pointer %s", utFieldGetDestName(field));
    case UT_UNION: return utSprintf("union %s", utFieldGetDestName(field));
    default:
        utExit("Unknown property type");
    }
    return NULL; /* Dummy return */
}

/*--------------------------------------------------------------------------------------------------
  Just return a name for the field type.
--------------------------------------------------------------------------------------------------*/
static char *findFieldTypeName(
    utField field)
{
    char *baseName = findBaseFieldTypeName(field);

    if(utFieldArray(field)) {
        if(!utFieldFixedSize(field)) {
            return utSprintf("array %s", baseName);
        }
        return utSprintf("array %s[%u]", baseName, utFieldGetLength(field));
    }
    return baseName;
}

/*--------------------------------------------------------------------------------------------------
  List the fields of the class.
--------------------------------------------------------------------------------------------------*/
static void listFields(
    utClass theClass)
{
    utField field;
    char *length;

    fprintf(utOutputFile, "Class %s fields:\n", utClassGetName(theClass));
    utForeachClassField(theClass, field) {
        if(!utFieldHidden(field) || utShowHidden) {
            length = "";
            if(utFieldFixedSize(field)) {
                length = utSprintf("[%u]", utFieldGetLength(field));
            }
            fprintf(utOutputFile, "    %s %s%s\n", findFieldTypeName(field), utFieldGetName(field), length);
        }
    } utEndClassField;
}

/*--------------------------------------------------------------------------------------------------
  Find the enum with the given name.
--------------------------------------------------------------------------------------------------*/
static utEnum findEnum(
    utModule module,
    char *name)
{
    utEnum theEnum;

    utForeachModuleEnum(module, theEnum) {
        if(!strcmp(utEnumGetName(theEnum), name)) {
            return theEnum;
        }
    } utEndModuleEnum;
    return NULL;
}

/*--------------------------------------------------------------------------------------------------
  Find the entry with the given value.
--------------------------------------------------------------------------------------------------*/
static utEntry findEntryFromValue(
    utEnum theEnum,
    uint32 value)
{
    utEntry entry;

    utForeachEnumEntry(theEnum, entry) {
        if(utEntryGetValue(entry) == value) {
            return entry;
        }
    } utEndEnumEntry;
    return NULL;
}

/*--------------------------------------------------------------------------------------------------
  Find the entry with the given name.
--------------------------------------------------------------------------------------------------*/
static utEntry findEntryFromName(
    utEnum theEnum,
    char *name)
{
    utEntry entry;

    utForeachEnumEntry(theEnum, entry) {
        if(!strcmp(utEntryGetName(entry), name)) {
            return entry;
        }
    } utEndEnumEntry;
    return NULL;
}

/*--------------------------------------------------------------------------------------------------
  Return a hexidecimal digit representation of the 4-bit value.
--------------------------------------------------------------------------------------------------*/
static char findHexDigit(
    uint8 value)
{
    if(value <= 9) {
        return '0' + value;
    }
    return 'A' + value - 10;
}

/*--------------------------------------------------------------------------------------------------
  Return a long hexidecimal string representing the binary value.
--------------------------------------------------------------------------------------------------*/
char *utFindHexString(
    uint8 *values,
    uint32 size)
{
    char *buffer = utMakeString((size << 1) + 1);
    char *p = buffer + (size << 1) - 1;

    buffer[size*2] = '\0';
    while(size-- != 0) {
        *p-- = findHexDigit((uint8)(*values & 0xf));
        *p-- = findHexDigit((uint8)((*values++) >> 4));
    }
    return buffer;
}

/*--------------------------------------------------------------------------------------------------
  Count the non "isprint" characters in the string.
--------------------------------------------------------------------------------------------------*/
static uint32 countNonprintableChars(
    char *string)
{
    uint32 nonPrintableChars = 0;

    while(*string) {
        if(!isprint((unsigned char)*string)) {
            nonPrintableChars++;
        }
        string++;
    }
    return nonPrintableChars;
}

/*--------------------------------------------------------------------------------------------------
  Convert any non-string friendly characters to \" or \<number> format.
--------------------------------------------------------------------------------------------------*/
static char *mungeString(
    char *string)
{
    char *buffer = utMakeString(strlen(string) + 4*countNonprintableChars(string) + 1);
    char *p = buffer;
    char intValue[4];

    while(*string != '\0') {
        if(isprint((unsigned char)*string)) {
            if(*string == '"' || *string == '\\') {
                *p++ = '\\';
            }
            *p++ = *string++;
        } else {
            sprintf(intValue, "\\%u", *string);
            intValue[3] = '\0';
            strcpy(p, intValue);
            p += strlen(intValue);
            string++;
        }
    }
    *p = '\0';
    return buffer;
}

/*--------------------------------------------------------------------------------------------------
  Find the union case based on the switch field.
--------------------------------------------------------------------------------------------------*/
static utUnioncase findUnioncase(
    utUnion theUnion,
    uint64 objectNumber)
{
    utField switchField = utUnionGetSwitchField(theUnion);
    utUnioncase unioncase;
    uint32 value = *(*(uint32 **)(utFieldGetArrayPtr(switchField)) + objectNumber);

    utForeachUnionUnioncase(theUnion, unioncase) {
        if(utUnioncaseGetValue(unioncase) == value) {
            return unioncase;
        }
    } utEndUnionUnioncase;
    return NULL;
}

/*--------------------------------------------------------------------------------------------------
  Return a string representation of the field value.
--------------------------------------------------------------------------------------------------*/
static char *findFieldValue(
    utField field,
    uint64 objectNumber)
{
    utModule module;
    utClass theClass;
    utEnum theEnum;
    utEntry entry;
    utUnion theUnion;
    utUnioncase unioncase;
    utSym sym;
    uint8 *values;
    uint32 size = utFieldGetSize(field);
    utFieldType type = utFieldGetType(field);

    if(type == UT_BIT) {
        values = *(uint8 **)(utFieldGetArrayPtr(field)) + (objectNumber >> 3)*utFieldGetSize(field);
    } else {
        values = *(uint8 **)(utFieldGetArrayPtr(field)) + objectNumber*utFieldGetSize(field);
    }
    if(type == UT_UNION) {
        theUnion = utFieldGetUnion(field);
        unioncase = findUnioncase(theUnion, objectNumber);
        if(unioncase != NULL) {
            type = utUnioncaseGetType(unioncase);
            size = utUnioncaseGetSize(unioncase);
        }
    }
    switch(type) {
    case UT_BIT:
        if((*values >> (objectNumber & 7)) & 1) {
            return "true";
        }
        return "false";
    case UT_BOOL:
        if(*values == 1) {
            return "true";
        } else if(*values == 0) {
            return "false";
        }
        return utSprintf("%d", *values);
    case UT_INT:
        switch(size) {
        case 1: return utSprintf("%d", *(int8 *)values);
        case 2: return utSprintf("%d", *(int16 *)values);
        case 4: return utSprintf("%d", *(int32 *)values);
        case 8: return utSprintf("%d", *(int64 *)values);
        }
        utExit("Invalid integer size");
        break;
    case UT_UINT:
        switch(size) {
        case 1: return utSprintf("%u", *(uint8 *)values);
        case 2: return utSprintf("%u", *(uint16 *)values);
        case 4: return utSprintf("%u", *(uint32 *)values);
        case 8: return utSprintf("%u", *(uint64 *)values);
        }
        utExit("Invalid integer size");
        break;
    case UT_CHAR:
        if(isprint(*values)) {
            return utSprintf("'%c'", *values);
        }
        return utSprintf("'\\%u'", *values);
    case UT_FLOAT:
        return utSprintf("%g", *(float *)values);
    case UT_DOUBLE:
        return utSprintf("%g", *(double *)values);
    case UT_POINTER:
        switch(size) {
        case 1: return utSprintf("0x%x", *(uint8 *)values);
        case 2: return utSprintf("0x%x", *(uint16 *)values);
        case 4: return utSprintf("0x%x", *(uint32 *)values);
        case 8: return utSprintf("0x%x", *(uint64 *)values);
        }
        break;
    case UT_TYPEDEF: case UT_UNION:
        return utFindHexString(values, size);
    case UT_ENUM:
        theClass = utFieldGetClass(field);
        module = utClassGetModule(theClass);
        theEnum = findEnum(module, utFieldGetDestName(field));
        entry = findEntryFromValue(theEnum, *(uint32 *)values);
        return utEntryGetName(entry);
    case UT_SYM:
        sym = *(utSym *)values;
        if(sym == utSymNull) {
            return utSprintf("0x%x", utSymNull);
        }
        if((uint32)(sym - (utSym)0) < utRootData.usedSym) {
            return utSprintf("\"%s\"", mungeString(utSymGetName(*(utSym *)values)));
        }
        return utSprintf("invalid:0x%x", (sym - (utSym)0));
    }
    utExit("Unknow field type");
    return NULL;
}

/*--------------------------------------------------------------------------------------------------
  Determine if the only '\0' is at the end of the character array.
--------------------------------------------------------------------------------------------------*/
static bool zeroTerminated(
    char *values,
    uint32 length)
{
    if(values[--length] != '\0') {
        return false;
    }
    while(length != 0 && values[length] != '\0') {
        length--;
    }
    return values[length] == '\0';
}

/*--------------------------------------------------------------------------------------------------
  Show the data in the array.  If we go over maxFields, we print '...'.
--------------------------------------------------------------------------------------------------*/
static void writeArray(
    utField field,
    uint64 objectNumber,
    uint32 maxFields)
{
    uint32 numElements;
    uint32 xElement;
    bool firstTime = true;
    char *values = utFieldGetGetValues(field)(objectNumber, &numElements);
    uint32 firstElement = (values - *(char **)(utFieldGetArrayPtr(field)))/utFieldGetSize(field);

    if(numElements != 0 && utFieldGetType(field) == UT_CHAR && zeroTerminated(values, numElements)) {
        fprintf(utOutputFile, "\"%s\"", mungeString(values));
        return;
    }
    fprintf(utOutputFile, "(");
    for(xElement = 0; xElement < numElements && xElement < maxFields; xElement++) {
        if(!firstTime) {
            fprintf(utOutputFile, ", ");
        }
        firstTime = false;
        fprintf(utOutputFile, "%s", findFieldValue(field, firstElement + xElement));
    }
    if(xElement == maxFields) {
        fprintf(utOutputFile, ", ...");
    }
    fprintf(utOutputFile, ")");
}

/*--------------------------------------------------------------------------------------------------
  Show the fields of an object.
--------------------------------------------------------------------------------------------------*/
static void showObject(
    utClass theClass,
    uint64 objectNumber)
{
    utField field;
    bool firstTime = true;

    utForeachClassField(theClass, field) {
        if(!utFieldHidden(field) || utShowHidden) {
            if(!firstTime) {
                fprintf(utOutputFile, ", ");
            }
            firstTime = false;
            fprintf(utOutputFile, "%s=", utFieldGetName(field));
            if(!utFieldArray(field)) {
                fprintf(utOutputFile, "%s", findFieldValue(field, objectNumber));
            } else {
                writeArray(field, objectNumber, 16);
            }
        }
    } utEndClassField;
    fprintf(utOutputFile, "\n");
}

/*--------------------------------------------------------------------------------------------------
  Just print the field names.
--------------------------------------------------------------------------------------------------*/
static void printColumnHeaders(
    utClass theClass)
{
    utField field;
    bool firstTime = true;

    fprintf(utOutputFile, "ObjectNumber, ");
    utForeachClassField(theClass, field) {
        if(!utFieldHidden(field)) {
            if(!firstTime) {
                fprintf(utOutputFile, ", ");
            }
            firstTime = false;
            fprintf(utOutputFile, "%s", utFieldGetName(field));
        }
    } utEndClassField;
    fprintf(utOutputFile, "\n");
}

/*--------------------------------------------------------------------------------------------------
  Just print the field types.
--------------------------------------------------------------------------------------------------*/
static void printColumnTypes(
    utClass theClass)
{
    utField field;
    bool firstTime = true;

    fprintf(utOutputFile, "uint%u, ", utClassGetReferenceSize(theClass) << 3);
    utForeachClassField(theClass, field) {
        if(!utFieldHidden(field)) {
            if(!firstTime) {
                fprintf(utOutputFile, ", ");
            }
            firstTime = false;
            fprintf(utOutputFile, "%s", findFieldTypeName(field));
        }
    } utEndClassField;
    fprintf(utOutputFile, "\n");
}

/*--------------------------------------------------------------------------------------------------
  Print the object's values in a comma separated list.
--------------------------------------------------------------------------------------------------*/
static void showObjectFields(
    utClass theClass,
    uint64 objectNumber)
{
    utField field;
    bool firstTime = true;

    fprintf(utOutputFile, "0x%llx, ", objectNumber);
    utForeachClassField(theClass, field) {
        if(!utFieldHidden(field) || utShowHidden) {
            if(!firstTime) {
                fprintf(utOutputFile, ", ");
            }
            firstTime = false;
            if(!utFieldArray(field)) {
                fprintf(utOutputFile, "%s", findFieldValue(field, objectNumber));
            } else {
                writeArray(field, objectNumber, UINT32_MAX);
            }
        }
    } utEndClassField;
    fprintf(utOutputFile, "\n");
}

/*--------------------------------------------------------------------------------------------------
  Find the base class for this class.
--------------------------------------------------------------------------------------------------*/
utClass utClassGetBaseClass(
    utClass theClass)
{
    uint16 baseClassIndex = utClassGetBaseClassIndex(theClass);

    if(baseClassIndex == UINT16_MAX) {
        return utClassNull;
    }
    return utClasses + baseClassIndex;
}

/*--------------------------------------------------------------------------------------------------
  Find the root class for this class, where we will find the memory management stuff.
--------------------------------------------------------------------------------------------------*/
utClass utClassFindRootClass(
    utClass theClass)
{
    utClass baseClass;

    utDo {
        baseClass = utClassGetBaseClass(theClass);
    } utWhile(baseClass != utClassNull) {
        theClass = baseClass;
    } utRepeat;
    return theClass;
}

/*--------------------------------------------------------------------------------------------------
  Set the used flags for the objects of the class.
--------------------------------------------------------------------------------------------------*/
static void setObjectFreeFlags(
    utClass theClass,
    uint8 *objectFree)
{
    utField field;
    uint64 objectNumber;

    theClass = utClassFindRootClass(theClass);
    field = utClassGetNextFreeField(theClass);
    objectNumber = utFindIntValue(utClassGetFirstFreePtr(theClass), utClassGetReferenceSize(theClass));
    while(objectNumber != 0) {
        objectFree[objectNumber >> 3] |= 1 << (objectNumber & 7);
        objectNumber = utFindIntValue(*(uint8 **)(utFieldGetArrayPtr(field)) + objectNumber*utFieldGetSize(field),
            (uint8)utFieldGetSize(field));
    }
}

/*--------------------------------------------------------------------------------------------------
  Show all the objects of a class.
--------------------------------------------------------------------------------------------------*/
static void showClass(
    utModule module,
    utClass theClass)
{
    uint64 numUsed = utFindIntValue(utClassGetNumUsedPtr(theClass), utClassGetReferenceSize(theClass));
    uint64 objectNumber;
    bool hasFreeList = utClassGetDestructor(theClass) != NULL;
    uint64 spaceNeeded = (numUsed + 7) >> 3;
    uint8 *objectFree = NULL;

    if(hasFreeList) {
        objectFree = calloc(spaceNeeded, sizeof(uint8));
        setObjectFreeFlags(theClass, objectFree);
    }
    fprintf(utOutputFile, "class %s %llu\n", utClassGetName(theClass), numUsed);
    printColumnHeaders(theClass);
    printColumnTypes(theClass);
    for(objectNumber = 1; objectNumber < numUsed; objectNumber++) {
        if(!hasFreeList || !(objectFree[objectNumber >> 3] & (1 << (objectNumber & 7)))) {
            showObjectFields(theClass, objectNumber);
        }
    }
    if(hasFreeList) {
        free(objectFree);
    }
}

/*--------------------------------------------------------------------------------------------------
  Write the integer, given the width.
--------------------------------------------------------------------------------------------------*/
void utSetInteger(
    uint8 *dest,
    uint64 value,
    uint8 width)
{
    switch(width) {
    case 1:
        *dest = (uint8)value;
        break;
    case 2:
        *(uint16 *)dest = (uint16)value;
        break;
    case 4:
        *(uint32 *)dest = (uint32)value;
        break;
    case 8:
        *(uint64 *)dest = (uint64)value;
        break;
    default:
        utExit("Invalid integer width");
    }
}

/*--------------------------------------------------------------------------------------------------
  Parse the integer and return it's value.  Allow hex if we see "0x" in front.  Allow negation.
  Set passed to indicate success.
--------------------------------------------------------------------------------------------------*/
bool utParseInteger(
    int64 *dest,
    char *string)
{
    uint64 value = 0, oldValue = 0;
    char c;
    uint8 digit;
    bool negate = false;

    if(*string == '0' && *(string + 1) == 'x') {
        string += 2;
        while(*string) {
            c = toupper((unsigned char)*string++);
            if(c >= '0' && c <= '9') {
                digit = c - '0';
            } else if(c >= 'A' && c <= 'F') {
                digit = c - 'A' + 10;
            } else {
                return false;
            }
            value <<= 4;
            value |= digit;
            if(value < oldValue) {
                return false;
            }
            oldValue = value;
        }
    } else {
        if(*string == '-') {
            string++;
            negate = true;
        }
        while(*string) {
            c = *string++;
            if(c < '0' || c > '9') {
                return false;
            }
            value *= 10;
            value += c - '0';
            if(value < oldValue) {
                return false;
            }
            oldValue = value;
        }
    }
    *dest = negate? -value : value;
    return true;
}

/*--------------------------------------------------------------------------------------------------
  Read an integer and complain if it has bad width or syntax.
--------------------------------------------------------------------------------------------------*/
static bool readInteger(
    uint8 *dest,
    char *string,
    uint8 width,
    bool unsignedInt)
{
    int64 value;

    if(!utParseInteger(&value, string)) {
        return false;
    }
    utSetInteger(dest, value, width);
    return true;
}

/*--------------------------------------------------------------------------------------------------
  Parse a character value.
--------------------------------------------------------------------------------------------------*/
static bool readChar(
    uint8 *dest,
    char *value)
{
    char *end = value + strlen(value) - 1;

    if(*value++ != '\'') {
        return false;
    }
    if(*--end != '\'') {
        return false;
    }
    *end = '\0';
    if(*value == '\\') {
        return readInteger(dest, value, 1, true);
    }
    *(char *)dest = *value;
    return true;
}

/*--------------------------------------------------------------------------------------------------
  Parse a floating point number.
--------------------------------------------------------------------------------------------------*/
static bool readFloat(
    uint8 *dest,
    char *value,
    bool isFloat)
{
    double floatVal;
    char *endPtr;

    floatVal = strtod(value, &endPtr);
    if(endPtr == NULL || *endPtr != '\0') {
        return false;
    }
    if(isFloat) {
        *(float *)dest = (float)floatVal;
    } else {
        *(double *)dest = floatVal;
    }
    return true;
}

/*--------------------------------------------------------------------------------------------------
  Read a 2-character hex byte.
--------------------------------------------------------------------------------------------------*/
static bool readHex(
    uint8 *dest,
    char *value)
{
    char c = toupper((unsigned char)*value++);
    uint8 byte;

    if(c >= '0' && c <= '9') {
        byte = c - '0';
    } else if(c >= 'A' && c <= 'F') {
        byte = c - 'A' + 10;
    } else {
        return false;
    }
    byte <<= 4;
    c = toupper((unsigned char)*value);
    if(c >= '0' && c <= '9') {
        byte |= c - '0';
    } else if(c >= 'A' && c <= 'F') {
        byte |= c - 'A' + 10;
    } else {
        return false;
    }
    *dest = byte;
    return true;
}

/*--------------------------------------------------------------------------------------------------
  Parse a typedef value.
--------------------------------------------------------------------------------------------------*/
bool utReadHex(
    uint8 *dest,
    char *value,
    uint32 size)
{
    uint32 length = 0;

    while(*value != '\0' && length <= size) {
        if(!readHex(dest, value)) {
            return false;
        }
        dest++;
        value += 2;
        length++;
    }
    return length == size;
}

/*--------------------------------------------------------------------------------------------------
  Convert any \ escapes into characters.
--------------------------------------------------------------------------------------------------*/
static char *unmungeString(
    char *string,
    char **end)
{
    char *buffer = utMakeString(strlen(string) + 1);
    char *p = buffer;
    char value;

    *end = NULL;
    if(*string++ != '"') {
        return NULL;
    }
    while(*string != '\0' && *string != '"') {
        if(*string == '\\') {
            string++;
            if(*string >= '0' && *string <= '9') {
                value = 0;
                do {
                    value = 10*value + *string++ - '0';
                } while(*string >= '0' && *string <= '9');
                *p++ = value;
            } else {
                *p++ = *string++;
            }
        } else {
            if(!isprint((unsigned char)*string)) {
                return NULL;
            }
            *p++ = *string++;
        }
    }
    *p = '\0';
    if(*string == '\0') {
        return NULL;
    }
    *end = string + 1;
    return buffer;
}

/*--------------------------------------------------------------------------------------------------
  Parse a symbol.  Valid values are a string, or 0x0.
--------------------------------------------------------------------------------------------------*/
static bool readSym(
    uint8 *dest,
    char *value)
{
    utSym *symPtr = (utSym *)dest;
    char *end;
    char *string;

    if(*value == '"') {
        string = unmungeString(value, &end);
        if(end == NULL) {
            return false;
        }
        *symPtr = utSymCreate(string);
    } else {
        readInteger(dest, value, sizeof(utSym), true);
        if(*symPtr != utSymNull) {
            return false;
        }
    }
    return true;
}

/*--------------------------------------------------------------------------------------------------
  Parse an enumerated type, and write it to the destination.
--------------------------------------------------------------------------------------------------*/
static bool readEnum(
    utEnum theEnum,
    uint8 *dest,
    char *value)
{
    utEntry entry;

    entry = findEntryFromName(theEnum, value);
    if(entry == NULL) {
        reportError("Unknown enumerated type value %s", value);
        entry = utEnumGetFirstEntry(theEnum);
    }
    *(uint32 *)dest = utEntryGetValue(entry);
    return true;
}

/*--------------------------------------------------------------------------------------------------
  Parse a value of a given type, and write it to the destination.
--------------------------------------------------------------------------------------------------*/
static bool parseSpecificValue(
    uint8 *dest,
    char *value,
    utFieldType type,
    uint32 size,
    uint64 objectNumber)
{
    switch(type) {
    case UT_BIT:
        if(!strcasecmp(value, "true")) {
            *(uint8 *)dest |= 1 << (objectNumber & 7);
        } else if(!strcasecmp(value, "false")) {
            *(uint8 *)dest &= ~(1 << (objectNumber & 7));
        } else {
            return false;
        }
        return true;
    case UT_BOOL:
        if(!strcasecmp(value, "true")) {
            *(bool *)dest = true;
        } else if(!strcasecmp(value, "false")) {
            *(bool *)dest = false;
        } else {
            return readInteger(dest, value, 1, true);
        }
        return true;
    case UT_INT:
        return readInteger(dest, value, (uint8)size, false);
    case UT_UINT:
        return readInteger(dest, value, (uint8)size, true);
    case UT_CHAR:
        return readChar(dest, value);
    case UT_FLOAT:
        return readFloat(dest, value, true);
    case UT_DOUBLE:
        return readFloat(dest, value, true);
    case UT_POINTER:
        return readInteger(dest, value, (uint8)size, true);
    case UT_TYPEDEF:
        return utReadHex(dest, value, size);
    case UT_SYM:
        return readSym(dest, value);
    default:
        utExit("Invalid type");
    }
    return false; /* Dummy return */
}

/*--------------------------------------------------------------------------------------------------
  Parse a value of a given type, and write it to the destination.
--------------------------------------------------------------------------------------------------*/
static bool parseValue(
    uint8 *dest,
    char *value,
    utField field,
    uint64 objectNumber)
{
    utClass theClass = utFieldGetClass(field);
    utModule module = utClassGetModule(theClass);
    utUnion theUnion;
    utUnioncase unioncase;
    utEnum theEnum;

    if(utFieldGetType(field) == UT_ENUM) {
        theEnum = findEnum(module, utFieldGetDestName(field));
        return readEnum(theEnum, dest, value);
    }
    if(utFieldGetType(field) != UT_UNION) {
        return parseSpecificValue(dest, value, utFieldGetType(field), utFieldGetSize(field), objectNumber);
    }
    theUnion = utFieldGetUnion(field);
    unioncase = findUnioncase(theUnion, objectNumber);
    if(unioncase == NULL) {
        return utReadHex(dest, value, utFieldGetSize(field));
    }
    if(utUnioncaseGetType(unioncase) == UT_ENUM) {
        theEnum = findEnum(module, utFieldGetDestName(field));
        return readEnum(theEnum, dest, value);
    }
    return parseSpecificValue(dest, value, utUnioncaseGetType(unioncase), utUnioncaseGetSize(unioncase), objectNumber);
}

/*--------------------------------------------------------------------------------------------------
  Skip to the next comma or the end of a value list.  If we find a syntax error of some sort,
  return NULL.
--------------------------------------------------------------------------------------------------*/
static char *skipToNextComma(
    char *valueList)
{
    char *end, *prevChar;

    if(*valueList == ',') {
        valueList++;
    }
    while(isspace((unsigned char)*valueList)) {
        valueList++;
    }
    if(*valueList == '"') {
        unmungeString(valueList, &end);
        if(end == NULL) {
            return NULL;
        }
        valueList = end;
    } else if(*valueList == '(') {
        valueList++;
        do {
            valueList = skipToNextComma(valueList);
            prevChar = valueList - 1;
            while(isspace((unsigned char)*prevChar)) {
                prevChar--;
            }
        } while(valueList != NULL && *prevChar != ')');
        if(valueList == NULL) {
            return NULL;
        }
    }
    while(*valueList != '\0' && *valueList != ',') {
        valueList++;
    }
    if(*valueList != ',' && *valueList != '\0') {
        return NULL;
    }
    return valueList;
}

/*--------------------------------------------------------------------------------------------------
  Count how many values are in the list.
--------------------------------------------------------------------------------------------------*/
static uint32 countListValues(
    char *valueList)
{
    uint32 numValues = 0;

    while(isspace((unsigned char)*valueList)) {
        valueList++;
    }
    while(valueList != NULL && *valueList != '\0') {
        valueList = skipToNextComma(valueList);
        numValues++;
    }
    if(valueList == NULL || *valueList != '\0') {
        return 0; /* Invalid list */
    }
    return numValues;
}

/*--------------------------------------------------------------------------------------------------
  Return the next element in a list of array values.  Update the list pointer to the next element.
--------------------------------------------------------------------------------------------------*/
static char *parseArrayValue(
    char **valueList)
{
    char *end, *buffer, *prevChar;
    uint32 length;
    bool isArray;

    while(isspace((unsigned char)**valueList)) {
        (*valueList)++;
    }
    isArray = **valueList == '(';
    end = skipToNextComma(*valueList);
    if(end == NULL) {
        return NULL;
    }
    prevChar = end;
    if(isArray) {
        (*valueList)++;
        prevChar = end - 1;
        while(isspace((unsigned char)*prevChar)) {
            prevChar--;
        }
    }
    length = prevChar - *valueList;
    buffer = utMakeString(length + 1);
    strncpy(buffer, *valueList, length);
    buffer[length] = '\0';
    if(*end == ',') {
        end++;
    }
    *valueList = end;
    utLastToken = buffer;
    return buffer;
}

/*--------------------------------------------------------------------------------------------------
  Parse an array of values.
--------------------------------------------------------------------------------------------------*/
static bool parseArray(
    utField field,
    uint64 objectNumber,
    char *valueList)
{
    uint32 numValues, destSize;
    uint8 *dest;
    char *value, *string, *end;

    if(utFieldGetType(field) == UT_CHAR && *valueList == '"') {
        string = unmungeString(valueList, &end);
        if(end == NULL || *end != '\0') {
            return false;
        }
        numValues = strlen(string) + 1;
        dest = utFieldGetAllocValues(field)(objectNumber, numValues);
        strcpy((char *)dest, string);
        return true;
    }
    numValues = countListValues(valueList);
    if(!utFieldFixedSize(field)) {
        dest = utFieldGetAllocValues(field)(objectNumber, numValues);
    } else {
        dest = utFieldGetGetValues(field)(objectNumber, &destSize);
        numValues = utMin(numValues, destSize);
    }
    while(numValues-- != 0) {
        value = parseArrayValue(&valueList);
        if(!parseValue(dest, value, field, objectNumber)) {
            return false;
        }
        dest += utFieldGetSize(field);
    }
    if(*valueList != '\0' && !utFieldFixedSize(field)) {
        return false;
    }
    return true;
}

/*--------------------------------------------------------------------------------------------------
  Set a field of an object.
--------------------------------------------------------------------------------------------------*/
static bool setObjectField(
    utClass theClass,
    utField field,
    uint64 objectNumber,
    char *value)
{
    uint64 numUsed = utFindIntValue(utClassGetNumUsedPtr(theClass), utClassGetReferenceSize(theClass));
    uint8 *dest;
    bool passed;

    if(objectNumber >= numUsed) {
        reportError("Invalid object ID");
        return false;
    }
    if(!utFieldArray(field)) {
        if(utFieldGetType(field) == UT_BIT) {
            dest = *(uint8 **)(utFieldGetArrayPtr(field)) + (objectNumber >> 3)*utFieldGetSize(field);
        } else {
            dest = *(uint8 **)(utFieldGetArrayPtr(field)) + objectNumber*utFieldGetSize(field);
        }
        passed = parseValue(dest, value, field, objectNumber);
    } else {
        passed = parseArray(field, objectNumber, value);
    }
    if(!passed) {
        reportError("Invalid value for field %s", utFieldGetName(field));
    }
    return passed;
}

/*--------------------------------------------------------------------------------------------------
  Set all the fields of an object.
--------------------------------------------------------------------------------------------------*/
static bool setObjectFields(
    utClass theClass,
    uint64 objectNumber,
    char *valueList,
    utField *fieldTranslationTable)
{
    utField field;
    uint16 numValues = countListValues(valueList);
    uint16 xField;
    char *value;

    if(numValues != utClassGetNumFields(theClass) - utClassGetNumHiddenFields(theClass)) {
        reportError("Incorrect number of values for class %s", utClassGetName(theClass));
        return false;
    }
    for(xField = 0; xField < numValues; xField++) {
        if(fieldTranslationTable == NULL) {
            field = utClassGetiField(theClass, xField);
        } else {
            field = fieldTranslationTable[xField];
        }
        value = parseArrayValue(&valueList);
        if(field != NULL && !setObjectField(theClass, field, objectNumber, value)) {
            return false;
        }
    }
    return true;
}

/*--------------------------------------------------------------------------------------------------
  Read a line of text from stdin.
--------------------------------------------------------------------------------------------------*/
static bool readLine(void)
{
    uint32 linePosition = 0;
    int c;

    utDo {
        c = getc(utInputFile);
    } utWhile(c != '\n' && c != EOF) {
        if(linePosition == utLineSize) {
            utLineSize <<= 1;
            utResizeArray(utLineBuffer, utLineSize);
        }
        if(isprint(c)) {
            utLineBuffer[linePosition++] = c;
        }
    } utRepeat;
    if(c == EOF) {
        utAtEndOfFile = true;
    }
    if(linePosition == utLineSize) {
        utLineSize <<= 1;
        utResizeArray(utLineBuffer, utLineSize);
    }
    utLineBuffer[linePosition] = '\0';
    utLinePosition = 0;
    utLineNum++;
    return c != EOF;
}

/*--------------------------------------------------------------------------------------------------
  Find the length of the token.
--------------------------------------------------------------------------------------------------*/
static uint32 findTokenLength(void)
{
    uint32 linePosition = utLinePosition;
    char c = utLineBuffer[linePosition++];

    if(c == '\0') {
        return 0;
    }
    if(!isalnum((unsigned char)c) && c != '_') {
        return 1;
    }
    do {
        c = utLineBuffer[linePosition++];
    } while(isalnum((unsigned char)c) || c == '_');
    return linePosition - utLinePosition - 1;
}

/*--------------------------------------------------------------------------------------------------
  Just skip space.
--------------------------------------------------------------------------------------------------*/
void skipSpace(void)
{
    while(isspace((unsigned char)utLineBuffer[utLinePosition])) {
        utLinePosition++;
    }
}

/*--------------------------------------------------------------------------------------------------
  Read a token from the line buffer.
--------------------------------------------------------------------------------------------------*/
char *readToken(void)
{
    char *token;
    uint32 length;

    skipSpace();
    length = findTokenLength();
    token = utMakeString(length + 1);
    strncpy(token, utLineBuffer + utLinePosition, length);
    utLinePosition += length;
    token[length] = '\0';
    utLastToken = token;
    return token;
}

/*--------------------------------------------------------------------------------------------------
  Find the module with the given prefix.
--------------------------------------------------------------------------------------------------*/
utModule utFindModule(
    char *prefix)
{
    utModule module;
    uint8 xModule;

    for(xModule = 0; xModule < utUsedModules; xModule++) {
        module = utModules + xModule;
        if(!strcmp(utModuleGetPrefix(module), prefix)) {
            return module;
        }
    }
    return NULL;
}

/*--------------------------------------------------------------------------------------------------
  Find the class with the given name.
--------------------------------------------------------------------------------------------------*/
static utClass findClass(
    utModule module,
    char *name)
{
    utClass theClass;

    utForeachModuleClass(module, theClass) {
        if(!strcmp(utClassGetName(theClass), name)) {
            return theClass;
        }
    } utEndModuleClass;
    return NULL;
}

/*--------------------------------------------------------------------------------------------------
  Find the field with the given name.
--------------------------------------------------------------------------------------------------*/
utField utFindField(
    utClass theClass,
    char *name)
{
    utField field;

    utForeachClassField(theClass, field) {
        if(!strcmp(utFieldGetName(field), name)) {
            return field;
        }
    } utEndClassField;
    return NULL;
}

/*--------------------------------------------------------------------------------------------------
  Check that we parsed the whole line, and complain if we didn't.  Return true if the end was
  correct.
--------------------------------------------------------------------------------------------------*/
static bool expectEnd(void)
{
    char *token = readToken();

    if(*token != '\0') {
        reportError("Extra characters at end of command.\n");
        return false;
    }
    return true;
}

/*--------------------------------------------------------------------------------------------------
  Parse a module name.
--------------------------------------------------------------------------------------------------*/
static utModule parseModule(
    bool required,
    bool *error)
{
    utModule module;
    char *token = readToken();

    *error = false;
    if(*token == '\0') {
        if(required) {
            reportError("Expecting a module name.");
            *error = true;
        }
        return NULL;
    }
    module = utFindModule(token);
    if(module == NULL) {
        reportError("Invalid module name.");
        *error = true;
    }
    return module;
}

/*--------------------------------------------------------------------------------------------------
  Parse a class name.
--------------------------------------------------------------------------------------------------*/
static utClass parseClass(
    utModule module,
    bool required,
    bool *error)
{
    utClass theClass;
    char *token = readToken();

    *error = false;
    if(*token == '\0') {
        if(required) {
            reportError("Expecting a class name.");
            *error = true;
        }
        return NULL;
    }
    theClass = findClass(module, token);
    if(theClass == NULL) {
        reportError("Invalid class name.");
        *error = true;
    }
    return theClass;
}

/*--------------------------------------------------------------------------------------------------
  Parse an object number.
--------------------------------------------------------------------------------------------------*/
static uint64 parseObjectNumber(
    utClass theClass,
    bool required,
    bool *error)
{
    uint64 numUsed = utFindIntValue(utClassGetNumUsedPtr(theClass), utClassGetReferenceSize(theClass));
    char *token = readToken();
    uint64 objectNumber;

    *error = false;
    if(*token == '\0') {
        if(required) {
            reportError("Expecting an object number");
            *error = true;
        }
        return UINT64_MAX;
    }
    if(!readInteger((uint8 *)&objectNumber, token, 8, true)) {
        reportError("Invalid number");
        *error = true;
        return UINT64_MAX;
    }
    if(objectNumber >= numUsed) {
        reportError("Object number too large");
        *error = true;
        return UINT64_MAX;
    }
    return objectNumber;
}

/*--------------------------------------------------------------------------------------------------
  Process a list command.
--------------------------------------------------------------------------------------------------*/
static void processListCommand(void)
{
    bool error;
    utModule module = parseModule(false, &error);
    utClass theClass;

    if(error) {
        return;
    }
    if(module == NULL) {
        listModules();
        return;
    }
    theClass = parseClass(module, false, &error);
    if(error) {
        return;
    }
    if(theClass == NULL) {
        listClasses(module);
        return;
    }
    if(!expectEnd()) {
        return;
    }
    listFields(theClass);
}

/*--------------------------------------------------------------------------------------------------
  Process a show command.
--------------------------------------------------------------------------------------------------*/
static void processShowCommand(void)
{
    bool error;
    utModule module = parseModule(true, &error);
    utClass theClass;
    uint64 objectNumber;

    if(error) {
        return;
    }
    theClass = parseClass(module, true, &error);
    if(error) {
        return;
    }
    objectNumber = parseObjectNumber(theClass, false, &error);
    if(error) {
        return;
    }
    if(objectNumber == UINT64_MAX) {
        showClass(module, theClass);
        return;
    }
    if(!expectEnd()) {
        return;
    }
    showObject(theClass, objectNumber);
}

/*--------------------------------------------------------------------------------------------------
  Process a set command.
        "set <module> <class> <object number> = comma separated values – \n"
        "    set all fields of an object\n"
        "set <module> <class> <object number> <field> = <value> – set a field value\n"
--------------------------------------------------------------------------------------------------*/
static void processSetCommand(void)
{
    bool error;
    utModule module = parseModule(true, &error);
    utClass theClass;
    utField field;
    uint64 objectNumber;
    char *token;

    if(error) {
        return;
    }
    theClass = parseClass(module, true, &error);
    if(error) {
        return;
    }
    objectNumber = parseObjectNumber(theClass, true, &error);
    if(error) {
        return;
    }
    token = readToken();
    if(!strcmp(token, "=")) {
        skipSpace();
        setObjectFields(theClass, objectNumber, utLineBuffer + utLinePosition, NULL);
        return;
    }
    field = utFindField(theClass, token);
    if(field == NULL) {
        reportError("Field not found");
        return;
    }
    token = readToken();
    if(strcmp(token, "=")) {
        reportError("Expected '='");
        return;
    }
    skipSpace();
    setObjectField(theClass, field, objectNumber, utLineBuffer + utLinePosition);
}

/*--------------------------------------------------------------------------------------------------
  Process a create command.
--------------------------------------------------------------------------------------------------*/
static void processCreateCommand(void)
{
    bool error;
    utModule module = parseModule(true, &error);
    utClass theClass;

    if(error) {
        return;
    }
    theClass = parseClass(module, true, &error);
    if(error) {
        return;
    }
    if(utClassGetConstructor(theClass) == NULL) {
        reportError("This is a class extension.  Call the base class constructor instead");
        return;
    }
    if(!expectEnd()) {
        return;
    }
    fprintf(utOutputFile, "New %s object 0x%llx\n", utClassGetName(theClass), utClassGetConstructor(theClass)());
}

/*--------------------------------------------------------------------------------------------------
  Destroy an object.
--------------------------------------------------------------------------------------------------*/
static void processDestroyCommand(void)
{
    bool error;
    utModule module = parseModule(true, &error);
    utClass theClass;
    uint64 objectNumber;

    if(error) {
        return;
    }
    theClass = parseClass(module, true, &error);
    if(error) {
        return;
    }
    objectNumber = parseObjectNumber(theClass, true, &error);
    if(error) {
        return;
    }
    if(!expectEnd()) {
        return;
    }
    if(utClassGetDestructor(theClass) == NULL) {
        reportError("This class has no destructor, either because it is an extension, or "
            "because it is declared 'create_only'");
        return;
    }
    utClassGetDestructor(theClass)(objectNumber);
    fprintf(utOutputFile, "Destroyed %s object 0x%llx\n", utClassGetName(theClass), objectNumber);
}

/*--------------------------------------------------------------------------------------------------
  Process a show_hidden command.
--------------------------------------------------------------------------------------------------*/
static void processShowHiddenCommand(void)
{
    char *token = readToken();

    if(!expectEnd()) {
        return;
    }
    if(!strcasecmp(token, "true")) {
        utShowHidden = true;
    } else if(!strcasecmp(token, "false")) {
        utShowHidden = false;
    } else {
        reportError("Expecting 'true' or 'false'\n");
    }
}

/*--------------------------------------------------------------------------------------------------
  Process a load text command.
--------------------------------------------------------------------------------------------------*/
static void processLoadTextCommand(void)
{
    char *fileName = readToken();
    FILE *file;

    if(!expectEnd()) {
        return;
    }
    file = fopen(fileName, "r");
    if(file == NULL) {
        reportError("Could not open file %s for reading", fileName);
        return;
    }
    utLoadTextDatabase(file);
    fclose(file);
    utMemCheck();
}

/*--------------------------------------------------------------------------------------------------
  Process a write command.
--------------------------------------------------------------------------------------------------*/
static void processSaveTextCommand(void)
{
    char *fileName = readToken();
    FILE *file;

    if(!expectEnd()) {
        return;
    }
    file = fopen(fileName, "w");
    if(file == NULL) {
        reportError("Could not open file %s for writing", fileName);
        return;
    }
    utSaveTextDatabase(file);
    fclose(file);
}

/*--------------------------------------------------------------------------------------------------
  Process a load binary command.
--------------------------------------------------------------------------------------------------*/
static void processLoadBinaryCommand(void)
{
    char *fileName = readToken();
    FILE *file;

    if(!expectEnd()) {
        return;
    }
    file = fopen(fileName, "rb");
    if(file == NULL) {
        reportError("Could not open file %s for reading", fileName);
        return;
    }
    utLoadBinaryDatabase(file);
    fclose(file);
}

/*--------------------------------------------------------------------------------------------------
  Process a save binary command.
--------------------------------------------------------------------------------------------------*/
static void processSaveBinaryCommand(void)
{
    char *fileName = readToken();
    FILE *file;

    if(!expectEnd()) {
        return;
    }
    file = fopen(fileName, "wb");
    if(file == NULL) {
        reportError("Could not open file %s for writing", fileName);
        return;
    }
    utSaveBinaryDatabase(file);
    fclose(file);
}

/*--------------------------------------------------------------------------------------------------
  Process the command in the line buffer.
--------------------------------------------------------------------------------------------------*/
static bool processCommand(void)
{
    char *token = readToken();

    if(*token == '\0') {
        return true; /* Empty line */
    }
    if(!strcmp(token, "create")) {
        processCreateCommand();
    } else if(!strcmp(token, "compact")) {
        if(utPersistenceInitialized) {
            utCompactDatabase();
        } else {
            reportError("Compact is only for persitent databases.");
        }
    } else if(!strcmp(token, "destroy")) {
        processDestroyCommand();
    } else if(!strcmp(token, "help")) {
        printHelp();
        return true;
    } else if(!strcmp(token, "list")) {
        processListCommand();
    } else if(!strcmp(token, "quit")) {
        return false;
    } else if(!strcmp(token, "set")) {
        processSetCommand();
    } else if(!strcmp(token, "show")) {
        processShowCommand();
    } else if(!strcmp(token, "show_hidden")) {
        processShowHiddenCommand();
    } else if(!strcmp(token, "load_text")) {
        processLoadTextCommand();
    } else if(!strcmp(token, "save_text")) {
        processSaveTextCommand();
    } else if(!strcmp(token, "load_binary")) {
        processLoadBinaryCommand();
    } else if(!strcmp(token, "save_binary")) {
        processSaveBinaryCommand();
    } else {
        fprintf(utOutputFile, "Invalid command.  Type 'help' for a list of commands\n");
    }
    return true;
}

/*--------------------------------------------------------------------------------------------------
  Initialize the database manager.
--------------------------------------------------------------------------------------------------*/
void utDatabaseManagerStart(void)
{
    utLineSize = 42;
    utLineBuffer = utNewA(char, utLineSize);
    utOutputFile = stdout;
    utInputFile = stdin;
}

/*--------------------------------------------------------------------------------------------------
  Clean up after the database manager.
--------------------------------------------------------------------------------------------------*/
void utDatabaseManagerStop(void)
{
    utFree(utLineBuffer);
}

/*--------------------------------------------------------------------------------------------------
  Interpret commands to manage the database.
--------------------------------------------------------------------------------------------------*/
void utManager(void)
{
    utAtEndOfFile = false;
    fprintf(utOutputFile, "For help, enter the 'help' command\n");
    do {
        fprintf(utOutputFile, "> ");
        readLine();
    } while(processCommand());
}

/*--------------------------------------------------------------------------------------------------
  Skip blank lines.
--------------------------------------------------------------------------------------------------*/
static void skipBlankLines(void)
{
    while(readLine()) {
        if(utLineBuffer[0] != '\0') {
            return;
        }
    }
}

/*--------------------------------------------------------------------------------------------------
  Read a module header.
--------------------------------------------------------------------------------------------------*/
static utModule readModuleHeader(void)
{
    utModule module;
    char *token;

    token = readToken();
    if(strcmp(token, "module")) {
        if(!utAtEndOfFile) {
            reportError("Expected 'module' keyword");
        }
        return NULL;
    }
    token = readToken();
    if(token == NULL) {
        reportError("Expected module name");
        return NULL;
    }
    module = utFindModule(token);
    if(module == NULL) {
        reportError("Module %s not found", token);
        return NULL;
    }
    if(!expectEnd()) {
        return NULL;
    }
    return module;
}

/*--------------------------------------------------------------------------------------------------
  Reallocate object fields for the class.
--------------------------------------------------------------------------------------------------*/
static void reallocObjects(
    utClass theClass,
    uint64 numUsed)
{
    utField field;

    utSetInteger(utClassGetNumAllocatedPtr(theClass), numUsed, utClassGetReferenceSize(theClass));
    utForeachClassField(theClass, field) {
        if(!utFieldArray(field)) {
            *(uint8 **)(utFieldGetArrayPtr(field)) = utRealloc(*(uint8 **)utFieldGetArrayPtr(field), numUsed,
                utFieldGetSize(field));
        } else if(utFieldFixedSize(field)) {
            *(uint8 **)(utFieldGetArrayPtr(field)) = utRealloc(*(uint8 **)utFieldGetArrayPtr(field),
                numUsed*utFieldGetLength(field), utFieldGetSize(field));
        }
    } utEndClassField;
}

/*--------------------------------------------------------------------------------------------------
  Read a class header, and allocate space for the objects.
--------------------------------------------------------------------------------------------------*/
static utClass readClassHeader(
    utModule module)
{
    utClass theClass;
    char *token;
    uint64 numUsed;

    skipBlankLines();
    token = readToken();
    if(strcmp(token, "class")) {
        utLinePosition = 0;
        return NULL;
    }
    token = readToken();
    if(token == NULL) {
        reportError("Expected class name");
        return NULL;
    }
    theClass = findClass(module, token);
    if(theClass == NULL) {
        reportError("Class %s not found", token);
        return NULL;
    }
    token = readToken();
    if(!readInteger(utClassGetNumUsedPtr(theClass), token, utClassGetReferenceSize(theClass), true)) {
        reportError("Expected number of objects");
        return NULL;
    }
    if(!expectEnd()) {
        return NULL;
    }
    numUsed = utFindIntValue(utClassGetNumUsedPtr(theClass), utClassGetReferenceSize(theClass));
    numUsed = utMax(2, numUsed);
    reallocObjects(theClass, numUsed);
    return theClass;
}

/*--------------------------------------------------------------------------------------------------
  Read in column headers.  Build a translation table from column number to field.  The caller
  will have to free utFieldTranslationTable.
--------------------------------------------------------------------------------------------------*/
static bool readColumnHeaders(
    utClass theClass)
{
    utField field;
    char *valueList, *value;
    uint32 numFields, xField;

    if(!readLine()) {
        reportError("Expected column headers");
        return false;
    }
    valueList = utLineBuffer;
    numFields = countListValues(valueList) - 1;
    utFieldTranslationTable = calloc(numFields, sizeof(utField));
    value = parseArrayValue(&valueList);
    if(value == NULL || strcmp(value, "ObjectNumber")) {
        reportError("Expected 'objectNumber' to be the first field");
        return false;
    }
    for(xField = 0; xField < numFields; xField++) {
        value = parseArrayValue(&valueList);
        if(value == NULL) {
            return false;
        }
        field = utFindField(theClass, value);
        if(field == NULL) {
            reportError("Unknown field %s in class %s -- dropping", value, utClassGetName(theClass));
        }
        utFieldTranslationTable[xField] = field;
    }
    utNumFields = numFields;
    return true;
}

/*--------------------------------------------------------------------------------------------------
  Read in column types.  If it is incompatible with the field, print a warning, and null out the
  entry in the translation table.
--------------------------------------------------------------------------------------------------*/
static bool readColumnTypes(
    utClass theClass)
{
    char *valueList, *value;
    uint32 numFields, xField;

    if(!readLine()) {
        reportError("Expected column types");
        return false;
    }
    valueList = utLineBuffer;
    numFields = countListValues(valueList) - 1;
    if(numFields != utNumFields) {
        reportError("Column number mismatch");
        return false;
    }
    value = parseArrayValue(&valueList);
    if(value == NULL) {
        reportError("Expected class reference type to be the first field");
        return false;
    }
    for(xField = 0; xField < numFields; xField++) {
        value = parseArrayValue(&valueList);
        if(value == NULL) {
            return false;
        }
    }
    return true;
}

/*--------------------------------------------------------------------------------------------------
  Add the objects between the start and stop to the free list.
--------------------------------------------------------------------------------------------------*/
static void addObjectsToFreeList(
    utModule module,
    utClass theClass,
    uint32 start,
    uint32 stop)
{
    utField field = utClassGetNextFreeField(theClass);
    uint64 firstFree = utFindIntValue(utClassGetFirstFreePtr(theClass), utClassGetReferenceSize(theClass));
    uint64 xObject;
    uint8 *dest;

    for(xObject = start + 1; xObject < stop; xObject++) {
        firstFree = utFindIntValue(utClassGetFirstFreePtr(theClass), utClassGetReferenceSize(theClass));
        dest = *(uint8 **)(utFieldGetArrayPtr(field)) + xObject*utFieldGetSize(field);
        utSetInteger(dest, firstFree, utClassGetReferenceSize(theClass));
        utSetInteger(utClassGetFirstFreePtr(theClass), xObject, utClassGetReferenceSize(theClass));
    }
}

/*--------------------------------------------------------------------------------------------------
  Read in a table of objects.  A blank line ends the table.
--------------------------------------------------------------------------------------------------*/
static bool readClassTable(
    utModule module,
    utClass theClass)
{
    char *valueList;
    uint32 numValues;
    uint64 objectNumber, lastObjectNumber = 0;
    uint64 numUsed;
    bool error;

    utDo {
        if(!readLine()) {
            return false;
        }
        skipSpace();
    } utWhile(utLineBuffer[utLinePosition] != '\0') {
        objectNumber = parseObjectNumber(theClass, true, &error);
        if(error) {
            return false;
        }
        if(strcmp(readToken(), ",")) {
            reportError("Expecting a ','");
        }
        valueList = utLineBuffer + utLinePosition;
        numValues = countListValues(valueList);
        if(numValues != utNumFields) {
            reportError("Mismatched number of fields");
            return false;
        }
        if(!setObjectFields(theClass, objectNumber, valueList, utFieldTranslationTable)) {
            return false;
        }
        if(utClassGetDestructor(theClass) != NULL) {
            addObjectsToFreeList(module, theClass, lastObjectNumber, objectNumber);
        }
        lastObjectNumber = objectNumber;
    } utRepeat;
    if(utClassGetDestructor(theClass) != NULL) {
        numUsed = utFindIntValue(utClassGetNumUsedPtr(theClass), utClassGetReferenceSize(theClass));
        addObjectsToFreeList(module, theClass, lastObjectNumber, numUsed);
    }
    return true;
}

/*--------------------------------------------------------------------------------------------------
  Read in the module.
--------------------------------------------------------------------------------------------------*/
static bool readModule(
    utModule module)
{
    utClass theClass;
    bool passed;

    utDo {
        theClass = readClassHeader(module);
    } utWhile(theClass != NULL) {
        if(!readColumnHeaders(theClass)) {
            return false;
        }
        if(!readColumnTypes(theClass)) {
            return false;
        }
        passed = readClassTable(module, theClass);
        free(utFieldTranslationTable);
        if(!passed) {
            return false;
        }
    } utRepeat;
    return true;
}

/*--------------------------------------------------------------------------------------------------
  Read in the database in ASCII.
--------------------------------------------------------------------------------------------------*/
void utLoadTextDatabase(
    FILE *file)
{
    char *fileName = NULL;
    utModule module;
    FILE *savedFile = utInputFile;
    bool useDefault = file == NULL;

    if(useDefault) {
        fileName = utSprintf("%s%cdatabase", utDatabaseDirectory, UTDIRSEP);
        file = fopen(fileName, "r");
    }
    if(file == NULL) {
        utExit("Could not read from %s", fileName);
    }
    utResetDatabase();
    utAtEndOfFile = false;
    utLineNum = 0;
    utInputFile = file;
    skipBlankLines();
    do {
        module = readModuleHeader();
    } while(module != NULL && readModule(module));
    utInputFile = savedFile;
    if(useDefault) {
        fclose(file);
    }
}

/*--------------------------------------------------------------------------------------------------
  Write out the database in ASCII.  Don't write the utility module.
--------------------------------------------------------------------------------------------------*/
void utSaveTextDatabase(
    FILE *file)
{
    char *fileName = NULL;
    utModule module;
    utClass theClass;
    FILE *savedFile = utOutputFile;
    bool useDefault = file == NULL;

    if(useDefault) {
        fileName = utSprintf("%s%cdatabase", utDatabaseDirectory, UTDIRSEP);
        file = fopen(fileName, "w");
    }
    if(file == NULL) {
        utExit("Could not write to %s", fileName);
    }
    utOutputFile = file;
    utForeachModule(module) {
        if(strcmp(utModuleGetPrefix(module), "ut") && utModuleInitialized(module) &&
                (utModulePersistent(module) || !utPersistenceInitialized) &&
                utModuleSaved(module)) {
            fprintf(utOutputFile, "module %s\n\n", utModuleGetPrefix(module));
            utForeachModuleClass(module, theClass) {
                showClass(module, theClass);
                fprintf(utOutputFile, "\n");
            } utEndModuleClass;
        }
    } utEndModule;
    if(useDefault) {
        fclose(file);
    }
    utOutputFile = savedFile;
}

/*--------------------------------------------------------------------------------------------------
  Write out the fields of an object.
--------------------------------------------------------------------------------------------------*/
void utDatabaseShowObject(
    char *modulePrefix,
    char *className,
    uint64 objectNumber)
{
    utModule module = utFindModule(modulePrefix);
    utClass theClass;
    uint64 numUsed;
    FILE *savedFile = utOutputFile;

    if(module == NULL) {
        reportError("Invalid module %s\n", modulePrefix);
        return;
    }
    theClass = findClass(module, className);
    if(theClass == NULL) {
        reportError("Invalid class %s\n", className);
        return;
    }
    utOutputFile = stdout;
    showObject(theClass, objectNumber);
    utOutputFile = savedFile;
    numUsed = utFindIntValue(utClassGetNumUsedPtr(theClass), utClassGetReferenceSize(theClass));
    if(objectNumber >= numUsed) {
        reportError("Object number too large");
        return;
    }
}
