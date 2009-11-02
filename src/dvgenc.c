/*
    DataDraw
    Copyright(C) 1992-2006 Bill Cox <bill@billrocks.org>

    This program can be distributed under the terms of the GNU Library GPL.
    See the file COPYING.LIB.
*/

#include "dv.h"

static char *dvPrefix;


/*--------------------------------------------------------------------------------------------------
  Write a hash function for just this key.
--------------------------------------------------------------------------------------------------*/
static void writeKeyHash(
    dvKey key,
    bool useParamName)
{
    dvProperty property = dvKeypropertyGetProperty(dvKeyGetLastKeyproperty(key));
    dvClass childClass = dvPropertyGetClass(property);
    dvClass pointerClass;
    dvPropertyType type = dvPropertyGetType(property);
    char *param = utSprintf("_%s",
        dvClassGetName(dvPropertyGetClass(dvKeypropertyGetProperty(dvKeyGetFirstKeyproperty(key)))));

    if(dvPropertyArray(property)) {
        dvWrtemp(dvFile, "utHashData(%0, sizeof(%1)*%2)",
            dvKeyGetAccessMacro(key, useParamName, param),
            dvPropertyGetTypeName(property),
            dvKeyGetLengthMacro(key, useParamName, param));
        return;
    }
    switch(type) {
    case PROP_UINT: case PROP_INT: case PROP_CHAR: case PROP_ENUM: case PROP_BOOL: case PROP_BIT:
        dvWrtemp(dvFile, "(uint32)%0", dvKeyGetAccessMacro(key, useParamName, param));
        break;
    case PROP_FLOAT:
        dvWrtemp(dvFile, "utHashFloat(%0)", dvKeyGetAccessMacro(key, useParamName, param));
        break;
    case PROP_DOUBLE:
        dvWrtemp(dvFile, "utHashDouble(%0)", dvKeyGetAccessMacro(key, useParamName, param));
        break;
    case PROP_SYM:
        dvWrtemp(dvFile, "utSymGetHashValue(%0)", dvKeyGetAccessMacro(key, useParamName, param));
        break;
    case PROP_TYPEDEF:
        if(useParamName) {
            dvWrtemp(dvFile, "utHashData(&%0, sizeof(%1))",
                dvKeyGetAccessMacro(key, useParamName, param),
                dvPropertyGetTypeName(property));
        } else {
            dvWrtemp(dvFile, "utHashData(%0%1s.%2 + %4%12Index(_%1), sizeof(%3))",
                dvPrefix,
                dvClassGetName(childClass), 
                dvPropertyGetName(property),
                dvPropertyGetTypeName(property),
                dvClassGetPrefix(childClass));
        }
        break;
    case PROP_POINTER:
        pointerClass = dvPropertyGetClassProp(property);
        dvWrtemp(dvFile, "%1%22Index(%0)",
            dvKeyGetAccessMacro(key, useParamName, param),
            dvClassGetPrefix(pointerClass),
            dvClassGetName(pointerClass));
        break;
    default:
        utExit("Unexpected key type");
    }
}

/*--------------------------------------------------------------------------------------------------
  Write a hash function for this key, and all the next keys on the relationship.
--------------------------------------------------------------------------------------------------*/
static void writeHashFunction(
    dvKey key,
    bool useParamName)
{
    dvKey nextKey = dvKeyGetNextRelationshipKey(key);

    if(nextKey == dvKeyNull) {
        writeKeyHash(key, useParamName);
        return;
    }
    dvWrtemp(dvFile, "utHashValues(");
    writeKeyHash(key, useParamName);
    dvWrtemp(dvFile, ", ");
    writeHashFunction(nextKey, useParamName);
    dvWrtemp(dvFile, ")");
}

/*--------------------------------------------------------------------------------------------------
  Write a function to resize a hash table.
--------------------------------------------------------------------------------------------------*/
static void writeHashTableResizeFunction(
    dvRelationship relationship)
{
    dvClass parentClass = dvRelationshipGetParentClass(relationship);
    dvClass childClass = dvRelationshipGetChildClass(relationship);
    char *propName = utSprintf("%s%sTable", dvRelationshipGetChildLabel(relationship),
        dvClassGetName(childClass));
    utSym propSym = utSymCreate(propName);
    dvProperty tablesProp = dvClassFindProperty(parentClass, propSym);

    utAssert(tablesProp != dvPropertyNull);
    dvWrtemp(dvFile, 
        "/*----------------------------------------------------------------------------------------\n"
        "  Increase the size of the hash table.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "static void resize%1%2%3HashTable(\n"
        "    %4%1 %1)\n"
        "{\n"
        "    %5%3 _%3, prev%3, next%3;\n"
        "    uint32 oldNum%3s = %0%1GetNum%2%3Table(%1);\n"
        "    uint32 newNum%3s = oldNum%3s << 1;\n"
        "    uint32 x%3, index;\n"
        "\n"
        "    if(newNum%3s == 0) {\n"
        "        newNum%3s = 2;\n"
        "        %0%1Alloc%2%3Tables(%1, 2);\n"
        "    } else {\n"
        "        %0%1Resize%2%3Tables(%1, newNum%3s);\n"
        "    }\n"
        "    for(x%3 = 0; x%3 < oldNum%3s; x%3++) {\n"
        "        _%3 = %0%1Geti%2%3Table(%1, x%3);\n"
        "        prev%3 = %5%3Null;\n"
        "        while(_%3 != %5%3Null) {\n"
        "            next%3 = %0%3GetNextTable%1%2%3(_%3);\n"
        "            index = (newNum%3s - 1) & ",
        dvPrefix, dvClassGetName(parentClass), dvRelationshipGetChildLabel(relationship),
        dvClassGetName(childClass), dvClassGetPrefix(parentClass), dvClassGetPrefix(childClass));
    writeHashFunction(dvRelationshipGetFirstKey(relationship), false);
    dvWrtemp(dvFile, 
        ";\n"
        "            if(index != x%3) {\n"
        "                if(prev%3 == %5%3Null) {\n"
        "                    %0%1Seti%2%3Table(%1, x%3, next%3);\n"
        "                } else {\n"
        "                    %0%3SetNextTable%1%2%3(prev%3, next%3);\n"
        "                }\n"
        "                %0%3SetNextTable%1%2%3(_%3, %0%1Geti%2%3Table(%1, index));\n"
        "                %0%1Seti%2%3Table(%1, index, _%3);\n"
        "            } else {\n"
        "                prev%3 = _%3;\n"
        "            }\n"
        "            _%3 = next%3;\n"
        "        }\n"
        "    }\n"
        "}\n\n",
        dvPrefix, dvClassGetName(parentClass), dvRelationshipGetChildLabel(relationship),
        dvClassGetName(childClass), dvClassGetPrefix(parentClass), dvClassGetPrefix(childClass));
}

/*--------------------------------------------------------------------------------------------------
  Write a function to add an element to a hash table.
--------------------------------------------------------------------------------------------------*/
static void writeHashTableAddFunction(
    dvRelationship relationship)
{
    dvClass parentClass = dvRelationshipGetParentClass(relationship);
    dvClass childClass = dvRelationshipGetChildClass(relationship);

    dvWrtemp(dvFile, 
        "/*----------------------------------------------------------------------------------------\n"
        "  Add the %2%3 to the %1.  If the table is near full, build a new one twice\n"
        "  as big, delete the old one, and return the new one.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "static void add%1%2%3ToHashTable(\n"
        "    %4%1 %1,\n"
        "    %5%3 _%3)\n"
        "{\n"
        "    %5%3 next%3;\n"
        "    uint32 index;\n"
        "\n"
        "    if(%0%1GetNum%2%3(%1) >> 1 >= %0%1GetNum%2%3Table(%1)) {\n"
        "        resize%1%2%3HashTable(%1);\n" /* This adds _%3 to the hash table */
        "    }\n"
        "    index = (%0%1GetNum%2%3Table(%1) - 1) & ",
        dvPrefix, dvClassGetName(parentClass), dvRelationshipGetChildLabel(relationship),
        dvClassGetName(childClass), dvClassGetPrefix(parentClass), dvClassGetPrefix(childClass));
    writeHashFunction(dvRelationshipGetFirstKey(relationship), false);
    dvWrtemp(dvFile, 
        ";\n"
        "    next%3 = %0%1Geti%2%3Table(%1, index);\n"
        "    %0%3SetNextTable%1%2%3(_%3, next%3);\n"
        "    %0%1Seti%2%3Table(%1, index, _%3);\n"
        "    %0%1SetNum%2%3(%1, %0%1GetNum%2%3(%1) + 1);\n"
        "}\n\n",
        dvPrefix, dvClassGetName(parentClass), dvRelationshipGetChildLabel(relationship),
        dvClassGetName(childClass), dvClassGetPrefix(parentClass), dvClassGetPrefix(childClass));
}

/*--------------------------------------------------------------------------------------------------
  Write a function to remove an element from a hash table.
--------------------------------------------------------------------------------------------------*/
static void writeHashTableRemoveFunction(
    dvRelationship relationship)
{
    dvClass parentClass = dvRelationshipGetParentClass(relationship);
    dvClass childClass = dvRelationshipGetChildClass(relationship);

    dvWrtemp(dvFile, 
        "/*----------------------------------------------------------------------------------------\n"
        "  Remove the %2%3 from the hash table.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "static void remove%1%2%3FromHashTable(\n"
        "    %4%1 %1,\n"
        "    %5%3 _%3)\n"
        "{\n"
        "    uint32 index = (%0%1GetNum%2%3Table(%1) - 1) & ",
        dvPrefix, dvClassGetName(parentClass), dvRelationshipGetChildLabel(relationship),
        dvClassGetName(childClass), dvClassGetPrefix(parentClass), dvClassGetPrefix(childClass));
    writeHashFunction(dvRelationshipGetFirstKey(relationship), false);
    dvWrtemp(dvFile, 
        ";\n"
        "    %5%3 prev%3, next%3;\n"
        "    \n"
        "    next%3 = %0%1Geti%2%3Table(%1, index);\n"
        "    if(next%3 == _%3) {\n"
        "        %0%1Seti%2%3Table(%1, index, %0%3GetNextTable%1%2%3(next%3));\n"
        "    } else {\n"
        "        do {\n"
        "            prev%3 = next%3;\n"
        "            next%3 = %0%3GetNextTable%1%2%3(next%3);\n"
        "        } while(next%3 != _%3);\n"
        "        %0%3SetNextTable%1%2%3(prev%3, %0%3GetNextTable%1%2%3(_%3));\n"
        "    }\n"
        "    %0%1SetNum%2%3(%1, %0%1GetNum%2%3(%1) - 1);\n"
        "    %0%3SetNextTable%1%2%3(_%3, %5%3Null);\n"
        "}\n\n",
        dvPrefix, dvClassGetName(parentClass), dvRelationshipGetChildLabel(relationship),
        dvClassGetName(childClass), dvClassGetPrefix(parentClass), dvClassGetPrefix(childClass));
}

/*--------------------------------------------------------------------------------------------------
  Write the key parameters to the find function.
--------------------------------------------------------------------------------------------------*/
static void writeKeyParameters(
    dvRelationship relationship)
{
    dvProperty property;
    dvKey key;

    dvForeachRelationshipKey(relationship, key) {
        property = dvKeypropertyGetProperty(dvKeyGetLastKeyproperty(key));
        if(!dvPropertyArray(property)) {
            dvWrtemp(dvFile, ",\n    %0 %1", dvPropertyGetTypeName(property), dvPropertyGetName(property));
        } else if(!dvPropertyFixedSize(property)) {
            dvWrtemp(dvFile, ",\n    %0 *%1,\n    uint32 %1Length", dvPropertyGetTypeName(property),
                dvPropertyGetName(property));
        } else {
            dvWrtemp(dvFile, ",\n    %0 *%1", dvPropertyGetTypeName(property), dvPropertyGetName(property));
        }
    } dvEndRelationshipKey;
}

/*--------------------------------------------------------------------------------------------------
  Write a hash key match conditional expression.
--------------------------------------------------------------------------------------------------*/
static void writeHashKeyMatch(
    dvRelationship relationship)
{
    dvClass childClass = dvRelationshipGetChildClass(relationship);
    dvKeyproperty keyproperty;
    dvProperty property;
    dvKey key;
    bool isFirst = true;

    dvForeachRelationshipKey(relationship, key) {
        if(!isFirst) {
            dvWrtemp(dvFile, " && ");
        }
        isFirst = false;
        keyproperty = dvKeyGetLastKeyproperty(key);
        property = dvKeypropertyGetProperty(keyproperty);
        if(!dvPropertyArray(property)) {
            dvWrtemp(dvFile, "%0 == %1",
                dvKeyGetAccessMacro(key,false,utSprintf("_%s", dvClassGetName(childClass))),
                dvPropertyGetName(property));
        } else if(!dvPropertyFixedSize(property)) {
            dvWrtemp(dvFile, "%0 == %2Length && !memcmp(%1, %2, sizeof(%3)*%2Length)",
                dvKeyGetLengthMacro(key, false, utSprintf("_%s", dvClassGetName(childClass))),
                dvKeyGetAccessMacro(key, false, utSprintf("_%s", dvClassGetName(childClass))),
                dvPropertyGetName(property),
                dvPropertyGetTypeName(property));
        } else {
            dvWrtemp(dvFile, "!memcmp(%0, %1, sizeof(%2)*(%3))",
                dvKeyGetAccessMacro(key, false, utSprintf("_%s", dvClassGetName(childClass))),
                dvPropertyGetName(property),
                dvPropertyGetTypeName(property),
                dvPropertyGetIndex(property));
        }
    } dvEndRelationshipKey;
}

/*--------------------------------------------------------------------------------------------------
  Write a query by key lookup function for a hash-table relationship.
--------------------------------------------------------------------------------------------------*/
static void writeFindFunction(
    dvRelationship relationship)
{
    dvClass parentClass = dvRelationshipGetParentClass(relationship);
    dvClass childClass = dvRelationshipGetChildClass(relationship);

    dvWrtemp(dvFile,
        "/*----------------------------------------------------------------------------------------\n"
        "  Find the %2%3 from the %1 and its hash key.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "%5%3 %0%1Find%2%3(\n"
        "    %4%1 %1",
        dvPrefix, dvClassGetName(parentClass), dvRelationshipGetChildLabel(relationship),
        dvClassGetName(childClass), dvClassGetPrefix(parentClass), dvClassGetPrefix(childClass));
    writeKeyParameters(relationship);
    dvWrtemp(dvFile,
        ")\n"
        "{\n"
        "    uint32 mask = %0%1GetNum%2%3Table(%1) - 1;\n"
        "    %5%3 _%3;\n"
        "\n"
        "    if(mask + 1 != 0) {\n"
        "        _%3 = %0%1Geti%2%3Table(%1, ",
        dvPrefix, dvClassGetName(parentClass), dvRelationshipGetChildLabel(relationship),
        dvClassGetName(childClass), dvClassGetPrefix(parentClass), dvClassGetPrefix(childClass));
    writeHashFunction(dvRelationshipGetFirstKey(relationship), true);
    dvWrtemp(dvFile,
        " & mask);\n"
        "        while(_%3 != %5%3Null) {\n"
        "            if(",
        dvPrefix, dvClassGetName(parentClass), dvRelationshipGetChildLabel(relationship),
        dvClassGetName(childClass), dvClassGetPrefix(parentClass), dvClassGetPrefix(childClass));
    writeHashKeyMatch(relationship);
    dvWrtemp(dvFile,
        ") {\n"
        "                return _%3;\n"
        "            }\n"
        "            _%3 = %0%3GetNextTable%1%2%3(_%3);\n"
        "        }\n"
        "    }\n"
        "    return %5%3Null;\n"
        "}\n\n",
        dvPrefix, dvClassGetName(parentClass), dvRelationshipGetChildLabel(relationship),
        dvClassGetName(childClass), dvClassGetPrefix(parentClass), dvClassGetPrefix(childClass));
}

/*--------------------------------------------------------------------------------------------------
  Write a function to rename the object.
--------------------------------------------------------------------------------------------------*/
static void writeRenameFunction(
    dvRelationship relationship)
{
    dvClass parentClass = dvRelationshipGetParentClass(relationship);
    dvClass childClass = dvRelationshipGetChildClass(relationship);

    dvWrtemp(dvFile,
        "/*----------------------------------------------------------------------------------------\n"
        "  Find the %2%3 from the %1 and its name.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "void %0%1Rename%2%3(\n"
        "    %4%1 %1,\n"
        "    %5%3 _%3,\n"
        "    utSym sym)\n"
        "{\n"
        "    if(%0%3Get%2Sym(_%3) != utSymNull) {\n"
        "        remove%1%2%3FromHashTable(%1, _%3);\n"
        "    }\n"
        "    %0%3Set%2Sym(_%3, sym);\n"
        "    if(sym != utSymNull) {\n"
        "        add%1%2%3ToHashTable(%1, _%3);\n"
        "    }\n"
        "}\n\n",
        dvPrefix, dvClassGetName(parentClass), dvRelationshipGetChildLabel(relationship),
        dvClassGetName(childClass), dvClassGetPrefix(parentClass), dvClassGetPrefix(childClass),
        dvRelationshipGetParentLabel(relationship));
}

/*--------------------------------------------------------------------------------------------------
  Write an add relationship routine for theClass.
--------------------------------------------------------------------------------------------------*/
static void writeClassInsertFunction(
    dvClass theClass,
    dvRelationship relationship)
{
    char *parentName = dvClassGetName(theClass);
    dvClass childClass = dvRelationshipGetChildClass(relationship);
    char *childName = dvClassGetName(childClass);
    dvRelationshipType type = dvRelationshipGetType(relationship);

    dvWrtemp(dvFile, 
        "/*----------------------------------------------------------------------------------------\n"
        "  Add the %2%3 to the head of the list on the %1.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "void %0%1Insert%2%3(\n"
        "    %4%1 %1,\n"
        "    %5%3 _%3)\n"
        "{\n"
        "#if defined(DD_DEBUG)\n"
        "    if(%1 == %4%1Null) {\n"
        "        utExit(\"Non-existent %1\");\n"
        "    }\n"
        "    if(_%3 == %5%3Null) {\n"
        "        utExit(\"Non-existent %3\");\n"
        "    }\n",
        dvPrefix, parentName, dvRelationshipGetChildLabel(relationship), childName,
        dvClassGetPrefix(theClass), dvClassGetPrefix(childClass));
    if(dvRelationshipAccessParent(relationship)) {
        dvWrtemp(dvFile, 
            "    if(%0%2Get%4%1(_%2) != %3%1Null) {\n"
            "        utExit(\"Attempting to add %2 to %1 twice\");\n"
            "    }\n",
            dvPrefix, parentName, childName, dvClassGetPrefix(theClass),
            dvRelationshipGetParentLabel(relationship));
    }
    dvWrtemp(dvFile, "#endif\n");
    if(type != REL_HASHED || !dvRelationshipUnordered(relationship)) {
        if(type == REL_DOUBLY_LINKED || type == REL_HASHED) {
            dvWrtemp(dvFile, 
                "    %0%3SetNext%1%2%3(_%3, %0%1GetFirst%2%3(%1));\n"
                "    if(%0%1GetFirst%2%3(%1) != %5%3Null) {\n"
                "        %0%3SetPrev%1%2%3(%0%1GetFirst%2%3(%1), _%3);\n"
                "    }\n"
                "    %0%1SetFirst%2%3(%1, _%3);\n"
                "    %0%3SetPrev%1%2%3(_%3, %5%3Null);\n"
                "    if(%0%1GetLast%2%3(%1) == %5%3Null) {\n"
                "        %0%1SetLast%2%3(%1, _%3);\n"
                "    }\n",
                dvPrefix, parentName, dvRelationshipGetChildLabel(relationship), childName,
                dvClassGetPrefix(theClass), dvClassGetPrefix(childClass));
        } else {
            dvWrtemp(dvFile, 
                "    %0%3SetNext%1%2%3(_%3, %0%1GetFirst%2%3(%1));\n"
                "    %0%1SetFirst%2%3(%1, _%3);\n",
                dvPrefix, parentName, dvRelationshipGetChildLabel(relationship), childName,
                dvClassGetPrefix(theClass), dvClassGetPrefix(childClass));
            if(type == REL_TAIL_LINKED) {
                dvWrtemp(dvFile,
                    "    if(%0%1GetLast%2%3(%1) == %5%3Null) {\n"
                    "        %0%1SetLast%2%3(%1, _%3);\n"
                    "    }\n",
                    dvPrefix, parentName, dvRelationshipGetChildLabel(relationship), childName,
                    dvClassGetPrefix(theClass), dvClassGetPrefix(childClass));
           }
        }
    }
    if(dvRelationshipAccessParent(relationship)) {
        dvWrtemp(dvFile, "    %0%3Set%2%1(_%3, %1);\n", dvPrefix, parentName,
            dvRelationshipGetParentLabel(relationship), childName);
    }
    if(type == REL_HASHED) {
        if(dvRelationshipHashedByName(relationship)) {
            dvWrtemp(dvFile,
                "    if(%0%3Get%2Sym(_%3) != utSymNull) {\n"
                "        add%1%2%3ToHashTable(%1, _%3);\n"
                "    }\n",
                dvPrefix, parentName, dvRelationshipGetChildLabel(relationship), childName);
        } else {
            dvWrtemp(dvFile,
                "    add%1%2%3ToHashTable(%1, _%3);\n",
                dvPrefix, parentName, dvRelationshipGetChildLabel(relationship), childName);
        }
    }
    dvWrtemp(dvFile, "}\n\n");
}

/*--------------------------------------------------------------------------------------------------
  Write an append relationship routine for theClass.
--------------------------------------------------------------------------------------------------*/
static void writeClassAppendFunction(
    dvClass theClass,
    dvRelationship relationship)
{
    char *parentName = dvClassGetName(theClass);
    dvClass childClass = dvRelationshipGetChildClass(relationship);
    char *childName = dvClassGetName(childClass);
    dvRelationshipType type = dvRelationshipGetType(relationship);

    dvWrtemp(dvFile, 
        "/*----------------------------------------------------------------------------------------\n"
        "  Add the %2%3 to the end of the list on the %1.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "void %0%1Append%2%3(\n"
        "    %4%1 %1,\n"
        "    %5%3 _%3)\n"
        "{\n"
        "#if defined(DD_DEBUG)\n"
        "    if(%1 == %4%1Null) {\n"
        "        utExit(\"Non-existent %1\");\n"
        "    }\n"
        "    if(_%3 == %5%3Null) {\n"
        "        utExit(\"Non-existent %3\");\n"
        "    }\n",
        dvPrefix, parentName, dvRelationshipGetChildLabel(relationship), childName,
        dvClassGetPrefix(theClass), dvClassGetPrefix(childClass));
    if(dvRelationshipAccessParent(relationship)) {
        dvWrtemp(dvFile, 
            "    if(%0%2Get%4%1(_%2) != %3%1Null) {\n"
            "        utExit(\"Attempting to add %2 to %1 twice\");\n"
            "    }\n",
            dvPrefix, parentName, childName, dvClassGetPrefix(theClass),
            dvRelationshipGetParentLabel(relationship));
    }
    dvWrtemp(dvFile, "#endif\n");
    if(type == REL_DOUBLY_LINKED || type == REL_HASHED) {
        dvWrtemp(dvFile, 
            "    %0%3SetPrev%1%2%3(_%3, %0%1GetLast%2%3(%1));\n"
            "    if(%0%1GetLast%2%3(%1) != %5%3Null) {\n"
            "        %0%3SetNext%1%2%3(%0%1GetLast%2%3(%1), _%3);\n"
            "    }\n"
            "    %0%1SetLast%2%3(%1, _%3);\n"
            "    %0%3SetNext%1%2%3(_%3, %5%3Null);\n"
            "    if(%0%1GetFirst%2%3(%1) == %5%3Null) {\n"
            "        %0%1SetFirst%2%3(%1, _%3);\n"
            "    }\n",
            dvPrefix, parentName, dvRelationshipGetChildLabel(relationship), childName,
            dvClassGetPrefix(theClass), dvClassGetPrefix(childClass));
    } else {
        dvWrtemp(dvFile, 
            "    if(%0%1GetLast%2%3(%1) != %5%3Null) {\n"
            "        %0%3SetNext%1%2%3(%0%1GetLast%2%3(%1), _%3);\n"
            "    } else {\n"
            "        %0%1SetFirst%2%3(%1, _%3);\n"
            "    }\n"
            "    %0%1SetLast%2%3(%1, _%3);\n"
            "    %0%3SetNext%1%2%3(_%3, %5%3Null);\n",
            dvPrefix, parentName, dvRelationshipGetChildLabel(relationship), childName,
            dvClassGetPrefix(theClass), dvClassGetPrefix(childClass));
    }
    if(dvRelationshipAccessParent(relationship)) {
        dvWrtemp(dvFile, "    %0%3Set%2%1(_%3, %1);\n", dvPrefix, parentName,
            dvRelationshipGetParentLabel(relationship), childName);
    }
    if(type == REL_HASHED) {
        if(dvRelationshipHashedByName(relationship)) {
            dvWrtemp(dvFile,
                "    if(%0%3Get%2Sym(_%3) != utSymNull) {\n"
                "        add%1%2%3ToHashTable(%1, _%3);\n"
                "    }\n",
                dvPrefix, parentName, dvRelationshipGetChildLabel(relationship), childName);
        } else {
            dvWrtemp(dvFile,
                "    add%1%2%3ToHashTable(%1, _%3);\n",
                dvPrefix, parentName, dvRelationshipGetChildLabel(relationship), childName);
        }
    }
    dvWrtemp(dvFile, "}\n\n");
}

/*--------------------------------------------------------------------------------------------------
  Write an insert after relationship routine for theClass.
--------------------------------------------------------------------------------------------------*/
static void writeClassInsertAfter(
    dvClass theClass,
    dvRelationship relationship)
{
    char *parentName = dvClassGetName(theClass);
    dvClass childClass = dvRelationshipGetChildClass(relationship);
    char *childName = dvClassGetName(childClass);
    dvRelationshipType type = dvRelationshipGetType(relationship);
    
    dvWrtemp(dvFile, 
        "/*----------------------------------------------------------------------------------------\n"
        "  Insert the %2%3 to the %1 after the previous %2%3.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "void %0%1InsertAfter%2%3(\n"
        "    %4%1 %1,\n"
        "    %5%3 prev%3,\n"
        "    %5%3 _%3)\n"
        "{\n"
        "    %5%3 next%3 = %0%3GetNext%1%2%3(prev%3);\n"
        "\n"
        "#if defined(DD_DEBUG)\n"
        "    if(%1 == %4%1Null) {\n"
        "        utExit(\"Non-existent %1\");\n"
        "    }\n"
        "    if(_%3 == %5%3Null) {\n"
        "        utExit(\"Non-existent %3\");\n"
        "    }\n",
        dvPrefix, parentName, dvRelationshipGetChildLabel(relationship), childName,
        dvClassGetPrefix(theClass), dvClassGetPrefix(childClass));
    if(dvRelationshipAccessParent(relationship)) {
        dvWrtemp(dvFile, 
            "    if(%0%2Get%4%1(_%2) != %3%1Null) {\n"
            "        utExit(\"Attempting to add %2 to %1 twice\");\n"
            "    }\n",
            dvPrefix, parentName, childName, dvClassGetPrefix(theClass),
            dvRelationshipGetParentLabel(relationship));
    }
    dvWrtemp(dvFile, "#endif\n");
    dvWrtemp(dvFile, 
        "    %0%3SetNext%1%2%3(_%3, next%3);\n"
        "    %0%3SetNext%1%2%3(prev%3, _%3);\n",
        dvPrefix, parentName, dvRelationshipGetChildLabel(relationship), childName,
        dvClassGetPrefix(theClass), dvClassGetPrefix(childClass));
    if(type == REL_DOUBLY_LINKED || type == REL_HASHED) {
        dvWrtemp(dvFile, 
            "    %0%3SetPrev%1%2%3(_%3, prev%3);\n"
            "    if(next%3 != %5%3Null) {\n"
            "        %0%3SetPrev%1%2%3(next%3, _%3);\n"
            "    }\n",
            dvPrefix, parentName, dvRelationshipGetChildLabel(relationship), childName,
            dvClassGetPrefix(theClass), dvClassGetPrefix(childClass));
    }
    if(type == REL_TAIL_LINKED || type == REL_DOUBLY_LINKED || type == REL_HASHED) {
        dvWrtemp(dvFile,
            "    if(%0%1GetLast%2%3(%1) == prev%3) {\n"
            "        %0%1SetLast%2%3(%1, _%3);\n"
            "    }\n",
            dvPrefix, parentName, dvRelationshipGetChildLabel(relationship), childName);
    }
    if(dvRelationshipAccessParent(relationship)) {
        dvWrtemp(dvFile, "    %0%3Set%2%1(_%3, %1);\n", dvPrefix, parentName,
            dvRelationshipGetParentLabel(relationship), childName);
    }
    if(type == REL_HASHED) {
        if(dvRelationshipHashedByName(relationship)) {
            dvWrtemp(dvFile,
                "    if(%0%3Get%2Sym(_%3) != utSymNull) {\n"
                "        add%1%2%3ToHashTable(%1, _%3);\n"
                "    }\n",
                dvPrefix, parentName, dvRelationshipGetChildLabel(relationship), childName);
        } else {
            dvWrtemp(dvFile,
                "    add%1%2%3ToHashTable(%1, _%3);\n",
                dvPrefix, parentName, dvRelationshipGetChildLabel(relationship), childName);
        }
    }
    dvWrtemp(dvFile, "}\n\n");
}

/*--------------------------------------------------------------------------------------------------
  Write a delete relationship routine for theClass.
--------------------------------------------------------------------------------------------------*/
static void writeClassRemoveFunction(
    dvClass theClass,
    dvRelationship relationship)
{
    char *parentName = dvClassGetName(theClass);
    dvClass childClass = dvRelationshipGetChildClass(relationship);
    dvRelationshipType type = dvRelationshipGetType(relationship);
    char *childName = dvClassGetName(childClass);
    char *childLabel = dvRelationshipGetChildLabel(relationship);
    
    dvWrtemp(dvFile, 
        "/*----------------------------------------------------------------------------------------\n"
        " Remove the %2%3 from the %1.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "void %0%1Remove%2%3(\n"
        "    %4%1 %1,\n"
        "    %5%3 _%3)\n"
        "{\n",
        dvPrefix, parentName, childLabel, childName, dvClassGetPrefix(theClass),
        dvClassGetPrefix(childClass));
    if(type != REL_HASHED || !dvRelationshipUnordered(relationship)) {
        dvWrtemp(dvFile, 
            "    %5%3 p%3, n%3;\n"
            "\n",
            dvPrefix, parentName, childLabel, childName, dvClassGetPrefix(theClass),
            dvClassGetPrefix(childClass));
    }
    dvWrtemp(dvFile,
        "#if defined(DD_DEBUG)\n"
        "    if(_%3 == %5%3Null) {\n"
        "        utExit(\"Non-existent %3\");\n"
        "    }\n", dvPrefix, parentName, childLabel, childName,
        dvClassGetPrefix(theClass), dvClassGetPrefix(childClass));
    if(dvRelationshipAccessParent(relationship)) {
        dvWrtemp(dvFile, 
            "    if(%0%3Get%1%2(_%3) != %4%2Null && %0%3Get%1%2(_%3) != %2) {\n"
            "        utExit(\"Delete %3 from non-owning %2\");\n"
            "    }\n",
            dvPrefix,  dvRelationshipGetParentLabel(relationship), parentName, childName,
            dvClassGetPrefix(theClass));
    }
    dvWrtemp(dvFile, "#endif\n");
    if(type != REL_HASHED || !dvRelationshipUnordered(relationship)) {
        if(type == REL_DOUBLY_LINKED || type == REL_HASHED) {
            dvWrtemp(dvFile, 
                "    n%3 = %0%3GetNext%1%2%3(_%3);\n"
                "    p%3 = %0%3GetPrev%1%2%3(_%3);\n"
                "    if(p%3 != %5%3Null) {\n"
                "        %0%3SetNext%1%2%3(p%3, n%3);\n"
                "    } else if(%0%1GetFirst%2%3(%1) == _%3) {\n"
                "        %0%1SetFirst%2%3(%1, n%3);\n"
                "    }\n"
                "    if(n%3 != %5%3Null) {\n"
                "        %0%3SetPrev%1%2%3(n%3, p%3);\n"
                "    } else if(%0%1GetLast%2%3(%1) == _%3) {\n"
                "        %0%1SetLast%2%3(%1, p%3);\n"
                "    }\n"
                "    %0%3SetNext%1%2%3(_%3, %5%3Null);\n"
                "    %0%3SetPrev%1%2%3(_%3, %5%3Null);\n",
                dvPrefix, parentName, childLabel, childName,
                dvClassGetPrefix(theClass), dvClassGetPrefix(childClass));
        } else {
            dvWrtemp(dvFile, 
                "    p%3 = %5%3Null;\n"
                "    for(n%3 = %0%1GetFirst%2%3(%1); n%3 != %5%3Null && n%3 != _%3;\n"
                "            n%3 = %0%3GetNext%1%2%3(n%3)) {\n"
                "        p%3 = n%3;\n"
                "    }\n"
                "    if(p%3 != %5%3Null) {\n"
                "        %0%3SetNext%1%2%3(p%3, %0%3GetNext%1%2%3(_%3));\n"
                "    } else {\n"
                "        %0%1SetFirst%2%3(%1, %0%3GetNext%1%2%3(_%3));\n"
                "    }\n"
                "    %0%3SetNext%1%2%3(_%3, %5%3Null);\n",
                dvPrefix, parentName, childLabel, childName,
                dvClassGetPrefix(theClass), dvClassGetPrefix(childClass));
            if(type == REL_TAIL_LINKED) {
                dvWrtemp(dvFile, 
                    "    if(%0%1GetLast%2%3(%1) == _%3) {\n"
                    "        %0%1SetLast%2%3(%1, p%3);\n"
                    "    }\n",
                    dvPrefix, parentName, childLabel, childName);
            }
        }
    }
    if(dvRelationshipAccessParent(relationship)) {
        dvWrtemp(dvFile,
            "    %0%3Set%1%2(_%3, %4%2Null);\n",
            dvPrefix, dvRelationshipGetParentLabel(relationship), parentName, childName,
            dvClassGetPrefix(theClass));
    }
    if(type == REL_HASHED) {
        if(dvRelationshipHashedByName(relationship)) {
            dvWrtemp(dvFile,
                "    if(%0%3Get%2Sym(_%3) != utSymNull) {\n"
                "        remove%1%2%3FromHashTable(%1, _%3);\n"
                "    }\n",
                dvPrefix, parentName, dvRelationshipGetChildLabel(relationship), childName);
        } else {
            dvWrtemp(dvFile,
                "    remove%1%2%3FromHashTable(%1, _%3);\n",
                dvPrefix, parentName, dvRelationshipGetChildLabel(relationship), childName);
        }
    }
    dvWrtemp(dvFile, "}\n\n");
}

/*--------------------------------------------------------------------------------------------------
  Write an array insert relationship routine for theClass.
--------------------------------------------------------------------------------------------------*/
static void writeClassArrayInsertFunction(
    dvClass theClass,
    dvRelationship relationship)
{
    char *parentName = dvClassGetName(theClass);
    dvClass childClass = dvRelationshipGetChildClass(relationship);
    char *childName = dvClassGetName(childClass);

    dvWrtemp(dvFile, 
        "/*----------------------------------------------------------------------------------------\n"
        "  Add the indexed %2%3 to the %1.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "void %0%1Insert%2%3(\n"
        "    %4%1 %1,\n"
        "    uint32 x,\n"
        "    %5%3 _%3)\n"
        "{\n"
        "#if defined(DD_DEBUG)\n"
        "    if(%1 == %4%1Null) {\n"
        "        utExit(\"Non existent %1\");\n"
        "    }\n",
        dvPrefix, parentName, dvRelationshipGetChildLabel(relationship), childName,
        dvClassGetPrefix(theClass), dvClassGetPrefix(childClass));
    if(dvRelationshipAccessParent(relationship)) {
        dvWrtemp(dvFile, 
            "    if(%0%2Get%4%1(_%2) != %3%1Null) {\n"
            "        utExit(\"Attempting to add %2 to %1 twice\");\n"
            "    }\n",
            dvPrefix, parentName, childName, dvClassGetPrefix(theClass),
            dvRelationshipGetParentLabel(relationship));
    }
    dvWrtemp(dvFile, 
        "#endif\n"
        "    %0%1Seti%2%3(%1, x, _%3);\n"
        "    %0%1SetUsed%2%3(%1, utMax(%0%1GetUsed%2%3(%1), x + 1));\n",
        dvPrefix, parentName, dvRelationshipGetChildLabel(relationship), childName);
    if(dvRelationshipAccessParent(relationship)) {
        dvWrtemp(dvFile, 
            "    %0%3Set%1%4Index(_%3, x);\n"
            "    %0%3Set%2%1(_%3, %1);\n",
            dvPrefix, parentName, dvRelationshipGetParentLabel(relationship), childName,
            dvRelationshipGetChildLabel(relationship));
    }
    dvWrtemp(dvFile, "}\n\n");
}

/*--------------------------------------------------------------------------------------------------
  Write an array append relationship routine for theClass.
--------------------------------------------------------------------------------------------------*/
static void writeClassArrayAppendFunction(
    dvClass theClass,
    dvRelationship relationship,
    bool isHeap)
{
    char *parentName = dvClassGetName(theClass);
    dvClass childClass = dvRelationshipGetChildClass(relationship);
    char *childName = dvClassGetName(childClass);

    if(!isHeap) {
        dvWrtemp(dvFile, 
            "/*----------------------------------------------------------------------------------------\n"
            "  Add the %2%3 to the end of the %1%2%3% array.\n"
            "----------------------------------------------------------------------------------------*/\n"
            "void %0%1Append%2%3(\n",
            dvPrefix, parentName, dvRelationshipGetChildLabel(relationship), childName);
    } else {
        dvWrtemp(dvFile, 
            "/*----------------------------------------------------------------------------------------\n"
            "  Push the %2%3 onto the %1%2%3% heap.\n"
            "----------------------------------------------------------------------------------------*/\n"
            "void %0%1Push%2%3(\n",
            dvPrefix, parentName, dvRelationshipGetChildLabel(relationship), childName);
    }
    dvWrtemp(dvFile, 
        "    %4%1 %1,\n"
        "    %5%3 _%3)\n"
        "{\n"
        "    uint32 used%2%3 = %0%1GetUsed%2%3(%1);\n"
        "\n"
        "#if defined(DD_DEBUG)\n"
        "    if(%1 == %4%1Null) {\n"
        "        utExit(\"Non existent %1\");\n"
        "    }\n"
        "#endif\n"
        "    if(used%2%3 >= %0%1GetNum%2%3(%1)) {\n"
        "        %0%1Resize%2%3s(%1, used%2%3 + (used%2%3 << 1) + 1);\n"
        "    }\n"
        "    %0%1Seti%2%3(%1, used%2%3, _%3);\n"
        "    %0%1SetUsed%2%3(%1, used%2%3 + 1);\n",
        dvPrefix, parentName, dvRelationshipGetChildLabel(relationship), childName,
        dvClassGetPrefix(theClass), dvClassGetPrefix(childClass));
    if(dvRelationshipAccessParent(relationship)) {
        dvWrtemp(dvFile,
            "    %0%3Set%1%4Index(_%3, used%4%3);\n"
            "    %0%3Set%2%1(_%3, %1);\n",
            dvPrefix, parentName, dvRelationshipGetParentLabel(relationship), childName,
            dvRelationshipGetChildLabel(relationship));
    }
    if(isHeap) {
        dvWrtemp(dvFile,
            "    %l1HeapUp%2%3(%1, used%2%3);\n",
            dvPrefix, parentName, dvRelationshipGetChildLabel(relationship), childName);
    }
    dvWrtemp(dvFile, "}\n\n");
}

/*--------------------------------------------------------------------------------------------------
  Write a delete relationship routine for array relationship.
--------------------------------------------------------------------------------------------------*/
static void writeClassRemoveArray(
    dvClass theClass,
    dvRelationship relationship,
    bool isHeap)
{
    char *parentName = dvClassGetName(theClass);
    dvClass childClass = dvRelationshipGetChildClass(relationship);
    char *childName = dvClassGetName(childClass);
    char *childLabel = dvRelationshipGetChildLabel(relationship);
    
    dvWrtemp(dvFile, 
        "/*----------------------------------------------------------------------------------------\n"
        "  Remove the %2%3 from the %1.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "void %0%1Remove%2%3(\n"
        "    %4%1 %1,\n"
        "    %5%3 _%3)\n"
        "{\n",
        dvPrefix, parentName, childLabel, childName, dvClassGetPrefix(theClass),
        dvClassGetPrefix(childClass));
    dvWrtemp(dvFile,
        "#if defined(DD_DEBUG)\n"
        "    if(_%3 == %5%3Null) {\n"
        "        utExit(\"Non-existent %3\");\n"
        "    }\n",
        dvPrefix, parentName, childLabel, childName, dvClassGetPrefix(theClass),
        dvClassGetPrefix(childClass));
    if(dvRelationshipAccessParent(relationship)) {
        dvWrtemp(dvFile, 
            "    if(%0%3Get%1%2(_%3) != %4%2Null && %0%3Get%1%2(_%3) != %2) {\n"
            "        utExit(\"Delete %3 from non-owning %2\");\n"
            "    }\n",
            dvPrefix,  dvRelationshipGetParentLabel(relationship), parentName, childName,
            dvClassGetPrefix(theClass));
    }
    if(!isHeap) {
        dvWrtemp(dvFile, 
            "#endif\n"
            "    %0%1Seti%2%3(%1, %0%3Get%1%2Index(_%3), %5%3Null);\n"
            "    %0%3Set%1%2Index(_%3, UINT32_MAX);\n",
            dvPrefix, parentName, dvRelationshipGetChildLabel(relationship), childName,
            dvClassGetPrefix(theClass), dvClassGetPrefix(childClass));
        if(dvRelationshipAccessParent(relationship)) {
            dvWrtemp(dvFile,
                "    %0%1Set%2%3(_%1, %4%3Null);\n",
                dvPrefix, childName, dvRelationshipGetParentLabel(relationship), parentName,
                dvClassGetPrefix(theClass));
        }
    } else {
        dvWrtemp(dvFile, 
            "#endif\n"
            "    %l1Pop%2%3(%1, %0%3Get%1%2Index(_%3));\n",
            dvPrefix, parentName, dvRelationshipGetChildLabel(relationship), childName,
            dvClassGetPrefix(theClass), dvClassGetPrefix(childClass));
    }
    dvWrtemp(dvFile, "}\n\n");
}

/*--------------------------------------------------------------------------------------------------
  Write code to initialize property array values.
--------------------------------------------------------------------------------------------------*/
static void writePropertyInits(
    dvProperty prop,
    char *firstIndex,
    char *numValues,
    char *indent)
{
    dvClass theClass = dvPropertyGetClass(prop);
    dvUnion theUnion = dvPropertyGetUnion(prop);
    dvCache cache = dvPropertyGetCache(prop);
    char *initValue = dvPropertyFindInitializer(prop);
    char *name = dvClassGetName(theClass);
    char *propName = dvPropertyGetName(prop);
    char *fieldName;

    if(dvPropertyView(prop)) {
        dvWrtemp(dvFile, "%1    /* property initialisation for %0 is user provided */\n",
            propName, indent);
        return;
    }

    if(strcmp(initValue, "0") || cache != dvCacheNull) {
        if(theUnion != dvUnionNull) {
            fieldName = utSprintf("%s[xValue].%s", dvUnionGetFieldName(theUnion), propName);
        } else if(cache != dvCacheNull) {
            fieldName = utSprintf("%s[xValue].%s", dvCacheGetFieldName(cache), propName);
        } else {
            fieldName = utSprintf("%s[xValue]", propName);
        }
        dvWrtemp(dvFile,
            "%6    {\n"
            "%6        uint32 xValue;\n"
            "%6        for(xValue = (uint32)(%2); xValue < %2 + %3; xValue++) {\n"
            "%6            %0%1s.%5 = %4;\n"
            "%6        }\n"
            "%6    }\n",
            dvPrefix, name, firstIndex, numValues, initValue, fieldName, indent);
    } else {
        dvWrtemp(dvFile,
            "%7    memset(%0%1s.%2 + %5, 0, ((%6%4)*sizeof(%3));\n",
            dvPrefix, name, theUnion == dvUnionNull? propName : dvUnionGetFieldName(theUnion),
            theUnion == dvUnionNull? dvPropertyGetTypeName(prop) : dvUnionGetTypeName(theUnion),
            dvPropertyGetType(prop) == PROP_BIT? " + 7) >> 3" : ")", firstIndex, numValues, indent);
    }
}

/*--------------------------------------------------------------------------------------------------
  Write the allocation for the property fields of theClass.
--------------------------------------------------------------------------------------------------*/
static void writeClassPropFieldAllocs(
    dvClass theClass)
{
    dvUnion theUnion;
    dvCache cache;
    dvProperty prop;
    char *name = dvClassGetName(theClass);

    dvForeachClassProperty(theClass, prop) {
        if(dvPropertyGetUnion(prop) == dvUnionNull && dvPropertyGetCache(prop) == dvCacheNull &&
                !dvPropertySparse(prop)) {
            if(dvPropertyView(prop)) {
                dvWrtemp(dvFile, "    /* allocation for %0 is user provided */\n",
                    dvPropertyGetName(prop));
            }
            else if(!dvPropertyArray(prop)) {
                dvWrtemp(dvFile, 
                    "    %0%1s.%2 = utNewAInitFirst(%4, (%3Allocated%1()%5);\n",
                    dvPrefix, name, dvPropertyGetName(prop), dvClassGetPrefix(theClass),
                    dvPropertyGetTypeName(prop),
                    dvPropertyGetType(prop) == PROP_BIT? " + 7) >> 3" : ")");
                if(dvClassGetBaseClass(theClass) != dvClassNull) {
                    writePropertyInits(prop, "0", utSprintf("%sAllocated%s()", dvClassGetPrefix(theClass), name), "");
                }
            } else if(!dvPropertyFixedSize(prop)) {
                dvWrtemp(dvFile, 
                    "    %0SetUsed%1%2(0);\n"
                    "    %0SetAllocated%1%2(2);\n"
                    "    %0SetFree%1%2(0);\n"
                    "    %0%1s.%2 = utNewAInitFirst(%4, %0Allocated%1%2());\n",
                    dvPrefix, name, dvPropertyGetName(prop), dvClassGetPrefix(theClass),
                    dvPropertyGetTypeName(prop));
            } else {
                dvWrtemp(dvFile, 
                    "    %0%1s.%2 = utNewAInitFirst(%4, %3Allocated%1()*(%5));\n",
                    dvPrefix, name, dvPropertyGetName(prop), dvClassGetPrefix(theClass),
                    dvPropertyGetTypeName(prop), dvPropertyGetIndex(prop));
                if(dvClassGetBaseClass(theClass) != dvClassNull) {
                    writePropertyInits(prop, "0", utSprintf("%sAllocated%s()*(%s)",
                        dvClassGetPrefix(theClass), name, dvPropertyGetIndex(prop)), "");
                }
            }
        }
    } dvEndClassProperty;
    dvForeachClassUnion(theClass, theUnion) {
        dvWrtemp(dvFile, 
            "    %0%1s.%3 = utNewAInitFirst(%4, %2Allocated%1());\n",
            dvPrefix, dvClassGetName(theClass), dvClassGetPrefix(theClass),
            dvUnionGetFieldName(theUnion), dvUnionGetTypeName(theUnion));
        if(dvClassGetBaseClass(theClass) != dvClassNull) {
            writePropertyInits(dvUnionGetFirstProperty(theUnion), "0",
                utSprintf("%sAllocated%s()", dvClassGetPrefix(theClass), name), "");
        }
    } dvEndClassUnion;
    dvForeachClassCache(theClass, cache) {
        dvWrtemp(dvFile, 
            "    %0%1s.%3 = utNewAInitFirst(%4, %2Allocated%1());\n",
            dvPrefix, dvClassGetName(theClass), dvClassGetPrefix(theClass),
            dvCacheGetFieldName(cache), dvCacheGetTypeName(cache));
        if(dvClassGetBaseClass(theClass) != dvClassNull) {
            dvForeachCacheProperty(cache, prop) {
                writePropertyInits(prop, "0", utSprintf("%sAllocated%s()", dvClassGetPrefix(theClass), name), "");
            } dvEndCacheProperty;
        }
    } dvEndClassUnion;
}

/*--------------------------------------------------------------------------------------------------
  Write the reallocation for the property fields of theClass.
--------------------------------------------------------------------------------------------------*/
static void writeClassPropFieldReallocs(
    dvClass theClass)
{
    dvUnion theUnion;
    dvCache cache;
    dvProperty prop;
    char *name = dvClassGetName(theClass);
    char *shift, *multiplier;

    dvForeachClassProperty(theClass, prop) {
        if(dvPropertyView(prop)) {
            dvWrtemp(dvFile, "    /* reallocation for %0 is user provided */\n",
                dvPropertyGetName(prop));
        }
        else if((!dvPropertyArray(prop) || dvPropertyFixedSize(prop)) && dvPropertyGetUnion(prop) == dvUnionNull &&
                dvPropertyGetCache(prop) == dvCacheNull && !dvPropertySparse(prop)) {
            shift = dvPropertyGetType(prop) == PROP_BIT? " + 7) >> 3" : ")";
            multiplier = dvPropertyFixedSize(prop)? utSprintf("*(%s)", dvPropertyGetIndex(prop)) : "";
            if(dvClassUndo(theClass)) {
                dvWrtemp(dvFile, 
                    "    utRecordResize(%0ModuleID, %3, (%2Allocated%1()%4%5, true);\n",
                    dvPrefix, name, dvClassGetPrefix(theClass),
                    utSprintf("%u", dvPropertyGetFieldNumber(prop)), shift, multiplier);
            }
            dvWrtemp(dvFile, 
                "    utResizeArray(%0%1s.%2, (newSize%4%5);\n",
                dvPrefix, name, dvPropertyGetName(prop), dvClassGetPrefix(theClass), shift, multiplier);
            if(dvClassRedo(theClass)) {
                dvWrtemp(dvFile, 
                    "    utRecordResize(%0ModuleID, %3, (newSize%4%5, false);\n",
                    dvPrefix, name, dvClassGetPrefix(theClass),
                    utSprintf("%u", dvPropertyGetFieldNumber(prop)), shift, multiplier);
            }
        }
    } dvEndClassProperty;
    dvForeachClassUnion(theClass, theUnion) {
        if(dvClassUndo(theClass)) {
            dvWrtemp(dvFile, 
                "    utRecordResize(%0ModuleID, %3, %2Allocated%1(), true);\n",
                dvPrefix, dvClassGetName(theClass), dvClassGetPrefix(theClass),
                utSprintf("%u", dvUnionGetFieldNumber(theUnion)));
        }
        dvWrtemp(dvFile, 
            "    utResizeArray(%0%1s.%2, newSize);\n",
            dvPrefix, dvClassGetName(theClass), dvUnionGetFieldName(theUnion),
            dvClassGetPrefix(theClass));
        if(dvClassRedo(theClass)) {
            dvWrtemp(dvFile, 
                "    utRecordResize(%0ModuleID, %3, newSize, false);\n",
                dvPrefix, dvClassGetName(theClass), dvClassGetPrefix(theClass),
                utSprintf("%u", dvPropertyGetFieldNumber(prop)));
        }
    } dvEndClassUnion;
    if(dvClassRedo(theClass)) {
        return;
    }
    dvForeachClassCache(theClass, cache) {
        dvWrtemp(dvFile, 
            "    utResizeArray(%0%1s.%2, newSize);\n",
            dvPrefix, dvClassGetName(theClass), dvCacheGetFieldName(cache), dvClassGetPrefix(theClass));
    } dvEndClassCache;
}

/*--------------------------------------------------------------------------------------------------
  Write the free statements for the property fields of theClass.
--------------------------------------------------------------------------------------------------*/
static void writeClassPropFieldFrees(
    dvClass theClass)
{
    dvUnion theUnion;
    dvCache cache;
    dvProperty prop;
    char *name = dvClassGetName(theClass);

    dvForeachClassProperty(theClass, prop) {
        if(dvPropertyView(prop)) {
            dvWrtemp(dvFile, "    /* free for %0 is user provided */\n",
                dvPropertyGetName(prop));
        }
        else if(dvPropertyGetUnion(prop) == dvUnionNull && dvPropertyGetCache(prop) == dvCacheNull &&
                !dvPropertySparse(prop)) {
            dvWrtemp(dvFile, 
                "    utFree(%0%1s.%2);\n",
                dvPrefix, name, dvPropertyGetName(prop));
        }
    } dvEndClassProperty;
    dvForeachClassUnion(theClass, theUnion) {
        dvWrtemp(dvFile, 
            "    utFree(%0%1s.%2);\n",
            dvPrefix, dvClassGetName(theClass), dvUnionGetFieldName(theUnion));
    } dvEndClassUnion;
    dvForeachClassCache(theClass, cache) {
        dvWrtemp(dvFile, 
            "    utFree(%0%1s.%2);\n",
            dvPrefix, dvClassGetName(theClass), dvCacheGetFieldName(cache));
    } dvEndClassCache;
}

/*--------------------------------------------------------------------------------------------------
  Write out the allocation stuff for a class.
--------------------------------------------------------------------------------------------------*/
static void writeClassAllocs(
    dvClass theClass)
{
    char *name = dvClassGetName(theClass);

    dvWrtemp(dvFile, 
        "/*----------------------------------------------------------------------------------------\n"
        "  Allocate the field arrays of %1.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "static void alloc%1s(void)\n"
        "{\n", dvPrefix, name);
    if(dvClassGetBaseClass(theClass) == dvClassNull) {
        dvWrtemp(dvFile,
            "    %0SetAllocated%1(2);\n"
            "    %0SetUsed%1(1);\n",
            dvPrefix, name);
        if(dvClassGetMemoryStyle(theClass) == MEM_FREE_LIST) {
            dvWrtemp(dvFile,
                "    %0SetFirstFree%1(%0%1Null);\n",
                dvPrefix, name);
        }
    }
    writeClassPropFieldAllocs(theClass);
    if(dvClassGetBaseClass(theClass) != dvClassNull) {
        dvWrtemp(dvFile, 
            "    %0SetAllocated%1(%2Allocated%1());\n"
            "    %0%1ConstructorCallback = %2%1GetConstructorCallback();\n"
            "    %2%1SetConstructorCallback(init%1);\n",
            dvPrefix, name, dvClassGetPrefix(theClass));
        if(dvClassGetMemoryStyle(theClass) == MEM_FREE_LIST) {
            dvWrtemp(dvFile, 
                "    %0%1DestructorCallback = %2%1GetDestructorCallback();\n"
                "    %2%1SetDestructorCallback(destroy%1);\n",
                dvPrefix, name, dvClassGetPrefix(theClass));
        }
    }
    dvWrtemp(dvFile, "}\n\n");
}

/*--------------------------------------------------------------------------------------------------
  Write out the reallocation stuff for a class.
--------------------------------------------------------------------------------------------------*/
static void writeClassReallocs(
    dvClass theClass)
{
    dvWrtemp(dvFile, 
        "/*----------------------------------------------------------------------------------------\n"
        "  Realloc the arrays of properties for class %0.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "static void realloc%0s(\n"
        "    %1 newSize)\n"
        "{\n",
        dvClassGetName(theClass), dvClassGetReferenceTypeName(theClass));
    writeClassPropFieldReallocs(theClass);
    dvWrtemp(dvFile,
        "    %0SetAllocated%1(newSize);\n"
        "}\n\n",
        dvPrefix, dvClassGetName(theClass));
}

/*--------------------------------------------------------------------------------------------------
  Write the database stop routine.
--------------------------------------------------------------------------------------------------*/
static void writeStop(
    dvModule module)
{
    dvClass theClass;

    dvWrtemp(dvFile, 
        "/*----------------------------------------------------------------------------------------\n"
        "  Free memory used by the %0 database.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "void %0DatabaseStop(void)\n{\n",
        dvPrefix);
    dvForeachModuleClass(module, theClass) {
        writeClassPropFieldFrees(theClass);
        if(dvClassGetBaseClass(theClass) != dvClassNull) {
            dvWrtemp(dvFile,
                "    %2%1SetConstructorCallback(%0%1ConstructorCallback);\n"
                "    %0%1ConstructorCallback = NULL;\n",
                dvPrefix, dvClassGetName(theClass), dvClassGetPrefix(theClass));
            if(dvClassGetMemoryStyle(theClass) == MEM_FREE_LIST) {
                dvWrtemp(dvFile,
                    "    if(%2%1GetDestructorCallback() != destroy%1) {\n"
                    "        utExit(\"%0DatabaseClose called out of order\");\n"
                    "    }\n"
                    "    %2%1SetDestructorCallback(%0%1DestructorCallback);\n"
                    "    %0%1DestructorCallback = NULL;\n",
                    dvPrefix, dvClassGetName(theClass), dvClassGetPrefix(theClass));
            }
        }
    } dvEndModuleClass;
    dvWrtemp(dvFile,
        "    utUnregisterModule(%0ModuleID);\n"
        "}\n\n",
        dvPrefix);
}

/*--------------------------------------------------------------------------------------------------
  Write code to register this module's fields so that changes can be mirrored to disk.
--------------------------------------------------------------------------------------------------*/
static void writeRegisterFields(
    dvModule module)
{
    dvClass theClass, baseClass;
    dvUnion theUnion;
    dvCase theCase;
    dvEnum theEnum;
    dvEntry entry;
    dvProperty property;

    dvWrtemp(dvFile,
        "    %0ModuleID = utRegisterModule(\"%0\", %2, %0Hash(), %1, sizeof(struct %0RootType_),\n"
        "        &%0RootData, %0DatabaseStart, %0DatabaseStop);\n",
        dvPrefix, utSprintf("%u, %u, %u", dvModuleGetNumClasses(module), dvModuleGetNumFields(module),
        dvModuleGetNumEnums(module)), dvModulePersistent(module)? "true" : "false");
    dvForeachModuleEnum(module, theEnum) {
        dvWrtemp(dvFile,
            "    utRegisterEnum(\"%0\", %1);\n",
            dvEnumGetName(theEnum), utSprintf("%u", dvEnumGetNumEntries(theEnum)));
        dvForeachEnumEntry(theEnum, entry) {
            dvWrtemp(dvFile,
                "    utRegisterEntry(\"%0\", %1);\n",
                dvEntryGetName(entry), utSprintf("%u", dvEntryGetValue(entry)));
        } dvEndEnumEntry;
    } dvEndModuleEnum;
    dvForeachModuleClass(module, theClass) {
        baseClass = dvClassGetBaseClass(theClass);
        dvWrtemp(dvFile,
            "    utRegisterClass(\"%1\", %3, &%2RootData.used%1, &%2RootData.allocated%1,\n",
            dvModuleGetPrefix(module), dvClassGetName(theClass), dvClassGetPrefix(theClass),
            utSprintf("%u", dvClassGetNumFields(theClass)));
        if(dvClassGetMemoryStyle(theClass) == MEM_FREE_LIST) {
            dvWrtemp(dvFile,
                "        &%1RootData.firstFree%0, ",
                dvClassGetName(theClass), dvClassGetPrefix(theClass));
        } else {
            dvWrtemp(dvFile, "        NULL, ");
        }
        property = dvClassGetFreeListProperty(theClass);
        dvWrtemp(dvFile,
            "%0, %1, ",
            utSprintf("%u", property != dvPropertyNull? dvPropertyGetFieldNumber(property) : UINT16_MAX),
            utSprintf("%u", dvClassGetReferenceSize(theClass) >> 3));
        if(baseClass == dvClassNull) {
            dvWrtemp(dvFile, "alloc%0, ", dvClassGetName(theClass));
            if(dvClassGetMemoryStyle(theClass) != MEM_CREATE_ONLY) {
                dvWrtemp(dvFile, "destroy%0);\n", dvClassGetName(theClass));
            } else {
                dvWrtemp(dvFile, "NULL);\n");
            }
        } else {
            dvWrtemp(dvFile, "NULL, NULL);\n");
            dvWrtemp(dvFile, "    utRegisterBaseClass(\"%0\", %1);\n",
                dvModuleGetPrefix(dvClassGetModule(baseClass)), utSprintf("%u", dvClassGetNumber(baseClass)));
        }
        dvForeachClassProperty(theClass, property) {
            if(dvPropertyView(property)) {
                dvWrtemp(dvFile,
                    "    /* %0.%1 not managed by this database */\n",
                    dvClassGetName(theClass),
                    dvPropertyGetName(property));
            }
            else if(dvPropertyGetUnion(property) == dvUnionNull && dvPropertyGetCache(property) == dvCacheNull &&
                    !dvPropertySparse(property)) {
                dvWrtemp(dvFile,
                    "    utRegisterField(\"%2\", &%0%1s.%2, sizeof(%3), %4,",
                    dvModuleGetPrefix(module), dvClassGetName(theClass),
                    dvPropertyGetName(property), dvPropertyGetTypeName(property),
                    dvPropertyGetFieldTypeName(property));
                if(dvPropertyGetType(property) == PROP_POINTER) {
                    dvWrtemp(dvFile, " \"%0\");\n", dvClassGetName(dvPropertyGetClassProp(property)));
                } else if(dvPropertyGetType(property) == PROP_ENUM) {
                    dvWrtemp(dvFile, " \"%0\");\n", dvEnumGetName(dvPropertyGetEnumProp(property)));
                } else {
                    dvWrtemp(dvFile, " NULL);\n");
                }
                if(dvPropertyArray(property)) {
                    if(!dvPropertyFixedSize(property)) {
                        dvWrtemp(dvFile,
                            "    utRegisterArray(&%0RootData.used%1%2, &%0RootData.allocated%1%2,\n"
                            "        get%1%2s, alloc%1%2s, %0Compact%1%2s);\n",
                            dvPrefix, dvClassGetName(theClass), dvPropertyGetName(property));
                    } else {
                        dvWrtemp(dvFile,
                            "    utRegisterFixedArray((%3), get%1%2s);\n",
                            dvPrefix, dvClassGetName(theClass), dvPropertyGetName(property),
                            dvPropertyGetIndex(property));
                    }
                }
                if(dvPropertyHidden(property)) {
                    dvWrtemp(dvFile, "    utSetFieldHidden();\n");
                }
            }
        } dvEndClassProperty;
        dvForeachClassUnion(theClass, theUnion) {
            property = dvUnionGetTypeProperty(theUnion);
            dvWrtemp(dvFile,
                "    utRegisterField(\"%2\", &%0%1s.%2, sizeof(%3), UT_UNION, \"%4\");\n"
                "    utRegisterUnion(\"%4\", %5);\n",
                dvModuleGetPrefix(module), dvClassGetName(theClass), dvUnionGetFieldName(theUnion),
                dvUnionGetTypeName(theUnion), dvPropertyGetName(property),
                utSprintf("%u", dvUnionGetNumCases(theUnion)));
            dvForeachUnionProperty(theUnion, property) {
                dvForeachPropertyCase(property, theCase) {
                    entry = dvCaseGetEntry(theCase);
                    dvWrtemp(dvFile,
                        "    utRegisterUnionCase(%0, %1, sizeof(%2));\n",
                        utSprintf("%u", dvEntryGetValue(entry)), dvPropertyGetFieldTypeName(property),
                        dvPropertyGetTypeName(property));
                } dvEndPropertyCase;
            } dvEndUnionProperty;
        } dvEndClassUnion;
    } dvEndModuleClass;
    //temp register caches
}

/*--------------------------------------------------------------------------------------------------
  Write the database initialization routine.
--------------------------------------------------------------------------------------------------*/
static void writeStart(
    dvModule module)
{
    dvClass theClass;

    dvWrtemp(dvFile, 
        "/*----------------------------------------------------------------------------------------\n"
        "  Allocate memory used by the %0 database.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "void %0DatabaseStart(void)\n{\n"
        "    if(!utInitialized()) {\n"
        "        utStart();\n"
        "    }\n"
        "    %0RootData.hash = %1;\n",
        dvPrefix, utSprintf("0x%x", dvComputeDatabaseHash()));
    writeRegisterFields(module);
    dvForeachModuleClass(module, theClass) {
        dvWrtemp(dvFile, "    alloc%0s();\n", dvClassGetName(theClass));
    } dvEndModuleClass;
    if(dvModuleHasSparseData(module)) {
        dvWrtemp(dvFile, "    (void)%0DatadrawRootAlloc();\n", dvPrefix);
    }
    dvWrtemp(dvFile, "}\n\n");
}

/*--------------------------------------------------------------------------------------------------
  Write the function to allocate more objects of the class.
--------------------------------------------------------------------------------------------------*/
static void writeClassAllocateMore(
    dvClass theClass)
{
    dvWrtemp(dvFile, 
        "/*----------------------------------------------------------------------------------------\n"
        "  Allocate more %1s.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "void %0%1AllocMore(void)\n"
        "{\n"
        "    realloc%1s((%3)(%2Allocated%1() + (%2Allocated%1() >> 1)));\n"
        "}\n\n",
        dvPrefix, dvClassGetName(theClass), dvClassGetPrefix(theClass),
                    dvClassGetReferenceTypeName(theClass));
}

/*--------------------------------------------------------------------------------------------------
  Write the init function for local fields of an object.
--------------------------------------------------------------------------------------------------*/
static void writeExtendedClassInitFunc(
    dvClass theClass)
{
    dvWrtemp(dvFile, 
        "static void realloc%1s(%3 newSize);\n"
        "/*----------------------------------------------------------------------------------------\n"
        "  Initialize a new %1.  This is a constructor callback from the base class.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "static void init%1(\n"
        "    %2%1 %1)\n"
        "{\n"
        "    if(%0Allocated%1() != %2Allocated%1()) {\n"
        "        realloc%1s(%2Allocated%1());\n"
        "    }\n"
        "    %0%1Init(%1);\n"
        "    if(%0%1ConstructorCallback != NULL) {\n"
        "        %0%1ConstructorCallback(%1);\n"
        "    }\n"
        "}\n\n",
        dvPrefix, dvClassGetName(theClass), dvClassGetPrefix(theClass), dvClassGetReferenceTypeName(theClass));
}

/*--------------------------------------------------------------------------------------------------
  Write simple constructor/destructor wrapper functions for the database manager's use.
--------------------------------------------------------------------------------------------------*/
static void writeConstructorDestructorWrappers(
    dvClass theClass)
{
    dvWrtemp(dvFile, 
        "/*----------------------------------------------------------------------------------------\n"
        "  Default constructor wrapper for the database manager.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "static uint64 alloc%1(void)\n"
        "{\n"
        "    %0%1 %1 = %0%1Alloc();\n"
        "\n"
        "    return %0%12Index(%1);\n"
        "}\n\n",
        dvPrefix, dvClassGetName(theClass));
    if(dvClassGetMemoryStyle(theClass) != MEM_CREATE_ONLY) {
        dvWrtemp(dvFile, 
            "/*-----------------------------------------------------"
            "-----------------------------------\n"
            "  Destructor wrapper for the database manager.\n"
            "-------------------------------------------------------"
            "---------------------------------*/\n"
            "static void destroy%1(\n"
            "    uint64 objectIndex)\n"
            "{\n"
            "    %0%1Destroy(%0Index2%1((%2)objectIndex));\n"
            "}\n\n" ,
            dvPrefix, dvClassGetName(theClass), dvClassGetReferenceTypeName(theClass));
    }
}

/*--------------------------------------------------------------------------------------------------
  Write a function to compact a property array.
--------------------------------------------------------------------------------------------------*/
static void writeArrayCompact(
    dvProperty property)
{
    dvClass theClass = dvPropertyGetClass(property);

    if(dvPropertyView(property)) {
        dvWrtemp(dvFile, "/* compact function for %0 is user provided */\n",
            dvPropertyGetName(property));
        return;
    }

    dvWrtemp(dvFile, 
        "/*----------------------------------------------------------------------------------------\n"
        "  Compact the %1.%2 heap to free memory.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "void %0Compact%1%2s(void)\n"
        "{\n"
        "    uint32 elementSize = sizeof(%4);\n"
        "    uint32 usedHeaderSize = (sizeof(%3%1) + elementSize - 1)/elementSize;\n"
        "    uint32 freeHeaderSize = (sizeof(%3%1) + sizeof(uint32) + elementSize - 1)/elementSize;\n"
        "    %4 *toPtr = %0%1s.%2;\n"
        "    %4 *fromPtr = toPtr;\n"
        "    %3%1 %1;\n"
        "    uint32 size;\n"
        "\n"
        "    while(fromPtr < %0%1s.%2 + %0Used%1%2()) {\n"
        "        %1 = *(%3%1 *)(void *)fromPtr;\n"
        "        if(%1 != %3%1Null) {\n"
        "            /* Need to move it to toPtr */\n"
        "            size = utMax(%0%1GetNum%2(%1) + usedHeaderSize, freeHeaderSize);\n",
        dvPrefix, dvClassGetName(theClass), dvPropertyGetName(property), dvClassGetPrefix(theClass),
        dvPropertyGetTypeName(property));
    if(dvClassUndo(theClass)) {
        dvWrtemp(dvFile,
            "            utRecordArray(%0ModuleID, %3, toPtr - %0%1s.%2, size, true);\n",
            dvPrefix, dvClassGetName(theClass), dvPropertyGetName(property),
            dvPropertyGetID(property));
    }
    dvWrtemp(dvFile, 
        "            memmove((void *)toPtr, (void *)fromPtr, size*elementSize);\n");
    if(dvClassRedo(theClass)) {
        dvWrtemp(dvFile,
            "            utRecordArray(%0ModuleID, %3, toPtr - %0%1s.%2, size, false);\n",
            dvPrefix, dvClassGetName(theClass), dvPropertyGetName(property),
            dvPropertyGetID(property));
    }
    dvWrtemp(dvFile, 
        "            %0%1Set%2Index_(%1, toPtr - %0%1s.%2 + usedHeaderSize);\n"
        "            toPtr += size;\n"
        "        } else {\n"
        "            /* Just skip it */\n"
        "            size = *(uint32 *)(void *)(((%3%1 *)(void *)fromPtr) + 1);\n"
        "        }\n"
        "        fromPtr += size;\n"
        "    }\n"
        "    %0SetUsed%1%2(toPtr - %0%1s.%2);\n"
        "    %0SetFree%1%2(0);\n"
        "}\n\n",
        dvPrefix, dvClassGetName(theClass), dvPropertyGetName(property), dvClassGetPrefix(theClass));
}

/*--------------------------------------------------------------------------------------------------
  Write a function to compact and/or allocate more space on the property heap.
--------------------------------------------------------------------------------------------------*/
static void writeArrayAllocMore(
    dvProperty property)
{
    dvClass theClass = dvPropertyGetClass(property);

    if(dvPropertyView(property)) {
        dvWrtemp(dvFile, "/* compact and/or allocate more space function for %0 is user provided */\n",
            dvPropertyGetName(property));
        return;
    }

    dvWrtemp(dvFile, 
        "/*----------------------------------------------------------------------------------------\n"
        "  Allocate more memory for the %1.%2 heap.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "static void allocMore%1%2s(\n"
        "    uint32 spaceNeeded)\n"
        "{\n"
        "    uint32 freeSpace = %0Allocated%1%2() - %0Used%1%2();\n"
        "\n"
        "    if((%0Free%1%2() << 2) > %0Used%1%2()) {\n"
        "        %0Compact%1%2s();\n"
        "        freeSpace = %0Allocated%1%2() - %0Used%1%2();\n"
        "    }\n"
        "    if(freeSpace < spaceNeeded) {\n",
        dvPrefix, dvClassGetName(theClass), dvPropertyGetName(property), dvClassGetPrefix(theClass),
        dvPropertyGetTypeName(property));
    if(dvClassUndo(theClass)) {
        dvWrtemp(dvFile, 
            "        utRecordResize(%0ModuleID, %3, %0Allocated%1%2(), true);\n",
            dvPrefix, dvClassGetName(theClass), dvPropertyGetName(property),
            utSprintf("%u", dvPropertyGetFieldNumber(property)));
    }
    dvWrtemp(dvFile,
        "        %0SetAllocated%1%2(%0Allocated%1%2() + spaceNeeded - freeSpace +\n"
        "            (%0Allocated%1%2() >> 1));\n"
        "        utResizeArray(%0%1s.%2, %0Allocated%1%2());\n",
        dvPrefix, dvClassGetName(theClass), dvPropertyGetName(property));
    if(dvClassRedo(theClass)) {
        dvWrtemp(dvFile, 
            "        utRecordResize(%0ModuleID, %3, %0Allocated%1%2(), false);\n",
            dvPrefix, dvClassGetName(theClass), dvPropertyGetName(property),
            utSprintf("%u", dvPropertyGetFieldNumber(property)));
    }
    dvWrtemp(dvFile, 
        "    }\n"
        "}\n\n");
}

/*--------------------------------------------------------------------------------------------------
  Write a function to allocate the requested space for the array.  The request is always placed
  at the end of the heap, but if the ammount of free memory withing the heap is > 25%, then we
  compact the heap first.
--------------------------------------------------------------------------------------------------*/
static void writeArrayAlloc(
    dvProperty property)
{
    dvClass theClass = dvPropertyGetClass(property);

    if(dvPropertyView(property)) {
        dvWrtemp(dvFile, "/* allocate more space function for %0 is user provided */\n",
            dvPropertyGetName(property));
        return;
    }

    dvWrtemp(dvFile, 
        "/*----------------------------------------------------------------------------------------\n"
        "  Allocate memory for a new %1.%2 array.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "void %0%1Alloc%2s(\n"
        "    %3%1 %1,\n"
        "    uint32 num%2s)\n"
        "{\n"
        "    uint32 freeSpace = %0Allocated%1%2() - %0Used%1%2();\n"
        "    uint32 elementSize = sizeof(%4);\n"
        "    uint32 usedHeaderSize = (sizeof(%3%1) + elementSize - 1)/elementSize;\n"
        "    uint32 freeHeaderSize = (sizeof(%3%1) + sizeof(uint32) + elementSize - 1)/elementSize;\n"
        "    uint32 spaceNeeded = utMax(num%2s + usedHeaderSize, freeHeaderSize);\n"
        "\n"
        "#if defined(DD_DEBUG)\n"
        "    utAssert(%0%1GetNum%2(%1) == 0);\n"
        "#endif\n"
        "    if(num%2s == 0) {\n"
        "        return;\n"
        "    }\n"
        "    if(freeSpace < spaceNeeded) {\n"
        "        allocMore%1%2s(spaceNeeded);\n"
        "    }\n"
        "    %0%1Set%2Index_(%1, %0Used%1%2() + usedHeaderSize);\n"
        "    %0%1SetNum%2(%1, num%2s);\n",
        dvPrefix, dvClassGetName(theClass), dvPropertyGetName(property), dvClassGetPrefix(theClass),
        dvPropertyGetTypeName(property));
    if(dvClassUndo(theClass)) {
        dvWrtemp(dvFile,
            "    utRecordArray(%0ModuleID, %3, %0Used%1%2(), num%2s + usedHeaderSize, true);\n",
            dvPrefix, dvClassGetName(theClass), dvPropertyGetName(property),
            dvPropertyGetID(property));
    }
    dvWrtemp(dvFile,
        "    *(%3%1 *)(void *)(%0%1s.%2 + %0Used%1%2()) = %1;\n",
        dvPrefix, dvClassGetName(theClass), dvPropertyGetName(property), dvClassGetPrefix(theClass));
    writePropertyInits(property, dvSwrtemp("%0%1Get%2Index_(%1)", dvPrefix, dvClassGetName(theClass),
        dvPropertyGetName(property)), utSprintf("num%ss", dvPropertyGetName(property)), "");
    if(dvClassRedo(theClass)) {
        dvWrtemp(dvFile,
            "    utRecordArray(%0ModuleID, %3, %0Used%1%2(), num%2s + usedHeaderSize, false);\n",
            dvPrefix, dvClassGetName(theClass), dvPropertyGetName(property),
            dvPropertyGetID(property));
    }
    dvWrtemp(dvFile,
        "    %0SetUsed%1%2(%0Used%1%2() + spaceNeeded);\n"
        "}\n\n",
        dvPrefix, dvClassGetName(theClass), dvPropertyGetName(property));
    dvWrtemp(dvFile, 
        "/*----------------------------------------------------------------------------------------\n"
        "  Wrapper around %0%1Get%2s for the database manager.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "static void *get%1%2s(\n"
        "    uint64 objectNumber,\n"
        "    uint32 *numValues)\n"
        "{\n"
        "    %3%1 %1 = %3Index2%1((%4)objectNumber);\n"
        "\n"
        "    *numValues = %0%1GetNum%2(%1);\n"
        "    return %0%1Get%2s(%1);\n"
        "}\n"
        "\n"
        "/*----------------------------------------------------------------------------------------\n"
        "  Wrapper around %0%1Alloc%2s for the database manager.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "static void *alloc%1%2s(\n"
        "    uint64 objectNumber,\n"
        "    uint32 numValues)\n"
        "{\n"
        "    %3%1 %1 = %3Index2%1((%4)objectNumber);\n"
        "\n"
        "    %0%1Set%2Index_(%1, 0);\n"
        "    %0%1SetNum%2(%1, 0);\n"
        "    if(numValues == 0) {\n"
        "        return NULL;\n"
        "    }\n"
        "    %0%1Alloc%2s(%1, numValues);\n"
        "    return %0%1Get%2s(%1);\n"
        "}\n\n",
        dvPrefix, dvClassGetName(theClass), dvPropertyGetName(property), dvClassGetPrefix(theClass),
                dvClassGetReferenceTypeName(theClass));
}

/*--------------------------------------------------------------------------------------------------
  Write a function to return a pointer to the array of values.  This is needed by the database
  manager.
--------------------------------------------------------------------------------------------------*/
static void writeArrayGetValues(
    dvProperty property)
{
    dvClass theClass = dvPropertyGetClass(property);

    dvWrtemp(dvFile, 
        "/*----------------------------------------------------------------------------------------\n"
        "  Wrapper around %0%1Get%2s for the database manager.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "static void *get%1%2s(\n"
        "    uint64 objectNumber,\n"
        "    uint32 *numValues)\n"
        "{\n"
        "    %3%1 %1 = %3Index2%1((%4)objectNumber);\n"
        "\n"
        "    *numValues = (%5);\n"
        "    return %0%1Get%2s(%1);\n"
        "}\n"
        "\n",
        dvPrefix, dvClassGetName(theClass), dvPropertyGetName(property), dvClassGetPrefix(theClass),
                dvClassGetReferenceTypeName(theClass), dvPropertyGetIndex(property));
}

/*--------------------------------------------------------------------------------------------------
  Write a function to free a property array.
--------------------------------------------------------------------------------------------------*/
static void writeArrayFree(
    dvProperty property)
{
    dvClass theClass = dvPropertyGetClass(property);

    if(dvPropertyView(property)) {
        dvWrtemp(dvFile, "/* free space function for %0 is user provided */\n",
            dvPropertyGetName(property));
        return;
    }

    dvWrtemp(dvFile, 
        "/*----------------------------------------------------------------------------------------\n"
        "  Free memory used by the %1.%2 array.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "void %0%1Free%2s(\n"
        "    %3%1 %1)\n"
        "{\n"
        "    uint32 elementSize = sizeof(%4);\n"
        "    uint32 usedHeaderSize = (sizeof(%3%1) + elementSize - 1)/elementSize;\n"
        "    uint32 freeHeaderSize = (sizeof(%3%1) + sizeof(uint32) + elementSize - 1)/elementSize;\n"
        "    uint32 size = utMax(%0%1GetNum%2(%1) + usedHeaderSize, freeHeaderSize);\n"
        "    %4 *dataPtr = %0%1Get%2s(%1) - usedHeaderSize;\n"
        "\n"
        "    if(%0%1GetNum%2(%1) == 0) {\n"
        "        return;\n"
        "    }\n",
        dvPrefix, dvClassGetName(theClass), dvPropertyGetName(property), dvClassGetPrefix(theClass),
        dvPropertyGetTypeName(property));
    if(dvClassUndo(theClass)) {
        dvWrtemp(dvFile,
            "    utRecordArray(%0ModuleID, %3, dataPtr - %0%1s.%2, freeHeaderSize, true);\n",
            dvPrefix, dvClassGetName(theClass), dvPropertyGetName(property),
            dvPropertyGetID(property));
    }
    dvWrtemp(dvFile, 
        "    *(%1%0 *)(void *)(dataPtr) = %1%0Null;\n"
        "    *(uint32 *)(void *)(((%1%0 *)(void *)dataPtr) + 1) = size;\n",
        dvClassGetName(theClass), dvClassGetPrefix(theClass));
    if(dvClassRedo(theClass)) {
        dvWrtemp(dvFile,
            "    utRecordArray(%0ModuleID, %3, dataPtr - %0%1s.%2, freeHeaderSize, false);\n",
            dvPrefix, dvClassGetName(theClass), dvPropertyGetName(property),
            dvPropertyGetID(property));
    }
    dvWrtemp(dvFile, 
        "    %0%1SetNum%2(%1, 0);\n"
        "    %0SetFree%1%2(%0Free%1%2() + size);\n"
        "}\n\n",
        dvPrefix, dvClassGetName(theClass), dvPropertyGetName(property));
}

/*--------------------------------------------------------------------------------------------------
  Write a function to reallocate a property array.
--------------------------------------------------------------------------------------------------*/
static void writeArrayResize(
    dvProperty property)
{
    dvClass theClass = dvPropertyGetClass(property);

    if(dvPropertyView(property)) {
        dvWrtemp(dvFile, "/* reallocate space function for %0 is user provided */\n",
            dvPropertyGetName(property));
        return;
    }

    dvWrtemp(dvFile, 
        "/*----------------------------------------------------------------------------------------\n"
        "  Resize the %1.%2 array.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "void %0%1Resize%2s(\n"
        "    %3%1 %1,\n"
        "    uint32 num%2s)\n"
        "{\n"
        "    uint32 freeSpace;\n"
        "    uint32 elementSize = sizeof(%4);\n"
        "    uint32 usedHeaderSize = (sizeof(%3%1) + elementSize - 1)/elementSize;\n"
        "    uint32 freeHeaderSize = (sizeof(%3%1) + sizeof(uint32) + elementSize - 1)/elementSize;\n"
        "    uint32 newSize = utMax(num%2s + usedHeaderSize, freeHeaderSize);\n"
        "    uint32 oldSize = utMax(%0%1GetNum%2(%1) + usedHeaderSize, freeHeaderSize);\n"
        "    %4 *dataPtr;\n"
        "\n"
        "    if(num%2s == 0) {\n"
        "        if(%0%1GetNum%2(%1) != 0) {\n"
        "            %0%1Free%2s(%1);\n"
        "        }\n"
        "        return;\n"
        "    }\n"
        "    if(%0%1GetNum%2(%1) == 0) {\n"
        "        %0%1Alloc%2s(%1, num%2s);\n"
        "        return;\n"
        "    }\n"
        "    freeSpace = %0Allocated%1%2() - %0Used%1%2();\n"
        "    if(freeSpace < newSize) {\n"
        "        allocMore%1%2s(newSize);\n"
        "    }\n"
        "    dataPtr = %0%1Get%2s(%1) - usedHeaderSize;\n",
        dvPrefix, dvClassGetName(theClass), dvPropertyGetName(property), dvClassGetPrefix(theClass),
        dvPropertyGetTypeName(property));
    if(dvClassUndo(theClass)) {
        dvWrtemp(dvFile,
            "    utRecordArray(%0ModuleID, %3, %0Used%1%2(), newSize, true);\n"
            "    utRecordArray(%0ModuleID, %3, dataPtr - %0%1s.%2, freeHeaderSize, true);\n",
            dvPrefix, dvClassGetName(theClass), dvPropertyGetName(property),
            dvPropertyGetID(property));
    }
    dvWrtemp(dvFile,
        "    memcpy((void *)(%0%1s.%2 + %0Used%1%2()), dataPtr,\n"
        "        elementSize*utMin(oldSize, newSize));\n"
        "    if(newSize > oldSize) {\n",
        dvPrefix, dvClassGetName(theClass), dvPropertyGetName(property));
    writePropertyInits(property, utSprintf("%sUsed%s%s() + oldSize", dvPrefix, dvClassGetName(theClass),
        dvPropertyGetName(property)), "newSize - oldSize", "    ");
    dvWrtemp(dvFile,
        "    }\n"
        "    *(%1%0 *)(void *)dataPtr = %1%0Null;\n"
        "    *(uint32 *)(void *)(((%1%0 *)(void *)dataPtr) + 1) = oldSize;\n",
        dvClassGetName(theClass), dvClassGetPrefix(theClass));
    if(dvClassRedo(theClass)) {
        dvWrtemp(dvFile,
            "    utRecordArray(%0ModuleID, %3, %0Used%1%2(), newSize, false);\n"
            "    utRecordArray(%0ModuleID, %3, dataPtr - %0%1s.%2, freeHeaderSize, false);\n",
            dvPrefix, dvClassGetName(theClass), dvPropertyGetName(property),
            dvPropertyGetID(property));
    }
    dvWrtemp(dvFile,
        "    %0SetFree%1%2(%0Free%1%2() + oldSize);\n"
        "    %0%1Set%2Index_(%1, %0Used%1%2() + usedHeaderSize);\n"
        "    %0%1SetNum%2(%1, num%2s);\n"
        "    %0SetUsed%1%2(%0Used%1%2() + newSize);\n"
        "}\n\n",
        dvPrefix, dvClassGetName(theClass), dvPropertyGetName(property));
}

/*--------------------------------------------------------------------------------------------------
  Write functions that manipulate property arrays on this class.
--------------------------------------------------------------------------------------------------*/
static void writeClassPropertyArrayFunctions(
    dvClass theClass)
{
    dvProperty property;

    dvForeachClassProperty(theClass, property) {
        if(dvPropertyArray(property)) {
            if(!dvPropertyFixedSize(property)) {
                writeArrayCompact(property);
                writeArrayAllocMore(property);
                writeArrayAlloc(property);
                writeArrayFree(property);
                writeArrayResize(property);
            } else {
                writeArrayGetValues(property);
            }
        }
    } dvEndClassProperty;
}

/*--------------------------------------------------------------------------------------------------
  This relationship needs to have children processed during destruction.
--------------------------------------------------------------------------------------------------*/
bool needsChildrenProcessed(
    dvRelationship relationship)
{
    if(!dvRelationshipAccessChild(relationship)) {
        return false; /* Can't get there from here */
    }
    if(dvRelationshipCascade(relationship)) {
        return true; /* User decrees it! */
    }
    if(dvRelationshipAccessParent(relationship)) {
        return true; /* Children have owner pointers to be nulled out */
    }
    return false;
}

/*--------------------------------------------------------------------------------------------------
  Write Binary Heap functions for array relationship.
--------------------------------------------------------------------------------------------------*/
static void writeRelationshipBinaryHeapFunctions(
    dvRelationship relationship)
{
    dvClass parent = dvRelationshipGetParentClass(relationship);
    dvClass child = dvRelationshipGetChildClass(relationship);

    dvWrtemp(dvFile,
        "/*----------------------------------------------------------------------------------------\n"
        "  Swap two elements in the heap.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "static void %l1Swap%2%3(\n"
        "    %4%1 %1,\n"
        "    uint32 x,\n"
        "    uint32 y)\n"
        "{\n"
        "    %5%3 newY = %0%1Geti%2%3(%1, x);\n"
        "    %5%3 newX = %0%1Geti%2%3(%1, y);\n"
        "\n"
        "    %0%1Seti%2%3(%1, x, newX);\n"
        "    %0%1Seti%2%3(%1, y, newY);\n"
        "    %0%3Set%1%2Index(newX, x);\n"
        "    %0%3Set%1%2Index(newY, y);\n"
        "}\n"
        "\n"
        "/*----------------------------------------------------------------------------------------\n"
        "  Swap the element down in the heap until it's below all it's parents in cost.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "static void %l1HeapDown%2%3(\n"
        "    %4%1 %1,\n"
        "    uint32 startX)\n"
        "{\n"
        "    uint32 x = startX;\n"
        "    uint32 leftIndex;\n"
        "    uint32 rightIndex;\n"
        "    %5%3 cur = %0%1Geti%2%3(%1, x);\n"
        "    %5%3 left = %5%3Null;\n"
        "    %5%3 right = %5%3Null;\n"
        "    %5%3 best;\n"
        "    \n"
        "    utDo {\n"
        "        best = cur;\n"
        "        leftIndex = (x << 1) + 1;\n"
        "        rightIndex = leftIndex + 1;\n"
        "        if(leftIndex < %0%1GetUsed%2%3(%1)) {\n"
        "            left = %0%1Geti%2%3(%1, leftIndex);\n"
        "            if(%0%1Compare%2%3(best, left) > 0) {\n"
        "                best = left;\n"
        "            }\n"
        "            if(rightIndex < %0%1GetUsed%2%3(%1)) {\n"
        "                right = %0%1Geti%2%3(%1, rightIndex);\n"
        "                if(%0%1Compare%2%3(best, right) > 0) {\n"
        "                    best = right;\n"
        "                }\n"
        "            }\n"
        "        }\n"
        "    } utWhile(best != cur) {\n"
        "        if(best == left) {\n"
        "            %l1Swap%2%3(%1, x, leftIndex);\n"
        "            x = leftIndex;\n"
        "        } else {\n"
        "            %l1Swap%2%3(%1, x, rightIndex);\n"
        "            x = rightIndex;\n"
        "        }\n"
        "    } utRepeat;\n"
        "}\n"
        "\n"
        "/*----------------------------------------------------------------------------------------\n"
        "  Swap the element up in the heap until it's greater than all it's children in cost.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "static void %l1HeapUp%2%3(\n"
        "    %4%1 %1,\n"
        "    uint32 startX)\n"
        "{\n"
        "    \n"
        "    uint32 x = startX;\n"
        "    %5%3 cur = %0%1Geti%2%3(%1, x);\n"
        "    uint32 parentIndex;\n"
        "    \n"
        "    utDo {\n"
        "        parentIndex = (x - 1) >> 1;\n"
        "    } utWhile(x > 0 && %0%1Compare%2%3(%0%1Geti%2%3(%1, parentIndex), cur) > 0) {\n"
        "        %l1Swap%2%3(%1, parentIndex, x);\n"
        "        x = parentIndex;\n"
        "    } utRepeat\n"
        "}\n"
        "\n"
        "/*----------------------------------------------------------------------------------------\n"
        "  Return the top of the heap without removing it.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "%5%3 %0%1Peek%2%3(\n"
        "    %4%1 %1)\n"
        "{\n"
        "    if(%0%1GetUsed%2%3(%1) == 0) {\n"
        "        return %5%3Null;\n"
        "    }\n"
        "    return %0%1Geti%2%3(%1, 0);\n"
        "}\n"
        "\n"
        "/*----------------------------------------------------------------------------------------\n"
        "  Remove the top element of the heap and return it.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "static %5%3 %l1Pop%2%3(\n"
        "    %4%1 %1,\n"
        "    uint32 index)\n"
        "{\n"
        "    %5%3 cur;\n"
        "    %5%3 retval = %0%1Geti%2%3(%1, index);\n"
        "    uint32 newNum = %0%1GetUsed%2%3(%1) - 1;\n"
        "\n"
        "    %0%3Set%6%1(retval, %4%1Null);\n"
        "    %0%1SetUsed%2%3(%1, newNum);\n"
        "    if(newNum != 0) {\n"
        "        cur = %0%1Geti%2%3(%1, newNum);\n"
        "        %0%1Seti%2%3(%1, index, cur);\n"
        "        %0%3Set%1%2Index(cur, index);\n"
        "        %l1HeapDown%2%3(%1, index);\n"
        "    }\n"
        "    return retval;\n"
        "}\n"
        "\n"
        "/*----------------------------------------------------------------------------------------\n"
        "  Remove the top element of the heap and return it.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "%5%3 %0%1Pop%2%3(\n"
        "    %4%1 %1)\n"
        "{\n"
        "    if(%0%1GetUsed%2%3(%1) == 0) {\n"
        "        return %5%3Null;\n"
        "    }\n"
        "    return %l1Pop%2%3(%1, 0);\n"
        "}\n"
        "\n"
        "/*----------------------------------------------------------------------------------------\n"
        "  Float up the heap element.  This is for cases where an element's cost has been changed.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "void %0%1Update%2%3(\n"
        "    %4%1 %1,\n"
        "    %5%3 _%3)\n"
        "{\n"
        "    uint32 _index = %0%3Get%1%2Index(_%3);\n"
        "\n"
        "    %l1HeapUp%2%3(%1, _index);\n"
        "    %l1HeapDown%2%3(%1, _index);\n"
        "}\n"
        "\n",
        dvPrefix, dvClassGetName(parent), dvRelationshipGetChildLabel(relationship), 
             dvClassGetName(child), dvClassGetPrefix(parent), dvClassGetPrefix(child), 
             dvRelationshipGetParentLabel(relationship));
}

/*--------------------------------------------------------------------------------------------------
  Write the lines of the ordered list find function to compare a key value.
--------------------------------------------------------------------------------------------------*/
static void writeOrderedListComparison(
    char *compareString,
    bool firstTime)
{
    if(firstTime) {
        dvWrtemp(dvFile, "        comparison = %0;\n", compareString);
    } else {
        dvWrtemp(dvFile,
            "        if(comparison == 0) {\n"
            "            comparison = %0;\n"
            "        }\n",
            compareString);
    }
}

/*--------------------------------------------------------------------------------------------------
  Write code to compare key values to an element in an ordered list.
--------------------------------------------------------------------------------------------------*/
static void writeOrderedListComparisons(
    dvRelationship relationship)
{
    dvProperty property;
    dvKey key;
    bool firstTime = true;
    char *compareString;

    dvForeachRelationshipKey(relationship, key) {
        property = dvKeypropertyGetProperty(dvKeyGetLastKeyproperty(key));
        if(!dvPropertyArray(property)) {
            compareString = dvSwrtemp("%0 - %1",
                dvPropertyGetName(property),
                dvKeyGetAccessMacro(key, false, "node"));
            writeOrderedListComparison(compareString, firstTime);
        } else if(!dvPropertyFixedSize(property)) {
            compareString = dvSwrtemp("%0Length - %1",
                dvPropertyGetName(property),
                dvKeyGetLengthMacro(key, false, "node"));
            writeOrderedListComparison(compareString, firstTime);
            compareString = dvSwrtemp("memcmp(%0, %1, sizeof(%2)*%1Length)",
                dvKeyGetAccessMacro(key, false, "node"),
                dvPropertyGetName(property),
                dvPropertyGetTypeName(property));
            writeOrderedListComparison(compareString, false);
        } else {
            compareString = dvSwrtemp("memcmp(%0, %1, sizeof(%2)*(%3))",
                dvKeyGetAccessMacro(key, false, "node"),
                dvPropertyGetName(property),
                dvPropertyGetTypeName(property),
                dvPropertyGetIndex(property));
            writeOrderedListComparison(compareString, firstTime);
        }
        firstTime = false;
    } dvEndRelationshipKey;
}

/*--------------------------------------------------------------------------------------------------
  Write an ordered-list find following element function.
--------------------------------------------------------------------------------------------------*/
static void writeOrderedListFindNextFunction(
    dvRelationship relationship)
{
    dvClass parent = dvRelationshipGetParentClass(relationship);
    dvClass child = dvRelationshipGetChildClass(relationship);

    dvWrtemp(dvFile,
        "/*----------------------------------------------------------------------------------------\n"
        "  Find the node in the graph exactly (last of sequence) or next to specified key\n"
        "----------------------------------------------------------------------------------------*/\n"
        "%5%3 %0%1FindNext%2%3(\n"
        "    %4%1 %1",
        dvPrefix, dvClassGetName(parent), dvRelationshipGetChildLabel(relationship), 
             dvClassGetName(child), dvClassGetPrefix(parent), dvClassGetPrefix(child), 
             dvRelationshipGetParentLabel(relationship));
    writeKeyParameters(relationship);
    dvWrtemp(dvFile,
        ")\n"
        "{\n"
        "    %5%3 node = %0%1GetRoot%2%3(%1);\n"
        "    %5%3 next%3 = %5%3Null;\n"
        "    int comparison;\n"
        "\n"
        "    while(node != %5%3Null) {\n"
        "        next%3 = node;\n",
        dvPrefix, dvClassGetName(parent), dvRelationshipGetChildLabel(relationship), 
             dvClassGetName(child), dvClassGetPrefix(parent), dvClassGetPrefix(child), 
             dvRelationshipGetParentLabel(relationship));
    writeOrderedListComparisons(relationship);
    dvWrtemp(dvFile,
        "        if(comparison < 0) {\n"
        "            node = %0%3GetLeft%1%2%3(node);\n"
        "        } else if(comparison > 0) {\n"
        "            node = %0%3GetRight%1%2%3(node);\n"
        "        } else {\n"
        "            next%3 = node;\n"
        "            utDo {\n"
        "                node = %0%3GetRight%1%2%3(node);\n"
        "                if(node != %5%3Null) {\n",
        dvPrefix, dvClassGetName(parent), dvRelationshipGetChildLabel(relationship), 
        dvClassGetName(child), dvClassGetPrefix(parent), dvClassGetPrefix(child), 
        dvRelationshipGetParentLabel(relationship));
    writeOrderedListComparisons(relationship);
    dvWrtemp(dvFile,
        "                }\n"
        "                else {\n"
        "                    break;\n"
        "                }\n"
        "            }\n"
        "            utWhile(comparison == 0) {\n"
        "                next%3 = node;\n"
        "            }\n"
        "            utRepeat;\n"
        "            return next%3;\n"
        "        }\n"
        "    }\n"
        "    if(next%3 != %5%3Null) {\n"
        "        if(comparison > 0) {\n"
        "            return %0%3GetNext%1%2%3(next%3);\n"
        "        }\n"
        "    }\n"
        "    return next%3;\n"
        "}\n"
        "\n",
        dvPrefix, dvClassGetName(parent), dvRelationshipGetChildLabel(relationship), 
             dvClassGetName(child), dvClassGetPrefix(parent), dvClassGetPrefix(child), 
             dvRelationshipGetParentLabel(relationship));
}


/*--------------------------------------------------------------------------------------------------
  Write an ordered-list find previous element function.
--------------------------------------------------------------------------------------------------*/
static void writeOrderedListFindPrevFunction(
    dvRelationship relationship)
{
    dvClass parent = dvRelationshipGetParentClass(relationship);
    dvClass child = dvRelationshipGetChildClass(relationship);

    dvWrtemp(dvFile,
        "/*----------------------------------------------------------------------------------------\n"
        "  Find the node in the graph exactly (first of sequence) or previous to specified key\n"
        "----------------------------------------------------------------------------------------*/\n"
        "%5%3 %0%1FindPrev%2%3(\n"
        "    %4%1 %1",
        dvPrefix, dvClassGetName(parent), dvRelationshipGetChildLabel(relationship), 
             dvClassGetName(child), dvClassGetPrefix(parent), dvClassGetPrefix(child), 
             dvRelationshipGetParentLabel(relationship));
    writeKeyParameters(relationship);
    dvWrtemp(dvFile,
        ")\n"
        "{\n"
        "    %5%3 node = %0%1GetRoot%2%3(%1);\n"
        "    %5%3 prev%3 = %5%3Null;\n"
        "    int comparison;\n"
        "\n"
        "    while(node != %5%3Null) {\n"
        "        prev%3 = node;\n",
        dvPrefix, dvClassGetName(parent), dvRelationshipGetChildLabel(relationship), 
        dvClassGetName(child), dvClassGetPrefix(parent), dvClassGetPrefix(child), 
        dvRelationshipGetParentLabel(relationship));
    writeOrderedListComparisons(relationship);
    dvWrtemp(dvFile,
        "        if(comparison < 0) {\n"
        "            node = %0%3GetLeft%1%2%3(node);\n"
        "        } else if(comparison > 0) {\n"
        "            node = %0%3GetRight%1%2%3(node);\n"
        "        } else {\n"
        "            prev%3 = node;\n"
        "            utDo {\n"
        "                node = %0%3GetLeft%1%2%3(node);\n"
        "                if(node != %5%3Null) {\n",
        dvPrefix, dvClassGetName(parent), dvRelationshipGetChildLabel(relationship), 
        dvClassGetName(child), dvClassGetPrefix(parent), dvClassGetPrefix(child), 
        dvRelationshipGetParentLabel(relationship));
    writeOrderedListComparisons(relationship);
    dvWrtemp(dvFile,
        "                }\n"
        "                else {\n"
        "                    break;\n"
        "                }\n"
        "            }\n"
        "            utWhile(comparison == 0) {\n"
        "                prev%3 = node;\n"
        "            }\n"
        "            utRepeat;\n"
        "            return prev%3;\n"
        "        }\n"
        "    }\n"
        "    if(prev%3 != %5%3Null) {\n"
        "        if(comparison < 0) {\n"
        "            return %0%3GetPrev%1%2%3(prev%3);\n"
        "        }\n"
        "    }\n"
        "    return prev%3;\n"
        "}\n"
        "\n",
        dvPrefix, dvClassGetName(parent), dvRelationshipGetChildLabel(relationship), 
        dvClassGetName(child), dvClassGetPrefix(parent), dvClassGetPrefix(child), 
        dvRelationshipGetParentLabel(relationship));
}

/*--------------------------------------------------------------------------------------------------
  Write an ordered-list find previous function.
--------------------------------------------------------------------------------------------------*/
static void writeOrderedListFindFunction(
    dvRelationship relationship)
{
    dvClass parent = dvRelationshipGetParentClass(relationship);
    dvClass child = dvRelationshipGetChildClass(relationship);

    dvWrtemp(dvFile,
        "/*----------------------------------------------------------------------------------------\n"
        "  Find the node in the graph matching key, first of sequence if many.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "%5%3 %0%1Find%2%3(\n"
        "    %4%1 %1",
        dvPrefix, dvClassGetName(parent), dvRelationshipGetChildLabel(relationship), 
             dvClassGetName(child), dvClassGetPrefix(parent), dvClassGetPrefix(child), 
             dvRelationshipGetParentLabel(relationship));
    writeKeyParameters(relationship);
    dvWrtemp(dvFile,
        ")\n"
        "{\n"
        "    %5%3 next;\n"
        "    %5%3 node = %0%1GetRoot%2%3(%1);\n"
        "    int comparison;\n"
        "\n"
        "    while(node != %5%3Null) {\n",
        dvPrefix, dvClassGetName(parent), dvRelationshipGetChildLabel(relationship), 
             dvClassGetName(child), dvClassGetPrefix(parent), dvClassGetPrefix(child), 
             dvRelationshipGetParentLabel(relationship));
    writeOrderedListComparisons(relationship);
    dvWrtemp(dvFile,
        "        if(comparison < 0) {\n"
        "            node = %0%3GetLeft%1%2%3(node);\n"
        "        } else if(comparison > 0) {\n"
        "            node = %0%3GetRight%1%2%3(node);\n"
        "        } else {\n"
        "            next = node;\n"
        "            utDo {\n"
        "                node = %0%3GetLeft%1%2%3(node);\n"
        "                if(node != %5%3Null) {\n",
        dvPrefix, dvClassGetName(parent), dvRelationshipGetChildLabel(relationship), 
        dvClassGetName(child), dvClassGetPrefix(parent), dvClassGetPrefix(child), 
        dvRelationshipGetParentLabel(relationship));
    writeOrderedListComparisons(relationship);
    dvWrtemp(dvFile,
        "                }\n"
        "                else {\n"
        "                    break;\n"
        "                }\n"
        "            }\n"
        "            utWhile(comparison == 0) {\n"
        "                next = node;\n"
        "            }\n"
        "            utRepeat;\n"
        "            return next;\n"
        "        }\n"
        "    }\n"
        "    return %5%3Null;\n"
        "}\n"
        "\n",
        dvPrefix, dvClassGetName(parent), dvRelationshipGetChildLabel(relationship), 
             dvClassGetName(child), dvClassGetPrefix(parent), dvClassGetPrefix(child), 
             dvRelationshipGetParentLabel(relationship));
}

/*--------------------------------------------------------------------------------------------------
  Write ordered-list functions using left-leaning red-black trees (LLRB).
--------------------------------------------------------------------------------------------------*/
static void writeOrderedListFunctions(
    dvRelationship relationship)
{
    dvClass parent = dvRelationshipGetParentClass(relationship);
    dvClass child = dvRelationshipGetChildClass(relationship);

    dvWrtemp(dvFile,
        "/*----------------------------------------------------------------------------------------\n"
        "  Ensure tree consistency to a certain extent.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "#if defined(DD_DEBUG)\n"
        "void %0Verify%1%2%3(\n"
        "    %4%1 %1)\n"
        "{\n"
        "    int treeblackcount = 0, currentblackcount;\n"
        "    %5%3 current, previous = %0%1GetFirst%2%3(%1), counter;\n"
        "\n"
        "    utAssert(%0%1GetRoot%2%3(%1) == %5%3Null || !%0%3IsRed%1%2%3(%0%1GetRoot%2%3(%1)));\n"
        "    %0Foreach%1%2%3(%1,current) {\n"
        "        utAssert(%0%1Compare%2%3(previous, current) >= 0);\n"
        "        previous = current;\n",
        dvPrefix, dvClassGetName(parent), dvRelationshipGetChildLabel(relationship), 
             dvClassGetName(child), dvClassGetPrefix(parent), dvClassGetPrefix(child), 
             dvRelationshipGetParentLabel(relationship));

    if(dvRelationshipAccessParent(relationship)) {
        dvWrtemp(dvFile,
            "        utAssert(%0%3Get%6%1(current) == %1);\n",
            dvPrefix, dvClassGetName(parent), dvRelationshipGetChildLabel(relationship), 
                 dvClassGetName(child), dvClassGetPrefix(parent), dvClassGetPrefix(child), 
                 dvRelationshipGetParentLabel(relationship));
    }

    dvWrtemp(dvFile,
        "        utAssert(!%0%3IsRed%1%2%3(current) || (\n"
        "            (%0%3GetLeft%1%2%3(current) == %5%3Null || !%0%3IsRed%1%2%3(%0%3GetLeft%1%2%3(current))) &&\n"
        "            (%0%3GetRight%1%2%3(current) == %5%3Null || !%0%3IsRed%1%2%3(%0%3GetRight%1%2%3(current)))));\n"
        "        utAssert(%0%3GetParent%1%2%3(current) != %5%3Null || %0%1GetRoot%2%3(%1) == current);\n"
        "        utAssert(%0%3GetParent%1%2%3(current) == %5%3Null ||\n"
        "            (%0%3GetLeft%1%2%3(%0%3GetParent%1%2%3(current)) == current &&\n"
        "            %0%3GetRight%1%2%3(%0%3GetParent%1%2%3(current)) != current) ||\n"
        "            (%0%3GetLeft%1%2%3(%0%3GetParent%1%2%3(current)) != current &&\n"
        "            %0%3GetRight%1%2%3(%0%3GetParent%1%2%3(current)) == current));\n"
        "        if(%0%3GetLeft%1%2%3(current) == %5%3Null || %0%3GetRight%1%2%3(current) == %5%3Null) {\n"
        "            counter = current;\n"
        "            currentblackcount = 0;\n"
        "            while(counter != %5%3Null) {\n"
        "                currentblackcount += !%0%3IsRed%1%2%3(counter);\n"
        "                counter = %0%3GetParent%1%2%3(counter);\n"
        "            }\n"
        "            if(treeblackcount == 0) {\n"
        "                treeblackcount = currentblackcount;\n"
        "            }\n"
        "            else {\n"
        "                utAssert(treeblackcount == currentblackcount);"
        "            }\n"
        "        }\n"
        "    }\n"
        "    %0End%1%2%3;\n"
        "}\n"
        "#endif\n"
        "\n"
         "/*----------------------------------------------------------------------------------------\n"
        "  Retreive the smallest %l3 in the Ordered List i.e. the first one\n"
        "----------------------------------------------------------------------------------------*/\n"
        "static %5%3 nodeFindMin%1%2%3(\n"
        "    %5%3 %3)\n"
        "{\n"
        "    while(%0%3GetLeft%1%2%3(%3) != %5%3Null) {\n"
        "        %3 = %0%3GetLeft%1%2%3(%3);\n"
        "    }\n"
        "    return %3;\n"
        "}\n"
        "\n"
        "/*----------------------------------------------------------------------------------------\n"
        "  Retreive the largest %l3 in the Ordered List i.e. the last one\n"
        "----------------------------------------------------------------------------------------*/\n"
        "static %5%3 nodeFindMax%1%2%3(\n"
        "    %5%3 %3)\n"
        "{\n"
        "    while(%0%3GetRight%1%2%3(%3) != %5%3Null) {\n"
        "        %3 = %0%3GetRight%1%2%3(%3);\n"
        "    }\n"
        "    return %3;\n"
        "}\n"
        "\n"
        "/*----------------------------------------------------------------------------------------\n"
        "  Return the first %2%3 in the %l1.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "%5%3 %0%1GetFirst%2%3(\n"
        "    %4%1 %1)\n"
        "{\n"
        "    %5%3 node = %0%1GetRoot%2%3(%1);\n"
        "\n"
        "    if(node == %5%3Null) {\n"
        "        return %5%3Null;\n"
        "    }\n"
        "    return nodeFindMin%1%2%3(%0%1GetRoot%2%3(%1));\n"
        "}\n"
        "\n"
        "/*----------------------------------------------------------------------------------------\n"
        "  Return the last %2%3 in the %l1.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "%5%3 %0%1GetLast%2%3(\n"
        "    %4%1 %1)\n"
        "{\n"
        "    %5%3 node = %0%1GetRoot%2%3(%1);\n"
        "\n"
        "    if(node == %5%3Null) {\n"
        "        return %5%3Null;\n"
        "    }\n"
        "    return nodeFindMax%1%2%3(%0%1GetRoot%2%3(%1));\n"
        "}\n"
        "\n"
        "/*----------------------------------------------------------------------------------------\n"
        "  Find the %3 after this one in the tree.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "%5%3 %0%3GetNext%1%2%3(\n"
        "    %5%3 %3)\n"
        "{\n"
        "    if(%0%3GetRight%1%2%3(%3) != %5%3Null) {\n"
        "        return nodeFindMin%1%2%3(%0%3GetRight%1%2%3(%3));\n"
        "    }\n"
        "    while(%0%3GetParent%1%2%3(%3) != %5%3Null && \n"
        "            %0%3GetRight%1%2%3(%0%3GetParent%1%2%3(%3)) == %3) {\n"
        "        %3 = %0%3GetParent%1%2%3(%3);\n"
        "    }\n"
        "    return %0%3GetParent%1%2%3(%3);\n"
        "}\n"
        "\n"
        "/*----------------------------------------------------------------------------------------\n"
        "  Find the %3 before this one in the tree.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "%5%3 %0%3GetPrev%1%2%3(\n"
        "    %5%3 %3)\n"
        "{\n"
        "    if(%0%3GetLeft%1%2%3(%3) != %5%3Null) {\n"
        "        return nodeFindMax%1%2%3(%0%3GetLeft%1%2%3(%3));\n"
        "    }\n"
        "    while(%0%3GetParent%1%2%3(%3) != %5%3Null && \n"
        "            %0%3GetLeft%1%2%3(%0%3GetParent%1%2%3(%3)) == %3) {\n"
        "        %3 = %0%3GetParent%1%2%3(%3);\n"
        "    }\n"
        "    return %0%3GetParent%1%2%3(%3);\n"
        "}\n"
        "\n"
        "/*----------------------------------------------------------------------------------------\n"
        "  Rotate a node left.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "static %5%3 rotateLeft%1%2%3(\n"
        "    %4%1 %1,\n"
        "    %5%3 current)\n"
        "{\n"
        "    %5%3 parent = %0%3GetParent%1%2%3(current);\n"
        "    %5%3 right = %0%3GetRight%1%2%3(current);\n"
        "    %5%3 middleChild = %0%3GetLeft%1%2%3(right);\n"
        "\n"
        "    utAssert(current != parent);\n"
        "    utAssert(current != right);\n"
        "    utAssert(current != middleChild);\n"
        "    utAssert(parent == %5%3Null || parent != right);\n"
        "    utAssert(parent == %5%3Null || parent != middleChild);\n"
        "    utAssert(right == %5%3Null || right != middleChild);\n"
        "\n"
        "    if(parent != %5%3Null) {\n"
        "        if(%0%3GetLeft%1%2%3(parent) == current) {\n"
        "            %0%3SetLeft%1%2%3(parent, right);\n"
        "        } else {\n"
        "            %0%3SetRight%1%2%3(parent, right);\n"
        "        }\n"
        "    }\n"
        "    else {\n"
        "        %0%3SetIsRed%1%2%3(right, false);\n"
        "        %0%1SetRoot%2%3(%1, right);\n"
        "    }\n"
        "    %0%3SetParent%1%2%3(right, parent);\n"
        "    %0%3SetRight%1%2%3(current, middleChild);\n"
        "    if(middleChild != %5%3Null) {\n"
        "        %0%3SetParent%1%2%3(middleChild, current);\n"
        "    }\n"
        "    %0%3SetLeft%1%2%3(right, current);\n"
        "    %0%3SetParent%1%2%3(current, right);\n"
        "    return right;\n"
        "}\n"
        "\n"
        "/*----------------------------------------------------------------------------------------\n"
        "  Rotate a node right.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "static %5%3 rotateRight%1%2%3(\n"
        "    %4%1 %1,\n"
        "    %5%3 current)\n"
        "{\n"
        "    %5%3 parent = %0%3GetParent%1%2%3(current);\n"
        "    %5%3 left = %0%3GetLeft%1%2%3(current);\n"
        "    %5%3 middleChild = %0%3GetRight%1%2%3(left);\n"
        "\n"
        "    utAssert(current != parent);\n"
        "    utAssert(current != left);\n"
        "    utAssert(current != middleChild);\n"
        "    utAssert(parent == %5%3Null || parent != left);\n"
        "    utAssert(parent == %5%3Null || parent != middleChild);\n"
        "    utAssert(left == %5%3Null || left != middleChild);\n"
        "\n"
        "    if(parent != %5%3Null) {\n"
        "        if(%0%3GetLeft%1%2%3(parent) == current) {\n"
        "            %0%3SetLeft%1%2%3(parent, left);\n"
        "        } else {\n"
        "            %0%3SetRight%1%2%3(parent, left);\n"
        "        }\n"
        "    }\n"
        "    else {\n"
        "        %0%3SetIsRed%1%2%3(left, false);\n"
        "        %0%1SetRoot%2%3(%1, left);\n"
        "    }\n"
        "    %0%3SetParent%1%2%3(left, parent);\n"
        "    %0%3SetLeft%1%2%3(current, middleChild);\n"
        "    if(middleChild != %5%3Null) {\n"
        "        %0%3SetParent%1%2%3(middleChild, current);\n"
        "    }\n"
        "    %0%3SetParent%1%2%3(current, left);\n"
        "    %0%3SetRight%1%2%3(left, current);\n"
        "    return left;\n"
        "}\n"
        "\n"
        "/*----------------------------------------------------------------------------------------\n"
        "  Return the grandparent of the node.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "static %5%3 getGrandparent%1%2%3(\n"
        "    %5%3 n)\n"
        "{\n"
        "    if(n != %5%3Null && %0%3GetParent%1%2%3(n) != %5%3Null) {\n"
        "        utAssert(n != %0%3GetParent%1%2%3(n));\n"
        "        utAssert(n != %0%3GetParent%1%2%3(%0%3GetParent%1%2%3(n)));\n"
        "        utAssert(%0%3GetParent%1%2%3(n) != %0%3GetParent%1%2%3(%0%3GetParent%1%2%3(n)));\n"
        "        return %0%3GetParent%1%2%3(%0%3GetParent%1%2%3(n));\n"
        "    } else {\n"
        "        return %5%3Null;\n"
        "    }\n"
        "}\n"
        " \n"
        "/*----------------------------------------------------------------------------------------\n"
        "  Return the uncle of the node.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "static %5%3 getUncle%1%2%3(\n"
        "    %5%3 n)\n"
        "{\n"
        "    %5%3 g = getGrandparent%1%2%3(n);\n"
        "\n"
        "    if(g == %5%3Null) {\n"
        "        return %5%3Null;\n"
        "    }\n"
        "    utAssert(n != %0%3GetLeft%1%2%3(g));\n"
        "    utAssert(n != %0%3GetRight%1%2%3(g));\n"
        "    if(%0%3GetParent%1%2%3(n) == %0%3GetLeft%1%2%3(g)) {\n"
        "        return %0%3GetRight%1%2%3(g);\n"
        "    } else {\n"
        "        return %0%3GetLeft%1%2%3(g);\n"
        "    }\n"
        "}\n"
        "\n"
        "/*----------------------------------------------------------------------------------------\n"
        "  Return the sibling of the node.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "static %5%3 getSibling%1%2%3(\n"
        "    %5%3 n)\n"
        "{\n"
        "    %5%3 parent = %0%3GetParent%1%2%3(n);\n"
        "\n"
        "    if(n == %0%3GetLeft%1%2%3(parent)) {\n"
        "        utAssert(n != %0%3GetRight%1%2%3(parent));\n"
        "        return %0%3GetRight%1%2%3(parent);\n"
        "    }\n"
        "    return %0%3GetLeft%1%2%3(parent);\n"
        "}\n"
        "\n"
        "/*----------------------------------------------------------------------------------------\n"
        "  Rebalance the tree from the leaf node, making sure red-black properties are preserved.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "static void fixupNode%1%2%3(\n"
        "    %4%1 %1,\n"
        "    %5%3 n)\n"
        "{\n"
        "case1:\n"
        "    if(%0%3GetParent%1%2%3(n) == %5%3Null) {\n"
        "        %0%3SetIsRed%1%2%3(n, false);\n"
        "        %0%1SetRoot%2%3(%1, n);\n"
        "    } else {\n"
        "        if(!%0%3IsRed%1%2%3(%0%3GetParent%1%2%3(n))) {\n"
        "            return; /* Tree is still valid */\n"
        "        } else {\n"
        "            %5%3 u = getUncle%1%2%3(n), g;\n"
        "            if(u != %5%3Null && %0%3IsRed%1%2%3(u)) {\n"
        "                %0%3SetIsRed%1%2%3(%0%3GetParent%1%2%3(n), false);\n"
        "                %0%3SetIsRed%1%2%3(u, false);\n"
        "                g = getGrandparent%1%2%3(n);\n"
        "                %0%3SetIsRed%1%2%3(g, true);\n"
        "                n = g;\n"
        "                goto case1;\n"
        "            } else {\n"
        "                %5%3 g = getGrandparent%1%2%3(n);\n"
        "                if(%0%3GetParent%1%2%3(g) == %5%3Null) {\n"
        "                    %0%3SetIsRed%1%2%3(g, false);\n"
        "                    %0%1SetRoot%2%3(%1, g);\n"
        "                }\n"
        "                if(n == %0%3GetRight%1%2%3(%0%3GetParent%1%2%3(n)) &&\n"
        "                        %0%3GetParent%1%2%3(n) == %0%3GetLeft%1%2%3(g)) {\n"
        "                    rotateLeft%1%2%3(%1, %0%3GetParent%1%2%3(n));\n"
        "                    n = %0%3GetLeft%1%2%3(n);\n"
        "                } else if(n == %0%3GetLeft%1%2%3(%0%3GetParent%1%2%3(n)) &&\n"
        "                        %0%3GetParent%1%2%3(n) == %0%3GetRight%1%2%3(g)) {\n"
        "                    rotateRight%1%2%3(%1, %0%3GetParent%1%2%3(n));\n"
        "                    n = %0%3GetRight%1%2%3(n);\n"
        "                }\n"
        "                g = getGrandparent%1%2%3(n);\n"
        "                %0%3SetIsRed%1%2%3(%0%3GetParent%1%2%3(n), false);\n"
        "                %0%3SetIsRed%1%2%3(g, true);\n"
        "                if(n == %0%3GetLeft%1%2%3(%0%3GetParent%1%2%3(n)) &&\n"
        "                        %0%3GetParent%1%2%3(n) == %0%3GetLeft%1%2%3(g)) {\n"
        "                    n = rotateRight%1%2%3(%1, g);\n"
        "                } else {\n"
        "                    n = rotateLeft%1%2%3(%1, g);\n"
        "                }\n"
        "            }\n"
        "        }\n"
        "    }\n"
        "}\n"
        "\n"
        "/*----------------------------------------------------------------------------------------\n"
        "  Insert a node as a child of this one.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "static void nodeInsert%1%2%3(\n"
        "    %4%1 %1,\n"
        "    %5%3 current,\n"
        "    %5%3 toinsert)\n"
        "{\n"
        "    %5%3 leaf;\n"
        "    int comparison;\n"
        "\n"
        "    %0%3SetIsRed%1%2%3(toinsert, true);\n"
        "    do {\n"
        "        utAssert(current != toinsert);\n"
        "        leaf = current;\n"
        "        comparison = %0%1Compare%2%3(current, toinsert);\n"
        "        if(comparison < 0) {\n"
        "            current = %0%3GetLeft%1%2%3(current);\n"
        "        }\n"
        "        else /* if(comparison > 0) */ {\n"
        "            current = %0%3GetRight%1%2%3(current);\n"
        "        }\n"
        "        /* else {\n"
        "            current = %5%3Null;\n"
        "        } */\n"
        "    } while(current != %5%3Null);\n"
        "    /* do {\n"
        "        current = %0%3GetRight%1%2%3(leaf);\n"
        "        if(current != %5%3Null) {\n"
        "            utAssert(current != toinsert);\n"
        "            comparison = %0%1Compare%2%3(current, toinsert);\n"
        "            if(comparison == 0) {\n"
        "                leaf = current;\n"
        "                continue;\n"
        "            }\n"
        "        }\n"
        "        break;\n"
        "    } while(1); */\n"
        "    if(comparison < 0) {\n"
        "        current = %0%3GetLeft%1%2%3(leaf);\n"
        "        %0%3SetLeft%1%2%3(leaf, toinsert);\n"
        "    }\n"
        "    else {\n"
        "        current = %0%3GetRight%1%2%3(leaf);\n"
        "        %0%3SetRight%1%2%3(leaf, toinsert);\n"
        "    }\n"
        "    %0%3SetParent%1%2%3(toinsert, leaf);\n"
        "    /* if(current != %5%3Null) {\n"
        "        utAssert(current != toinsert);\n"
        "        comparison = %0%1Compare%2%3(current, toinsert);\n"
        "        if(comparison < 0) {\n"
        "            %0%3SetLeft%1%2%3(toinsert, current);\n"
        "        }\n"
        "        else {\n"
        "            %0%3SetRight%1%2%3(toinsert, current);\n"
        "        }\n"
        "        fixupNode%1%2%3(%1, current);\n"
        "    }\n"
        "    else */ {\n"
        "        fixupNode%1%2%3(%1, toinsert);\n"
        "    }\n"
        "}\n"
        "\n"
        "/*----------------------------------------------------------------------------------------\n"
        "  Insert a node into a %l1.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "void %0%1Insert%2%3(\n"
        "    %4%1 %1,\n"
        "    %5%3 _%3)\n"
        "{\n"
        "    %5%3 root = %0%1GetRoot%2%3(%1);\n"
        "\n"
        "    utAssert(root != _%3);\n",
        dvPrefix, dvClassGetName(parent), dvRelationshipGetChildLabel(relationship), 
             dvClassGetName(child), dvClassGetPrefix(parent), dvClassGetPrefix(child), 
             dvRelationshipGetParentLabel(relationship));
    if(dvRelationshipAccessParent(relationship)) {
        dvWrtemp(dvFile,
            "    utAssert(%0%2Get%3%1(_%2) == %4%1Null);\n"
            "    %0%2Set%3%1(_%2, %1);\n",
            dvPrefix, dvClassGetName(parent), dvClassGetName(child), dvRelationshipGetParentLabel(relationship));
    }
    dvWrtemp(dvFile,
        "    utAssert(%0%3GetParent%1%2%3(_%3) == %5%3Null);\n"
        "    utAssert(%0%3GetLeft%1%2%3(_%3) == %5%3Null);\n"
        "    utAssert(%0%3GetRight%1%2%3(_%3) == %5%3Null);\n"
        "    %0%3SetParent%1%2%3(_%3, %5%3Null);\n"
        "    %0%3SetLeft%1%2%3(_%3, %5%3Null);\n"
        "    %0%3SetRight%1%2%3(_%3, %5%3Null);\n"
        "    if(root != %5%3Null) {\n"
        "        nodeInsert%1%2%3(%1, root, _%3);\n"
        "    }\n"
        "    else {\n"
        "        %0%1SetRoot%2%3(%1, _%3);\n"
        "        %0%3SetIsRed%1%2%3(_%3, false);\n"
        "    }\n"
        "}\n"
        "\n"
        "/*----------------------------------------------------------------------------------------\n"
        "  Swap the two node's positions in the tree, and their colors.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "static void swapNodes%1%2%3(\n"
        "    %5%3 node1,\n"
        "    %5%3 node2)\n"
        "{\n"
        "    %5%3 parent1 = %0%3GetParent%1%2%3(node1);\n"
        "    %5%3 left1 = %0%3GetLeft%1%2%3(node1);\n"
        "    %5%3 right1 = %0%3GetRight%1%2%3(node1);\n"
        "    uint32 isred1 = %0%3IsRed%1%2%3(node1);\n"
        "    %5%3 parent2 = %0%3GetParent%1%2%3(node2);\n"
        "    %5%3 left2 = %0%3GetLeft%1%2%3(node2);\n"
        "    uint32 isred2 = %0%3IsRed%1%2%3(node2);\n"
        "\n"
        "    if(left1 != node2) {\n"
        "        %0%3SetParent%1%2%3(node1, parent2);\n"
        "        %0%3SetLeft%1%2%3(node2, left1);\n"
        "        if(left1 != %5%3Null) {\n"
        "            %0%3SetParent%1%2%3(left1, node2);\n"
        "        }\n"
        "        if(%0%3GetLeft%1%2%3(parent2) == node2) {\n"
        "            %0%3SetLeft%1%2%3(parent2, node1);\n"
        "        } else {\n"
        "            %0%3SetRight%1%2%3(parent2, node1);\n"
        "        }\n"
        "    } else {\n"
        "        %0%3SetParent%1%2%3(node1, node2);\n"
        "        %0%3SetLeft%1%2%3(node2, node1);\n"
        "    }\n"
        "    %0%3SetLeft%1%2%3(node1, left2);\n"
        "    %0%3SetRight%1%2%3(node1, %5%3Null);\n"
        "    %0%3SetIsRed%1%2%3(node1, isred2);\n"
        "    %0%3SetParent%1%2%3(node2, parent1);\n"
        "    %0%3SetRight%1%2%3(node2, right1);\n"
        "    %0%3SetIsRed%1%2%3(node2, isred1);\n"
        "    if(right1 != %5%3Null) {\n"
        "        %0%3SetParent%1%2%3(right1, node2);\n"
        "    }\n"
        "    if(left2 != %5%3Null) {\n"
        "        %0%3SetParent%1%2%3(left2, node1);\n"
        "    }\n"
        "    if(parent1 != %5%3Null) {\n"
        "        if(%0%3GetLeft%1%2%3(parent1) == node1) {\n"
        "            %0%3SetLeft%1%2%3(parent1, node2);\n"
        "        } else {\n"
        "            %0%3SetRight%1%2%3(parent1, node2);\n"
        "        }\n"
        "    }\n"
        "}\n"
        "\n"
        "/*----------------------------------------------------------------------------------------\n"
        "  After deleting a node with at most one child, we must fixup the black path counts.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "static %5%3 fixupAfterDelete%1%2%3(\n"
        "    %4%1 %1,\n"
        "    %5%3 n)\n"
        "{\n"
        "    %5%3 retval = n;\n"
        "\n"
        "    /* delete_case1(child): */\n"
        "    /* this loop performs tail recursion on delete_case1(n) */\n"
        "    for(;;) {\n"
        "        /* delete_case1(n): */\n"
        "        if(%0%3GetParent%1%2%3(n) != %5%3Null) {\n"
        "            /* delete_case2(n): */\n"
        "            %5%3 s;\n"
        "            s = getSibling%1%2%3(n);\n"
        "            /* Note: sibling and it's children must exist, since it has one greater black count than n, and n is\n"
        "               black */\n"
        "            if(%0%3IsRed%1%2%3(s)) {\n"
        "                %0%3SetIsRed%1%2%3(%0%3GetParent%1%2%3(n), true);\n"
        "                %0%3SetIsRed%1%2%3(s, false);\n"
        "                if(n == %0%3GetLeft%1%2%3(%0%3GetParent%1%2%3(n))) {\n"
        "                    retval = rotateLeft%1%2%3(%1, %0%3GetParent%1%2%3(n));\n"
        "                } else {\n"
        "                    retval = rotateRight%1%2%3(%1, %0%3GetParent%1%2%3(n));\n"
        "                }\n"
        "            }\n"
        "            /* delete_case3(n): */\n"
        "            s = getSibling%1%2%3(n);\n"
        "            if((!%0%3IsRed%1%2%3(%0%3GetParent%1%2%3(n))) &&\n"
        "                   (!%0%3IsRed%1%2%3(s)) &&\n"
        "                   (%0%3GetLeft%1%2%3(s) == %5%3Null || !%0%3IsRed%1%2%3(%0%3GetLeft%1%2%3(s))) &&\n"
        "                   (%0%3GetRight%1%2%3(s) == %5%3Null || !%0%3IsRed%1%2%3(%0%3GetRight%1%2%3(s)))) {\n"
        "                %0%3SetIsRed%1%2%3(s, true);\n"
        "                /* delete_case1(%0%3GetParent%1%2%3(n)): */\n"
        "                n = %0%3GetParent%1%2%3(n);\n"
        "                continue; /* tail recursion loop */\n"
        "            } else {\n"
        "                /* delete_case4(n): */\n"
        "                if((%0%3IsRed%1%2%3(%0%3GetParent%1%2%3(n))) &&\n"
        "                       (!%0%3IsRed%1%2%3(s)) &&\n"
        "                       (%0%3GetLeft%1%2%3(s) == %5%3Null || !%0%3IsRed%1%2%3(%0%3GetLeft%1%2%3(s))) &&\n"
        "                       (%0%3GetRight%1%2%3(s) == %5%3Null || !%0%3IsRed%1%2%3(%0%3GetRight%1%2%3(s)))) {\n"
        "                    %0%3SetIsRed%1%2%3(s, true);\n"
        "                    %0%3SetIsRed%1%2%3(%0%3GetParent%1%2%3(n), false);\n"
        "                    return retval; /* terminate tail recusion */\n"
        "                } else {\n"
        "                    /* delete_case5(n): */\n"
        "                    if((n == %0%3GetLeft%1%2%3(%0%3GetParent%1%2%3(n))) &&\n"
        "                           (!%0%3IsRed%1%2%3(s)) &&\n"
        "                           (%0%3GetLeft%1%2%3(s) != %5%3Null && %0%3IsRed%1%2%3(%0%3GetLeft%1%2%3(s))) &&\n"
        "                           (%0%3GetRight%1%2%3(s) == %5%3Null || !%0%3IsRed%1%2%3(%0%3GetRight%1%2%3(s)))) {\n"
        "                        %0%3SetIsRed%1%2%3(s, true);\n"
        "                        %0%3SetIsRed%1%2%3(%0%3GetLeft%1%2%3(s), false);\n"
        "                        retval = rotateRight%1%2%3(%1, s);\n"
        "                    } else if((n == %0%3GetRight%1%2%3(%0%3GetParent%1%2%3(n))) &&\n"
        "                           (!%0%3IsRed%1%2%3(s)) &&\n"
        "                           (%0%3GetRight%1%2%3(s) != %5%3Null && %0%3IsRed%1%2%3(%0%3GetRight%1%2%3(s))) &&\n"
        "                           (%0%3GetLeft%1%2%3(s) == %5%3Null || !%0%3IsRed%1%2%3(%0%3GetLeft%1%2%3(s)))) {\n"
        "                        %0%3SetIsRed%1%2%3(s, true);\n"
        "                        %0%3SetIsRed%1%2%3(%0%3GetRight%1%2%3(s), false);\n"
        "                        retval = rotateLeft%1%2%3(%1, s);\n"
        "                    }\n"
        "                    /* delete_case6(n): */\n"
        "                    s = getSibling%1%2%3(n);\n"
        "                    %0%3SetIsRed%1%2%3(s, %0%3IsRed%1%2%3(%0%3GetParent%1%2%3(n)));\n"
        "                    %0%3SetIsRed%1%2%3(%0%3GetParent%1%2%3(n), false);\n"
        "                    if(n == %0%3GetLeft%1%2%3(%0%3GetParent%1%2%3(n))) {\n"
        "                        /*\n"
        "                        * Here, %0%3IsRed%1%2%3(%0%3GetRight%1%2%3(s)).\n"
        "                        */\n"
        "                        %0%3SetIsRed%1%2%3(%0%3GetRight%1%2%3(s), false);\n"
        "                        retval = rotateLeft%1%2%3(%1, %0%3GetParent%1%2%3(n));\n"
        "                    } else {\n"
        "                        /*\n"
        "                        * Here, %0%3IsRed%1%2%3(%0%3GetLeft%1%2%3(s)).\n"
        "                        */\n"
        "                        %0%3SetIsRed%1%2%3(%0%3GetLeft%1%2%3(s), false);\n"
        "                        retval = rotateRight%1%2%3(%1, %0%3GetParent%1%2%3(n));\n"
        "                    }\n"
        "                    return retval; /* terminate tail recursion */\n"
        "                }\n"
        "            }\n"
        "        } else {\n"
        "            return retval; /* terminate tail recursion */\n"
        "        }\n"
        "    }\n"
        "    return %5%3Null; /* Dummy return */\n"
        "}\n"
        "\n"
        "/*----------------------------------------------------------------------------------------\n"
        "  Remove the %2%3 from the %1.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "void %0%1Remove%2%3(\n"
        "    %4%1 %1,\n"
        "    %5%3 _%3)\n"
        "{\n"
        "    %5%3 parent, child, topModifiedNode;\n"
        "    %5%3 left = %0%3GetLeft%1%2%3(_%3);\n"
        "    %5%3 right = %0%3GetRight%1%2%3(_%3);\n"
        "\n"
        "    if(left != %5%3Null && right != %5%3Null) {\n"
        "        child = nodeFindMax%1%2%3(left);\n"
        "        if(%0%1GetRoot%2%3(%1) == _%3) {\n"
        "            %0%1SetRoot%2%3(%1, child);\n"
        "        }\n"
        "        swapNodes%1%2%3(_%3, child);\n"
        "        child = %0%3GetLeft%1%2%3(_%3);\n"
        "    } else {\n"
        "        child = left == %5%3Null? right : left;\n"
        "        if(%0%1GetRoot%2%3(%1) == _%3) {\n"
        "            %0%1SetRoot%2%3(%1, child);\n"
        "        }\n"
        "    }\n"
        "    topModifiedNode = _%3;\n"
        "    if(!%0%3IsRed%1%2%3(_%3)) {\n"
        "        if(child != %5%3Null && %0%3IsRed%1%2%3(child)) {\n"
        "            %0%3SetIsRed%1%2%3(child, false);\n"
        "        } else {\n"
        "            topModifiedNode = fixupAfterDelete%1%2%3(%1, _%3);\n"
        "        }\n"
        "    }\n"
        "    if(%0%3GetParent%1%2%3(topModifiedNode) == %5%3Null) {\n"
        "        if(topModifiedNode == _%3) {\n"
        "            %0%1SetRoot%2%3(%1, child);\n"
        "        } else {\n"
        "            %0%1SetRoot%2%3(%1, topModifiedNode);\n"
        "            %0%3SetIsRed%1%2%3(topModifiedNode, false);\n"
        "        }\n"
        "    }\n"
        "    parent = %0%3GetParent%1%2%3(_%3);\n"
        "    %0%3SetParent%1%2%3(_%3, %5%3Null);\n"
        "    %0%3SetLeft%1%2%3(_%3, %5%3Null);\n"
        "    %0%3SetRight%1%2%3(_%3, %5%3Null);\n"
        "    if(parent != %5%3Null) {\n"
        "        if(%0%3GetLeft%1%2%3(parent) == _%3) {\n"
        "            %0%3SetLeft%1%2%3(parent, child);\n"
        "        } else {\n"
        "            %0%3SetRight%1%2%3(parent, child);\n"
        "        }\n"
        "    }\n"
        "    if(child != %5%3Null) {\n"
        "        %0%3SetParent%1%2%3(child, parent);\n"
        "    }\n",
        dvPrefix, dvClassGetName(parent), dvRelationshipGetChildLabel(relationship), 
             dvClassGetName(child), dvClassGetPrefix(parent), dvClassGetPrefix(child), 
             dvRelationshipGetParentLabel(relationship));

    if(dvRelationshipAccessParent(relationship)) {
        dvWrtemp(dvFile,
            "    utAssert(%0%2Get%3%1(_%2) != %4%1Null);\n"
            "    %0%2Set%3%1(_%2, %4%1Null);\n",
            dvPrefix, dvClassGetName(parent), dvClassGetName(child), dvRelationshipGetParentLabel(relationship));
    }
    dvWrtemp(dvFile, "}\n\n");

    if(dvRelationshipAccessParent(relationship)) {
    dvWrtemp(dvFile,
        "/*----------------------------------------------------------------------------------------\n"
        "  Replaces and returns %5%3 equals to _%3 by _%3 in %1.\n"
        "  %5%3 from %1 tree will become part of _%3 tree if exists, otherwise removed.\n"
        "  %5%3 is different then %5%3Null iff nodes have been exchanged.\n"
        "  %5%3 is %5%3Null if _%3 is inserted or already in that tree.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "%5%3 %0%1Exchange%2%3(\n"
        "    %4%1 %1,\n"
        "    %5%3 _%3)\n"
        "{\n"
        "    %5%3 current, leaf = %5%3Null;\n"
        "    int comparison;\n"
        "\n"
        "    if(%0%3Get%6%1(_%3) == %1) {\n"
        "        return %5%3Null;\n"
        "    }\n"
        "\n"
        "    current = %0%1GetRoot%2%3(%1);\n"
        "    if(current == %5%3Null) {\n"
        "        /* transfert */\n"
        "        if(%0%3Get%6%1(_%3) != %4%1Null) {\n"
        "            %0%1Remove%2%3(%0%3Get%6%1(_%3), _%3);\n"
        "        }\n"
        "        %0%1SetRoot%2%3(%1, _%3);\n"
        "        %0%3SetIsRed%1%2%3(_%3, false);\n"
        "    }\n"
        "    else {\n"
        "        do {\n"
        "            utAssert(current != _%3);\n"
        "            leaf = current;\n"
        "            comparison = %0%1Compare%2%3(current, _%3);\n"
        "            if(comparison < 0) {\n"
        "                current = %0%3GetLeft%1%2%3(current);\n"
        "            }\n"
        "            else if(comparison > 0) {\n"
        "                current = %0%3GetRight%1%2%3(current);\n"
        "            }\n"
        "            else {\n"
        "                current = %5%3Null;\n"
        "            }\n"
        "        } while(current != %5%3Null);\n"
        "        if(comparison == 0) {\n"
        "            /* exchange */\n"
        "            while(1) {\n"
        "                current = %0%3GetRight%1%2%3(leaf);\n"
        "                if(current != %5%3Null) {\n"
        "                    if(%0%1Compare%2%3(current, _%3) == 0) {\n"
        "                        leaf = current;\n"
        "                        break;\n"
        "                    }\n"
        "                }\n"
        "                break;"
        "            }\n"
        "            swapNodes%1%2%3(_%3, leaf);\n"
        "        }\n"
        "        else {\n"
        "            /* transfert */\n"
        "            if(%0%3Get%6%1(_%3) != %4%1Null) {\n"
        "                %0%1Remove%2%3(%0%3Get%6%1(_%3), _%3);\n"
        "            }\n"
        "            %0%3SetIsRed%1%2%3(_%3, true);\n"
        "            nodeInsert%1%2%3(%1, leaf, _%3);\n"
        "            leaf = %5%3Null;\n"
        "        }\n"
        "    }\n"
        "\n"
        "    return leaf;\n"
        "}\n\n"
        ,
         dvPrefix, dvClassGetName(parent), dvRelationshipGetChildLabel(relationship), 
             dvClassGetName(child), dvClassGetPrefix(parent), dvClassGetPrefix(child), 
             dvRelationshipGetParentLabel(relationship));
    }
    if(dvRelationshipGetFirstKey(relationship) != dvKeyNull) {
        writeOrderedListFindFunction(relationship);
        writeOrderedListFindPrevFunction(relationship);
        writeOrderedListFindNextFunction(relationship);
    }
}

/*--------------------------------------------------------------------------------------------------
  Declaration of local variables for destructor.
--------------------------------------------------------------------------------------------------*/
static bool declareClassChildren(
    dvClass theClass)
{
    dvRelationship relationship;
    dvClass childClass;
    dvRelationshipType type;
    bool declaredSomething = false;

    dvForeachClassChildRelationship(theClass, relationship) {
        childClass = dvRelationshipGetChildClass(relationship);
        type = dvRelationshipGetType(relationship);
        if(needsChildrenProcessed(relationship)) {
            if(type == REL_LINKED_LIST || type == REL_DOUBLY_LINKED || type == REL_TAIL_LINKED ||
                    type == REL_HASHED || type == REL_ORDERED_LIST) {
                dvWrtemp(dvFile, 
                    "    %0%2 %1%2_;\n",
                    dvClassGetPrefix(childClass), dvRelationshipGetChildLabel(relationship),
                    dvClassGetName(childClass));
            } else if(type == REL_ARRAY || type == REL_HEAP) {
                dvWrtemp(dvFile, 
                    "    %0%2 %1%2_;\n"
                    "    uint32 x%1%2;\n",
                    dvClassGetPrefix(childClass), dvRelationshipGetChildLabel(relationship),
                    dvClassGetName(childClass));
            } else if(type == REL_POINTER) {
                dvWrtemp(dvFile, 
                    "    %0%2 %1%2_;\n", dvClassGetPrefix(childClass),
                    dvRelationshipGetChildLabel(relationship), dvClassGetName(childClass));
            }
            declaredSomething = true;
        }
    } dvEndClassChildRelationship;
    return declaredSomething;
}

/*--------------------------------------------------------------------------------------------------
  Declaration of local variables for rip.  Return true if we declare anything.
--------------------------------------------------------------------------------------------------*/
static bool declareClassParents(
    dvClass theClass)
{
    dvRelationship relationship;
    dvClass parentClass;
    bool declaredSomething = false;

    dvForeachClassParentRelationship(theClass, relationship) {
        if(dvRelationshipAccessParent(relationship) &&
                dvRelationshipAccessChild(relationship) &&
                !dvRelationshipSharedParent(relationship)) {
            parentClass = dvRelationshipGetParentClass(relationship);
            dvWrtemp(dvFile, 
                "    %0%1 owning%2%1 = %3%4Get%2%1(%4);\n",
                dvClassGetPrefix(parentClass), dvClassGetName(parentClass),
                dvRelationshipGetParentLabel(relationship), dvPrefix, dvClassGetName(theClass));
            declaredSomething = true;
        }
    } dvEndClassParentRelationship;
    return declaredSomething;
}

/*--------------------------------------------------------------------------------------------------
  Destroy the cascade delete children AND null out weak child parents.
--------------------------------------------------------------------------------------------------*/
static void processClassChildren(
    dvClass theClass)
{
    dvRelationship relationship;
    dvClass childClass;
    bool indented, endSafeLoop, needsValidTest;
    dvRelationshipType type;

    dvForeachClassChildRelationship(theClass, relationship) {
        childClass = dvRelationshipGetChildClass(relationship);
        if(needsChildrenProcessed(relationship)) {
            type = dvRelationshipGetType(relationship);
            indented = false;
            endSafeLoop = false;
            needsValidTest = true;
            if(type == REL_LINKED_LIST || type == REL_DOUBLY_LINKED || type == REL_TAIL_LINKED ||
                    type == REL_HASHED || type == REL_ORDERED_LIST) {
                dvWrtemp(dvFile, 
                    "    %0SafeForeach%1%2%3(%1, %2%3_) {\n",
                    dvPrefix, dvClassGetName(theClass), dvRelationshipGetChildLabel(relationship),
                    dvClassGetName(childClass));
                indented = true;
                endSafeLoop = true;
                needsValidTest = false;  /* test already in for loop */
            } else if(type == REL_ARRAY || type == REL_HEAP) {
                dvWrtemp(dvFile, 
                    "    for(x%1%2 = 0; x%1%2 < %4%3GetNum%1%2(%3); x%1%2++) {\n"
                    "        %1%2_ = %4%3Geti%1%2(%3, x%1%2);\n",
                    dvClassGetPrefix(childClass), dvRelationshipGetChildLabel(relationship),
                    dvClassGetName(childClass), dvClassGetName(theClass), dvPrefix);
                indented = true;
            } else if(type == REL_POINTER) {
                dvWrtemp(dvFile,
                    "    %1%2_ = %4%3Get%1%2(%3);\n",
                    dvClassGetPrefix(childClass), dvRelationshipGetChildLabel(relationship),
                    dvClassGetName(childClass), dvClassGetName(theClass), dvPrefix);
            }
            if(dvRelationshipCascade(relationship)) {
                dvWrtemp(dvFile, needsValidTest?
                    "%3    if(%1%2_ != %0%2Null) {\n"
                    "%3        %0%2Destroy(%1%2_);\n"
                    "%3    }\n"
                    : "%3    %0%2Destroy(%1%2_);\n",
                    dvClassGetPrefix(childClass), dvRelationshipGetChildLabel(relationship),
                    dvClassGetName(childClass), indented ? "    " : "");
            } else if(dvRelationshipAccessParent(relationship)) {
                dvWrtemp(dvFile, needsValidTest?
                    "%7    if(%5%2_ != %6%2Null) {\n"
                    "%7        %0%2Set%1%3(%5%2_, %4%3Null);\n"
                    "%7    }\n"
                    : "%7    %0%2Set%1%3(%5%2_, %4%3Null);\n",
                    dvPrefix, dvRelationshipGetParentLabel(relationship), dvClassGetName(childClass),
                    dvClassGetName(theClass), dvClassGetPrefix(theClass),
                    dvRelationshipGetChildLabel(relationship),
                    dvClassGetPrefix(childClass), indented ? "    " : "");
            }
            if(indented) {
                if(endSafeLoop) {
                    dvWrtemp(dvFile, 
                        "    } %0EndSafe%1%2%3;\n",
                        dvPrefix, dvClassGetName(theClass), dvRelationshipGetChildLabel(relationship),
                        dvClassGetName(childClass));
                } else {
                    dvWrtemp(dvFile, "    }\n");
                }
            }
        }
    } dvEndClassChildRelationship;
}

/*--------------------------------------------------------------------------------------------------
  Write code to remove the child object from the relationship.
--------------------------------------------------------------------------------------------------*/
static void removeChildFromRelationship(
    dvRelationship relationship)
{
    dvClass parentClass = dvRelationshipGetParentClass(relationship);
    dvClass childClass = dvRelationshipGetChildClass(relationship);

    if(dvRelationshipAccessParent(relationship) && dvRelationshipAccessChild(relationship)) {
        if(dvRelationshipGetType(relationship) != REL_POINTER) {
            dvWrtemp(dvFile,
                "    if(owning%5%2 != %0%2Null) {\n"
                "        %4%2Remove%1%3(owning%5%2, %3);\n",
                dvClassGetPrefix(parentClass), dvRelationshipGetChildLabel(relationship),
                dvClassGetName(parentClass), dvClassGetName(childClass), dvPrefix,
                dvRelationshipGetParentLabel(relationship));
        } else {
            dvWrtemp(dvFile, 
                "    if(owning%6%2 != %0%2Null) {\n"
                "        %4%2Set%1%3(owning%6%2, %5%3Null);\n",
                dvClassGetPrefix(parentClass), dvRelationshipGetChildLabel(relationship),
                dvClassGetName(parentClass), dvClassGetName(childClass), dvPrefix,
                dvClassGetPrefix(childClass), dvRelationshipGetParentLabel(relationship));
        }
        if(dvRelationshipMandatory(relationship)) {
            dvWrtemp(dvFile, 
                "#if defined(DD_DEBUG)\n"
                "    } else {\n"
                "        utExit(\"%2 without owning %1\");\n"
                "#endif\n",
                dvClassGetPrefix(parentClass), dvClassGetName(parentClass),
                dvClassGetName(childClass));
        }
        fprintf(dvFile, "    }\n");
    }
}

/*--------------------------------------------------------------------------------------------------
  Unlink the object from its parents.  Process hashed relationships first, because they may
  depend on the values of other pointers for keys.
--------------------------------------------------------------------------------------------------*/
static void processClassParents(
    dvClass theClass)
{
    dvRelationship relationship;

    dvForeachClassParentRelationship(theClass, relationship) {
        if(dvRelationshipGetType(relationship) == REL_HASHED) {
            removeChildFromRelationship(relationship);
        }
    } dvEndClassParentRelationship;
    dvForeachClassParentRelationship(theClass, relationship) {
        if(dvRelationshipGetType(relationship) != REL_HASHED) {
            removeChildFromRelationship(relationship);
        }
    } dvEndClassParentRelationship;
}

/*--------------------------------------------------------------------------------------------------
  Determine if the union has any cascade-delete properties.
--------------------------------------------------------------------------------------------------*/
static bool unionHasCascadeProperty(
    dvUnion theUnion)
{
    dvProperty property;

    dvForeachUnionProperty(theUnion, property) {
        if(dvPropertyCascade(property)) {
            return true;
        }
    } dvEndUnionProperty;
    return false;
}

/*--------------------------------------------------------------------------------------------------
  Write code to cascade delete through pointer properties.
--------------------------------------------------------------------------------------------------*/
static void processClassCascadeProperties(
    dvClass theClass)
{
    dvClass childClass;
    dvUnion theUnion;
    dvProperty property, typeProperty;
    dvCase theCase;

    dvForeachClassProperty(theClass, property) {
        if(dvPropertyCascade(property) && dvPropertyGetUnion(property) == dvUnionNull) {
            childClass = dvPropertyGetClassProp(property);
            dvWrtemp(dvFile,
                "    if(%0%1Get%2(%1) != %3%4Null) {\n"
                "        %3%4Destroy(%0%1Get%2(%1));\n"
                "    }\n",
                dvClassGetPrefix(theClass), dvClassGetName(theClass), dvPropertyGetName(property),
                dvClassGetPrefix(childClass), dvClassGetName(childClass));
        }
    } dvEndClassProperty;
    dvForeachClassUnion(theClass, theUnion) {
        if(unionHasCascadeProperty(theUnion)) {
            typeProperty = dvUnionGetTypeProperty(theUnion);
            dvWrtemp(dvFile,
                "    switch(%0%1Get%2(%1)) {\n",
                dvPrefix, dvClassGetName(theClass), dvPropertyGetName(typeProperty));
            dvForeachUnionProperty(theUnion, property) {
                if(dvPropertyCascade(property)) {
                    dvForeachPropertyCase(property, theCase) {
                        dvWrtemp(dvFile,
                            "    case %0:",
                            dvEntryGetName(dvCaseGetEntry(theCase)));
                    } dvEndPropertyCase;
                    childClass = dvPropertyGetClassProp(property);
                    dvWrtemp(dvFile,
                        "\n"
                        "        %3%4Destroy(%0%1Get%2(%1));\n"
                        "        break;\n",
                        dvPrefix, dvClassGetName(theClass), dvPropertyGetName(property),
                        dvClassGetPrefix(childClass), dvClassGetName(childClass));
                }
            } dvEndUnionProperty;
            dvWrtemp(dvFile,
                "    default:\n"
                "        break;\n"
                "    }\n");
        }
    } dvEndClassUnion;
}

/*--------------------------------------------------------------------------------------------------
  Write the destructor for theClass.
--------------------------------------------------------------------------------------------------*/
static void writeClassDestructor(
    dvClass theClass)
{
    bool isExtension = dvClassGetBaseClass(theClass) != dvClassNull;
    bool declaredSomething;

    if(dvClassGetMemoryStyle(theClass) == MEM_CREATE_ONLY) {
        return;
    }
    dvWrtemp(dvFile, 
        "/*----------------------------------------------------------------------------------------\n"
        "  Destroy %0 including everything in it. Remove from parents.\n"
        "----------------------------------------------------------------------------------------*/\n"
        , dvClassGetName(theClass));
    if(isExtension) {
        dvWrtemp(dvFile, 
            "static void destroy%1(\n"
            "    %2%1 %1)\n"
            "{\n",
            dvPrefix, dvClassGetName(theClass), dvClassGetPrefix(theClass));
    } else {
        dvWrtemp(dvFile, 
            "void %0%1Destroy(\n"
            "    %2%1 %1)\n"
            "{\n",
            dvPrefix, dvClassGetName(theClass), dvClassGetPrefix(theClass));
    }
    declaredSomething = declareClassChildren(theClass);
    declaredSomething |= declareClassParents(theClass);
    if(declaredSomething) {
        dvWrtemp(dvFile, "\n");
    }
    dvWrtemp(dvFile, 
        "    if(%0%1DestructorCallback != NULL) {\n"
        "        %0%1DestructorCallback(%1);\n"
        "    }\n",
        dvPrefix, dvClassGetName(theClass));
    processClassChildren(theClass);
    processClassParents(theClass);
    processClassCascadeProperties(theClass);
    if(!isExtension) {
        dvWrtemp(dvFile, 
        "    %0%1Free(%1);\n", dvPrefix, dvClassGetName(theClass));
    }
    fprintf(dvFile, "}\n\n");
}

/*--------------------------------------------------------------------------------------------------
  Write out constructor/destructor variable declarations.
--------------------------------------------------------------------------------------------------*/
static void writeClassConstructorDestructorVariables(
    dvClass theClass)
{
    char *name = dvClassGetName(theClass);

    dvWrtemp(dvFile,
        "%0%1CallbackType %0%1ConstructorCallback;\n",
        dvPrefix, name, dvClassGetPrefix(theClass));
    if(dvClassGetMemoryStyle(theClass) != MEM_CREATE_ONLY) {
        dvWrtemp(dvFile,
            "%0%1CallbackType %0%1DestructorCallback;\n",
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

    dvWrtemp(dvFile, 
        "/*----------------------------------------------------------------------------------------\n"
        "  Constructor/Destructor hooks.\n"
        "----------------------------------------------------------------------------------------*/\n"
        );
    dvForeachModuleClass(module, theClass) {
        writeClassConstructorDestructorVariables(theClass);
    } dvEndModuleClass;
    fputs("\n", dvFile);
}

/*--------------------------------------------------------------------------------------------------
  Write the file top portion.  Note that we write a slightly different include statment if we
  detect we are generating DatadrawUtil.  This breaks a loop that otherwise would have it
  including itself!
--------------------------------------------------------------------------------------------------*/
static void writeFileTop(
    dvModule module)
{
    dvClass theClass;
    char *includeFile = utSprintf("%sdatabase.h", dvPrefix);

    if(!strcmp(dvModuleGetName(module), "DatadrawUtil")) {
        includeFile = "ddutil.h"; /* This hack breaks a recursive include loop */
    }
    dvWrtemp(dvFile, 
        "/*----------------------------------------------------------------------------------------\n"
        "  Database %0\n"
        "----------------------------------------------------------------------------------------*/\n"
        "\n"
        "#include \"%1\"\n"
        "\n"
        "struct %0RootType_ %0RootData;\n",
        dvPrefix, includeFile);
    dvWrtemp(dvFile, "uint8 %0ModuleID;\n", dvPrefix);
    dvForeachModuleClass(module, theClass) {
        dvWrtemp(dvFile, "struct %0%1Fields %0%1s;\n", dvPrefix, dvClassGetName(theClass));
    } dvEndModuleClass;
    fputs("\n", dvFile);
    writeConstructorDestructorVariables(module);
}

/*--------------------------------------------------------------------------------------------------
  Write a function show all the fields of an object.
--------------------------------------------------------------------------------------------------*/
static void writeClassShowFunction(
    dvClass theClass)
{
    dvWrtemp(dvFile, 
        "#if defined(DD_DEBUG)\n"
        "/*----------------------------------------------------------------------------------------\n"
        "  Write out all the fields of an object.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "void %0Show%1(\n"
        "    %2%1 %1)\n"
        "{\n"
        "    utDatabaseShowObject(\"%0\", \"%1\", %2%12Index(%1));\n"
        "}\n"
        "#endif\n\n",
        dvPrefix, dvClassGetName(theClass), dvClassGetPrefix(theClass));
}

/*--------------------------------------------------------------------------------------------------
    See if this property is a part of the class or because of an owning/child
--------------------------------------------------------------------------------------------------*/
static bool propIsClassProp(
    dvProperty prop)
{
    if(dvPropertyArray(prop)) {
        return false;
    }
    if(dvPropertyHidden(prop)) {
        return false;
    }
    if(dvPropertyGetRelationship(prop) != dvRelationshipNull) {
        return false;
    }
    return true;
}

/*--------------------------------------------------------------------------------------------------
  Write the class's property access macros.
--------------------------------------------------------------------------------------------------*/
static void writeClassCopyPropFuncs(
    dvClass theClass)
{
    dvUnion theUnion;
    dvProperty prop;
    dvPropertyType propType;
    char *name = dvClassGetName(theClass);
    char *propName;
    
    dvWrtemp(dvFile, 
        "/*----------------------------------------------------------------------------------------\n"
        "  Copy the properties of %1.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "void %0%1CopyProps(\n"
        "    %2%1 old%1,\n"
        "    %2%1 new%1)\n"
        "{\n",
        dvPrefix, name, dvClassGetPrefix(theClass));
    dvForeachClassProperty(theClass, prop) {
        name = dvClassGetName(theClass);
        propName = dvPropertyGetName(prop);
        if(dvPropertyView(prop)) {
            dvWrtemp(dvFile, "    /* copy for %0 is irrelevant */\n",
                dvPropertyGetName(prop));
        }
        else if(propIsClassProp(prop)) {
            theUnion = dvPropertyGetUnion(prop);
            propType = dvPropertyGetType(prop);
            if(theUnion == dvUnionNull && propType != PROP_POINTER) {
                dvWrtemp(dvFile, "    %0%1Set%2(new%1, %0%1%3%2(old%1));\n", dvPrefix, name, propName,
                    ((propType == PROP_BOOL) || (propType == PROP_BIT))? "" : "Get");
            }
        }
    } dvEndClassProperty;
    if(dvClassGenerateAttributes(theClass)) {
        dvWrtemp(dvFile, "    %0%1CopyAttributes(old%1, new%1);\n", dvPrefix, name);
    }
    dvWrtemp(dvFile, "}\n\n");
}

/*--------------------------------------------------------------------------------------------------
    class has bit fields
--------------------------------------------------------------------------------------------------*/
static bool classHasBitfields(
    dvClass theClass)
{
    dvProperty prop;
    dvForeachClassProperty(theClass, prop) {
        if(dvPropertyGetType(prop) == PROP_BIT && !dvPropertyView(prop)) {
            return true;
        }
    } dvEndClassProperty;
    return false;
}

/*--------------------------------------------------------------------------------------------------
  Write the class bit field Get/Set Bitfields function.
--------------------------------------------------------------------------------------------------*/
static void writeClassBitfieldFuncs(
    dvClass theClass)
{
    dvProperty prop;
    dvUnion theUnion;
    char *name, *propName, *prefix;
    
    if(!classHasBitfields(theClass)) {
        return;
    }
    name = dvClassGetName(theClass);
    if(dvClassGetBaseClass(theClass) != dvClassNull) {
        prefix = dvClassGetPrefix(theClass);
    } else {
        prefix = dvPrefix;
    }
    dvWrtemp(dvFile, 
        "/*----------------------------------------------------------------------------------------\n"
        "  Return the integer equivalent for the bit fields in %1.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "uint32 %0%1GetBitfield(\n"
        "    %2%1 _%1)\n"
        "{\n"
        "    uint32 bitfield = 0;\n"
        "    uint8 xLevel = 0;\n\n", dvPrefix, name, prefix);
    dvForeachClassProperty(theClass, prop) {
        if(propIsClassProp(prop) && dvPropertyGetType(prop) == PROP_BIT && !dvPropertyView(prop)) {
            name = dvClassGetName(theClass);
            propName = dvPropertyGetName(prop);
            theUnion = dvPropertyGetUnion(prop);
            if(theUnion == dvUnionNull) {
                dvWrtemp(dvFile, "    bitfield |= %0%1%2(_%1) << xLevel++;\n", dvPrefix, name, propName);
            }
        }
    } dvEndClassProperty;
    dvWrtemp(dvFile, "    return bitfield;\n}\n\n");
    dvWrtemp(dvFile, 
        "/*----------------------------------------------------------------------------------------\n"
        "  Set bit fields in %1 using bitfield.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "void %0%1SetBitfield(\n"
        "    %2%1 _%1,\n"
        "     uint32 bitfield)\n"
        "{\n", dvPrefix, name, prefix);
    dvForeachClassProperty(theClass, prop) {
        if(propIsClassProp(prop) && dvPropertyGetType(prop) == PROP_BIT && !dvPropertyView(prop)) {
            name = dvClassGetName(theClass);
            propName = dvPropertyGetName(prop);
            theUnion = dvPropertyGetUnion(prop);
            if(theUnion == dvUnionNull) {
                dvWrtemp(dvFile, "    %0%1Set%2(_%1, bitfield & 1);\n", dvPrefix, name, propName);
                dvWrtemp(dvFile, "    bitfield >>= 1;\n");
            }
        }
    } dvEndClassProperty;
    dvWrtemp(dvFile, "}\n\n");
}

/*--------------------------------------------------------------------------------------------------
  Write the copy attrlist and attribute function.
--------------------------------------------------------------------------------------------------*/
static void writeCopyAttrlistFunction(void)
{
    char *upperPrefix = utStringToUpperCase(dvPrefix);

    dvWrtemp(dvFile,
        "/*----------------------------------------------------------------------------------------\n"
        "  Make a copy of the attrlist.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "%0Attribute %0CopyAttribute(\n"
        "    %0Attribute oldAttribute)\n"
        "{\n"
        "    %0Attribute newAttribute = %0AttributeAlloc();\n"
        "    %0AttributeType type = %0AttributeGetType(oldAttribute);\n"
        "\n"
        "    %0AttributeSetType(newAttribute, type);\n"
        "    switch(type) {\n"
        "    case %1_ATTRINT64:\n"
        "        %0AttributeSetInt64Val(newAttribute, %0AttributeGetInt64Val(oldAttribute));\n"
        "        break;\n"
        "    case %1_ATTRDOUBLE:\n"
        "        %0AttributeSetDoubleVal(newAttribute, %0AttributeGetDoubleVal(oldAttribute));\n"
        "        break;\n"
        "    case %1_ATTRBOOL:\n"
        "        %0AttributeSetBoolVal(newAttribute, %0AttributeBoolVal(oldAttribute));\n"
        "        break;\n"
        "    case %1_ATTRSYM:\n"
        "        %0AttributeSetSymVal(newAttribute, %0AttributeGetSymVal(oldAttribute));\n"
        "        break;\n"
        "    case %1_ATTRSTRING: case %1_ATTRBLOB:\n"
        "        if(%0AttributeGetNumData(oldAttribute) != 0) {\n"
        "            %0AttributeSetData(newAttribute, %0AttributeGetData(oldAttribute),\n"
        "                %0AttributeGetNumData(oldAttribute));\n"
        "        }\n"
        "        break;\n"
        "    default:\n"
        "        utExit(\"Unknown attribute type\");\n"
        "    }\n"
        "    return newAttribute;\n"
        "}\n"
        "\n"
        "/*----------------------------------------------------------------------------------------\n"
        "  Make a copy of the attrlist.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "%0Attrlist %0CopyAttrlist(\n"
        "    %0Attrlist oldAttrlist)\n"
        "{\n"
        "    %0Attrlist newAttrlist = %0AttrlistAlloc();\n"
        "    %0Attribute oldAttribute, newAttribute;\n"
        "\n"
        "    %0ForeachAttrlistAttribute(oldAttrlist, oldAttribute) {\n"
        "        newAttribute = %0CopyAttribute(oldAttribute);\n"
        "        %0AttrlistAppendAttribute(newAttrlist, newAttribute);\n"
        "    } %0EndAttrlistAttribute;\n"
        "    return newAttrlist;\n"
        "}\n\n",
        dvPrefix, upperPrefix);
}

/*--------------------------------------------------------------------------------------------------
  Write get/set wrappers for the attribute type.
--------------------------------------------------------------------------------------------------*/
static void writeAttributeAccess(
    dvClass theClass,
    char *type,
    char *typeName,
    char *nullVal,
    char *propType,
    char *accessString)
{
    char *entryName = utStringToUpperCase(utSprintf("%s_ATTR%s", dvPrefix, propType));

    dvWrtemp(dvFile,
        "/*----------------------------------------------------------------------------------------\n"
        "  Get the %1's %2 attribute value.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "%3 %0%1Get%2Attribute(\n"
        "    %7%1 _%1,\n"
        "    utSym sym)\n"
        "{\n"
        "    %0Attribute attribute = %0%1FindAttribute(_%1, (sym));\n"
        "\n"
        "    if(attribute == %0AttributeNull || %0AttributeGetType(attribute) != %5) {\n"
        "        return %4;\n"
        "    }\n"
        "    return %0Attribute%6%2Val(attribute);\n"
        "}\n"
        "\n"
        "/*----------------------------------------------------------------------------------------\n"
        "  Set the %1's %2 attribute value.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "void %0%1Set%2Attribute(\n"
        "    %7%1 _%1,\n"
        "    utSym sym,\n"
        "    %3 value)\n"
        "{\n"
        "    %0Attribute attribute = %0%1AttributeCreate(_%1, sym, %5);\n"
        "\n"
        "    %0AttributeSet%2Val(attribute, value);\n"
        "}\n\n",
        dvPrefix, dvClassGetName(theClass), typeName, type, nullVal, entryName, accessString,
        dvClassGetPrefix(theClass));
}

/*--------------------------------------------------------------------------------------------------
  Write get/set wrappers for string attributes.
--------------------------------------------------------------------------------------------------*/
static void writeAttributeStringAccess(
    dvClass theClass)
{
    char *upperPrefix = utStringToUpperCase(dvPrefix);

    dvWrtemp(dvFile,
        "/*----------------------------------------------------------------------------------------\n"
        "  Get the %1's string attribute value.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "char *%0%1GetStringAttribute(\n"
        "    %3%1 _%1,\n"
        "    utSym sym)\n"
        "{\n"
        "    %0Attribute attribute = %0%1FindAttribute(_%1, (sym));\n"
        "\n"
        "    if(attribute == %0AttributeNull || %0AttributeGetType(attribute) != %2_ATTRSTRING) {\n"
        "        return NULL;\n"
        "    }\n"
        "    return (char *)%0AttributeGetData(attribute);\n"
        "}\n"
        "\n"
        "/*----------------------------------------------------------------------------------------\n"
        "  Set the %1's string attribute value.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "void %0%1SetStringAttribute(\n"
        "    %3%1 _%1,\n"
        "    utSym sym,\n"
        "    char *string)\n"
        "{\n"
        "    %0Attribute attribute = %0%1AttributeCreate(_%1, sym, %2_ATTRSTRING);\n"
        "\n"
        "    %0AttributeSetData(attribute, (uint8 *)string, strlen(string) + 1);\n"
        "}\n\n",
        dvPrefix, dvClassGetName(theClass), upperPrefix, dvClassGetPrefix(theClass));
}

/*--------------------------------------------------------------------------------------------------
  Write get/set wrappers for blob attributes.
--------------------------------------------------------------------------------------------------*/
static void writeAttributeBlobAccess(
    dvClass theClass)
{
    char *upperPrefix = utStringToUpperCase(dvPrefix);

    dvWrtemp(dvFile,
        "/*----------------------------------------------------------------------------------------\n"
        "  Get the %1's blob attribute value.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "uint8 *%0%1GetBlobAttribute(\n"
        "    %3%1 _%1,\n"
        "    utSym sym,\n"
        "    uint32 *length)\n"
        "{\n"
        "    %0Attribute attribute = %0%1FindAttribute(_%1, (sym));\n"
        "\n"
        "    if(attribute == %0AttributeNull || %0AttributeGetType(attribute) != %2_ATTRBLOB) {\n"
        "        return NULL;\n"
        "    }\n"
        "    *length = %0AttributeGetNumData(attribute);\n"
        "    return %0AttributeGetData(attribute);\n"
        "}\n"
        "\n"
        "/*----------------------------------------------------------------------------------------\n"
        "  Set the %1's blob attribute value.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "void %0%1SetBlobAttribute(\n"
        "    %3%1 _%1,\n"
        "    utSym sym,\n"
        "    uint8 *data,"
        "    uint32 length)\n"
        "{\n"
        "    %0Attribute attribute = %0%1AttributeCreate(_%1, sym, %2_ATTRBLOB);\n"
        "\n"
        "    %0AttributeSetData(attribute, data, length);\n"
        "}\n\n",
        dvPrefix, dvClassGetName(theClass), upperPrefix, dvClassGetPrefix(theClass));
}

/*--------------------------------------------------------------------------------------------------
  Write functions for manipulating class attributes.
--------------------------------------------------------------------------------------------------*/
static void writeClassAttributeFunctions(
    dvClass theClass)
{
    dvWrtemp(dvFile,
        "/*----------------------------------------------------------------------------------------\n"
        "  Create the attribute on the %1.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "%0Attribute %0%1AttributeCreate(\n"
        "    %2%1 _%1,\n"
        "    utSym sym,\n"
        "    %0AttributeType type)\n"
        "{\n"
        "    %0Attrlist attrlist = %0%1GetAttrlist(_%1);\n"
        "    %0Attribute attribute;\n"
        "\n"
        "    if(attrlist == %0AttrlistNull) {\n"
        "        attrlist = %0AttrlistAlloc();\n"
        "        %0%1SetAttrlist(_%1, attrlist);\n"
        "    }\n"
        "    attribute = %0AttrlistFindAttribute(attrlist, sym);\n"
        "    if(attribute == %0AttributeNull) {\n"
        "        attribute = %0AttributeAlloc();\n"
        "        %0AttributeSetSym(attribute, sym);\n"
        "        %0AttrlistAppendAttribute(attrlist, attribute);\n"
        "    }\n"
        "    %0AttributeSetType(attribute, type);\n"
        "    return attribute;\n"
        "}\n"
        "\n"
        "/*----------------------------------------------------------------------------------------\n"
        "  Delete the attribute from the %1.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "void %0%1DeleteAttribute(\n"
        "    %2%1 _%1,\n"
        "    utSym sym)\n"
        "{\n"
        "    %0Attribute attribute = %0%1FindAttribute(_%1, sym);\n"
        "\n"
        "    if(attribute == %0AttributeNull) {\n"
        "        return;\n"
        "    }\n"
        "    %0AttributeDestroy(attribute);\n"
        "}\n"
        "\n"
        "/*----------------------------------------------------------------------------------------\n"
        "  Copy attributes from one %1 to another.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "void %0%1CopyAttributes(\n"
        "    %2%1 old%1,\n"
        "    %2%1 new%1)\n"
        "{\n"
        "    %0Attrlist oldAttrlist = %0%1GetAttrlist(old%1);\n"
        "    %0Attrlist newAttrlist;\n"
        "\n"
        "    if(oldAttrlist == %0AttrlistNull) {\n"
        "        return;\n"
        "    }\n"
        "    newAttrlist = %0CopyAttrlist(oldAttrlist);\n"
        "    %0%1SetAttrlist(new%1, newAttrlist);\n"
        "}\n\n",
        dvPrefix, dvClassGetName(theClass), dvClassGetPrefix(theClass));
    writeAttributeAccess(theClass, "int64", "Int64", "0", "INT64", "Get");
    writeAttributeAccess(theClass, "double", "Double", "0.0", "DOUBLE", "Get");
    writeAttributeAccess(theClass, "bool", "Bool", "false", "BOOL", "");
    writeAttributeAccess(theClass, "utSym", "Sym", "utSymNull", "SYM", "Get");
    writeAttributeStringAccess(theClass);
    writeAttributeBlobAccess(theClass);
}

/*--------------------------------------------------------------------------------------------------
  Just count the number of properties on a sparsegroup.
--------------------------------------------------------------------------------------------------*/
static uint32 countSparsegroupProperties(
    dvSparsegroup sparsegroup)
{
    dvProperty property;
    uint32 numProperties = 0;

    dvForeachSparsegroupProperty(sparsegroup, property) {
        numProperties++;
    } dvEndSparsegroupProperty;
    return numProperties;
}

/*--------------------------------------------------------------------------------------------------
  Write a function to set sparse variable values.
--------------------------------------------------------------------------------------------------*/
static void writeSparseFieldAccessFunction(
    dvProperty property)
{
    dvClass theClass = dvPropertyGetClass(property);
    dvPropertyType type = dvPropertyGetType(property);
    dvSparsegroup sparsegroup = dvPropertyGetSparsegroup(property);
    if(dvPropertyView(property)) {
    dvWrtemp(dvFile, "/* Getting and setting %0.%1 field is user provided */\n", 
        dvClassGetName(theClass), dvPropertyGetName(property));
        return;
    }
    dvWrtemp(dvFile,
        "/*----------------------------------------------------------------------------------------\n"
        "  Get the %1.%2 field.\n"
        "----------------------------------------------------------------------------------------*/\n"
        "%4 %0%1%7%2(\n"
        "    %3%1 %1)\n"
        "{\n"
        "    %0%5 object = %0DatadrawRootFind%5(%0FirstDatadrawRoot(), %1);\n"
        "\n"
        "    if(object == %0%5Null) {\n"
        "        return %6;\n"
        "    }\n"
        "    return %0%5%7%2(object);\n"
        "}\n\n",
        dvPrefix, dvClassGetName(theClass), dvPropertyGetName(property),
        dvClassGetPrefix(theClass), dvPropertyGetTypeName(property),
        dvSparsegroupGetName(sparsegroup), dvPropertyFindInitializer(property),
        type == PROP_BOOL || type == PROP_BIT? "" : "Get");
    if(countSparsegroupProperties(sparsegroup) > 1) {
        dvWrtemp(dvFile,
            "/*----------------------------------------------------------------------------------------\n"
            "  Set the %1.%2 field.\n"
            "----------------------------------------------------------------------------------------*/\n"
            "void %0%1Set%2(\n"
            "    %3%1 %1,\n"
            "    %4 value)\n"
            "{\n"
            "    %0DatadrawRoot theRoot = %0FirstDatadrawRoot();\n"
            "    %0%5 object = %0DatadrawRootFind%5(theRoot, %1);\n"
            "\n"
            "    if(value != %6 || object != %0%5Null) {\n"
            "        if(object == %0%5Null) {\n"
            "            object = %0%5Alloc();\n"
            "            %0%5Set%1Key(object, %1);\n"
            "            %0DatadrawRootInsert%5(theRoot, object);\n"
            "        }\n"
            "        %0%5Set%2(object, value);\n"
            "    }\n"
            "}\n\n",
            dvPrefix, dvClassGetName(theClass), dvPropertyGetName(property),
            dvClassGetPrefix(theClass), dvPropertyGetTypeName(property),
            dvSparsegroupGetName(sparsegroup), dvPropertyFindInitializer(property));
    } else {
        dvWrtemp(dvFile,
            "/*----------------------------------------------------------------------------------------\n"
            "  Set the %1.%2 field.\n"
            "----------------------------------------------------------------------------------------*/\n"
            "void %0%1Set%2(\n"
            "    %3%1 %1,\n"
            "    %4 value)\n"
            "{\n"
            "    %0DatadrawRoot theRoot = %0FirstDatadrawRoot();\n"
            "    %0%5 object = %0DatadrawRootFind%5(theRoot, %1);\n"
            "\n"
            "    if(value != %6) {\n"
            "        if(object == %0%5Null) {\n"
            "            object = %0%5Alloc();\n"
            "            %0%5Set%1Key(object, %1);\n"
            "            %0DatadrawRootInsert%5(theRoot, object);\n"
            "        }\n"
            "        %0%5Set%2(object, value);\n"
            "    } else if(object != %0%5Null) {\n"
            "        %0%5Destroy(object);\n"
            "    }\n"
            "}\n\n",
            dvPrefix, dvClassGetName(theClass), dvPropertyGetName(property),
            dvClassGetPrefix(theClass), dvPropertyGetTypeName(property),
            dvSparsegroupGetName(sparsegroup), dvPropertyFindInitializer(property));
    }
}

/*--------------------------------------------------------------------------------------------------
  Write functions to set sparse variable values.
--------------------------------------------------------------------------------------------------*/
static void writeSparseFieldAccessFunctions(
    dvClass theClass)
{
    dvProperty property;

    dvForeachClassProperty(theClass, property) {
        if(dvPropertySparse(property)) {
            writeSparseFieldAccessFunction(property);
        }
    } dvEndClassProperty;
}

/*--------------------------------------------------------------------------------------------------
  Write the database support file.
--------------------------------------------------------------------------------------------------*/
void dvWriteCFile(
    dvModule module,
    char *sourceFile)
{
    dvClass theClass;
    dvRelationship relationship;
    dvRelationshipType type;
    char *fileName = sourceFile;
    bool firstAttribute = true;

    if(utDirectoryExists(sourceFile)) {
        fileName = utSprintf("%s%c%sdatabase.c", sourceFile, UTDIRSEP,
            dvModuleGetPrefix(module));
    }
    dvFile = fopen(fileName, "wt");
    if(dvFile == NULL) {
        utError("Could not open file %s", fileName);
    }
    utLogMessage("Generating C file %s", fileName);
    dvPrefix = dvModuleGetPrefix(module);
    writeFileTop(module);
    dvForeachModuleClass(module, theClass) {
        writeClassDestructor(theClass);
        if(dvClassGetBaseClass(theClass) != dvClassNull) {
            writeExtendedClassInitFunc(theClass);
        } else {
            writeConstructorDestructorWrappers(theClass);
        }
        writeClassAllocs(theClass);
        writeClassReallocs(theClass);
        if(dvClassGetBaseClass(theClass) == dvClassNull) {
            writeClassAllocateMore(theClass);
        }
        writeClassPropertyArrayFunctions(theClass);
        writeClassCopyPropFuncs(theClass);
        writeClassBitfieldFuncs(theClass);
        if(dvClassGenerateAttributes(theClass)) {
            if(firstAttribute) {
                writeCopyAttrlistFunction();
                firstAttribute = false;
            }
            writeClassAttributeFunctions(theClass);
        }
        writeSparseFieldAccessFunctions(theClass);
        dvForeachClassChildRelationship(theClass, relationship) {
            if(dvRelationshipAccessChild(relationship)) {
                type = dvRelationshipGetType(relationship);
                if(type == REL_LINKED_LIST || type == REL_DOUBLY_LINKED || type == REL_TAIL_LINKED ||
                        type == REL_HASHED) {
                    if(type == REL_HASHED) {
                        writeHashTableResizeFunction(relationship);
                        writeHashTableAddFunction(relationship);
                        writeHashTableRemoveFunction(relationship);
                        writeFindFunction(relationship);
                        if(dvRelationshipHashedByName(relationship)) {
                            writeRenameFunction(relationship);
                        }
                    }
                    writeClassInsertFunction(theClass, relationship);
                    if(type != REL_HASHED || !dvRelationshipUnordered(relationship)) {
                        if(type != REL_LINKED_LIST) {
                           writeClassAppendFunction(theClass, relationship);
                        }
                        writeClassInsertAfter(theClass, relationship);
                    }
                    writeClassRemoveFunction(theClass, relationship);
                } else if(type == REL_ORDERED_LIST) {
                    writeOrderedListFunctions(relationship);
                } else if(type == REL_ARRAY) {
                    writeClassArrayInsertFunction(theClass, relationship);
                    writeClassArrayAppendFunction(theClass, relationship, false);
                    if(dvRelationshipAccessParent(relationship)) {
                        writeClassRemoveArray(theClass, relationship, false);
                    }
                } else if(type == REL_HEAP) {
                    writeRelationshipBinaryHeapFunctions(relationship);
                    writeClassArrayAppendFunction(theClass, relationship, true);
                    writeClassRemoveArray(theClass, relationship, true);
                }
            }
        } dvEndClassChildRelationship;
        writeClassShowFunction(theClass);
    } dvEndModuleClass;
    writeStop(module);
    writeStart(module);
    fclose(dvFile);
}
