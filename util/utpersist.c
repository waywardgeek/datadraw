/*
 * This file was written by Bill Cox.  It is hereby placed into the public domain.
 */

/*--------------------------------------------------------------------------------------------------
  Functions supporting database persistence.  It would really complicate things if this module
  depended in any way on itself, so it's coded bare-bones in C.
--------------------------------------------------------------------------------------------------*/
#include <stdlib.h>
#include "ddutil.h"
#include "utpersist.h"

/* This buffer reduces calls to fwrite */
/* #define UT_CHANGE_BUFFER_LENGTH 16384 */
/* temp - for debugging */
#define UT_CHANGE_BUFFER_LENGTH 16
static uint8 *utCommandBuffer;
static uint32 utBufferPosition, utBufferSize;
static FILE *utRecentChangesFile;
/* The utBufferStartPosition is relative to the first change in recentChanges if we are
 * persistent, and should be equal to 0 otherwise. */
static uint32 utLastTransactionPosition, utBufferStartPosition;
/* utRedoTransaction is the next transaction that would be redone if we did one.  It's usually
 * NULL, unless we undo one. */
static utTransaction utRedoTransaction;
/* These are used while undoing a transaction, since we have to compute the command sizes */
static uint32 *utTransactionCommands;
static uint32 utTransactionUsedCommands, utTransactionAllocatedCommands;
/* This says whether or not we are in the middle of a transaction */
static bool utTransactionInProgress;
/* These are used while registering classes and fields so we don't have to keep passing it. */
static utModule utCurrentModule;
static utClass utCurrentClass;
/* The size of the database file */
static uint64 utDatabaseSize;
/* When true, keep a copy of the previous database file */
static bool utKeepBackup;

/* The command buffer is only used while applying commands.  It holds one transaction at a time. */
static uint32 utAllocatedCommands;
static uint32 utUsedCommands;
static uint8 *utCommands;

/* This checksum is to verify a transaction has not been corrupted */
static uint32 utChecksum;

/* Globals that keep track of the allocated objects */
struct utModuleStruct *utModules;
uint8 utAllocatedModules, utUsedModules;
struct utClassStruct *utClasses;
uint16 utAllocatedClasses, utUsedClasses;
struct utFieldStruct *utFields;
uint16 utAllocatedFields, utUsedFields;
struct utTransactionStruct *utTransactions;
uint32 utUsedTransactions, utAllocatedTransactions;
struct utFieldStruct *utFields;
uint16 utAllocatedFields, utUsedFields;
struct utEnumStruct *utEnums;
uint16 utAllocatedEnums, utUsedEnums;
struct utEntryStruct *utEntries;
uint16 utAllocatedEntries, utUsedEntries;
struct utUnionStruct *utUnions;
uint16 utAllocatedUnions, utUsedUnions;
struct utUnioncaseStruct *utUnioncases;
uint16 utAllocatedUnioncases, utUsedUnioncases;

/* This keeps us from writing the the unopened changes file before starting persistence */
bool utPersistenceInitialized;
char *utDatabaseDirectory;
bool utUseTextDatabaseFormat;

/*--------------------------------------------------------------------------------------------------
  Open the recentChanges file with the mode.
--------------------------------------------------------------------------------------------------*/
static void openRecentChanges(
    char *mode)
{
    char *fileName = utSprintf("%s%crecentChanges", utDatabaseDirectory, UTDIRSEP);

    utRecentChangesFile = fopen(fileName, mode);
    if(utRecentChangesFile == NULL) {
        utExit("Could not open file %s", fileName);
    }
}

/*--------------------------------------------------------------------------------------------------
  Register a module.
--------------------------------------------------------------------------------------------------*/
uint8 utRegisterModule(
    char *prefix,
    bool persistent,
    uint32 hashValue,
    uint16 numClasses,
    uint16 numFields,
    uint16 numEnums,
    uint16 globalSize,
    void *globalData,
    void (*start)(void),
    void (*stop)(void))
{
    utModule module = utFindModule(prefix);

    if(module != NULL) {
        /* Already registered, so ignore it */
        utCurrentModule = NULL;
        utModuleSetInitialized(module,  true);
        return module - utModules;
    }
    if(utUsedModules == utAllocatedModules) {
        utAllocatedModules += utAllocatedModules >> 1;
        utResizeArray(utModules, utAllocatedModules);
    }
    module = utModules + utUsedModules;
    utModuleSetPrefix(module,  calloc(strlen(prefix) + 1, sizeof(char)));
    strcpy(utModuleGetPrefix(module), prefix);
    utModuleSetPersistent(module, persistent);
    utModuleSetHashValue(module, hashValue);
    utModuleSetGlobalSize(module,  globalSize);
    utModuleSetGlobalData(module, globalData);
    utModuleSetStart(module, start);
    utModuleSetStop(module, stop);
    utModuleSetFirstClassIndex(module, utUsedClasses);
    utModuleSetNumClasses(module, numClasses);
    utModuleSetFirstFieldIndex(module, utUsedFields);
    utModuleSetNumFields(module, numFields);
    utModuleSetFirstEnumIndex(module, utUsedEnums);
    utModuleSetNumEnums(module, numEnums);
    utModuleSetInitialized(module,  true);
    utModuleSetSaved(module,  true);
    utCurrentModule = module;
    return utUsedModules++;
}

/*--------------------------------------------------------------------------------------------------
  Set a module to uninitialized.
--------------------------------------------------------------------------------------------------*/
void utUnregisterModule(
    uint8 moduleID)
{
    utModule module = utModules + moduleID;

    utModuleSetInitialized(module,  false);
}

/*--------------------------------------------------------------------------------------------------
  Register a class.
--------------------------------------------------------------------------------------------------*/
void utRegisterClass(
    char *name,
    uint16 numFields,
    void *numUsedPtr,
    void *numAllocatedPtr,
    void *firstFreePtr,
    uint16 nextFreeFieldIndex,
    uint8 referenceSize,
    uint64 (*constructor)(void),
    void (*destructor)(uint64 objectIndex))
{
    utClass theClass;

    if(utCurrentModule == NULL) {
        return;
    }
    if(utUsedClasses == utAllocatedClasses) {
        utAllocatedClasses += utAllocatedClasses >> 1;
        utResizeArray(utClasses, utAllocatedClasses);
    }
    theClass = utClasses + utUsedClasses++;
    utClassSetName(theClass, calloc(strlen(name) + 1, sizeof(char)));
    strcpy(utClassGetName(theClass), name);
    utClassSetFirstFieldIndex(theClass, utUsedFields);
    utClassSetNumFields(theClass, numFields);
    utClassSetNumUsedPtr(theClass, numUsedPtr);
    utClassSetNumAllocatedPtr(theClass, numAllocatedPtr);
    utClassSetFirstFreePtr(theClass, firstFreePtr);
    utClassSetNextFreeFieldIndex(theClass, nextFreeFieldIndex);
    utClassSetReferenceSize(theClass, referenceSize);
    utClassSetConstructor(theClass, constructor);
    utClassSetDestructor(theClass, destructor);
    utClassSetModuleIndex(theClass, utCurrentModule - utModules);
    utClassSetNumHiddenFields(theClass, 0);
    utClassSetBaseClassIndex(theClass, UINT16_MAX);
    utClassSetBaseModuleIndex(theClass, UINT8_MAX);
    utCurrentClass = theClass;
}

/*--------------------------------------------------------------------------------------------------
  This function tells the manager the base class of a class extension.
--------------------------------------------------------------------------------------------------*/
void utRegisterBaseClass(
    char *baseModulePrefix,
    uint16 baseClassIndex)
{
    utModule baseModule = utFindModule(baseModulePrefix);

    if(baseModule == utModuleNull) {
        utExit("Base module %s not registered", baseModulePrefix);
    }
    utClassSetBaseModuleIndex(utCurrentClass, baseModule - utModules);
    utClassSetBaseClassIndex(utCurrentClass, utModuleGetFirstClassIndex(baseModule) + baseClassIndex);
}

/*--------------------------------------------------------------------------------------------------
  Register a field.
--------------------------------------------------------------------------------------------------*/
void utRegisterField(
    char *name,
    void *arrayPtr,
    uint32 size,
    utFieldType type,
    char *destName)
{
    utField field;

    if(utCurrentModule == NULL) {
        return;
    }
    if(utUsedFields == utAllocatedFields) {
        utAllocatedFields += utAllocatedFields >> 1;
        utResizeArray(utFields, utAllocatedFields);
    }
    field = utFields + utUsedFields++;
    utFieldSetName(field, calloc(strlen(name) + 1, sizeof(char)));
    strcpy(utFieldGetName(field), name);
    utFieldSetArrayPtr(field, arrayPtr);
    utFieldSetSize(field, size);
    utFieldSetType(field, type);
    utFieldSetClassIndex(field, utCurrentClass - utClasses);
    if(destName == NULL) {
        utFieldSetDestName(field, NULL);
    } else {
        utFieldSetDestName(field, calloc(strlen(destName) + 1, sizeof(char)));
        strcpy(utFieldGetDestName(field), destName);
    }
    utFieldSetHidden(field, false);
    utFieldSetArray(field, false);
    utFieldSetNumUsedPtr(field, NULL);
    utFieldSetNumAllocatedPtr(field, NULL);
    utFieldSetGetValues(field, NULL);
    utFieldSetAllocValues(field, NULL);
    utFieldSetUnionIndex(field, 0);
}

/*--------------------------------------------------------------------------------------------------
  Set the previously registered field as hidden.
--------------------------------------------------------------------------------------------------*/
void utSetFieldHidden(void)
{
    utField field = utFields + utUsedFields - 1;

    if(utCurrentModule == NULL) {
        return;
    }
    utFieldSetHidden(field, true);
        utClassSetNumHiddenFields(utCurrentClass, utClassGetNumHiddenFields(utCurrentClass) + 1);
}

/*--------------------------------------------------------------------------------------------------
  Set the field as an array.
--------------------------------------------------------------------------------------------------*/
void utRegisterArray(
    uint32 *numUsedPtr,
    uint32 *numAllocatedPtr,
    void *(*getValues)(uint64 objectNumber, uint32 *numValues),
    void *(*allocValues)(uint64 objectNumber, uint32 numValues),
    void (*compactArray)(void))
{
    utField arrayField = utFields + utUsedFields - 1;

    if(utCurrentModule == NULL) {
        return;
    }
    utFieldSetArray(arrayField, true);
    utFieldSetNumUsedPtr(arrayField, numUsedPtr);
    utFieldSetNumAllocatedPtr(arrayField, numAllocatedPtr);
    utFieldSetGetValues(arrayField, getValues);
    utFieldSetAllocValues(arrayField, allocValues);
    utFieldSetCompactArray(arrayField, compactArray);
}

/*--------------------------------------------------------------------------------------------------
  Set the field as a fixed size array.
--------------------------------------------------------------------------------------------------*/
void utRegisterFixedArray(
    uint32 length,
    void *(*getValues)(uint64 objectNumber, uint32 *numValues))
{
    utField arrayField = utFields + utUsedFields - 1;

    if(utCurrentModule == NULL) {
        return;
    }
    utFieldSetArray(arrayField, true);
    utFieldSetFixedSize(arrayField, true);
    utFieldSetLength(arrayField, length);
    utFieldSetGetValues(arrayField, getValues);
}

/*--------------------------------------------------------------------------------------------------
  Flush recent changes to disk.
--------------------------------------------------------------------------------------------------*/
static void flushRecentChanges(void)
{
    if(utPersistenceInitialized) {
        if(utBufferPosition > 0) {
            fwrite((void *)utCommandBuffer, sizeof(uint8), utBufferPosition, utRecentChangesFile);
        }
        utBufferStartPosition += utBufferPosition;
        utBufferPosition = 0;
    } else if(utBufferPosition == utBufferSize) {
        utBufferSize += utBufferSize >> 1;
        utResizeArray(utCommandBuffer, utBufferSize);
    }
}

/*--------------------------------------------------------------------------------------------------
  Write a byte to the recentChanges buffer.
--------------------------------------------------------------------------------------------------*/
static void writeUint8(
    uint8 value)
{
    if(utBufferPosition == utBufferSize) {
        flushRecentChanges();
    }
    utAssert(utTransactionInProgress || utChecksum == 0);
    if(utRedoTransaction != NULL) {
        utBufferPosition = utTransactionGetPosition(utRedoTransaction) - utBufferStartPosition;
        utLastTransactionPosition = utBufferStartPosition + utBufferPosition;
        utUsedTransactions = utRedoTransaction - utTransactions;
        utRedoTransaction = NULL;
    }
    utCommandBuffer[utBufferPosition++] = value;
    utTransactionInProgress = true;
    utChecksum = (utChecksum ^ value)*1103515245 + 12345;
}

/*--------------------------------------------------------------------------------------------------
  Write a uint16 to the recentChanges buffer.
--------------------------------------------------------------------------------------------------*/
static void writeUint16(
    uint16 value)
{
    uint8 *values = (uint8 *)(void *)&value;

    writeUint8(*values++);
    writeUint8(*values);
}

/*--------------------------------------------------------------------------------------------------
  Write a uint32 to the recentChanges buffer.
--------------------------------------------------------------------------------------------------*/
static void writeUint32(
    uint32 value)
{
    uint8 *values = (uint8 *)(void *)&value;

    writeUint8(*values++);
    writeUint8(*values++);
    writeUint8(*values++);
    writeUint8(*values);
}

/*--------------------------------------------------------------------------------------------------
  Write a uint64 to the recentChanges buffer.
--------------------------------------------------------------------------------------------------*/
static void writeUint64(
    uint64 value)
{
    uint8 *values = (uint8 *)(void *)&value;

    writeUint8(*values++);
    writeUint8(*values++);
    writeUint8(*values++);
    writeUint8(*values++);
    writeUint8(*values++);
    writeUint8(*values++);
    writeUint8(*values++);
    writeUint8(*values);
}

/*--------------------------------------------------------------------------------------------------
  Write a number of bytes to the recentChanges buffer.
--------------------------------------------------------------------------------------------------*/
static void writeValues(
    uint8 *values,
    uint32 numBytes)
{
    while(numBytes-- != 0) {
        writeUint8(*values++);
    }
}

/*--------------------------------------------------------------------------------------------------
  Create a new transaction object to track a group of commands associated with a transaction.
--------------------------------------------------------------------------------------------------*/
static utTransaction utTransactionCreate(
    uint32 position,
    uint32 length)
{
    utTransaction transaction;

    if(length == 0) {
        return utTransactionNull;
    }
    if(utPersistenceInitialized && utBufferStartPosition == 0) {
        utAssert(utCommandBuffer[position] != 0);
    }
    if(utUsedTransactions == utAllocatedTransactions) {
        utAllocatedTransactions += utAllocatedTransactions >> 1;
        utResizeArray(utTransactions, utAllocatedTransactions);
    }
    transaction = utTransactions + utUsedTransactions++;
        utTransactionSetPosition(transaction, position);
        utTransactionSetLength(transaction, length);
    utTransactionInProgress = false;
    return transaction;
}

/*--------------------------------------------------------------------------------------------------
  Empty the command buffer.
--------------------------------------------------------------------------------------------------*/
static void resetCommandBuffer(void)
{
    utBufferPosition = 0;
    utLastTransactionPosition = 0;
    utBufferStartPosition = 0;
    utChecksum = 0;
    utTransactionInProgress = false;
    utUsedTransactions = 0;
    utRedoTransaction = NULL;
}

/*--------------------------------------------------------------------------------------------------
  Resize all our arrays so that numAllocated == numUsed.  This is needed when saving an ASCII
  database, since next time we load it, we don't know how much extra will be allocated for each
  class.
--------------------------------------------------------------------------------------------------*/
static void shrinkMemoryToFit(void)
{
    utModule module;
    utField field;
    utClass theClass;
    uint64 numUsed;

    utForeachModule(module) {
        if(utModuleInitialized(module)) {
            utForeachModuleClass(module, theClass) {
                numUsed = utFindIntValue(utClassGetNumUsedPtr(theClass), utClassGetReferenceSize(theClass));
                numUsed = utMax(numUsed, 2);
                utSetInteger(utClassGetNumAllocatedPtr(theClass), numUsed, utClassGetReferenceSize(theClass));
            } utEndModuleClass;
            utForeachModuleField(module, field) {
                if(utFieldArray(field) && !utFieldFixedSize(field)) {
                    (utFieldGetCompactArray(field))();
                    numUsed = utMax(*(utFieldGetNumUsedPtr(field)), 2);
                    *utFieldGetNumAllocatedPtr(field) = (uint32)numUsed;
                    *(uint8 **)(utFieldGetArrayPtr(field)) = utRealloc(*(uint8 **)utFieldGetArrayPtr(field), numUsed,
                        utFieldGetSize(field));
                } else {
                    theClass = utClasses + utFieldGetClassIndex(field);
                    numUsed = utFindIntValue(utClassGetNumAllocatedPtr(theClass), utClassGetReferenceSize(theClass));
                    if(!utFieldFixedSize(field)) {
                        *(uint8 **)(utFieldGetArrayPtr(field)) = utRealloc(*(uint8 **)utFieldGetArrayPtr(field), numUsed,
                            utFieldGetSize(field));
                    } else {
                        *(uint8 **)(utFieldGetArrayPtr(field)) = utRealloc(*(uint8 **)utFieldGetArrayPtr(field),
                            numUsed*utFieldGetLength(field), utFieldGetSize(field));
                    }
               }
            } utEndModuleField;
        }
    } utEndModule;
}

/*--------------------------------------------------------------------------------------------------
  Compact the database, and truncate recentChanges.
--------------------------------------------------------------------------------------------------*/
void utCompactDatabase(void)
{
    char *fileName, *backupFileName;

    if(utKeepBackup) {
        fileName = utSprintf("%s%cdatabase", utDatabaseDirectory, UTDIRSEP);
        backupFileName = utSprintf("%s.old", fileName);
        rename(fileName, backupFileName);
    }
    shrinkMemoryToFit();
    if(utUseTextDatabaseFormat) {
        utSaveTextDatabase(NULL);
    } else {
        utSaveBinaryDatabase(NULL);
    }
    utDatabaseSize = utFindFileSize(utSprintf("%s%cdatabase", utDatabaseDirectory, UTDIRSEP));
    resetCommandBuffer();
    fclose(utRecentChangesFile);
    openRecentChanges("wb");
}

/*--------------------------------------------------------------------------------------------------
  Mark the transaction complete, and optionally fush to disk.
--------------------------------------------------------------------------------------------------*/
void utTransactionComplete(
    bool flushToDisk)
{
    uint32 checksum = utChecksum;
    uint32 length;

    writeUint8(UT_TRANSACTION_COMPLETE);
    writeUint32(checksum);
    utChecksum = 0;
    length = utBufferStartPosition + utBufferPosition - utLastTransactionPosition;
    utTransactionCreate(utLastTransactionPosition, length);
    utLastTransactionPosition += length;
    if(utPersistenceInitialized) {
        if(flushToDisk) {
            flushRecentChanges();
            fflush(utRecentChangesFile);
        }
#ifndef UT_USE_UTDATABASEUP_H
        /* Don't do this if we are a persistent undo/redo database */
        if((utBufferStartPosition << 2) > utDatabaseSize) {
            utCompactDatabase();
        }
#endif
    }
}

/*--------------------------------------------------------------------------------------------------
  Register an enumerated type.
--------------------------------------------------------------------------------------------------*/
void utRegisterEnum(
    char *name,
    uint16 numEntries)
{
    utEnum theEnum;

    if(utUsedEnums == utAllocatedEnums) {
        utAllocatedEnums += utAllocatedEnums >> 1;
        utResizeArray(utEnums, utAllocatedEnums);
    }
    theEnum = utEnums + utUsedEnums++;
    utEnumSetName(theEnum, calloc(strlen(name) + 1, sizeof(char)));
    strcpy(utEnumGetName(theEnum), name);
    utEnumSetFirstEntryIndex(theEnum, utUsedEntries);
    utEnumSetNumEntries(theEnum, numEntries);
}

/*--------------------------------------------------------------------------------------------------
  Register an entry in an enumerated type.
--------------------------------------------------------------------------------------------------*/
void utRegisterEntry(
    char *name,
    uint32 value)
{
    utEntry entry;

    if(utUsedEntries == utAllocatedEntries) {
        utAllocatedEntries += utAllocatedEntries >> 1;
        utResizeArray(utEntries, utAllocatedEntries);
    }
    entry = utEntries + utUsedEntries++;
    utEntrySetName(entry, calloc(strlen(name) + 1, sizeof(char)));
    utEntrySetValue(entry, value);
    strcpy(utEntryGetName(entry), name);
}

/*--------------------------------------------------------------------------------------------------
  Register the previously registered field as a union.
--------------------------------------------------------------------------------------------------*/
void utRegisterUnion(
    char *switchFieldName,
    uint16 numCases)
{
    utField unionField = utFields + utUsedFields - 1;
    utUnion theUnion;
    utField switchField = utFindField(utCurrentClass, switchFieldName);

    if(utUsedUnions == utAllocatedUnions) {
        utAllocatedUnions += utAllocatedUnions >> 1;
        utResizeArray(utUnions, utAllocatedUnions);
    }
    utFieldSetUnionIndex(unionField, utUsedUnions);
    theUnion = utUnions + utUsedUnions++;
    utUnionSetSwitchFieldIndex(theUnion, switchField - utFields);
    utUnionSetFieldIndex(theUnion, utUsedFields - 1);
    utUnionSetFirstUnioncaseIndex(theUnion, utUsedUnioncases);
    utUnionSetNumUnioncases(theUnion, numCases);
}

/*--------------------------------------------------------------------------------------------------
  Write the variable sized integer.
--------------------------------------------------------------------------------------------------*/
void utRegisterUnionCase(
    uint32 value,
    utFieldType type,
    uint32 size)
{
    utUnioncase unionCase;

    if(utUsedUnioncases == utAllocatedUnioncases) {
        utAllocatedUnioncases += utAllocatedUnioncases >> 1;
        utResizeArray(utUnioncases, utAllocatedUnioncases);
    }
    unionCase = utUnioncases + utUsedUnioncases++;
    utUnioncaseSetValue(unionCase, value);
    if(type == UT_BIT) {
        type = UT_BOOL; /* We convert bit to bool in unions */
        size = sizeof(bool);
    }
    utUnioncaseSetSize(unionCase, size);
    utUnioncaseSetType(unionCase, type);
}

/*--------------------------------------------------------------------------------------------------
  Write the variable sized integer.
--------------------------------------------------------------------------------------------------*/
static void writeInteger(
    uint64 value,
    uint8 size)
{
    switch(size) {
    case 1: writeUint8((uint8)value); break;
    case 2: writeUint16((uint16)value); break;
    case 4: writeUint32((uint32)value); break;
    case 8: writeUint64(value); break;
    default:
        utExit("Invalid reference size");
    }
}

/*--------------------------------------------------------------------------------------------------
  Record the change to the field.
--------------------------------------------------------------------------------------------------*/
void utRecordField(
    uint8 moduleID,
    uint16 fieldIndex,
    uint64 objectNumber,
    bool undo)
{
    utModule module = utModules + moduleID;
    uint16 fieldPosition = utModuleGetFirstFieldIndex(module) + fieldIndex;
    utField field = utFields + fieldPosition;
    utClass theClass = utClasses + utFieldGetClassIndex(field);
    uint8 *values = *(uint8 **)(utFieldGetArrayPtr(field)) + objectNumber*utFieldGetSize(field);

    writeUint8((uint8)(UT_WRITE_FIELD | (undo? 0x80 : 0)));
    writeUint16(fieldPosition);
    writeInteger(objectNumber, utClassGetReferenceSize(theClass));
    writeValues(values, utFieldGetSize(field));
}

/*--------------------------------------------------------------------------------------------------
  Record the change to the field.
--------------------------------------------------------------------------------------------------*/
void utRecordArray(
    uint8 moduleID,
    uint16 fieldIndex,
    uint32 dataIndex,
    uint32 length,
    bool undo)
{
    utModule module = utModules + moduleID;
    uint16 fieldPosition = utModuleGetFirstFieldIndex(module) + fieldIndex;
    utField field = utFields + fieldPosition;
    uint8 *values = *(uint8 **)(utFieldGetArrayPtr(field)) + dataIndex*utFieldGetSize(field);

    writeUint8((uint8)(UT_WRITE_ARRAY | (undo? 0x80 : 0)));
    writeUint16(fieldPosition);
    writeUint32(dataIndex);
    writeUint32(length);
    writeValues(values, utFieldGetSize(field)*length);
}

/*--------------------------------------------------------------------------------------------------
  Record the change to the global value.
--------------------------------------------------------------------------------------------------*/
void utRecordGlobal(
    uint8 moduleID,
    uint8 numBytes,
    void *location,
    bool undo)
{
    utModule module = utModules + moduleID;
    uint16 offset = ((uint8 *) location) - ((uint8 *) utModuleGetGlobalData(module));

    writeUint8((uint8)(UT_WRITE_GLOBAL | (undo? 0x80 : 0)));
    writeUint8(moduleID);
    writeUint16(offset);
    writeUint8(numBytes);
    writeValues(location, numBytes);
}

/*--------------------------------------------------------------------------------------------------
  Record when a field is resized.
--------------------------------------------------------------------------------------------------*/
void utRecordResize(
    uint8 moduleID,
    uint16 fieldIndex,
    uint64 length,
    bool undo)
{
    utModule module = utModules + moduleID;
    uint16 fieldPosition = utModuleGetFirstFieldIndex(module) + fieldIndex;

    writeUint8((uint8)(UT_RESIZE_FIELD | (undo? 0x80 : 0)));
    writeUint16(fieldPosition);
    writeUint64(length);
}

#define readUint8(buffer) (*(buffer))

/*--------------------------------------------------------------------------------------------------
  Read a uint16 from the command buffer.
--------------------------------------------------------------------------------------------------*/
static uint16 readUint16(
    uint8 *buffer)
{
    return *(uint16 *)(void *)(buffer);
}

/*--------------------------------------------------------------------------------------------------
  Read a uint32 from a buffer.
--------------------------------------------------------------------------------------------------*/
static uint32 readUint32(
    uint8 *buffer)
{
    return *(uint32 *)(void *)(buffer);
}

/*--------------------------------------------------------------------------------------------------
  Read a uint64 from a buffer.
--------------------------------------------------------------------------------------------------*/
static uint64 readUint64(
    uint8 *buffer)
{
    return *(uint64 *)(void *)(buffer);
}

/*--------------------------------------------------------------------------------------------------
  Apply a field value.
  WRITE_FIELD <16-bit field #> <32-bit index> <value>
--------------------------------------------------------------------------------------------------*/
static void applyField(
    uint8 *command)
{
    uint16 xField = readUint16(command);
    utField field = utFields + xField;
    utClass theClass = utClasses + utFieldGetClassIndex(field);
    uint8 *values;
    uint64 objectNumber;

    command += 2;
    objectNumber = utFindIntValue(command, utClassGetReferenceSize(theClass));
    command += utClassGetReferenceSize(theClass);
    values = *(uint8 **)(utFieldGetArrayPtr(field)) + objectNumber*utFieldGetSize(field);
    memcpy(values, command, utFieldGetSize(field));
}

/*--------------------------------------------------------------------------------------------------
  Apply an array of values.
  WRITE_ARRAY <16-bit field #> <32-bit index> <32-bit numValues> <values>
--------------------------------------------------------------------------------------------------*/
static void applyArray(
    uint8 *command)
{
    uint16 xField = readUint16(command);
    utField field = utFields + xField;
    uint8 *values;
    uint32 length, bytes;
    uint32 dataIndex;

    command += 2;
    dataIndex = readUint32(command);
    command += 4;
    length = readUint32(command);
    command += 4;
    bytes = utFieldGetSize(field)*length;
    values = *(uint8 **)(utFieldGetArrayPtr(field)) + dataIndex*utFieldGetSize(field);
    memcpy(values, command, bytes);
}

/*--------------------------------------------------------------------------------------------------
  Apply a global value change.
  WRITE_GLOBAL <8-bit moduleID> <16-bit offset> <8-bit numBytes> <values>
--------------------------------------------------------------------------------------------------*/
static void applyGlobal(
    uint8 *command)
{
    uint8 moduleID = readUint8(command);
    utModule module = utModules + moduleID;
    uint16 offset;
    uint8 numBytes;
    uint8 *values;

    command++;
    offset = readUint16(command);
    command += 2;
    numBytes = readUint8(command);
    command++;
    values = (uint8 *)(utModuleGetGlobalData(module)) + offset;
    memcpy(values, command, numBytes);
}

/*--------------------------------------------------------------------------------------------------
  Apply a resize field command.
  RESIZE_FIELD <16-bit field #> <64-bit size>
--------------------------------------------------------------------------------------------------*/
static void applyResize(
    uint8 *command)
{
    uint16 xField = readUint16(command);
    utField field = utFields + xField;
    uint64 size;

    command += 2;
    size = readUint64(command);
    command += 8;
    *(uint8 **)(utFieldGetArrayPtr(field)) =utRealloc(*(uint8 **)utFieldGetArrayPtr(field), size, utFieldGetSize(field));
}

/*--------------------------------------------------------------------------------------------------
  Find the size of a command.
--------------------------------------------------------------------------------------------------*/
static uint32 findCommandSize(
    uint8 *command)
{
    utClass theClass;
    utField field;
    uint16 xField;

    switch((*command) & 0x7f) {
    case UT_TRANSACTION_COMPLETE:
        /* TRANSACTION_COMPLETE <32-bit checksum> */
        return  5;
    case UT_WRITE_FIELD:
        /* WRITE_FIELD <16-bit field #> <32-bit index> <value> */
        xField = readUint16(command + 1);
        field = utFields + xField;
        theClass = utClasses + utFieldGetClassIndex(field);
        return 3 + utFieldGetSize(field) + utClassGetReferenceSize(theClass);
    case UT_WRITE_ARRAY:
        /* WRITE_ARRAY <16-bit field #> <32-bit index> <32-bit numValues> <values> */
        xField = readUint16(command + 1);
        field = utFields + xField;
        return 11 + utFieldGetSize(field)*readUint32(command + 7);
    case UT_WRITE_GLOBAL:
        /* WRITE_GLOBAL <8-bit moduleID> <16-bit offset> <8-bit numBytes> <values> */
        return 5 + readUint8(command + 4);
    case UT_RESIZE_FIELD:
        /* RESIZE_FIELD <16-bit field #> <64-bit size> */
        return 11;
    default:
        utExit("Invalid command");
    }
    return 0;
}

/*--------------------------------------------------------------------------------------------------
  Apply the command.
--------------------------------------------------------------------------------------------------*/
static void applyCommand(
    uint8 *command)
{
    utIfDebug(3) {
        utDumpCommand(command);
    }
    switch(*command++ & 0x7f) {
    case UT_WRITE_FIELD:
        applyField(command);
        break;
    case UT_WRITE_ARRAY:
        applyArray(command);
        break;
    case UT_WRITE_GLOBAL:
        applyGlobal(command);
        break;
    case UT_RESIZE_FIELD:
        applyResize(command);
        break;
    default:
        utExit("Unknown command in recentChanges file");
    }
}

/*--------------------------------------------------------------------------------------------------
  Apply changes from the commands in the buffer.
--------------------------------------------------------------------------------------------------*/
static void applyCommands(
    uint8 *commands,
    uint32 length)
{
    uint32 xCommand = 0;
    uint32 size;
    
    while(xCommand < length) {
        size = findCommandSize(commands + xCommand);
        if(!(commands[xCommand] & 0x80)) {
            /* Skip undo commands */
            utIfDebug(3) {
                utMemCheckTrace("foo", 1);
            }
            applyCommand(commands + xCommand);
            utIfDebug(3) {
                utMemCheckTrace("foo", 1);
            }
        }
        xCommand += size;
    }
}

/*--------------------------------------------------------------------------------------------------
  Truncate the recentChanges file
--------------------------------------------------------------------------------------------------*/
static void truncateChangesFile(
    uint32 size)
{
    utWarning("recentChanges file damaged... truncating back to last complete transaction.");
    fclose(utRecentChangesFile);
    utTruncateFile(utSprintf("%s%crecentChanges", utDatabaseDirectory, UTDIRSEP), size);
    openRecentChanges("rb");
}

/*--------------------------------------------------------------------------------------------------
  Apply recent changes from the commands in the recent changes buffer.  utBufferStartPosition and
  lastTransactionPosition are used to help truncate the recentChanges file if it is corrupt at the
  end.  Return true if everything is ok, and false if we had to truncate the file.
--------------------------------------------------------------------------------------------------*/
static bool applyCommandsInBuffer(
    bool endOfInput,
    uint32 *checksum,
    uint32 *unparsedCommands)
{
    uint32 xCommand = 0;
    uint32 size;
    bool done = false;

    while(!done && (xCommand + UT_COMMAND_MAX_HEADER_SIZE < utBufferPosition ||
            (endOfInput && xCommand < utBufferPosition))) {
        if(utCommandBuffer[xCommand] == UT_TRANSACTION_COMPLETE) {
            xCommand++;
            if(readUint32(utCommandBuffer + xCommand) != *checksum) {
                truncateChangesFile(utLastTransactionPosition);
                return false;
            }
            xCommand += 4;
            utTransactionCreate(utLastTransactionPosition, utUsedCommands);
            applyCommands(utCommands, utUsedCommands);
            utUsedCommands = 0;
            utLastTransactionPosition = utBufferStartPosition + xCommand;
            *checksum = 0;
        } else {
            size = findCommandSize(utCommandBuffer + xCommand);
            if(size == 0) {
                truncateChangesFile(utLastTransactionPosition);
                return false;
            }
            if(utUsedCommands + size >= utAllocatedCommands) {
                utAllocatedCommands += (utAllocatedCommands >> 1) + size;
                utResizeArray(utCommands, utAllocatedCommands);
            }
            if(xCommand + size <= utBufferPosition) {
                while(size-- != 0) {
                    *checksum = (*checksum ^ utCommandBuffer[xCommand])*1103515245 + 12345;
                    utCommands[utUsedCommands++] = utCommandBuffer[xCommand++];
                }
            } else {
                done = true;
            }
        }
    }
    *unparsedCommands = utBufferPosition - xCommand;
    if(*unparsedCommands > 0) {
        memmove(utCommandBuffer, utCommandBuffer + xCommand, *unparsedCommands);
    }
    return true;
}

/*--------------------------------------------------------------------------------------------------
  Apply recent changes to the database.  The changes will have been recorded in recentChanges.
--------------------------------------------------------------------------------------------------*/
static void applyRecentChanges(void)
{
    uint32 checksum = 0;
    uint32 unparsedCommands = 0;

    utAssert(utChecksum == 0);
    utLastTransactionPosition = 0;
    utBufferStartPosition = 0;
    utUsedCommands = 0;
    fclose(utRecentChangesFile);
    openRecentChanges("rb");
    utDo {
        if(unparsedCommands + UT_CHANGE_BUFFER_LENGTH > utBufferSize) {
            utBufferSize = unparsedCommands + UT_CHANGE_BUFFER_LENGTH;
            utResizeArray(utCommandBuffer, utBufferSize);
        }
        utBufferPosition = fread(utCommandBuffer + unparsedCommands, sizeof(uint8),
            UT_CHANGE_BUFFER_LENGTH, utRecentChangesFile) + unparsedCommands;
    } utWhile(utBufferPosition > 0 &&
            applyCommandsInBuffer((bool)(utBufferPosition < UT_CHANGE_BUFFER_LENGTH + unparsedCommands),
            &checksum, &unparsedCommands)) {
        utBufferStartPosition += utBufferPosition - unparsedCommands;
    } utRepeat;
    fclose(utRecentChangesFile);
    openRecentChanges("ab");
    utBufferPosition = 0;
    utAssert(utChecksum == 0);
}

/*--------------------------------------------------------------------------------------------------
  Load the module from the file.
--------------------------------------------------------------------------------------------------*/
static bool loadModule(
    utModule module,
    FILE *file)
{
    utClass theClass;
    utField field;
    uint8 *values;
    uint64 numUsed, numAllocated;

    utModuleGetStop(module)();
    if(fread(utModuleGetGlobalData(module), utModuleGetGlobalSize(module), 1, file) != 1) {
        utWarning("File too short, and does not contain header info");
        return false;
    }
    if(utModuleGetHashValue(module) != *(uint32 *)(utModuleGetGlobalData(module))) {
        utWarning("Incompatible database format");
        return false;
    }
    utForeachModuleField(module, field) {
        theClass = utClasses + utFieldGetClassIndex(field);
        if(utFieldArray(field)) {
            numUsed = *(utFieldGetNumUsedPtr(field));
            numAllocated =  *(utFieldGetNumAllocatedPtr(field));
        } else {
            numUsed = utFindIntValue(utClassGetNumUsedPtr(theClass), utClassGetReferenceSize(theClass));
            numAllocated = utFindIntValue(utClassGetNumAllocatedPtr(theClass), utClassGetReferenceSize(theClass));
        }
        values = utCalloc(numAllocated, utFieldGetSize(field));
        *(uint8 **)(utFieldGetArrayPtr(field)) = values;
        if(fread(values, utFieldGetSize(field), numUsed, file) != numUsed) {
            utWarning("Unable to read from file");
            return false;
        }
    } utEndModuleField;
    return true;
}

/*--------------------------------------------------------------------------------------------------
  Load the database from memory.
--------------------------------------------------------------------------------------------------*/
void utLoadBinaryDatabase(
    FILE *databaseFile)
{
    char *fileName;
    utModule module;
    bool useDefault = databaseFile == NULL;

    if(useDefault) {
        fileName = utSprintf("%s%cdatabase", utDatabaseDirectory, UTDIRSEP);
        databaseFile = fopen(fileName, "rb");
        if(databaseFile == NULL) {
            utExit("Could not read from %s", fileName);
        }
    }
    utForeachModule(module) {
        if(utModuleInitialized(module) && !loadModule(module, databaseFile)) {
            utExit("Could not read from %s module", utModuleGetPrefix(module));
        }
    } utEndModule;
    if(useDefault) {
        fclose(databaseFile);
    }
}

/*--------------------------------------------------------------------------------------------------
  Save the module to the file.
--------------------------------------------------------------------------------------------------*/
static bool saveModule(
    utModule module,
    FILE *file)
{
    utClass theClass;
    utField field;
    uint8 *values;
    uint64 numUsed;

    if(fwrite(utModuleGetGlobalData(module), utModuleGetGlobalSize(module), 1, file) != 1) {
        utWarning("Unable to write to file");
        return false;
    }
    utForeachModuleField(module, field) {
        theClass = utClasses + utFieldGetClassIndex(field);
        values = *(uint8 **)(utFieldGetArrayPtr(field));
        if(utFieldArray(field)) {
            numUsed = *(utFieldGetNumUsedPtr(field));
        } else {
            numUsed = utFindIntValue(utClassGetNumUsedPtr(theClass), utClassGetReferenceSize(theClass));
        }
        if(fwrite(values, utFieldGetSize(field), numUsed, file) != numUsed) {
            utWarning("Unable to write to file");
            return false;
        }
    } utEndModuleField;
    return true;
}

/*--------------------------------------------------------------------------------------------------
  Save the database to disk.
--------------------------------------------------------------------------------------------------*/
void utSaveBinaryDatabase(
    FILE *databaseFile)
{
    char *fileName = NULL;
    utModule module;
    bool useDefault = databaseFile == NULL;

    if(useDefault) {
        fileName = utSprintf("%s%cdatabase", utDatabaseDirectory, UTDIRSEP);
        databaseFile = fopen(fileName, "wb");
    }
    if(databaseFile == NULL) {
        utExit("Could not write to %s", fileName);
    }
    utForeachModule(module) {
        if(utModuleInitialized(module) && utModuleSaved(module) && !saveModule(module, databaseFile)) {
            utExit("Could not save database");
        }
    } utEndModule;
    if(useDefault) {
        fclose(databaseFile);
    }
}

/*--------------------------------------------------------------------------------------------------
  Find the oldest transaction indicated by numChanges, and load it into the database.
--------------------------------------------------------------------------------------------------*/
static void loadChangesIntoCommandBuffer(
    uint32 xTransaction)
{
    utTransaction transaction = utTransactions + xTransaction;
    uint32 filePosition, bufferSize;
    
    if(utTransactionGetPosition(transaction) >= utBufferStartPosition) {
        return; /* Already loaded */
    }
    flushRecentChanges();
    if(utBufferStartPosition >= UT_CHANGE_BUFFER_LENGTH) {
        filePosition = utBufferStartPosition - UT_CHANGE_BUFFER_LENGTH;
    } else {
        filePosition = 0;
    }
    filePosition = utMin(filePosition, utTransactionGetPosition(transaction));
    bufferSize = utBufferStartPosition + utBufferPosition - filePosition;
    if(bufferSize > utBufferSize) {
        utBufferSize = bufferSize + (bufferSize >> 1);
        utResizeArray(utCommandBuffer, utBufferSize);
    }
    fclose(utRecentChangesFile);
    openRecentChanges("rb");
    fseek(utRecentChangesFile, filePosition, SEEK_SET);
    utBufferPosition = fread(utCommandBuffer, sizeof(uint8), bufferSize, utRecentChangesFile);
    utAssert(utBufferPosition == bufferSize);
    utBufferStartPosition = filePosition;
    fclose(utRecentChangesFile);
    utTruncateFile(utSprintf("%s%crecentChanges", utDatabaseDirectory, UTDIRSEP), filePosition);
    openRecentChanges("ab");
}

/*--------------------------------------------------------------------------------------------------
  Compute the transaction command positions.
--------------------------------------------------------------------------------------------------*/
static void computeTransactionCommandPositions(
    utTransaction transaction)
{
    uint8 *commands = utCommandBuffer + utTransactionGetPosition(transaction) - utBufferStartPosition;
    uint32 xCommand = 0;
    uint32 size;

    utTransactionUsedCommands = 0;
    while(xCommand < utTransactionGetLength(transaction)) {
        size = findCommandSize(commands + xCommand);
        if(utTransactionUsedCommands == utTransactionAllocatedCommands) {
            utTransactionAllocatedCommands += utTransactionAllocatedCommands >> 1;
            utResizeArray(utTransactionCommands, utTransactionAllocatedCommands);
        }
        utTransactionCommands[utTransactionUsedCommands++] = xCommand + utTransactionGetPosition(transaction) -
            utBufferStartPosition;
        xCommand += size;
    }
}

/*--------------------------------------------------------------------------------------------------
  Undo the transaction.
--------------------------------------------------------------------------------------------------*/
static void undoTransaction(
    utTransaction transaction)
{
    uint8 *command;
    uint32 xCommand;

    computeTransactionCommandPositions(transaction);
    xCommand = utTransactionUsedCommands;
    while(xCommand-- != 0) {
        command = utCommandBuffer + utTransactionCommands[xCommand];
        if(*command & 0x80) {
            applyCommand(command);
        }
    }
}

/*--------------------------------------------------------------------------------------------------
  Undo the last transaction, by scanning backwards through the recent changes buffer, executing
  undo commands.
--------------------------------------------------------------------------------------------------*/
uint32 utUndo(
    uint32 numChanges)
{
    utTransaction transaction = NULL;
    uint32 xTransaction;
    uint32 firstTransaction, lastTransaction;

    if(utTransactionInProgress) {
        utExit("The current transaction has not yet been completed... can't undo!");
    }
    utAssert(utChecksum == 0);
    if(utRedoTransaction == NULL) {
        firstTransaction = utUsedTransactions;
    } else {
        firstTransaction = utRedoTransaction - utTransactions;
    }
    lastTransaction = firstTransaction >= numChanges? firstTransaction - numChanges : 0;
    if(firstTransaction == lastTransaction) {
        /* Nothing to do */
        return 0;
    }
    if(utPersistenceInitialized) {
        loadChangesIntoCommandBuffer(lastTransaction);
    }
    for(xTransaction = firstTransaction; xTransaction > lastTransaction; xTransaction--) {
        transaction = utTransactions + xTransaction - 1;
        undoTransaction(transaction);
    }
    utRedoTransaction = transaction;
    utAssert(utChecksum == 0);
    return firstTransaction - lastTransaction;
}

/*--------------------------------------------------------------------------------------------------
  Redo the transaction.
--------------------------------------------------------------------------------------------------*/
static void redoTransaction(
    utTransaction transaction)
{
    uint8 *command = utCommandBuffer + utTransactionGetPosition(transaction) - utBufferStartPosition;
    uint32 length = utTransactionGetLength(transaction) - 5; /* Subract out the transaction-complete command */

    applyCommands(command, length);
}

/*--------------------------------------------------------------------------------------------------
  Redo the last transaction, by scanning forwards through the recent changes buffer, executing
  non-undo commands.
--------------------------------------------------------------------------------------------------*/
uint32 utRedo(
    uint32 numChanges)
{
    utTransaction transaction;
    uint32 xTransaction;
    uint32 firstTransaction, lastTransaction;

    if(utRedoTransaction == NULL) {
        return 0;
    }
    utAssert(utChecksum == 0);
    firstTransaction = utRedoTransaction - utTransactions;
    lastTransaction = utMin(firstTransaction + numChanges, utUsedTransactions);
    for(xTransaction = firstTransaction; xTransaction < lastTransaction; xTransaction++) {
        transaction = utTransactions + xTransaction;
        redoTransaction(transaction);
    }
    if(lastTransaction == utUsedTransactions) {
        utRedoTransaction = NULL;
    } else {
        utRedoTransaction = utTransactions + lastTransaction;
    }
    utAssert(utChecksum == 0);
    return firstTransaction - lastTransaction;
}

/*--------------------------------------------------------------------------------------------------
  Load the persistent database into memory if it exists.  Return true if it does, otherwise false.
  If useTextDatabaseFormat is true, we'll keep the database in text instead of binary.  If
  keepBackup is true, we'll rename the old database file before writing the new one.
--------------------------------------------------------------------------------------------------*/
bool utStartPersistence(
    char *directory,
    bool useTextDatabaseFormat,
    bool keepBackup)
{
    char *fileName;

    if(!utAccess(directory, "w")) {
        utExit("Cannot write to directory %s\n", directory);
    }
    utDatabaseDirectory = utAllocString(directory);
    utUseTextDatabaseFormat = useTextDatabaseFormat;
    utKeepBackup = keepBackup;
    resetCommandBuffer();
    utTransactionInProgress = false;
    fileName = utSprintf("%s%cdatabase", directory, UTDIRSEP);
    if(utFileExists(fileName)) {
        utDatabaseSize = utFindFileSize(fileName);
        if(useTextDatabaseFormat) {
            utLoadTextDatabase(NULL);
        } else {
            utLoadBinaryDatabase(NULL);
        }
        resetCommandBuffer(); /* Loading a text database can issue record commands */
        openRecentChanges("rb");
        applyRecentChanges();
        utPersistenceInitialized = true;
        return true;
    }
    if(useTextDatabaseFormat) {
        utSaveTextDatabase(NULL);
    } else {
        utSaveBinaryDatabase(NULL);
    }
    utDatabaseSize = utFindFileSize(utSprintf("%s%cdatabase", directory, UTDIRSEP));
    openRecentChanges("wb");
    fclose(utRecentChangesFile);
    openRecentChanges("ab");
    utPersistenceInitialized = true;
    return false;
}

/*--------------------------------------------------------------------------------------------------
  Destroy the field.
--------------------------------------------------------------------------------------------------*/
void utFieldDestroy(
    utField field)
{
    free(utFieldGetName(field));
    if(utFieldGetDestName(field) != NULL) {
        free(utFieldGetDestName(field));
    }
}

/*--------------------------------------------------------------------------------------------------
  Destroy the module.
--------------------------------------------------------------------------------------------------*/
void utClassDestroy(
    utClass theClass)
{
    free(utClassGetName(theClass));
}

/*--------------------------------------------------------------------------------------------------
  Destroy the entry.
--------------------------------------------------------------------------------------------------*/
void utEntryDestroy(
    utEntry entry)
{
    free(utEntryGetName(entry));
}

/*--------------------------------------------------------------------------------------------------
  Destroy the enum.
--------------------------------------------------------------------------------------------------*/
void utEnumDestroy(
    utEnum theEnum)
{
    utEntry entry;
    uint16 xEntry;

    for(xEntry = 0; xEntry < utEnumGetNumEntries(theEnum); xEntry++) {
        entry = utEntries + utEnumGetFirstEntryIndex(theEnum) + xEntry;
        utEntryDestroy(entry);
    }
    free(utEnumGetName(theEnum));
}

/*--------------------------------------------------------------------------------------------------
  Destroy the module.
--------------------------------------------------------------------------------------------------*/
void utModuleDestroy(
    utModule module)
{
    utClass theClass, nClass;
    utField field, nField;
    utEnum theEnum, nEnum;

    for(field = utModuleGetFirstField(module); field != utFieldNull; field = nField) {
        nField = utModuleGetNextModuleField(module, field);
        utFieldDestroy(field);
    }
    for(theClass = utModuleGetFirstClass(module); theClass != utClassNull; theClass = nClass) {
        nClass = utModuleGetNextModuleClass(module, theClass);
        utClassDestroy(theClass);
    }
    for(theEnum = utModuleGetFirstEnum(module); theEnum != utEnumNull; theEnum = nEnum) {
        nEnum = utModuleGetNextModuleEnum(module, theEnum);
        utEnumDestroy(theEnum);
    }
    free(utModuleGetPrefix(module));
}

/*--------------------------------------------------------------------------------------------------
  Stop the persistent database.  Flush changes to disk, and close files.
--------------------------------------------------------------------------------------------------*/
void utStopPersistence(void)
{
    flushRecentChanges();
    fclose(utRecentChangesFile);
    if(utRedoTransaction != NULL) {
        utTruncateFile(utSprintf("%s%crecentChanges", utDatabaseDirectory, UTDIRSEP),
            utTransactionGetPosition(utRedoTransaction));
    }
    utFree(utDatabaseDirectory);
    utPersistenceInitialized = false;
}

/*--------------------------------------------------------------------------------------------------
  Load the persistent database into memory if it exists.  Return true if it does, otherwise false.
--------------------------------------------------------------------------------------------------*/
void utAllocPersistenceObjects(void)
{
    utAllocatedModules = 2;
    utUsedModules = 0;
    utModules = utNewA(struct utModuleStruct, utAllocatedModules);
    utAllocatedClasses = 2;
    utUsedClasses = 0;
    utClasses = utNewA(struct utClassStruct, utAllocatedClasses);
    utAllocatedFields = 2;
    utUsedFields = 0;
    utFields = utNewA(struct utFieldStruct, utAllocatedFields);
    utBufferPosition = 0;
    utBufferSize = UT_CHANGE_BUFFER_LENGTH;
    utCommandBuffer = utNewA(uint8, utBufferSize);
    utAllocatedTransactions = 2;
    utUsedTransactions = 0;
    utTransactions = utNewA(struct utTransactionStruct, utAllocatedTransactions);
    utTransactionAllocatedCommands = 2;
    utTransactionUsedCommands = 0;
    utTransactionCommands = utNewA(uint32, utTransactionAllocatedCommands);
    utAllocatedCommands = 2;
    utUsedCommands = 0;
    utCommands = utNewA(uint8, utAllocatedCommands);
    utAllocatedEnums = 2;
    utUsedEnums = 0;
    utEnums = utNewA(struct utEnumStruct, utAllocatedEnums);
    utAllocatedEntries = 2;
    utUsedEntries = 0;
    utEntries = utNewA(struct utEntryStruct, utAllocatedEntries);
    utAllocatedUnions = 2;
    utUsedUnions = 0;
    utUnions = utNewA(struct utUnionStruct, utAllocatedUnions);
    utAllocatedUnioncases = 2;
    utUsedUnioncases = 0;
    utUnioncases = utNewA(struct utUnioncaseStruct, utAllocatedUnioncases);
    utRedoTransaction = NULL;
}

/*--------------------------------------------------------------------------------------------------
  Stop the persistent database.  Flush changes to disk, and close files.
--------------------------------------------------------------------------------------------------*/
void utFreePersistenceObjects(void)
{
    utModule module;
    uint8 xModule;

    for(xModule = 0; xModule < utUsedModules; xModule++) {
        module = utModules + xModule;
        utModuleDestroy(module);
    }
    utUsedModules = 0;
    utUsedClasses = 0;
    utUsedFields = 0;
    utFree(utModules);
    utFree(utClasses);
    utFree(utFields);
    utFree(utCommandBuffer);
    utFree(utTransactions);
    utFree(utTransactionCommands);
    utFree(utCommands);
    utFree(utEnums);
    utFree(utEntries);
    utFree(utUnions);
    utFree(utUnioncases);
}

/*--------------------------------------------------------------------------------------------------
  Reset the database, so that we can read in a new one.
--------------------------------------------------------------------------------------------------*/
void utResetDatabase(void)
{
    utModule module;
    uint8 xModule;

    for(xModule = 0; xModule < utUsedModules; xModule++) {
        module = utModules + xModule;
        if(utModuleInitialized(module)) {
            utModuleGetStop(module)();
            utModuleGetStart(module)();
        }
    }
    utInitSymTable();
}

/*--------------------------------------------------------------------------------------------------
  Dump a transaction complete command.
  TRANSACTION_COMPLETE <32-bit checksum>
--------------------------------------------------------------------------------------------------*/
static void dumpTransactionComplete(
    uint8 *command)
{
    uint32 checksum = readUint32(command);

    utDebug("transaction complete with checksum 0x%x\n", checksum);
}

/*--------------------------------------------------------------------------------------------------
  Dump a field command.
  WRITE_FIELD <16-bit field #> <32-bit index> <value>
--------------------------------------------------------------------------------------------------*/
static void dumpField(
    uint8 *command)
{
    uint16 xField = readUint16(command);
    utField field = utFields + xField;
    utClass theClass = utClasses + utFieldGetClassIndex(field);
    utModule module = utModules + utClassGetModuleIndex(theClass);
    uint64 objectNumber;

    command += 2;
    objectNumber = utFindIntValue(command, utClassGetReferenceSize(theClass));
    command += utClassGetReferenceSize(theClass);
    utDebug("%s%ss.%s[%llu] = 0x%s\n", utModuleGetPrefix(module), utClassGetName(theClass), utFieldGetName(field), objectNumber,
        utFindHexString(command, utFieldGetSize(field)));
}

/*--------------------------------------------------------------------------------------------------
  Dump an array command.
  WRITE_ARRAY <16-bit field #> <32-bit index> <32-bit numValues> <values>
--------------------------------------------------------------------------------------------------*/
static void dumpArray(
    uint8 *command)
{
    uint16 xField = readUint16(command);
    utField field = utFields + xField;
    utClass theClass = utClasses + utFieldGetClassIndex(field);
    utModule module = utModules + utClassGetModuleIndex(theClass);
    uint32 length;
    uint32 dataIndex;
    uint32 xValue;
    bool firstTime = true;

    command += 2;
    dataIndex = readUint32(command);
    command += 4;
    length = readUint32(command);
    command += 4;
    utDebug("%s%ss.%s[%u] = (", utModuleGetPrefix(module), utClassGetName(theClass), utFieldGetName(field), dataIndex);
    for(xValue = 0; xValue < length; xValue++) {
        if(!firstTime) {
            utDebug(", ");
        }
        firstTime = false;
        utDebug("0x%s", utFindHexString(command, utFieldGetSize(field)));
    }
    utDebug(")\n");
}

/*--------------------------------------------------------------------------------------------------
  Dump a global command.
  WRITE_GLOBAL <8-bit moduleID> <16-bit offset> <8-bit numBytes> <values>
--------------------------------------------------------------------------------------------------*/
static void dumpGlobal(
    uint8 *command)
{
    uint8 moduleID = readUint8(command);
    utModule module = utModules + moduleID;
    uint16 offset;
    uint8 numBytes;

    command++;
    offset = readUint16(command);
    command += 2;
    numBytes = readUint8(command);
    command++;
    utDebug("%sRootData[%u] = 0x%s\n", utModuleGetPrefix(module), offset, utFindHexString(command, numBytes));
}

/*--------------------------------------------------------------------------------------------------
  Dump a resize command.
  RESIZE_FIELD <16-bit field #> <64-bit size>
--------------------------------------------------------------------------------------------------*/
static void dumpResize(
    uint8 *command)
{
    uint16 xField = readUint16(command);
    utField field = utFields + xField;
    utClass theClass = utClasses + utFieldGetClassIndex(field);
    utModule module = utModules + utClassGetModuleIndex(theClass);
    uint64 size;

    command += 2;
    size = readUint64(command);
    command += 8;
    utDebug("%s%ss.%s resized to %llu\n", utModuleGetPrefix(module), utClassGetName(theClass), utFieldGetName(field), size);
}

/*--------------------------------------------------------------------------------------------------
  Dump the command 
--------------------------------------------------------------------------------------------------*/
void utDumpCommand(
    uint8 *command)
{
    if((*command) & 0x80) {
        utDebug("undo ");
    }
    switch(*command++ & 0x7f) {
    case UT_TRANSACTION_COMPLETE:
        dumpTransactionComplete(command);
        break;
    case UT_WRITE_FIELD:
        dumpField(command);
        break;
    case UT_WRITE_ARRAY:
        dumpArray(command);
        break;
    case UT_WRITE_GLOBAL:
        dumpGlobal(command);
        break;
    case UT_RESIZE_FIELD:
        dumpResize(command);
        break;
    default:
        utExit("Unknown command in recentChanges file");
    }
}

/*--------------------------------------------------------------------------------------------------
  Dump the commands in the buffer to the log file.
--------------------------------------------------------------------------------------------------*/
static void dumpCommands(
    uint8 *commands,
    uint32 length)
{
    uint32 xCommand = 0;
    uint32 size;
    uint32 checksum = 0;

    while(xCommand < length) {
        utDebug("%8x %8x ", checksum, xCommand);
        size = findCommandSize(commands + xCommand);
        utDumpCommand(commands + xCommand);
        if(commands[xCommand] == UT_TRANSACTION_COMPLETE) {
            checksum = 0;
        }
        while(size-- != 0) {
            checksum = (checksum ^ commands[xCommand++])*1103515245 + 12345;
        }
    }
}

/*--------------------------------------------------------------------------------------------------
  Dump the recent changes buffer to the log file in text.
--------------------------------------------------------------------------------------------------*/
void utDumpRecentChanges(void)
{
    char *fileName = utSprintf("%s%crecentChanges", utDatabaseDirectory, UTDIRSEP);
    uint32 fileSize = utFindFileSize(fileName);
    uint8 *commands = utNewA(uint8, fileSize);
    uint32 sizeRead;
    
    fclose(utRecentChangesFile);
    openRecentChanges("rb");
    sizeRead = fread(commands, sizeof(uint8), fileSize, utRecentChangesFile);
    if(sizeRead != fileSize) {
	utError("an error occurred while reading file: %s", fileName);
    }
    fclose(utRecentChangesFile);
    dumpCommands(commands, fileSize);
    openRecentChanges("ab");
    utFree(commands);
}

