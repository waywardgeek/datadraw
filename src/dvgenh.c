/*
    DataDraw
    Copyright(C) 1992-2006 Bill Cox <bill@billrocks.org>

    This program can be distributed under the terms of the GNU Library GPL.
    See the file COPYING.LIB.
*/

#include "dv.h"

static char *dvPrefix;

/*--------------------------------------------------------------------------------------------------
  Write out constructor/destructor variable declarations.
--------------------------------------------------------------------------------------------------*/
static void writeClassConstructorDestructorVariables(
    dvClass theClass)
{
    char *name = dvClassGetName(theClass);

    dvWrtemp(dvFile,
        "typedef void (*%0%1CallbackType)(%2%1);\n"
        "extern %0%1CallbackType %0%1ConstructorCallback;\n",
        dvPrefix, name, dvClassGetPrefix(theClass));
    if(dvClassGetMemoryStyle(theClass) != MEM_CREATE_ONLY) {
        dvWrtemp(dvFile,
            "extern %0%1CallbackType %0%1DestructorCallback;\n",
            dvPrefix, name, dvClassGetPrefix(theClass));
    }
}

/*--------------------------------------------------------------------------------------------------
  Write out all the constructor/destructor variable declarations.
--------------------------------------------------------------------------------------------------*/
static void writeConstructorDestructorVariables(
    dvModule module)
{
    dvClass theClass;

    dvWrtemp(dvFile, "/* Constructor/Destructor hooks. */\n");
    dvForeachModuleClass(module, theClass) {
        writeClassConstructorDestructorVariables(theClass);
    } dvEndModuleClass;
    fputs("\n", dvFile);
}

/*--------------------------------------------------------------------------------------------------
  Write out the general/internal fields in root struct for theClass.
--------------------------------------------------------------------------------------------------*/
static void writeClassRootMemoryManagementFields(
    dvClass theClass)
{
    char *name = dvClassGetName(theClass);

    if(dvClassGetBaseClass(theClass) == dvClassNull) {
        if(dvClassGetMemoryStyle(theClass) == MEM_FREE_LIST) {
            dvWrtemp(dvFile,
                "    %0%1 firstFree%1;\n",
                dvPrefix, name);
        }
        dvWrtemp(dvFile,
            "    %1 used%0, allocated%0;\n",
            name, dvClassGetReferenceTypeName(theClass));
    } else {
        dvWrtemp(dvFile,
            "    %1 allocated%0;\n",
            name, dvClassGetReferenceTypeName(theClass));
    }
}

/*--------------------------------------------------------------------------------------------------
  Write out the property fields in root struct for theClass.
--------------------------------------------------------------------------------------------------*/
static void writeClassRootPropertyFields(
    dvClass theClass)
{
    char *name = dvClassGetName(theClass);
    dvProperty prop;

    dvForeachClassProperty(theClass, prop) {
        if(dvPropertyArray(prop) && !dvPropertyFixedSize(prop) && !dvPropertyView(prop)) {
            dvWrtemp(dvFile,
                "    uint32 used%0%1, allocated%0%1, free%0%1;\n",
                name, dvPropertyGetName(prop));
        }
    } dvEndClassProperty;
}

/*--------------------------------------------------------------------------------------------------
  Write root structure fields for theClass.
--------------------------------------------------------------------------------------------------*/
static void writeClassRootFields(
    dvClass theClass)
{
    writeClassRootMemoryManagementFields(theClass);
    writeClassRootPropertyFields(theClass);
}

/*--------------------------------------------------------------------------------------------------
  Write out the general/internal macros in root struct for theClass.
--------------------------------------------------------------------------------------------------*/
static void writeClassRootMemoryManagementMacros(
    dvClass theClass)
{
    char *name = dvClassGetName(theClass);
    char *preString, *postString;

    if(dvClassGetBaseClass(theClass) == dvClassNull) {
        if(dvClassGetMemoryStyle(theClass) == MEM_FREE_LIST) {
            dvWrtemp(dvFile,
                "utInlineC %0%1 %0FirstFree%1(void) {return %0RootData.firstFree%1;}\n",
                dvPrefix, name);
            preString = dvSwrtemp(!dvClassUndo(theClass)? "" :
                "\n    utRecordGlobal(%0ModuleID, %2, &%0RootData.firstFree%1, true);\n    ",
                dvPrefix, name, utSprintf("%u", dvClassGetReferenceSize(theClass) >> 3));
            postString = dvSwrtemp(!dvClassRedo(theClass)? "" :
                "\n    utRecordGlobal(%0ModuleID, %2, &%0RootData.firstFree%1, false);",
                dvPrefix, name, utSprintf("%u", dvClassGetReferenceSize(theClass) >> 3));
            dvWrtemp(dvFile,
                "utInlineC void %0SetFirstFree%1(%0%1 value) {%2%0RootData.firstFree%1 = (value);%3}\n",
                dvPrefix, name, preString, postString);
        }
        dvWrtemp(dvFile,
            "utInlineC %2 %0Used%1(void) {return %0RootData.used%1;}\n"
            "utInlineC %2 %0Allocated%1(void) {return %0RootData.allocated%1;}\n",
            dvPrefix, name, dvClassGetReferenceTypeName(theClass));
        preString = dvSwrtemp(!dvClassUndo(theClass)? "" :
            "\n    utRecordGlobal(%0ModuleID, %2, &%0RootData.used%1, true);\n    ",
            dvPrefix, name, utSprintf("%u", dvClassGetReferenceSize(theClass) >> 3));
        postString = dvSwrtemp(!dvClassRedo(theClass)? "" :
            "\n    utRecordGlobal(%0ModuleID, %2, &%0RootData.used%1, false);",
            dvPrefix, name, utSprintf("%u", dvClassGetReferenceSize(theClass) >> 3));
        dvWrtemp(dvFile,
            "utInlineC void %0SetUsed%1(%4 value) {%2%0RootData.used%1 = value;%3}\n",
            dvPrefix, name, preString, postString, dvClassGetReferenceTypeName(theClass));
        preString = dvSwrtemp(!dvClassUndo(theClass)? "" :
                "\n    utRecordGlobal(%0ModuleID, %2, &%0RootData.allocated%1, true);\n    ",
                dvPrefix, name, utSprintf("%u", dvClassGetReferenceSize(theClass) >> 3));
        postString = dvSwrtemp(!dvClassRedo(theClass)? "" :
                "\n    utRecordGlobal(%0ModuleID, %2, &%0RootData.allocated%1, false);",
                dvPrefix, name, utSprintf("%u", dvClassGetReferenceSize(theClass) >> 3));
        dvWrtemp(dvFile,
            "utInlineC void %0SetAllocated%1(%4 value) {%2%0RootData.allocated%1 = value;%3}\n",
            dvPrefix, name, preString, postString, dvClassGetReferenceTypeName(theClass));
    } else {
        dvWrtemp(dvFile,
            "utInlineC %2 %0Allocated%1(void) {return %0RootData.allocated%1;}\n",
            dvPrefix, name, dvClassGetReferenceTypeName(theClass));
        preString = dvSwrtemp(!dvClassUndo(theClass)? "" :
            "\n    utRecordGlobal(%0ModuleID, %2, &%0RootData.allocated%1, true);\n",
            dvPrefix, name, utSprintf("%u", dvClassGetReferenceSize(theClass) >> 3));
        postString = dvSwrtemp(!dvClassRedo(theClass)? "" :
            "\n    utRecordGlobal(%0ModuleID, %2, &%0RootData.allocated%1, false);",
            dvPrefix, name, utSprintf("%u", dvClassGetReferenceSize(theClass) >> 3));
        dvWrtemp(dvFile,
            "utInlineC void %0SetAllocated%1(%4 value) {%2%0RootData.allocated%1 = value;%3}\n",
            dvPrefix, name, preString, postString, dvClassGetReferenceTypeName(theClass));
    }
}

/*--------------------------------------------------------------------------------------------------
  Write out the property macros in root struct for theClass.
--------------------------------------------------------------------------------------------------*/
static void writeClassRootPropertyMacros(
    dvClass theClass)
{
    char *name = dvClassGetName(theClass);
    dvProperty prop;
    char *preString, *postString;

    dvForeachClassProperty(theClass, prop) {
        if(dvPropertyArray(prop) && !dvPropertyFixedSize(prop) && !dvPropertyView(prop)) {
            dvWrtemp(dvFile,
                "utInlineC uint32 %0Used%1%2(void) {return %0RootData.used%1%2;}\n"
                "utInlineC uint32 %0Allocated%1%2(void) {return %0RootData.allocated%1%2;}\n"
                "utInlineC uint32 %0Free%1%2(void) {return %0RootData.free%1%2;}\n",
                dvPrefix, name, dvPropertyGetName(prop));
            preString = dvSwrtemp(!dvClassUndo(theClass)? "" :
                "\n    utRecordGlobal(%0ModuleID, sizeof(uint32), &%0RootData.used%1%2, true);\n    ",
                dvPrefix, name, dvPropertyGetName(prop));
            postString = dvSwrtemp(!dvClassRedo(theClass)? "" :
                "\n    utRecordGlobal(%0ModuleID, sizeof(uint32), &%0RootData.used%1%2, false);",
                dvPrefix, name, dvPropertyGetName(prop));
            dvWrtemp(dvFile,
                "utInlineC void %0SetUsed%1%2(uint32 value) {%3%0RootData.used%1%2 = value;%4}\n",
                dvPrefix, name, dvPropertyGetName(prop), preString, postString);
            preString = dvSwrtemp(!dvClassUndo(theClass)? "" :
                "\n    utRecordGlobal(%0ModuleID, sizeof(uint32), &%0RootData.allocated%1%2, true);\n    ",
                dvPrefix, name, dvPropertyGetName(prop));
            postString = dvSwrtemp(!dvClassRedo(theClass)? "" :
                "\n    utRecordGlobal(%0ModuleID, sizeof(uint32), &%0RootData.allocated%1%2, false);",
                dvPrefix, name, dvPropertyGetName(prop));
            dvWrtemp(dvFile,
                "utInlineC void %0SetAllocated%1%2(uint32 value) {%3%0RootData.allocated%1%2 = value;%4}\n",
                dvPrefix, name, dvPropertyGetName(prop), preString, postString);
            preString = dvSwrtemp(!dvClassUndo(theClass)? "" :
                "\n    utRecordGlobal(%0ModuleID, sizeof(uint32), &%0RootData.free%1%2, true);\n    ",
                dvPrefix, name, dvPropertyGetName(prop));
            postString = dvSwrtemp(!dvClassRedo(theClass)? "" :
                "\n    utRecordGlobal(%0ModuleID, sizeof(uint32), &%0RootData.free%1%2, false);",
                dvPrefix, name, dvPropertyGetName(prop));
            dvWrtemp(dvFile,
                "utInlineC void %0SetFree%1%2(int32 value) {%3%0RootData.free%1%2 = value;%4}\n",
                dvPrefix, name, dvPropertyGetName(prop), preString, postString);
        }
    } dvEndClassProperty;
}

/*--------------------------------------------------------------------------------------------------
  Write out the macros for the gen/internal fields in root struct for theClass.
--------------------------------------------------------------------------------------------------*/
static void writeClassRootMacros(
    dvClass theClass)
{
    writeClassRootMemoryManagementMacros(theClass);
    writeClassRootPropertyMacros(theClass);
}

/*--------------------------------------------------------------------------------------------------
  Write the root structure and associated macros.
--------------------------------------------------------------------------------------------------*/
static void writeRoot(
    dvModule module)
{
    dvClass theClass;

    dvWrtemp(dvFile,
        "/*----------------------------------------------------------------------------------------\n"
        "  Root structure\n"
        "----------------------------------------------------------------------------------------*/\n"
        "struct %0RootType_ {\n"
        "    uint32 hash; /* This depends only on the structure of the database */\n",
        dvPrefix);
    dvForeachModuleClass(module, theClass) {
        writeClassRootFields(theClass);
    } dvEndModuleClass;
    dvWrtemp(dvFile, "};\nextern struct %0RootType_ %0RootData;\n\n", dvPrefix);
    dvWrtemp(dvFile,
        "utInlineC uint32 %0Hash(void) {return %0RootData.hash;}\n", dvPrefix);
    dvForeachModuleClass(module, theClass) {
        writeClassRootMacros(theClass);
    } dvEndModuleClass;
    fputs("\n", dvFile);
}

/*--------------------------------------------------------------------------------------------------
  Write out union types for the class.
--------------------------------------------------------------------------------------------------*/
static void writeUnionTypes(
    dvClass theClass)
{
    dvUnion theUnion;
    dvProperty property;

    if(dvClassGetFirstUnion(theClass) == dvUnionNull) {
        return;
    }
    dvWrtemp(dvFile,
        "/*----------------------------------------------------------------------------------------\n"
        "  Unions for class %0.\n"
        "----------------------------------------------------------------------------------------*/\n"
        , dvClassGetName(theClass));
    dvForeachClassUnion(theClass, theUnion) {
        dvWrtemp(dvFile, "typedef union {\n");
        dvForeachUnionProperty(theUnion, property) {
            dvWrtemp(dvFile,
                "    %0 %1;\n",
                dvPropertyGetTypeName(property), dvPropertyGetName(property));
        } dvEndUnionProperty;
        dvWrtemp(dvFile,
            "} %0;\n\n",
            dvUnionGetTypeName(theUnion));
    } dvEndClassUnion;
}

/*--------------------------------------------------------------------------------------------------
  Write out cashe types for the class.
--------------------------------------------------------------------------------------------------*/
static void writeCacheTypes(
    dvClass theClass)
{
    dvCache cache;
    dvProperty property;

    if(dvClassGetFirstCache(theClass) == dvCacheNull) {
        return;
    }
    dvWrtemp(dvFile,
        "/*----------------------------------------------------------------------------------------\n"
        "  Caches for class %0.\n"
        "----------------------------------------------------------------------------------------*/\n"
        , dvClassGetName(theClass));
    dvForeachClassCache(theClass, cache) {
        dvWrtemp(dvFile, "%0 {\n", dvCacheGetTypeName(cache));
        dvForeachCacheProperty(cache, property) {
            dvWrtemp(dvFile,
                "    %0 %1;\n",
                dvPropertyGetTypeName(property), dvPropertyGetName(property));
        } dvEndCacheProperty;
        dvWrtemp(dvFile, "};\n\n");
    } dvEndClassCache;
}

/*--------------------------------------------------------------------------------------------------
  Write out the property fields for theClass.
--------------------------------------------------------------------------------------------------*/
static void writeClassFields(
    dvClass theClass)
{
    dvUnion theUnion;
    dvCache cache;
    dvProperty prop;

    dvForeachClassProperty(theClass, prop) {
        if(!dvPropertySparse(prop) &&
            dvPropertyGetUnion(prop) == dvUnionNull &&
            dvPropertyGetCache(prop) == dvCacheNull &&
           !dvPropertyView(prop)) {
            dvWrtemp(dvFile, "    %0 *%1;\n", dvPropertyGetTypeName(prop), dvPropertyGetName(prop));
        }
    } dvEndClassProperty;
    dvForeachClassUnion(theClass, theUnion) {
        dvWrtemp(dvFile,
            "    %0 *%1;\n",
            dvUnionGetTypeName(theUnion), dvUnionGetFieldName(theUnion));
    } dvEndClassUnion;
    dvForeachClassCache(theClass, cache) {
        dvWrtemp(dvFile,
            "    %0 *%1;\n",
            dvCacheGetTypeName(cache), dvCacheGetFieldName(cache));
    } dvEndClassCache;
}

/*--------------------------------------------------------------------------------------------------
  Write out the structure for a class.
--------------------------------------------------------------------------------------------------*/
static void writeClassStruct(
    dvClass theClass)
{
    char *name = dvClassGetName(theClass);

    dvWrtemp(dvFile,
        "/*----------------------------------------------------------------------------------------\n"
        "  Fields for class %1.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "struct %0%1Fields {\n", dvPrefix, name);
    writeClassFields(theClass);
    dvWrtemp(dvFile,
        "};\n"
        "extern struct %0%1Fields %0%1s;\n"
        "\n", dvPrefix, name);
}

/*--------------------------------------------------------------------------------------------------
  Write the header's bottom portion.
--------------------------------------------------------------------------------------------------*/
static void writeHeaderBot(
    dvModule module)
{

    dvWrtemp(dvFile,
        "void %0DatabaseStart(void);\n"
        "void %0DatabaseStop(void);\n"
        "#if defined __cplusplus\n"
        "}\n"
        "#endif\n\n"
        "#endif\n",
        dvPrefix);
}

/*--------------------------------------------------------------------------------------------------
    class has bit fields
--------------------------------------------------------------------------------------------------*/
static bool classHasBitfields(
    dvClass theClass)
{
    dvProperty prop;
    dvForeachClassProperty(theClass, prop) {
        if(dvPropertyGetType(prop) == PROP_BIT) {
            return true;
        }
    } dvEndClassProperty;
    return false;
}

/*--------------------------------------------------------------------------------------------------
  Write object's create function declaration.
--------------------------------------------------------------------------------------------------*/
static void writeClassCreateExternFunc(
    dvClass theClass)
{
    char *name = dvClassGetName(theClass);

    if(dvClassGetBaseClass(theClass) == dvClassNull) {
        dvWrtemp(dvFile, "void %0%1AllocMore(void);\n", dvPrefix, name);
        dvWrtemp(dvFile, "void %0%1CopyProps(%0%1 %0Old%1, %0%1 %0New%1);\n", dvPrefix, name);
        if(classHasBitfields(theClass)) {
            dvWrtemp(dvFile, "void %0%1SetBitfield(%0%1 _%1, uint32 bitfield);\n", dvPrefix, name);
            dvWrtemp(dvFile, "uint32 %0%1GetBitfield(%0%1 _%1);\n", dvPrefix, name);
        }
    }
}

/*--------------------------------------------------------------------------------------------------
  Write the object's function declarations for property allocs.
--------------------------------------------------------------------------------------------------*/
static void writeClassPropExternFuncs(
    dvClass theClass)
{
    char *name = dvClassGetName(theClass);
    dvProperty prop;

    dvForeachClassProperty(theClass, prop) {
        if(dvPropertyArray(prop) && !dvPropertyFixedSize(prop) &&!dvPropertyView(prop)) {
            dvWrtemp(dvFile,
                "void %0%1Alloc%2s(%3%1 %1, uint32 num%2s);\n"
                "void %0%1Resize%2s(%3%1 %1, uint32 num%2s);\n"
                "void %0%1Free%2s(%3%1 %1);\n"
                "void %0Compact%1%2s(void);\n",
                dvPrefix, name, dvPropertyGetName(prop), dvClassGetPrefix(theClass));
        }
    } dvEndClassProperty;
}

/*--------------------------------------------------------------------------------------------------
  Write the comparison of two values.
--------------------------------------------------------------------------------------------------*/
static void writeComparison(
    char *compareString,
    bool lastValue)
{
    if(!lastValue) {
        dvWrtemp(dvFile, "(%0 != 0? %0 :\n", compareString);
    } else {
        dvWrtemp(dvFile, "%0", compareString);
    }
}

/*--------------------------------------------------------------------------------------------------
  Write the compare function for the relationship.
--------------------------------------------------------------------------------------------------*/
static void writeCompareFunction(
    dvRelationship relationship)
{
    dvClass parent = dvRelationshipGetParentClass(relationship);
    dvClass child = dvRelationshipGetChildClass(relationship);
    dvKey key = dvRelationshipGetFirstKey(relationship);
    dvProperty property = dvKeypropertyGetProperty(dvKeyGetLastKeyproperty(key));
    char *compareString;
    uint32 numKeys = 0;

    dvWrtemp(dvFile,
        "utInlineC int %0%1Compare%2%3(%5%3 left, %5%3 right) {return ",
        dvPrefix, dvClassGetName(parent), dvRelationshipGetChildLabel(relationship), 
             dvClassGetName(child), dvClassGetPrefix(parent), dvClassGetPrefix(child), 
             dvRelationshipGetParentLabel(relationship));

    dvForeachRelationshipKey(relationship, key) {
        numKeys++;
        property = dvKeypropertyGetProperty(dvKeyGetLastKeyproperty(key));
        if(!dvPropertyArray(property)) {
            compareString = dvSwrtemp("%0 > %1? 1 : (%0 < %1? -1 : 0)",
                dvKeyGetAccessMacro(key, false, "right"), dvKeyGetAccessMacro(key, false, "left"));
            writeComparison(compareString, dvKeyGetNextRelationshipKey(key) == dvKeyNull);
        } else if(!dvPropertyFixedSize(property)) {
            compareString = dvSwrtemp("%0 > %1? 1 : (%0 < %1? -1 : 0)",
                dvKeyGetLengthMacro(key, false, "right"), dvKeyGetLengthMacro(key, false, "left"));
            writeComparison(compareString, false);
            compareString = dvSwrtemp("memcmp(%0, %1, sizeof(%2)*%3)",
                dvKeyGetAccessMacro(key, false, "left"),
                dvKeyGetAccessMacro(key, false, "right"),
                dvPropertyGetTypeName(property),
                dvKeyGetLengthMacro(key, false, "left"));
            writeComparison(compareString, dvKeyGetNextRelationshipKey(key) == dvKeyNull);
        } else {
            compareString = dvSwrtemp("memcmp(%0, %1, sizeof(%2)*(%3))",
                dvKeyGetAccessMacro(key, false, "left"),
                dvKeyGetAccessMacro(key, false, "right"),
                dvPropertyGetTypeName(property),
                dvPropertyGetIndex(property));
            writeComparison(compareString, dvKeyGetNextRelationshipKey(key) == dvKeyNull);
        }
    } dvEndRelationshipKey;
    while(--numKeys) {
        dvWrtemp(dvFile, ")");
    }
    dvWrtemp(dvFile, ";}\n");
}

/*--------------------------------------------------------------------------------------------------
  Write the object's extern function declarations for relationships.
--------------------------------------------------------------------------------------------------*/
static void writeClassRelExternFuncs(
    dvClass theClass)
{
    dvRelationship relationship;
    dvClass childClass;
    dvRelationshipType type;
    char *name = dvClassGetName(theClass);
    char *childLabel, *childName;
    char *parentLabel;

    dvForeachClassChildRelationship(theClass, relationship) {
        type = dvRelationshipGetType(relationship);
        childClass = dvRelationshipGetChildClass(relationship);
        childLabel = dvRelationshipGetChildLabel(relationship);
        parentLabel = dvRelationshipGetParentLabel(relationship);
        childName = dvClassGetName(childClass);
        if(dvRelationshipAccessChild(relationship)) {
            if(type == REL_POINTER && dvRelationshipAccessParent(relationship)) {
                dvWrtemp(dvFile,
                    "utInlineC void %0%1Insert%2%3(%4%1 %1, %5%3 _%3) {%0%1Set%2%3(%1, _%3); %0%3Set%6%1(_%3, %1);}\n"
                    "utInlineC void %0%1Remove%2%3(%4%1 %1, %5%3 _%3) {%0%1Set%2%3(%1, %5%3Null); "
                    "%0%3Set%6%1(_%3, %4%1Null);}\n",
                    dvPrefix, name, childLabel, childName,
                    dvClassGetPrefix(theClass), dvClassGetPrefix(childClass), parentLabel);
            } else if(type == REL_LINKED_LIST || type == REL_DOUBLY_LINKED || type == REL_TAIL_LINKED ||
                    (type == REL_HASHED && !dvRelationshipUnordered(relationship))) {
                dvWrtemp(dvFile,
                    "void %0%1Insert%2%3(%4%1 %1, %5%3 _%3);\n"
                    "void %0%1Remove%2%3(%4%1 %1, %5%3 _%3);\n"
                    "void %0%1InsertAfter%2%3(%4%1 %1, %5%3 prev%3, %5%3 _%3);\n",
                    dvPrefix, name, childLabel, childName,
                    dvClassGetPrefix(theClass), dvClassGetPrefix(childClass));
                if(type != REL_LINKED_LIST) {
                    dvWrtemp(dvFile,
                        "void %0%1Append%2%3(%4%1 %1, %5%3 _%3);\n",
                        dvPrefix, name, childLabel, childName,
                        dvClassGetPrefix(theClass), dvClassGetPrefix(childClass));
                }
            } else if((type == REL_HASHED && dvRelationshipUnordered(relationship)) || type == REL_ORDERED_LIST) {
                dvWrtemp(dvFile,
                    "void %0%1Insert%2%3(%4%1 %1, %5%3 _%3);\n"
                    "void %0%1Remove%2%3(%4%1 %1, %5%3 _%3);\n",
                    dvPrefix, name, childLabel, childName,
                    dvClassGetPrefix(theClass), dvClassGetPrefix(childClass));
                if(type == REL_ORDERED_LIST) {
                    if(dvRelationshipAccessParent(relationship)) {
                        dvWrtemp(dvFile,
                            "%5%3 %0%1Exchange%1%2%3(%4%1 %1, %5%3 _%3); /* alpha release */\n",
                            dvPrefix, name, childLabel, childName,
                            dvClassGetPrefix(theClass), dvClassGetPrefix(childClass));
                    }
                    dvWrtemp(dvFile,
                        "%5%3 %0%1GetFirst%2%3(%4%1 %1);\n"
                        "%5%3 %0%1GetLast%2%3(%4%1 %1);\n"
                        "%5%3 %0%3GetNext%1%2%3(%5%3 %3);\n"
                        "%5%3 %0%3GetPrev%1%2%3(%5%3 %3);\n",
                        dvPrefix, name, childLabel, childName,
                        dvClassGetPrefix(theClass), dvClassGetPrefix(childClass));
                    if(dvRelationshipGetFirstKey(relationship) == dvKeyNull) {
                        dvWrtemp(dvFile,
                            "#ifndef %0%1Compare%2%3\n"
                            "int %0%1Compare%2%3(%5%3 a, %5%3 b);\n"
                            "#endif\n",
                            dvPrefix, name, childLabel, dvClassGetName(childClass),
                            dvClassGetPrefix(theClass), dvClassGetPrefix(childClass));
                    } else {
                        writeCompareFunction(relationship);
                    }
                }
            } else if(type == REL_ARRAY || type == REL_HEAP) {
                if(type == REL_ARRAY) {
                    dvWrtemp(dvFile,
                        "void %0%1Insert%2%3(%4%1 %1, uint32 x, %5%3 _%3);\n"
                        "void %0%1Append%2%3(%4%1 %1, %5%3 _%3);\n",
                        dvPrefix, name, childLabel, dvClassGetName(childClass),
                        dvClassGetPrefix(theClass), dvClassGetPrefix(childClass));
                }
                if(dvRelationshipAccessParent(relationship)) {
                    dvWrtemp(dvFile,
                        "void %0%1Remove%2%3(%4%1 %1, %5%3 _%3);\n",
                        dvPrefix, name, childLabel, dvClassGetName(childClass),
                        dvClassGetPrefix(theClass), dvClassGetPrefix(childClass));
                }
                if(type == REL_HEAP) {
                    dvWrtemp(dvFile,
                        "void %0%1Update%2%3(%4%1 %1, %5%3 _%3);\n"
                        "%5%3 %0%1Peek%2%3(%4%1 %1);\n"
                        "void %0%1Push%2%3(%4%1 %1, %5%3 _%3);\n"
                        "%5%3 %0%1Pop%2%3(%4%1 %1);\n",
                        dvPrefix, name, childLabel, dvClassGetName(childClass),
                        dvClassGetPrefix(theClass), dvClassGetPrefix(childClass));
                    if(dvRelationshipGetFirstKey(relationship) == dvKeyNull) {
                        dvWrtemp(dvFile,
                            "#ifndef %0%1Compare%2%3\n"
                            "int %0%1Compare%2%3(%5%3 a, %5%3 b);\n"
                            "#endif\n",
                            dvPrefix, name, childLabel, dvClassGetName(childClass),
                            dvClassGetPrefix(theClass), dvClassGetPrefix(childClass));
                    } else {
                        writeCompareFunction(relationship);
                    }
                }

            }
        }
    } dvEndClassChildRelationship;
}

/*--------------------------------------------------------------------------------------------------
  Write the external function declarations for an object.
--------------------------------------------------------------------------------------------------*/
static void writeClassExternFuncs(
    dvClass theClass)
{
    writeClassCreateExternFunc(theClass);
    writeClassPropExternFuncs(theClass);
}

/*--------------------------------------------------------------------------------------------------
  Write the class's property access macros.
--------------------------------------------------------------------------------------------------*/
static void writeClassPropMacros(
    dvClass theClass)
{
    dvUnion theUnion;
    dvCache cache;
    dvProperty prop;
    char *name;
    char *propName, *preString, *postString;
    dvPropertyType type;
    char *accessString;
    char *propTypeString;

    dvForeachClassProperty(theClass, prop) {
        name = dvClassGetName(theClass);
        propName = dvPropertyGetName(prop);
        type = dvPropertyGetType(prop);
        propTypeString = dvPropertyGetTypeName(prop);
        if(type == PROP_BOOL || type == PROP_BIT) {
            accessString = "";
        } else {
            accessString = "Get";
        }
        if(dvPropertyView(prop) && dvPropertyArray(prop) && !dvPropertyFixedSize(prop)) {
            /* view on variable size array */
            dvWrtemp(dvFile,
                "#ifndef %0%1Geti%2\n"
                "extern %5 %0%1Geti%2(%4%1 %1, uint32 x);\n"
                "#endif\n"
                "#ifndef %0%1Get%2\n"
                "extern %5 *%0%1Get%2(%4%1 %1);\n"
                "#endif\n"
                "#ifndef %0%1Get%2s\n"
                "#define %0%1Get%2s %0%1Get%2\n"
                "#endif\n",
                dvPrefix, name, propName, dvClassGetReferenceTypeName(theClass),
                dvClassGetPrefix(theClass), propTypeString);
            dvWrtemp(dvFile,
                "#ifndef %0%1Set%2\n"
                "extern void %0%1Set%2(%4%1 %1, %3 *valuePtr, uint32 num%2);\n"
                "#endif\n",
                dvPrefix, name, propName, propTypeString, dvClassGetPrefix(theClass));
            dvWrtemp(dvFile,
                "#ifndef %0%1Seti%2\n"
                "extern void %0%1Seti%2(%3%1 %1, uint32 x, %4 value);\n"
                "#endif\n",
                dvPrefix, name, propName, dvClassGetPrefix(theClass), propTypeString);
        } else if(dvPropertyArray(prop) && !dvPropertyFixedSize(prop)) {
            /* variable size array */
            dvWrtemp(dvFile,
                "#if defined(DD_DEBUG)\n"
                "utInlineC uint32 %0%1Check%2Index(%4%1 %1, uint32 x) {utAssert(x < %0%1GetNum%2(%1)); return x;}\n"
                "#else\n"
                "utInlineC uint32 %0%1Check%2Index(%4%1 %1, uint32 x) { (void)%1; return x;}\n"
                "#endif\n"
                "utInlineC %5 %0%1Geti%2(%4%1 %1, uint32 x) {return %0%1s.%2[\n"
                "    %0%1Get%2Index_(%1) + %0%1Check%2Index(%1, x)];}\n"
                "utInlineC %5 *%0%1Get%2(%4%1 %1) {return %0%1s.%2 + %0%1Get%2Index_(%1);}\n"
                "#define %0%1Get%2s %0%1Get%2\n",
                dvPrefix, name, propName, dvClassGetReferenceTypeName(theClass),
                dvClassGetPrefix(theClass), propTypeString);
            preString = dvSwrtemp(!dvClassUndo(theClass)? "" :
                "\n    utRecordArray(%0ModuleID, %3, %0%1Get%2Index_(%1), %0%1GetNum%2(%1), true);\n    ",
                dvPrefix, name, propName, utSprintf("%u", dvPropertyGetFieldNumber(prop)));
            postString = dvSwrtemp(!dvClassRedo(theClass)? "" :
                "\n    utRecordArray(%0ModuleID, %3, %0%1Get%2Index_(%1), %0%1GetNum%2(%1), false);\n",
                dvPrefix, name, propName, utSprintf("%u", dvPropertyGetFieldNumber(prop)));
            dvWrtemp(dvFile,
                "utInlineC void %0%1Set%2(%6%1 %1, %3 *valuePtr, uint32 num%2) {\n"
                "    %0%1Resize%2s(%1, num%2);\n"
                "%4    memcpy(%0%1Get%2s(%1), valuePtr, num%2*sizeof(%3));%5}\n",
                dvPrefix, name, propName, propTypeString, preString, postString, dvClassGetPrefix(theClass));
            preString = dvSwrtemp(!dvClassUndo(theClass)? "" :
                "\n    utRecordField(%0ModuleID, %3, %0%1Get%2Index_(%1) + (x), true);\n",
                dvPrefix, name, propName, utSprintf("%u", dvPropertyGetFieldNumber(prop)));
            postString = dvSwrtemp(!dvClassRedo(theClass)? "" :
                "\n    utRecordField(%0ModuleID, %3, %0%1Get%2Index_(%1) + (x), false);",
                dvPrefix, name, propName, utSprintf("%u", dvPropertyGetFieldNumber(prop)));
            dvWrtemp(dvFile,
                    "utInlineC void %0%1Seti%2(%5%1 %1, uint32 x, %6 value) {\n"
                    "%3    %0%1s.%2[%0%1Get%2Index_(%1) + %0%1Check%2Index(%1, (x))] = value;%4}\n",
                dvPrefix, name, propName, preString, postString, dvClassGetPrefix(theClass), propTypeString);
            if(dvPropertyGetRelationship(prop) == dvRelationshipNull) {
                dvWrtemp(dvFile,
                    "utInlineC void %0%1Move%2s(%4%1 %1, uint32 from, uint32 to, uint32 count) {\n"
                    "    utAssert((to+count) <= %0%1GetNum%2(%1));\n"
                    "    utAssert((from+count) <= %0%1GetNum%2(%1));\n"
                    "    memmove(%0%1Get%2s(%1)+to,%0%1Get%2s(%1)+from,((int32)count)*sizeof(%5));\n"
                    "}\n"
                    "utInlineC void %0%1Copy%2s(%4%1 %1, uint32 x, %5 * values, uint32 count) {\n"
                    "    utAssert((x+count) <= %0%1GetNum%2(%1));\n"
                    "    memcpy(%0%1Get%2s(%1)+x, values, count*sizeof(%5));\n"
                    "}\n"
                    "utInlineC void %0%1Append%2s(%4%1 %1, %5 * values, uint32 count) {\n"
                    "    uint32 num = %0%1GetNum%2(%1);\n"
                    "    %0%1Resize%2s(%1, num+count);\n"
                    "    %0%1Copy%2s(%1, num, values, count);\n"
                    "}\n"
                    "utInlineC void %0%1Append%2(%4%1 %1, %5 %2) {\n"
                    "    %0%1Resize%2s(%1, %0%1GetNum%2(%1)+1);\n"
                    "    %0%1Seti%2(%1, %0%1GetNum%2(%1)-1, %2);\n"
                    "}\n"
                    "utInlineC void %0%1Insert%2s(%4%1 %1, uint32 x, %5 *%2, uint32 count) {\n"
                    "    utAssert(x <= %0%1GetNum%2(%1));\n"
                    "    if(x < %0%1GetNum%2(%1)) {\n" 
                    "        %0%1Resize%2s(%1, %0%1GetNum%2(%1)+count);\n"
                    "        %0%1Move%2s(%1, x, x+count, %0%1GetNum%2(%1)-x-count);\n"
                    "        %0%1Copy%2s(%1, x, %2, count);\n"
                    "    }\n"
                    "    else {\n"
                    "        %0%1Append%2s(%1, %2, count);\n"
                    "    }\n"
                    "}\n"
                    "utInlineC void %0%1Insert%2(%4%1 %1, uint32 x, %5 %2) {\n"
                    "    %0%1Insert%2s(%1, x, &%2, 1);\n"
                    "}\n"
                    "utInlineC void %0%1Remove%2s(%4%1 %1, uint32 x, uint32 count) {\n"
                    "    utAssert((x+count) <= %0%1GetNum%2(%1));\n"
                    "    if((x+count) < %0%1GetNum%2(%1)) {\n"
                    "        %0%1Move%2s(%1, x+count,x,%0%1GetNum%2(%1)-x-count);\n"
                    "    }\n"
                    "    %0%1Resize%2s(%1, %0%1GetNum%2(%1)-(int32)count);\n"
                    "}\n"
                    "utInlineC void %0%1Remove%2(%4%1 %1, uint32 x) {\n"
                    "    %0%1Remove%2s(%1, x, 1);\n"
                    "}\n"
                    "utInlineC void %0%1Swap%2(%4%1 %1, uint32 from, uint32 to) {\n"
                    "    utAssert(from <= %0%1GetNum%2(%1));\n"
                    "    utAssert(to <= %0%1GetNum%2(%1));\n"
                    "    %5 tmp = %0%1Geti%2(%1, from);\n"
                    "    %0%1Seti%2(%1, from, %0%1Geti%2(%1, to));\n"
                    "    %0%1Seti%2(%1, to, tmp);\n"
                    "}\n"
                    "utInlineC void %0%1Swap%2s(%4%1 %1, uint32 from, uint32 to, uint32 count) {\n"
                    "    utAssert((from+count) < %0%1GetNum%2(%1));\n"
                    "    utAssert((to+count) < %0%1GetNum%2(%1));\n"
                    "    %5 tmp[count];\n"
                    "    memcpy(tmp, %0%1Get%2s(%1)+from, count*sizeof(%5));\n"
                    "    memcpy(%0%1Get%2s(%1)+from, %0%1Get%2s(%1)+to, count*sizeof(%5));\n"
                    "    memcpy(%0%1Get%2s(%1)+to, tmp, count*sizeof(%5));\n"
                    "}\n",
                    dvPrefix, name, propName, dvClassGetReferenceTypeName(theClass),
                    dvClassGetPrefix(theClass), propTypeString);
                dvWrtemp(dvFile,
                    "#define %0Foreach%1%2(pVar, cVar) { \\\n"
                    "    uint32 _x%2; \\\n"
                    "    for(_x%2 = 0; _x%2 < %0%1GetNum%2(pVar); _x%2++) { \\\n"
                    "        cVar = %0%1Geti%2(pVar, _x%2);\n"
                    "#define %0End%1%2 }}\n",
                    dvPrefix, name, propName, propTypeString);
            }
            else { /* so dvPropertyGetRelationship(prop) != dvRelationshipNull */
                /* TODO if pertinent */
            }

        } else if(dvPropertyArray(prop) && dvPropertyView(prop)) {
            /* view on fixed sized arrays */
            dvWrtemp(dvFile,
                "#ifndef %0%1Geti%2\n"
                "extern %6 %0%1Geti%2(%5%1 %1, uint32 x);\n"
                "#endif\n"
                "#ifndef %0%1Get%2\n"
                "extern %6 *%0%1Get%2(%5%1 %1);\n"
                "#endif\n"
                "#ifndef %0%1Get%2s\n"
                "#define %0%1Get%2s %0%1Get%2\n"
                "#endif\n",
                dvPrefix, name, propName, dvClassGetReferenceTypeName(theClass), dvPropertyGetIndex(prop),
                dvClassGetPrefix(theClass), propTypeString);
            dvWrtemp(dvFile,
                "#ifndef %0%1Set%2\n"
                "extern void %0%1Set%2(%4%1 %1, %3 *valuePtr, uint32 num%2);\n"
                "#endif\n",
                dvPrefix, name, propName, propTypeString, dvClassGetPrefix(theClass));
            dvWrtemp(dvFile,
                "#ifndef %0%1Seti%2\n"
                "extern void %0%1Seti%2(%4%1 %1, uint32 x, %5 value);\n"
                "#endif\n",
                dvPrefix, name, propName, dvPropertyGetIndex(prop), dvClassGetPrefix(theClass), propTypeString);
        } else if(dvPropertyArray(prop)) {
            /* Fixed sized arrays */
            dvWrtemp(dvFile,
                "#if defined(DD_DEBUG)\n"
                "utInlineC uint32 %0%1Check%2Index(%5%1 %1, uint32 x) {\n"
                "    utAssert(x < (%4));\n"
                "    return x;}\n"
                "#else\n"
                "utInlineC uint32 %0%1Check%2Index(%5%1 %1, uint32 x) {return x;}\n"
                "#endif\n"
                "utInlineC %6 %0%1Geti%2(%5%1 %1, uint32 x) {\n"
                "    return %0%1s.%2[%0%12Index(%1)*(%4) + %0%1Check%2Index(%1, x)];}\n"
                "utInlineC %6 *%0%1Get%2(%5%1 %1) {return %0%1s.%2 + %0%12Index(%1)*(%4);}\n"
                "#define %0%1Get%2s %0%1Get%2\n",
                dvPrefix, name, propName, dvClassGetReferenceTypeName(theClass), dvPropertyGetIndex(prop),
                dvClassGetPrefix(theClass), propTypeString);
            preString = dvSwrtemp(!dvClassUndo(theClass)? "" :
                "    utRecordArray(%0ModuleID, %3, %0%12Index(%1)*(%4), (%4), true);\n",
                dvPrefix, name, propName, utSprintf("%u", dvPropertyGetFieldNumber(prop)), dvPropertyGetIndex(prop));
            postString = dvSwrtemp(!dvClassRedo(theClass)? "" :
                "\n    utRecordArray(%0ModuleID, %3, %0%12Index(%1)*(%4), (%4), false);",
                dvPrefix, name, propName, utSprintf("%u", dvPropertyGetFieldNumber(prop)), dvPropertyGetIndex(prop));
            dvWrtemp(dvFile,
                "utInlineC void %0%1Set%2(%6%1 %1, %3 *valuePtr, uint32 num%2) {\n"
                "%4    memcpy(%0%1Get%2s(%1), valuePtr, num%2*sizeof(%3));%5}\n",
                dvPrefix, name, propName, propTypeString, preString, postString, dvClassGetPrefix(theClass));
            preString = dvSwrtemp(!dvClassUndo(theClass)? "" :
                "\n    utRecordField(%0ModuleID, %3, %0%12Index(%1)*(%4) + (x), true);\n    ",
                dvPrefix, name, propName, utSprintf("%u", dvPropertyGetFieldNumber(prop)), dvPropertyGetIndex(prop));
            postString = dvSwrtemp(!dvClassRedo(theClass)? "" :
                "\n    utRecordField(%0ModuleID, %3, %0%12Index(%1)*(%4) + (x), false);",
                dvPrefix, name, propName, utSprintf("%u", dvPropertyGetFieldNumber(prop)), dvPropertyGetIndex(prop));
            dvWrtemp(dvFile,
                    "utInlineC void %0%1Seti%2(%6%1 %1, uint32 x, %7 value) {\n"
                    "%4    %0%1s.%2[%0%12Index(%1)*(%3) + %0%1Check%2Index(%1, x)] = value;%5}\n",
                dvPrefix, name, propName, dvPropertyGetIndex(prop), preString, postString, dvClassGetPrefix(theClass),
                propTypeString);
        } else if(dvPropertySparse(prop) && !dvPropertyView(prop)) {
            /* Access the data through the hash table */
            dvWrtemp(dvFile,
                "extern %5 %0%1%3%2(%4%1 %1);\n"
                "extern void %0%1Set%2(%4%1 %1, %5 value);\n",
                dvPrefix, name, propName, accessString,
                dvClassGetPrefix(theClass), propTypeString);
        } else {
            theUnion = dvPropertyGetUnion(prop);
            cache = dvPropertyGetCache(prop);
            if(theUnion == dvUnionNull && cache == dvCacheNull) {
                preString = dvSwrtemp(!dvClassUndo(theClass)? "" :
                    "\n    utRecordField(%0ModuleID, %3, %2%12ValidIndex(%1)%4, true);\n    ",
                    dvPrefix, name, dvClassGetPrefix(theClass),
                    utSprintf("%u", dvPropertyGetFieldNumber(prop)),
                    dvPropertyGetType(prop) == PROP_BIT? " >> 3" : "");
                postString = dvSwrtemp(!dvClassRedo(theClass)? "" :
                    "\n    utRecordField(%0ModuleID, %3, %2%12ValidIndex(%1)%4, false);",
                    dvPrefix, name, dvClassGetPrefix(theClass),
                    utSprintf("%u", dvPropertyGetFieldNumber(prop)),
                    dvPropertyGetType(prop) == PROP_BIT? " >> 3" : "");
            } else if(theUnion != dvUnionNull) {
                preString = dvSwrtemp(!dvClassUndo(theClass)? "" :
                    "\n    utRecordField(%0ModuleID, %3, %2%12ValidIndex(%1), true);\n    ",
                    dvPrefix, name, dvClassGetPrefix(theClass),
                    utSprintf("%u", dvUnionGetFieldNumber(theUnion)));
                postString = dvSwrtemp(!dvClassRedo(theClass)? "" :
                    "\n    utRecordField(%0ModuleID, %3, %2%12ValidIndex(%1), false);",
                    dvPrefix, name, dvClassGetPrefix(theClass),
                    utSprintf("%u", dvUnionGetFieldNumber(theUnion)));
            } else {
                preString = "";
                postString = "";
            }
            if(dvPropertyView(prop)) {
                dvWrtemp(dvFile,
                    "#ifndef %0%1%5%2\n"
                    "extern %4 %0%1%5%2(%3%1 %1);\n"
                    "#endif\n"
                    "#ifndef %0%1Set%2\n"
                    "extern void %0%1Set%2(%3%1 %1, %4 value);\n"
                    "#endif\n",
                    dvPrefix, name, propName, dvClassGetPrefix(theClass), propTypeString, accessString);
            }
            else if(dvPropertyGetType(prop) == PROP_BIT) {
                dvWrtemp(dvFile,
                    "utInlineC bool %0%1%2(%3%1 %1) {\n"
                    "    return (%0%1s.%2[%3%12ValidIndex(%1) >> 3] >> (%3%12ValidIndex(%1) & 7)) & 1;}\n"
                    "utInlineC void %0%1Set%2(%3%1 %1, bool value) {\n"
                    "%4    %0%1s.%2[%3%12ValidIndex(%1) >> 3] = (uint8)((%0%1s.%2[%3%12ValidIndex(%1) >> 3] &\n"
                    "        ~(1 << (%3%12ValidIndex(%1) & 7))) | ((value != 0) << (%3%12ValidIndex(%1) & 7)));%5}\n",
                    dvPrefix, name, propName, dvClassGetPrefix(theClass), preString, postString);
            } else {
                if(theUnion == dvUnionNull && cache == dvCacheNull) {
                    dvWrtemp(dvFile,
                        "utInlineC %7 %0%1%3%2(%4%1 %1) {return %0%1s.%2[%4%12ValidIndex(%1)];}\n"
                        "utInlineC void %0%1Set%2(%4%1 %1, %7 value) {%5%0%1s.%2[%4%12ValidIndex(%1)] = "
                        "value;%6}\n",
                        dvPrefix, name, propName, accessString,
                        dvClassGetPrefix(theClass), preString, postString, propTypeString);
                } else if(theUnion != dvUnionNull) {
                    dvWrtemp(dvFile,
                        "utInlineC %8 %0%1%3%2(%4%1 %1) {return %0%1s.%7[%4%12ValidIndex(%1)].%2;}\n"
                        "utInlineC void %0%1Set%2(%4%1 %1, %8 value) {\n"
                        "%5    %0%1s.%7[%4%12ValidIndex(%1)].%2 = value;%6}\n",
                        dvPrefix, name, propName, accessString, dvClassGetPrefix(theClass), preString, postString,
                        dvUnionGetFieldName(theUnion), propTypeString);
                } else {
                    dvWrtemp(dvFile,
                        "utInlineC %6 %0%1%3%2(%4%1 %1) {return %0%1s.%5[%4%12ValidIndex(%1)].%2;}\n"
                        "utInlineC void %0%1Set%2(%4%1 %1, %6 value) {\n"
                        "    %0%1s.%5[%4%12ValidIndex(%1)].%2 = value;}\n",
                        dvPrefix, name, propName, accessString, dvClassGetPrefix(theClass), dvCacheGetFieldName(cache),
                        propTypeString);
                }
            }
        }
    } dvEndClassProperty;
}

/*--------------------------------------------------------------------------------------------------
  Return a string representing the parameters to the find function for the relationship.
--------------------------------------------------------------------------------------------------*/
static char *getRelationshipKeyParameters(
    dvRelationship relationship)
{
    dvProperty property;
    dvKey key;
    char *parameters = "";

    dvForeachRelationshipKey(relationship, key) {
        property = dvKeypropertyGetProperty(dvKeyGetLastKeyproperty(key));
        if(!dvPropertyArray(property)) {
            parameters = utSprintf("%s, %s %s", parameters, dvPropertyGetTypeName(property), dvPropertyGetName(property));
        } else if(!dvPropertyFixedSize(property)) {
            parameters = utSprintf("%s, %s *%s, uint32 %sLength", parameters, dvPropertyGetTypeName(property),
                dvPropertyGetName(property), dvPropertyGetName(property));
        } else {
            parameters = utSprintf("%s, %s *%s", parameters, dvPropertyGetTypeName(property),
                dvPropertyGetName(property));
        }
    } dvEndRelationshipKey;
    return parameters;
}

/*--------------------------------------------------------------------------------------------------
  Write theClass's child rel macros.
--------------------------------------------------------------------------------------------------*/
static void writeClassChildRelMacros(
    dvClass theClass)
{
    dvRelationship relationship;
    dvClass childClass;
    dvRelationshipType type;
    char *name = dvClassGetName(theClass);
    char *keyParams;

    dvForeachClassChildRelationship(theClass, relationship) {
        type = dvRelationshipGetType(relationship);
        if(dvRelationshipAccessChild(relationship)) {
            childClass = dvRelationshipGetChildClass(relationship);
            if(type == REL_HASHED || (type == REL_ORDERED_LIST && dvRelationshipGetFirstKey(relationship) != dvKeyNull)) {
                keyParams = getRelationshipKeyParameters(relationship);
                dvWrtemp(dvFile,
                    "%5%3 %0%1Find%2%3(%4%1 %1%6);\n",
                    dvPrefix, name, dvRelationshipGetChildLabel(relationship),
                    dvClassGetName(childClass), dvClassGetPrefix(theClass),
                    dvClassGetPrefix(childClass), keyParams);
                if(type == REL_ORDERED_LIST) {
                    dvWrtemp(dvFile,
                        "%5%3 %0%1FindNext%2%3(%4%1 %1%6);\n"
                        "%5%3 %0%1FindPrev%2%3(%4%1 %1%6);\n",
                        dvPrefix, name, dvRelationshipGetChildLabel(relationship),
                        dvClassGetName(childClass), dvClassGetPrefix(theClass),
                        dvClassGetPrefix(childClass), keyParams);
                }
                if(type == REL_HASHED && dvRelationshipHashedByName(relationship)) {
                    dvWrtemp(dvFile,
                        "void %0%1Rename%2%3(%4%1 %1, %5%3 _%3, utSym sym);\n",
                        dvPrefix, name, dvRelationshipGetChildLabel(relationship),
                        dvClassGetName(childClass), dvClassGetPrefix(theClass),
                        dvClassGetPrefix(childClass));
                    dvWrtemp(dvFile,
                        "utInlineC char *%0%2Get%1Name(%3%2 %2) {return utSymGetName(%0%2Get%1Sym(%2));}\n",
                        dvPrefix, dvRelationshipGetChildLabel(relationship), dvClassGetName(childClass),
                        dvClassGetPrefix(childClass));
                }
            }
            if(type == REL_LINKED_LIST || type == REL_DOUBLY_LINKED || type == REL_TAIL_LINKED ||
                    type == REL_ORDERED_LIST || (type == REL_HASHED && !dvRelationshipUnordered(relationship))) {
                dvWrtemp(dvFile,
                    "#define %0Foreach%1%2%3(pVar, cVar) \\\n"
                    "    for(cVar = %0%1GetFirst%2%3(pVar); cVar != %5%3Null; \\\n"
                    "        cVar = %0%3GetNext%1%2%3(cVar))\n"
                    "#define %0End%1%2%3\n",
                    dvPrefix, name, dvRelationshipGetChildLabel(relationship),
                    dvClassGetName(childClass), dvClassGetPrefix(theClass),
                    dvClassGetPrefix(childClass));
                dvWrtemp(dvFile,
                    "#define %0SafeForeach%1%2%3(pVar, cVar) { \\\n"
                    "    %5%3 _next%3; \\\n"
                    "    for(cVar = %0%1GetFirst%2%3(pVar); cVar != %5%3Null; cVar = _next%3) { \\\n"
                    "        _next%3 = %0%3GetNext%1%2%3(cVar);\n"
                    "#define %0EndSafe%1%2%3 }}\n",
                    dvPrefix, name, dvRelationshipGetChildLabel(relationship),
                    dvClassGetName(childClass), dvClassGetPrefix(theClass),
                    dvClassGetPrefix(childClass));
            } else if(type == REL_HASHED && dvRelationshipUnordered(relationship)) {
                dvWrtemp(dvFile,
                    "#define %0Foreach%1%2%3(pVar, cVar) {\\\n"
                    "    uint32 _xChild;\\\n"
                    "    for(_xChild = 0; _xChild < %0%1GetNum%2%3Table(pVar); _xChild++) {\\\n"
                    "        for(cVar = %0%1Geti%2%3Table(pVar, _xChild); cVar != %5%3Null; \\\n"
                    "                cVar = %0%3GetNextTable%1%2%3(cVar))\n"
                    "#define %0End%1%2%3 }}\n",
                    dvPrefix, name, dvRelationshipGetChildLabel(relationship),
                    dvClassGetName(childClass), dvClassGetPrefix(theClass),
                    dvClassGetPrefix(childClass));
                dvWrtemp(dvFile,
                    "#define %0SafeForeach%1%2%3(pVar, cVar) {\\\n"
                    "    %5%3 _next%3;\\\n"
                    "    uint32 _xChild;\\\n"
                    "    for(_xChild = 0; _xChild < %0%1GetNum%2%3Table(pVar); _xChild++) {\\\n"
                    "        for(cVar = %0%1Geti%2%3Table(pVar, _xChild); cVar != %5%3Null; cVar = _next%3) {\\\n"
                    "            _next%3 = %0%3GetNextTable%1%2%3(cVar);\n"
                    "#define %0EndSafe%1%2%3 }}}\n",
                    dvPrefix, name, dvRelationshipGetChildLabel(relationship),
                    dvClassGetName(childClass), dvClassGetPrefix(theClass),
                    dvClassGetPrefix(childClass));
            }
            if(type == REL_ARRAY || type == REL_HEAP) {
                dvWrtemp(dvFile,
                    "#define %0Foreach%1%2%3(pVar, cVar) { \\\n"
                    "    uint32 _x%3; \\\n"
                    "    for(_x%3 = 0; _x%3 < %0%1GetUsed%2%3(pVar); _x%3++) { \\\n"
                    "        cVar = %0%1Geti%2%3(pVar, _x%3); \\\n"
                    "        if(cVar != %4%3Null) {\n"
                    "#define %0End%1%2%3 }}}\n",
                    dvPrefix, name, dvRelationshipGetChildLabel(relationship),
                    dvClassGetName(childClass), dvClassGetPrefix(childClass));
            }
        }
    } dvEndClassChildRelationship;
}

/*--------------------------------------------------------------------------------------------------
  Write the constructor callback in the creation macro.
--------------------------------------------------------------------------------------------------*/
static void writeClassConstructorCallback(
    dvClass theClass)
{
    dvWrtemp(dvFile,
        "    if(%0%1ConstructorCallback != NULL) {\n"
        "        %0%1ConstructorCallback(%1);\n"
        "    }\n",
        dvPrefix, dvClassGetName(theClass));
}

/*--------------------------------------------------------------------------------------------------
  Write code to initialize properties of an object.
--------------------------------------------------------------------------------------------------*/
void writePropertyInits(
    dvClass theClass)
{
    dvUnion theUnion;
    dvProperty prop;
    char *nullObj;

    dvForeachClassProperty(theClass, prop) {
        if(dvPropertyView(prop)) {
                dvWrtemp(dvFile,
                    "    /* %0.%1 init is user responsability */\n",
                    dvClassGetName(theClass), dvPropertyGetName(prop));
        }
        else if(dvPropertyArray(prop)) {
            if(!dvPropertyFixedSize(prop)) {
                dvWrtemp(dvFile,
                    "    %0%1SetNum%2(%1, 0);\n",
                    dvPrefix, dvClassGetName(theClass), dvPropertyGetName(prop));
            } else {
                /* TODO: write this */
            }
        } else {
            theUnion = dvPropertyGetUnion(prop);
            if(theUnion == dvUnionNull || dvUnionGetFirstProperty(theUnion) == prop) {
                nullObj = dvPropertyFindInitializer(prop);
                dvWrtemp(dvFile,
                    "    %0%1Set%2(%1, %3);\n",
                    dvPrefix, dvClassGetName(theClass), dvPropertyGetName(prop), nullObj);
            }
        }
    } dvEndClassProperty;
}

/*--------------------------------------------------------------------------------------------------
  Write a create macro for a free-list class.
--------------------------------------------------------------------------------------------------*/
static void writeCreateMacrosForFreeListClass(
    dvClass theClass)
{
    char *name = dvClassGetName(theClass);

    dvWrtemp(dvFile,
        "utInlineC %0%1 %0%1AllocRaw(void) {\n"
        "    %0%1 %1;\n"
        "    if(%0RootData.firstFree%1 != %0%1Null) {\n"
        "        %1 = %0RootData.firstFree%1;\n"
        "        %0SetFirstFree%1(%0%1NextFree(%1));\n"
        "    } else {\n"
        "        if(%0RootData.used%1 == %0RootData.allocated%1) {\n"
        "            %0%1AllocMore();\n"
        "        }\n"
        "        %1 = %0Index2%1(%0RootData.used%1);\n"
        "        %0SetUsed%1(%0Used%1() + 1);\n"
        "    }\n"
        "    return %1;}\n",
        dvPrefix, name);
    dvWrtemp(dvFile,
        "utInlineC %0%1 %0%1Alloc(void) {\n"
        "    %0%1 %1 = %0%1AllocRaw();\n",
        dvPrefix, name);
    writePropertyInits(theClass);
    writeClassConstructorCallback(theClass);
    dvWrtemp(dvFile, "    return %0;}\n", name);
}

/*--------------------------------------------------------------------------------------------------
  Write a create macro for a create-only class.
--------------------------------------------------------------------------------------------------*/
static void writeCreateMacrosForCreateOnlyClass(
    dvClass theClass)
{
    char *name = dvClassGetName(theClass);

    dvWrtemp(dvFile,
        "utInlineC %0%1 %0%1AllocRaw(void) {\n"
        "    %0%1 %1;\n"
        "    if(%0RootData.used%1 == %0RootData.allocated%1) {\n"
        "        %0%1AllocMore();\n"
        "    }\n"
        "    %1 = %0Index2%1(%0RootData.used%1);\n"
        "    %0SetUsed%1(%0Used%1() + 1);\n"
        "    return %1;}\n",
        dvPrefix, name);
    dvWrtemp(dvFile,
        "utInlineC %0%1 %0%1Alloc(void) {\n"
        "    %0%1 %1 = %0%1AllocRaw();\n",
        dvPrefix, name);
    writePropertyInits(theClass);
    writeClassConstructorCallback(theClass);
    dvWrtemp(dvFile, "    return %0;}\n", name);
}

/*--------------------------------------------------------------------------------------------------
  Write the create macro for theClass.
--------------------------------------------------------------------------------------------------*/
static void writeClassCreateMacro(
    dvClass theClass)
{
    if(dvClassGetMemoryStyle(theClass) == MEM_FREE_LIST) {
        writeCreateMacrosForFreeListClass(theClass);
    } else if(dvClassGetMemoryStyle(theClass) == MEM_CREATE_ONLY) {
        writeCreateMacrosForCreateOnlyClass(theClass);
    } else {
        utExit("Unknown type of memory option for %s", dvClassGetName(theClass));
    }
}

/*--------------------------------------------------------------------------------------------------
  Write an init macro for a free-list class.
--------------------------------------------------------------------------------------------------*/
static void writeClassInitMacro(
    dvClass theClass)
{
    char *name = dvClassGetName(theClass);

    dvWrtemp(dvFile,
        "utInlineC void %0%1Init(%2%1 %1) {\n",
        dvPrefix, name, dvClassGetPrefix(theClass));
    writePropertyInits(theClass);
    writeClassConstructorCallback(theClass);
    dvWrtemp(dvFile, "}\n");
}

/*--------------------------------------------------------------------------------------------------
  Write all misc. macros for traversal of theClass.
--------------------------------------------------------------------------------------------------*/
static void writeClassMiscMacros(
    dvClass theClass)
{
    char *name = dvClassGetName(theClass);
    dvProperty prop;
    char *preString, *postString;

    dvWrtemp(dvFile,
        "utInlineC void %0%1SetConstructorCallback(void(*func)(%0%1)) {%0%1ConstructorCallback = func;}\n"
        "utInlineC %0%1CallbackType %0%1GetConstructorCallback(void) {return %0%1ConstructorCallback;}\n",
        dvPrefix, name);
    if(dvClassGetMemoryStyle(theClass) == MEM_CREATE_ONLY) {
        dvWrtemp(dvFile,
            "utInlineC %0%1 %0First%1(void) {return %0RootData.used%1 == 1? %0%1Null : %0Index2%1(1);}\n"
            "utInlineC %0%1 %0Last%1(void) {return %0RootData.used%1 == 1? %0%1Null :\n"
            "    %0Index2%1(%0RootData.used%1 - 1);}\n"
            "utInlineC %0%1 %0Next%1(%0%1 %1) {return %0%12ValidIndex(%1) + 1 == %0RootData.used%1? %0%1Null :\n"
            "    %1 + 1;}\n"
            "utInlineC %0%1 %0Prev%1(%0%1 %1) {return %0%12ValidIndex(%1) == 1? %0%1Null : %1 - 1;}\n"
            "#define %0Foreach%1(var) \\\n"
            "    for(var = %0Index2%1(1); %0%12Index(var) != %0RootData.used%1; var++)\n"
            "#define %0End%1\n",
            dvPrefix, name, dvClassGetReferenceTypeName(theClass));
        dvWrtemp(dvFile, "utInlineC void %0%1FreeAll(void) {%0SetUsed%1(1);", dvPrefix, name);
        dvForeachClassProperty(theClass, prop) {
            if(dvPropertyArray(prop) && !dvPropertyFixedSize(prop) && !dvPropertyView(prop)) {
                dvWrtemp(dvFile, " %0SetUsed%1%2(0);" , dvPrefix, name,
                dvPropertyGetName(prop));
            }
        } dvEndClassProperty;
        dvWrtemp(dvFile, "}\n");
    } else if(dvClassGetMemoryStyle(theClass) == MEM_FREE_LIST) {
        prop = dvClassGetFreeListProperty(theClass);
        dvWrtemp(dvFile,
            "utInlineC void %0%1SetDestructorCallback(void(*func)(%0%1)) {%0%1DestructorCallback = func;}\n"
            "utInlineC %0%1CallbackType %0%1GetDestructorCallback(void) {return %0%1DestructorCallback;}\n",
            dvPrefix, name);
        dvWrtemp(dvFile,
            "utInlineC %0%1 %0%1NextFree(%0%1 %1) {return ((%0%1 *)(void *)(%0%1s.%2))[%0%12ValidIndex(%1)];}\n",
            dvPrefix, name, dvPropertyGetName(prop));
        preString = dvSwrtemp(!dvClassUndo(theClass)? "" :
            "\n    utRecordField(%0ModuleID, %2, %0%12ValidIndex(%1), true);\n    ",
            dvPrefix, name, utSprintf("%u", dvPropertyGetFieldNumber(prop)));
        postString = dvSwrtemp(!dvClassRedo(theClass)? "" :
            "\n    utRecordField(%0ModuleID, %2, %0%12ValidIndex(%1), false);",
            dvPrefix, name, utSprintf("%u", dvPropertyGetFieldNumber(prop)));
        dvWrtemp(dvFile,
            "utInlineC void %0%1SetNextFree(%0%1 %1, %0%1 value) {\n"
            "%3    ((%0%1 *)(void *)(%0%1s.%2))[%0%12ValidIndex(%1)] = value;%4}\n",
            dvPrefix, name, dvPropertyGetName(prop), preString, postString);
        dvWrtemp(dvFile,
            "utInlineC void %0%1Free(%0%1 %1) {\n",
            dvPrefix, name);
        dvForeachClassProperty(theClass, prop) {
            if(dvPropertyArray(prop) && !dvPropertyFixedSize(prop) && !dvPropertyView(prop)) {
                dvWrtemp(dvFile,
                    "    %0%1Free%2s(%1);\n",
                    dvPrefix, name, dvPropertyGetName(prop));
            }
        } dvEndClassProperty;
        dvWrtemp(dvFile,
            "    %0%1SetNextFree(%1, %0RootData.firstFree%1);\n"
            "    %0SetFirstFree%1(%1);}\n"
            "void %0%1Destroy(%0%1 %1);\n",
            dvPrefix, name);
    }
}

/*--------------------------------------------------------------------------------------------------
  Write attribute shortcut macros.
--------------------------------------------------------------------------------------------------*/
static void writeAttributeShortcuts(
    dvClass theClass)
{
    dvWrtemp(dvFile,
        "utInlineC %0Attribute %0%1FindAttribute(%2%1 %1, utSym sym) {\n"
        "    return %0%1GetAttrlist(%1) == %0AttrlistNull? %0AttributeNull :\n"
        "        %0AttrlistFindAttribute(%0%1GetAttrlist(%1), (sym));}\n"
        "void %0%1DeleteAttribute(%2%1 _%1, utSym sym);\n"
        "int64 %0%1GetInt64Attribute(%2%1 _%1, utSym sym);\n"
        "double %0%1GetDoubleAttribute(%2%1 _%1, utSym sym);\n"
        "bool %0%1GetBoolAttribute(%2%1 _%1, utSym sym);\n"
        "utSym %0%1GetSymAttribute(%2%1 _%1, utSym sym);\n"
        "char *%0%1GetStringAttribute(%2%1 _%1, utSym sym);\n"
        "uint8 *%0%1GetBlobAttribute(%2%1 _%1, utSym sym, uint32 *length);\n"
        "void %0%1SetInt64Attribute(%2%1 _%1, utSym sym, int64 value);\n"
        "void %0%1SetDoubleAttribute(%2%1 _%1, utSym sym, double value);\n"
        "void %0%1SetBoolAttribute(%2%1 _%1, utSym sym, bool value);\n"
        "void %0%1SetSymAttribute(%2%1 _%1, utSym sym, utSym value);\n"
        "void %0%1SetStringAttribute(%2%1 _%1, utSym sym, char *string);\n"
        "void %0%1SetBlobAttribute(%2%1 _%1, utSym sym, uint8 *data, uint32 length);\n"
        "utInlineC %0Attribute %0%1GetFirstAttribute(%2%1 %1) {\n"
        "    %0Attrlist attrlist = %0%1GetAttrlist(%1);\n"
        "    return attrlist == %0AttrlistNull? %0AttributeNull : %0AttrlistGetFirstAttribute(attrlist);}\n"
        "#define %0Foreach%1Attribute(_%1, attribute) \\\n"
        "    for(attribute = %0%1GetFirstAttribute(_%1), attribute != %0AttributeNull; \\\n"
        "        attribute = %0AttributeGetNextAttrlistAttribute(attribute))\n"
        "#define %0End%1Attribute\n"
        "void %0%1CopyAttributes(%2%1 old%1, %2%1 new%1);\n",
        dvPrefix, dvClassGetName(theClass), dvClassGetPrefix(theClass));
}

/*--------------------------------------------------------------------------------------------------
  Write all the access/modify macros for theClass.
--------------------------------------------------------------------------------------------------*/
static void writeClassMacros(
    dvClass theClass)
{
    writeClassPropMacros(theClass);
    if(dvClassGetBaseClass(theClass) == dvClassNull) {
        writeClassMiscMacros(theClass);
        writeClassCreateMacro(theClass);
    } else {
        writeClassInitMacro(theClass);
    }
}

/*--------------------------------------------------------------------------------------------------
  Check to see if Typedef section is needed for this tool.
--------------------------------------------------------------------------------------------------*/
static bool needTypedefSection(
    dvModule module)
{
    dvClass theClass;

    dvForeachModuleClass(module, theClass) {
        if(dvClassGetBaseClass(theClass) == dvClassNull) {
            return true;
        }
    } dvEndModuleClass;
    return false;
}

/*--------------------------------------------------------------------------------------------------
  Write the type definitions for handles to objects.
--------------------------------------------------------------------------------------------------*/
static void writeTypedefs(
    dvModule module)
{
    dvClass theClass;

    dvWrtemp(dvFile,
        "/* Class reference definitions */\n"
        "#if (defined(DD_DEBUG) && !defined(DD_NOSTRICT)) || defined(DD_STRICT)\n", dvFile);
    dvForeachModuleClass(module, theClass) {
        if(dvClassGetBaseClass(theClass) == dvClassNull) {
            dvWrtemp(dvFile,
                "typedef struct _struct_%0%1{char val;} *%0%1;\n"
                "#define %0%1Null ((%0%1)0)\n",
                dvPrefix, dvClassGetName(theClass), dvClassGetReferenceTypeName(theClass));
        }
    } dvEndModuleClass;
    fputs("#else\n", dvFile);
    dvForeachModuleClass(module, theClass) {
        if(dvClassGetBaseClass(theClass) == dvClassNull) {
            dvWrtemp(dvFile, "typedef %2 %0%1;\n"
                "#define %0%1Null 0\n",
                dvPrefix, dvClassGetName(theClass), dvClassGetReferenceTypeName(theClass));
        }
    } dvEndModuleClass;
    fputs("#endif\n\n", dvFile);
}

/*--------------------------------------------------------------------------------------------------
  Write macros for validating object references.
--------------------------------------------------------------------------------------------------*/
static void writeValidateMacros(
    dvModule module)
{
    dvClass theClass;

    dvWrtemp(dvFile, "/* Validate macros */\n");
    fputs("#if defined(DD_DEBUG)\n", dvFile);
    dvForeachModuleClass(module, theClass) {
        if(dvClassGetBaseClass(theClass) == dvClassNull) {
            dvWrtemp(dvFile,
                "utInlineC %0%1 %0Valid%1(%0%1 %1) {\n"
                "    utAssert(utLikely(%1 != %0%1Null && (%2)(%1 - (%0%1)0) < %0RootData.used%1));\n"
                "    return %1;}\n",
                dvPrefix, dvClassGetName(theClass), dvClassGetReferenceTypeName(theClass));
        }
    } dvEndModuleClass;
    fputs("#else\n", dvFile);
    dvForeachModuleClass(module, theClass) {
        if(dvClassGetBaseClass(theClass) == dvClassNull) {
            dvWrtemp(dvFile,
                "utInlineC %0%1 %0Valid%1(%0%1 %1) {return %1;}\n",
                dvPrefix, dvClassGetName(theClass));
        }
    } dvEndModuleClass;
    fputs("#endif\n\n", dvFile);
}

/*--------------------------------------------------------------------------------------------------
  Write the enumerated types.
--------------------------------------------------------------------------------------------------*/
static void writeEnms(
    dvModule module)
{
    dvEnum theEnum;
    dvEntry entry;

    dvForeachModuleEnum(module, theEnum) {
        dvWrtemp(dvFile, "/* %0 enumerated type */\n", dvEnumGetName(theEnum));
        fputs("typedef enum {\n", dvFile);
        dvForeachEnumEntry(theEnum, entry) {
            dvWrtemp(dvFile,
                "    %0%1",
                utSymGetName(dvEnumGetPrefixSym(theEnum)), dvEntryGetName(entry));
            if(dvEntryGetNextEnumEntry(entry) != dvEntryNull) {
                fprintf(dvFile, " = %u,\n", dvEntryGetValue(entry));
            } else {
                fprintf(dvFile, " = %u\n", dvEntryGetValue(entry));
            }
        } dvEndEnumEntry;
        dvWrtemp(dvFile, "} %0%1;\n\n", dvPrefix, dvEnumGetName(theEnum));
    } dvEndModuleEnum;
}

/*--------------------------------------------------------------------------------------------------
  Write the header's top portion.
--------------------------------------------------------------------------------------------------*/
static void writeHeaderTop(
    dvModule module)
{
    dvLink link;
    bool hasHeader = false;

    dvWrtemp(dvFile,
        "/*----------------------------------------------------------------------------------------\n"
        "  Module header file for: %0 module\n"
        "----------------------------------------------------------------------------------------*/\n"
        "#ifndef %c0DATABASE_H\n\n"
        "#define %c0DATABASE_H\n\n"
        "#if defined __cplusplus\n"
        "extern \"C\" {\n"
        "#endif\n\n"
        "#ifndef DD_UTIL_H\n"
        "#include \"ddutil.h\"\n"
        "#endif\n\n",
        dvPrefix);
    if(dvModuleGetFirstTypedef(module) != dvTypedefNull) {
        dvWrtemp(dvFile,
            "#include \"%0typedef.h\"\n",
            dvPrefix);
        hasHeader = true;
    }
    dvForeachModuleImportLink(module, link) {
        dvWrtemp(dvFile,
            "#include \"%0database.h\"\n",
            dvModuleGetPrefix(dvLinkGetExportModule(link)));
        hasHeader = true;
    } dvEndModuleImportLink;
    if(hasHeader) {
        dvWrtemp(dvFile, "\n");
    }
    dvWrtemp(dvFile,
        "extern uint8 %0ModuleID;\n",
        dvPrefix);
}

/*--------------------------------------------------------------------------------------------------
  Write the overall types in tool including enums.
--------------------------------------------------------------------------------------------------*/
static void writeTypes(
    dvModule module)
{
    if(needTypedefSection(module)) {
        writeTypedefs(module);
    }
    writeEnms(module);
}

/*--------------------------------------------------------------------------------------------------
  Write the type definitions for handles to objects.
--------------------------------------------------------------------------------------------------*/
static void writeTypeConversionMacros(
    dvModule module)
{
    dvClass theClass;

    dvWrtemp(dvFile,
        "/* Object ref to integer conversions */\n"
        "#if (defined(DD_DEBUG) && !defined(DD_NOSTRICT)) || defined(DD_STRICT)\n", dvFile);
    dvForeachModuleClass(module, theClass) {
        if(dvClassGetBaseClass(theClass) == dvClassNull) {
            dvWrtemp(dvFile,
                "utInlineC %2 %0%12Index(%0%1 %1) {return %1 - (%0%1)0;}\n"
                "utInlineC %2 %0%12ValidIndex(%0%1 %1) {return %0Valid%1(%1) - (%0%1)0;}\n"
                "utInlineC %0%1 %0Index2%1(%2 x%1) {return (%0%1)(x%1 + (%0%1)(0));}\n",
                dvPrefix, dvClassGetName(theClass), dvClassGetReferenceTypeName(theClass));
        }
    } dvEndModuleClass;
    fputs("#else\n", dvFile);
    dvForeachModuleClass(module, theClass) {
        if(dvClassGetBaseClass(theClass) == dvClassNull) {
            dvWrtemp(dvFile,
                "utInlineC %2 %0%12Index(%0%1 %1) {return %1;}\n"
                "utInlineC %2 %0%12ValidIndex(%0%1 %1) {return %0Valid%1(%1);}\n"
                "utInlineC %0%1 %0Index2%1(%2 x%1) {return x%1;}\n",
                dvPrefix, dvClassGetName(theClass), dvClassGetReferenceTypeName(theClass));
        }
    } dvEndModuleClass;
    fputs("#endif\n\n", dvFile);
}


/*--------------------------------------------------------------------------------------------------
  Write the header file.
--------------------------------------------------------------------------------------------------*/
void dvWriteHeaderFile(
    dvModule module,
    char *includeFile)
{
    dvClass theClass;
    char *fileName = includeFile;

    if(utDirectoryExists(includeFile)) {
        fileName = utSprintf("%s%c%sdatabase.h", includeFile, UTDIRSEP,
            dvModuleGetPrefix(module));
    }
    dvFile = fopen(fileName, "wt");
    if(dvFile == NULL) {
        utError("Could not open file %s", fileName);
        return;
    }
    utLogMessage("Generating header file %s", fileName);
    dvPrefix = dvModuleGetPrefix(module);
    writeHeaderTop(module);
    writeTypes(module);
    writeConstructorDestructorVariables(module);
    writeRoot(module);
    writeValidateMacros(module);
    writeTypeConversionMacros(module);
    dvForeachModuleClass(module, theClass) {
        writeUnionTypes(theClass);
        writeCacheTypes(theClass);
        writeClassStruct(theClass);
        writeClassExternFuncs(theClass);
        writeClassMacros(theClass);
        fputs("\n", dvFile);
    } dvEndModuleClass;
    /* Have to do these after all the get/set macros are declared */
    dvWrtemp(dvFile,
        "/*----------------------------------------------------------------------------------------\n"
        "  Relationship macros between classes.\n"
        "----------------------------------------------------------------------------------------*/\n");
    dvForeachModuleClass(module, theClass) {
        writeClassChildRelMacros(theClass);
        writeClassRelExternFuncs(theClass);
    } dvEndModuleClass;
    /* Have to do these after all the relationship macros are declared */
    dvForeachModuleClass(module, theClass) {
        if(dvClassGenerateAttributes(theClass)) {
            writeAttributeShortcuts(theClass);
        }
    } dvEndModuleClass;
    writeHeaderBot(module);
    fclose(dvFile);
}
