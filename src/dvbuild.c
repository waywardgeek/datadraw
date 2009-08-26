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
  Constructors
--------------------------------------------------------------------------------------------------*/
#include "dv.h"

dvRoot dvTheRoot;

/*--------------------------------------------------------------------------------------------------
  Add a new directory to the module path.
--------------------------------------------------------------------------------------------------*/
dvModpath dvModpathCreate(
    utSym sym,
    bool insertAtHeadOfList)
{
    dvModpath modpath = dvModpathAlloc();

    dvModpathSetSym(modpath, sym);
    if(insertAtHeadOfList) {
        dvRootInsertModpath(dvTheRoot, modpath);
    } else {
        dvRootAppendModpath(dvTheRoot, modpath);
    }
    return modpath;
}

/*--------------------------------------------------------------------------------------------------
  Build a new module object.
--------------------------------------------------------------------------------------------------*/
dvModule dvModuleCreate(
    utSym sym,
    utSym prefix)
{
    dvModule module = dvModuleAlloc();
    utSym lowerPrefix;

    dvModuleSetSym(module, sym);
    if(prefix == utSymNull) {
        prefix = sym;
    }
    lowerPrefix = utSymCreate(utStringToLowerCase(utSymGetName(prefix)));
    dvModuleSetPrefixSym(module, lowerPrefix);
    dvRootAppendModule(dvTheRoot, module);
    return module;
}

/*--------------------------------------------------------------------------------------------------
  Build a new enum object.
--------------------------------------------------------------------------------------------------*/
dvEnum dvEnumCreate(
    dvModule module,
    utSym sym,
    utSym prefix)
{
    dvEnum theEnum = dvEnumAlloc();

    if(prefix == utSymNull) {
        prefix = utSymCreate("");
    }
    dvEnumSetSym(theEnum, sym);
    dvEnumSetPrefixSym(theEnum, prefix);
    dvModuleAppendEnum(module, theEnum);
    return theEnum;
}

/*--------------------------------------------------------------------------------------------------
  Build a new entry object.
--------------------------------------------------------------------------------------------------*/
dvEntry dvEntryCreate(
    dvEnum owningEnum,
    utSym sym,
    uint32 value)
{
    dvEntry entry = dvEntryAlloc();

    dvEntrySetSym(entry, sym);
    dvEntrySetValue(entry, value);
    dvEnumAppendEntry(owningEnum, entry);
    return entry;
}

/*--------------------------------------------------------------------------------------------------
  Build a new typedef object.
--------------------------------------------------------------------------------------------------*/
dvTypedef dvTypedefCreate(
    dvModule module,
    utSym sym,
    char *initializer)
{
    dvTypedef theTypedef = dvTypedefAlloc();
    uint32 length = strlen(initializer);

    dvTypedefSetSym(theTypedef, sym);
    dvTypedefSetInitializer(theTypedef, initializer, length + 1);
    dvModuleAppendTypedef(module, theTypedef);
    return theTypedef;
}

/*--------------------------------------------------------------------------------------------------
  Create a new schema, an object for organizing classes into logical groups.
--------------------------------------------------------------------------------------------------*/
dvSchema dvSchemaCreate(
    dvModule module,
    utSym sym)
{
    dvSchema schema = dvSchemaAlloc();

    dvSchemaSetSym(schema, sym);
    dvModuleAppendSchema(module, schema);
    return schema;
}

/*--------------------------------------------------------------------------------------------------
  Create a new class.
--------------------------------------------------------------------------------------------------*/
dvClass dvClassCreate(
    dvModule module,
    utSym sym,
    dvClass baseClass)
{
    dvClass theClass = dvModuleFindClass(module, sym);

    if(theClass == dvClassNull) {
        theClass = dvClassAlloc();
        dvClassSetSym(theClass, sym);
        dvModuleAppendClass(module, theClass);
        dvClassSetReferenceSize(theClass, 32);
        dvClassSetMemoryStyle(theClass, MEM_FREE_LIST);
    }
    if(baseClass != dvClassNull && dvClassGetBaseClass(theClass) == dvClassNull) {
        dvClassAppendDerivedClass(baseClass, theClass);
        dvClassSetMemoryStyle(theClass, dvClassGetMemoryStyle(baseClass));
    }
    return theClass;
}

/*--------------------------------------------------------------------------------------------------
  Create a new property.
--------------------------------------------------------------------------------------------------*/
dvProperty dvPropertyCreate(
    dvClass owningClass,
    dvUnion owningUnion,
    dvPropertyType type,
    utSym sym)
{
    dvProperty property = dvClassFindProperty(owningClass, sym);

    if(property != dvPropertyNull) {
        utWarning("Property %s already exists on class %s", utSymGetName(sym),
            dvClassGetName(owningClass));
        if(dvPropertyGetClass(property) != owningClass ||
                dvPropertyGetUnion(property) != owningUnion ||
                dvPropertyGetType(property) != type) {
            utError("Property %s defined differently the second time", utSymGetName(sym));
        }
        return property;
    }
    property = dvPropertyAlloc();
    dvPropertySetType(property, type);
    dvPropertySetSym(property, sym);
    dvClassAppendProperty(owningClass, property);
    if(owningUnion != dvUnionNull) {
        dvUnionAppendProperty(owningUnion, property);
    }
    return property;
}

/*--------------------------------------------------------------------------------------------------
  Create a new union.
--------------------------------------------------------------------------------------------------*/
dvUnion dvUnionCreate(
    dvClass owningClass,
    utSym propertySym,
    uint16 unionNumber)
{
    dvUnion theUnion = dvUnionAlloc();

    dvClassAppendUnion(owningClass, theUnion);
    dvUnionSetPropertySym(theUnion, propertySym);
    dvUnionSetNumber(theUnion, unionNumber);
    return theUnion;
}

/*--------------------------------------------------------------------------------------------------
  Create a new relationship.
--------------------------------------------------------------------------------------------------*/
dvRelationship dvRelationshipCreate(
    dvSchema schema,
    dvClass parent,
    dvClass child,
    dvRelationshipType type,
    utSym parentLabel,
    utSym childLabel)
{
    dvRelationship relationship = dvRelationshipAlloc();

    if(schema != dvSchemaNull) {
        dvSchemaAppendRelationship(schema, relationship);
    }
    if(parentLabel == utSymNull) {
        parentLabel = utSymCreate("");
    }
    if(childLabel == utSymNull) {
        childLabel = utSymCreate("");
    }
    dvRelationshipSetType(relationship, type);
    dvRelationshipSetParentLabelSym(relationship, parentLabel);
    dvRelationshipSetChildLabelSym(relationship, childLabel);
    dvRelationshipSetAccessParent(relationship, true);
    dvRelationshipSetAccessChild(relationship, true);
    dvClassAppendChildRelationship(parent, relationship);
    dvClassAppendParentRelationship(child, relationship);
    return relationship;
}

/*--------------------------------------------------------------------------------------------------
  Create a new link.
--------------------------------------------------------------------------------------------------*/
dvLink dvLinkCreate(
    dvModule importModule,
    dvModule exportModule)
{
    dvLink link = dvLinkAlloc();

    dvModuleAppendImportLink(importModule, link);
    dvModuleAppendExportLink(exportModule, link);
    return link;
}

/*--------------------------------------------------------------------------------------------------
  Create a new key, which is part of a complex relationship.
--------------------------------------------------------------------------------------------------*/
dvKey dvKeyCreate(
    dvRelationship relationship,
    dvProperty property)
{
    dvKey key = dvKeyAlloc();

    dvRelationshipAppendKey(relationship, key);
    dvKeypropertyCreate(key, property);
    return key;
}

/*--------------------------------------------------------------------------------------------------
  Create a new key, which is part of a complex relationship.
--------------------------------------------------------------------------------------------------*/
dvKeyproperty dvKeypropertyCreate(
    dvKey key,
    dvProperty property)
{
    dvKeyproperty keyproperty = dvKeypropertyAlloc();

    dvKeyAppendKeyproperty(key, keyproperty);
    dvPropertyAppendKeyproperty(property, keyproperty);
    return keyproperty;
}


/*--------------------------------------------------------------------------------------------------
  Create a new unbound key, which is part of a complex relationship.
--------------------------------------------------------------------------------------------------*/
dvKey dvUnboundKeyCreate(
    dvRelationship relationship,
    utSym propertySym,
    uint32 lineNum)
{
    dvKey key = dvKeyAlloc();

    dvRelationshipAppendKey(relationship, key);
    dvUnboundKeypropertyCreate(key, propertySym);
    dvKeySetLineNum(key, lineNum);
    return key;
}

/*--------------------------------------------------------------------------------------------------
  Create a new unbound keyproperty, which is part of a complex relationship.
--------------------------------------------------------------------------------------------------*/
dvKeyproperty dvUnboundKeypropertyCreate(
    dvKey key,
    utSym propertySym)
{
    dvKeyproperty keyproperty = dvKeypropertyAlloc();

    dvKeyAppendKeyproperty(key, keyproperty);
    dvKeypropertySetPropertySym(keyproperty, propertySym);
    return keyproperty;
}


/*--------------------------------------------------------------------------------------------------
  Create a new case object linking a property in a union to an entry.
--------------------------------------------------------------------------------------------------*/
dvCase dvCaseCreate(
    dvProperty property,
    dvEntry entry)
{
    dvCase theCase = dvCaseAlloc();

    dvPropertyAppendCase(property, theCase);
    dvEntryAppendCase(entry, theCase);
    return theCase;
}

/*--------------------------------------------------------------------------------------------------
  Create a new case object used to group related sparse fields.
--------------------------------------------------------------------------------------------------*/
dvSparsegroup dvSparsegroupCreate(
    dvClass theClass,
    utSym sym)
{
    dvSparsegroup sparsegroup = dvSparsegroupAlloc();

    dvSparsegroupSetSym(sparsegroup, sym);
    dvClassAppendSparsegroup(theClass, sparsegroup);
    return sparsegroup;
}

/*--------------------------------------------------------------------------------------------------
  Create a new cache object for grouping fields in memory.
--------------------------------------------------------------------------------------------------*/
dvCache dvCacheCreate(
    dvClass theClass)
{
    dvCache cache = dvCacheAlloc();

    dvClassAppendCache(theClass, cache);
    return cache;
}
