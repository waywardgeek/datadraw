/*
 * Copyright(C) 2006 Bill Cox
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 *(at your option) any later version.
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

#include "dv.h"

/*--------------------------------------------------------------------------------------------------
  Create new classes that are just useful for holding dynamic arrays of objects.
--------------------------------------------------------------------------------------------------*/
static void createClassDynamicArrayClasses(
    dvModule module)
{
    dvClass theClass, dynarrayClass;
    dvClass lastClass = dvModuleGetLastClass(module);
    dvRelationship rel;
    utSym sym;

    dvForeachModuleClass(module, theClass) {
        if(dvClassGenerateArrayClass(theClass)) {
            sym = utSymCreateFormatted("%sArray", utCopyString(dvClassGetName(theClass)));
            dynarrayClass = dvClassCreate(module, sym, dvClassNull);
            rel = dvRelationshipCreate(dvSchemaNull, dynarrayClass, theClass, REL_ARRAY, utSymNull, utSymNull);
            dvRelationshipSetAccessParent(rel, false);
        }
        if(theClass == lastClass) {
            return; /* Don't build array classes for themselves! */
        }
    } dvEndModuleClass;
}

/*--------------------------------------------------------------------------------------------------
  Create the attribute class.
--------------------------------------------------------------------------------------------------*/
static dvClass createAttributeClass(
    dvModule module,
    dvEnum attributeTypeEnum)
{ 
    dvClass attributeClass = dvClassCreate(module, utSymCreate("Attribute"), dvClassNull);
    utSym typeSym = utSymCreate("Type");
    dvProperty typeProp = dvPropertyCreate(attributeClass, dvUnionNull, PROP_ENUM, typeSym);
    dvUnion valueUnion = dvUnionCreate(attributeClass, typeSym, 1);
    dvProperty int64Prop, doubleProp, boolProp, symProp;
    dvProperty dataProp;

    dvPropertySetEnumProp(typeProp, attributeTypeEnum);
    dvUnionSetTypeProperty(valueUnion, typeProp);
    int64Prop = dvPropertyCreate(attributeClass, valueUnion, PROP_INT, utSymCreate("Int64Val"));
    dvPropertySetWidth(int64Prop, 64);
    doubleProp = dvPropertyCreate(attributeClass, valueUnion, PROP_DOUBLE, utSymCreate("DoubleVal"));
    boolProp = dvPropertyCreate(attributeClass, valueUnion, PROP_BOOL, utSymCreate("BoolVal"));
    symProp = dvPropertyCreate(attributeClass, valueUnion, PROP_SYM, utSymCreate("SymVal"));
    dvCaseCreate(int64Prop, dvEnumFindEntry(attributeTypeEnum, utSymCreate("INT64")));
    dvCaseCreate(doubleProp, dvEnumFindEntry(attributeTypeEnum, utSymCreate("DOUBLE")));
    dvCaseCreate(boolProp, dvEnumFindEntry(attributeTypeEnum, utSymCreate("BOOL")));
    dvCaseCreate(symProp, dvEnumFindEntry(attributeTypeEnum, utSymCreate("SYM")));
    dataProp = dvPropertyCreate(attributeClass, dvUnionNull, PROP_UINT, utSymCreate("Data"));
    dvPropertySetWidth(dataProp, 8);
    dvPropertySetArray(dataProp, true);
    return attributeClass;
}

/*--------------------------------------------------------------------------------------------------
  Create local Attrlist and Attribute classes, with a hash table between them, and put a cascade
  pointer from each class with attributes to Attrlist.
--------------------------------------------------------------------------------------------------*/
static void createClassAttributesWithAttributeTypeEnum(
    dvModule module,
    dvEnum attributeTypeEnum)
{
    utSym attrlistSym = utSymCreate("Attrlist");
    dvClass attrlistClass = dvClassCreate(module, attrlistSym, dvClassNull);
    dvClass attributeClass = createAttributeClass(module, attributeTypeEnum);
    dvRelationship hashRel = dvRelationshipCreate(dvSchemaNull, attrlistClass, attributeClass, REL_HASHED, utSymNull,
        utSymNull);
    dvRelationship relationship;
    dvClass theClass;

    dvAddDefaultKey(hashRel);
    dvRelationshipSetMandatory(hashRel, true);
    dvForeachModuleClass(module, theClass) {
        if(dvClassGenerateAttributes(theClass)) {
            relationship = dvRelationshipCreate(dvSchemaNull, theClass, attrlistClass, REL_POINTER, utSymNull, utSymNull);
            dvRelationshipSetAccessParent(relationship, false);
            dvRelationshipSetCascade(relationship, true);
            dvRelationshipSetSparse(relationship, true);
        }
    } dvEndModuleClass;
}

/*--------------------------------------------------------------------------------------------------
  Create local Attrlist and Attribute classes, with a hash table between them, and put a cascade
  pointer from each class with attributes to Attrlist.
--------------------------------------------------------------------------------------------------*/
static void createClassAttributes(
    dvModule module)
{
    utSym sym = utSymCreate("AttributeType");
    utSym prefix = utSymCreateFormatted("%s_ATTR", utStringToUpperCase(dvModuleGetPrefix(module)));
    dvEnum attributeTypeEnum = dvEnumCreate(module, sym, prefix);

    dvEntryCreate(attributeTypeEnum, utSymCreate("INT64"), 0);
    dvEntryCreate(attributeTypeEnum, utSymCreate("DOUBLE"), 1);
    dvEntryCreate(attributeTypeEnum, utSymCreate("BOOL"), 2);
    dvEntryCreate(attributeTypeEnum, utSymCreate("SYM"), 3);
    dvEntryCreate(attributeTypeEnum, utSymCreate("STRING"), 4);
    dvEntryCreate(attributeTypeEnum, utSymCreate("BLOB"), 5);
    createClassAttributesWithAttributeTypeEnum(module, attributeTypeEnum);
}

/*--------------------------------------------------------------------------------------------------
  Create the property, and copy the sparse field from the relationship to it.
--------------------------------------------------------------------------------------------------*/
static dvProperty relPropertyCreate(
    dvClass theClass,
    dvRelationship relationship,
    dvPropertyType type,
    utSym sym)
{
    dvProperty property = dvPropertyCreate(theClass, dvUnionNull, type, sym);

    dvRelationshipAppendProperty(relationship, property);
    dvPropertySetSparse(property, dvRelationshipSparse(relationship));
    return property;
}

/*--------------------------------------------------------------------------------------------------
  Add hash key properties to the cache object, so we'll create them together in memory.
--------------------------------------------------------------------------------------------------*/
static void addKeysToCache(
    dvCache cache,
    dvRelationship relationship)
{
    dvProperty property;
    dvKeyproperty keyproperty;
    dvPropident propident;
    dvKey key;
    utSym propSym;

    dvForeachRelationshipKey(relationship, key) {
        keyproperty = dvKeyGetLastKeyproperty(key);
        property = dvKeypropertyGetProperty(keyproperty);
        if(property != dvPropertyNull) {
            if(dvPropertyGetCache(property) == dvCacheNull) {
                dvCacheAppendProperty(cache, property);
            }
        } else {
            propSym = dvKeypropertyGetPropertySym(keyproperty);
            if(propSym != utSymNull) {
                propident = dvPropidentAlloc();
                dvPropidentSetSym(propident, propSym);
                dvCacheAppendPropident(cache, propident);
            }
        }
    } dvEndRelationshipKey;
}

/*--------------------------------------------------------------------------------------------------
  Create fields used by the relationship.  Note that we copy the label strings because we are
  creating new symbols in this fuction, which could cause them to move in memory.
--------------------------------------------------------------------------------------------------*/
static void generateRelationshipFields(
    dvRelationship relationship)
{
    dvClass parent = dvRelationshipGetParentClass(relationship);
    dvClass child = dvRelationshipGetChildClass(relationship);
//    dvCache cache;
    dvProperty prop;
    utSym sym;
    char *parentName = utCopyString(dvClassGetName(parent));
    char *childName = utCopyString(dvClassGetName(child));
    char *parentLabel = utCopyString(dvRelationshipGetParentLabel(relationship));
    char *childLabel = utCopyString(dvRelationshipGetChildLabel(relationship));
    dvRelationshipType type = dvRelationshipGetType(relationship);

    if(dvRelationshipAccessParent(relationship)) {
        /* Owner pointer */
        sym = utSymCreateFormatted("%s%s", parentLabel, parentName);
        prop = dvClassFindProperty(child, sym);
        if(prop != dvPropertyNull) {
            if(dvPropertyGetClass(prop) != child ||
                    dvPropertyGetUnion(prop) != dvUnionNull ||
                    dvPropertyGetType(prop) != PROP_POINTER) {
                utError("Property %s on class %s defined differently the second time",
                    utSymGetName(sym), dvClassGetName(child));
            }
            /* Must be a duplicated relationship */
            dvRelationshipSetSharedParent(relationship, true);
        } else {
            prop = relPropertyCreate(child, relationship, PROP_POINTER, sym);
            dvPropertySetClassProp(prop, parent);
        }
    }
    if(!dvRelationshipAccessChild(relationship)) {
        return;
    }
    if(type == REL_POINTER) {
        /* Child pointer */
        sym = utSymCreateFormatted("%s%s", childLabel, childName);
        prop = relPropertyCreate(parent, relationship, PROP_POINTER, sym);
        dvPropertySetClassProp(prop, child);
    }
    if(type == REL_LINKED_LIST || type == REL_TAIL_LINKED || type == REL_DOUBLY_LINKED ||
            (type == REL_HASHED && !dvRelationshipUnordered(relationship))) {
        /* First pointer */
        sym = utSymCreateFormatted("First%s%s", childLabel, childName);
        prop = relPropertyCreate(parent, relationship, PROP_POINTER, sym);
        dvPropertySetClassProp(prop, child);
        /* Next child pointer */
        sym = utSymCreateFormatted("Next%s%s%s", parentName, childLabel, childName);
        prop = relPropertyCreate(child, relationship, PROP_POINTER, sym);
        dvPropertySetClassProp(prop, child);
    }
    if(type == REL_TAIL_LINKED || type == REL_DOUBLY_LINKED || type == REL_HASHED) {
        /* Last pointer */
        sym = utSymCreateFormatted("Last%s%s", childLabel, childName);
        prop = relPropertyCreate(parent, relationship, PROP_POINTER, sym);
        dvPropertySetClassProp(prop, child);
    }
    if(type == REL_DOUBLY_LINKED || (type == REL_HASHED && !dvRelationshipUnordered(relationship))) {
        /* Previous child pointer */
        sym = utSymCreateFormatted("Prev%s%s%s", parentName, childLabel, childName);
        prop = relPropertyCreate(child, relationship, PROP_POINTER, sym);
        dvPropertySetClassProp(prop, child);
    }
    if(type == REL_ARRAY || type == REL_HEAP) {
//temp: register these fields in dvgenc.c, then put these back.
        //cache = dvCacheCreate(child);
        /* Array of children */
        sym = utSymCreateFormatted("%s%s", childLabel, childName);
        prop = relPropertyCreate(parent, relationship, PROP_POINTER, sym);
        dvPropertySetClassProp(prop, child);
        dvPropertySetArray(prop, true);
        if(dvRelationshipAccessParent(relationship)) {
            /* Index on parent */
            sym = utSymCreateFormatted("%s%sIndex", parentLabel, parentName);
            prop = relPropertyCreate(child, relationship, PROP_UINT, sym);
            dvPropertySetWidth(prop, 32);
            if(type == REL_ARRAY) { // TODO: Only array?  I don't know about heaps, to be double-checked.  Richard 
                dvPropertySetInitializer(prop, "UINT32_MAX", sizeof("UINT32_MAX"));
            }
        }
        /* Used children on parent */
        sym = utSymCreateFormatted("Used%s%s", childLabel, childName);
        prop = relPropertyCreate(parent, relationship, PROP_UINT, sym);
        dvPropertySetWidth(prop, 32);
        //addKeysToCache(cache, relationship);
    }
    if(type == REL_HASHED) {
        //cache = dvCacheCreate(child);
        /* Hash table */
        sym = utSymCreateFormatted("%s%sTable", childLabel, childName);
        prop = relPropertyCreate(parent, relationship, PROP_POINTER, sym);
        dvPropertySetClassProp(prop, child);
        dvPropertySetArray(prop, true);
        /* Number of entries in table */
        sym = utSymCreateFormatted("Num%s%s", childLabel, childName);
        prop = relPropertyCreate(parent, relationship, PROP_UINT, sym);
        dvPropertySetWidth(prop, 32);
        /* Next child in table */
        sym = utSymCreateFormatted("NextTable%s%s%s", parentName, childLabel, childName);
        prop = relPropertyCreate(child, relationship, PROP_POINTER, sym);
        dvPropertySetClassProp(prop, child);
        //addKeysToCache(cache, relationship);
    }
    if(type == REL_ORDERED_LIST) {
        //cache = dvCacheCreate(child);
        /* Root pointer */
        sym = utSymCreateFormatted("Root%s%s", childLabel, childName);
        prop = relPropertyCreate(parent, relationship, PROP_POINTER, sym);
        dvPropertySetClassProp(prop, child);
        /* Parent pointer */
        sym = utSymCreateFormatted("Parent%s%s%s", parentName, childLabel, childName);
        prop = relPropertyCreate(child, relationship, PROP_POINTER, sym);
        dvPropertySetClassProp(prop, child);
        /* Left pointer */
        sym = utSymCreateFormatted("Left%s%s%s", parentName, childLabel, childName);
        prop = relPropertyCreate(child, relationship, PROP_POINTER, sym);
        dvPropertySetClassProp(prop, child);
        //dvCacheAppendProperty(cache, prop);
        /* Right pointer */
        sym = utSymCreateFormatted("Right%s%s%s", parentName, childLabel, childName);
        prop = relPropertyCreate(child, relationship, PROP_POINTER, sym);
        dvPropertySetClassProp(prop, child);
        //dvCacheAppendProperty(cache, prop);
        /* IsRed */
        sym = utSymCreateFormatted("IsRed%s%s%s", parentName, childLabel, childName);
        prop = relPropertyCreate(child, relationship, PROP_BOOL, sym);
        //addKeysToCache(cache, relationship);
    }
}

/*--------------------------------------------------------------------------------------------------
  Create all the module's fields used by relationships.  Also make cascade and mandatory
  relationships to create_only objects weak.
--------------------------------------------------------------------------------------------------*/
static void createModuleRelationshipFields(
    dvModule module)
{
    dvRelationship relationship;
    dvClass theClass, child;

    dvForeachModuleClass(module, theClass) {
        dvForeachClassChildRelationship(theClass, relationship) {
            if(!dvRelationshipExpanded(relationship)) {
                child = dvRelationshipGetChildClass(relationship);
                if(dvClassGetMemoryStyle(child) == MEM_CREATE_ONLY || !dvRelationshipAccessChild(relationship)) {
                    dvRelationshipSetCascade(relationship, false);
                    dvRelationshipSetMandatory(relationship, false);
                }
                dvRelationshipSetExpanded(relationship, true);
                generateRelationshipFields(relationship);
            }
        } dvEndClassChildRelationship;
    } dvEndModuleClass;
}

/*--------------------------------------------------------------------------------------------------
  Build sparsegroups for sparse classes.
--------------------------------------------------------------------------------------------------*/
static void buildClassExtensionSparsegroups(
    dvModule module)
{
    dvClass theClass;
    dvSparsegroup sparsegroup;
    dvProperty property;
    utSym sym;

    dvForeachModuleClass(module, theClass) {
        if(dvClassSparse(theClass)) {
            sym = utSymCreateFormatted("Sparse%sData", dvClassGetName(theClass));
            sparsegroup = dvSparsegroupCreate(theClass, sym);
            dvForeachClassProperty(theClass, property) {
                if(dvPropertyGetUnion(property) == dvUnionNull && !dvPropertyArray(property)) {
                    dvSparsegroupAppendProperty(sparsegroup, property);
                    dvPropertySetSparse(property, true);
                }
            } dvEndClassProperty;
        }
    } dvEndModuleClass;
}

/*--------------------------------------------------------------------------------------------------
  Build sparsegroups for sparse relationships.
--------------------------------------------------------------------------------------------------*/
static void buildRelationshipSparsegroups(
    dvModule module)
{
    dvClass parent, child;
    dvRelationship relationship;
    dvSparsegroup sparsegroup;
    dvProperty property;
    utSym sym;

    dvForeachModuleClass(module, parent) {
        dvForeachClassChildRelationship(parent, relationship) {
            if(dvRelationshipSparse(relationship)) {
                child = dvRelationshipGetChildClass(relationship);
                sym = utSymCreateFormatted("SparseParent%s%s%sData", dvRelationshipGetParentLabel(relationship),
                    dvClassGetName(parent), dvClassGetName(child));
                sparsegroup = dvSparsegroupCreate(parent, sym);
                dvRelationshipSetParentSparsegroup(relationship, sparsegroup);
                dvSparsegroupSetRelationship(sparsegroup, relationship);
                sym = utSymCreateFormatted("SparseChild%s%s%sData",  dvClassGetName(parent),
                    dvRelationshipGetChildLabel(relationship), dvClassGetName(child));
                sparsegroup = dvSparsegroupCreate(child, sym);
                dvRelationshipSetChildSparsegroup(relationship, sparsegroup);
                dvSparsegroupSetRelationship(sparsegroup, relationship);
                dvForeachRelationshipProperty(relationship, property) {
                    if(dvPropertyGetSparsegroup(property) == dvSparsegroupNull && !dvPropertyArray(property)) {
                        if(dvPropertyGetClass(property) == parent) {
                            dvSparsegroupAppendProperty(dvRelationshipGetParentSparsegroup(relationship), property);
                            dvPropertySetSparse(property, true);
                        } else {
                            dvSparsegroupAppendProperty(dvRelationshipGetChildSparsegroup(relationship), property);
                            dvPropertySetSparse(property, true);
                        }
                    }
                } dvEndRelationshipProperty;
            }
        } dvEndClassChildRelationship;
    } dvEndModuleClass;
}

/*--------------------------------------------------------------------------------------------------
  Build sparsegroups for sparse array properties.
--------------------------------------------------------------------------------------------------*/
static void buildArraySparsegroups(
    dvModule module)
{
    dvClass theClass;
    dvSparsegroup sparsegroup;
    dvProperty property, firstProp, numProp;
    utSym sym;

    dvForeachModuleClass(module, theClass) {
        dvForeachClassProperty(theClass, property) {
            if(dvPropertyArray(property) && !dvPropertyFixedSize(property)) {
                firstProp = dvPropertyGetFirstElementProp(property);
                numProp = dvPropertyGetNumElementsProp(property);
                if(dvPropertySparse(firstProp) && dvPropertyGetSparsegroup(firstProp) == dvSparsegroupNull) {
                    sym = utSymCreateFormatted("Sparse%s%sData",  dvClassGetName(theClass),
                        dvPropertyGetName(property));
                    sparsegroup = dvSparsegroupCreate(theClass, sym);
                    dvSparsegroupAppendProperty(sparsegroup, firstProp);
                    dvSparsegroupAppendProperty(sparsegroup, numProp);
                }
            }
        } dvEndClassProperty;
    } dvEndModuleClass;
}

/*--------------------------------------------------------------------------------------------------
  Build sparsegroups for any remaining sparse properties.
--------------------------------------------------------------------------------------------------*/
static void buildPropertySparsegroups(
    dvModule module)
{
    dvClass theClass;
    dvSparsegroup sparsegroup;
    dvProperty property;
    utSym sym;

    dvForeachModuleClass(module, theClass) {
        dvForeachClassProperty(theClass, property) {
            if(dvPropertySparse(property) && dvPropertyGetSparsegroup(property) == dvSparsegroupNull) {
                sym = utSymCreateFormatted("Sparse%s%sData",  dvClassGetName(theClass),
                    dvPropertyGetName(property));
                sparsegroup = dvSparsegroupCreate(theClass, sym);
                dvSparsegroupAppendProperty(sparsegroup, property);
            }
        } dvEndClassProperty;
    } dvEndModuleClass;
}

/*--------------------------------------------------------------------------------------------------
  Build sparsegroups, which represent groups of related properties in a class that should be kept
  together in a single sparse data hash table.
--------------------------------------------------------------------------------------------------*/
static void buildSparsegroups(
    dvModule module)
{
    buildClassExtensionSparsegroups(module);
    buildRelationshipSparsegroups(module);
    buildArraySparsegroups(module);
    buildPropertySparsegroups(module);
}

/*--------------------------------------------------------------------------------------------------
  Create the DatadrawRoot class to keep track  of sparse data.
--------------------------------------------------------------------------------------------------*/
static dvClass createGlobalRoot(
    dvModule module)
{
    dvClass datadrawRoot = dvClassCreate(module, utSymCreate("DatadrawRoot"), dvClassNull);

    dvClassSetMemoryStyle(datadrawRoot, MEM_CREATE_ONLY);
    return datadrawRoot;
}

/*--------------------------------------------------------------------------------------------------
  Copy the property to the sparse data class.
--------------------------------------------------------------------------------------------------*/
static void addSparseDataProperty(
    dvModule module,
    dvClass dataClass,
    dvProperty property)
{
    dvProperty valueProperty = dvPropertyCreate(dataClass, dvUnionNull, dvPropertyGetType(property),
        dvPropertyGetSym(property));

    dvPropertyCopyProps(property, valueProperty);
    dvPropertySetSparse(valueProperty, false);
    switch(dvPropertyGetType(property)) {
    case PROP_ENUM:
        dvPropertySetEnumProp(valueProperty, dvPropertyGetEnumProp(property));
        break;
    case PROP_TYPEDEF:
        dvPropertySetTypedefProp(valueProperty, dvPropertyGetTypedefProp(property));
        break;
    case PROP_POINTER:
        dvPropertySetClassProp(valueProperty, dvPropertyGetClassProp(property));
        break;
    case PROP_SYM:
        dvPropertySetTypeSym(valueProperty, dvPropertyGetTypeSym(property));
        break;
    case PROP_INT: case PROP_UINT:
        dvPropertySetWidth(valueProperty, dvPropertyGetWidth(property));
        break;
    default:
        /* Nothing needed */
        break;
    }
}

/*--------------------------------------------------------------------------------------------------
  Create a class to hold the sparsegroup's property data, and create a hashed relationship from
  the root to it.
--------------------------------------------------------------------------------------------------*/
static void createSparsegroupClass(
    dvModule module,
    dvSparsegroup sparsegroup,
    dvClass datadrawRoot)
{
    dvClass theClass = dvSparsegroupGetClass(sparsegroup);
    /*dvModule module = dvClassGetModule(theClass); class' module can be different */
    dvClass dataClass = dvClassCreate(module, dvSparsegroupGetSym(sparsegroup), dvClassNull);
    dvRelationship relationship = dvRelationshipCreate(dvSchemaNull, datadrawRoot, dataClass, REL_HASHED, utSymNull,
        utSymNull);
    utSym sym = utSymCreateFormatted("%sKey", dvClassGetName(theClass));
    dvProperty objectProperty = dvPropertyCreate(dataClass, dvUnionNull, PROP_POINTER, sym);
    dvProperty property;

    dvPropertySetClassProp(objectProperty, theClass);
    dvKeyCreate(relationship, objectProperty);
    dvForeachSparsegroupProperty(sparsegroup, property) {
        addSparseDataProperty(module, dataClass, property);
    } dvEndSparsegroupProperty;
    dvRelationshipSetAccessParent(relationship, false);
}

/*--------------------------------------------------------------------------------------------------
  Create sparse data helper classes.
--------------------------------------------------------------------------------------------------*/
static void createSparseDataClasses(
    dvModule module)
{
    dvClass theClass;
    dvClass datadrawRoot = dvClassNull;
    dvSparsegroup sparsegroup;
    bool firstTime = true;

    dvForeachModuleClass(module, theClass) {
        dvForeachClassSparsegroup(theClass, sparsegroup) {
            if(dvSparsegroupGetFirstProperty(sparsegroup) != dvPropertyNull) {
                /* We wait to deal with arrays until after their fields are generated */
                if(firstTime) {
                    dvModuleSetHasSparseData(module, true);
                    datadrawRoot = createGlobalRoot(module);
                    firstTime = false;
                }
                createSparsegroupClass(module, sparsegroup, datadrawRoot);
            }
        } dvEndClassSparsegroup;
    } dvEndModuleClass;
}

/*--------------------------------------------------------------------------------------------------
  Bind keys to the properties they refer to.
--------------------------------------------------------------------------------------------------*/
static void bindKeysToProperties(
    dvModule module)
{
    dvClass theClass, keyClass;
    dvRelationship relationship;
    dvProperty property;
    dvKey key;
    dvKeyproperty keyproperty;

    dvForeachModuleClass(module, theClass) {
        dvForeachClassParentRelationship(theClass, relationship) {
            dvForeachRelationshipKey(relationship, key) {
                keyClass = theClass;
                dvForeachKeyKeyproperty(key, keyproperty) {
                    if((property = dvKeypropertyGetProperty(keyproperty)) == dvPropertyNull) {
                        property = dvClassLookupProperty(keyClass, dvKeypropertyGetPropertySym(keyproperty));
                        if(property == dvPropertyNull) {
                            utError("Line %u: key %s not found on class %s", dvKeyGetLineNum(key),
                                utSymGetName(dvKeypropertyGetPropertySym(key)), dvClassGetName(theClass));
                        }
                        dvPropertyAppendKeyproperty(property, keyproperty);
                    }
                    /* check for requirements on key element in the chain */
                    if(dvKeypropertyGetNextKeyKeyproperty(keyproperty) != dvKeypropertyNull) {
                        if(dvPropertyFixedSize(property)) {
                                utError("Line %u: only last element of a key chain is supported as variable size, here %s",
                                    dvKeyGetLineNum(key),
                                    utSymGetName(dvKeypropertyGetPropertySym(key)));
                        }
                        if(dvPropertyGetType(property) != PROP_POINTER) {
                            utError("Line %u: key %s must be bound to a class", dvKeyGetLineNum(key),
                                utSymGetName(dvKeypropertyGetPropertySym(key)));
                        }
                    }
                    keyClass  = dvPropertyGetClassProp(property);
                } dvEndKeyKeyproperty;
            } dvEndRelationshipKey;
        } dvEndClassParentRelationship;
    } dvEndModuleClass;
}

/*--------------------------------------------------------------------------------------------------
  Bind cacheprops to the properties they refer to.
--------------------------------------------------------------------------------------------------*/
static void bindCacheToProperties(
    dvModule module)
{
    dvClass theClass;
    dvProperty property;
    dvCache cache;
    dvPropident propident;
    utSym propSym;

    dvForeachModuleClass(module, theClass) {
        dvSafeForeachClassCache(theClass, cache) {
            dvForeachCachePropident(cache, propident) {
                propSym = dvPropidentGetSym(propident);
                property = dvClassFindProperty(theClass, propSym);
                if(property == dvPropertyNull) {
                    utError("Line %u: property %s not found on class %s", dvCacheGetLine(cache),
                        utSymGetName(propSym), dvClassGetName(theClass));
                }
                if(dvPropertyGetCache(property) == dvCacheNull && !dvPropertyArray(property)) {
                    if(dvPropertyGetType(property) == PROP_BIT) {
                        dvPropertySetType(property, PROP_BOOL);
                    }
                    dvCacheAppendProperty(cache, property);
                }
            } dvEndCachePropident;
            if(dvCacheGetFirstProperty(cache) == dvPropertyNull ||
                    dvPropertyGetNextCacheProperty(dvCacheGetFirstProperty(cache)) == dvPropertyNull) {
                dvCacheDestroy(cache); /* No point caching just one property */
            }
        } dvEndSafeClassCache;
    } dvEndModuleClass;
}

/*--------------------------------------------------------------------------------------------------
  Move the new property in front of the old property in the list of properties on a class.
--------------------------------------------------------------------------------------------------*/
static void movePropInFront(
    dvProperty newProp,
    dvProperty oldProp)
{
    dvClass theClass = dvPropertyGetClass(newProp);

    dvClassRemoveProperty(theClass, newProp);
    if(dvClassGetFirstProperty(theClass) == oldProp) {
        dvClassInsertProperty(theClass, newProp);
    } else {
        dvClassInsertAfterProperty(theClass, dvPropertyGetPrevClassProperty(oldProp), newProp);
    }
}

/*--------------------------------------------------------------------------------------------------
  Generate array fields.  Move them in front of their array properties so when we read them from
  a backup file, they already have been defined.
--------------------------------------------------------------------------------------------------*/
static void createArrayFields(
    dvModule module)
{
    dvClass theClass;
    dvProperty property, newProp;
    utSym sym;

    dvForeachModuleClass(module, theClass) {
        dvForeachClassProperty(theClass, property) {
            if(dvPropertyArray(property) && !dvPropertyExpanded(property) && !dvPropertyFixedSize(property)) {
                dvPropertySetExpanded(property, true);
                /* Index into heap for first element */
                sym = utSymCreateFormatted("%sIndex_", utCopyString(dvPropertyGetName(property)));
                newProp = dvPropertyCreate(theClass, dvUnionNull, PROP_UINT, sym);
                dvPropertySetWidth(newProp, 32);
                dvPropertySetFirstElementProp(property, newProp);
                movePropInFront(newProp, property);
                dvPropertySetHidden(newProp, true);
                dvPropertySetSparse(newProp, dvPropertySparse(property));
                if(dvPropertyGetRelationship(property) != dvRelationshipNull) {
                    dvRelationshipAppendProperty(dvPropertyGetRelationship(property), newProp);
                }
                /* Number of elements allocated in array */
                sym = utSymCreateFormatted("Num%s", utCopyString(dvPropertyGetName(property)));
                newProp = dvPropertyCreate(theClass, dvUnionNull, PROP_UINT, sym);
                dvPropertySetWidth(newProp, 32);
                dvPropertySetNumElementsProp(property, newProp);
                movePropInFront(newProp, property);
                dvPropertySetHidden(newProp, true);
                dvPropertySetSparse(newProp, dvPropertySparse(property));
                dvPropertySetSparse(property, false);
                if(dvPropertyGetRelationship(property) != dvRelationshipNull) {
                    dvRelationshipAppendProperty(dvPropertyGetRelationship(property), newProp);
                }
            }
        } dvEndClassProperty;
    } dvEndModuleClass;
}

/*--------------------------------------------------------------------------------------------------
  Find the size of a property element in bits.  For typedef properties, we return 0.
--------------------------------------------------------------------------------------------------*/
uint8 dvFindPropertySize(
    dvProperty property)
{
    switch(dvPropertyGetType(property)) {
    case PROP_INT:
    case PROP_UINT:
        return dvPropertyGetWidth(property);
    case PROP_FLOAT:
        return sizeof(float) << 3;
    case PROP_DOUBLE:
        return sizeof(double) << 3;
    case PROP_BOOL:
        return sizeof(bool) << 3;
    case PROP_CHAR:
        return sizeof(char) << 3;
    case PROP_ENUM:
        return sizeof(int) << 3;
    case PROP_TYPEDEF:
        return 0; /* We don't really know */
    case PROP_POINTER:
        return dvClassGetReferenceSize(dvPropertyGetClassProp(property));
    case PROP_SYM:
        return sizeof(utSym) << 3;
    case PROP_BIT:
        return 1;
    default:
        utExit("Unknown property type");
    }
    return 0; /* Dummy return */
}

/*--------------------------------------------------------------------------------------------------
  Find a field on the class for use as a next pointer on a free list.  With DD_DEBUG set,
  references are either 32-bit or 64-bit, depending on if we are on a 64-bit machine.  So, only
  use reference fields for next pointers, not data.
--------------------------------------------------------------------------------------------------*/
static dvProperty findFieldForNextFreePointer(
    dvClass theClass)
{
    dvProperty property;
    dvPropertyType type;
    uint32 refSize = dvClassGetReferenceSize(theClass);

    dvForeachClassProperty(theClass, property) {
        type = dvPropertyGetType(property);
        if((type == PROP_POINTER || type == PROP_SYM) && !dvPropertyArray(property) && !dvPropertySparse(property) &&
                dvFindPropertySize(property) == refSize && dvPropertyGetUnion(property) == dvUnionNull &&
                dvPropertyGetCache(property) == dvCacheNull &&!dvPropertyView(property)) {
            return property;
        }
    } dvEndClassProperty;
    return dvPropertyNull;
}

/*--------------------------------------------------------------------------------------------------
  Find a field on the class large enough to hold the next pointer.  If none exist, allocate it.
--------------------------------------------------------------------------------------------------*/
static void setFreeListFields(
    dvModule module)
{
    dvClass theClass;
    dvProperty property;
    utSym sym;

    dvForeachModuleClass(module, theClass) {
        if(dvClassGetBaseClass(theClass) == dvClassNull && dvClassGetMemoryStyle(theClass) == MEM_FREE_LIST) {
            property = findFieldForNextFreePointer(theClass);
            if(property == dvPropertyNull) {
                sym = utSymCreate("FreeList");
                property = dvPropertyCreate(theClass, dvUnionNull, PROP_POINTER, sym);
                dvPropertySetClassProp(property, theClass);
                dvPropertySetHidden(property, true);
            }
            dvClassSetFreeListProperty(theClass, property);
        }
    } dvEndModuleClass;
}

/*--------------------------------------------------------------------------------------------------
  Just count the number fields that we will need to track.  Set the total number of fields on
  dvTheRoot.
--------------------------------------------------------------------------------------------------*/
static void setPropertyFieldNumbers(
    dvModule module)
{
    dvClass theClass;
    dvUnion theUnion;
    dvCase theCase;
    dvEnum theEnum;
    dvEntry entry;
    dvProperty property;
    uint16 classNumber = 0;
    uint16 fieldNumber = 0;
    uint16 enumNumber = 0;
    uint16 entryNumber;
    uint16 unionCaseNumber;
    uint16 xField;

    dvForeachModuleClass(module, theClass) {
        xField = 0;
        dvForeachClassProperty(theClass, property) {
            if(dvPropertyGetUnion(property) == dvUnionNull && !dvPropertySparse(property)) {
                dvPropertySetFieldNumber(property, fieldNumber);
                fieldNumber++;
                xField++;
            }
        } dvEndClassProperty;
        dvForeachClassUnion(theClass, theUnion) {
            dvUnionSetFieldNumber(theUnion, fieldNumber);
            unionCaseNumber = 0;
            dvForeachUnionProperty(theUnion, property) {
                dvForeachPropertyCase(property, theCase) {
                    unionCaseNumber++;
                } dvEndPropertyCase;
            } dvEndUnionProperty;
            dvUnionSetNumCases(theUnion, unionCaseNumber);
            fieldNumber++;
            xField++;
        } dvEndClassUnion;
        dvClassSetNumFields(theClass, xField);
        dvClassSetNumber(theClass, classNumber);
        classNumber++;
    } dvEndModuleClass;
    dvModuleSetNumFields(module, fieldNumber);
    dvModuleSetNumClasses(module, classNumber);
    dvForeachModuleEnum(module, theEnum) {
        enumNumber++;
        entryNumber = 0;
        dvForeachEnumEntry(theEnum, entry) {
            entryNumber++;
        } dvEndEnumEntry;
        dvEnumSetNumEntries(theEnum, entryNumber);
    } dvEndModuleEnum;
    dvModuleSetNumEnums(module, enumNumber);
}

/*--------------------------------------------------------------------------------------------------
  Generate code from the file.
--------------------------------------------------------------------------------------------------*/
void dvElaborateModule(
    dvModule module)
{
    createClassDynamicArrayClasses(module);
    if(dvModuleHasClassAttributes(module)) {
        createClassAttributes(module);
    }
    createModuleRelationshipFields(module);
    createArrayFields(module);
    /* Since this creates new relationships, we have to generate relationship and array fields again.
       We set an "Expanded" flag on relationships and array properties that have already been processed */
    buildSparsegroups(module);
    createSparseDataClasses(module);
    createModuleRelationshipFields(module);
    createArrayFields(module);
    dvModuleSetElaborated(module, true);
    bindKeysToProperties(module); /* Wait until here, so we can use generated fields for hash keys */
    bindCacheToProperties(module);
    setFreeListFields(module);
    setPropertyFieldNumbers(module);
}

/*--------------------------------------------------------------------------------------------------
  Generate code from the file.
--------------------------------------------------------------------------------------------------*/
bool dvGenerateCode(
    dvModule module,
    char *includeFile,
    char *sourceFile)
{
    dvElaborateModule(module);
    dvWriteHeaderFile(module, includeFile);
    dvWriteCFile(module, sourceFile);
    return true;
}

/*--------------------------------------------------------------------------------------------------
  Look up a property on a class, given that it migh be on a base class, rather than this one.
--------------------------------------------------------------------------------------------------*/
dvProperty dvClassLookupProperty(
    dvClass theClass,
    utSym sym)
{
    dvModule module;
    dvProperty property;

    do {
        module = dvClassGetModule(theClass);
        if(!dvModuleElaborated(module)) {
            dvElaborateModule(module); /* This insures we can find generated properties */
        }
        property = dvClassFindProperty(theClass, sym);
        if(property != dvPropertyNull) {
            return property;
        }
        theClass = dvClassGetBaseClass(theClass);
    } while(theClass != dvClassNull);
    return dvPropertyNull;
}
