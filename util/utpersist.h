/*
 * This file was written by Bill Cox.  It is hereby placed into the public domain.
 */
#ifndef UTPERSIST_H
#define UTPERSIST_H

/* These are the commands supported in the recentChanges.  The high bit of a command byte is used
 * to indicate that the command is an undo command, rather than a redo command. */
typedef enum {
    UT_TRANSACTION_COMPLETE = 0, /* TRANSACTION_COMPLETE <32-bit checksum> */
    UT_WRITE_FIELD = 1, /* WRITE_FIELD <16-bit field #> <object number> <value> */
    UT_WRITE_ARRAY = 2, /* WRITE_ARRAY <16-bit field #> <32-bit index> <32-bit numValues> <values> */
    UT_WRITE_GLOBAL = 3, /* WRITE_GLOBAL <8-bit moduleID> <16-bit offset> <8-bit numBytes> <values> */
    UT_RESIZE_FIELD = 4 /* RESIZE_FIELD <16-bit field #> <64-bit size> */
} utCommandType;

/* This is the maximum header size of a command needed to determine it's size */
#define UT_COMMAND_MAX_HEADER_SIZE 11

typedef struct utModuleStruct *utModule;
typedef struct utClassStruct *utClass;
typedef struct utFieldStruct *utField;
typedef struct utTransactionStruct *utTransaction;
typedef struct utEnumStruct *utEnum;
typedef struct utEntryStruct *utEntry;
typedef struct utUnionStruct *utUnion;
typedef struct utUnioncaseStruct *utUnioncase;

#define utModuleNull ((utModule)NULL)
#define utClassNull ((utClass)NULL)
#define utFieldNull ((utField)NULL)
#define utTransactionNull ((utTransaction)NULL)
#define utEnumNull ((utEnum)NULL)
#define utEntryNull ((utEntry)NULL)
#define utUnionNull ((utUnion)NULL)
#define utUnioncaseNull ((utUnioncase)NULL)

struct utModuleStruct {
    char *prefix;
    uint32 hashValue;
    uint32 globalSize;
    void *globalData;
    uint16 firstFieldIndex, numFields;
    uint16 firstClassIndex, numClasses;
    uint16 firstEnumIndex, numEnums;
    void (*start)(void);
    void (*stop)(void);
    bool initialized;
    bool persistent;
};

extern struct utModuleStruct *utModules;
extern uint8 utAllocatedModules, utUsedModules;
#define utModuleInitialized(module) module->initialized
#define utModuleSetInitialized(module, value) (module->initialized = (value))
#define utModulePersistent(module) module->persistent
#define utModuleSetPersistent(module, value) (module->persistent = (value))
#define utModuleGetHashValue(module) (module->hashValue)
#define utModuleSetHashValue(module, value) (module->hashValue = (value))
#define utModuleGetGlobalSize(module) (module->globalSize)
#define utModuleSetGlobalSize(module, value) (module->globalSize = (value))
#define utModuleGetPrefix(module) (module->prefix)
#define utModuleSetPrefix(module, value) (module->prefix = (value))
#define utModuleGetGlobalData(module) (module->globalData)
#define utModuleSetGlobalData(module, value) (module->globalData = (value))
#define utModuleGetStop(module) (module->stop)
#define utModuleSetStop(module, value) (module->stop = (value))
#define utModuleGetStart(module) (module->start)
#define utModuleSetStart(module, value) (module->start = (value))
#define utModuleGetFirstClassIndex(module) (module->firstClassIndex)
#define utModuleSetFirstClassIndex(module, value) (module->firstClassIndex = (value))
#define utModuleGetFirstFieldIndex(module) (module->firstFieldIndex)
#define utModuleSetFirstFieldIndex(module, value) (module->firstFieldIndex = (value))
#define utModuleGetFirstEnumIndex(module) (module->firstEnumIndex)
#define utModuleSetFirstEnumIndex(module, value) (module->firstEnumIndex = (value))
#define utModuleGetNumClasses(module) (module->numClasses)
#define utModuleSetNumClasses(module, value) (module->numClasses = (value))
#define utModuleGetNumFields(module) (module->numFields)
#define utModuleSetNumFields(module, value) (module->numFields = (value))
#define utModuleGetNumEnums(module) (module->numEnums)
#define utModuleSetNumEnums(module, value) (module->numEnums = (value))
#define utModuleGetNextModule(module) \
    ((((module) + 1) >= (utModules + utUsedModules))?utModuleNull:((module) + 1))
#define utForeachModule(module) \
    for((module) = utModules; (module) != utModuleNull; module = utModuleGetNextModule(module)) 
#define utEndModule
#define utModuleGetFirstClass(module) ((module)->numClasses > 0? utClasses + (module)->firstClassIndex : \
        utClassNull)
#define utModuleGetNextModuleClass(module, Class) \
    ((((Class) - utClasses + 1) >= ((module)->firstClassIndex + (module)->numClasses))?utClassNull:((Class) + 1))
#define utForeachModuleClass(module, Class)\
    for((Class) = utModuleGetFirstClass(module); (Class) != utClassNull; \
        (Class) = utModuleGetNextModuleClass(module, Class))  
#define utEndModuleClass 
#define utModuleGetFirstField(module) ((module)->numFields > 0? utFields + (module)->firstFieldIndex : utFieldNull)
#define utModuleGetNextModuleField(module, field) \
     ((((field) - utFields + 1) >= ((module)->firstFieldIndex + (module)->numFields))?utFieldNull:((field) + 1))
#define utForeachModuleField(module, field) \
     for((field) = utModuleGetFirstField(module); (field) != utFieldNull; \
         (field) = utModuleGetNextModuleField(module, field)) 
#define utEndModuleField 
#define utModuleGetFirstEnum(module) ((module)->numEnums > 0? utEnums + (module)->firstEnumIndex : utEnumNull)
#define utModuleGetNextModuleEnum(module, Enum) \
     ((((Enum) - utEnums + 1) >= ((module)->firstEnumIndex + (module)->numEnums))?utEnumNull:((Enum) + 1))
#define utForeachModuleEnum(module, Enum) \
     for(Enum = utModuleGetFirstEnum(module); Enum != utEnumNull; \
         Enum = utModuleGetNextModuleEnum(module, Enum)) 
#define utEndModuleEnum 

utModule utFindModule(char *prefix);

struct utClassStruct {
    char *name;
    uint16 firstFieldIndex, numFields, numHiddenFields;
    uint16 baseClassIndex, baseModuleIndex;
    uint64 (*constructor)(void);
    void (*destructor)(uint64 objectNumber);
    void *numUsedPtr;
    void *numAllocatedPtr;
    void *firstFreePtr;
    uint16 nextFreeFieldIndex;
    uint8 moduleIndex, referenceSize;
};

extern struct utClassStruct *utClasses;
extern uint16 utAllocatedClasses, utUsedClasses;
#define utClassGetName(theClass) (theClass->name)
#define utClassGetFirstFieldIndex(theClass) (theClass->firstFieldIndex)
#define utClassGetNumFields(theClass) (theClass->numFields)
#define utClassGetNumUsedPtr(theClass) (theClass->numUsedPtr)
#define utClassGetNumAllocatedPtr(theClass) (theClass->numAllocatedPtr)
#define utClassGetFirstFreePtr(theClass) (theClass->firstFreePtr)
#define utClassGetNextFreeFieldIndex(theClass) (theClass->nextFreeFieldIndex)
#define utClassGetNextFreeField(theClass) (utFields + utClassGetModule(theClass)->firstFieldIndex + \
    theClass->nextFreeFieldIndex)
#define utClassGetReferenceSize(theClass) (theClass->referenceSize)
#define utClassGetConstructor(theClass) (theClass->constructor)
#define utClassGetDestructor(theClass) (theClass->destructor)
#define utClassGetModuleIndex(theClass) (theClass->moduleIndex)
#define utClassGetModule(theClass) (utModules + (theClass)->moduleIndex)
#define utClassGetNumHiddenFields(theClass) (theClass->numHiddenFields)
#define utClassGetBaseClassIndex(theClass) (theClass->baseClassIndex)
#define utClassGetBaseModuleIndex(theClass) (theClass->BaseModuleIndex)
#define utClassSetName(theClass, value) (theClass->name = (value))
#define utClassSetFirstFieldIndex(theClass, value) (theClass->firstFieldIndex = (value))
#define utClassSetNumFields(theClass, value) (theClass->numFields = (value))
#define utClassSetNumUsedPtr(theClass, value) (theClass->numUsedPtr = (value))
#define utClassSetNumAllocatedPtr(theClass, value) (theClass->numAllocatedPtr = (value))
#define utClassSetFirstFreePtr(theClass, value) (theClass->firstFreePtr = (value))
#define utClassSetNextFreeFieldIndex(theClass, value) (theClass->nextFreeFieldIndex = (value))
#define utClassSetReferenceSize(theClass, value) (theClass->referenceSize = (value))
#define utClassSetConstructor(theClass, value) (theClass->constructor = (value))
#define utClassSetDestructor(theClass, value) (theClass->destructor = (value))
#define utClassSetModuleIndex(theClass, value) (theClass->moduleIndex = (value))
#define utClassSetNumHiddenFields(theClass, value) (theClass->numHiddenFields = (value))
#define utClassSetBaseClassIndex(theClass, value) (theClass->baseClassIndex = (value))
#define utClassSetBaseModuleIndex(theClass, value) (theClass->baseModuleIndex = (value))
#define utClassGetiField(theClass, xField) (utFields + (theClass)->firstFieldIndex + xField)
#define utClassGetFirstField(Class) ((Class)->numFields > 0? utFields + (Class)->firstFieldIndex : utFieldNull)
#define utClassGetNextClassField(Class, Field) \
    ((((Field) - utFields + 1) >= ((Class)->firstFieldIndex + (Class)->numFields))?utFieldNull:((Field) + 1))
#define utForeachClassField(Class, Field)\
    for((Field) = utClassGetFirstField(Class); (Field) != utFieldNull; \
        (Field) = utClassGetNextClassField(Class, Field))  
#define utEndClassField 

struct utFieldStruct {
    char *name;
    void *arrayPtr;
    uint32 size;
    uint32 length; /* Only for fixed sized arrays */
    utFieldType type;
    char *destName;
    uint32 *numUsedPtr; /* Only for arrays */
    uint32 *numAllocatedPtr; /* Only for arrays */
    void *(*getValues)(uint64 objectNumber, uint32 *numValues);
    void *(*allocValues)(uint64 objectNumber, uint32 numValues);
    void (*compactArray)(void);
    uint16 classIndex;
    uint16 unionIndex; /* Only for unions */
    bool array;
    bool fixedSize;
    bool hidden; /* Only true for fields tracking array properties, or if declared hidden */
};

extern struct utFieldStruct *utFields;
extern uint16 utAllocatedFields, utUsedFields;
#define utFieldSetName(field, value) (field->name = (value))
#define utFieldSetArrayPtr(field, value) (field->arrayPtr = (value))
#define utFieldSetSize(field, value) (field->size = (value))
#define utFieldSetLength(field, value) (field->length = (value))
#define utFieldSetType(field, value) (field->type = (value))
#define utFieldSetClassIndex(field, value) (field->classIndex = (value))
#define utFieldSetDestName(field, value) (field->destName = (value))
#define utFieldSetHidden(field, value) (field->hidden = (value))
#define utFieldSetArray(field, value) (field->array = (value))
#define utFieldSetFixedSize(field, value) (field->fixedSize = (value))
#define utFieldSetNumUsedPtr(field, value) (field->numUsedPtr = (value))
#define utFieldSetNumAllocatedPtr(field, value) (field->numAllocatedPtr = (value))
#define utFieldSetGetValues(field, value) (field->getValues = (value))
#define utFieldSetAllocValues(field, value) (field->allocValues = (value))
#define utFieldSetCompactArray(field, value) (field->compactArray = (value))
#define utFieldSetUnionIndex(field, value) (field->unionIndex = (value))
#define utFieldGetName(field) (field->name)
#define utFieldGetArrayPtr(field) (field->arrayPtr)
#define utFieldGetSize(field) (field->size)
#define utFieldGetLength(field) (field->length)
#define utFieldGetType(field) (field->type)
#define utFieldGetClassIndex(field) (field->classIndex)
#define utFieldGetDestName(field) (field->destName)
#define utFieldHidden(field) (field->hidden)
#define utFieldArray(field) (field->array)
#define utFieldFixedSize(field) (field->fixedSize)
#define utFieldGetNumUsedPtr(field) (field->numUsedPtr)
#define utFieldGetNumAllocatedPtr(field) (field->numAllocatedPtr)
#define utFieldGetGetValues(field) (field->getValues)
#define utFieldGetAllocValues(field) (field->allocValues)
#define utFieldGetCompactArray(field) (field->compactArray)
#define utFieldGetUnionIndex(field) (field->unionIndex)
utField utFindField(utClass theClass, char *name);
#define utFieldGetUnion(field) (utUnions + (field)->unionIndex)
#define utFieldGetClass(field) (utClasses + (field)->classIndex)

struct utTransactionStruct {
    uint32 position; /* Position in recent changes file, or in utCommandBuffer if not persistent */
    uint32 length;
};

extern struct utTransactionStruct *utTransactions;
extern uint32 utUsedTransactions, utAllocatedTransactions;
#define utTransactionGetPosition(transaction) (transaction->position)
#define utTransactionGetLength(transaction) (transaction->length)
#define utTransactionSetPosition(transaction, value) (transaction->position = (value))
#define utTransactionSetLength(transaction, value) (transaction->length = (value))

struct utEnumStruct {
    uint16 firstEntryIndex, numEntries;
    char *name;
};

extern struct utEnumStruct *utEnums;
extern uint16 utAllocatedEnums, utUsedEnums;
#define utEnumGetName(theEnum) (theEnum->name)
#define utEnumGetFirstEntryIndex(theEnum) (theEnum->firstEntryIndex)
#define utEnumGetNumEntries(theEnum) (theEnum->numEntries)
#define utEnumSetName(theEnum, value) (theEnum->name = (value))
#define utEnumSetFirstEntryIndex(theEnum, value) (theEnum->firstEntryIndex = (value))
#define utEnumSetNumEntries(theEnum, value) (theEnum->numEntries = (value))
#define utEnumGetFirstEntry(Enum) ((Enum)->numEntries > 0? utEntries + (Enum)->firstEntryIndex: utEntryNull)
#define utEnumGetNextEnumEntry(Enum, Entry) \
    ((((Entry) - utEntries + 1) >= ((Enum)->firstEntryIndex + (Enum)->numEntries))?utEntryNull:((Entry) + 1))
#define utForeachEnumEntry(Enum, Entry)\
    for((Entry) = utEnumGetFirstEntry(Enum); (Entry) != utEntryNull; \
        (Entry) = utEnumGetNextEnumEntry(Enum, Entry))  
#define utEndEnumEntry 

struct utEntryStruct {
    char *name;
    uint32 value;
};
#define utEntrySetName(entry, value) (entry->name = (value))
#define utEntrySetValue(entry, value) (entry->value = (value))
#define utEntryGetName(entry) (entry->name)
#define utEntryGetValue(entry) (entry->value)

extern struct utEntryStruct *utEntries;
extern uint16 utAllocatedEntries, utUsedEntries;

struct utUnionStruct {
    uint16 fieldIndex;
    uint16 switchFieldIndex;
    uint16 firstUnioncaseIndex, numUnioncases;
};
#define utUnionGetSwitchField(theUnion) (utFields + (theUnion)->switchFieldIndex)
#define utUnionGetSwitchFieldIndex(theUnion) (theUnion->switchFieldIndex)
#define utUnionGetFieldIndex(theUnion) (theUnion->fieldIndex)
#define utUnionGetFirstUnioncaseIndex(theUnion) (theUnion->firstUnioncaseIndex)
#define utUnionGetNumUnioncases(theUnion) (theUnion->numUnioncases)
#define utUnionSetSwitchFieldIndex(theUnion, value) (theUnion->switchFieldIndex = (value))
#define utUnionSetFieldIndex(theUnion, value) (theUnion->fieldIndex = (value))
#define utUnionSetFirstUnioncaseIndex(theUnion, value) (theUnion->firstUnioncaseIndex = (value))
#define utUnionSetNumUnioncases(theUnion, value) (theUnion->numUnioncases = (value))
#define utUnionGetFirstUnioncase(Union) ((Union)->numUnioncases > 0? utUnioncases + (Union)->firstUnioncaseIndex: \
        utUnioncaseNull)
#define utUnionGetNextUnionUnioncase(Union, Unioncase) \
    ((((Unioncase) - utUnioncases + 1) >= ((Union)->firstUnioncaseIndex + (Union)->numUnioncases))? \
     utUnioncaseNull:((Unioncase) + 1))
#define utForeachUnionUnioncase(Union, Unioncase)\
    for((Unioncase) = utUnionGetFirstUnioncase(Union); (Unioncase) != utUnioncaseNull; \
        (Unioncase) = utUnionGetNextUnionUnioncase(Union, Unioncase))  
#define utEndUnionUnioncase 

extern struct utUnionStruct *utUnions;
extern uint16 utAllocatedUnions, utUsedUnions;

struct utUnioncaseStruct {
    uint32 value;
    utFieldType type;
    uint32 size;
};

extern struct utUnioncaseStruct *utUnioncases;
extern uint16 utAllocatedUnioncases, utUsedUnioncases;
#define utUnioncaseGetValue(Unioncase) (Unioncase->value)
#define utUnioncaseGetSize(Unioncase) (Unioncase->size)
#define utUnioncaseGetType(Unioncase) (Unioncase->type)
#define utUnioncaseSetValue(Unioncase, value) (Unioncase->value = (value))
#define utUnioncaseSetSize(Unioncase, value) (Unioncase->size = (value))
#define utUnioncaseSetType(Unioncase, value) (Unioncase->type = (value))
/* Some utilities */
uint64 utFindIntValue(void *values, uint8 size);
void utSetInteger(uint8 *dest, uint64 value, uint8 width);

/* This keeps us from writing the the unopened changes file before starting persistence */
extern bool utPersistenceInitialized;
extern char *utDatabaseDirectory;
extern bool utUseTextDatabaseFormat;

#endif
