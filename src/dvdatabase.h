/*----------------------------------------------------------------------------------------
  Module header file for: dv module
----------------------------------------------------------------------------------------*/
#ifndef DVDATABASE_H

#define DVDATABASE_H

#if defined __cplusplus
extern "C" {
#endif

#ifndef DD_UTIL_H
#include "ddutil.h"
#endif

extern uint8 dvModuleID;
/* Class reference definitions */
#if (defined(DD_DEBUG) && !defined(DD_NOSTRICT)) || defined(DD_STRICT)
typedef struct _struct_dvRoot{char val;} *dvRoot;
#define dvRootNull ((dvRoot)0)
typedef struct _struct_dvModpath{char val;} *dvModpath;
#define dvModpathNull ((dvModpath)0)
typedef struct _struct_dvModule{char val;} *dvModule;
#define dvModuleNull ((dvModule)0)
typedef struct _struct_dvLink{char val;} *dvLink;
#define dvLinkNull ((dvLink)0)
typedef struct _struct_dvSchema{char val;} *dvSchema;
#define dvSchemaNull ((dvSchema)0)
typedef struct _struct_dvEnum{char val;} *dvEnum;
#define dvEnumNull ((dvEnum)0)
typedef struct _struct_dvEntry{char val;} *dvEntry;
#define dvEntryNull ((dvEntry)0)
typedef struct _struct_dvTypedef{char val;} *dvTypedef;
#define dvTypedefNull ((dvTypedef)0)
typedef struct _struct_dvClass{char val;} *dvClass;
#define dvClassNull ((dvClass)0)
typedef struct _struct_dvProperty{char val;} *dvProperty;
#define dvPropertyNull ((dvProperty)0)
typedef struct _struct_dvSparsegroup{char val;} *dvSparsegroup;
#define dvSparsegroupNull ((dvSparsegroup)0)
typedef struct _struct_dvRelationship{char val;} *dvRelationship;
#define dvRelationshipNull ((dvRelationship)0)
typedef struct _struct_dvKey{char val;} *dvKey;
#define dvKeyNull ((dvKey)0)
typedef struct _struct_dvKeyproperty{char val;} *dvKeyproperty;
#define dvKeypropertyNull ((dvKeyproperty)0)
typedef struct _struct_dvUnion{char val;} *dvUnion;
#define dvUnionNull ((dvUnion)0)
typedef struct _struct_dvCase{char val;} *dvCase;
#define dvCaseNull ((dvCase)0)
typedef struct _struct_dvCache{char val;} *dvCache;
#define dvCacheNull ((dvCache)0)
typedef struct _struct_dvPropident{char val;} *dvPropident;
#define dvPropidentNull ((dvPropident)0)
#else
typedef uint32 dvRoot;
#define dvRootNull 0
typedef uint32 dvModpath;
#define dvModpathNull 0
typedef uint32 dvModule;
#define dvModuleNull 0
typedef uint32 dvLink;
#define dvLinkNull 0
typedef uint32 dvSchema;
#define dvSchemaNull 0
typedef uint32 dvEnum;
#define dvEnumNull 0
typedef uint32 dvEntry;
#define dvEntryNull 0
typedef uint32 dvTypedef;
#define dvTypedefNull 0
typedef uint32 dvClass;
#define dvClassNull 0
typedef uint32 dvProperty;
#define dvPropertyNull 0
typedef uint32 dvSparsegroup;
#define dvSparsegroupNull 0
typedef uint32 dvRelationship;
#define dvRelationshipNull 0
typedef uint32 dvKey;
#define dvKeyNull 0
typedef uint32 dvKeyproperty;
#define dvKeypropertyNull 0
typedef uint32 dvUnion;
#define dvUnionNull 0
typedef uint32 dvCase;
#define dvCaseNull 0
typedef uint32 dvCache;
#define dvCacheNull 0
typedef uint32 dvPropident;
#define dvPropidentNull 0
#endif

/* RelationshipType enumerated type */
typedef enum {
    REL_LINKED_LIST = 0,
    REL_DOUBLY_LINKED = 1,
    REL_TAIL_LINKED = 2,
    REL_POINTER = 3,
    REL_ARRAY = 4,
    REL_HEAP = 5,
    REL_HASHED = 6,
    REL_ORDERED_LIST = 7,
    REL_UNBOUND = 8
} dvRelationshipType;

/* PropertyType enumerated type */
typedef enum {
    PROP_INT = 0,
    PROP_UINT = 1,
    PROP_FLOAT = 2,
    PROP_DOUBLE = 3,
    PROP_BIT = 4,
    PROP_BOOL = 5,
    PROP_CHAR = 6,
    PROP_ENUM = 7,
    PROP_TYPEDEF = 8,
    PROP_POINTER = 9,
    PROP_SYM = 10,
    PROP_UNBOUND = 11
} dvPropertyType;

/* MemoryStyle enumerated type */
typedef enum {
    MEM_CREATE_ONLY = 0,
    MEM_FREE_LIST = 1
} dvMemoryStyle;

/* Constructor/Destructor hooks. */
typedef void (*dvRootCallbackType)(dvRoot);
extern dvRootCallbackType dvRootConstructorCallback;
typedef void (*dvModpathCallbackType)(dvModpath);
extern dvModpathCallbackType dvModpathConstructorCallback;
typedef void (*dvModuleCallbackType)(dvModule);
extern dvModuleCallbackType dvModuleConstructorCallback;
typedef void (*dvLinkCallbackType)(dvLink);
extern dvLinkCallbackType dvLinkConstructorCallback;
typedef void (*dvSchemaCallbackType)(dvSchema);
extern dvSchemaCallbackType dvSchemaConstructorCallback;
typedef void (*dvEnumCallbackType)(dvEnum);
extern dvEnumCallbackType dvEnumConstructorCallback;
typedef void (*dvEntryCallbackType)(dvEntry);
extern dvEntryCallbackType dvEntryConstructorCallback;
typedef void (*dvTypedefCallbackType)(dvTypedef);
extern dvTypedefCallbackType dvTypedefConstructorCallback;
typedef void (*dvClassCallbackType)(dvClass);
extern dvClassCallbackType dvClassConstructorCallback;
typedef void (*dvPropertyCallbackType)(dvProperty);
extern dvPropertyCallbackType dvPropertyConstructorCallback;
typedef void (*dvSparsegroupCallbackType)(dvSparsegroup);
extern dvSparsegroupCallbackType dvSparsegroupConstructorCallback;
extern dvSparsegroupCallbackType dvSparsegroupDestructorCallback;
typedef void (*dvRelationshipCallbackType)(dvRelationship);
extern dvRelationshipCallbackType dvRelationshipConstructorCallback;
typedef void (*dvKeyCallbackType)(dvKey);
extern dvKeyCallbackType dvKeyConstructorCallback;
extern dvKeyCallbackType dvKeyDestructorCallback;
typedef void (*dvKeypropertyCallbackType)(dvKeyproperty);
extern dvKeypropertyCallbackType dvKeypropertyConstructorCallback;
extern dvKeypropertyCallbackType dvKeypropertyDestructorCallback;
typedef void (*dvUnionCallbackType)(dvUnion);
extern dvUnionCallbackType dvUnionConstructorCallback;
extern dvUnionCallbackType dvUnionDestructorCallback;
typedef void (*dvCaseCallbackType)(dvCase);
extern dvCaseCallbackType dvCaseConstructorCallback;
extern dvCaseCallbackType dvCaseDestructorCallback;
typedef void (*dvCacheCallbackType)(dvCache);
extern dvCacheCallbackType dvCacheConstructorCallback;
extern dvCacheCallbackType dvCacheDestructorCallback;
typedef void (*dvPropidentCallbackType)(dvPropident);
extern dvPropidentCallbackType dvPropidentConstructorCallback;
extern dvPropidentCallbackType dvPropidentDestructorCallback;

/*----------------------------------------------------------------------------------------
  Root structure
----------------------------------------------------------------------------------------*/
struct dvRootType_ {
    uint32 hash; /* This depends only on the structure of the database */
    uint32 usedRoot, allocatedRoot;
    uint32 usedRootModpathTable, allocatedRootModpathTable, freeRootModpathTable;
    uint32 usedRootModuleTable, allocatedRootModuleTable, freeRootModuleTable;
    uint32 usedModpath, allocatedModpath;
    uint32 usedModule, allocatedModule;
    uint32 usedModuleClassTable, allocatedModuleClassTable, freeModuleClassTable;
    uint32 usedModuleEnumTable, allocatedModuleEnumTable, freeModuleEnumTable;
    uint32 usedModuleTypedefTable, allocatedModuleTypedefTable, freeModuleTypedefTable;
    uint32 usedModuleSchemaTable, allocatedModuleSchemaTable, freeModuleSchemaTable;
    uint32 usedLink, allocatedLink;
    uint32 usedSchema, allocatedSchema;
    uint32 usedEnum, allocatedEnum;
    uint32 usedEnumEntryTable, allocatedEnumEntryTable, freeEnumEntryTable;
    uint32 usedEntry, allocatedEntry;
    uint32 usedTypedef, allocatedTypedef;
    uint32 usedTypedefInitializer, allocatedTypedefInitializer, freeTypedefInitializer;
    uint32 usedClass, allocatedClass;
    uint32 usedClassPropertyTable, allocatedClassPropertyTable, freeClassPropertyTable;
    uint32 usedClassSparsegroupTable, allocatedClassSparsegroupTable, freeClassSparsegroupTable;
    uint32 usedProperty, allocatedProperty;
    uint32 usedPropertyInitializer, allocatedPropertyInitializer, freePropertyInitializer;
    uint32 usedPropertyIndex, allocatedPropertyIndex, freePropertyIndex;
    dvSparsegroup firstFreeSparsegroup;
    uint32 usedSparsegroup, allocatedSparsegroup;
    uint32 usedRelationship, allocatedRelationship;
    dvKey firstFreeKey;
    uint32 usedKey, allocatedKey;
    dvKeyproperty firstFreeKeyproperty;
    uint32 usedKeyproperty, allocatedKeyproperty;
    dvUnion firstFreeUnion;
    uint32 usedUnion, allocatedUnion;
    dvCase firstFreeCase;
    uint32 usedCase, allocatedCase;
    dvCache firstFreeCache;
    uint32 usedCache, allocatedCache;
    dvPropident firstFreePropident;
    uint32 usedPropident, allocatedPropident;
};
extern struct dvRootType_ dvRootData;

utInlineC uint32 dvHash(void) {return dvRootData.hash;}
utInlineC uint32 dvUsedRoot(void) {return dvRootData.usedRoot;}
utInlineC uint32 dvAllocatedRoot(void) {return dvRootData.allocatedRoot;}
utInlineC void dvSetUsedRoot(uint32 value) {dvRootData.usedRoot = value;}
utInlineC void dvSetAllocatedRoot(uint32 value) {dvRootData.allocatedRoot = value;}
utInlineC uint32 dvUsedRootModpathTable(void) {return dvRootData.usedRootModpathTable;}
utInlineC uint32 dvAllocatedRootModpathTable(void) {return dvRootData.allocatedRootModpathTable;}
utInlineC uint32 dvFreeRootModpathTable(void) {return dvRootData.freeRootModpathTable;}
utInlineC void dvSetUsedRootModpathTable(uint32 value) {dvRootData.usedRootModpathTable = value;}
utInlineC void dvSetAllocatedRootModpathTable(uint32 value) {dvRootData.allocatedRootModpathTable = value;}
utInlineC void dvSetFreeRootModpathTable(int32 value) {dvRootData.freeRootModpathTable = value;}
utInlineC uint32 dvUsedRootModuleTable(void) {return dvRootData.usedRootModuleTable;}
utInlineC uint32 dvAllocatedRootModuleTable(void) {return dvRootData.allocatedRootModuleTable;}
utInlineC uint32 dvFreeRootModuleTable(void) {return dvRootData.freeRootModuleTable;}
utInlineC void dvSetUsedRootModuleTable(uint32 value) {dvRootData.usedRootModuleTable = value;}
utInlineC void dvSetAllocatedRootModuleTable(uint32 value) {dvRootData.allocatedRootModuleTable = value;}
utInlineC void dvSetFreeRootModuleTable(int32 value) {dvRootData.freeRootModuleTable = value;}
utInlineC uint32 dvUsedModpath(void) {return dvRootData.usedModpath;}
utInlineC uint32 dvAllocatedModpath(void) {return dvRootData.allocatedModpath;}
utInlineC void dvSetUsedModpath(uint32 value) {dvRootData.usedModpath = value;}
utInlineC void dvSetAllocatedModpath(uint32 value) {dvRootData.allocatedModpath = value;}
utInlineC uint32 dvUsedModule(void) {return dvRootData.usedModule;}
utInlineC uint32 dvAllocatedModule(void) {return dvRootData.allocatedModule;}
utInlineC void dvSetUsedModule(uint32 value) {dvRootData.usedModule = value;}
utInlineC void dvSetAllocatedModule(uint32 value) {dvRootData.allocatedModule = value;}
utInlineC uint32 dvUsedModuleClassTable(void) {return dvRootData.usedModuleClassTable;}
utInlineC uint32 dvAllocatedModuleClassTable(void) {return dvRootData.allocatedModuleClassTable;}
utInlineC uint32 dvFreeModuleClassTable(void) {return dvRootData.freeModuleClassTable;}
utInlineC void dvSetUsedModuleClassTable(uint32 value) {dvRootData.usedModuleClassTable = value;}
utInlineC void dvSetAllocatedModuleClassTable(uint32 value) {dvRootData.allocatedModuleClassTable = value;}
utInlineC void dvSetFreeModuleClassTable(int32 value) {dvRootData.freeModuleClassTable = value;}
utInlineC uint32 dvUsedModuleEnumTable(void) {return dvRootData.usedModuleEnumTable;}
utInlineC uint32 dvAllocatedModuleEnumTable(void) {return dvRootData.allocatedModuleEnumTable;}
utInlineC uint32 dvFreeModuleEnumTable(void) {return dvRootData.freeModuleEnumTable;}
utInlineC void dvSetUsedModuleEnumTable(uint32 value) {dvRootData.usedModuleEnumTable = value;}
utInlineC void dvSetAllocatedModuleEnumTable(uint32 value) {dvRootData.allocatedModuleEnumTable = value;}
utInlineC void dvSetFreeModuleEnumTable(int32 value) {dvRootData.freeModuleEnumTable = value;}
utInlineC uint32 dvUsedModuleTypedefTable(void) {return dvRootData.usedModuleTypedefTable;}
utInlineC uint32 dvAllocatedModuleTypedefTable(void) {return dvRootData.allocatedModuleTypedefTable;}
utInlineC uint32 dvFreeModuleTypedefTable(void) {return dvRootData.freeModuleTypedefTable;}
utInlineC void dvSetUsedModuleTypedefTable(uint32 value) {dvRootData.usedModuleTypedefTable = value;}
utInlineC void dvSetAllocatedModuleTypedefTable(uint32 value) {dvRootData.allocatedModuleTypedefTable = value;}
utInlineC void dvSetFreeModuleTypedefTable(int32 value) {dvRootData.freeModuleTypedefTable = value;}
utInlineC uint32 dvUsedModuleSchemaTable(void) {return dvRootData.usedModuleSchemaTable;}
utInlineC uint32 dvAllocatedModuleSchemaTable(void) {return dvRootData.allocatedModuleSchemaTable;}
utInlineC uint32 dvFreeModuleSchemaTable(void) {return dvRootData.freeModuleSchemaTable;}
utInlineC void dvSetUsedModuleSchemaTable(uint32 value) {dvRootData.usedModuleSchemaTable = value;}
utInlineC void dvSetAllocatedModuleSchemaTable(uint32 value) {dvRootData.allocatedModuleSchemaTable = value;}
utInlineC void dvSetFreeModuleSchemaTable(int32 value) {dvRootData.freeModuleSchemaTable = value;}
utInlineC uint32 dvUsedLink(void) {return dvRootData.usedLink;}
utInlineC uint32 dvAllocatedLink(void) {return dvRootData.allocatedLink;}
utInlineC void dvSetUsedLink(uint32 value) {dvRootData.usedLink = value;}
utInlineC void dvSetAllocatedLink(uint32 value) {dvRootData.allocatedLink = value;}
utInlineC uint32 dvUsedSchema(void) {return dvRootData.usedSchema;}
utInlineC uint32 dvAllocatedSchema(void) {return dvRootData.allocatedSchema;}
utInlineC void dvSetUsedSchema(uint32 value) {dvRootData.usedSchema = value;}
utInlineC void dvSetAllocatedSchema(uint32 value) {dvRootData.allocatedSchema = value;}
utInlineC uint32 dvUsedEnum(void) {return dvRootData.usedEnum;}
utInlineC uint32 dvAllocatedEnum(void) {return dvRootData.allocatedEnum;}
utInlineC void dvSetUsedEnum(uint32 value) {dvRootData.usedEnum = value;}
utInlineC void dvSetAllocatedEnum(uint32 value) {dvRootData.allocatedEnum = value;}
utInlineC uint32 dvUsedEnumEntryTable(void) {return dvRootData.usedEnumEntryTable;}
utInlineC uint32 dvAllocatedEnumEntryTable(void) {return dvRootData.allocatedEnumEntryTable;}
utInlineC uint32 dvFreeEnumEntryTable(void) {return dvRootData.freeEnumEntryTable;}
utInlineC void dvSetUsedEnumEntryTable(uint32 value) {dvRootData.usedEnumEntryTable = value;}
utInlineC void dvSetAllocatedEnumEntryTable(uint32 value) {dvRootData.allocatedEnumEntryTable = value;}
utInlineC void dvSetFreeEnumEntryTable(int32 value) {dvRootData.freeEnumEntryTable = value;}
utInlineC uint32 dvUsedEntry(void) {return dvRootData.usedEntry;}
utInlineC uint32 dvAllocatedEntry(void) {return dvRootData.allocatedEntry;}
utInlineC void dvSetUsedEntry(uint32 value) {dvRootData.usedEntry = value;}
utInlineC void dvSetAllocatedEntry(uint32 value) {dvRootData.allocatedEntry = value;}
utInlineC uint32 dvUsedTypedef(void) {return dvRootData.usedTypedef;}
utInlineC uint32 dvAllocatedTypedef(void) {return dvRootData.allocatedTypedef;}
utInlineC void dvSetUsedTypedef(uint32 value) {dvRootData.usedTypedef = value;}
utInlineC void dvSetAllocatedTypedef(uint32 value) {dvRootData.allocatedTypedef = value;}
utInlineC uint32 dvUsedTypedefInitializer(void) {return dvRootData.usedTypedefInitializer;}
utInlineC uint32 dvAllocatedTypedefInitializer(void) {return dvRootData.allocatedTypedefInitializer;}
utInlineC uint32 dvFreeTypedefInitializer(void) {return dvRootData.freeTypedefInitializer;}
utInlineC void dvSetUsedTypedefInitializer(uint32 value) {dvRootData.usedTypedefInitializer = value;}
utInlineC void dvSetAllocatedTypedefInitializer(uint32 value) {dvRootData.allocatedTypedefInitializer = value;}
utInlineC void dvSetFreeTypedefInitializer(int32 value) {dvRootData.freeTypedefInitializer = value;}
utInlineC uint32 dvUsedClass(void) {return dvRootData.usedClass;}
utInlineC uint32 dvAllocatedClass(void) {return dvRootData.allocatedClass;}
utInlineC void dvSetUsedClass(uint32 value) {dvRootData.usedClass = value;}
utInlineC void dvSetAllocatedClass(uint32 value) {dvRootData.allocatedClass = value;}
utInlineC uint32 dvUsedClassPropertyTable(void) {return dvRootData.usedClassPropertyTable;}
utInlineC uint32 dvAllocatedClassPropertyTable(void) {return dvRootData.allocatedClassPropertyTable;}
utInlineC uint32 dvFreeClassPropertyTable(void) {return dvRootData.freeClassPropertyTable;}
utInlineC void dvSetUsedClassPropertyTable(uint32 value) {dvRootData.usedClassPropertyTable = value;}
utInlineC void dvSetAllocatedClassPropertyTable(uint32 value) {dvRootData.allocatedClassPropertyTable = value;}
utInlineC void dvSetFreeClassPropertyTable(int32 value) {dvRootData.freeClassPropertyTable = value;}
utInlineC uint32 dvUsedClassSparsegroupTable(void) {return dvRootData.usedClassSparsegroupTable;}
utInlineC uint32 dvAllocatedClassSparsegroupTable(void) {return dvRootData.allocatedClassSparsegroupTable;}
utInlineC uint32 dvFreeClassSparsegroupTable(void) {return dvRootData.freeClassSparsegroupTable;}
utInlineC void dvSetUsedClassSparsegroupTable(uint32 value) {dvRootData.usedClassSparsegroupTable = value;}
utInlineC void dvSetAllocatedClassSparsegroupTable(uint32 value) {dvRootData.allocatedClassSparsegroupTable = value;}
utInlineC void dvSetFreeClassSparsegroupTable(int32 value) {dvRootData.freeClassSparsegroupTable = value;}
utInlineC uint32 dvUsedProperty(void) {return dvRootData.usedProperty;}
utInlineC uint32 dvAllocatedProperty(void) {return dvRootData.allocatedProperty;}
utInlineC void dvSetUsedProperty(uint32 value) {dvRootData.usedProperty = value;}
utInlineC void dvSetAllocatedProperty(uint32 value) {dvRootData.allocatedProperty = value;}
utInlineC uint32 dvUsedPropertyInitializer(void) {return dvRootData.usedPropertyInitializer;}
utInlineC uint32 dvAllocatedPropertyInitializer(void) {return dvRootData.allocatedPropertyInitializer;}
utInlineC uint32 dvFreePropertyInitializer(void) {return dvRootData.freePropertyInitializer;}
utInlineC void dvSetUsedPropertyInitializer(uint32 value) {dvRootData.usedPropertyInitializer = value;}
utInlineC void dvSetAllocatedPropertyInitializer(uint32 value) {dvRootData.allocatedPropertyInitializer = value;}
utInlineC void dvSetFreePropertyInitializer(int32 value) {dvRootData.freePropertyInitializer = value;}
utInlineC uint32 dvUsedPropertyIndex(void) {return dvRootData.usedPropertyIndex;}
utInlineC uint32 dvAllocatedPropertyIndex(void) {return dvRootData.allocatedPropertyIndex;}
utInlineC uint32 dvFreePropertyIndex(void) {return dvRootData.freePropertyIndex;}
utInlineC void dvSetUsedPropertyIndex(uint32 value) {dvRootData.usedPropertyIndex = value;}
utInlineC void dvSetAllocatedPropertyIndex(uint32 value) {dvRootData.allocatedPropertyIndex = value;}
utInlineC void dvSetFreePropertyIndex(int32 value) {dvRootData.freePropertyIndex = value;}
utInlineC dvSparsegroup dvFirstFreeSparsegroup(void) {return dvRootData.firstFreeSparsegroup;}
utInlineC void dvSetFirstFreeSparsegroup(dvSparsegroup value) {dvRootData.firstFreeSparsegroup = (value);}
utInlineC uint32 dvUsedSparsegroup(void) {return dvRootData.usedSparsegroup;}
utInlineC uint32 dvAllocatedSparsegroup(void) {return dvRootData.allocatedSparsegroup;}
utInlineC void dvSetUsedSparsegroup(uint32 value) {dvRootData.usedSparsegroup = value;}
utInlineC void dvSetAllocatedSparsegroup(uint32 value) {dvRootData.allocatedSparsegroup = value;}
utInlineC uint32 dvUsedRelationship(void) {return dvRootData.usedRelationship;}
utInlineC uint32 dvAllocatedRelationship(void) {return dvRootData.allocatedRelationship;}
utInlineC void dvSetUsedRelationship(uint32 value) {dvRootData.usedRelationship = value;}
utInlineC void dvSetAllocatedRelationship(uint32 value) {dvRootData.allocatedRelationship = value;}
utInlineC dvKey dvFirstFreeKey(void) {return dvRootData.firstFreeKey;}
utInlineC void dvSetFirstFreeKey(dvKey value) {dvRootData.firstFreeKey = (value);}
utInlineC uint32 dvUsedKey(void) {return dvRootData.usedKey;}
utInlineC uint32 dvAllocatedKey(void) {return dvRootData.allocatedKey;}
utInlineC void dvSetUsedKey(uint32 value) {dvRootData.usedKey = value;}
utInlineC void dvSetAllocatedKey(uint32 value) {dvRootData.allocatedKey = value;}
utInlineC dvKeyproperty dvFirstFreeKeyproperty(void) {return dvRootData.firstFreeKeyproperty;}
utInlineC void dvSetFirstFreeKeyproperty(dvKeyproperty value) {dvRootData.firstFreeKeyproperty = (value);}
utInlineC uint32 dvUsedKeyproperty(void) {return dvRootData.usedKeyproperty;}
utInlineC uint32 dvAllocatedKeyproperty(void) {return dvRootData.allocatedKeyproperty;}
utInlineC void dvSetUsedKeyproperty(uint32 value) {dvRootData.usedKeyproperty = value;}
utInlineC void dvSetAllocatedKeyproperty(uint32 value) {dvRootData.allocatedKeyproperty = value;}
utInlineC dvUnion dvFirstFreeUnion(void) {return dvRootData.firstFreeUnion;}
utInlineC void dvSetFirstFreeUnion(dvUnion value) {dvRootData.firstFreeUnion = (value);}
utInlineC uint32 dvUsedUnion(void) {return dvRootData.usedUnion;}
utInlineC uint32 dvAllocatedUnion(void) {return dvRootData.allocatedUnion;}
utInlineC void dvSetUsedUnion(uint32 value) {dvRootData.usedUnion = value;}
utInlineC void dvSetAllocatedUnion(uint32 value) {dvRootData.allocatedUnion = value;}
utInlineC dvCase dvFirstFreeCase(void) {return dvRootData.firstFreeCase;}
utInlineC void dvSetFirstFreeCase(dvCase value) {dvRootData.firstFreeCase = (value);}
utInlineC uint32 dvUsedCase(void) {return dvRootData.usedCase;}
utInlineC uint32 dvAllocatedCase(void) {return dvRootData.allocatedCase;}
utInlineC void dvSetUsedCase(uint32 value) {dvRootData.usedCase = value;}
utInlineC void dvSetAllocatedCase(uint32 value) {dvRootData.allocatedCase = value;}
utInlineC dvCache dvFirstFreeCache(void) {return dvRootData.firstFreeCache;}
utInlineC void dvSetFirstFreeCache(dvCache value) {dvRootData.firstFreeCache = (value);}
utInlineC uint32 dvUsedCache(void) {return dvRootData.usedCache;}
utInlineC uint32 dvAllocatedCache(void) {return dvRootData.allocatedCache;}
utInlineC void dvSetUsedCache(uint32 value) {dvRootData.usedCache = value;}
utInlineC void dvSetAllocatedCache(uint32 value) {dvRootData.allocatedCache = value;}
utInlineC dvPropident dvFirstFreePropident(void) {return dvRootData.firstFreePropident;}
utInlineC void dvSetFirstFreePropident(dvPropident value) {dvRootData.firstFreePropident = (value);}
utInlineC uint32 dvUsedPropident(void) {return dvRootData.usedPropident;}
utInlineC uint32 dvAllocatedPropident(void) {return dvRootData.allocatedPropident;}
utInlineC void dvSetUsedPropident(uint32 value) {dvRootData.usedPropident = value;}
utInlineC void dvSetAllocatedPropident(uint32 value) {dvRootData.allocatedPropident = value;}

/* Validate macros */
#if defined(DD_DEBUG)
utInlineC dvRoot dvValidRoot(dvRoot Root) {
    utAssert(utLikely(Root != dvRootNull && (uint32)(Root - (dvRoot)0) < dvRootData.usedRoot));
    return Root;}
utInlineC dvModpath dvValidModpath(dvModpath Modpath) {
    utAssert(utLikely(Modpath != dvModpathNull && (uint32)(Modpath - (dvModpath)0) < dvRootData.usedModpath));
    return Modpath;}
utInlineC dvModule dvValidModule(dvModule Module) {
    utAssert(utLikely(Module != dvModuleNull && (uint32)(Module - (dvModule)0) < dvRootData.usedModule));
    return Module;}
utInlineC dvLink dvValidLink(dvLink Link) {
    utAssert(utLikely(Link != dvLinkNull && (uint32)(Link - (dvLink)0) < dvRootData.usedLink));
    return Link;}
utInlineC dvSchema dvValidSchema(dvSchema Schema) {
    utAssert(utLikely(Schema != dvSchemaNull && (uint32)(Schema - (dvSchema)0) < dvRootData.usedSchema));
    return Schema;}
utInlineC dvEnum dvValidEnum(dvEnum Enum) {
    utAssert(utLikely(Enum != dvEnumNull && (uint32)(Enum - (dvEnum)0) < dvRootData.usedEnum));
    return Enum;}
utInlineC dvEntry dvValidEntry(dvEntry Entry) {
    utAssert(utLikely(Entry != dvEntryNull && (uint32)(Entry - (dvEntry)0) < dvRootData.usedEntry));
    return Entry;}
utInlineC dvTypedef dvValidTypedef(dvTypedef Typedef) {
    utAssert(utLikely(Typedef != dvTypedefNull && (uint32)(Typedef - (dvTypedef)0) < dvRootData.usedTypedef));
    return Typedef;}
utInlineC dvClass dvValidClass(dvClass Class) {
    utAssert(utLikely(Class != dvClassNull && (uint32)(Class - (dvClass)0) < dvRootData.usedClass));
    return Class;}
utInlineC dvProperty dvValidProperty(dvProperty Property) {
    utAssert(utLikely(Property != dvPropertyNull && (uint32)(Property - (dvProperty)0) < dvRootData.usedProperty));
    return Property;}
utInlineC dvSparsegroup dvValidSparsegroup(dvSparsegroup Sparsegroup) {
    utAssert(utLikely(Sparsegroup != dvSparsegroupNull && (uint32)(Sparsegroup - (dvSparsegroup)0) < dvRootData.usedSparsegroup));
    return Sparsegroup;}
utInlineC dvRelationship dvValidRelationship(dvRelationship Relationship) {
    utAssert(utLikely(Relationship != dvRelationshipNull && (uint32)(Relationship - (dvRelationship)0) < dvRootData.usedRelationship));
    return Relationship;}
utInlineC dvKey dvValidKey(dvKey Key) {
    utAssert(utLikely(Key != dvKeyNull && (uint32)(Key - (dvKey)0) < dvRootData.usedKey));
    return Key;}
utInlineC dvKeyproperty dvValidKeyproperty(dvKeyproperty Keyproperty) {
    utAssert(utLikely(Keyproperty != dvKeypropertyNull && (uint32)(Keyproperty - (dvKeyproperty)0) < dvRootData.usedKeyproperty));
    return Keyproperty;}
utInlineC dvUnion dvValidUnion(dvUnion Union) {
    utAssert(utLikely(Union != dvUnionNull && (uint32)(Union - (dvUnion)0) < dvRootData.usedUnion));
    return Union;}
utInlineC dvCase dvValidCase(dvCase Case) {
    utAssert(utLikely(Case != dvCaseNull && (uint32)(Case - (dvCase)0) < dvRootData.usedCase));
    return Case;}
utInlineC dvCache dvValidCache(dvCache Cache) {
    utAssert(utLikely(Cache != dvCacheNull && (uint32)(Cache - (dvCache)0) < dvRootData.usedCache));
    return Cache;}
utInlineC dvPropident dvValidPropident(dvPropident Propident) {
    utAssert(utLikely(Propident != dvPropidentNull && (uint32)(Propident - (dvPropident)0) < dvRootData.usedPropident));
    return Propident;}
#else
utInlineC dvRoot dvValidRoot(dvRoot Root) {return Root;}
utInlineC dvModpath dvValidModpath(dvModpath Modpath) {return Modpath;}
utInlineC dvModule dvValidModule(dvModule Module) {return Module;}
utInlineC dvLink dvValidLink(dvLink Link) {return Link;}
utInlineC dvSchema dvValidSchema(dvSchema Schema) {return Schema;}
utInlineC dvEnum dvValidEnum(dvEnum Enum) {return Enum;}
utInlineC dvEntry dvValidEntry(dvEntry Entry) {return Entry;}
utInlineC dvTypedef dvValidTypedef(dvTypedef Typedef) {return Typedef;}
utInlineC dvClass dvValidClass(dvClass Class) {return Class;}
utInlineC dvProperty dvValidProperty(dvProperty Property) {return Property;}
utInlineC dvSparsegroup dvValidSparsegroup(dvSparsegroup Sparsegroup) {return Sparsegroup;}
utInlineC dvRelationship dvValidRelationship(dvRelationship Relationship) {return Relationship;}
utInlineC dvKey dvValidKey(dvKey Key) {return Key;}
utInlineC dvKeyproperty dvValidKeyproperty(dvKeyproperty Keyproperty) {return Keyproperty;}
utInlineC dvUnion dvValidUnion(dvUnion Union) {return Union;}
utInlineC dvCase dvValidCase(dvCase Case) {return Case;}
utInlineC dvCache dvValidCache(dvCache Cache) {return Cache;}
utInlineC dvPropident dvValidPropident(dvPropident Propident) {return Propident;}
#endif

/* Object ref to integer conversions */
#if (defined(DD_DEBUG) && !defined(DD_NOSTRICT)) || defined(DD_STRICT)
utInlineC uint32 dvRoot2Index(dvRoot Root) {return Root - (dvRoot)0;}
utInlineC uint32 dvRoot2ValidIndex(dvRoot Root) {return dvValidRoot(Root) - (dvRoot)0;}
utInlineC dvRoot dvIndex2Root(uint32 xRoot) {return (dvRoot)(xRoot + (dvRoot)(0));}
utInlineC uint32 dvModpath2Index(dvModpath Modpath) {return Modpath - (dvModpath)0;}
utInlineC uint32 dvModpath2ValidIndex(dvModpath Modpath) {return dvValidModpath(Modpath) - (dvModpath)0;}
utInlineC dvModpath dvIndex2Modpath(uint32 xModpath) {return (dvModpath)(xModpath + (dvModpath)(0));}
utInlineC uint32 dvModule2Index(dvModule Module) {return Module - (dvModule)0;}
utInlineC uint32 dvModule2ValidIndex(dvModule Module) {return dvValidModule(Module) - (dvModule)0;}
utInlineC dvModule dvIndex2Module(uint32 xModule) {return (dvModule)(xModule + (dvModule)(0));}
utInlineC uint32 dvLink2Index(dvLink Link) {return Link - (dvLink)0;}
utInlineC uint32 dvLink2ValidIndex(dvLink Link) {return dvValidLink(Link) - (dvLink)0;}
utInlineC dvLink dvIndex2Link(uint32 xLink) {return (dvLink)(xLink + (dvLink)(0));}
utInlineC uint32 dvSchema2Index(dvSchema Schema) {return Schema - (dvSchema)0;}
utInlineC uint32 dvSchema2ValidIndex(dvSchema Schema) {return dvValidSchema(Schema) - (dvSchema)0;}
utInlineC dvSchema dvIndex2Schema(uint32 xSchema) {return (dvSchema)(xSchema + (dvSchema)(0));}
utInlineC uint32 dvEnum2Index(dvEnum Enum) {return Enum - (dvEnum)0;}
utInlineC uint32 dvEnum2ValidIndex(dvEnum Enum) {return dvValidEnum(Enum) - (dvEnum)0;}
utInlineC dvEnum dvIndex2Enum(uint32 xEnum) {return (dvEnum)(xEnum + (dvEnum)(0));}
utInlineC uint32 dvEntry2Index(dvEntry Entry) {return Entry - (dvEntry)0;}
utInlineC uint32 dvEntry2ValidIndex(dvEntry Entry) {return dvValidEntry(Entry) - (dvEntry)0;}
utInlineC dvEntry dvIndex2Entry(uint32 xEntry) {return (dvEntry)(xEntry + (dvEntry)(0));}
utInlineC uint32 dvTypedef2Index(dvTypedef Typedef) {return Typedef - (dvTypedef)0;}
utInlineC uint32 dvTypedef2ValidIndex(dvTypedef Typedef) {return dvValidTypedef(Typedef) - (dvTypedef)0;}
utInlineC dvTypedef dvIndex2Typedef(uint32 xTypedef) {return (dvTypedef)(xTypedef + (dvTypedef)(0));}
utInlineC uint32 dvClass2Index(dvClass Class) {return Class - (dvClass)0;}
utInlineC uint32 dvClass2ValidIndex(dvClass Class) {return dvValidClass(Class) - (dvClass)0;}
utInlineC dvClass dvIndex2Class(uint32 xClass) {return (dvClass)(xClass + (dvClass)(0));}
utInlineC uint32 dvProperty2Index(dvProperty Property) {return Property - (dvProperty)0;}
utInlineC uint32 dvProperty2ValidIndex(dvProperty Property) {return dvValidProperty(Property) - (dvProperty)0;}
utInlineC dvProperty dvIndex2Property(uint32 xProperty) {return (dvProperty)(xProperty + (dvProperty)(0));}
utInlineC uint32 dvSparsegroup2Index(dvSparsegroup Sparsegroup) {return Sparsegroup - (dvSparsegroup)0;}
utInlineC uint32 dvSparsegroup2ValidIndex(dvSparsegroup Sparsegroup) {return dvValidSparsegroup(Sparsegroup) - (dvSparsegroup)0;}
utInlineC dvSparsegroup dvIndex2Sparsegroup(uint32 xSparsegroup) {return (dvSparsegroup)(xSparsegroup + (dvSparsegroup)(0));}
utInlineC uint32 dvRelationship2Index(dvRelationship Relationship) {return Relationship - (dvRelationship)0;}
utInlineC uint32 dvRelationship2ValidIndex(dvRelationship Relationship) {return dvValidRelationship(Relationship) - (dvRelationship)0;}
utInlineC dvRelationship dvIndex2Relationship(uint32 xRelationship) {return (dvRelationship)(xRelationship + (dvRelationship)(0));}
utInlineC uint32 dvKey2Index(dvKey Key) {return Key - (dvKey)0;}
utInlineC uint32 dvKey2ValidIndex(dvKey Key) {return dvValidKey(Key) - (dvKey)0;}
utInlineC dvKey dvIndex2Key(uint32 xKey) {return (dvKey)(xKey + (dvKey)(0));}
utInlineC uint32 dvKeyproperty2Index(dvKeyproperty Keyproperty) {return Keyproperty - (dvKeyproperty)0;}
utInlineC uint32 dvKeyproperty2ValidIndex(dvKeyproperty Keyproperty) {return dvValidKeyproperty(Keyproperty) - (dvKeyproperty)0;}
utInlineC dvKeyproperty dvIndex2Keyproperty(uint32 xKeyproperty) {return (dvKeyproperty)(xKeyproperty + (dvKeyproperty)(0));}
utInlineC uint32 dvUnion2Index(dvUnion Union) {return Union - (dvUnion)0;}
utInlineC uint32 dvUnion2ValidIndex(dvUnion Union) {return dvValidUnion(Union) - (dvUnion)0;}
utInlineC dvUnion dvIndex2Union(uint32 xUnion) {return (dvUnion)(xUnion + (dvUnion)(0));}
utInlineC uint32 dvCase2Index(dvCase Case) {return Case - (dvCase)0;}
utInlineC uint32 dvCase2ValidIndex(dvCase Case) {return dvValidCase(Case) - (dvCase)0;}
utInlineC dvCase dvIndex2Case(uint32 xCase) {return (dvCase)(xCase + (dvCase)(0));}
utInlineC uint32 dvCache2Index(dvCache Cache) {return Cache - (dvCache)0;}
utInlineC uint32 dvCache2ValidIndex(dvCache Cache) {return dvValidCache(Cache) - (dvCache)0;}
utInlineC dvCache dvIndex2Cache(uint32 xCache) {return (dvCache)(xCache + (dvCache)(0));}
utInlineC uint32 dvPropident2Index(dvPropident Propident) {return Propident - (dvPropident)0;}
utInlineC uint32 dvPropident2ValidIndex(dvPropident Propident) {return dvValidPropident(Propident) - (dvPropident)0;}
utInlineC dvPropident dvIndex2Propident(uint32 xPropident) {return (dvPropident)(xPropident + (dvPropident)(0));}
#else
utInlineC uint32 dvRoot2Index(dvRoot Root) {return Root;}
utInlineC uint32 dvRoot2ValidIndex(dvRoot Root) {return dvValidRoot(Root);}
utInlineC dvRoot dvIndex2Root(uint32 xRoot) {return xRoot;}
utInlineC uint32 dvModpath2Index(dvModpath Modpath) {return Modpath;}
utInlineC uint32 dvModpath2ValidIndex(dvModpath Modpath) {return dvValidModpath(Modpath);}
utInlineC dvModpath dvIndex2Modpath(uint32 xModpath) {return xModpath;}
utInlineC uint32 dvModule2Index(dvModule Module) {return Module;}
utInlineC uint32 dvModule2ValidIndex(dvModule Module) {return dvValidModule(Module);}
utInlineC dvModule dvIndex2Module(uint32 xModule) {return xModule;}
utInlineC uint32 dvLink2Index(dvLink Link) {return Link;}
utInlineC uint32 dvLink2ValidIndex(dvLink Link) {return dvValidLink(Link);}
utInlineC dvLink dvIndex2Link(uint32 xLink) {return xLink;}
utInlineC uint32 dvSchema2Index(dvSchema Schema) {return Schema;}
utInlineC uint32 dvSchema2ValidIndex(dvSchema Schema) {return dvValidSchema(Schema);}
utInlineC dvSchema dvIndex2Schema(uint32 xSchema) {return xSchema;}
utInlineC uint32 dvEnum2Index(dvEnum Enum) {return Enum;}
utInlineC uint32 dvEnum2ValidIndex(dvEnum Enum) {return dvValidEnum(Enum);}
utInlineC dvEnum dvIndex2Enum(uint32 xEnum) {return xEnum;}
utInlineC uint32 dvEntry2Index(dvEntry Entry) {return Entry;}
utInlineC uint32 dvEntry2ValidIndex(dvEntry Entry) {return dvValidEntry(Entry);}
utInlineC dvEntry dvIndex2Entry(uint32 xEntry) {return xEntry;}
utInlineC uint32 dvTypedef2Index(dvTypedef Typedef) {return Typedef;}
utInlineC uint32 dvTypedef2ValidIndex(dvTypedef Typedef) {return dvValidTypedef(Typedef);}
utInlineC dvTypedef dvIndex2Typedef(uint32 xTypedef) {return xTypedef;}
utInlineC uint32 dvClass2Index(dvClass Class) {return Class;}
utInlineC uint32 dvClass2ValidIndex(dvClass Class) {return dvValidClass(Class);}
utInlineC dvClass dvIndex2Class(uint32 xClass) {return xClass;}
utInlineC uint32 dvProperty2Index(dvProperty Property) {return Property;}
utInlineC uint32 dvProperty2ValidIndex(dvProperty Property) {return dvValidProperty(Property);}
utInlineC dvProperty dvIndex2Property(uint32 xProperty) {return xProperty;}
utInlineC uint32 dvSparsegroup2Index(dvSparsegroup Sparsegroup) {return Sparsegroup;}
utInlineC uint32 dvSparsegroup2ValidIndex(dvSparsegroup Sparsegroup) {return dvValidSparsegroup(Sparsegroup);}
utInlineC dvSparsegroup dvIndex2Sparsegroup(uint32 xSparsegroup) {return xSparsegroup;}
utInlineC uint32 dvRelationship2Index(dvRelationship Relationship) {return Relationship;}
utInlineC uint32 dvRelationship2ValidIndex(dvRelationship Relationship) {return dvValidRelationship(Relationship);}
utInlineC dvRelationship dvIndex2Relationship(uint32 xRelationship) {return xRelationship;}
utInlineC uint32 dvKey2Index(dvKey Key) {return Key;}
utInlineC uint32 dvKey2ValidIndex(dvKey Key) {return dvValidKey(Key);}
utInlineC dvKey dvIndex2Key(uint32 xKey) {return xKey;}
utInlineC uint32 dvKeyproperty2Index(dvKeyproperty Keyproperty) {return Keyproperty;}
utInlineC uint32 dvKeyproperty2ValidIndex(dvKeyproperty Keyproperty) {return dvValidKeyproperty(Keyproperty);}
utInlineC dvKeyproperty dvIndex2Keyproperty(uint32 xKeyproperty) {return xKeyproperty;}
utInlineC uint32 dvUnion2Index(dvUnion Union) {return Union;}
utInlineC uint32 dvUnion2ValidIndex(dvUnion Union) {return dvValidUnion(Union);}
utInlineC dvUnion dvIndex2Union(uint32 xUnion) {return xUnion;}
utInlineC uint32 dvCase2Index(dvCase Case) {return Case;}
utInlineC uint32 dvCase2ValidIndex(dvCase Case) {return dvValidCase(Case);}
utInlineC dvCase dvIndex2Case(uint32 xCase) {return xCase;}
utInlineC uint32 dvCache2Index(dvCache Cache) {return Cache;}
utInlineC uint32 dvCache2ValidIndex(dvCache Cache) {return dvValidCache(Cache);}
utInlineC dvCache dvIndex2Cache(uint32 xCache) {return xCache;}
utInlineC uint32 dvPropident2Index(dvPropident Propident) {return Propident;}
utInlineC uint32 dvPropident2ValidIndex(dvPropident Propident) {return dvValidPropident(Propident);}
utInlineC dvPropident dvIndex2Propident(uint32 xPropident) {return xPropident;}
#endif

/*----------------------------------------------------------------------------------------
  Fields for class Root.
----------------------------------------------------------------------------------------*/
struct dvRootFields {
    dvModpath *FirstModpath;
    dvModpath *LastModpath;
    uint32 *ModpathTableIndex_;
    uint32 *NumModpathTable;
    dvModpath *ModpathTable;
    uint32 *NumModpath;
    dvModule *FirstModule;
    dvModule *LastModule;
    uint32 *ModuleTableIndex_;
    uint32 *NumModuleTable;
    dvModule *ModuleTable;
    uint32 *NumModule;
};
extern struct dvRootFields dvRoots;

void dvRootAllocMore(void);
void dvRootCopyProps(dvRoot dvOldRoot, dvRoot dvNewRoot);
void dvRootAllocModpathTables(dvRoot Root, uint32 numModpathTables);
void dvRootResizeModpathTables(dvRoot Root, uint32 numModpathTables);
void dvRootFreeModpathTables(dvRoot Root);
void dvCompactRootModpathTables(void);
void dvRootAllocModuleTables(dvRoot Root, uint32 numModuleTables);
void dvRootResizeModuleTables(dvRoot Root, uint32 numModuleTables);
void dvRootFreeModuleTables(dvRoot Root);
void dvCompactRootModuleTables(void);
utInlineC dvModpath dvRootGetFirstModpath(dvRoot Root) {return dvRoots.FirstModpath[dvRoot2ValidIndex(Root)];}
utInlineC void dvRootSetFirstModpath(dvRoot Root, dvModpath value) {dvRoots.FirstModpath[dvRoot2ValidIndex(Root)] = value;}
utInlineC dvModpath dvRootGetLastModpath(dvRoot Root) {return dvRoots.LastModpath[dvRoot2ValidIndex(Root)];}
utInlineC void dvRootSetLastModpath(dvRoot Root, dvModpath value) {dvRoots.LastModpath[dvRoot2ValidIndex(Root)] = value;}
utInlineC uint32 dvRootGetModpathTableIndex_(dvRoot Root) {return dvRoots.ModpathTableIndex_[dvRoot2ValidIndex(Root)];}
utInlineC void dvRootSetModpathTableIndex_(dvRoot Root, uint32 value) {dvRoots.ModpathTableIndex_[dvRoot2ValidIndex(Root)] = value;}
utInlineC uint32 dvRootGetNumModpathTable(dvRoot Root) {return dvRoots.NumModpathTable[dvRoot2ValidIndex(Root)];}
utInlineC void dvRootSetNumModpathTable(dvRoot Root, uint32 value) {dvRoots.NumModpathTable[dvRoot2ValidIndex(Root)] = value;}
#if defined(DD_DEBUG)
utInlineC uint32 dvRootCheckModpathTableIndex(dvRoot Root, uint32 x) {utAssert(x < dvRootGetNumModpathTable(Root)); return x;}
#else
utInlineC uint32 dvRootCheckModpathTableIndex(dvRoot Root, uint32 x) {return x;}
#endif
utInlineC dvModpath dvRootGetiModpathTable(dvRoot Root, uint32 x) {return dvRoots.ModpathTable[
    dvRootGetModpathTableIndex_(Root) + dvRootCheckModpathTableIndex(Root, x)];}
utInlineC dvModpath *dvRootGetModpathTable(dvRoot Root) {return dvRoots.ModpathTable + dvRootGetModpathTableIndex_(Root);}
#define dvRootGetModpathTables dvRootGetModpathTable
utInlineC void dvRootSetModpathTable(dvRoot Root, dvModpath *valuePtr, uint32 numModpathTable) {
    dvRootResizeModpathTables(Root, numModpathTable);
    memcpy(dvRootGetModpathTables(Root), valuePtr, numModpathTable*sizeof(dvModpath));}
utInlineC void dvRootSetiModpathTable(dvRoot Root, uint32 x, dvModpath value) {
    dvRoots.ModpathTable[dvRootGetModpathTableIndex_(Root) + dvRootCheckModpathTableIndex(Root, (x))] = value;}
utInlineC uint32 dvRootGetNumModpath(dvRoot Root) {return dvRoots.NumModpath[dvRoot2ValidIndex(Root)];}
utInlineC void dvRootSetNumModpath(dvRoot Root, uint32 value) {dvRoots.NumModpath[dvRoot2ValidIndex(Root)] = value;}
utInlineC dvModule dvRootGetFirstModule(dvRoot Root) {return dvRoots.FirstModule[dvRoot2ValidIndex(Root)];}
utInlineC void dvRootSetFirstModule(dvRoot Root, dvModule value) {dvRoots.FirstModule[dvRoot2ValidIndex(Root)] = value;}
utInlineC dvModule dvRootGetLastModule(dvRoot Root) {return dvRoots.LastModule[dvRoot2ValidIndex(Root)];}
utInlineC void dvRootSetLastModule(dvRoot Root, dvModule value) {dvRoots.LastModule[dvRoot2ValidIndex(Root)] = value;}
utInlineC uint32 dvRootGetModuleTableIndex_(dvRoot Root) {return dvRoots.ModuleTableIndex_[dvRoot2ValidIndex(Root)];}
utInlineC void dvRootSetModuleTableIndex_(dvRoot Root, uint32 value) {dvRoots.ModuleTableIndex_[dvRoot2ValidIndex(Root)] = value;}
utInlineC uint32 dvRootGetNumModuleTable(dvRoot Root) {return dvRoots.NumModuleTable[dvRoot2ValidIndex(Root)];}
utInlineC void dvRootSetNumModuleTable(dvRoot Root, uint32 value) {dvRoots.NumModuleTable[dvRoot2ValidIndex(Root)] = value;}
#if defined(DD_DEBUG)
utInlineC uint32 dvRootCheckModuleTableIndex(dvRoot Root, uint32 x) {utAssert(x < dvRootGetNumModuleTable(Root)); return x;}
#else
utInlineC uint32 dvRootCheckModuleTableIndex(dvRoot Root, uint32 x) {return x;}
#endif
utInlineC dvModule dvRootGetiModuleTable(dvRoot Root, uint32 x) {return dvRoots.ModuleTable[
    dvRootGetModuleTableIndex_(Root) + dvRootCheckModuleTableIndex(Root, x)];}
utInlineC dvModule *dvRootGetModuleTable(dvRoot Root) {return dvRoots.ModuleTable + dvRootGetModuleTableIndex_(Root);}
#define dvRootGetModuleTables dvRootGetModuleTable
utInlineC void dvRootSetModuleTable(dvRoot Root, dvModule *valuePtr, uint32 numModuleTable) {
    dvRootResizeModuleTables(Root, numModuleTable);
    memcpy(dvRootGetModuleTables(Root), valuePtr, numModuleTable*sizeof(dvModule));}
utInlineC void dvRootSetiModuleTable(dvRoot Root, uint32 x, dvModule value) {
    dvRoots.ModuleTable[dvRootGetModuleTableIndex_(Root) + dvRootCheckModuleTableIndex(Root, (x))] = value;}
utInlineC uint32 dvRootGetNumModule(dvRoot Root) {return dvRoots.NumModule[dvRoot2ValidIndex(Root)];}
utInlineC void dvRootSetNumModule(dvRoot Root, uint32 value) {dvRoots.NumModule[dvRoot2ValidIndex(Root)] = value;}
utInlineC void dvRootSetConstructorCallback(void(*func)(dvRoot)) {dvRootConstructorCallback = func;}
utInlineC dvRootCallbackType dvRootGetConstructorCallback(void) {return dvRootConstructorCallback;}
utInlineC dvRoot dvFirstRoot(void) {return dvRootData.usedRoot == 1? dvRootNull : dvIndex2Root(1);}
utInlineC dvRoot dvLastRoot(void) {return dvRootData.usedRoot == 1? dvRootNull :
    dvIndex2Root(dvRootData.usedRoot - 1);}
utInlineC dvRoot dvNextRoot(dvRoot Root) {return dvRoot2ValidIndex(Root) + 1 == dvRootData.usedRoot? dvRootNull :
    Root + 1;}
utInlineC dvRoot dvPrevRoot(dvRoot Root) {return dvRoot2ValidIndex(Root) == 1? dvRootNull : Root - 1;}
#define dvForeachRoot(var) \
    for(var = dvIndex2Root(1); dvRoot2Index(var) != dvRootData.usedRoot; var++)
#define dvEndRoot
utInlineC void dvRootFreeAll(void) {dvSetUsedRoot(1); dvSetUsedRootModpathTable(0); dvSetUsedRootModuleTable(0);}
utInlineC dvRoot dvRootAllocRaw(void) {
    dvRoot Root;
    if(dvRootData.usedRoot == dvRootData.allocatedRoot) {
        dvRootAllocMore();
    }
    Root = dvIndex2Root(dvRootData.usedRoot);
    dvSetUsedRoot(dvUsedRoot() + 1);
    return Root;}
utInlineC dvRoot dvRootAlloc(void) {
    dvRoot Root = dvRootAllocRaw();
    dvRootSetFirstModpath(Root, dvModpathNull);
    dvRootSetLastModpath(Root, dvModpathNull);
    dvRootSetModpathTableIndex_(Root, 0);
    dvRootSetNumModpathTable(Root, 0);
    dvRootSetNumModpathTable(Root, 0);
    dvRootSetNumModpath(Root, 0);
    dvRootSetFirstModule(Root, dvModuleNull);
    dvRootSetLastModule(Root, dvModuleNull);
    dvRootSetModuleTableIndex_(Root, 0);
    dvRootSetNumModuleTable(Root, 0);
    dvRootSetNumModuleTable(Root, 0);
    dvRootSetNumModule(Root, 0);
    if(dvRootConstructorCallback != NULL) {
        dvRootConstructorCallback(Root);
    }
    return Root;}

/*----------------------------------------------------------------------------------------
  Fields for class Modpath.
----------------------------------------------------------------------------------------*/
struct dvModpathFields {
    utSym *Sym;
    dvRoot *Root;
    dvModpath *NextRootModpath;
    dvModpath *PrevRootModpath;
    dvModpath *NextTableRootModpath;
};
extern struct dvModpathFields dvModpaths;

void dvModpathAllocMore(void);
void dvModpathCopyProps(dvModpath dvOldModpath, dvModpath dvNewModpath);
utInlineC utSym dvModpathGetSym(dvModpath Modpath) {return dvModpaths.Sym[dvModpath2ValidIndex(Modpath)];}
utInlineC void dvModpathSetSym(dvModpath Modpath, utSym value) {dvModpaths.Sym[dvModpath2ValidIndex(Modpath)] = value;}
utInlineC dvRoot dvModpathGetRoot(dvModpath Modpath) {return dvModpaths.Root[dvModpath2ValidIndex(Modpath)];}
utInlineC void dvModpathSetRoot(dvModpath Modpath, dvRoot value) {dvModpaths.Root[dvModpath2ValidIndex(Modpath)] = value;}
utInlineC dvModpath dvModpathGetNextRootModpath(dvModpath Modpath) {return dvModpaths.NextRootModpath[dvModpath2ValidIndex(Modpath)];}
utInlineC void dvModpathSetNextRootModpath(dvModpath Modpath, dvModpath value) {dvModpaths.NextRootModpath[dvModpath2ValidIndex(Modpath)] = value;}
utInlineC dvModpath dvModpathGetPrevRootModpath(dvModpath Modpath) {return dvModpaths.PrevRootModpath[dvModpath2ValidIndex(Modpath)];}
utInlineC void dvModpathSetPrevRootModpath(dvModpath Modpath, dvModpath value) {dvModpaths.PrevRootModpath[dvModpath2ValidIndex(Modpath)] = value;}
utInlineC dvModpath dvModpathGetNextTableRootModpath(dvModpath Modpath) {return dvModpaths.NextTableRootModpath[dvModpath2ValidIndex(Modpath)];}
utInlineC void dvModpathSetNextTableRootModpath(dvModpath Modpath, dvModpath value) {dvModpaths.NextTableRootModpath[dvModpath2ValidIndex(Modpath)] = value;}
utInlineC void dvModpathSetConstructorCallback(void(*func)(dvModpath)) {dvModpathConstructorCallback = func;}
utInlineC dvModpathCallbackType dvModpathGetConstructorCallback(void) {return dvModpathConstructorCallback;}
utInlineC dvModpath dvFirstModpath(void) {return dvRootData.usedModpath == 1? dvModpathNull : dvIndex2Modpath(1);}
utInlineC dvModpath dvLastModpath(void) {return dvRootData.usedModpath == 1? dvModpathNull :
    dvIndex2Modpath(dvRootData.usedModpath - 1);}
utInlineC dvModpath dvNextModpath(dvModpath Modpath) {return dvModpath2ValidIndex(Modpath) + 1 == dvRootData.usedModpath? dvModpathNull :
    Modpath + 1;}
utInlineC dvModpath dvPrevModpath(dvModpath Modpath) {return dvModpath2ValidIndex(Modpath) == 1? dvModpathNull : Modpath - 1;}
#define dvForeachModpath(var) \
    for(var = dvIndex2Modpath(1); dvModpath2Index(var) != dvRootData.usedModpath; var++)
#define dvEndModpath
utInlineC void dvModpathFreeAll(void) {dvSetUsedModpath(1);}
utInlineC dvModpath dvModpathAllocRaw(void) {
    dvModpath Modpath;
    if(dvRootData.usedModpath == dvRootData.allocatedModpath) {
        dvModpathAllocMore();
    }
    Modpath = dvIndex2Modpath(dvRootData.usedModpath);
    dvSetUsedModpath(dvUsedModpath() + 1);
    return Modpath;}
utInlineC dvModpath dvModpathAlloc(void) {
    dvModpath Modpath = dvModpathAllocRaw();
    dvModpathSetSym(Modpath, utSymNull);
    dvModpathSetRoot(Modpath, dvRootNull);
    dvModpathSetNextRootModpath(Modpath, dvModpathNull);
    dvModpathSetPrevRootModpath(Modpath, dvModpathNull);
    dvModpathSetNextTableRootModpath(Modpath, dvModpathNull);
    if(dvModpathConstructorCallback != NULL) {
        dvModpathConstructorCallback(Modpath);
    }
    return Modpath;}

/*----------------------------------------------------------------------------------------
  Fields for class Module.
----------------------------------------------------------------------------------------*/
struct dvModuleFields {
    utSym *Sym;
    utSym *PrefixSym;
    uint8 *Persistent;
    uint8 *UndoRedo;
    uint8 *HasSparseData;
    uint16 *NumFields;
    uint32 *NumClasses;
    uint32 *NumEnums;
    uint8 *Elaborated;
    dvModule *NextRootModule;
    dvModule *PrevRootModule;
    dvModule *NextTableRootModule;
    dvClass *FirstClass;
    dvClass *LastClass;
    uint32 *ClassTableIndex_;
    uint32 *NumClassTable;
    dvClass *ClassTable;
    uint32 *NumClass;
    dvEnum *FirstEnum;
    dvEnum *LastEnum;
    uint32 *EnumTableIndex_;
    uint32 *NumEnumTable;
    dvEnum *EnumTable;
    uint32 *NumEnum;
    dvTypedef *FirstTypedef;
    dvTypedef *LastTypedef;
    uint32 *TypedefTableIndex_;
    uint32 *NumTypedefTable;
    dvTypedef *TypedefTable;
    uint32 *NumTypedef;
    dvSchema *FirstSchema;
    dvSchema *LastSchema;
    uint32 *SchemaTableIndex_;
    uint32 *NumSchemaTable;
    dvSchema *SchemaTable;
    uint32 *NumSchema;
    dvLink *FirstImportLink;
    dvLink *LastImportLink;
    dvLink *FirstExportLink;
    dvLink *LastExportLink;
};
extern struct dvModuleFields dvModules;

void dvModuleAllocMore(void);
void dvModuleCopyProps(dvModule dvOldModule, dvModule dvNewModule);
void dvModuleSetBitfield(dvModule _Module, uint32 bitfield);
uint32 dvModuleGetBitfield(dvModule _Module);
void dvModuleAllocClassTables(dvModule Module, uint32 numClassTables);
void dvModuleResizeClassTables(dvModule Module, uint32 numClassTables);
void dvModuleFreeClassTables(dvModule Module);
void dvCompactModuleClassTables(void);
void dvModuleAllocEnumTables(dvModule Module, uint32 numEnumTables);
void dvModuleResizeEnumTables(dvModule Module, uint32 numEnumTables);
void dvModuleFreeEnumTables(dvModule Module);
void dvCompactModuleEnumTables(void);
void dvModuleAllocTypedefTables(dvModule Module, uint32 numTypedefTables);
void dvModuleResizeTypedefTables(dvModule Module, uint32 numTypedefTables);
void dvModuleFreeTypedefTables(dvModule Module);
void dvCompactModuleTypedefTables(void);
void dvModuleAllocSchemaTables(dvModule Module, uint32 numSchemaTables);
void dvModuleResizeSchemaTables(dvModule Module, uint32 numSchemaTables);
void dvModuleFreeSchemaTables(dvModule Module);
void dvCompactModuleSchemaTables(void);
utInlineC utSym dvModuleGetSym(dvModule Module) {return dvModules.Sym[dvModule2ValidIndex(Module)];}
utInlineC void dvModuleSetSym(dvModule Module, utSym value) {dvModules.Sym[dvModule2ValidIndex(Module)] = value;}
utInlineC utSym dvModuleGetPrefixSym(dvModule Module) {return dvModules.PrefixSym[dvModule2ValidIndex(Module)];}
utInlineC void dvModuleSetPrefixSym(dvModule Module, utSym value) {dvModules.PrefixSym[dvModule2ValidIndex(Module)] = value;}
utInlineC bool dvModulePersistent(dvModule Module) {
    return (dvModules.Persistent[dvModule2ValidIndex(Module) >> 3] >> (dvModule2ValidIndex(Module) & 7)) & 1;}
utInlineC void dvModuleSetPersistent(dvModule Module, bool value) {
    dvModules.Persistent[dvModule2ValidIndex(Module) >> 3] = (uint8)((dvModules.Persistent[dvModule2ValidIndex(Module) >> 3] &
        ~(1 << (dvModule2ValidIndex(Module) & 7))) | ((value != 0) << (dvModule2ValidIndex(Module) & 7)));}
utInlineC bool dvModuleUndoRedo(dvModule Module) {
    return (dvModules.UndoRedo[dvModule2ValidIndex(Module) >> 3] >> (dvModule2ValidIndex(Module) & 7)) & 1;}
utInlineC void dvModuleSetUndoRedo(dvModule Module, bool value) {
    dvModules.UndoRedo[dvModule2ValidIndex(Module) >> 3] = (uint8)((dvModules.UndoRedo[dvModule2ValidIndex(Module) >> 3] &
        ~(1 << (dvModule2ValidIndex(Module) & 7))) | ((value != 0) << (dvModule2ValidIndex(Module) & 7)));}
utInlineC bool dvModuleHasSparseData(dvModule Module) {
    return (dvModules.HasSparseData[dvModule2ValidIndex(Module) >> 3] >> (dvModule2ValidIndex(Module) & 7)) & 1;}
utInlineC void dvModuleSetHasSparseData(dvModule Module, bool value) {
    dvModules.HasSparseData[dvModule2ValidIndex(Module) >> 3] = (uint8)((dvModules.HasSparseData[dvModule2ValidIndex(Module) >> 3] &
        ~(1 << (dvModule2ValidIndex(Module) & 7))) | ((value != 0) << (dvModule2ValidIndex(Module) & 7)));}
utInlineC uint16 dvModuleGetNumFields(dvModule Module) {return dvModules.NumFields[dvModule2ValidIndex(Module)];}
utInlineC void dvModuleSetNumFields(dvModule Module, uint16 value) {dvModules.NumFields[dvModule2ValidIndex(Module)] = value;}
utInlineC uint32 dvModuleGetNumClasses(dvModule Module) {return dvModules.NumClasses[dvModule2ValidIndex(Module)];}
utInlineC void dvModuleSetNumClasses(dvModule Module, uint32 value) {dvModules.NumClasses[dvModule2ValidIndex(Module)] = value;}
utInlineC uint32 dvModuleGetNumEnums(dvModule Module) {return dvModules.NumEnums[dvModule2ValidIndex(Module)];}
utInlineC void dvModuleSetNumEnums(dvModule Module, uint32 value) {dvModules.NumEnums[dvModule2ValidIndex(Module)] = value;}
utInlineC bool dvModuleElaborated(dvModule Module) {
    return (dvModules.Elaborated[dvModule2ValidIndex(Module) >> 3] >> (dvModule2ValidIndex(Module) & 7)) & 1;}
utInlineC void dvModuleSetElaborated(dvModule Module, bool value) {
    dvModules.Elaborated[dvModule2ValidIndex(Module) >> 3] = (uint8)((dvModules.Elaborated[dvModule2ValidIndex(Module) >> 3] &
        ~(1 << (dvModule2ValidIndex(Module) & 7))) | ((value != 0) << (dvModule2ValidIndex(Module) & 7)));}
utInlineC dvModule dvModuleGetNextRootModule(dvModule Module) {return dvModules.NextRootModule[dvModule2ValidIndex(Module)];}
utInlineC void dvModuleSetNextRootModule(dvModule Module, dvModule value) {dvModules.NextRootModule[dvModule2ValidIndex(Module)] = value;}
utInlineC dvModule dvModuleGetPrevRootModule(dvModule Module) {return dvModules.PrevRootModule[dvModule2ValidIndex(Module)];}
utInlineC void dvModuleSetPrevRootModule(dvModule Module, dvModule value) {dvModules.PrevRootModule[dvModule2ValidIndex(Module)] = value;}
utInlineC dvModule dvModuleGetNextTableRootModule(dvModule Module) {return dvModules.NextTableRootModule[dvModule2ValidIndex(Module)];}
utInlineC void dvModuleSetNextTableRootModule(dvModule Module, dvModule value) {dvModules.NextTableRootModule[dvModule2ValidIndex(Module)] = value;}
utInlineC dvClass dvModuleGetFirstClass(dvModule Module) {return dvModules.FirstClass[dvModule2ValidIndex(Module)];}
utInlineC void dvModuleSetFirstClass(dvModule Module, dvClass value) {dvModules.FirstClass[dvModule2ValidIndex(Module)] = value;}
utInlineC dvClass dvModuleGetLastClass(dvModule Module) {return dvModules.LastClass[dvModule2ValidIndex(Module)];}
utInlineC void dvModuleSetLastClass(dvModule Module, dvClass value) {dvModules.LastClass[dvModule2ValidIndex(Module)] = value;}
utInlineC uint32 dvModuleGetClassTableIndex_(dvModule Module) {return dvModules.ClassTableIndex_[dvModule2ValidIndex(Module)];}
utInlineC void dvModuleSetClassTableIndex_(dvModule Module, uint32 value) {dvModules.ClassTableIndex_[dvModule2ValidIndex(Module)] = value;}
utInlineC uint32 dvModuleGetNumClassTable(dvModule Module) {return dvModules.NumClassTable[dvModule2ValidIndex(Module)];}
utInlineC void dvModuleSetNumClassTable(dvModule Module, uint32 value) {dvModules.NumClassTable[dvModule2ValidIndex(Module)] = value;}
#if defined(DD_DEBUG)
utInlineC uint32 dvModuleCheckClassTableIndex(dvModule Module, uint32 x) {utAssert(x < dvModuleGetNumClassTable(Module)); return x;}
#else
utInlineC uint32 dvModuleCheckClassTableIndex(dvModule Module, uint32 x) {return x;}
#endif
utInlineC dvClass dvModuleGetiClassTable(dvModule Module, uint32 x) {return dvModules.ClassTable[
    dvModuleGetClassTableIndex_(Module) + dvModuleCheckClassTableIndex(Module, x)];}
utInlineC dvClass *dvModuleGetClassTable(dvModule Module) {return dvModules.ClassTable + dvModuleGetClassTableIndex_(Module);}
#define dvModuleGetClassTables dvModuleGetClassTable
utInlineC void dvModuleSetClassTable(dvModule Module, dvClass *valuePtr, uint32 numClassTable) {
    dvModuleResizeClassTables(Module, numClassTable);
    memcpy(dvModuleGetClassTables(Module), valuePtr, numClassTable*sizeof(dvClass));}
utInlineC void dvModuleSetiClassTable(dvModule Module, uint32 x, dvClass value) {
    dvModules.ClassTable[dvModuleGetClassTableIndex_(Module) + dvModuleCheckClassTableIndex(Module, (x))] = value;}
utInlineC uint32 dvModuleGetNumClass(dvModule Module) {return dvModules.NumClass[dvModule2ValidIndex(Module)];}
utInlineC void dvModuleSetNumClass(dvModule Module, uint32 value) {dvModules.NumClass[dvModule2ValidIndex(Module)] = value;}
utInlineC dvEnum dvModuleGetFirstEnum(dvModule Module) {return dvModules.FirstEnum[dvModule2ValidIndex(Module)];}
utInlineC void dvModuleSetFirstEnum(dvModule Module, dvEnum value) {dvModules.FirstEnum[dvModule2ValidIndex(Module)] = value;}
utInlineC dvEnum dvModuleGetLastEnum(dvModule Module) {return dvModules.LastEnum[dvModule2ValidIndex(Module)];}
utInlineC void dvModuleSetLastEnum(dvModule Module, dvEnum value) {dvModules.LastEnum[dvModule2ValidIndex(Module)] = value;}
utInlineC uint32 dvModuleGetEnumTableIndex_(dvModule Module) {return dvModules.EnumTableIndex_[dvModule2ValidIndex(Module)];}
utInlineC void dvModuleSetEnumTableIndex_(dvModule Module, uint32 value) {dvModules.EnumTableIndex_[dvModule2ValidIndex(Module)] = value;}
utInlineC uint32 dvModuleGetNumEnumTable(dvModule Module) {return dvModules.NumEnumTable[dvModule2ValidIndex(Module)];}
utInlineC void dvModuleSetNumEnumTable(dvModule Module, uint32 value) {dvModules.NumEnumTable[dvModule2ValidIndex(Module)] = value;}
#if defined(DD_DEBUG)
utInlineC uint32 dvModuleCheckEnumTableIndex(dvModule Module, uint32 x) {utAssert(x < dvModuleGetNumEnumTable(Module)); return x;}
#else
utInlineC uint32 dvModuleCheckEnumTableIndex(dvModule Module, uint32 x) {return x;}
#endif
utInlineC dvEnum dvModuleGetiEnumTable(dvModule Module, uint32 x) {return dvModules.EnumTable[
    dvModuleGetEnumTableIndex_(Module) + dvModuleCheckEnumTableIndex(Module, x)];}
utInlineC dvEnum *dvModuleGetEnumTable(dvModule Module) {return dvModules.EnumTable + dvModuleGetEnumTableIndex_(Module);}
#define dvModuleGetEnumTables dvModuleGetEnumTable
utInlineC void dvModuleSetEnumTable(dvModule Module, dvEnum *valuePtr, uint32 numEnumTable) {
    dvModuleResizeEnumTables(Module, numEnumTable);
    memcpy(dvModuleGetEnumTables(Module), valuePtr, numEnumTable*sizeof(dvEnum));}
utInlineC void dvModuleSetiEnumTable(dvModule Module, uint32 x, dvEnum value) {
    dvModules.EnumTable[dvModuleGetEnumTableIndex_(Module) + dvModuleCheckEnumTableIndex(Module, (x))] = value;}
utInlineC uint32 dvModuleGetNumEnum(dvModule Module) {return dvModules.NumEnum[dvModule2ValidIndex(Module)];}
utInlineC void dvModuleSetNumEnum(dvModule Module, uint32 value) {dvModules.NumEnum[dvModule2ValidIndex(Module)] = value;}
utInlineC dvTypedef dvModuleGetFirstTypedef(dvModule Module) {return dvModules.FirstTypedef[dvModule2ValidIndex(Module)];}
utInlineC void dvModuleSetFirstTypedef(dvModule Module, dvTypedef value) {dvModules.FirstTypedef[dvModule2ValidIndex(Module)] = value;}
utInlineC dvTypedef dvModuleGetLastTypedef(dvModule Module) {return dvModules.LastTypedef[dvModule2ValidIndex(Module)];}
utInlineC void dvModuleSetLastTypedef(dvModule Module, dvTypedef value) {dvModules.LastTypedef[dvModule2ValidIndex(Module)] = value;}
utInlineC uint32 dvModuleGetTypedefTableIndex_(dvModule Module) {return dvModules.TypedefTableIndex_[dvModule2ValidIndex(Module)];}
utInlineC void dvModuleSetTypedefTableIndex_(dvModule Module, uint32 value) {dvModules.TypedefTableIndex_[dvModule2ValidIndex(Module)] = value;}
utInlineC uint32 dvModuleGetNumTypedefTable(dvModule Module) {return dvModules.NumTypedefTable[dvModule2ValidIndex(Module)];}
utInlineC void dvModuleSetNumTypedefTable(dvModule Module, uint32 value) {dvModules.NumTypedefTable[dvModule2ValidIndex(Module)] = value;}
#if defined(DD_DEBUG)
utInlineC uint32 dvModuleCheckTypedefTableIndex(dvModule Module, uint32 x) {utAssert(x < dvModuleGetNumTypedefTable(Module)); return x;}
#else
utInlineC uint32 dvModuleCheckTypedefTableIndex(dvModule Module, uint32 x) {return x;}
#endif
utInlineC dvTypedef dvModuleGetiTypedefTable(dvModule Module, uint32 x) {return dvModules.TypedefTable[
    dvModuleGetTypedefTableIndex_(Module) + dvModuleCheckTypedefTableIndex(Module, x)];}
utInlineC dvTypedef *dvModuleGetTypedefTable(dvModule Module) {return dvModules.TypedefTable + dvModuleGetTypedefTableIndex_(Module);}
#define dvModuleGetTypedefTables dvModuleGetTypedefTable
utInlineC void dvModuleSetTypedefTable(dvModule Module, dvTypedef *valuePtr, uint32 numTypedefTable) {
    dvModuleResizeTypedefTables(Module, numTypedefTable);
    memcpy(dvModuleGetTypedefTables(Module), valuePtr, numTypedefTable*sizeof(dvTypedef));}
utInlineC void dvModuleSetiTypedefTable(dvModule Module, uint32 x, dvTypedef value) {
    dvModules.TypedefTable[dvModuleGetTypedefTableIndex_(Module) + dvModuleCheckTypedefTableIndex(Module, (x))] = value;}
utInlineC uint32 dvModuleGetNumTypedef(dvModule Module) {return dvModules.NumTypedef[dvModule2ValidIndex(Module)];}
utInlineC void dvModuleSetNumTypedef(dvModule Module, uint32 value) {dvModules.NumTypedef[dvModule2ValidIndex(Module)] = value;}
utInlineC dvSchema dvModuleGetFirstSchema(dvModule Module) {return dvModules.FirstSchema[dvModule2ValidIndex(Module)];}
utInlineC void dvModuleSetFirstSchema(dvModule Module, dvSchema value) {dvModules.FirstSchema[dvModule2ValidIndex(Module)] = value;}
utInlineC dvSchema dvModuleGetLastSchema(dvModule Module) {return dvModules.LastSchema[dvModule2ValidIndex(Module)];}
utInlineC void dvModuleSetLastSchema(dvModule Module, dvSchema value) {dvModules.LastSchema[dvModule2ValidIndex(Module)] = value;}
utInlineC uint32 dvModuleGetSchemaTableIndex_(dvModule Module) {return dvModules.SchemaTableIndex_[dvModule2ValidIndex(Module)];}
utInlineC void dvModuleSetSchemaTableIndex_(dvModule Module, uint32 value) {dvModules.SchemaTableIndex_[dvModule2ValidIndex(Module)] = value;}
utInlineC uint32 dvModuleGetNumSchemaTable(dvModule Module) {return dvModules.NumSchemaTable[dvModule2ValidIndex(Module)];}
utInlineC void dvModuleSetNumSchemaTable(dvModule Module, uint32 value) {dvModules.NumSchemaTable[dvModule2ValidIndex(Module)] = value;}
#if defined(DD_DEBUG)
utInlineC uint32 dvModuleCheckSchemaTableIndex(dvModule Module, uint32 x) {utAssert(x < dvModuleGetNumSchemaTable(Module)); return x;}
#else
utInlineC uint32 dvModuleCheckSchemaTableIndex(dvModule Module, uint32 x) {return x;}
#endif
utInlineC dvSchema dvModuleGetiSchemaTable(dvModule Module, uint32 x) {return dvModules.SchemaTable[
    dvModuleGetSchemaTableIndex_(Module) + dvModuleCheckSchemaTableIndex(Module, x)];}
utInlineC dvSchema *dvModuleGetSchemaTable(dvModule Module) {return dvModules.SchemaTable + dvModuleGetSchemaTableIndex_(Module);}
#define dvModuleGetSchemaTables dvModuleGetSchemaTable
utInlineC void dvModuleSetSchemaTable(dvModule Module, dvSchema *valuePtr, uint32 numSchemaTable) {
    dvModuleResizeSchemaTables(Module, numSchemaTable);
    memcpy(dvModuleGetSchemaTables(Module), valuePtr, numSchemaTable*sizeof(dvSchema));}
utInlineC void dvModuleSetiSchemaTable(dvModule Module, uint32 x, dvSchema value) {
    dvModules.SchemaTable[dvModuleGetSchemaTableIndex_(Module) + dvModuleCheckSchemaTableIndex(Module, (x))] = value;}
utInlineC uint32 dvModuleGetNumSchema(dvModule Module) {return dvModules.NumSchema[dvModule2ValidIndex(Module)];}
utInlineC void dvModuleSetNumSchema(dvModule Module, uint32 value) {dvModules.NumSchema[dvModule2ValidIndex(Module)] = value;}
utInlineC dvLink dvModuleGetFirstImportLink(dvModule Module) {return dvModules.FirstImportLink[dvModule2ValidIndex(Module)];}
utInlineC void dvModuleSetFirstImportLink(dvModule Module, dvLink value) {dvModules.FirstImportLink[dvModule2ValidIndex(Module)] = value;}
utInlineC dvLink dvModuleGetLastImportLink(dvModule Module) {return dvModules.LastImportLink[dvModule2ValidIndex(Module)];}
utInlineC void dvModuleSetLastImportLink(dvModule Module, dvLink value) {dvModules.LastImportLink[dvModule2ValidIndex(Module)] = value;}
utInlineC dvLink dvModuleGetFirstExportLink(dvModule Module) {return dvModules.FirstExportLink[dvModule2ValidIndex(Module)];}
utInlineC void dvModuleSetFirstExportLink(dvModule Module, dvLink value) {dvModules.FirstExportLink[dvModule2ValidIndex(Module)] = value;}
utInlineC dvLink dvModuleGetLastExportLink(dvModule Module) {return dvModules.LastExportLink[dvModule2ValidIndex(Module)];}
utInlineC void dvModuleSetLastExportLink(dvModule Module, dvLink value) {dvModules.LastExportLink[dvModule2ValidIndex(Module)] = value;}
utInlineC void dvModuleSetConstructorCallback(void(*func)(dvModule)) {dvModuleConstructorCallback = func;}
utInlineC dvModuleCallbackType dvModuleGetConstructorCallback(void) {return dvModuleConstructorCallback;}
utInlineC dvModule dvFirstModule(void) {return dvRootData.usedModule == 1? dvModuleNull : dvIndex2Module(1);}
utInlineC dvModule dvLastModule(void) {return dvRootData.usedModule == 1? dvModuleNull :
    dvIndex2Module(dvRootData.usedModule - 1);}
utInlineC dvModule dvNextModule(dvModule Module) {return dvModule2ValidIndex(Module) + 1 == dvRootData.usedModule? dvModuleNull :
    Module + 1;}
utInlineC dvModule dvPrevModule(dvModule Module) {return dvModule2ValidIndex(Module) == 1? dvModuleNull : Module - 1;}
#define dvForeachModule(var) \
    for(var = dvIndex2Module(1); dvModule2Index(var) != dvRootData.usedModule; var++)
#define dvEndModule
utInlineC void dvModuleFreeAll(void) {dvSetUsedModule(1); dvSetUsedModuleClassTable(0); dvSetUsedModuleEnumTable(0); dvSetUsedModuleTypedefTable(0); dvSetUsedModuleSchemaTable(0);}
utInlineC dvModule dvModuleAllocRaw(void) {
    dvModule Module;
    if(dvRootData.usedModule == dvRootData.allocatedModule) {
        dvModuleAllocMore();
    }
    Module = dvIndex2Module(dvRootData.usedModule);
    dvSetUsedModule(dvUsedModule() + 1);
    return Module;}
utInlineC dvModule dvModuleAlloc(void) {
    dvModule Module = dvModuleAllocRaw();
    dvModuleSetSym(Module, utSymNull);
    dvModuleSetPrefixSym(Module, utSymNull);
    dvModuleSetPersistent(Module, 0);
    dvModuleSetUndoRedo(Module, 0);
    dvModuleSetHasSparseData(Module, 0);
    dvModuleSetNumFields(Module, 0);
    dvModuleSetNumClasses(Module, 0);
    dvModuleSetNumEnums(Module, 0);
    dvModuleSetElaborated(Module, 0);
    dvModuleSetNextRootModule(Module, dvModuleNull);
    dvModuleSetPrevRootModule(Module, dvModuleNull);
    dvModuleSetNextTableRootModule(Module, dvModuleNull);
    dvModuleSetFirstClass(Module, dvClassNull);
    dvModuleSetLastClass(Module, dvClassNull);
    dvModuleSetClassTableIndex_(Module, 0);
    dvModuleSetNumClassTable(Module, 0);
    dvModuleSetNumClassTable(Module, 0);
    dvModuleSetNumClass(Module, 0);
    dvModuleSetFirstEnum(Module, dvEnumNull);
    dvModuleSetLastEnum(Module, dvEnumNull);
    dvModuleSetEnumTableIndex_(Module, 0);
    dvModuleSetNumEnumTable(Module, 0);
    dvModuleSetNumEnumTable(Module, 0);
    dvModuleSetNumEnum(Module, 0);
    dvModuleSetFirstTypedef(Module, dvTypedefNull);
    dvModuleSetLastTypedef(Module, dvTypedefNull);
    dvModuleSetTypedefTableIndex_(Module, 0);
    dvModuleSetNumTypedefTable(Module, 0);
    dvModuleSetNumTypedefTable(Module, 0);
    dvModuleSetNumTypedef(Module, 0);
    dvModuleSetFirstSchema(Module, dvSchemaNull);
    dvModuleSetLastSchema(Module, dvSchemaNull);
    dvModuleSetSchemaTableIndex_(Module, 0);
    dvModuleSetNumSchemaTable(Module, 0);
    dvModuleSetNumSchemaTable(Module, 0);
    dvModuleSetNumSchema(Module, 0);
    dvModuleSetFirstImportLink(Module, dvLinkNull);
    dvModuleSetLastImportLink(Module, dvLinkNull);
    dvModuleSetFirstExportLink(Module, dvLinkNull);
    dvModuleSetLastExportLink(Module, dvLinkNull);
    if(dvModuleConstructorCallback != NULL) {
        dvModuleConstructorCallback(Module);
    }
    return Module;}

/*----------------------------------------------------------------------------------------
  Fields for class Link.
----------------------------------------------------------------------------------------*/
struct dvLinkFields {
    dvModule *ImportModule;
    dvLink *NextModuleImportLink;
    dvModule *ExportModule;
    dvLink *NextModuleExportLink;
};
extern struct dvLinkFields dvLinks;

void dvLinkAllocMore(void);
void dvLinkCopyProps(dvLink dvOldLink, dvLink dvNewLink);
utInlineC dvModule dvLinkGetImportModule(dvLink Link) {return dvLinks.ImportModule[dvLink2ValidIndex(Link)];}
utInlineC void dvLinkSetImportModule(dvLink Link, dvModule value) {dvLinks.ImportModule[dvLink2ValidIndex(Link)] = value;}
utInlineC dvLink dvLinkGetNextModuleImportLink(dvLink Link) {return dvLinks.NextModuleImportLink[dvLink2ValidIndex(Link)];}
utInlineC void dvLinkSetNextModuleImportLink(dvLink Link, dvLink value) {dvLinks.NextModuleImportLink[dvLink2ValidIndex(Link)] = value;}
utInlineC dvModule dvLinkGetExportModule(dvLink Link) {return dvLinks.ExportModule[dvLink2ValidIndex(Link)];}
utInlineC void dvLinkSetExportModule(dvLink Link, dvModule value) {dvLinks.ExportModule[dvLink2ValidIndex(Link)] = value;}
utInlineC dvLink dvLinkGetNextModuleExportLink(dvLink Link) {return dvLinks.NextModuleExportLink[dvLink2ValidIndex(Link)];}
utInlineC void dvLinkSetNextModuleExportLink(dvLink Link, dvLink value) {dvLinks.NextModuleExportLink[dvLink2ValidIndex(Link)] = value;}
utInlineC void dvLinkSetConstructorCallback(void(*func)(dvLink)) {dvLinkConstructorCallback = func;}
utInlineC dvLinkCallbackType dvLinkGetConstructorCallback(void) {return dvLinkConstructorCallback;}
utInlineC dvLink dvFirstLink(void) {return dvRootData.usedLink == 1? dvLinkNull : dvIndex2Link(1);}
utInlineC dvLink dvLastLink(void) {return dvRootData.usedLink == 1? dvLinkNull :
    dvIndex2Link(dvRootData.usedLink - 1);}
utInlineC dvLink dvNextLink(dvLink Link) {return dvLink2ValidIndex(Link) + 1 == dvRootData.usedLink? dvLinkNull :
    Link + 1;}
utInlineC dvLink dvPrevLink(dvLink Link) {return dvLink2ValidIndex(Link) == 1? dvLinkNull : Link - 1;}
#define dvForeachLink(var) \
    for(var = dvIndex2Link(1); dvLink2Index(var) != dvRootData.usedLink; var++)
#define dvEndLink
utInlineC void dvLinkFreeAll(void) {dvSetUsedLink(1);}
utInlineC dvLink dvLinkAllocRaw(void) {
    dvLink Link;
    if(dvRootData.usedLink == dvRootData.allocatedLink) {
        dvLinkAllocMore();
    }
    Link = dvIndex2Link(dvRootData.usedLink);
    dvSetUsedLink(dvUsedLink() + 1);
    return Link;}
utInlineC dvLink dvLinkAlloc(void) {
    dvLink Link = dvLinkAllocRaw();
    dvLinkSetImportModule(Link, dvModuleNull);
    dvLinkSetNextModuleImportLink(Link, dvLinkNull);
    dvLinkSetExportModule(Link, dvModuleNull);
    dvLinkSetNextModuleExportLink(Link, dvLinkNull);
    if(dvLinkConstructorCallback != NULL) {
        dvLinkConstructorCallback(Link);
    }
    return Link;}

/*----------------------------------------------------------------------------------------
  Fields for class Schema.
----------------------------------------------------------------------------------------*/
struct dvSchemaFields {
    utSym *Sym;
    dvModule *Module;
    dvSchema *NextModuleSchema;
    dvSchema *PrevModuleSchema;
    dvSchema *NextTableModuleSchema;
    dvRelationship *FirstRelationship;
    dvRelationship *LastRelationship;
};
extern struct dvSchemaFields dvSchemas;

void dvSchemaAllocMore(void);
void dvSchemaCopyProps(dvSchema dvOldSchema, dvSchema dvNewSchema);
utInlineC utSym dvSchemaGetSym(dvSchema Schema) {return dvSchemas.Sym[dvSchema2ValidIndex(Schema)];}
utInlineC void dvSchemaSetSym(dvSchema Schema, utSym value) {dvSchemas.Sym[dvSchema2ValidIndex(Schema)] = value;}
utInlineC dvModule dvSchemaGetModule(dvSchema Schema) {return dvSchemas.Module[dvSchema2ValidIndex(Schema)];}
utInlineC void dvSchemaSetModule(dvSchema Schema, dvModule value) {dvSchemas.Module[dvSchema2ValidIndex(Schema)] = value;}
utInlineC dvSchema dvSchemaGetNextModuleSchema(dvSchema Schema) {return dvSchemas.NextModuleSchema[dvSchema2ValidIndex(Schema)];}
utInlineC void dvSchemaSetNextModuleSchema(dvSchema Schema, dvSchema value) {dvSchemas.NextModuleSchema[dvSchema2ValidIndex(Schema)] = value;}
utInlineC dvSchema dvSchemaGetPrevModuleSchema(dvSchema Schema) {return dvSchemas.PrevModuleSchema[dvSchema2ValidIndex(Schema)];}
utInlineC void dvSchemaSetPrevModuleSchema(dvSchema Schema, dvSchema value) {dvSchemas.PrevModuleSchema[dvSchema2ValidIndex(Schema)] = value;}
utInlineC dvSchema dvSchemaGetNextTableModuleSchema(dvSchema Schema) {return dvSchemas.NextTableModuleSchema[dvSchema2ValidIndex(Schema)];}
utInlineC void dvSchemaSetNextTableModuleSchema(dvSchema Schema, dvSchema value) {dvSchemas.NextTableModuleSchema[dvSchema2ValidIndex(Schema)] = value;}
utInlineC dvRelationship dvSchemaGetFirstRelationship(dvSchema Schema) {return dvSchemas.FirstRelationship[dvSchema2ValidIndex(Schema)];}
utInlineC void dvSchemaSetFirstRelationship(dvSchema Schema, dvRelationship value) {dvSchemas.FirstRelationship[dvSchema2ValidIndex(Schema)] = value;}
utInlineC dvRelationship dvSchemaGetLastRelationship(dvSchema Schema) {return dvSchemas.LastRelationship[dvSchema2ValidIndex(Schema)];}
utInlineC void dvSchemaSetLastRelationship(dvSchema Schema, dvRelationship value) {dvSchemas.LastRelationship[dvSchema2ValidIndex(Schema)] = value;}
utInlineC void dvSchemaSetConstructorCallback(void(*func)(dvSchema)) {dvSchemaConstructorCallback = func;}
utInlineC dvSchemaCallbackType dvSchemaGetConstructorCallback(void) {return dvSchemaConstructorCallback;}
utInlineC dvSchema dvFirstSchema(void) {return dvRootData.usedSchema == 1? dvSchemaNull : dvIndex2Schema(1);}
utInlineC dvSchema dvLastSchema(void) {return dvRootData.usedSchema == 1? dvSchemaNull :
    dvIndex2Schema(dvRootData.usedSchema - 1);}
utInlineC dvSchema dvNextSchema(dvSchema Schema) {return dvSchema2ValidIndex(Schema) + 1 == dvRootData.usedSchema? dvSchemaNull :
    Schema + 1;}
utInlineC dvSchema dvPrevSchema(dvSchema Schema) {return dvSchema2ValidIndex(Schema) == 1? dvSchemaNull : Schema - 1;}
#define dvForeachSchema(var) \
    for(var = dvIndex2Schema(1); dvSchema2Index(var) != dvRootData.usedSchema; var++)
#define dvEndSchema
utInlineC void dvSchemaFreeAll(void) {dvSetUsedSchema(1);}
utInlineC dvSchema dvSchemaAllocRaw(void) {
    dvSchema Schema;
    if(dvRootData.usedSchema == dvRootData.allocatedSchema) {
        dvSchemaAllocMore();
    }
    Schema = dvIndex2Schema(dvRootData.usedSchema);
    dvSetUsedSchema(dvUsedSchema() + 1);
    return Schema;}
utInlineC dvSchema dvSchemaAlloc(void) {
    dvSchema Schema = dvSchemaAllocRaw();
    dvSchemaSetSym(Schema, utSymNull);
    dvSchemaSetModule(Schema, dvModuleNull);
    dvSchemaSetNextModuleSchema(Schema, dvSchemaNull);
    dvSchemaSetPrevModuleSchema(Schema, dvSchemaNull);
    dvSchemaSetNextTableModuleSchema(Schema, dvSchemaNull);
    dvSchemaSetFirstRelationship(Schema, dvRelationshipNull);
    dvSchemaSetLastRelationship(Schema, dvRelationshipNull);
    if(dvSchemaConstructorCallback != NULL) {
        dvSchemaConstructorCallback(Schema);
    }
    return Schema;}

/*----------------------------------------------------------------------------------------
  Fields for class Enum.
----------------------------------------------------------------------------------------*/
struct dvEnumFields {
    utSym *Sym;
    utSym *PrefixSym;
    uint16 *NumEntries;
    dvModule *Module;
    dvEnum *NextModuleEnum;
    dvEnum *PrevModuleEnum;
    dvEnum *NextTableModuleEnum;
    dvEntry *FirstEntry;
    dvEntry *LastEntry;
    uint32 *EntryTableIndex_;
    uint32 *NumEntryTable;
    dvEntry *EntryTable;
    uint32 *NumEntry;
};
extern struct dvEnumFields dvEnums;

void dvEnumAllocMore(void);
void dvEnumCopyProps(dvEnum dvOldEnum, dvEnum dvNewEnum);
void dvEnumAllocEntryTables(dvEnum Enum, uint32 numEntryTables);
void dvEnumResizeEntryTables(dvEnum Enum, uint32 numEntryTables);
void dvEnumFreeEntryTables(dvEnum Enum);
void dvCompactEnumEntryTables(void);
utInlineC utSym dvEnumGetSym(dvEnum Enum) {return dvEnums.Sym[dvEnum2ValidIndex(Enum)];}
utInlineC void dvEnumSetSym(dvEnum Enum, utSym value) {dvEnums.Sym[dvEnum2ValidIndex(Enum)] = value;}
utInlineC utSym dvEnumGetPrefixSym(dvEnum Enum) {return dvEnums.PrefixSym[dvEnum2ValidIndex(Enum)];}
utInlineC void dvEnumSetPrefixSym(dvEnum Enum, utSym value) {dvEnums.PrefixSym[dvEnum2ValidIndex(Enum)] = value;}
utInlineC uint16 dvEnumGetNumEntries(dvEnum Enum) {return dvEnums.NumEntries[dvEnum2ValidIndex(Enum)];}
utInlineC void dvEnumSetNumEntries(dvEnum Enum, uint16 value) {dvEnums.NumEntries[dvEnum2ValidIndex(Enum)] = value;}
utInlineC dvModule dvEnumGetModule(dvEnum Enum) {return dvEnums.Module[dvEnum2ValidIndex(Enum)];}
utInlineC void dvEnumSetModule(dvEnum Enum, dvModule value) {dvEnums.Module[dvEnum2ValidIndex(Enum)] = value;}
utInlineC dvEnum dvEnumGetNextModuleEnum(dvEnum Enum) {return dvEnums.NextModuleEnum[dvEnum2ValidIndex(Enum)];}
utInlineC void dvEnumSetNextModuleEnum(dvEnum Enum, dvEnum value) {dvEnums.NextModuleEnum[dvEnum2ValidIndex(Enum)] = value;}
utInlineC dvEnum dvEnumGetPrevModuleEnum(dvEnum Enum) {return dvEnums.PrevModuleEnum[dvEnum2ValidIndex(Enum)];}
utInlineC void dvEnumSetPrevModuleEnum(dvEnum Enum, dvEnum value) {dvEnums.PrevModuleEnum[dvEnum2ValidIndex(Enum)] = value;}
utInlineC dvEnum dvEnumGetNextTableModuleEnum(dvEnum Enum) {return dvEnums.NextTableModuleEnum[dvEnum2ValidIndex(Enum)];}
utInlineC void dvEnumSetNextTableModuleEnum(dvEnum Enum, dvEnum value) {dvEnums.NextTableModuleEnum[dvEnum2ValidIndex(Enum)] = value;}
utInlineC dvEntry dvEnumGetFirstEntry(dvEnum Enum) {return dvEnums.FirstEntry[dvEnum2ValidIndex(Enum)];}
utInlineC void dvEnumSetFirstEntry(dvEnum Enum, dvEntry value) {dvEnums.FirstEntry[dvEnum2ValidIndex(Enum)] = value;}
utInlineC dvEntry dvEnumGetLastEntry(dvEnum Enum) {return dvEnums.LastEntry[dvEnum2ValidIndex(Enum)];}
utInlineC void dvEnumSetLastEntry(dvEnum Enum, dvEntry value) {dvEnums.LastEntry[dvEnum2ValidIndex(Enum)] = value;}
utInlineC uint32 dvEnumGetEntryTableIndex_(dvEnum Enum) {return dvEnums.EntryTableIndex_[dvEnum2ValidIndex(Enum)];}
utInlineC void dvEnumSetEntryTableIndex_(dvEnum Enum, uint32 value) {dvEnums.EntryTableIndex_[dvEnum2ValidIndex(Enum)] = value;}
utInlineC uint32 dvEnumGetNumEntryTable(dvEnum Enum) {return dvEnums.NumEntryTable[dvEnum2ValidIndex(Enum)];}
utInlineC void dvEnumSetNumEntryTable(dvEnum Enum, uint32 value) {dvEnums.NumEntryTable[dvEnum2ValidIndex(Enum)] = value;}
#if defined(DD_DEBUG)
utInlineC uint32 dvEnumCheckEntryTableIndex(dvEnum Enum, uint32 x) {utAssert(x < dvEnumGetNumEntryTable(Enum)); return x;}
#else
utInlineC uint32 dvEnumCheckEntryTableIndex(dvEnum Enum, uint32 x) {return x;}
#endif
utInlineC dvEntry dvEnumGetiEntryTable(dvEnum Enum, uint32 x) {return dvEnums.EntryTable[
    dvEnumGetEntryTableIndex_(Enum) + dvEnumCheckEntryTableIndex(Enum, x)];}
utInlineC dvEntry *dvEnumGetEntryTable(dvEnum Enum) {return dvEnums.EntryTable + dvEnumGetEntryTableIndex_(Enum);}
#define dvEnumGetEntryTables dvEnumGetEntryTable
utInlineC void dvEnumSetEntryTable(dvEnum Enum, dvEntry *valuePtr, uint32 numEntryTable) {
    dvEnumResizeEntryTables(Enum, numEntryTable);
    memcpy(dvEnumGetEntryTables(Enum), valuePtr, numEntryTable*sizeof(dvEntry));}
utInlineC void dvEnumSetiEntryTable(dvEnum Enum, uint32 x, dvEntry value) {
    dvEnums.EntryTable[dvEnumGetEntryTableIndex_(Enum) + dvEnumCheckEntryTableIndex(Enum, (x))] = value;}
utInlineC uint32 dvEnumGetNumEntry(dvEnum Enum) {return dvEnums.NumEntry[dvEnum2ValidIndex(Enum)];}
utInlineC void dvEnumSetNumEntry(dvEnum Enum, uint32 value) {dvEnums.NumEntry[dvEnum2ValidIndex(Enum)] = value;}
utInlineC void dvEnumSetConstructorCallback(void(*func)(dvEnum)) {dvEnumConstructorCallback = func;}
utInlineC dvEnumCallbackType dvEnumGetConstructorCallback(void) {return dvEnumConstructorCallback;}
utInlineC dvEnum dvFirstEnum(void) {return dvRootData.usedEnum == 1? dvEnumNull : dvIndex2Enum(1);}
utInlineC dvEnum dvLastEnum(void) {return dvRootData.usedEnum == 1? dvEnumNull :
    dvIndex2Enum(dvRootData.usedEnum - 1);}
utInlineC dvEnum dvNextEnum(dvEnum Enum) {return dvEnum2ValidIndex(Enum) + 1 == dvRootData.usedEnum? dvEnumNull :
    Enum + 1;}
utInlineC dvEnum dvPrevEnum(dvEnum Enum) {return dvEnum2ValidIndex(Enum) == 1? dvEnumNull : Enum - 1;}
#define dvForeachEnum(var) \
    for(var = dvIndex2Enum(1); dvEnum2Index(var) != dvRootData.usedEnum; var++)
#define dvEndEnum
utInlineC void dvEnumFreeAll(void) {dvSetUsedEnum(1); dvSetUsedEnumEntryTable(0);}
utInlineC dvEnum dvEnumAllocRaw(void) {
    dvEnum Enum;
    if(dvRootData.usedEnum == dvRootData.allocatedEnum) {
        dvEnumAllocMore();
    }
    Enum = dvIndex2Enum(dvRootData.usedEnum);
    dvSetUsedEnum(dvUsedEnum() + 1);
    return Enum;}
utInlineC dvEnum dvEnumAlloc(void) {
    dvEnum Enum = dvEnumAllocRaw();
    dvEnumSetSym(Enum, utSymNull);
    dvEnumSetPrefixSym(Enum, utSymNull);
    dvEnumSetNumEntries(Enum, 0);
    dvEnumSetModule(Enum, dvModuleNull);
    dvEnumSetNextModuleEnum(Enum, dvEnumNull);
    dvEnumSetPrevModuleEnum(Enum, dvEnumNull);
    dvEnumSetNextTableModuleEnum(Enum, dvEnumNull);
    dvEnumSetFirstEntry(Enum, dvEntryNull);
    dvEnumSetLastEntry(Enum, dvEntryNull);
    dvEnumSetEntryTableIndex_(Enum, 0);
    dvEnumSetNumEntryTable(Enum, 0);
    dvEnumSetNumEntryTable(Enum, 0);
    dvEnumSetNumEntry(Enum, 0);
    if(dvEnumConstructorCallback != NULL) {
        dvEnumConstructorCallback(Enum);
    }
    return Enum;}

/*----------------------------------------------------------------------------------------
  Fields for class Entry.
----------------------------------------------------------------------------------------*/
struct dvEntryFields {
    utSym *Sym;
    uint32 *Value;
    dvEnum *Enum;
    dvEntry *NextEnumEntry;
    dvEntry *PrevEnumEntry;
    dvEntry *NextTableEnumEntry;
    dvCase *FirstCase;
    dvCase *LastCase;
};
extern struct dvEntryFields dvEntrys;

void dvEntryAllocMore(void);
void dvEntryCopyProps(dvEntry dvOldEntry, dvEntry dvNewEntry);
utInlineC utSym dvEntryGetSym(dvEntry Entry) {return dvEntrys.Sym[dvEntry2ValidIndex(Entry)];}
utInlineC void dvEntrySetSym(dvEntry Entry, utSym value) {dvEntrys.Sym[dvEntry2ValidIndex(Entry)] = value;}
utInlineC uint32 dvEntryGetValue(dvEntry Entry) {return dvEntrys.Value[dvEntry2ValidIndex(Entry)];}
utInlineC void dvEntrySetValue(dvEntry Entry, uint32 value) {dvEntrys.Value[dvEntry2ValidIndex(Entry)] = value;}
utInlineC dvEnum dvEntryGetEnum(dvEntry Entry) {return dvEntrys.Enum[dvEntry2ValidIndex(Entry)];}
utInlineC void dvEntrySetEnum(dvEntry Entry, dvEnum value) {dvEntrys.Enum[dvEntry2ValidIndex(Entry)] = value;}
utInlineC dvEntry dvEntryGetNextEnumEntry(dvEntry Entry) {return dvEntrys.NextEnumEntry[dvEntry2ValidIndex(Entry)];}
utInlineC void dvEntrySetNextEnumEntry(dvEntry Entry, dvEntry value) {dvEntrys.NextEnumEntry[dvEntry2ValidIndex(Entry)] = value;}
utInlineC dvEntry dvEntryGetPrevEnumEntry(dvEntry Entry) {return dvEntrys.PrevEnumEntry[dvEntry2ValidIndex(Entry)];}
utInlineC void dvEntrySetPrevEnumEntry(dvEntry Entry, dvEntry value) {dvEntrys.PrevEnumEntry[dvEntry2ValidIndex(Entry)] = value;}
utInlineC dvEntry dvEntryGetNextTableEnumEntry(dvEntry Entry) {return dvEntrys.NextTableEnumEntry[dvEntry2ValidIndex(Entry)];}
utInlineC void dvEntrySetNextTableEnumEntry(dvEntry Entry, dvEntry value) {dvEntrys.NextTableEnumEntry[dvEntry2ValidIndex(Entry)] = value;}
utInlineC dvCase dvEntryGetFirstCase(dvEntry Entry) {return dvEntrys.FirstCase[dvEntry2ValidIndex(Entry)];}
utInlineC void dvEntrySetFirstCase(dvEntry Entry, dvCase value) {dvEntrys.FirstCase[dvEntry2ValidIndex(Entry)] = value;}
utInlineC dvCase dvEntryGetLastCase(dvEntry Entry) {return dvEntrys.LastCase[dvEntry2ValidIndex(Entry)];}
utInlineC void dvEntrySetLastCase(dvEntry Entry, dvCase value) {dvEntrys.LastCase[dvEntry2ValidIndex(Entry)] = value;}
utInlineC void dvEntrySetConstructorCallback(void(*func)(dvEntry)) {dvEntryConstructorCallback = func;}
utInlineC dvEntryCallbackType dvEntryGetConstructorCallback(void) {return dvEntryConstructorCallback;}
utInlineC dvEntry dvFirstEntry(void) {return dvRootData.usedEntry == 1? dvEntryNull : dvIndex2Entry(1);}
utInlineC dvEntry dvLastEntry(void) {return dvRootData.usedEntry == 1? dvEntryNull :
    dvIndex2Entry(dvRootData.usedEntry - 1);}
utInlineC dvEntry dvNextEntry(dvEntry Entry) {return dvEntry2ValidIndex(Entry) + 1 == dvRootData.usedEntry? dvEntryNull :
    Entry + 1;}
utInlineC dvEntry dvPrevEntry(dvEntry Entry) {return dvEntry2ValidIndex(Entry) == 1? dvEntryNull : Entry - 1;}
#define dvForeachEntry(var) \
    for(var = dvIndex2Entry(1); dvEntry2Index(var) != dvRootData.usedEntry; var++)
#define dvEndEntry
utInlineC void dvEntryFreeAll(void) {dvSetUsedEntry(1);}
utInlineC dvEntry dvEntryAllocRaw(void) {
    dvEntry Entry;
    if(dvRootData.usedEntry == dvRootData.allocatedEntry) {
        dvEntryAllocMore();
    }
    Entry = dvIndex2Entry(dvRootData.usedEntry);
    dvSetUsedEntry(dvUsedEntry() + 1);
    return Entry;}
utInlineC dvEntry dvEntryAlloc(void) {
    dvEntry Entry = dvEntryAllocRaw();
    dvEntrySetSym(Entry, utSymNull);
    dvEntrySetValue(Entry, 0);
    dvEntrySetEnum(Entry, dvEnumNull);
    dvEntrySetNextEnumEntry(Entry, dvEntryNull);
    dvEntrySetPrevEnumEntry(Entry, dvEntryNull);
    dvEntrySetNextTableEnumEntry(Entry, dvEntryNull);
    dvEntrySetFirstCase(Entry, dvCaseNull);
    dvEntrySetLastCase(Entry, dvCaseNull);
    if(dvEntryConstructorCallback != NULL) {
        dvEntryConstructorCallback(Entry);
    }
    return Entry;}

/*----------------------------------------------------------------------------------------
  Fields for class Typedef.
----------------------------------------------------------------------------------------*/
struct dvTypedefFields {
    utSym *Sym;
    uint32 *InitializerIndex_;
    uint32 *NumInitializer;
    char *Initializer;
    dvModule *Module;
    dvTypedef *NextModuleTypedef;
    dvTypedef *PrevModuleTypedef;
    dvTypedef *NextTableModuleTypedef;
};
extern struct dvTypedefFields dvTypedefs;

void dvTypedefAllocMore(void);
void dvTypedefCopyProps(dvTypedef dvOldTypedef, dvTypedef dvNewTypedef);
void dvTypedefAllocInitializers(dvTypedef Typedef, uint32 numInitializers);
void dvTypedefResizeInitializers(dvTypedef Typedef, uint32 numInitializers);
void dvTypedefFreeInitializers(dvTypedef Typedef);
void dvCompactTypedefInitializers(void);
utInlineC utSym dvTypedefGetSym(dvTypedef Typedef) {return dvTypedefs.Sym[dvTypedef2ValidIndex(Typedef)];}
utInlineC void dvTypedefSetSym(dvTypedef Typedef, utSym value) {dvTypedefs.Sym[dvTypedef2ValidIndex(Typedef)] = value;}
utInlineC uint32 dvTypedefGetInitializerIndex_(dvTypedef Typedef) {return dvTypedefs.InitializerIndex_[dvTypedef2ValidIndex(Typedef)];}
utInlineC void dvTypedefSetInitializerIndex_(dvTypedef Typedef, uint32 value) {dvTypedefs.InitializerIndex_[dvTypedef2ValidIndex(Typedef)] = value;}
utInlineC uint32 dvTypedefGetNumInitializer(dvTypedef Typedef) {return dvTypedefs.NumInitializer[dvTypedef2ValidIndex(Typedef)];}
utInlineC void dvTypedefSetNumInitializer(dvTypedef Typedef, uint32 value) {dvTypedefs.NumInitializer[dvTypedef2ValidIndex(Typedef)] = value;}
#if defined(DD_DEBUG)
utInlineC uint32 dvTypedefCheckInitializerIndex(dvTypedef Typedef, uint32 x) {utAssert(x < dvTypedefGetNumInitializer(Typedef)); return x;}
#else
utInlineC uint32 dvTypedefCheckInitializerIndex(dvTypedef Typedef, uint32 x) {return x;}
#endif
utInlineC char dvTypedefGetiInitializer(dvTypedef Typedef, uint32 x) {return dvTypedefs.Initializer[
    dvTypedefGetInitializerIndex_(Typedef) + dvTypedefCheckInitializerIndex(Typedef, x)];}
utInlineC char *dvTypedefGetInitializer(dvTypedef Typedef) {return dvTypedefs.Initializer + dvTypedefGetInitializerIndex_(Typedef);}
#define dvTypedefGetInitializers dvTypedefGetInitializer
utInlineC void dvTypedefSetInitializer(dvTypedef Typedef, char *valuePtr, uint32 numInitializer) {
    dvTypedefResizeInitializers(Typedef, numInitializer);
    memcpy(dvTypedefGetInitializers(Typedef), valuePtr, numInitializer*sizeof(char));}
utInlineC void dvTypedefSetiInitializer(dvTypedef Typedef, uint32 x, char value) {
    dvTypedefs.Initializer[dvTypedefGetInitializerIndex_(Typedef) + dvTypedefCheckInitializerIndex(Typedef, (x))] = value;}
utInlineC void dvTypedefMoveInitializers(dvTypedef Typedef, uint32 from, uint32 to, uint32 count) {
    utAssert((to+count) <= dvTypedefGetNumInitializer(Typedef));
    utAssert((from+count) <= dvTypedefGetNumInitializer(Typedef));
    memmove(dvTypedefGetInitializers(Typedef)+to,dvTypedefGetInitializers(Typedef)+from,((int32)count)*sizeof(char));
}
utInlineC void dvTypedefCopyInitializers(dvTypedef Typedef, uint32 x, char * values, uint32 count) {
    utAssert((x+count) <= dvTypedefGetNumInitializer(Typedef));
    memcpy(dvTypedefGetInitializers(Typedef)+x, values, count*sizeof(char));
}
utInlineC void dvTypedefAppendInitializers(dvTypedef Typedef, char * values, uint32 count) {
    uint32 num = dvTypedefGetNumInitializer(Typedef);
    dvTypedefResizeInitializers(Typedef, num+count);
    dvTypedefCopyInitializers(Typedef, num, values, count);
}
utInlineC void dvTypedefAppendInitializer(dvTypedef Typedef, char Initializer) {
    dvTypedefResizeInitializers(Typedef, dvTypedefGetNumInitializer(Typedef)+1);
    dvTypedefSetiInitializer(Typedef, dvTypedefGetNumInitializer(Typedef)-1, Initializer);
}
utInlineC void dvTypedefInsertInitializers(dvTypedef Typedef, uint32 x, char *Initializer, uint32 count) {
    utAssert(x <= dvTypedefGetNumInitializer(Typedef));
    if(x < dvTypedefGetNumInitializer(Typedef)) {
        dvTypedefResizeInitializers(Typedef, dvTypedefGetNumInitializer(Typedef)+count);
        dvTypedefMoveInitializers(Typedef, x, x+count, dvTypedefGetNumInitializer(Typedef)-x-count);
        dvTypedefCopyInitializers(Typedef, x, Initializer, count);
    }
    else {
        dvTypedefAppendInitializers(Typedef, Initializer, count);
    }
}
utInlineC void dvTypedefInsertInitializer(dvTypedef Typedef, uint32 x, char Initializer) {
    dvTypedefInsertInitializers(Typedef, x, &Initializer, 1);
}
utInlineC void dvTypedefRemoveInitializers(dvTypedef Typedef, uint32 x, uint32 count) {
    utAssert((x+count) <= dvTypedefGetNumInitializer(Typedef));
    if((x+count) < dvTypedefGetNumInitializer(Typedef)) {
        dvTypedefMoveInitializers(Typedef, x+count,x,dvTypedefGetNumInitializer(Typedef)-x-count);
    }
    dvTypedefResizeInitializers(Typedef, dvTypedefGetNumInitializer(Typedef)-(int32)count);
}
utInlineC void dvTypedefRemoveInitializer(dvTypedef Typedef, uint32 x) {
    dvTypedefRemoveInitializers(Typedef, x, 1);
}
utInlineC void dvTypedefSwapInitializer(dvTypedef Typedef, uint32 from, uint32 to) {
    utAssert(from <= dvTypedefGetNumInitializer(Typedef));
    utAssert(to <= dvTypedefGetNumInitializer(Typedef));
    char tmp = dvTypedefGetiInitializer(Typedef, from);
    dvTypedefSetiInitializer(Typedef, from, dvTypedefGetiInitializer(Typedef, to));
    dvTypedefSetiInitializer(Typedef, to, tmp);
}
utInlineC void dvTypedefSwapInitializers(dvTypedef Typedef, uint32 from, uint32 to, uint32 count) {
    utAssert((from+count) < dvTypedefGetNumInitializer(Typedef));
    utAssert((to+count) < dvTypedefGetNumInitializer(Typedef));
    char tmp[count];
    memcpy(tmp, dvTypedefGetInitializers(Typedef)+from, count*sizeof(char));
    memcpy(dvTypedefGetInitializers(Typedef)+from, dvTypedefGetInitializers(Typedef)+to, count*sizeof(char));
    memcpy(dvTypedefGetInitializers(Typedef)+to, tmp, count*sizeof(char));
}
#define dvForeachTypedefInitializer(pVar, cVar) { \
    uint32 _xInitializer; \
    for(_xInitializer = 0; _xInitializer < dvTypedefGetNumInitializer(pVar); _xInitializer++) { \
        cVar = dvTypedefGetiInitializer(pVar, _xInitializer);
#define dvEndTypedefInitializer }}
utInlineC dvModule dvTypedefGetModule(dvTypedef Typedef) {return dvTypedefs.Module[dvTypedef2ValidIndex(Typedef)];}
utInlineC void dvTypedefSetModule(dvTypedef Typedef, dvModule value) {dvTypedefs.Module[dvTypedef2ValidIndex(Typedef)] = value;}
utInlineC dvTypedef dvTypedefGetNextModuleTypedef(dvTypedef Typedef) {return dvTypedefs.NextModuleTypedef[dvTypedef2ValidIndex(Typedef)];}
utInlineC void dvTypedefSetNextModuleTypedef(dvTypedef Typedef, dvTypedef value) {dvTypedefs.NextModuleTypedef[dvTypedef2ValidIndex(Typedef)] = value;}
utInlineC dvTypedef dvTypedefGetPrevModuleTypedef(dvTypedef Typedef) {return dvTypedefs.PrevModuleTypedef[dvTypedef2ValidIndex(Typedef)];}
utInlineC void dvTypedefSetPrevModuleTypedef(dvTypedef Typedef, dvTypedef value) {dvTypedefs.PrevModuleTypedef[dvTypedef2ValidIndex(Typedef)] = value;}
utInlineC dvTypedef dvTypedefGetNextTableModuleTypedef(dvTypedef Typedef) {return dvTypedefs.NextTableModuleTypedef[dvTypedef2ValidIndex(Typedef)];}
utInlineC void dvTypedefSetNextTableModuleTypedef(dvTypedef Typedef, dvTypedef value) {dvTypedefs.NextTableModuleTypedef[dvTypedef2ValidIndex(Typedef)] = value;}
utInlineC void dvTypedefSetConstructorCallback(void(*func)(dvTypedef)) {dvTypedefConstructorCallback = func;}
utInlineC dvTypedefCallbackType dvTypedefGetConstructorCallback(void) {return dvTypedefConstructorCallback;}
utInlineC dvTypedef dvFirstTypedef(void) {return dvRootData.usedTypedef == 1? dvTypedefNull : dvIndex2Typedef(1);}
utInlineC dvTypedef dvLastTypedef(void) {return dvRootData.usedTypedef == 1? dvTypedefNull :
    dvIndex2Typedef(dvRootData.usedTypedef - 1);}
utInlineC dvTypedef dvNextTypedef(dvTypedef Typedef) {return dvTypedef2ValidIndex(Typedef) + 1 == dvRootData.usedTypedef? dvTypedefNull :
    Typedef + 1;}
utInlineC dvTypedef dvPrevTypedef(dvTypedef Typedef) {return dvTypedef2ValidIndex(Typedef) == 1? dvTypedefNull : Typedef - 1;}
#define dvForeachTypedef(var) \
    for(var = dvIndex2Typedef(1); dvTypedef2Index(var) != dvRootData.usedTypedef; var++)
#define dvEndTypedef
utInlineC void dvTypedefFreeAll(void) {dvSetUsedTypedef(1); dvSetUsedTypedefInitializer(0);}
utInlineC dvTypedef dvTypedefAllocRaw(void) {
    dvTypedef Typedef;
    if(dvRootData.usedTypedef == dvRootData.allocatedTypedef) {
        dvTypedefAllocMore();
    }
    Typedef = dvIndex2Typedef(dvRootData.usedTypedef);
    dvSetUsedTypedef(dvUsedTypedef() + 1);
    return Typedef;}
utInlineC dvTypedef dvTypedefAlloc(void) {
    dvTypedef Typedef = dvTypedefAllocRaw();
    dvTypedefSetSym(Typedef, utSymNull);
    dvTypedefSetInitializerIndex_(Typedef, 0);
    dvTypedefSetNumInitializer(Typedef, 0);
    dvTypedefSetNumInitializer(Typedef, 0);
    dvTypedefSetModule(Typedef, dvModuleNull);
    dvTypedefSetNextModuleTypedef(Typedef, dvTypedefNull);
    dvTypedefSetPrevModuleTypedef(Typedef, dvTypedefNull);
    dvTypedefSetNextTableModuleTypedef(Typedef, dvTypedefNull);
    if(dvTypedefConstructorCallback != NULL) {
        dvTypedefConstructorCallback(Typedef);
    }
    return Typedef;}

/*----------------------------------------------------------------------------------------
  Fields for class Class.
----------------------------------------------------------------------------------------*/
struct dvClassFields {
    utSym *Sym;
    dvMemoryStyle *MemoryStyle;
    uint8 *ReferenceSize;
    uint8 *GenerateArrayClass;
    uint8 *GenerateAttributes;
    uint8 *Sparse;
    uint16 *NumFields;
    uint16 *Number;
    utSym *BaseClassSym;
    dvModule *Module;
    dvClass *NextModuleClass;
    dvClass *PrevModuleClass;
    dvClass *NextTableModuleClass;
    dvProperty *FirstProperty;
    dvProperty *LastProperty;
    uint32 *PropertyTableIndex_;
    uint32 *NumPropertyTable;
    dvProperty *PropertyTable;
    uint32 *NumProperty;
    dvProperty *FreeListProperty;
    dvSparsegroup *FirstSparsegroup;
    dvSparsegroup *LastSparsegroup;
    uint32 *SparsegroupTableIndex_;
    uint32 *NumSparsegroupTable;
    dvSparsegroup *SparsegroupTable;
    uint32 *NumSparsegroup;
    dvClass *BaseClass;
    dvClass *FirstDerivedClass;
    dvClass *NextClassDerivedClass;
    dvClass *LastDerivedClass;
    dvRelationship *FirstChildRelationship;
    dvRelationship *LastChildRelationship;
    dvRelationship *FirstParentRelationship;
    dvRelationship *LastParentRelationship;
    dvUnion *FirstUnion;
    dvUnion *LastUnion;
    dvCache *FirstCache;
    dvCache *LastCache;
};
extern struct dvClassFields dvClasss;

void dvClassAllocMore(void);
void dvClassCopyProps(dvClass dvOldClass, dvClass dvNewClass);
void dvClassSetBitfield(dvClass _Class, uint32 bitfield);
uint32 dvClassGetBitfield(dvClass _Class);
void dvClassAllocPropertyTables(dvClass Class, uint32 numPropertyTables);
void dvClassResizePropertyTables(dvClass Class, uint32 numPropertyTables);
void dvClassFreePropertyTables(dvClass Class);
void dvCompactClassPropertyTables(void);
void dvClassAllocSparsegroupTables(dvClass Class, uint32 numSparsegroupTables);
void dvClassResizeSparsegroupTables(dvClass Class, uint32 numSparsegroupTables);
void dvClassFreeSparsegroupTables(dvClass Class);
void dvCompactClassSparsegroupTables(void);
utInlineC utSym dvClassGetSym(dvClass Class) {return dvClasss.Sym[dvClass2ValidIndex(Class)];}
utInlineC void dvClassSetSym(dvClass Class, utSym value) {dvClasss.Sym[dvClass2ValidIndex(Class)] = value;}
utInlineC dvMemoryStyle dvClassGetMemoryStyle(dvClass Class) {return dvClasss.MemoryStyle[dvClass2ValidIndex(Class)];}
utInlineC void dvClassSetMemoryStyle(dvClass Class, dvMemoryStyle value) {dvClasss.MemoryStyle[dvClass2ValidIndex(Class)] = value;}
utInlineC uint8 dvClassGetReferenceSize(dvClass Class) {return dvClasss.ReferenceSize[dvClass2ValidIndex(Class)];}
utInlineC void dvClassSetReferenceSize(dvClass Class, uint8 value) {dvClasss.ReferenceSize[dvClass2ValidIndex(Class)] = value;}
utInlineC bool dvClassGenerateArrayClass(dvClass Class) {
    return (dvClasss.GenerateArrayClass[dvClass2ValidIndex(Class) >> 3] >> (dvClass2ValidIndex(Class) & 7)) & 1;}
utInlineC void dvClassSetGenerateArrayClass(dvClass Class, bool value) {
    dvClasss.GenerateArrayClass[dvClass2ValidIndex(Class) >> 3] = (uint8)((dvClasss.GenerateArrayClass[dvClass2ValidIndex(Class) >> 3] &
        ~(1 << (dvClass2ValidIndex(Class) & 7))) | ((value != 0) << (dvClass2ValidIndex(Class) & 7)));}
utInlineC bool dvClassGenerateAttributes(dvClass Class) {
    return (dvClasss.GenerateAttributes[dvClass2ValidIndex(Class) >> 3] >> (dvClass2ValidIndex(Class) & 7)) & 1;}
utInlineC void dvClassSetGenerateAttributes(dvClass Class, bool value) {
    dvClasss.GenerateAttributes[dvClass2ValidIndex(Class) >> 3] = (uint8)((dvClasss.GenerateAttributes[dvClass2ValidIndex(Class) >> 3] &
        ~(1 << (dvClass2ValidIndex(Class) & 7))) | ((value != 0) << (dvClass2ValidIndex(Class) & 7)));}
utInlineC bool dvClassSparse(dvClass Class) {
    return (dvClasss.Sparse[dvClass2ValidIndex(Class) >> 3] >> (dvClass2ValidIndex(Class) & 7)) & 1;}
utInlineC void dvClassSetSparse(dvClass Class, bool value) {
    dvClasss.Sparse[dvClass2ValidIndex(Class) >> 3] = (uint8)((dvClasss.Sparse[dvClass2ValidIndex(Class) >> 3] &
        ~(1 << (dvClass2ValidIndex(Class) & 7))) | ((value != 0) << (dvClass2ValidIndex(Class) & 7)));}
utInlineC uint16 dvClassGetNumFields(dvClass Class) {return dvClasss.NumFields[dvClass2ValidIndex(Class)];}
utInlineC void dvClassSetNumFields(dvClass Class, uint16 value) {dvClasss.NumFields[dvClass2ValidIndex(Class)] = value;}
utInlineC uint16 dvClassGetNumber(dvClass Class) {return dvClasss.Number[dvClass2ValidIndex(Class)];}
utInlineC void dvClassSetNumber(dvClass Class, uint16 value) {dvClasss.Number[dvClass2ValidIndex(Class)] = value;}
utInlineC utSym dvClassGetBaseClassSym(dvClass Class) {return dvClasss.BaseClassSym[dvClass2ValidIndex(Class)];}
utInlineC void dvClassSetBaseClassSym(dvClass Class, utSym value) {dvClasss.BaseClassSym[dvClass2ValidIndex(Class)] = value;}
utInlineC dvModule dvClassGetModule(dvClass Class) {return dvClasss.Module[dvClass2ValidIndex(Class)];}
utInlineC void dvClassSetModule(dvClass Class, dvModule value) {dvClasss.Module[dvClass2ValidIndex(Class)] = value;}
utInlineC dvClass dvClassGetNextModuleClass(dvClass Class) {return dvClasss.NextModuleClass[dvClass2ValidIndex(Class)];}
utInlineC void dvClassSetNextModuleClass(dvClass Class, dvClass value) {dvClasss.NextModuleClass[dvClass2ValidIndex(Class)] = value;}
utInlineC dvClass dvClassGetPrevModuleClass(dvClass Class) {return dvClasss.PrevModuleClass[dvClass2ValidIndex(Class)];}
utInlineC void dvClassSetPrevModuleClass(dvClass Class, dvClass value) {dvClasss.PrevModuleClass[dvClass2ValidIndex(Class)] = value;}
utInlineC dvClass dvClassGetNextTableModuleClass(dvClass Class) {return dvClasss.NextTableModuleClass[dvClass2ValidIndex(Class)];}
utInlineC void dvClassSetNextTableModuleClass(dvClass Class, dvClass value) {dvClasss.NextTableModuleClass[dvClass2ValidIndex(Class)] = value;}
utInlineC dvProperty dvClassGetFirstProperty(dvClass Class) {return dvClasss.FirstProperty[dvClass2ValidIndex(Class)];}
utInlineC void dvClassSetFirstProperty(dvClass Class, dvProperty value) {dvClasss.FirstProperty[dvClass2ValidIndex(Class)] = value;}
utInlineC dvProperty dvClassGetLastProperty(dvClass Class) {return dvClasss.LastProperty[dvClass2ValidIndex(Class)];}
utInlineC void dvClassSetLastProperty(dvClass Class, dvProperty value) {dvClasss.LastProperty[dvClass2ValidIndex(Class)] = value;}
utInlineC uint32 dvClassGetPropertyTableIndex_(dvClass Class) {return dvClasss.PropertyTableIndex_[dvClass2ValidIndex(Class)];}
utInlineC void dvClassSetPropertyTableIndex_(dvClass Class, uint32 value) {dvClasss.PropertyTableIndex_[dvClass2ValidIndex(Class)] = value;}
utInlineC uint32 dvClassGetNumPropertyTable(dvClass Class) {return dvClasss.NumPropertyTable[dvClass2ValidIndex(Class)];}
utInlineC void dvClassSetNumPropertyTable(dvClass Class, uint32 value) {dvClasss.NumPropertyTable[dvClass2ValidIndex(Class)] = value;}
#if defined(DD_DEBUG)
utInlineC uint32 dvClassCheckPropertyTableIndex(dvClass Class, uint32 x) {utAssert(x < dvClassGetNumPropertyTable(Class)); return x;}
#else
utInlineC uint32 dvClassCheckPropertyTableIndex(dvClass Class, uint32 x) {return x;}
#endif
utInlineC dvProperty dvClassGetiPropertyTable(dvClass Class, uint32 x) {return dvClasss.PropertyTable[
    dvClassGetPropertyTableIndex_(Class) + dvClassCheckPropertyTableIndex(Class, x)];}
utInlineC dvProperty *dvClassGetPropertyTable(dvClass Class) {return dvClasss.PropertyTable + dvClassGetPropertyTableIndex_(Class);}
#define dvClassGetPropertyTables dvClassGetPropertyTable
utInlineC void dvClassSetPropertyTable(dvClass Class, dvProperty *valuePtr, uint32 numPropertyTable) {
    dvClassResizePropertyTables(Class, numPropertyTable);
    memcpy(dvClassGetPropertyTables(Class), valuePtr, numPropertyTable*sizeof(dvProperty));}
utInlineC void dvClassSetiPropertyTable(dvClass Class, uint32 x, dvProperty value) {
    dvClasss.PropertyTable[dvClassGetPropertyTableIndex_(Class) + dvClassCheckPropertyTableIndex(Class, (x))] = value;}
utInlineC uint32 dvClassGetNumProperty(dvClass Class) {return dvClasss.NumProperty[dvClass2ValidIndex(Class)];}
utInlineC void dvClassSetNumProperty(dvClass Class, uint32 value) {dvClasss.NumProperty[dvClass2ValidIndex(Class)] = value;}
utInlineC dvProperty dvClassGetFreeListProperty(dvClass Class) {return dvClasss.FreeListProperty[dvClass2ValidIndex(Class)];}
utInlineC void dvClassSetFreeListProperty(dvClass Class, dvProperty value) {dvClasss.FreeListProperty[dvClass2ValidIndex(Class)] = value;}
utInlineC dvSparsegroup dvClassGetFirstSparsegroup(dvClass Class) {return dvClasss.FirstSparsegroup[dvClass2ValidIndex(Class)];}
utInlineC void dvClassSetFirstSparsegroup(dvClass Class, dvSparsegroup value) {dvClasss.FirstSparsegroup[dvClass2ValidIndex(Class)] = value;}
utInlineC dvSparsegroup dvClassGetLastSparsegroup(dvClass Class) {return dvClasss.LastSparsegroup[dvClass2ValidIndex(Class)];}
utInlineC void dvClassSetLastSparsegroup(dvClass Class, dvSparsegroup value) {dvClasss.LastSparsegroup[dvClass2ValidIndex(Class)] = value;}
utInlineC uint32 dvClassGetSparsegroupTableIndex_(dvClass Class) {return dvClasss.SparsegroupTableIndex_[dvClass2ValidIndex(Class)];}
utInlineC void dvClassSetSparsegroupTableIndex_(dvClass Class, uint32 value) {dvClasss.SparsegroupTableIndex_[dvClass2ValidIndex(Class)] = value;}
utInlineC uint32 dvClassGetNumSparsegroupTable(dvClass Class) {return dvClasss.NumSparsegroupTable[dvClass2ValidIndex(Class)];}
utInlineC void dvClassSetNumSparsegroupTable(dvClass Class, uint32 value) {dvClasss.NumSparsegroupTable[dvClass2ValidIndex(Class)] = value;}
#if defined(DD_DEBUG)
utInlineC uint32 dvClassCheckSparsegroupTableIndex(dvClass Class, uint32 x) {utAssert(x < dvClassGetNumSparsegroupTable(Class)); return x;}
#else
utInlineC uint32 dvClassCheckSparsegroupTableIndex(dvClass Class, uint32 x) {return x;}
#endif
utInlineC dvSparsegroup dvClassGetiSparsegroupTable(dvClass Class, uint32 x) {return dvClasss.SparsegroupTable[
    dvClassGetSparsegroupTableIndex_(Class) + dvClassCheckSparsegroupTableIndex(Class, x)];}
utInlineC dvSparsegroup *dvClassGetSparsegroupTable(dvClass Class) {return dvClasss.SparsegroupTable + dvClassGetSparsegroupTableIndex_(Class);}
#define dvClassGetSparsegroupTables dvClassGetSparsegroupTable
utInlineC void dvClassSetSparsegroupTable(dvClass Class, dvSparsegroup *valuePtr, uint32 numSparsegroupTable) {
    dvClassResizeSparsegroupTables(Class, numSparsegroupTable);
    memcpy(dvClassGetSparsegroupTables(Class), valuePtr, numSparsegroupTable*sizeof(dvSparsegroup));}
utInlineC void dvClassSetiSparsegroupTable(dvClass Class, uint32 x, dvSparsegroup value) {
    dvClasss.SparsegroupTable[dvClassGetSparsegroupTableIndex_(Class) + dvClassCheckSparsegroupTableIndex(Class, (x))] = value;}
utInlineC uint32 dvClassGetNumSparsegroup(dvClass Class) {return dvClasss.NumSparsegroup[dvClass2ValidIndex(Class)];}
utInlineC void dvClassSetNumSparsegroup(dvClass Class, uint32 value) {dvClasss.NumSparsegroup[dvClass2ValidIndex(Class)] = value;}
utInlineC dvClass dvClassGetBaseClass(dvClass Class) {return dvClasss.BaseClass[dvClass2ValidIndex(Class)];}
utInlineC void dvClassSetBaseClass(dvClass Class, dvClass value) {dvClasss.BaseClass[dvClass2ValidIndex(Class)] = value;}
utInlineC dvClass dvClassGetFirstDerivedClass(dvClass Class) {return dvClasss.FirstDerivedClass[dvClass2ValidIndex(Class)];}
utInlineC void dvClassSetFirstDerivedClass(dvClass Class, dvClass value) {dvClasss.FirstDerivedClass[dvClass2ValidIndex(Class)] = value;}
utInlineC dvClass dvClassGetNextClassDerivedClass(dvClass Class) {return dvClasss.NextClassDerivedClass[dvClass2ValidIndex(Class)];}
utInlineC void dvClassSetNextClassDerivedClass(dvClass Class, dvClass value) {dvClasss.NextClassDerivedClass[dvClass2ValidIndex(Class)] = value;}
utInlineC dvClass dvClassGetLastDerivedClass(dvClass Class) {return dvClasss.LastDerivedClass[dvClass2ValidIndex(Class)];}
utInlineC void dvClassSetLastDerivedClass(dvClass Class, dvClass value) {dvClasss.LastDerivedClass[dvClass2ValidIndex(Class)] = value;}
utInlineC dvRelationship dvClassGetFirstChildRelationship(dvClass Class) {return dvClasss.FirstChildRelationship[dvClass2ValidIndex(Class)];}
utInlineC void dvClassSetFirstChildRelationship(dvClass Class, dvRelationship value) {dvClasss.FirstChildRelationship[dvClass2ValidIndex(Class)] = value;}
utInlineC dvRelationship dvClassGetLastChildRelationship(dvClass Class) {return dvClasss.LastChildRelationship[dvClass2ValidIndex(Class)];}
utInlineC void dvClassSetLastChildRelationship(dvClass Class, dvRelationship value) {dvClasss.LastChildRelationship[dvClass2ValidIndex(Class)] = value;}
utInlineC dvRelationship dvClassGetFirstParentRelationship(dvClass Class) {return dvClasss.FirstParentRelationship[dvClass2ValidIndex(Class)];}
utInlineC void dvClassSetFirstParentRelationship(dvClass Class, dvRelationship value) {dvClasss.FirstParentRelationship[dvClass2ValidIndex(Class)] = value;}
utInlineC dvRelationship dvClassGetLastParentRelationship(dvClass Class) {return dvClasss.LastParentRelationship[dvClass2ValidIndex(Class)];}
utInlineC void dvClassSetLastParentRelationship(dvClass Class, dvRelationship value) {dvClasss.LastParentRelationship[dvClass2ValidIndex(Class)] = value;}
utInlineC dvUnion dvClassGetFirstUnion(dvClass Class) {return dvClasss.FirstUnion[dvClass2ValidIndex(Class)];}
utInlineC void dvClassSetFirstUnion(dvClass Class, dvUnion value) {dvClasss.FirstUnion[dvClass2ValidIndex(Class)] = value;}
utInlineC dvUnion dvClassGetLastUnion(dvClass Class) {return dvClasss.LastUnion[dvClass2ValidIndex(Class)];}
utInlineC void dvClassSetLastUnion(dvClass Class, dvUnion value) {dvClasss.LastUnion[dvClass2ValidIndex(Class)] = value;}
utInlineC dvCache dvClassGetFirstCache(dvClass Class) {return dvClasss.FirstCache[dvClass2ValidIndex(Class)];}
utInlineC void dvClassSetFirstCache(dvClass Class, dvCache value) {dvClasss.FirstCache[dvClass2ValidIndex(Class)] = value;}
utInlineC dvCache dvClassGetLastCache(dvClass Class) {return dvClasss.LastCache[dvClass2ValidIndex(Class)];}
utInlineC void dvClassSetLastCache(dvClass Class, dvCache value) {dvClasss.LastCache[dvClass2ValidIndex(Class)] = value;}
utInlineC void dvClassSetConstructorCallback(void(*func)(dvClass)) {dvClassConstructorCallback = func;}
utInlineC dvClassCallbackType dvClassGetConstructorCallback(void) {return dvClassConstructorCallback;}
utInlineC dvClass dvFirstClass(void) {return dvRootData.usedClass == 1? dvClassNull : dvIndex2Class(1);}
utInlineC dvClass dvLastClass(void) {return dvRootData.usedClass == 1? dvClassNull :
    dvIndex2Class(dvRootData.usedClass - 1);}
utInlineC dvClass dvNextClass(dvClass Class) {return dvClass2ValidIndex(Class) + 1 == dvRootData.usedClass? dvClassNull :
    Class + 1;}
utInlineC dvClass dvPrevClass(dvClass Class) {return dvClass2ValidIndex(Class) == 1? dvClassNull : Class - 1;}
#define dvForeachClass(var) \
    for(var = dvIndex2Class(1); dvClass2Index(var) != dvRootData.usedClass; var++)
#define dvEndClass
utInlineC void dvClassFreeAll(void) {dvSetUsedClass(1); dvSetUsedClassPropertyTable(0); dvSetUsedClassSparsegroupTable(0);}
utInlineC dvClass dvClassAllocRaw(void) {
    dvClass Class;
    if(dvRootData.usedClass == dvRootData.allocatedClass) {
        dvClassAllocMore();
    }
    Class = dvIndex2Class(dvRootData.usedClass);
    dvSetUsedClass(dvUsedClass() + 1);
    return Class;}
utInlineC dvClass dvClassAlloc(void) {
    dvClass Class = dvClassAllocRaw();
    dvClassSetSym(Class, utSymNull);
    dvClassSetMemoryStyle(Class, MEM_CREATE_ONLY);
    dvClassSetReferenceSize(Class, 0);
    dvClassSetGenerateArrayClass(Class, 0);
    dvClassSetGenerateAttributes(Class, 0);
    dvClassSetSparse(Class, 0);
    dvClassSetNumFields(Class, 0);
    dvClassSetNumber(Class, 0);
    dvClassSetBaseClassSym(Class, utSymNull);
    dvClassSetModule(Class, dvModuleNull);
    dvClassSetNextModuleClass(Class, dvClassNull);
    dvClassSetPrevModuleClass(Class, dvClassNull);
    dvClassSetNextTableModuleClass(Class, dvClassNull);
    dvClassSetFirstProperty(Class, dvPropertyNull);
    dvClassSetLastProperty(Class, dvPropertyNull);
    dvClassSetPropertyTableIndex_(Class, 0);
    dvClassSetNumPropertyTable(Class, 0);
    dvClassSetNumPropertyTable(Class, 0);
    dvClassSetNumProperty(Class, 0);
    dvClassSetFreeListProperty(Class, dvPropertyNull);
    dvClassSetFirstSparsegroup(Class, dvSparsegroupNull);
    dvClassSetLastSparsegroup(Class, dvSparsegroupNull);
    dvClassSetSparsegroupTableIndex_(Class, 0);
    dvClassSetNumSparsegroupTable(Class, 0);
    dvClassSetNumSparsegroupTable(Class, 0);
    dvClassSetNumSparsegroup(Class, 0);
    dvClassSetBaseClass(Class, dvClassNull);
    dvClassSetFirstDerivedClass(Class, dvClassNull);
    dvClassSetNextClassDerivedClass(Class, dvClassNull);
    dvClassSetLastDerivedClass(Class, dvClassNull);
    dvClassSetFirstChildRelationship(Class, dvRelationshipNull);
    dvClassSetLastChildRelationship(Class, dvRelationshipNull);
    dvClassSetFirstParentRelationship(Class, dvRelationshipNull);
    dvClassSetLastParentRelationship(Class, dvRelationshipNull);
    dvClassSetFirstUnion(Class, dvUnionNull);
    dvClassSetLastUnion(Class, dvUnionNull);
    dvClassSetFirstCache(Class, dvCacheNull);
    dvClassSetLastCache(Class, dvCacheNull);
    if(dvClassConstructorCallback != NULL) {
        dvClassConstructorCallback(Class);
    }
    return Class;}

/*----------------------------------------------------------------------------------------
  Unions for class Property.
----------------------------------------------------------------------------------------*/
typedef union {
    dvEnum EnumProp;
    dvTypedef TypedefProp;
    dvClass ClassProp;
    utSym TypeSym;
    uint8 Width;
} dvPropertyUnion1;

/*----------------------------------------------------------------------------------------
  Fields for class Property.
----------------------------------------------------------------------------------------*/
struct dvPropertyFields {
    utSym *Sym;
    dvPropertyType *Type;
    uint8 *Array;
    uint8 *Cascade;
    uint8 *Sparse;
    uint8 *View;
    uint8 *Expanded;
    uint32 *FieldNumber;
    dvProperty *FirstElementProp;
    dvProperty *NumElementsProp;
    uint8 *Hidden;
    uint32 *InitializerIndex_;
    uint32 *NumInitializer;
    char *Initializer;
    uint8 *FixedSize;
    uint32 *IndexIndex_;
    uint32 *NumIndex;
    char *Index;
    uint32 *Line;
    dvClass *Class;
    dvProperty *NextClassProperty;
    dvProperty *PrevClassProperty;
    dvProperty *NextTableClassProperty;
    dvCase *FirstCase;
    dvCase *LastCase;
    dvKeyproperty *FirstKeyproperty;
    dvKeyproperty *LastKeyproperty;
    dvSparsegroup *Sparsegroup;
    dvProperty *NextSparsegroupProperty;
    dvRelationship *Relationship;
    dvProperty *NextRelationshipProperty;
    dvUnion *Union;
    dvProperty *NextUnionProperty;
    dvCache *Cache;
    dvProperty *NextCacheProperty;
    dvPropertyUnion1 *union1;
};
extern struct dvPropertyFields dvPropertys;

void dvPropertyAllocMore(void);
void dvPropertyCopyProps(dvProperty dvOldProperty, dvProperty dvNewProperty);
void dvPropertySetBitfield(dvProperty _Property, uint32 bitfield);
uint32 dvPropertyGetBitfield(dvProperty _Property);
void dvPropertyAllocInitializers(dvProperty Property, uint32 numInitializers);
void dvPropertyResizeInitializers(dvProperty Property, uint32 numInitializers);
void dvPropertyFreeInitializers(dvProperty Property);
void dvCompactPropertyInitializers(void);
void dvPropertyAllocIndexs(dvProperty Property, uint32 numIndexs);
void dvPropertyResizeIndexs(dvProperty Property, uint32 numIndexs);
void dvPropertyFreeIndexs(dvProperty Property);
void dvCompactPropertyIndexs(void);
utInlineC utSym dvPropertyGetSym(dvProperty Property) {return dvPropertys.Sym[dvProperty2ValidIndex(Property)];}
utInlineC void dvPropertySetSym(dvProperty Property, utSym value) {dvPropertys.Sym[dvProperty2ValidIndex(Property)] = value;}
utInlineC dvPropertyType dvPropertyGetType(dvProperty Property) {return dvPropertys.Type[dvProperty2ValidIndex(Property)];}
utInlineC void dvPropertySetType(dvProperty Property, dvPropertyType value) {dvPropertys.Type[dvProperty2ValidIndex(Property)] = value;}
utInlineC bool dvPropertyArray(dvProperty Property) {
    return (dvPropertys.Array[dvProperty2ValidIndex(Property) >> 3] >> (dvProperty2ValidIndex(Property) & 7)) & 1;}
utInlineC void dvPropertySetArray(dvProperty Property, bool value) {
    dvPropertys.Array[dvProperty2ValidIndex(Property) >> 3] = (uint8)((dvPropertys.Array[dvProperty2ValidIndex(Property) >> 3] &
        ~(1 << (dvProperty2ValidIndex(Property) & 7))) | ((value != 0) << (dvProperty2ValidIndex(Property) & 7)));}
utInlineC bool dvPropertyCascade(dvProperty Property) {
    return (dvPropertys.Cascade[dvProperty2ValidIndex(Property) >> 3] >> (dvProperty2ValidIndex(Property) & 7)) & 1;}
utInlineC void dvPropertySetCascade(dvProperty Property, bool value) {
    dvPropertys.Cascade[dvProperty2ValidIndex(Property) >> 3] = (uint8)((dvPropertys.Cascade[dvProperty2ValidIndex(Property) >> 3] &
        ~(1 << (dvProperty2ValidIndex(Property) & 7))) | ((value != 0) << (dvProperty2ValidIndex(Property) & 7)));}
utInlineC bool dvPropertySparse(dvProperty Property) {
    return (dvPropertys.Sparse[dvProperty2ValidIndex(Property) >> 3] >> (dvProperty2ValidIndex(Property) & 7)) & 1;}
utInlineC void dvPropertySetSparse(dvProperty Property, bool value) {
    dvPropertys.Sparse[dvProperty2ValidIndex(Property) >> 3] = (uint8)((dvPropertys.Sparse[dvProperty2ValidIndex(Property) >> 3] &
        ~(1 << (dvProperty2ValidIndex(Property) & 7))) | ((value != 0) << (dvProperty2ValidIndex(Property) & 7)));}
utInlineC bool dvPropertyView(dvProperty Property) {
    return (dvPropertys.View[dvProperty2ValidIndex(Property) >> 3] >> (dvProperty2ValidIndex(Property) & 7)) & 1;}
utInlineC void dvPropertySetView(dvProperty Property, bool value) {
    dvPropertys.View[dvProperty2ValidIndex(Property) >> 3] = (uint8)((dvPropertys.View[dvProperty2ValidIndex(Property) >> 3] &
        ~(1 << (dvProperty2ValidIndex(Property) & 7))) | ((value != 0) << (dvProperty2ValidIndex(Property) & 7)));}
utInlineC bool dvPropertyExpanded(dvProperty Property) {
    return (dvPropertys.Expanded[dvProperty2ValidIndex(Property) >> 3] >> (dvProperty2ValidIndex(Property) & 7)) & 1;}
utInlineC void dvPropertySetExpanded(dvProperty Property, bool value) {
    dvPropertys.Expanded[dvProperty2ValidIndex(Property) >> 3] = (uint8)((dvPropertys.Expanded[dvProperty2ValidIndex(Property) >> 3] &
        ~(1 << (dvProperty2ValidIndex(Property) & 7))) | ((value != 0) << (dvProperty2ValidIndex(Property) & 7)));}
utInlineC uint32 dvPropertyGetFieldNumber(dvProperty Property) {return dvPropertys.FieldNumber[dvProperty2ValidIndex(Property)];}
utInlineC void dvPropertySetFieldNumber(dvProperty Property, uint32 value) {dvPropertys.FieldNumber[dvProperty2ValidIndex(Property)] = value;}
utInlineC dvProperty dvPropertyGetFirstElementProp(dvProperty Property) {return dvPropertys.FirstElementProp[dvProperty2ValidIndex(Property)];}
utInlineC void dvPropertySetFirstElementProp(dvProperty Property, dvProperty value) {dvPropertys.FirstElementProp[dvProperty2ValidIndex(Property)] = value;}
utInlineC dvProperty dvPropertyGetNumElementsProp(dvProperty Property) {return dvPropertys.NumElementsProp[dvProperty2ValidIndex(Property)];}
utInlineC void dvPropertySetNumElementsProp(dvProperty Property, dvProperty value) {dvPropertys.NumElementsProp[dvProperty2ValidIndex(Property)] = value;}
utInlineC bool dvPropertyHidden(dvProperty Property) {
    return (dvPropertys.Hidden[dvProperty2ValidIndex(Property) >> 3] >> (dvProperty2ValidIndex(Property) & 7)) & 1;}
utInlineC void dvPropertySetHidden(dvProperty Property, bool value) {
    dvPropertys.Hidden[dvProperty2ValidIndex(Property) >> 3] = (uint8)((dvPropertys.Hidden[dvProperty2ValidIndex(Property) >> 3] &
        ~(1 << (dvProperty2ValidIndex(Property) & 7))) | ((value != 0) << (dvProperty2ValidIndex(Property) & 7)));}
utInlineC uint32 dvPropertyGetInitializerIndex_(dvProperty Property) {return dvPropertys.InitializerIndex_[dvProperty2ValidIndex(Property)];}
utInlineC void dvPropertySetInitializerIndex_(dvProperty Property, uint32 value) {dvPropertys.InitializerIndex_[dvProperty2ValidIndex(Property)] = value;}
utInlineC uint32 dvPropertyGetNumInitializer(dvProperty Property) {return dvPropertys.NumInitializer[dvProperty2ValidIndex(Property)];}
utInlineC void dvPropertySetNumInitializer(dvProperty Property, uint32 value) {dvPropertys.NumInitializer[dvProperty2ValidIndex(Property)] = value;}
#if defined(DD_DEBUG)
utInlineC uint32 dvPropertyCheckInitializerIndex(dvProperty Property, uint32 x) {utAssert(x < dvPropertyGetNumInitializer(Property)); return x;}
#else
utInlineC uint32 dvPropertyCheckInitializerIndex(dvProperty Property, uint32 x) {return x;}
#endif
utInlineC char dvPropertyGetiInitializer(dvProperty Property, uint32 x) {return dvPropertys.Initializer[
    dvPropertyGetInitializerIndex_(Property) + dvPropertyCheckInitializerIndex(Property, x)];}
utInlineC char *dvPropertyGetInitializer(dvProperty Property) {return dvPropertys.Initializer + dvPropertyGetInitializerIndex_(Property);}
#define dvPropertyGetInitializers dvPropertyGetInitializer
utInlineC void dvPropertySetInitializer(dvProperty Property, char *valuePtr, uint32 numInitializer) {
    dvPropertyResizeInitializers(Property, numInitializer);
    memcpy(dvPropertyGetInitializers(Property), valuePtr, numInitializer*sizeof(char));}
utInlineC void dvPropertySetiInitializer(dvProperty Property, uint32 x, char value) {
    dvPropertys.Initializer[dvPropertyGetInitializerIndex_(Property) + dvPropertyCheckInitializerIndex(Property, (x))] = value;}
utInlineC void dvPropertyMoveInitializers(dvProperty Property, uint32 from, uint32 to, uint32 count) {
    utAssert((to+count) <= dvPropertyGetNumInitializer(Property));
    utAssert((from+count) <= dvPropertyGetNumInitializer(Property));
    memmove(dvPropertyGetInitializers(Property)+to,dvPropertyGetInitializers(Property)+from,((int32)count)*sizeof(char));
}
utInlineC void dvPropertyCopyInitializers(dvProperty Property, uint32 x, char * values, uint32 count) {
    utAssert((x+count) <= dvPropertyGetNumInitializer(Property));
    memcpy(dvPropertyGetInitializers(Property)+x, values, count*sizeof(char));
}
utInlineC void dvPropertyAppendInitializers(dvProperty Property, char * values, uint32 count) {
    uint32 num = dvPropertyGetNumInitializer(Property);
    dvPropertyResizeInitializers(Property, num+count);
    dvPropertyCopyInitializers(Property, num, values, count);
}
utInlineC void dvPropertyAppendInitializer(dvProperty Property, char Initializer) {
    dvPropertyResizeInitializers(Property, dvPropertyGetNumInitializer(Property)+1);
    dvPropertySetiInitializer(Property, dvPropertyGetNumInitializer(Property)-1, Initializer);
}
utInlineC void dvPropertyInsertInitializers(dvProperty Property, uint32 x, char *Initializer, uint32 count) {
    utAssert(x <= dvPropertyGetNumInitializer(Property));
    if(x < dvPropertyGetNumInitializer(Property)) {
        dvPropertyResizeInitializers(Property, dvPropertyGetNumInitializer(Property)+count);
        dvPropertyMoveInitializers(Property, x, x+count, dvPropertyGetNumInitializer(Property)-x-count);
        dvPropertyCopyInitializers(Property, x, Initializer, count);
    }
    else {
        dvPropertyAppendInitializers(Property, Initializer, count);
    }
}
utInlineC void dvPropertyInsertInitializer(dvProperty Property, uint32 x, char Initializer) {
    dvPropertyInsertInitializers(Property, x, &Initializer, 1);
}
utInlineC void dvPropertyRemoveInitializers(dvProperty Property, uint32 x, uint32 count) {
    utAssert((x+count) <= dvPropertyGetNumInitializer(Property));
    if((x+count) < dvPropertyGetNumInitializer(Property)) {
        dvPropertyMoveInitializers(Property, x+count,x,dvPropertyGetNumInitializer(Property)-x-count);
    }
    dvPropertyResizeInitializers(Property, dvPropertyGetNumInitializer(Property)-(int32)count);
}
utInlineC void dvPropertyRemoveInitializer(dvProperty Property, uint32 x) {
    dvPropertyRemoveInitializers(Property, x, 1);
}
utInlineC void dvPropertySwapInitializer(dvProperty Property, uint32 from, uint32 to) {
    utAssert(from <= dvPropertyGetNumInitializer(Property));
    utAssert(to <= dvPropertyGetNumInitializer(Property));
    char tmp = dvPropertyGetiInitializer(Property, from);
    dvPropertySetiInitializer(Property, from, dvPropertyGetiInitializer(Property, to));
    dvPropertySetiInitializer(Property, to, tmp);
}
utInlineC void dvPropertySwapInitializers(dvProperty Property, uint32 from, uint32 to, uint32 count) {
    utAssert((from+count) < dvPropertyGetNumInitializer(Property));
    utAssert((to+count) < dvPropertyGetNumInitializer(Property));
    char tmp[count];
    memcpy(tmp, dvPropertyGetInitializers(Property)+from, count*sizeof(char));
    memcpy(dvPropertyGetInitializers(Property)+from, dvPropertyGetInitializers(Property)+to, count*sizeof(char));
    memcpy(dvPropertyGetInitializers(Property)+to, tmp, count*sizeof(char));
}
#define dvForeachPropertyInitializer(pVar, cVar) { \
    uint32 _xInitializer; \
    for(_xInitializer = 0; _xInitializer < dvPropertyGetNumInitializer(pVar); _xInitializer++) { \
        cVar = dvPropertyGetiInitializer(pVar, _xInitializer);
#define dvEndPropertyInitializer }}
utInlineC bool dvPropertyFixedSize(dvProperty Property) {
    return (dvPropertys.FixedSize[dvProperty2ValidIndex(Property) >> 3] >> (dvProperty2ValidIndex(Property) & 7)) & 1;}
utInlineC void dvPropertySetFixedSize(dvProperty Property, bool value) {
    dvPropertys.FixedSize[dvProperty2ValidIndex(Property) >> 3] = (uint8)((dvPropertys.FixedSize[dvProperty2ValidIndex(Property) >> 3] &
        ~(1 << (dvProperty2ValidIndex(Property) & 7))) | ((value != 0) << (dvProperty2ValidIndex(Property) & 7)));}
utInlineC uint32 dvPropertyGetIndexIndex_(dvProperty Property) {return dvPropertys.IndexIndex_[dvProperty2ValidIndex(Property)];}
utInlineC void dvPropertySetIndexIndex_(dvProperty Property, uint32 value) {dvPropertys.IndexIndex_[dvProperty2ValidIndex(Property)] = value;}
utInlineC uint32 dvPropertyGetNumIndex(dvProperty Property) {return dvPropertys.NumIndex[dvProperty2ValidIndex(Property)];}
utInlineC void dvPropertySetNumIndex(dvProperty Property, uint32 value) {dvPropertys.NumIndex[dvProperty2ValidIndex(Property)] = value;}
#if defined(DD_DEBUG)
utInlineC uint32 dvPropertyCheckIndexIndex(dvProperty Property, uint32 x) {utAssert(x < dvPropertyGetNumIndex(Property)); return x;}
#else
utInlineC uint32 dvPropertyCheckIndexIndex(dvProperty Property, uint32 x) {return x;}
#endif
utInlineC char dvPropertyGetiIndex(dvProperty Property, uint32 x) {return dvPropertys.Index[
    dvPropertyGetIndexIndex_(Property) + dvPropertyCheckIndexIndex(Property, x)];}
utInlineC char *dvPropertyGetIndex(dvProperty Property) {return dvPropertys.Index + dvPropertyGetIndexIndex_(Property);}
#define dvPropertyGetIndexs dvPropertyGetIndex
utInlineC void dvPropertySetIndex(dvProperty Property, char *valuePtr, uint32 numIndex) {
    dvPropertyResizeIndexs(Property, numIndex);
    memcpy(dvPropertyGetIndexs(Property), valuePtr, numIndex*sizeof(char));}
utInlineC void dvPropertySetiIndex(dvProperty Property, uint32 x, char value) {
    dvPropertys.Index[dvPropertyGetIndexIndex_(Property) + dvPropertyCheckIndexIndex(Property, (x))] = value;}
utInlineC void dvPropertyMoveIndexs(dvProperty Property, uint32 from, uint32 to, uint32 count) {
    utAssert((to+count) <= dvPropertyGetNumIndex(Property));
    utAssert((from+count) <= dvPropertyGetNumIndex(Property));
    memmove(dvPropertyGetIndexs(Property)+to,dvPropertyGetIndexs(Property)+from,((int32)count)*sizeof(char));
}
utInlineC void dvPropertyCopyIndexs(dvProperty Property, uint32 x, char * values, uint32 count) {
    utAssert((x+count) <= dvPropertyGetNumIndex(Property));
    memcpy(dvPropertyGetIndexs(Property)+x, values, count*sizeof(char));
}
utInlineC void dvPropertyAppendIndexs(dvProperty Property, char * values, uint32 count) {
    uint32 num = dvPropertyGetNumIndex(Property);
    dvPropertyResizeIndexs(Property, num+count);
    dvPropertyCopyIndexs(Property, num, values, count);
}
utInlineC void dvPropertyAppendIndex(dvProperty Property, char Index) {
    dvPropertyResizeIndexs(Property, dvPropertyGetNumIndex(Property)+1);
    dvPropertySetiIndex(Property, dvPropertyGetNumIndex(Property)-1, Index);
}
utInlineC void dvPropertyInsertIndexs(dvProperty Property, uint32 x, char *Index, uint32 count) {
    utAssert(x <= dvPropertyGetNumIndex(Property));
    if(x < dvPropertyGetNumIndex(Property)) {
        dvPropertyResizeIndexs(Property, dvPropertyGetNumIndex(Property)+count);
        dvPropertyMoveIndexs(Property, x, x+count, dvPropertyGetNumIndex(Property)-x-count);
        dvPropertyCopyIndexs(Property, x, Index, count);
    }
    else {
        dvPropertyAppendIndexs(Property, Index, count);
    }
}
utInlineC void dvPropertyInsertIndex(dvProperty Property, uint32 x, char Index) {
    dvPropertyInsertIndexs(Property, x, &Index, 1);
}
utInlineC void dvPropertyRemoveIndexs(dvProperty Property, uint32 x, uint32 count) {
    utAssert((x+count) <= dvPropertyGetNumIndex(Property));
    if((x+count) < dvPropertyGetNumIndex(Property)) {
        dvPropertyMoveIndexs(Property, x+count,x,dvPropertyGetNumIndex(Property)-x-count);
    }
    dvPropertyResizeIndexs(Property, dvPropertyGetNumIndex(Property)-(int32)count);
}
utInlineC void dvPropertyRemoveIndex(dvProperty Property, uint32 x) {
    dvPropertyRemoveIndexs(Property, x, 1);
}
utInlineC void dvPropertySwapIndex(dvProperty Property, uint32 from, uint32 to) {
    utAssert(from <= dvPropertyGetNumIndex(Property));
    utAssert(to <= dvPropertyGetNumIndex(Property));
    char tmp = dvPropertyGetiIndex(Property, from);
    dvPropertySetiIndex(Property, from, dvPropertyGetiIndex(Property, to));
    dvPropertySetiIndex(Property, to, tmp);
}
utInlineC void dvPropertySwapIndexs(dvProperty Property, uint32 from, uint32 to, uint32 count) {
    utAssert((from+count) < dvPropertyGetNumIndex(Property));
    utAssert((to+count) < dvPropertyGetNumIndex(Property));
    char tmp[count];
    memcpy(tmp, dvPropertyGetIndexs(Property)+from, count*sizeof(char));
    memcpy(dvPropertyGetIndexs(Property)+from, dvPropertyGetIndexs(Property)+to, count*sizeof(char));
    memcpy(dvPropertyGetIndexs(Property)+to, tmp, count*sizeof(char));
}
#define dvForeachPropertyIndex(pVar, cVar) { \
    uint32 _xIndex; \
    for(_xIndex = 0; _xIndex < dvPropertyGetNumIndex(pVar); _xIndex++) { \
        cVar = dvPropertyGetiIndex(pVar, _xIndex);
#define dvEndPropertyIndex }}
utInlineC dvEnum dvPropertyGetEnumProp(dvProperty Property) {return dvPropertys.union1[dvProperty2ValidIndex(Property)].EnumProp;}
utInlineC void dvPropertySetEnumProp(dvProperty Property, dvEnum value) {
    dvPropertys.union1[dvProperty2ValidIndex(Property)].EnumProp = value;}
utInlineC dvTypedef dvPropertyGetTypedefProp(dvProperty Property) {return dvPropertys.union1[dvProperty2ValidIndex(Property)].TypedefProp;}
utInlineC void dvPropertySetTypedefProp(dvProperty Property, dvTypedef value) {
    dvPropertys.union1[dvProperty2ValidIndex(Property)].TypedefProp = value;}
utInlineC dvClass dvPropertyGetClassProp(dvProperty Property) {return dvPropertys.union1[dvProperty2ValidIndex(Property)].ClassProp;}
utInlineC void dvPropertySetClassProp(dvProperty Property, dvClass value) {
    dvPropertys.union1[dvProperty2ValidIndex(Property)].ClassProp = value;}
utInlineC utSym dvPropertyGetTypeSym(dvProperty Property) {return dvPropertys.union1[dvProperty2ValidIndex(Property)].TypeSym;}
utInlineC void dvPropertySetTypeSym(dvProperty Property, utSym value) {
    dvPropertys.union1[dvProperty2ValidIndex(Property)].TypeSym = value;}
utInlineC uint8 dvPropertyGetWidth(dvProperty Property) {return dvPropertys.union1[dvProperty2ValidIndex(Property)].Width;}
utInlineC void dvPropertySetWidth(dvProperty Property, uint8 value) {
    dvPropertys.union1[dvProperty2ValidIndex(Property)].Width = value;}
utInlineC uint32 dvPropertyGetLine(dvProperty Property) {return dvPropertys.Line[dvProperty2ValidIndex(Property)];}
utInlineC void dvPropertySetLine(dvProperty Property, uint32 value) {dvPropertys.Line[dvProperty2ValidIndex(Property)] = value;}
utInlineC dvClass dvPropertyGetClass(dvProperty Property) {return dvPropertys.Class[dvProperty2ValidIndex(Property)];}
utInlineC void dvPropertySetClass(dvProperty Property, dvClass value) {dvPropertys.Class[dvProperty2ValidIndex(Property)] = value;}
utInlineC dvProperty dvPropertyGetNextClassProperty(dvProperty Property) {return dvPropertys.NextClassProperty[dvProperty2ValidIndex(Property)];}
utInlineC void dvPropertySetNextClassProperty(dvProperty Property, dvProperty value) {dvPropertys.NextClassProperty[dvProperty2ValidIndex(Property)] = value;}
utInlineC dvProperty dvPropertyGetPrevClassProperty(dvProperty Property) {return dvPropertys.PrevClassProperty[dvProperty2ValidIndex(Property)];}
utInlineC void dvPropertySetPrevClassProperty(dvProperty Property, dvProperty value) {dvPropertys.PrevClassProperty[dvProperty2ValidIndex(Property)] = value;}
utInlineC dvProperty dvPropertyGetNextTableClassProperty(dvProperty Property) {return dvPropertys.NextTableClassProperty[dvProperty2ValidIndex(Property)];}
utInlineC void dvPropertySetNextTableClassProperty(dvProperty Property, dvProperty value) {dvPropertys.NextTableClassProperty[dvProperty2ValidIndex(Property)] = value;}
utInlineC dvCase dvPropertyGetFirstCase(dvProperty Property) {return dvPropertys.FirstCase[dvProperty2ValidIndex(Property)];}
utInlineC void dvPropertySetFirstCase(dvProperty Property, dvCase value) {dvPropertys.FirstCase[dvProperty2ValidIndex(Property)] = value;}
utInlineC dvCase dvPropertyGetLastCase(dvProperty Property) {return dvPropertys.LastCase[dvProperty2ValidIndex(Property)];}
utInlineC void dvPropertySetLastCase(dvProperty Property, dvCase value) {dvPropertys.LastCase[dvProperty2ValidIndex(Property)] = value;}
utInlineC dvKeyproperty dvPropertyGetFirstKeyproperty(dvProperty Property) {return dvPropertys.FirstKeyproperty[dvProperty2ValidIndex(Property)];}
utInlineC void dvPropertySetFirstKeyproperty(dvProperty Property, dvKeyproperty value) {dvPropertys.FirstKeyproperty[dvProperty2ValidIndex(Property)] = value;}
utInlineC dvKeyproperty dvPropertyGetLastKeyproperty(dvProperty Property) {return dvPropertys.LastKeyproperty[dvProperty2ValidIndex(Property)];}
utInlineC void dvPropertySetLastKeyproperty(dvProperty Property, dvKeyproperty value) {dvPropertys.LastKeyproperty[dvProperty2ValidIndex(Property)] = value;}
utInlineC dvSparsegroup dvPropertyGetSparsegroup(dvProperty Property) {return dvPropertys.Sparsegroup[dvProperty2ValidIndex(Property)];}
utInlineC void dvPropertySetSparsegroup(dvProperty Property, dvSparsegroup value) {dvPropertys.Sparsegroup[dvProperty2ValidIndex(Property)] = value;}
utInlineC dvProperty dvPropertyGetNextSparsegroupProperty(dvProperty Property) {return dvPropertys.NextSparsegroupProperty[dvProperty2ValidIndex(Property)];}
utInlineC void dvPropertySetNextSparsegroupProperty(dvProperty Property, dvProperty value) {dvPropertys.NextSparsegroupProperty[dvProperty2ValidIndex(Property)] = value;}
utInlineC dvRelationship dvPropertyGetRelationship(dvProperty Property) {return dvPropertys.Relationship[dvProperty2ValidIndex(Property)];}
utInlineC void dvPropertySetRelationship(dvProperty Property, dvRelationship value) {dvPropertys.Relationship[dvProperty2ValidIndex(Property)] = value;}
utInlineC dvProperty dvPropertyGetNextRelationshipProperty(dvProperty Property) {return dvPropertys.NextRelationshipProperty[dvProperty2ValidIndex(Property)];}
utInlineC void dvPropertySetNextRelationshipProperty(dvProperty Property, dvProperty value) {dvPropertys.NextRelationshipProperty[dvProperty2ValidIndex(Property)] = value;}
utInlineC dvUnion dvPropertyGetUnion(dvProperty Property) {return dvPropertys.Union[dvProperty2ValidIndex(Property)];}
utInlineC void dvPropertySetUnion(dvProperty Property, dvUnion value) {dvPropertys.Union[dvProperty2ValidIndex(Property)] = value;}
utInlineC dvProperty dvPropertyGetNextUnionProperty(dvProperty Property) {return dvPropertys.NextUnionProperty[dvProperty2ValidIndex(Property)];}
utInlineC void dvPropertySetNextUnionProperty(dvProperty Property, dvProperty value) {dvPropertys.NextUnionProperty[dvProperty2ValidIndex(Property)] = value;}
utInlineC dvCache dvPropertyGetCache(dvProperty Property) {return dvPropertys.Cache[dvProperty2ValidIndex(Property)];}
utInlineC void dvPropertySetCache(dvProperty Property, dvCache value) {dvPropertys.Cache[dvProperty2ValidIndex(Property)] = value;}
utInlineC dvProperty dvPropertyGetNextCacheProperty(dvProperty Property) {return dvPropertys.NextCacheProperty[dvProperty2ValidIndex(Property)];}
utInlineC void dvPropertySetNextCacheProperty(dvProperty Property, dvProperty value) {dvPropertys.NextCacheProperty[dvProperty2ValidIndex(Property)] = value;}
utInlineC void dvPropertySetConstructorCallback(void(*func)(dvProperty)) {dvPropertyConstructorCallback = func;}
utInlineC dvPropertyCallbackType dvPropertyGetConstructorCallback(void) {return dvPropertyConstructorCallback;}
utInlineC dvProperty dvFirstProperty(void) {return dvRootData.usedProperty == 1? dvPropertyNull : dvIndex2Property(1);}
utInlineC dvProperty dvLastProperty(void) {return dvRootData.usedProperty == 1? dvPropertyNull :
    dvIndex2Property(dvRootData.usedProperty - 1);}
utInlineC dvProperty dvNextProperty(dvProperty Property) {return dvProperty2ValidIndex(Property) + 1 == dvRootData.usedProperty? dvPropertyNull :
    Property + 1;}
utInlineC dvProperty dvPrevProperty(dvProperty Property) {return dvProperty2ValidIndex(Property) == 1? dvPropertyNull : Property - 1;}
#define dvForeachProperty(var) \
    for(var = dvIndex2Property(1); dvProperty2Index(var) != dvRootData.usedProperty; var++)
#define dvEndProperty
utInlineC void dvPropertyFreeAll(void) {dvSetUsedProperty(1); dvSetUsedPropertyInitializer(0); dvSetUsedPropertyIndex(0);}
utInlineC dvProperty dvPropertyAllocRaw(void) {
    dvProperty Property;
    if(dvRootData.usedProperty == dvRootData.allocatedProperty) {
        dvPropertyAllocMore();
    }
    Property = dvIndex2Property(dvRootData.usedProperty);
    dvSetUsedProperty(dvUsedProperty() + 1);
    return Property;}
utInlineC dvProperty dvPropertyAlloc(void) {
    dvProperty Property = dvPropertyAllocRaw();
    dvPropertySetSym(Property, utSymNull);
    dvPropertySetType(Property, PROP_INT);
    dvPropertySetArray(Property, 0);
    dvPropertySetCascade(Property, 0);
    dvPropertySetSparse(Property, 0);
    dvPropertySetView(Property, 0);
    dvPropertySetExpanded(Property, 0);
    dvPropertySetFieldNumber(Property, 0);
    dvPropertySetFirstElementProp(Property, dvPropertyNull);
    dvPropertySetNumElementsProp(Property, dvPropertyNull);
    dvPropertySetHidden(Property, 0);
    dvPropertySetInitializerIndex_(Property, 0);
    dvPropertySetNumInitializer(Property, 0);
    dvPropertySetNumInitializer(Property, 0);
    dvPropertySetFixedSize(Property, 0);
    dvPropertySetIndexIndex_(Property, 0);
    dvPropertySetNumIndex(Property, 0);
    dvPropertySetNumIndex(Property, 0);
    dvPropertySetEnumProp(Property, dvEnumNull);
    dvPropertySetLine(Property, 0);
    dvPropertySetClass(Property, dvClassNull);
    dvPropertySetNextClassProperty(Property, dvPropertyNull);
    dvPropertySetPrevClassProperty(Property, dvPropertyNull);
    dvPropertySetNextTableClassProperty(Property, dvPropertyNull);
    dvPropertySetFirstCase(Property, dvCaseNull);
    dvPropertySetLastCase(Property, dvCaseNull);
    dvPropertySetFirstKeyproperty(Property, dvKeypropertyNull);
    dvPropertySetLastKeyproperty(Property, dvKeypropertyNull);
    dvPropertySetSparsegroup(Property, dvSparsegroupNull);
    dvPropertySetNextSparsegroupProperty(Property, dvPropertyNull);
    dvPropertySetRelationship(Property, dvRelationshipNull);
    dvPropertySetNextRelationshipProperty(Property, dvPropertyNull);
    dvPropertySetUnion(Property, dvUnionNull);
    dvPropertySetNextUnionProperty(Property, dvPropertyNull);
    dvPropertySetCache(Property, dvCacheNull);
    dvPropertySetNextCacheProperty(Property, dvPropertyNull);
    if(dvPropertyConstructorCallback != NULL) {
        dvPropertyConstructorCallback(Property);
    }
    return Property;}

/*----------------------------------------------------------------------------------------
  Fields for class Sparsegroup.
----------------------------------------------------------------------------------------*/
struct dvSparsegroupFields {
    utSym *Sym;
    dvClass *Class;
    dvSparsegroup *NextClassSparsegroup;
    dvSparsegroup *PrevClassSparsegroup;
    dvSparsegroup *NextTableClassSparsegroup;
    dvProperty *FirstProperty;
    dvProperty *LastProperty;
    dvRelationship *Relationship;
};
extern struct dvSparsegroupFields dvSparsegroups;

void dvSparsegroupAllocMore(void);
void dvSparsegroupCopyProps(dvSparsegroup dvOldSparsegroup, dvSparsegroup dvNewSparsegroup);
utInlineC utSym dvSparsegroupGetSym(dvSparsegroup Sparsegroup) {return dvSparsegroups.Sym[dvSparsegroup2ValidIndex(Sparsegroup)];}
utInlineC void dvSparsegroupSetSym(dvSparsegroup Sparsegroup, utSym value) {dvSparsegroups.Sym[dvSparsegroup2ValidIndex(Sparsegroup)] = value;}
utInlineC dvClass dvSparsegroupGetClass(dvSparsegroup Sparsegroup) {return dvSparsegroups.Class[dvSparsegroup2ValidIndex(Sparsegroup)];}
utInlineC void dvSparsegroupSetClass(dvSparsegroup Sparsegroup, dvClass value) {dvSparsegroups.Class[dvSparsegroup2ValidIndex(Sparsegroup)] = value;}
utInlineC dvSparsegroup dvSparsegroupGetNextClassSparsegroup(dvSparsegroup Sparsegroup) {return dvSparsegroups.NextClassSparsegroup[dvSparsegroup2ValidIndex(Sparsegroup)];}
utInlineC void dvSparsegroupSetNextClassSparsegroup(dvSparsegroup Sparsegroup, dvSparsegroup value) {dvSparsegroups.NextClassSparsegroup[dvSparsegroup2ValidIndex(Sparsegroup)] = value;}
utInlineC dvSparsegroup dvSparsegroupGetPrevClassSparsegroup(dvSparsegroup Sparsegroup) {return dvSparsegroups.PrevClassSparsegroup[dvSparsegroup2ValidIndex(Sparsegroup)];}
utInlineC void dvSparsegroupSetPrevClassSparsegroup(dvSparsegroup Sparsegroup, dvSparsegroup value) {dvSparsegroups.PrevClassSparsegroup[dvSparsegroup2ValidIndex(Sparsegroup)] = value;}
utInlineC dvSparsegroup dvSparsegroupGetNextTableClassSparsegroup(dvSparsegroup Sparsegroup) {return dvSparsegroups.NextTableClassSparsegroup[dvSparsegroup2ValidIndex(Sparsegroup)];}
utInlineC void dvSparsegroupSetNextTableClassSparsegroup(dvSparsegroup Sparsegroup, dvSparsegroup value) {dvSparsegroups.NextTableClassSparsegroup[dvSparsegroup2ValidIndex(Sparsegroup)] = value;}
utInlineC dvProperty dvSparsegroupGetFirstProperty(dvSparsegroup Sparsegroup) {return dvSparsegroups.FirstProperty[dvSparsegroup2ValidIndex(Sparsegroup)];}
utInlineC void dvSparsegroupSetFirstProperty(dvSparsegroup Sparsegroup, dvProperty value) {dvSparsegroups.FirstProperty[dvSparsegroup2ValidIndex(Sparsegroup)] = value;}
utInlineC dvProperty dvSparsegroupGetLastProperty(dvSparsegroup Sparsegroup) {return dvSparsegroups.LastProperty[dvSparsegroup2ValidIndex(Sparsegroup)];}
utInlineC void dvSparsegroupSetLastProperty(dvSparsegroup Sparsegroup, dvProperty value) {dvSparsegroups.LastProperty[dvSparsegroup2ValidIndex(Sparsegroup)] = value;}
utInlineC dvRelationship dvSparsegroupGetRelationship(dvSparsegroup Sparsegroup) {return dvSparsegroups.Relationship[dvSparsegroup2ValidIndex(Sparsegroup)];}
utInlineC void dvSparsegroupSetRelationship(dvSparsegroup Sparsegroup, dvRelationship value) {dvSparsegroups.Relationship[dvSparsegroup2ValidIndex(Sparsegroup)] = value;}
utInlineC void dvSparsegroupSetConstructorCallback(void(*func)(dvSparsegroup)) {dvSparsegroupConstructorCallback = func;}
utInlineC dvSparsegroupCallbackType dvSparsegroupGetConstructorCallback(void) {return dvSparsegroupConstructorCallback;}
utInlineC void dvSparsegroupSetDestructorCallback(void(*func)(dvSparsegroup)) {dvSparsegroupDestructorCallback = func;}
utInlineC dvSparsegroupCallbackType dvSparsegroupGetDestructorCallback(void) {return dvSparsegroupDestructorCallback;}
utInlineC dvSparsegroup dvSparsegroupNextFree(dvSparsegroup Sparsegroup) {return ((dvSparsegroup *)(void *)(dvSparsegroups.Sym))[dvSparsegroup2ValidIndex(Sparsegroup)];}
utInlineC void dvSparsegroupSetNextFree(dvSparsegroup Sparsegroup, dvSparsegroup value) {
    ((dvSparsegroup *)(void *)(dvSparsegroups.Sym))[dvSparsegroup2ValidIndex(Sparsegroup)] = value;}
utInlineC void dvSparsegroupFree(dvSparsegroup Sparsegroup) {
    dvSparsegroupSetNextFree(Sparsegroup, dvRootData.firstFreeSparsegroup);
    dvSetFirstFreeSparsegroup(Sparsegroup);}
void dvSparsegroupDestroy(dvSparsegroup Sparsegroup);
utInlineC dvSparsegroup dvSparsegroupAllocRaw(void) {
    dvSparsegroup Sparsegroup;
    if(dvRootData.firstFreeSparsegroup != dvSparsegroupNull) {
        Sparsegroup = dvRootData.firstFreeSparsegroup;
        dvSetFirstFreeSparsegroup(dvSparsegroupNextFree(Sparsegroup));
    } else {
        if(dvRootData.usedSparsegroup == dvRootData.allocatedSparsegroup) {
            dvSparsegroupAllocMore();
        }
        Sparsegroup = dvIndex2Sparsegroup(dvRootData.usedSparsegroup);
        dvSetUsedSparsegroup(dvUsedSparsegroup() + 1);
    }
    return Sparsegroup;}
utInlineC dvSparsegroup dvSparsegroupAlloc(void) {
    dvSparsegroup Sparsegroup = dvSparsegroupAllocRaw();
    dvSparsegroupSetSym(Sparsegroup, utSymNull);
    dvSparsegroupSetClass(Sparsegroup, dvClassNull);
    dvSparsegroupSetNextClassSparsegroup(Sparsegroup, dvSparsegroupNull);
    dvSparsegroupSetPrevClassSparsegroup(Sparsegroup, dvSparsegroupNull);
    dvSparsegroupSetNextTableClassSparsegroup(Sparsegroup, dvSparsegroupNull);
    dvSparsegroupSetFirstProperty(Sparsegroup, dvPropertyNull);
    dvSparsegroupSetLastProperty(Sparsegroup, dvPropertyNull);
    dvSparsegroupSetRelationship(Sparsegroup, dvRelationshipNull);
    if(dvSparsegroupConstructorCallback != NULL) {
        dvSparsegroupConstructorCallback(Sparsegroup);
    }
    return Sparsegroup;}

/*----------------------------------------------------------------------------------------
  Fields for class Relationship.
----------------------------------------------------------------------------------------*/
struct dvRelationshipFields {
    dvRelationshipType *Type;
    utSym *ParentLabelSym;
    utSym *ChildLabelSym;
    uint8 *Mandatory;
    uint8 *Cascade;
    uint8 *AccessChild;
    uint8 *AccessParent;
    uint8 *SharedParent;
    uint8 *Sparse;
    uint8 *Expanded;
    uint8 *Unordered;
    dvSchema *Schema;
    dvRelationship *NextSchemaRelationship;
    dvClass *ParentClass;
    dvRelationship *NextClassChildRelationship;
    dvClass *ChildClass;
    dvRelationship *NextClassParentRelationship;
    dvProperty *FirstProperty;
    dvProperty *LastProperty;
    dvKey *FirstKey;
    dvKey *LastKey;
    dvSparsegroup *ParentSparsegroup;
    dvSparsegroup *ChildSparsegroup;
};
extern struct dvRelationshipFields dvRelationships;

void dvRelationshipAllocMore(void);
void dvRelationshipCopyProps(dvRelationship dvOldRelationship, dvRelationship dvNewRelationship);
void dvRelationshipSetBitfield(dvRelationship _Relationship, uint32 bitfield);
uint32 dvRelationshipGetBitfield(dvRelationship _Relationship);
utInlineC dvRelationshipType dvRelationshipGetType(dvRelationship Relationship) {return dvRelationships.Type[dvRelationship2ValidIndex(Relationship)];}
utInlineC void dvRelationshipSetType(dvRelationship Relationship, dvRelationshipType value) {dvRelationships.Type[dvRelationship2ValidIndex(Relationship)] = value;}
utInlineC utSym dvRelationshipGetParentLabelSym(dvRelationship Relationship) {return dvRelationships.ParentLabelSym[dvRelationship2ValidIndex(Relationship)];}
utInlineC void dvRelationshipSetParentLabelSym(dvRelationship Relationship, utSym value) {dvRelationships.ParentLabelSym[dvRelationship2ValidIndex(Relationship)] = value;}
utInlineC utSym dvRelationshipGetChildLabelSym(dvRelationship Relationship) {return dvRelationships.ChildLabelSym[dvRelationship2ValidIndex(Relationship)];}
utInlineC void dvRelationshipSetChildLabelSym(dvRelationship Relationship, utSym value) {dvRelationships.ChildLabelSym[dvRelationship2ValidIndex(Relationship)] = value;}
utInlineC bool dvRelationshipMandatory(dvRelationship Relationship) {
    return (dvRelationships.Mandatory[dvRelationship2ValidIndex(Relationship) >> 3] >> (dvRelationship2ValidIndex(Relationship) & 7)) & 1;}
utInlineC void dvRelationshipSetMandatory(dvRelationship Relationship, bool value) {
    dvRelationships.Mandatory[dvRelationship2ValidIndex(Relationship) >> 3] = (uint8)((dvRelationships.Mandatory[dvRelationship2ValidIndex(Relationship) >> 3] &
        ~(1 << (dvRelationship2ValidIndex(Relationship) & 7))) | ((value != 0) << (dvRelationship2ValidIndex(Relationship) & 7)));}
utInlineC bool dvRelationshipCascade(dvRelationship Relationship) {
    return (dvRelationships.Cascade[dvRelationship2ValidIndex(Relationship) >> 3] >> (dvRelationship2ValidIndex(Relationship) & 7)) & 1;}
utInlineC void dvRelationshipSetCascade(dvRelationship Relationship, bool value) {
    dvRelationships.Cascade[dvRelationship2ValidIndex(Relationship) >> 3] = (uint8)((dvRelationships.Cascade[dvRelationship2ValidIndex(Relationship) >> 3] &
        ~(1 << (dvRelationship2ValidIndex(Relationship) & 7))) | ((value != 0) << (dvRelationship2ValidIndex(Relationship) & 7)));}
utInlineC bool dvRelationshipAccessChild(dvRelationship Relationship) {
    return (dvRelationships.AccessChild[dvRelationship2ValidIndex(Relationship) >> 3] >> (dvRelationship2ValidIndex(Relationship) & 7)) & 1;}
utInlineC void dvRelationshipSetAccessChild(dvRelationship Relationship, bool value) {
    dvRelationships.AccessChild[dvRelationship2ValidIndex(Relationship) >> 3] = (uint8)((dvRelationships.AccessChild[dvRelationship2ValidIndex(Relationship) >> 3] &
        ~(1 << (dvRelationship2ValidIndex(Relationship) & 7))) | ((value != 0) << (dvRelationship2ValidIndex(Relationship) & 7)));}
utInlineC bool dvRelationshipAccessParent(dvRelationship Relationship) {
    return (dvRelationships.AccessParent[dvRelationship2ValidIndex(Relationship) >> 3] >> (dvRelationship2ValidIndex(Relationship) & 7)) & 1;}
utInlineC void dvRelationshipSetAccessParent(dvRelationship Relationship, bool value) {
    dvRelationships.AccessParent[dvRelationship2ValidIndex(Relationship) >> 3] = (uint8)((dvRelationships.AccessParent[dvRelationship2ValidIndex(Relationship) >> 3] &
        ~(1 << (dvRelationship2ValidIndex(Relationship) & 7))) | ((value != 0) << (dvRelationship2ValidIndex(Relationship) & 7)));}
utInlineC bool dvRelationshipSharedParent(dvRelationship Relationship) {
    return (dvRelationships.SharedParent[dvRelationship2ValidIndex(Relationship) >> 3] >> (dvRelationship2ValidIndex(Relationship) & 7)) & 1;}
utInlineC void dvRelationshipSetSharedParent(dvRelationship Relationship, bool value) {
    dvRelationships.SharedParent[dvRelationship2ValidIndex(Relationship) >> 3] = (uint8)((dvRelationships.SharedParent[dvRelationship2ValidIndex(Relationship) >> 3] &
        ~(1 << (dvRelationship2ValidIndex(Relationship) & 7))) | ((value != 0) << (dvRelationship2ValidIndex(Relationship) & 7)));}
utInlineC bool dvRelationshipSparse(dvRelationship Relationship) {
    return (dvRelationships.Sparse[dvRelationship2ValidIndex(Relationship) >> 3] >> (dvRelationship2ValidIndex(Relationship) & 7)) & 1;}
utInlineC void dvRelationshipSetSparse(dvRelationship Relationship, bool value) {
    dvRelationships.Sparse[dvRelationship2ValidIndex(Relationship) >> 3] = (uint8)((dvRelationships.Sparse[dvRelationship2ValidIndex(Relationship) >> 3] &
        ~(1 << (dvRelationship2ValidIndex(Relationship) & 7))) | ((value != 0) << (dvRelationship2ValidIndex(Relationship) & 7)));}
utInlineC bool dvRelationshipExpanded(dvRelationship Relationship) {
    return (dvRelationships.Expanded[dvRelationship2ValidIndex(Relationship) >> 3] >> (dvRelationship2ValidIndex(Relationship) & 7)) & 1;}
utInlineC void dvRelationshipSetExpanded(dvRelationship Relationship, bool value) {
    dvRelationships.Expanded[dvRelationship2ValidIndex(Relationship) >> 3] = (uint8)((dvRelationships.Expanded[dvRelationship2ValidIndex(Relationship) >> 3] &
        ~(1 << (dvRelationship2ValidIndex(Relationship) & 7))) | ((value != 0) << (dvRelationship2ValidIndex(Relationship) & 7)));}
utInlineC bool dvRelationshipUnordered(dvRelationship Relationship) {
    return (dvRelationships.Unordered[dvRelationship2ValidIndex(Relationship) >> 3] >> (dvRelationship2ValidIndex(Relationship) & 7)) & 1;}
utInlineC void dvRelationshipSetUnordered(dvRelationship Relationship, bool value) {
    dvRelationships.Unordered[dvRelationship2ValidIndex(Relationship) >> 3] = (uint8)((dvRelationships.Unordered[dvRelationship2ValidIndex(Relationship) >> 3] &
        ~(1 << (dvRelationship2ValidIndex(Relationship) & 7))) | ((value != 0) << (dvRelationship2ValidIndex(Relationship) & 7)));}
utInlineC dvSchema dvRelationshipGetSchema(dvRelationship Relationship) {return dvRelationships.Schema[dvRelationship2ValidIndex(Relationship)];}
utInlineC void dvRelationshipSetSchema(dvRelationship Relationship, dvSchema value) {dvRelationships.Schema[dvRelationship2ValidIndex(Relationship)] = value;}
utInlineC dvRelationship dvRelationshipGetNextSchemaRelationship(dvRelationship Relationship) {return dvRelationships.NextSchemaRelationship[dvRelationship2ValidIndex(Relationship)];}
utInlineC void dvRelationshipSetNextSchemaRelationship(dvRelationship Relationship, dvRelationship value) {dvRelationships.NextSchemaRelationship[dvRelationship2ValidIndex(Relationship)] = value;}
utInlineC dvClass dvRelationshipGetParentClass(dvRelationship Relationship) {return dvRelationships.ParentClass[dvRelationship2ValidIndex(Relationship)];}
utInlineC void dvRelationshipSetParentClass(dvRelationship Relationship, dvClass value) {dvRelationships.ParentClass[dvRelationship2ValidIndex(Relationship)] = value;}
utInlineC dvRelationship dvRelationshipGetNextClassChildRelationship(dvRelationship Relationship) {return dvRelationships.NextClassChildRelationship[dvRelationship2ValidIndex(Relationship)];}
utInlineC void dvRelationshipSetNextClassChildRelationship(dvRelationship Relationship, dvRelationship value) {dvRelationships.NextClassChildRelationship[dvRelationship2ValidIndex(Relationship)] = value;}
utInlineC dvClass dvRelationshipGetChildClass(dvRelationship Relationship) {return dvRelationships.ChildClass[dvRelationship2ValidIndex(Relationship)];}
utInlineC void dvRelationshipSetChildClass(dvRelationship Relationship, dvClass value) {dvRelationships.ChildClass[dvRelationship2ValidIndex(Relationship)] = value;}
utInlineC dvRelationship dvRelationshipGetNextClassParentRelationship(dvRelationship Relationship) {return dvRelationships.NextClassParentRelationship[dvRelationship2ValidIndex(Relationship)];}
utInlineC void dvRelationshipSetNextClassParentRelationship(dvRelationship Relationship, dvRelationship value) {dvRelationships.NextClassParentRelationship[dvRelationship2ValidIndex(Relationship)] = value;}
utInlineC dvProperty dvRelationshipGetFirstProperty(dvRelationship Relationship) {return dvRelationships.FirstProperty[dvRelationship2ValidIndex(Relationship)];}
utInlineC void dvRelationshipSetFirstProperty(dvRelationship Relationship, dvProperty value) {dvRelationships.FirstProperty[dvRelationship2ValidIndex(Relationship)] = value;}
utInlineC dvProperty dvRelationshipGetLastProperty(dvRelationship Relationship) {return dvRelationships.LastProperty[dvRelationship2ValidIndex(Relationship)];}
utInlineC void dvRelationshipSetLastProperty(dvRelationship Relationship, dvProperty value) {dvRelationships.LastProperty[dvRelationship2ValidIndex(Relationship)] = value;}
utInlineC dvKey dvRelationshipGetFirstKey(dvRelationship Relationship) {return dvRelationships.FirstKey[dvRelationship2ValidIndex(Relationship)];}
utInlineC void dvRelationshipSetFirstKey(dvRelationship Relationship, dvKey value) {dvRelationships.FirstKey[dvRelationship2ValidIndex(Relationship)] = value;}
utInlineC dvKey dvRelationshipGetLastKey(dvRelationship Relationship) {return dvRelationships.LastKey[dvRelationship2ValidIndex(Relationship)];}
utInlineC void dvRelationshipSetLastKey(dvRelationship Relationship, dvKey value) {dvRelationships.LastKey[dvRelationship2ValidIndex(Relationship)] = value;}
utInlineC dvSparsegroup dvRelationshipGetParentSparsegroup(dvRelationship Relationship) {return dvRelationships.ParentSparsegroup[dvRelationship2ValidIndex(Relationship)];}
utInlineC void dvRelationshipSetParentSparsegroup(dvRelationship Relationship, dvSparsegroup value) {dvRelationships.ParentSparsegroup[dvRelationship2ValidIndex(Relationship)] = value;}
utInlineC dvSparsegroup dvRelationshipGetChildSparsegroup(dvRelationship Relationship) {return dvRelationships.ChildSparsegroup[dvRelationship2ValidIndex(Relationship)];}
utInlineC void dvRelationshipSetChildSparsegroup(dvRelationship Relationship, dvSparsegroup value) {dvRelationships.ChildSparsegroup[dvRelationship2ValidIndex(Relationship)] = value;}
utInlineC void dvRelationshipSetConstructorCallback(void(*func)(dvRelationship)) {dvRelationshipConstructorCallback = func;}
utInlineC dvRelationshipCallbackType dvRelationshipGetConstructorCallback(void) {return dvRelationshipConstructorCallback;}
utInlineC dvRelationship dvFirstRelationship(void) {return dvRootData.usedRelationship == 1? dvRelationshipNull : dvIndex2Relationship(1);}
utInlineC dvRelationship dvLastRelationship(void) {return dvRootData.usedRelationship == 1? dvRelationshipNull :
    dvIndex2Relationship(dvRootData.usedRelationship - 1);}
utInlineC dvRelationship dvNextRelationship(dvRelationship Relationship) {return dvRelationship2ValidIndex(Relationship) + 1 == dvRootData.usedRelationship? dvRelationshipNull :
    Relationship + 1;}
utInlineC dvRelationship dvPrevRelationship(dvRelationship Relationship) {return dvRelationship2ValidIndex(Relationship) == 1? dvRelationshipNull : Relationship - 1;}
#define dvForeachRelationship(var) \
    for(var = dvIndex2Relationship(1); dvRelationship2Index(var) != dvRootData.usedRelationship; var++)
#define dvEndRelationship
utInlineC void dvRelationshipFreeAll(void) {dvSetUsedRelationship(1);}
utInlineC dvRelationship dvRelationshipAllocRaw(void) {
    dvRelationship Relationship;
    if(dvRootData.usedRelationship == dvRootData.allocatedRelationship) {
        dvRelationshipAllocMore();
    }
    Relationship = dvIndex2Relationship(dvRootData.usedRelationship);
    dvSetUsedRelationship(dvUsedRelationship() + 1);
    return Relationship;}
utInlineC dvRelationship dvRelationshipAlloc(void) {
    dvRelationship Relationship = dvRelationshipAllocRaw();
    dvRelationshipSetType(Relationship, REL_LINKED_LIST);
    dvRelationshipSetParentLabelSym(Relationship, utSymNull);
    dvRelationshipSetChildLabelSym(Relationship, utSymNull);
    dvRelationshipSetMandatory(Relationship, 0);
    dvRelationshipSetCascade(Relationship, 0);
    dvRelationshipSetAccessChild(Relationship, 0);
    dvRelationshipSetAccessParent(Relationship, 0);
    dvRelationshipSetSharedParent(Relationship, 0);
    dvRelationshipSetSparse(Relationship, 0);
    dvRelationshipSetExpanded(Relationship, 0);
    dvRelationshipSetUnordered(Relationship, 0);
    dvRelationshipSetSchema(Relationship, dvSchemaNull);
    dvRelationshipSetNextSchemaRelationship(Relationship, dvRelationshipNull);
    dvRelationshipSetParentClass(Relationship, dvClassNull);
    dvRelationshipSetNextClassChildRelationship(Relationship, dvRelationshipNull);
    dvRelationshipSetChildClass(Relationship, dvClassNull);
    dvRelationshipSetNextClassParentRelationship(Relationship, dvRelationshipNull);
    dvRelationshipSetFirstProperty(Relationship, dvPropertyNull);
    dvRelationshipSetLastProperty(Relationship, dvPropertyNull);
    dvRelationshipSetFirstKey(Relationship, dvKeyNull);
    dvRelationshipSetLastKey(Relationship, dvKeyNull);
    dvRelationshipSetParentSparsegroup(Relationship, dvSparsegroupNull);
    dvRelationshipSetChildSparsegroup(Relationship, dvSparsegroupNull);
    if(dvRelationshipConstructorCallback != NULL) {
        dvRelationshipConstructorCallback(Relationship);
    }
    return Relationship;}

/*----------------------------------------------------------------------------------------
  Fields for class Key.
----------------------------------------------------------------------------------------*/
struct dvKeyFields {
    uint32 *LineNum;
    dvRelationship *Relationship;
    dvKey *NextRelationshipKey;
    dvKeyproperty *FirstKeyproperty;
    dvKeyproperty *LastKeyproperty;
};
extern struct dvKeyFields dvKeys;

void dvKeyAllocMore(void);
void dvKeyCopyProps(dvKey dvOldKey, dvKey dvNewKey);
utInlineC uint32 dvKeyGetLineNum(dvKey Key) {return dvKeys.LineNum[dvKey2ValidIndex(Key)];}
utInlineC void dvKeySetLineNum(dvKey Key, uint32 value) {dvKeys.LineNum[dvKey2ValidIndex(Key)] = value;}
utInlineC dvRelationship dvKeyGetRelationship(dvKey Key) {return dvKeys.Relationship[dvKey2ValidIndex(Key)];}
utInlineC void dvKeySetRelationship(dvKey Key, dvRelationship value) {dvKeys.Relationship[dvKey2ValidIndex(Key)] = value;}
utInlineC dvKey dvKeyGetNextRelationshipKey(dvKey Key) {return dvKeys.NextRelationshipKey[dvKey2ValidIndex(Key)];}
utInlineC void dvKeySetNextRelationshipKey(dvKey Key, dvKey value) {dvKeys.NextRelationshipKey[dvKey2ValidIndex(Key)] = value;}
utInlineC dvKeyproperty dvKeyGetFirstKeyproperty(dvKey Key) {return dvKeys.FirstKeyproperty[dvKey2ValidIndex(Key)];}
utInlineC void dvKeySetFirstKeyproperty(dvKey Key, dvKeyproperty value) {dvKeys.FirstKeyproperty[dvKey2ValidIndex(Key)] = value;}
utInlineC dvKeyproperty dvKeyGetLastKeyproperty(dvKey Key) {return dvKeys.LastKeyproperty[dvKey2ValidIndex(Key)];}
utInlineC void dvKeySetLastKeyproperty(dvKey Key, dvKeyproperty value) {dvKeys.LastKeyproperty[dvKey2ValidIndex(Key)] = value;}
utInlineC void dvKeySetConstructorCallback(void(*func)(dvKey)) {dvKeyConstructorCallback = func;}
utInlineC dvKeyCallbackType dvKeyGetConstructorCallback(void) {return dvKeyConstructorCallback;}
utInlineC void dvKeySetDestructorCallback(void(*func)(dvKey)) {dvKeyDestructorCallback = func;}
utInlineC dvKeyCallbackType dvKeyGetDestructorCallback(void) {return dvKeyDestructorCallback;}
utInlineC dvKey dvKeyNextFree(dvKey Key) {return ((dvKey *)(void *)(dvKeys.Relationship))[dvKey2ValidIndex(Key)];}
utInlineC void dvKeySetNextFree(dvKey Key, dvKey value) {
    ((dvKey *)(void *)(dvKeys.Relationship))[dvKey2ValidIndex(Key)] = value;}
utInlineC void dvKeyFree(dvKey Key) {
    dvKeySetNextFree(Key, dvRootData.firstFreeKey);
    dvSetFirstFreeKey(Key);}
void dvKeyDestroy(dvKey Key);
utInlineC dvKey dvKeyAllocRaw(void) {
    dvKey Key;
    if(dvRootData.firstFreeKey != dvKeyNull) {
        Key = dvRootData.firstFreeKey;
        dvSetFirstFreeKey(dvKeyNextFree(Key));
    } else {
        if(dvRootData.usedKey == dvRootData.allocatedKey) {
            dvKeyAllocMore();
        }
        Key = dvIndex2Key(dvRootData.usedKey);
        dvSetUsedKey(dvUsedKey() + 1);
    }
    return Key;}
utInlineC dvKey dvKeyAlloc(void) {
    dvKey Key = dvKeyAllocRaw();
    dvKeySetLineNum(Key, 0);
    dvKeySetRelationship(Key, dvRelationshipNull);
    dvKeySetNextRelationshipKey(Key, dvKeyNull);
    dvKeySetFirstKeyproperty(Key, dvKeypropertyNull);
    dvKeySetLastKeyproperty(Key, dvKeypropertyNull);
    if(dvKeyConstructorCallback != NULL) {
        dvKeyConstructorCallback(Key);
    }
    return Key;}

/*----------------------------------------------------------------------------------------
  Fields for class Keyproperty.
----------------------------------------------------------------------------------------*/
struct dvKeypropertyFields {
    utSym *PropertySym;
    dvProperty *Property;
    dvKeyproperty *NextPropertyKeyproperty;
    dvKey *Key;
    dvKeyproperty *NextKeyKeyproperty;
};
extern struct dvKeypropertyFields dvKeypropertys;

void dvKeypropertyAllocMore(void);
void dvKeypropertyCopyProps(dvKeyproperty dvOldKeyproperty, dvKeyproperty dvNewKeyproperty);
utInlineC utSym dvKeypropertyGetPropertySym(dvKeyproperty Keyproperty) {return dvKeypropertys.PropertySym[dvKeyproperty2ValidIndex(Keyproperty)];}
utInlineC void dvKeypropertySetPropertySym(dvKeyproperty Keyproperty, utSym value) {dvKeypropertys.PropertySym[dvKeyproperty2ValidIndex(Keyproperty)] = value;}
utInlineC dvProperty dvKeypropertyGetProperty(dvKeyproperty Keyproperty) {return dvKeypropertys.Property[dvKeyproperty2ValidIndex(Keyproperty)];}
utInlineC void dvKeypropertySetProperty(dvKeyproperty Keyproperty, dvProperty value) {dvKeypropertys.Property[dvKeyproperty2ValidIndex(Keyproperty)] = value;}
utInlineC dvKeyproperty dvKeypropertyGetNextPropertyKeyproperty(dvKeyproperty Keyproperty) {return dvKeypropertys.NextPropertyKeyproperty[dvKeyproperty2ValidIndex(Keyproperty)];}
utInlineC void dvKeypropertySetNextPropertyKeyproperty(dvKeyproperty Keyproperty, dvKeyproperty value) {dvKeypropertys.NextPropertyKeyproperty[dvKeyproperty2ValidIndex(Keyproperty)] = value;}
utInlineC dvKey dvKeypropertyGetKey(dvKeyproperty Keyproperty) {return dvKeypropertys.Key[dvKeyproperty2ValidIndex(Keyproperty)];}
utInlineC void dvKeypropertySetKey(dvKeyproperty Keyproperty, dvKey value) {dvKeypropertys.Key[dvKeyproperty2ValidIndex(Keyproperty)] = value;}
utInlineC dvKeyproperty dvKeypropertyGetNextKeyKeyproperty(dvKeyproperty Keyproperty) {return dvKeypropertys.NextKeyKeyproperty[dvKeyproperty2ValidIndex(Keyproperty)];}
utInlineC void dvKeypropertySetNextKeyKeyproperty(dvKeyproperty Keyproperty, dvKeyproperty value) {dvKeypropertys.NextKeyKeyproperty[dvKeyproperty2ValidIndex(Keyproperty)] = value;}
utInlineC void dvKeypropertySetConstructorCallback(void(*func)(dvKeyproperty)) {dvKeypropertyConstructorCallback = func;}
utInlineC dvKeypropertyCallbackType dvKeypropertyGetConstructorCallback(void) {return dvKeypropertyConstructorCallback;}
utInlineC void dvKeypropertySetDestructorCallback(void(*func)(dvKeyproperty)) {dvKeypropertyDestructorCallback = func;}
utInlineC dvKeypropertyCallbackType dvKeypropertyGetDestructorCallback(void) {return dvKeypropertyDestructorCallback;}
utInlineC dvKeyproperty dvKeypropertyNextFree(dvKeyproperty Keyproperty) {return ((dvKeyproperty *)(void *)(dvKeypropertys.PropertySym))[dvKeyproperty2ValidIndex(Keyproperty)];}
utInlineC void dvKeypropertySetNextFree(dvKeyproperty Keyproperty, dvKeyproperty value) {
    ((dvKeyproperty *)(void *)(dvKeypropertys.PropertySym))[dvKeyproperty2ValidIndex(Keyproperty)] = value;}
utInlineC void dvKeypropertyFree(dvKeyproperty Keyproperty) {
    dvKeypropertySetNextFree(Keyproperty, dvRootData.firstFreeKeyproperty);
    dvSetFirstFreeKeyproperty(Keyproperty);}
void dvKeypropertyDestroy(dvKeyproperty Keyproperty);
utInlineC dvKeyproperty dvKeypropertyAllocRaw(void) {
    dvKeyproperty Keyproperty;
    if(dvRootData.firstFreeKeyproperty != dvKeypropertyNull) {
        Keyproperty = dvRootData.firstFreeKeyproperty;
        dvSetFirstFreeKeyproperty(dvKeypropertyNextFree(Keyproperty));
    } else {
        if(dvRootData.usedKeyproperty == dvRootData.allocatedKeyproperty) {
            dvKeypropertyAllocMore();
        }
        Keyproperty = dvIndex2Keyproperty(dvRootData.usedKeyproperty);
        dvSetUsedKeyproperty(dvUsedKeyproperty() + 1);
    }
    return Keyproperty;}
utInlineC dvKeyproperty dvKeypropertyAlloc(void) {
    dvKeyproperty Keyproperty = dvKeypropertyAllocRaw();
    dvKeypropertySetPropertySym(Keyproperty, utSymNull);
    dvKeypropertySetProperty(Keyproperty, dvPropertyNull);
    dvKeypropertySetNextPropertyKeyproperty(Keyproperty, dvKeypropertyNull);
    dvKeypropertySetKey(Keyproperty, dvKeyNull);
    dvKeypropertySetNextKeyKeyproperty(Keyproperty, dvKeypropertyNull);
    if(dvKeypropertyConstructorCallback != NULL) {
        dvKeypropertyConstructorCallback(Keyproperty);
    }
    return Keyproperty;}

/*----------------------------------------------------------------------------------------
  Fields for class Union.
----------------------------------------------------------------------------------------*/
struct dvUnionFields {
    utSym *PropertySym;
    dvProperty *TypeProperty;
    uint32 *Line;
    uint16 *Number;
    uint32 *FieldNumber;
    uint16 *NumCases;
    dvClass *Class;
    dvUnion *NextClassUnion;
    dvProperty *FirstProperty;
    dvProperty *LastProperty;
};
extern struct dvUnionFields dvUnions;

void dvUnionAllocMore(void);
void dvUnionCopyProps(dvUnion dvOldUnion, dvUnion dvNewUnion);
utInlineC utSym dvUnionGetPropertySym(dvUnion Union) {return dvUnions.PropertySym[dvUnion2ValidIndex(Union)];}
utInlineC void dvUnionSetPropertySym(dvUnion Union, utSym value) {dvUnions.PropertySym[dvUnion2ValidIndex(Union)] = value;}
utInlineC dvProperty dvUnionGetTypeProperty(dvUnion Union) {return dvUnions.TypeProperty[dvUnion2ValidIndex(Union)];}
utInlineC void dvUnionSetTypeProperty(dvUnion Union, dvProperty value) {dvUnions.TypeProperty[dvUnion2ValidIndex(Union)] = value;}
utInlineC uint32 dvUnionGetLine(dvUnion Union) {return dvUnions.Line[dvUnion2ValidIndex(Union)];}
utInlineC void dvUnionSetLine(dvUnion Union, uint32 value) {dvUnions.Line[dvUnion2ValidIndex(Union)] = value;}
utInlineC uint16 dvUnionGetNumber(dvUnion Union) {return dvUnions.Number[dvUnion2ValidIndex(Union)];}
utInlineC void dvUnionSetNumber(dvUnion Union, uint16 value) {dvUnions.Number[dvUnion2ValidIndex(Union)] = value;}
utInlineC uint32 dvUnionGetFieldNumber(dvUnion Union) {return dvUnions.FieldNumber[dvUnion2ValidIndex(Union)];}
utInlineC void dvUnionSetFieldNumber(dvUnion Union, uint32 value) {dvUnions.FieldNumber[dvUnion2ValidIndex(Union)] = value;}
utInlineC uint16 dvUnionGetNumCases(dvUnion Union) {return dvUnions.NumCases[dvUnion2ValidIndex(Union)];}
utInlineC void dvUnionSetNumCases(dvUnion Union, uint16 value) {dvUnions.NumCases[dvUnion2ValidIndex(Union)] = value;}
utInlineC dvClass dvUnionGetClass(dvUnion Union) {return dvUnions.Class[dvUnion2ValidIndex(Union)];}
utInlineC void dvUnionSetClass(dvUnion Union, dvClass value) {dvUnions.Class[dvUnion2ValidIndex(Union)] = value;}
utInlineC dvUnion dvUnionGetNextClassUnion(dvUnion Union) {return dvUnions.NextClassUnion[dvUnion2ValidIndex(Union)];}
utInlineC void dvUnionSetNextClassUnion(dvUnion Union, dvUnion value) {dvUnions.NextClassUnion[dvUnion2ValidIndex(Union)] = value;}
utInlineC dvProperty dvUnionGetFirstProperty(dvUnion Union) {return dvUnions.FirstProperty[dvUnion2ValidIndex(Union)];}
utInlineC void dvUnionSetFirstProperty(dvUnion Union, dvProperty value) {dvUnions.FirstProperty[dvUnion2ValidIndex(Union)] = value;}
utInlineC dvProperty dvUnionGetLastProperty(dvUnion Union) {return dvUnions.LastProperty[dvUnion2ValidIndex(Union)];}
utInlineC void dvUnionSetLastProperty(dvUnion Union, dvProperty value) {dvUnions.LastProperty[dvUnion2ValidIndex(Union)] = value;}
utInlineC void dvUnionSetConstructorCallback(void(*func)(dvUnion)) {dvUnionConstructorCallback = func;}
utInlineC dvUnionCallbackType dvUnionGetConstructorCallback(void) {return dvUnionConstructorCallback;}
utInlineC void dvUnionSetDestructorCallback(void(*func)(dvUnion)) {dvUnionDestructorCallback = func;}
utInlineC dvUnionCallbackType dvUnionGetDestructorCallback(void) {return dvUnionDestructorCallback;}
utInlineC dvUnion dvUnionNextFree(dvUnion Union) {return ((dvUnion *)(void *)(dvUnions.PropertySym))[dvUnion2ValidIndex(Union)];}
utInlineC void dvUnionSetNextFree(dvUnion Union, dvUnion value) {
    ((dvUnion *)(void *)(dvUnions.PropertySym))[dvUnion2ValidIndex(Union)] = value;}
utInlineC void dvUnionFree(dvUnion Union) {
    dvUnionSetNextFree(Union, dvRootData.firstFreeUnion);
    dvSetFirstFreeUnion(Union);}
void dvUnionDestroy(dvUnion Union);
utInlineC dvUnion dvUnionAllocRaw(void) {
    dvUnion Union;
    if(dvRootData.firstFreeUnion != dvUnionNull) {
        Union = dvRootData.firstFreeUnion;
        dvSetFirstFreeUnion(dvUnionNextFree(Union));
    } else {
        if(dvRootData.usedUnion == dvRootData.allocatedUnion) {
            dvUnionAllocMore();
        }
        Union = dvIndex2Union(dvRootData.usedUnion);
        dvSetUsedUnion(dvUsedUnion() + 1);
    }
    return Union;}
utInlineC dvUnion dvUnionAlloc(void) {
    dvUnion Union = dvUnionAllocRaw();
    dvUnionSetPropertySym(Union, utSymNull);
    dvUnionSetTypeProperty(Union, dvPropertyNull);
    dvUnionSetLine(Union, 0);
    dvUnionSetNumber(Union, 0);
    dvUnionSetFieldNumber(Union, 0);
    dvUnionSetNumCases(Union, 0);
    dvUnionSetClass(Union, dvClassNull);
    dvUnionSetNextClassUnion(Union, dvUnionNull);
    dvUnionSetFirstProperty(Union, dvPropertyNull);
    dvUnionSetLastProperty(Union, dvPropertyNull);
    if(dvUnionConstructorCallback != NULL) {
        dvUnionConstructorCallback(Union);
    }
    return Union;}

/*----------------------------------------------------------------------------------------
  Fields for class Case.
----------------------------------------------------------------------------------------*/
struct dvCaseFields {
    utSym *EntrySym;
    dvEntry *Entry;
    dvCase *NextEntryCase;
    dvProperty *Property;
    dvCase *NextPropertyCase;
};
extern struct dvCaseFields dvCases;

void dvCaseAllocMore(void);
void dvCaseCopyProps(dvCase dvOldCase, dvCase dvNewCase);
utInlineC utSym dvCaseGetEntrySym(dvCase Case) {return dvCases.EntrySym[dvCase2ValidIndex(Case)];}
utInlineC void dvCaseSetEntrySym(dvCase Case, utSym value) {dvCases.EntrySym[dvCase2ValidIndex(Case)] = value;}
utInlineC dvEntry dvCaseGetEntry(dvCase Case) {return dvCases.Entry[dvCase2ValidIndex(Case)];}
utInlineC void dvCaseSetEntry(dvCase Case, dvEntry value) {dvCases.Entry[dvCase2ValidIndex(Case)] = value;}
utInlineC dvCase dvCaseGetNextEntryCase(dvCase Case) {return dvCases.NextEntryCase[dvCase2ValidIndex(Case)];}
utInlineC void dvCaseSetNextEntryCase(dvCase Case, dvCase value) {dvCases.NextEntryCase[dvCase2ValidIndex(Case)] = value;}
utInlineC dvProperty dvCaseGetProperty(dvCase Case) {return dvCases.Property[dvCase2ValidIndex(Case)];}
utInlineC void dvCaseSetProperty(dvCase Case, dvProperty value) {dvCases.Property[dvCase2ValidIndex(Case)] = value;}
utInlineC dvCase dvCaseGetNextPropertyCase(dvCase Case) {return dvCases.NextPropertyCase[dvCase2ValidIndex(Case)];}
utInlineC void dvCaseSetNextPropertyCase(dvCase Case, dvCase value) {dvCases.NextPropertyCase[dvCase2ValidIndex(Case)] = value;}
utInlineC void dvCaseSetConstructorCallback(void(*func)(dvCase)) {dvCaseConstructorCallback = func;}
utInlineC dvCaseCallbackType dvCaseGetConstructorCallback(void) {return dvCaseConstructorCallback;}
utInlineC void dvCaseSetDestructorCallback(void(*func)(dvCase)) {dvCaseDestructorCallback = func;}
utInlineC dvCaseCallbackType dvCaseGetDestructorCallback(void) {return dvCaseDestructorCallback;}
utInlineC dvCase dvCaseNextFree(dvCase Case) {return ((dvCase *)(void *)(dvCases.EntrySym))[dvCase2ValidIndex(Case)];}
utInlineC void dvCaseSetNextFree(dvCase Case, dvCase value) {
    ((dvCase *)(void *)(dvCases.EntrySym))[dvCase2ValidIndex(Case)] = value;}
utInlineC void dvCaseFree(dvCase Case) {
    dvCaseSetNextFree(Case, dvRootData.firstFreeCase);
    dvSetFirstFreeCase(Case);}
void dvCaseDestroy(dvCase Case);
utInlineC dvCase dvCaseAllocRaw(void) {
    dvCase Case;
    if(dvRootData.firstFreeCase != dvCaseNull) {
        Case = dvRootData.firstFreeCase;
        dvSetFirstFreeCase(dvCaseNextFree(Case));
    } else {
        if(dvRootData.usedCase == dvRootData.allocatedCase) {
            dvCaseAllocMore();
        }
        Case = dvIndex2Case(dvRootData.usedCase);
        dvSetUsedCase(dvUsedCase() + 1);
    }
    return Case;}
utInlineC dvCase dvCaseAlloc(void) {
    dvCase Case = dvCaseAllocRaw();
    dvCaseSetEntrySym(Case, utSymNull);
    dvCaseSetEntry(Case, dvEntryNull);
    dvCaseSetNextEntryCase(Case, dvCaseNull);
    dvCaseSetProperty(Case, dvPropertyNull);
    dvCaseSetNextPropertyCase(Case, dvCaseNull);
    if(dvCaseConstructorCallback != NULL) {
        dvCaseConstructorCallback(Case);
    }
    return Case;}

/*----------------------------------------------------------------------------------------
  Fields for class Cache.
----------------------------------------------------------------------------------------*/
struct dvCacheFields {
    uint16 *Number;
    uint32 *Line;
    dvClass *Class;
    dvCache *NextClassCache;
    dvProperty *FirstProperty;
    dvProperty *LastProperty;
    dvPropident *FirstPropident;
    dvPropident *LastPropident;
};
extern struct dvCacheFields dvCaches;

void dvCacheAllocMore(void);
void dvCacheCopyProps(dvCache dvOldCache, dvCache dvNewCache);
utInlineC uint16 dvCacheGetNumber(dvCache Cache) {return dvCaches.Number[dvCache2ValidIndex(Cache)];}
utInlineC void dvCacheSetNumber(dvCache Cache, uint16 value) {dvCaches.Number[dvCache2ValidIndex(Cache)] = value;}
utInlineC uint32 dvCacheGetLine(dvCache Cache) {return dvCaches.Line[dvCache2ValidIndex(Cache)];}
utInlineC void dvCacheSetLine(dvCache Cache, uint32 value) {dvCaches.Line[dvCache2ValidIndex(Cache)] = value;}
utInlineC dvClass dvCacheGetClass(dvCache Cache) {return dvCaches.Class[dvCache2ValidIndex(Cache)];}
utInlineC void dvCacheSetClass(dvCache Cache, dvClass value) {dvCaches.Class[dvCache2ValidIndex(Cache)] = value;}
utInlineC dvCache dvCacheGetNextClassCache(dvCache Cache) {return dvCaches.NextClassCache[dvCache2ValidIndex(Cache)];}
utInlineC void dvCacheSetNextClassCache(dvCache Cache, dvCache value) {dvCaches.NextClassCache[dvCache2ValidIndex(Cache)] = value;}
utInlineC dvProperty dvCacheGetFirstProperty(dvCache Cache) {return dvCaches.FirstProperty[dvCache2ValidIndex(Cache)];}
utInlineC void dvCacheSetFirstProperty(dvCache Cache, dvProperty value) {dvCaches.FirstProperty[dvCache2ValidIndex(Cache)] = value;}
utInlineC dvProperty dvCacheGetLastProperty(dvCache Cache) {return dvCaches.LastProperty[dvCache2ValidIndex(Cache)];}
utInlineC void dvCacheSetLastProperty(dvCache Cache, dvProperty value) {dvCaches.LastProperty[dvCache2ValidIndex(Cache)] = value;}
utInlineC dvPropident dvCacheGetFirstPropident(dvCache Cache) {return dvCaches.FirstPropident[dvCache2ValidIndex(Cache)];}
utInlineC void dvCacheSetFirstPropident(dvCache Cache, dvPropident value) {dvCaches.FirstPropident[dvCache2ValidIndex(Cache)] = value;}
utInlineC dvPropident dvCacheGetLastPropident(dvCache Cache) {return dvCaches.LastPropident[dvCache2ValidIndex(Cache)];}
utInlineC void dvCacheSetLastPropident(dvCache Cache, dvPropident value) {dvCaches.LastPropident[dvCache2ValidIndex(Cache)] = value;}
utInlineC void dvCacheSetConstructorCallback(void(*func)(dvCache)) {dvCacheConstructorCallback = func;}
utInlineC dvCacheCallbackType dvCacheGetConstructorCallback(void) {return dvCacheConstructorCallback;}
utInlineC void dvCacheSetDestructorCallback(void(*func)(dvCache)) {dvCacheDestructorCallback = func;}
utInlineC dvCacheCallbackType dvCacheGetDestructorCallback(void) {return dvCacheDestructorCallback;}
utInlineC dvCache dvCacheNextFree(dvCache Cache) {return ((dvCache *)(void *)(dvCaches.Class))[dvCache2ValidIndex(Cache)];}
utInlineC void dvCacheSetNextFree(dvCache Cache, dvCache value) {
    ((dvCache *)(void *)(dvCaches.Class))[dvCache2ValidIndex(Cache)] = value;}
utInlineC void dvCacheFree(dvCache Cache) {
    dvCacheSetNextFree(Cache, dvRootData.firstFreeCache);
    dvSetFirstFreeCache(Cache);}
void dvCacheDestroy(dvCache Cache);
utInlineC dvCache dvCacheAllocRaw(void) {
    dvCache Cache;
    if(dvRootData.firstFreeCache != dvCacheNull) {
        Cache = dvRootData.firstFreeCache;
        dvSetFirstFreeCache(dvCacheNextFree(Cache));
    } else {
        if(dvRootData.usedCache == dvRootData.allocatedCache) {
            dvCacheAllocMore();
        }
        Cache = dvIndex2Cache(dvRootData.usedCache);
        dvSetUsedCache(dvUsedCache() + 1);
    }
    return Cache;}
utInlineC dvCache dvCacheAlloc(void) {
    dvCache Cache = dvCacheAllocRaw();
    dvCacheSetNumber(Cache, 0);
    dvCacheSetLine(Cache, 0);
    dvCacheSetClass(Cache, dvClassNull);
    dvCacheSetNextClassCache(Cache, dvCacheNull);
    dvCacheSetFirstProperty(Cache, dvPropertyNull);
    dvCacheSetLastProperty(Cache, dvPropertyNull);
    dvCacheSetFirstPropident(Cache, dvPropidentNull);
    dvCacheSetLastPropident(Cache, dvPropidentNull);
    if(dvCacheConstructorCallback != NULL) {
        dvCacheConstructorCallback(Cache);
    }
    return Cache;}

/*----------------------------------------------------------------------------------------
  Fields for class Propident.
----------------------------------------------------------------------------------------*/
struct dvPropidentFields {
    utSym *Sym;
    dvCache *Cache;
    dvPropident *NextCachePropident;
};
extern struct dvPropidentFields dvPropidents;

void dvPropidentAllocMore(void);
void dvPropidentCopyProps(dvPropident dvOldPropident, dvPropident dvNewPropident);
utInlineC utSym dvPropidentGetSym(dvPropident Propident) {return dvPropidents.Sym[dvPropident2ValidIndex(Propident)];}
utInlineC void dvPropidentSetSym(dvPropident Propident, utSym value) {dvPropidents.Sym[dvPropident2ValidIndex(Propident)] = value;}
utInlineC dvCache dvPropidentGetCache(dvPropident Propident) {return dvPropidents.Cache[dvPropident2ValidIndex(Propident)];}
utInlineC void dvPropidentSetCache(dvPropident Propident, dvCache value) {dvPropidents.Cache[dvPropident2ValidIndex(Propident)] = value;}
utInlineC dvPropident dvPropidentGetNextCachePropident(dvPropident Propident) {return dvPropidents.NextCachePropident[dvPropident2ValidIndex(Propident)];}
utInlineC void dvPropidentSetNextCachePropident(dvPropident Propident, dvPropident value) {dvPropidents.NextCachePropident[dvPropident2ValidIndex(Propident)] = value;}
utInlineC void dvPropidentSetConstructorCallback(void(*func)(dvPropident)) {dvPropidentConstructorCallback = func;}
utInlineC dvPropidentCallbackType dvPropidentGetConstructorCallback(void) {return dvPropidentConstructorCallback;}
utInlineC void dvPropidentSetDestructorCallback(void(*func)(dvPropident)) {dvPropidentDestructorCallback = func;}
utInlineC dvPropidentCallbackType dvPropidentGetDestructorCallback(void) {return dvPropidentDestructorCallback;}
utInlineC dvPropident dvPropidentNextFree(dvPropident Propident) {return ((dvPropident *)(void *)(dvPropidents.Sym))[dvPropident2ValidIndex(Propident)];}
utInlineC void dvPropidentSetNextFree(dvPropident Propident, dvPropident value) {
    ((dvPropident *)(void *)(dvPropidents.Sym))[dvPropident2ValidIndex(Propident)] = value;}
utInlineC void dvPropidentFree(dvPropident Propident) {
    dvPropidentSetNextFree(Propident, dvRootData.firstFreePropident);
    dvSetFirstFreePropident(Propident);}
void dvPropidentDestroy(dvPropident Propident);
utInlineC dvPropident dvPropidentAllocRaw(void) {
    dvPropident Propident;
    if(dvRootData.firstFreePropident != dvPropidentNull) {
        Propident = dvRootData.firstFreePropident;
        dvSetFirstFreePropident(dvPropidentNextFree(Propident));
    } else {
        if(dvRootData.usedPropident == dvRootData.allocatedPropident) {
            dvPropidentAllocMore();
        }
        Propident = dvIndex2Propident(dvRootData.usedPropident);
        dvSetUsedPropident(dvUsedPropident() + 1);
    }
    return Propident;}
utInlineC dvPropident dvPropidentAlloc(void) {
    dvPropident Propident = dvPropidentAllocRaw();
    dvPropidentSetSym(Propident, utSymNull);
    dvPropidentSetCache(Propident, dvCacheNull);
    dvPropidentSetNextCachePropident(Propident, dvPropidentNull);
    if(dvPropidentConstructorCallback != NULL) {
        dvPropidentConstructorCallback(Propident);
    }
    return Propident;}

/*----------------------------------------------------------------------------------------
  Relationship macros between classes.
----------------------------------------------------------------------------------------*/
dvModpath dvRootFindModpath(dvRoot Root, utSym Sym);
void dvRootRenameModpath(dvRoot Root, dvModpath _Modpath, utSym sym);
utInlineC char *dvModpathGetName(dvModpath Modpath) {return utSymGetName(dvModpathGetSym(Modpath));}
#define dvForeachRootModpath(pVar, cVar) \
    for(cVar = dvRootGetFirstModpath(pVar); cVar != dvModpathNull; \
        cVar = dvModpathGetNextRootModpath(cVar))
#define dvEndRootModpath
#define dvSafeForeachRootModpath(pVar, cVar) { \
    dvModpath _nextModpath; \
    for(cVar = dvRootGetFirstModpath(pVar); cVar != dvModpathNull; cVar = _nextModpath) { \
        _nextModpath = dvModpathGetNextRootModpath(cVar);
#define dvEndSafeRootModpath }}
dvModule dvRootFindModule(dvRoot Root, utSym Sym);
void dvRootRenameModule(dvRoot Root, dvModule _Module, utSym sym);
utInlineC char *dvModuleGetName(dvModule Module) {return utSymGetName(dvModuleGetSym(Module));}
#define dvForeachRootModule(pVar, cVar) \
    for(cVar = dvRootGetFirstModule(pVar); cVar != dvModuleNull; \
        cVar = dvModuleGetNextRootModule(cVar))
#define dvEndRootModule
#define dvSafeForeachRootModule(pVar, cVar) { \
    dvModule _nextModule; \
    for(cVar = dvRootGetFirstModule(pVar); cVar != dvModuleNull; cVar = _nextModule) { \
        _nextModule = dvModuleGetNextRootModule(cVar);
#define dvEndSafeRootModule }}
void dvRootInsertModpath(dvRoot Root, dvModpath _Modpath);
void dvRootRemoveModpath(dvRoot Root, dvModpath _Modpath);
void dvRootInsertAfterModpath(dvRoot Root, dvModpath prevModpath, dvModpath _Modpath);
void dvRootAppendModpath(dvRoot Root, dvModpath _Modpath);
void dvRootInsertModule(dvRoot Root, dvModule _Module);
void dvRootRemoveModule(dvRoot Root, dvModule _Module);
void dvRootInsertAfterModule(dvRoot Root, dvModule prevModule, dvModule _Module);
void dvRootAppendModule(dvRoot Root, dvModule _Module);
dvClass dvModuleFindClass(dvModule Module, utSym Sym);
void dvModuleRenameClass(dvModule Module, dvClass _Class, utSym sym);
utInlineC char *dvClassGetName(dvClass Class) {return utSymGetName(dvClassGetSym(Class));}
#define dvForeachModuleClass(pVar, cVar) \
    for(cVar = dvModuleGetFirstClass(pVar); cVar != dvClassNull; \
        cVar = dvClassGetNextModuleClass(cVar))
#define dvEndModuleClass
#define dvSafeForeachModuleClass(pVar, cVar) { \
    dvClass _nextClass; \
    for(cVar = dvModuleGetFirstClass(pVar); cVar != dvClassNull; cVar = _nextClass) { \
        _nextClass = dvClassGetNextModuleClass(cVar);
#define dvEndSafeModuleClass }}
dvEnum dvModuleFindEnum(dvModule Module, utSym Sym);
void dvModuleRenameEnum(dvModule Module, dvEnum _Enum, utSym sym);
utInlineC char *dvEnumGetName(dvEnum Enum) {return utSymGetName(dvEnumGetSym(Enum));}
#define dvForeachModuleEnum(pVar, cVar) \
    for(cVar = dvModuleGetFirstEnum(pVar); cVar != dvEnumNull; \
        cVar = dvEnumGetNextModuleEnum(cVar))
#define dvEndModuleEnum
#define dvSafeForeachModuleEnum(pVar, cVar) { \
    dvEnum _nextEnum; \
    for(cVar = dvModuleGetFirstEnum(pVar); cVar != dvEnumNull; cVar = _nextEnum) { \
        _nextEnum = dvEnumGetNextModuleEnum(cVar);
#define dvEndSafeModuleEnum }}
dvTypedef dvModuleFindTypedef(dvModule Module, utSym Sym);
void dvModuleRenameTypedef(dvModule Module, dvTypedef _Typedef, utSym sym);
utInlineC char *dvTypedefGetName(dvTypedef Typedef) {return utSymGetName(dvTypedefGetSym(Typedef));}
#define dvForeachModuleTypedef(pVar, cVar) \
    for(cVar = dvModuleGetFirstTypedef(pVar); cVar != dvTypedefNull; \
        cVar = dvTypedefGetNextModuleTypedef(cVar))
#define dvEndModuleTypedef
#define dvSafeForeachModuleTypedef(pVar, cVar) { \
    dvTypedef _nextTypedef; \
    for(cVar = dvModuleGetFirstTypedef(pVar); cVar != dvTypedefNull; cVar = _nextTypedef) { \
        _nextTypedef = dvTypedefGetNextModuleTypedef(cVar);
#define dvEndSafeModuleTypedef }}
dvSchema dvModuleFindSchema(dvModule Module, utSym Sym);
void dvModuleRenameSchema(dvModule Module, dvSchema _Schema, utSym sym);
utInlineC char *dvSchemaGetName(dvSchema Schema) {return utSymGetName(dvSchemaGetSym(Schema));}
#define dvForeachModuleSchema(pVar, cVar) \
    for(cVar = dvModuleGetFirstSchema(pVar); cVar != dvSchemaNull; \
        cVar = dvSchemaGetNextModuleSchema(cVar))
#define dvEndModuleSchema
#define dvSafeForeachModuleSchema(pVar, cVar) { \
    dvSchema _nextSchema; \
    for(cVar = dvModuleGetFirstSchema(pVar); cVar != dvSchemaNull; cVar = _nextSchema) { \
        _nextSchema = dvSchemaGetNextModuleSchema(cVar);
#define dvEndSafeModuleSchema }}
#define dvForeachModuleImportLink(pVar, cVar) \
    for(cVar = dvModuleGetFirstImportLink(pVar); cVar != dvLinkNull; \
        cVar = dvLinkGetNextModuleImportLink(cVar))
#define dvEndModuleImportLink
#define dvSafeForeachModuleImportLink(pVar, cVar) { \
    dvLink _nextLink; \
    for(cVar = dvModuleGetFirstImportLink(pVar); cVar != dvLinkNull; cVar = _nextLink) { \
        _nextLink = dvLinkGetNextModuleImportLink(cVar);
#define dvEndSafeModuleImportLink }}
#define dvForeachModuleExportLink(pVar, cVar) \
    for(cVar = dvModuleGetFirstExportLink(pVar); cVar != dvLinkNull; \
        cVar = dvLinkGetNextModuleExportLink(cVar))
#define dvEndModuleExportLink
#define dvSafeForeachModuleExportLink(pVar, cVar) { \
    dvLink _nextLink; \
    for(cVar = dvModuleGetFirstExportLink(pVar); cVar != dvLinkNull; cVar = _nextLink) { \
        _nextLink = dvLinkGetNextModuleExportLink(cVar);
#define dvEndSafeModuleExportLink }}
void dvModuleInsertClass(dvModule Module, dvClass _Class);
void dvModuleRemoveClass(dvModule Module, dvClass _Class);
void dvModuleInsertAfterClass(dvModule Module, dvClass prevClass, dvClass _Class);
void dvModuleAppendClass(dvModule Module, dvClass _Class);
void dvModuleInsertEnum(dvModule Module, dvEnum _Enum);
void dvModuleRemoveEnum(dvModule Module, dvEnum _Enum);
void dvModuleInsertAfterEnum(dvModule Module, dvEnum prevEnum, dvEnum _Enum);
void dvModuleAppendEnum(dvModule Module, dvEnum _Enum);
void dvModuleInsertTypedef(dvModule Module, dvTypedef _Typedef);
void dvModuleRemoveTypedef(dvModule Module, dvTypedef _Typedef);
void dvModuleInsertAfterTypedef(dvModule Module, dvTypedef prevTypedef, dvTypedef _Typedef);
void dvModuleAppendTypedef(dvModule Module, dvTypedef _Typedef);
void dvModuleInsertSchema(dvModule Module, dvSchema _Schema);
void dvModuleRemoveSchema(dvModule Module, dvSchema _Schema);
void dvModuleInsertAfterSchema(dvModule Module, dvSchema prevSchema, dvSchema _Schema);
void dvModuleAppendSchema(dvModule Module, dvSchema _Schema);
void dvModuleInsertImportLink(dvModule Module, dvLink _Link);
void dvModuleRemoveImportLink(dvModule Module, dvLink _Link);
void dvModuleInsertAfterImportLink(dvModule Module, dvLink prevLink, dvLink _Link);
void dvModuleAppendImportLink(dvModule Module, dvLink _Link);
void dvModuleInsertExportLink(dvModule Module, dvLink _Link);
void dvModuleRemoveExportLink(dvModule Module, dvLink _Link);
void dvModuleInsertAfterExportLink(dvModule Module, dvLink prevLink, dvLink _Link);
void dvModuleAppendExportLink(dvModule Module, dvLink _Link);
#define dvForeachSchemaRelationship(pVar, cVar) \
    for(cVar = dvSchemaGetFirstRelationship(pVar); cVar != dvRelationshipNull; \
        cVar = dvRelationshipGetNextSchemaRelationship(cVar))
#define dvEndSchemaRelationship
#define dvSafeForeachSchemaRelationship(pVar, cVar) { \
    dvRelationship _nextRelationship; \
    for(cVar = dvSchemaGetFirstRelationship(pVar); cVar != dvRelationshipNull; cVar = _nextRelationship) { \
        _nextRelationship = dvRelationshipGetNextSchemaRelationship(cVar);
#define dvEndSafeSchemaRelationship }}
void dvSchemaInsertRelationship(dvSchema Schema, dvRelationship _Relationship);
void dvSchemaRemoveRelationship(dvSchema Schema, dvRelationship _Relationship);
void dvSchemaInsertAfterRelationship(dvSchema Schema, dvRelationship prevRelationship, dvRelationship _Relationship);
void dvSchemaAppendRelationship(dvSchema Schema, dvRelationship _Relationship);
dvEntry dvEnumFindEntry(dvEnum Enum, utSym Sym);
void dvEnumRenameEntry(dvEnum Enum, dvEntry _Entry, utSym sym);
utInlineC char *dvEntryGetName(dvEntry Entry) {return utSymGetName(dvEntryGetSym(Entry));}
#define dvForeachEnumEntry(pVar, cVar) \
    for(cVar = dvEnumGetFirstEntry(pVar); cVar != dvEntryNull; \
        cVar = dvEntryGetNextEnumEntry(cVar))
#define dvEndEnumEntry
#define dvSafeForeachEnumEntry(pVar, cVar) { \
    dvEntry _nextEntry; \
    for(cVar = dvEnumGetFirstEntry(pVar); cVar != dvEntryNull; cVar = _nextEntry) { \
        _nextEntry = dvEntryGetNextEnumEntry(cVar);
#define dvEndSafeEnumEntry }}
void dvEnumInsertEntry(dvEnum Enum, dvEntry _Entry);
void dvEnumRemoveEntry(dvEnum Enum, dvEntry _Entry);
void dvEnumInsertAfterEntry(dvEnum Enum, dvEntry prevEntry, dvEntry _Entry);
void dvEnumAppendEntry(dvEnum Enum, dvEntry _Entry);
#define dvForeachEntryCase(pVar, cVar) \
    for(cVar = dvEntryGetFirstCase(pVar); cVar != dvCaseNull; \
        cVar = dvCaseGetNextEntryCase(cVar))
#define dvEndEntryCase
#define dvSafeForeachEntryCase(pVar, cVar) { \
    dvCase _nextCase; \
    for(cVar = dvEntryGetFirstCase(pVar); cVar != dvCaseNull; cVar = _nextCase) { \
        _nextCase = dvCaseGetNextEntryCase(cVar);
#define dvEndSafeEntryCase }}
void dvEntryInsertCase(dvEntry Entry, dvCase _Case);
void dvEntryRemoveCase(dvEntry Entry, dvCase _Case);
void dvEntryInsertAfterCase(dvEntry Entry, dvCase prevCase, dvCase _Case);
void dvEntryAppendCase(dvEntry Entry, dvCase _Case);
dvProperty dvClassFindProperty(dvClass Class, utSym Sym);
void dvClassRenameProperty(dvClass Class, dvProperty _Property, utSym sym);
utInlineC char *dvPropertyGetName(dvProperty Property) {return utSymGetName(dvPropertyGetSym(Property));}
#define dvForeachClassProperty(pVar, cVar) \
    for(cVar = dvClassGetFirstProperty(pVar); cVar != dvPropertyNull; \
        cVar = dvPropertyGetNextClassProperty(cVar))
#define dvEndClassProperty
#define dvSafeForeachClassProperty(pVar, cVar) { \
    dvProperty _nextProperty; \
    for(cVar = dvClassGetFirstProperty(pVar); cVar != dvPropertyNull; cVar = _nextProperty) { \
        _nextProperty = dvPropertyGetNextClassProperty(cVar);
#define dvEndSafeClassProperty }}
dvSparsegroup dvClassFindSparsegroup(dvClass Class, utSym Sym);
void dvClassRenameSparsegroup(dvClass Class, dvSparsegroup _Sparsegroup, utSym sym);
utInlineC char *dvSparsegroupGetName(dvSparsegroup Sparsegroup) {return utSymGetName(dvSparsegroupGetSym(Sparsegroup));}
#define dvForeachClassSparsegroup(pVar, cVar) \
    for(cVar = dvClassGetFirstSparsegroup(pVar); cVar != dvSparsegroupNull; \
        cVar = dvSparsegroupGetNextClassSparsegroup(cVar))
#define dvEndClassSparsegroup
#define dvSafeForeachClassSparsegroup(pVar, cVar) { \
    dvSparsegroup _nextSparsegroup; \
    for(cVar = dvClassGetFirstSparsegroup(pVar); cVar != dvSparsegroupNull; cVar = _nextSparsegroup) { \
        _nextSparsegroup = dvSparsegroupGetNextClassSparsegroup(cVar);
#define dvEndSafeClassSparsegroup }}
#define dvForeachClassDerivedClass(pVar, cVar) \
    for(cVar = dvClassGetFirstDerivedClass(pVar); cVar != dvClassNull; \
        cVar = dvClassGetNextClassDerivedClass(cVar))
#define dvEndClassDerivedClass
#define dvSafeForeachClassDerivedClass(pVar, cVar) { \
    dvClass _nextClass; \
    for(cVar = dvClassGetFirstDerivedClass(pVar); cVar != dvClassNull; cVar = _nextClass) { \
        _nextClass = dvClassGetNextClassDerivedClass(cVar);
#define dvEndSafeClassDerivedClass }}
#define dvForeachClassChildRelationship(pVar, cVar) \
    for(cVar = dvClassGetFirstChildRelationship(pVar); cVar != dvRelationshipNull; \
        cVar = dvRelationshipGetNextClassChildRelationship(cVar))
#define dvEndClassChildRelationship
#define dvSafeForeachClassChildRelationship(pVar, cVar) { \
    dvRelationship _nextRelationship; \
    for(cVar = dvClassGetFirstChildRelationship(pVar); cVar != dvRelationshipNull; cVar = _nextRelationship) { \
        _nextRelationship = dvRelationshipGetNextClassChildRelationship(cVar);
#define dvEndSafeClassChildRelationship }}
#define dvForeachClassParentRelationship(pVar, cVar) \
    for(cVar = dvClassGetFirstParentRelationship(pVar); cVar != dvRelationshipNull; \
        cVar = dvRelationshipGetNextClassParentRelationship(cVar))
#define dvEndClassParentRelationship
#define dvSafeForeachClassParentRelationship(pVar, cVar) { \
    dvRelationship _nextRelationship; \
    for(cVar = dvClassGetFirstParentRelationship(pVar); cVar != dvRelationshipNull; cVar = _nextRelationship) { \
        _nextRelationship = dvRelationshipGetNextClassParentRelationship(cVar);
#define dvEndSafeClassParentRelationship }}
#define dvForeachClassUnion(pVar, cVar) \
    for(cVar = dvClassGetFirstUnion(pVar); cVar != dvUnionNull; \
        cVar = dvUnionGetNextClassUnion(cVar))
#define dvEndClassUnion
#define dvSafeForeachClassUnion(pVar, cVar) { \
    dvUnion _nextUnion; \
    for(cVar = dvClassGetFirstUnion(pVar); cVar != dvUnionNull; cVar = _nextUnion) { \
        _nextUnion = dvUnionGetNextClassUnion(cVar);
#define dvEndSafeClassUnion }}
#define dvForeachClassCache(pVar, cVar) \
    for(cVar = dvClassGetFirstCache(pVar); cVar != dvCacheNull; \
        cVar = dvCacheGetNextClassCache(cVar))
#define dvEndClassCache
#define dvSafeForeachClassCache(pVar, cVar) { \
    dvCache _nextCache; \
    for(cVar = dvClassGetFirstCache(pVar); cVar != dvCacheNull; cVar = _nextCache) { \
        _nextCache = dvCacheGetNextClassCache(cVar);
#define dvEndSafeClassCache }}
void dvClassInsertProperty(dvClass Class, dvProperty _Property);
void dvClassRemoveProperty(dvClass Class, dvProperty _Property);
void dvClassInsertAfterProperty(dvClass Class, dvProperty prevProperty, dvProperty _Property);
void dvClassAppendProperty(dvClass Class, dvProperty _Property);
void dvClassInsertSparsegroup(dvClass Class, dvSparsegroup _Sparsegroup);
void dvClassRemoveSparsegroup(dvClass Class, dvSparsegroup _Sparsegroup);
void dvClassInsertAfterSparsegroup(dvClass Class, dvSparsegroup prevSparsegroup, dvSparsegroup _Sparsegroup);
void dvClassAppendSparsegroup(dvClass Class, dvSparsegroup _Sparsegroup);
void dvClassInsertDerivedClass(dvClass Class, dvClass _Class);
void dvClassRemoveDerivedClass(dvClass Class, dvClass _Class);
void dvClassInsertAfterDerivedClass(dvClass Class, dvClass prevClass, dvClass _Class);
void dvClassAppendDerivedClass(dvClass Class, dvClass _Class);
void dvClassInsertChildRelationship(dvClass Class, dvRelationship _Relationship);
void dvClassRemoveChildRelationship(dvClass Class, dvRelationship _Relationship);
void dvClassInsertAfterChildRelationship(dvClass Class, dvRelationship prevRelationship, dvRelationship _Relationship);
void dvClassAppendChildRelationship(dvClass Class, dvRelationship _Relationship);
void dvClassInsertParentRelationship(dvClass Class, dvRelationship _Relationship);
void dvClassRemoveParentRelationship(dvClass Class, dvRelationship _Relationship);
void dvClassInsertAfterParentRelationship(dvClass Class, dvRelationship prevRelationship, dvRelationship _Relationship);
void dvClassAppendParentRelationship(dvClass Class, dvRelationship _Relationship);
void dvClassInsertUnion(dvClass Class, dvUnion _Union);
void dvClassRemoveUnion(dvClass Class, dvUnion _Union);
void dvClassInsertAfterUnion(dvClass Class, dvUnion prevUnion, dvUnion _Union);
void dvClassAppendUnion(dvClass Class, dvUnion _Union);
void dvClassInsertCache(dvClass Class, dvCache _Cache);
void dvClassRemoveCache(dvClass Class, dvCache _Cache);
void dvClassInsertAfterCache(dvClass Class, dvCache prevCache, dvCache _Cache);
void dvClassAppendCache(dvClass Class, dvCache _Cache);
#define dvForeachPropertyCase(pVar, cVar) \
    for(cVar = dvPropertyGetFirstCase(pVar); cVar != dvCaseNull; \
        cVar = dvCaseGetNextPropertyCase(cVar))
#define dvEndPropertyCase
#define dvSafeForeachPropertyCase(pVar, cVar) { \
    dvCase _nextCase; \
    for(cVar = dvPropertyGetFirstCase(pVar); cVar != dvCaseNull; cVar = _nextCase) { \
        _nextCase = dvCaseGetNextPropertyCase(cVar);
#define dvEndSafePropertyCase }}
#define dvForeachPropertyKeyproperty(pVar, cVar) \
    for(cVar = dvPropertyGetFirstKeyproperty(pVar); cVar != dvKeypropertyNull; \
        cVar = dvKeypropertyGetNextPropertyKeyproperty(cVar))
#define dvEndPropertyKeyproperty
#define dvSafeForeachPropertyKeyproperty(pVar, cVar) { \
    dvKeyproperty _nextKeyproperty; \
    for(cVar = dvPropertyGetFirstKeyproperty(pVar); cVar != dvKeypropertyNull; cVar = _nextKeyproperty) { \
        _nextKeyproperty = dvKeypropertyGetNextPropertyKeyproperty(cVar);
#define dvEndSafePropertyKeyproperty }}
void dvPropertyInsertCase(dvProperty Property, dvCase _Case);
void dvPropertyRemoveCase(dvProperty Property, dvCase _Case);
void dvPropertyInsertAfterCase(dvProperty Property, dvCase prevCase, dvCase _Case);
void dvPropertyAppendCase(dvProperty Property, dvCase _Case);
void dvPropertyInsertKeyproperty(dvProperty Property, dvKeyproperty _Keyproperty);
void dvPropertyRemoveKeyproperty(dvProperty Property, dvKeyproperty _Keyproperty);
void dvPropertyInsertAfterKeyproperty(dvProperty Property, dvKeyproperty prevKeyproperty, dvKeyproperty _Keyproperty);
void dvPropertyAppendKeyproperty(dvProperty Property, dvKeyproperty _Keyproperty);
#define dvForeachSparsegroupProperty(pVar, cVar) \
    for(cVar = dvSparsegroupGetFirstProperty(pVar); cVar != dvPropertyNull; \
        cVar = dvPropertyGetNextSparsegroupProperty(cVar))
#define dvEndSparsegroupProperty
#define dvSafeForeachSparsegroupProperty(pVar, cVar) { \
    dvProperty _nextProperty; \
    for(cVar = dvSparsegroupGetFirstProperty(pVar); cVar != dvPropertyNull; cVar = _nextProperty) { \
        _nextProperty = dvPropertyGetNextSparsegroupProperty(cVar);
#define dvEndSafeSparsegroupProperty }}
void dvSparsegroupInsertProperty(dvSparsegroup Sparsegroup, dvProperty _Property);
void dvSparsegroupRemoveProperty(dvSparsegroup Sparsegroup, dvProperty _Property);
void dvSparsegroupInsertAfterProperty(dvSparsegroup Sparsegroup, dvProperty prevProperty, dvProperty _Property);
void dvSparsegroupAppendProperty(dvSparsegroup Sparsegroup, dvProperty _Property);
#define dvForeachRelationshipProperty(pVar, cVar) \
    for(cVar = dvRelationshipGetFirstProperty(pVar); cVar != dvPropertyNull; \
        cVar = dvPropertyGetNextRelationshipProperty(cVar))
#define dvEndRelationshipProperty
#define dvSafeForeachRelationshipProperty(pVar, cVar) { \
    dvProperty _nextProperty; \
    for(cVar = dvRelationshipGetFirstProperty(pVar); cVar != dvPropertyNull; cVar = _nextProperty) { \
        _nextProperty = dvPropertyGetNextRelationshipProperty(cVar);
#define dvEndSafeRelationshipProperty }}
#define dvForeachRelationshipKey(pVar, cVar) \
    for(cVar = dvRelationshipGetFirstKey(pVar); cVar != dvKeyNull; \
        cVar = dvKeyGetNextRelationshipKey(cVar))
#define dvEndRelationshipKey
#define dvSafeForeachRelationshipKey(pVar, cVar) { \
    dvKey _nextKey; \
    for(cVar = dvRelationshipGetFirstKey(pVar); cVar != dvKeyNull; cVar = _nextKey) { \
        _nextKey = dvKeyGetNextRelationshipKey(cVar);
#define dvEndSafeRelationshipKey }}
void dvRelationshipInsertProperty(dvRelationship Relationship, dvProperty _Property);
void dvRelationshipRemoveProperty(dvRelationship Relationship, dvProperty _Property);
void dvRelationshipInsertAfterProperty(dvRelationship Relationship, dvProperty prevProperty, dvProperty _Property);
void dvRelationshipAppendProperty(dvRelationship Relationship, dvProperty _Property);
void dvRelationshipInsertKey(dvRelationship Relationship, dvKey _Key);
void dvRelationshipRemoveKey(dvRelationship Relationship, dvKey _Key);
void dvRelationshipInsertAfterKey(dvRelationship Relationship, dvKey prevKey, dvKey _Key);
void dvRelationshipAppendKey(dvRelationship Relationship, dvKey _Key);
utInlineC void dvRelationshipInsertParentSparsegroup(dvRelationship Relationship, dvSparsegroup _Sparsegroup) {dvRelationshipSetParentSparsegroup(Relationship, _Sparsegroup); dvSparsegroupSetRelationship(_Sparsegroup, Relationship);}
utInlineC void dvRelationshipRemoveParentSparsegroup(dvRelationship Relationship, dvSparsegroup _Sparsegroup) {dvRelationshipSetParentSparsegroup(Relationship, dvSparsegroupNull); dvSparsegroupSetRelationship(_Sparsegroup, dvRelationshipNull);}
utInlineC void dvRelationshipInsertChildSparsegroup(dvRelationship Relationship, dvSparsegroup _Sparsegroup) {dvRelationshipSetChildSparsegroup(Relationship, _Sparsegroup); dvSparsegroupSetRelationship(_Sparsegroup, Relationship);}
utInlineC void dvRelationshipRemoveChildSparsegroup(dvRelationship Relationship, dvSparsegroup _Sparsegroup) {dvRelationshipSetChildSparsegroup(Relationship, dvSparsegroupNull); dvSparsegroupSetRelationship(_Sparsegroup, dvRelationshipNull);}
#define dvForeachKeyKeyproperty(pVar, cVar) \
    for(cVar = dvKeyGetFirstKeyproperty(pVar); cVar != dvKeypropertyNull; \
        cVar = dvKeypropertyGetNextKeyKeyproperty(cVar))
#define dvEndKeyKeyproperty
#define dvSafeForeachKeyKeyproperty(pVar, cVar) { \
    dvKeyproperty _nextKeyproperty; \
    for(cVar = dvKeyGetFirstKeyproperty(pVar); cVar != dvKeypropertyNull; cVar = _nextKeyproperty) { \
        _nextKeyproperty = dvKeypropertyGetNextKeyKeyproperty(cVar);
#define dvEndSafeKeyKeyproperty }}
void dvKeyInsertKeyproperty(dvKey Key, dvKeyproperty _Keyproperty);
void dvKeyRemoveKeyproperty(dvKey Key, dvKeyproperty _Keyproperty);
void dvKeyInsertAfterKeyproperty(dvKey Key, dvKeyproperty prevKeyproperty, dvKeyproperty _Keyproperty);
void dvKeyAppendKeyproperty(dvKey Key, dvKeyproperty _Keyproperty);
#define dvForeachUnionProperty(pVar, cVar) \
    for(cVar = dvUnionGetFirstProperty(pVar); cVar != dvPropertyNull; \
        cVar = dvPropertyGetNextUnionProperty(cVar))
#define dvEndUnionProperty
#define dvSafeForeachUnionProperty(pVar, cVar) { \
    dvProperty _nextProperty; \
    for(cVar = dvUnionGetFirstProperty(pVar); cVar != dvPropertyNull; cVar = _nextProperty) { \
        _nextProperty = dvPropertyGetNextUnionProperty(cVar);
#define dvEndSafeUnionProperty }}
void dvUnionInsertProperty(dvUnion Union, dvProperty _Property);
void dvUnionRemoveProperty(dvUnion Union, dvProperty _Property);
void dvUnionInsertAfterProperty(dvUnion Union, dvProperty prevProperty, dvProperty _Property);
void dvUnionAppendProperty(dvUnion Union, dvProperty _Property);
#define dvForeachCacheProperty(pVar, cVar) \
    for(cVar = dvCacheGetFirstProperty(pVar); cVar != dvPropertyNull; \
        cVar = dvPropertyGetNextCacheProperty(cVar))
#define dvEndCacheProperty
#define dvSafeForeachCacheProperty(pVar, cVar) { \
    dvProperty _nextProperty; \
    for(cVar = dvCacheGetFirstProperty(pVar); cVar != dvPropertyNull; cVar = _nextProperty) { \
        _nextProperty = dvPropertyGetNextCacheProperty(cVar);
#define dvEndSafeCacheProperty }}
#define dvForeachCachePropident(pVar, cVar) \
    for(cVar = dvCacheGetFirstPropident(pVar); cVar != dvPropidentNull; \
        cVar = dvPropidentGetNextCachePropident(cVar))
#define dvEndCachePropident
#define dvSafeForeachCachePropident(pVar, cVar) { \
    dvPropident _nextPropident; \
    for(cVar = dvCacheGetFirstPropident(pVar); cVar != dvPropidentNull; cVar = _nextPropident) { \
        _nextPropident = dvPropidentGetNextCachePropident(cVar);
#define dvEndSafeCachePropident }}
void dvCacheInsertProperty(dvCache Cache, dvProperty _Property);
void dvCacheRemoveProperty(dvCache Cache, dvProperty _Property);
void dvCacheInsertAfterProperty(dvCache Cache, dvProperty prevProperty, dvProperty _Property);
void dvCacheAppendProperty(dvCache Cache, dvProperty _Property);
void dvCacheInsertPropident(dvCache Cache, dvPropident _Propident);
void dvCacheRemovePropident(dvCache Cache, dvPropident _Propident);
void dvCacheInsertAfterPropident(dvCache Cache, dvPropident prevPropident, dvPropident _Propident);
void dvCacheAppendPropident(dvCache Cache, dvPropident _Propident);
void dvDatabaseStart(void);
void dvDatabaseStop(void);
#if defined __cplusplus
}
#endif

#endif
