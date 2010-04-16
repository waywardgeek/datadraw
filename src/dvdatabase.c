/*----------------------------------------------------------------------------------------
  Database dv
----------------------------------------------------------------------------------------*/

#include "dvdatabase.h"

struct dvRootType_ dvRootData;
uint8 dvModuleID;
struct dvRootFields dvRoots;
struct dvModpathFields dvModpaths;
struct dvModuleFields dvModules;
struct dvLinkFields dvLinks;
struct dvSchemaFields dvSchemas;
struct dvEnumFields dvEnums;
struct dvEntryFields dvEntrys;
struct dvTypedefFields dvTypedefs;
struct dvClassFields dvClasss;
struct dvPropertyFields dvPropertys;
struct dvSparsegroupFields dvSparsegroups;
struct dvRelationshipFields dvRelationships;
struct dvKeyFields dvKeys;
struct dvKeypropertyFields dvKeypropertys;
struct dvUnionFields dvUnions;
struct dvCaseFields dvCases;
struct dvCacheFields dvCaches;
struct dvPropidentFields dvPropidents;

/*----------------------------------------------------------------------------------------
  Constructor/Destructor hooks.
----------------------------------------------------------------------------------------*/
dvRootCallbackType dvRootConstructorCallback;
dvModpathCallbackType dvModpathConstructorCallback;
dvModuleCallbackType dvModuleConstructorCallback;
dvLinkCallbackType dvLinkConstructorCallback;
dvSchemaCallbackType dvSchemaConstructorCallback;
dvEnumCallbackType dvEnumConstructorCallback;
dvEntryCallbackType dvEntryConstructorCallback;
dvTypedefCallbackType dvTypedefConstructorCallback;
dvClassCallbackType dvClassConstructorCallback;
dvPropertyCallbackType dvPropertyConstructorCallback;
dvSparsegroupCallbackType dvSparsegroupConstructorCallback;
dvSparsegroupCallbackType dvSparsegroupDestructorCallback;
dvRelationshipCallbackType dvRelationshipConstructorCallback;
dvKeyCallbackType dvKeyConstructorCallback;
dvKeyCallbackType dvKeyDestructorCallback;
dvKeypropertyCallbackType dvKeypropertyConstructorCallback;
dvKeypropertyCallbackType dvKeypropertyDestructorCallback;
dvUnionCallbackType dvUnionConstructorCallback;
dvUnionCallbackType dvUnionDestructorCallback;
dvCaseCallbackType dvCaseConstructorCallback;
dvCaseCallbackType dvCaseDestructorCallback;
dvCacheCallbackType dvCacheConstructorCallback;
dvCacheCallbackType dvCacheDestructorCallback;
dvPropidentCallbackType dvPropidentConstructorCallback;
dvPropidentCallbackType dvPropidentDestructorCallback;

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocRoot(void)
{
    dvRoot Root = dvRootAlloc();

    return dvRoot2Index(Root);
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Root.
----------------------------------------------------------------------------------------*/
static void allocRoots(void)
{
    dvSetAllocatedRoot(2);
    dvSetUsedRoot(1);
    dvRoots.FirstModpath = utNewAInitFirst(dvModpath, (dvAllocatedRoot()));
    dvRoots.LastModpath = utNewAInitFirst(dvModpath, (dvAllocatedRoot()));
    dvRoots.ModpathTableIndex_ = utNewAInitFirst(uint32, (dvAllocatedRoot()));
    dvRoots.NumModpathTable = utNewAInitFirst(uint32, (dvAllocatedRoot()));
    dvSetUsedRootModpathTable(0);
    dvSetAllocatedRootModpathTable(2);
    dvSetFreeRootModpathTable(0);
    dvRoots.ModpathTable = utNewAInitFirst(dvModpath, dvAllocatedRootModpathTable());
    dvRoots.NumModpath = utNewAInitFirst(uint32, (dvAllocatedRoot()));
    dvRoots.FirstModule = utNewAInitFirst(dvModule, (dvAllocatedRoot()));
    dvRoots.LastModule = utNewAInitFirst(dvModule, (dvAllocatedRoot()));
    dvRoots.ModuleTableIndex_ = utNewAInitFirst(uint32, (dvAllocatedRoot()));
    dvRoots.NumModuleTable = utNewAInitFirst(uint32, (dvAllocatedRoot()));
    dvSetUsedRootModuleTable(0);
    dvSetAllocatedRootModuleTable(2);
    dvSetFreeRootModuleTable(0);
    dvRoots.ModuleTable = utNewAInitFirst(dvModule, dvAllocatedRootModuleTable());
    dvRoots.NumModule = utNewAInitFirst(uint32, (dvAllocatedRoot()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Root.
----------------------------------------------------------------------------------------*/
static void reallocRoots(
    uint32 newSize)
{
    utResizeArray(dvRoots.FirstModpath, (newSize));
    utResizeArray(dvRoots.LastModpath, (newSize));
    utResizeArray(dvRoots.ModpathTableIndex_, (newSize));
    utResizeArray(dvRoots.NumModpathTable, (newSize));
    utResizeArray(dvRoots.NumModpath, (newSize));
    utResizeArray(dvRoots.FirstModule, (newSize));
    utResizeArray(dvRoots.LastModule, (newSize));
    utResizeArray(dvRoots.ModuleTableIndex_, (newSize));
    utResizeArray(dvRoots.NumModuleTable, (newSize));
    utResizeArray(dvRoots.NumModule, (newSize));
    dvSetAllocatedRoot(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Roots.
----------------------------------------------------------------------------------------*/
void dvRootAllocMore(void)
{
    reallocRoots((uint32)(dvAllocatedRoot() + (dvAllocatedRoot() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Compact the Root.ModpathTable heap to free memory.
----------------------------------------------------------------------------------------*/
void dvCompactRootModpathTables(void)
{
    uint32 elementSize = sizeof(dvModpath);
    uint32 usedHeaderSize = (sizeof(dvRoot) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvRoot) + sizeof(uint32) + elementSize - 1)/elementSize;
    dvModpath *toPtr = dvRoots.ModpathTable;
    dvModpath *fromPtr = toPtr;
    dvRoot Root;
    uint32 size;

    while(fromPtr < dvRoots.ModpathTable + dvUsedRootModpathTable()) {
        Root = *(dvRoot *)(void *)fromPtr;
        if(Root != dvRootNull) {
            /* Need to move it to toPtr */
            size = utMax(dvRootGetNumModpathTable(Root) + usedHeaderSize, freeHeaderSize);
            memmove((void *)toPtr, (void *)fromPtr, size*elementSize);
            dvRootSetModpathTableIndex_(Root, toPtr - dvRoots.ModpathTable + usedHeaderSize);
            toPtr += size;
        } else {
            /* Just skip it */
            size = *(uint32 *)(void *)(((dvRoot *)(void *)fromPtr) + 1);
        }
        fromPtr += size;
    }
    dvSetUsedRootModpathTable(toPtr - dvRoots.ModpathTable);
    dvSetFreeRootModpathTable(0);
}

/*----------------------------------------------------------------------------------------
  Allocate more memory for the Root.ModpathTable heap.
----------------------------------------------------------------------------------------*/
static void allocMoreRootModpathTables(
    uint32 spaceNeeded)
{
    uint32 freeSpace = dvAllocatedRootModpathTable() - dvUsedRootModpathTable();

    if((dvFreeRootModpathTable() << 2) > dvUsedRootModpathTable()) {
        dvCompactRootModpathTables();
        freeSpace = dvAllocatedRootModpathTable() - dvUsedRootModpathTable();
    }
    if(freeSpace < spaceNeeded) {
        dvSetAllocatedRootModpathTable(dvAllocatedRootModpathTable() + spaceNeeded - freeSpace +
            (dvAllocatedRootModpathTable() >> 1));
        utResizeArray(dvRoots.ModpathTable, dvAllocatedRootModpathTable());
    }
}

/*----------------------------------------------------------------------------------------
  Allocate memory for a new Root.ModpathTable array.
----------------------------------------------------------------------------------------*/
void dvRootAllocModpathTables(
    dvRoot Root,
    uint32 numModpathTables)
{
    uint32 freeSpace = dvAllocatedRootModpathTable() - dvUsedRootModpathTable();
    uint32 elementSize = sizeof(dvModpath);
    uint32 usedHeaderSize = (sizeof(dvRoot) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvRoot) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 spaceNeeded = utMax(numModpathTables + usedHeaderSize, freeHeaderSize);

#if defined(DD_DEBUG)
    utAssert(dvRootGetNumModpathTable(Root) == 0);
#endif
    if(numModpathTables == 0) {
        return;
    }
    if(freeSpace < spaceNeeded) {
        allocMoreRootModpathTables(spaceNeeded);
    }
    dvRootSetModpathTableIndex_(Root, dvUsedRootModpathTable() + usedHeaderSize);
    dvRootSetNumModpathTable(Root, numModpathTables);
    *(dvRoot *)(void *)(dvRoots.ModpathTable + dvUsedRootModpathTable()) = Root;
    {
        uint32 xValue;
        for(xValue = (uint32)(dvRootGetModpathTableIndex_(Root)); xValue < dvRootGetModpathTableIndex_(Root) + numModpathTables; xValue++) {
            dvRoots.ModpathTable[xValue] = dvModpathNull;
        }
    }
    dvSetUsedRootModpathTable(dvUsedRootModpathTable() + spaceNeeded);
}

/*----------------------------------------------------------------------------------------
  Wrapper around dvRootGetModpathTables for the database manager.
----------------------------------------------------------------------------------------*/
static void *getRootModpathTables(
    uint64 objectNumber,
    uint32 *numValues)
{
    dvRoot Root = dvIndex2Root((uint32)objectNumber);

    *numValues = dvRootGetNumModpathTable(Root);
    return dvRootGetModpathTables(Root);
}

/*----------------------------------------------------------------------------------------
  Wrapper around dvRootAllocModpathTables for the database manager.
----------------------------------------------------------------------------------------*/
static void *allocRootModpathTables(
    uint64 objectNumber,
    uint32 numValues)
{
    dvRoot Root = dvIndex2Root((uint32)objectNumber);

    dvRootSetModpathTableIndex_(Root, 0);
    dvRootSetNumModpathTable(Root, 0);
    if(numValues == 0) {
        return NULL;
    }
    dvRootAllocModpathTables(Root, numValues);
    return dvRootGetModpathTables(Root);
}

/*----------------------------------------------------------------------------------------
  Free memory used by the Root.ModpathTable array.
----------------------------------------------------------------------------------------*/
void dvRootFreeModpathTables(
    dvRoot Root)
{
    uint32 elementSize = sizeof(dvModpath);
    uint32 usedHeaderSize = (sizeof(dvRoot) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvRoot) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 size = utMax(dvRootGetNumModpathTable(Root) + usedHeaderSize, freeHeaderSize);
    dvModpath *dataPtr = dvRootGetModpathTables(Root) - usedHeaderSize;

    if(dvRootGetNumModpathTable(Root) == 0) {
        return;
    }
    *(dvRoot *)(void *)(dataPtr) = dvRootNull;
    *(uint32 *)(void *)(((dvRoot *)(void *)dataPtr) + 1) = size;
    dvRootSetNumModpathTable(Root, 0);
    dvSetFreeRootModpathTable(dvFreeRootModpathTable() + size);
}

/*----------------------------------------------------------------------------------------
  Resize the Root.ModpathTable array.
----------------------------------------------------------------------------------------*/
void dvRootResizeModpathTables(
    dvRoot Root,
    uint32 numModpathTables)
{
    uint32 freeSpace;
    uint32 elementSize = sizeof(dvModpath);
    uint32 usedHeaderSize = (sizeof(dvRoot) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvRoot) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 newSize = utMax(numModpathTables + usedHeaderSize, freeHeaderSize);
    uint32 oldSize = utMax(dvRootGetNumModpathTable(Root) + usedHeaderSize, freeHeaderSize);
    dvModpath *dataPtr;

    if(numModpathTables == 0) {
        if(dvRootGetNumModpathTable(Root) != 0) {
            dvRootFreeModpathTables(Root);
        }
        return;
    }
    if(dvRootGetNumModpathTable(Root) == 0) {
        dvRootAllocModpathTables(Root, numModpathTables);
        return;
    }
    freeSpace = dvAllocatedRootModpathTable() - dvUsedRootModpathTable();
    if(freeSpace < newSize) {
        allocMoreRootModpathTables(newSize);
    }
    dataPtr = dvRootGetModpathTables(Root) - usedHeaderSize;
    memcpy((void *)(dvRoots.ModpathTable + dvUsedRootModpathTable()), dataPtr,
        elementSize*utMin(oldSize, newSize));
    if(newSize > oldSize) {
        {
            uint32 xValue;
            for(xValue = (uint32)(dvUsedRootModpathTable() + oldSize); xValue < dvUsedRootModpathTable() + oldSize + newSize - oldSize; xValue++) {
                dvRoots.ModpathTable[xValue] = dvModpathNull;
            }
        }
    }
    *(dvRoot *)(void *)dataPtr = dvRootNull;
    *(uint32 *)(void *)(((dvRoot *)(void *)dataPtr) + 1) = oldSize;
    dvSetFreeRootModpathTable(dvFreeRootModpathTable() + oldSize);
    dvRootSetModpathTableIndex_(Root, dvUsedRootModpathTable() + usedHeaderSize);
    dvRootSetNumModpathTable(Root, numModpathTables);
    dvSetUsedRootModpathTable(dvUsedRootModpathTable() + newSize);
}

/*----------------------------------------------------------------------------------------
  Compact the Root.ModuleTable heap to free memory.
----------------------------------------------------------------------------------------*/
void dvCompactRootModuleTables(void)
{
    uint32 elementSize = sizeof(dvModule);
    uint32 usedHeaderSize = (sizeof(dvRoot) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvRoot) + sizeof(uint32) + elementSize - 1)/elementSize;
    dvModule *toPtr = dvRoots.ModuleTable;
    dvModule *fromPtr = toPtr;
    dvRoot Root;
    uint32 size;

    while(fromPtr < dvRoots.ModuleTable + dvUsedRootModuleTable()) {
        Root = *(dvRoot *)(void *)fromPtr;
        if(Root != dvRootNull) {
            /* Need to move it to toPtr */
            size = utMax(dvRootGetNumModuleTable(Root) + usedHeaderSize, freeHeaderSize);
            memmove((void *)toPtr, (void *)fromPtr, size*elementSize);
            dvRootSetModuleTableIndex_(Root, toPtr - dvRoots.ModuleTable + usedHeaderSize);
            toPtr += size;
        } else {
            /* Just skip it */
            size = *(uint32 *)(void *)(((dvRoot *)(void *)fromPtr) + 1);
        }
        fromPtr += size;
    }
    dvSetUsedRootModuleTable(toPtr - dvRoots.ModuleTable);
    dvSetFreeRootModuleTable(0);
}

/*----------------------------------------------------------------------------------------
  Allocate more memory for the Root.ModuleTable heap.
----------------------------------------------------------------------------------------*/
static void allocMoreRootModuleTables(
    uint32 spaceNeeded)
{
    uint32 freeSpace = dvAllocatedRootModuleTable() - dvUsedRootModuleTable();

    if((dvFreeRootModuleTable() << 2) > dvUsedRootModuleTable()) {
        dvCompactRootModuleTables();
        freeSpace = dvAllocatedRootModuleTable() - dvUsedRootModuleTable();
    }
    if(freeSpace < spaceNeeded) {
        dvSetAllocatedRootModuleTable(dvAllocatedRootModuleTable() + spaceNeeded - freeSpace +
            (dvAllocatedRootModuleTable() >> 1));
        utResizeArray(dvRoots.ModuleTable, dvAllocatedRootModuleTable());
    }
}

/*----------------------------------------------------------------------------------------
  Allocate memory for a new Root.ModuleTable array.
----------------------------------------------------------------------------------------*/
void dvRootAllocModuleTables(
    dvRoot Root,
    uint32 numModuleTables)
{
    uint32 freeSpace = dvAllocatedRootModuleTable() - dvUsedRootModuleTable();
    uint32 elementSize = sizeof(dvModule);
    uint32 usedHeaderSize = (sizeof(dvRoot) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvRoot) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 spaceNeeded = utMax(numModuleTables + usedHeaderSize, freeHeaderSize);

#if defined(DD_DEBUG)
    utAssert(dvRootGetNumModuleTable(Root) == 0);
#endif
    if(numModuleTables == 0) {
        return;
    }
    if(freeSpace < spaceNeeded) {
        allocMoreRootModuleTables(spaceNeeded);
    }
    dvRootSetModuleTableIndex_(Root, dvUsedRootModuleTable() + usedHeaderSize);
    dvRootSetNumModuleTable(Root, numModuleTables);
    *(dvRoot *)(void *)(dvRoots.ModuleTable + dvUsedRootModuleTable()) = Root;
    {
        uint32 xValue;
        for(xValue = (uint32)(dvRootGetModuleTableIndex_(Root)); xValue < dvRootGetModuleTableIndex_(Root) + numModuleTables; xValue++) {
            dvRoots.ModuleTable[xValue] = dvModuleNull;
        }
    }
    dvSetUsedRootModuleTable(dvUsedRootModuleTable() + spaceNeeded);
}

/*----------------------------------------------------------------------------------------
  Wrapper around dvRootGetModuleTables for the database manager.
----------------------------------------------------------------------------------------*/
static void *getRootModuleTables(
    uint64 objectNumber,
    uint32 *numValues)
{
    dvRoot Root = dvIndex2Root((uint32)objectNumber);

    *numValues = dvRootGetNumModuleTable(Root);
    return dvRootGetModuleTables(Root);
}

/*----------------------------------------------------------------------------------------
  Wrapper around dvRootAllocModuleTables for the database manager.
----------------------------------------------------------------------------------------*/
static void *allocRootModuleTables(
    uint64 objectNumber,
    uint32 numValues)
{
    dvRoot Root = dvIndex2Root((uint32)objectNumber);

    dvRootSetModuleTableIndex_(Root, 0);
    dvRootSetNumModuleTable(Root, 0);
    if(numValues == 0) {
        return NULL;
    }
    dvRootAllocModuleTables(Root, numValues);
    return dvRootGetModuleTables(Root);
}

/*----------------------------------------------------------------------------------------
  Free memory used by the Root.ModuleTable array.
----------------------------------------------------------------------------------------*/
void dvRootFreeModuleTables(
    dvRoot Root)
{
    uint32 elementSize = sizeof(dvModule);
    uint32 usedHeaderSize = (sizeof(dvRoot) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvRoot) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 size = utMax(dvRootGetNumModuleTable(Root) + usedHeaderSize, freeHeaderSize);
    dvModule *dataPtr = dvRootGetModuleTables(Root) - usedHeaderSize;

    if(dvRootGetNumModuleTable(Root) == 0) {
        return;
    }
    *(dvRoot *)(void *)(dataPtr) = dvRootNull;
    *(uint32 *)(void *)(((dvRoot *)(void *)dataPtr) + 1) = size;
    dvRootSetNumModuleTable(Root, 0);
    dvSetFreeRootModuleTable(dvFreeRootModuleTable() + size);
}

/*----------------------------------------------------------------------------------------
  Resize the Root.ModuleTable array.
----------------------------------------------------------------------------------------*/
void dvRootResizeModuleTables(
    dvRoot Root,
    uint32 numModuleTables)
{
    uint32 freeSpace;
    uint32 elementSize = sizeof(dvModule);
    uint32 usedHeaderSize = (sizeof(dvRoot) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvRoot) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 newSize = utMax(numModuleTables + usedHeaderSize, freeHeaderSize);
    uint32 oldSize = utMax(dvRootGetNumModuleTable(Root) + usedHeaderSize, freeHeaderSize);
    dvModule *dataPtr;

    if(numModuleTables == 0) {
        if(dvRootGetNumModuleTable(Root) != 0) {
            dvRootFreeModuleTables(Root);
        }
        return;
    }
    if(dvRootGetNumModuleTable(Root) == 0) {
        dvRootAllocModuleTables(Root, numModuleTables);
        return;
    }
    freeSpace = dvAllocatedRootModuleTable() - dvUsedRootModuleTable();
    if(freeSpace < newSize) {
        allocMoreRootModuleTables(newSize);
    }
    dataPtr = dvRootGetModuleTables(Root) - usedHeaderSize;
    memcpy((void *)(dvRoots.ModuleTable + dvUsedRootModuleTable()), dataPtr,
        elementSize*utMin(oldSize, newSize));
    if(newSize > oldSize) {
        {
            uint32 xValue;
            for(xValue = (uint32)(dvUsedRootModuleTable() + oldSize); xValue < dvUsedRootModuleTable() + oldSize + newSize - oldSize; xValue++) {
                dvRoots.ModuleTable[xValue] = dvModuleNull;
            }
        }
    }
    *(dvRoot *)(void *)dataPtr = dvRootNull;
    *(uint32 *)(void *)(((dvRoot *)(void *)dataPtr) + 1) = oldSize;
    dvSetFreeRootModuleTable(dvFreeRootModuleTable() + oldSize);
    dvRootSetModuleTableIndex_(Root, dvUsedRootModuleTable() + usedHeaderSize);
    dvRootSetNumModuleTable(Root, numModuleTables);
    dvSetUsedRootModuleTable(dvUsedRootModuleTable() + newSize);
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Root.
----------------------------------------------------------------------------------------*/
void dvRootCopyProps(
    dvRoot oldRoot,
    dvRoot newRoot)
{
}

/*----------------------------------------------------------------------------------------
  Increase the size of the hash table.
----------------------------------------------------------------------------------------*/
static void resizeRootModpathHashTable(
    dvRoot Root)
{
    dvModpath _Modpath, prevModpath, nextModpath;
    uint32 oldNumModpaths = dvRootGetNumModpathTable(Root);
    uint32 newNumModpaths = oldNumModpaths << 1;
    uint32 xModpath, index;

    if(newNumModpaths == 0) {
        newNumModpaths = 2;
        dvRootAllocModpathTables(Root, 2);
    } else {
        dvRootResizeModpathTables(Root, newNumModpaths);
    }
    for(xModpath = 0; xModpath < oldNumModpaths; xModpath++) {
        _Modpath = dvRootGetiModpathTable(Root, xModpath);
        prevModpath = dvModpathNull;
        while(_Modpath != dvModpathNull) {
            nextModpath = dvModpathGetNextTableRootModpath(_Modpath);
            index = (newNumModpaths - 1) & utSymGetHashValue(dvModpathGetSym(_Modpath));
            if(index != xModpath) {
                if(prevModpath == dvModpathNull) {
                    dvRootSetiModpathTable(Root, xModpath, nextModpath);
                } else {
                    dvModpathSetNextTableRootModpath(prevModpath, nextModpath);
                }
                dvModpathSetNextTableRootModpath(_Modpath, dvRootGetiModpathTable(Root, index));
                dvRootSetiModpathTable(Root, index, _Modpath);
            } else {
                prevModpath = _Modpath;
            }
            _Modpath = nextModpath;
        }
    }
}

/*----------------------------------------------------------------------------------------
  Add the Modpath to the Root.  If the table is near full, build a new one twice
  as big, delete the old one, and return the new one.
----------------------------------------------------------------------------------------*/
static void addRootModpathToHashTable(
    dvRoot Root,
    dvModpath _Modpath)
{
    dvModpath nextModpath;
    uint32 index;

    if(dvRootGetNumModpath(Root) >> 1 >= dvRootGetNumModpathTable(Root)) {
        resizeRootModpathHashTable(Root);
    }
    index = (dvRootGetNumModpathTable(Root) - 1) & utSymGetHashValue(dvModpathGetSym(_Modpath));
    nextModpath = dvRootGetiModpathTable(Root, index);
    dvModpathSetNextTableRootModpath(_Modpath, nextModpath);
    dvRootSetiModpathTable(Root, index, _Modpath);
    dvRootSetNumModpath(Root, dvRootGetNumModpath(Root) + 1);
}

/*----------------------------------------------------------------------------------------
  Remove the Modpath from the hash table.
----------------------------------------------------------------------------------------*/
static void removeRootModpathFromHashTable(
    dvRoot Root,
    dvModpath _Modpath)
{
    uint32 index = (dvRootGetNumModpathTable(Root) - 1) & utSymGetHashValue(dvModpathGetSym(_Modpath));
    dvModpath prevModpath, nextModpath;
    
    nextModpath = dvRootGetiModpathTable(Root, index);
    if(nextModpath == _Modpath) {
        dvRootSetiModpathTable(Root, index, dvModpathGetNextTableRootModpath(nextModpath));
    } else {
        do {
            prevModpath = nextModpath;
            nextModpath = dvModpathGetNextTableRootModpath(nextModpath);
        } while(nextModpath != _Modpath);
        dvModpathSetNextTableRootModpath(prevModpath, dvModpathGetNextTableRootModpath(_Modpath));
    }
    dvRootSetNumModpath(Root, dvRootGetNumModpath(Root) - 1);
    dvModpathSetNextTableRootModpath(_Modpath, dvModpathNull);
}

/*----------------------------------------------------------------------------------------
  Find the Modpath from the Root and its hash key.
----------------------------------------------------------------------------------------*/
dvModpath dvRootFindModpath(
    dvRoot Root,
    utSym Sym)
{
    uint32 mask = dvRootGetNumModpathTable(Root) - 1;
    dvModpath _Modpath;

    if(mask + 1 != 0) {
        _Modpath = dvRootGetiModpathTable(Root, utSymGetHashValue(Sym) & mask);
        while(_Modpath != dvModpathNull) {
            if(dvModpathGetSym(_Modpath) == Sym) {
                return _Modpath;
            }
            _Modpath = dvModpathGetNextTableRootModpath(_Modpath);
        }
    }
    return dvModpathNull;
}

/*----------------------------------------------------------------------------------------
  Find the Modpath from the Root and its name.
----------------------------------------------------------------------------------------*/
void dvRootRenameModpath(
    dvRoot Root,
    dvModpath _Modpath,
    utSym sym)
{
    if(dvModpathGetSym(_Modpath) != utSymNull) {
        removeRootModpathFromHashTable(Root, _Modpath);
    }
    dvModpathSetSym(_Modpath, sym);
    if(sym != utSymNull) {
        addRootModpathToHashTable(Root, _Modpath);
    }
}

/*----------------------------------------------------------------------------------------
  Add the Modpath to the head of the list on the Root.
----------------------------------------------------------------------------------------*/
void dvRootInsertModpath(
    dvRoot Root,
    dvModpath _Modpath)
{
#if defined(DD_DEBUG)
    if(Root == dvRootNull) {
        utExit("Non-existent Root");
    }
    if(_Modpath == dvModpathNull) {
        utExit("Non-existent Modpath");
    }
    if(dvModpathGetRoot(_Modpath) != dvRootNull) {
        utExit("Attempting to add Modpath to Root twice");
    }
#endif
    dvModpathSetNextRootModpath(_Modpath, dvRootGetFirstModpath(Root));
    if(dvRootGetFirstModpath(Root) != dvModpathNull) {
        dvModpathSetPrevRootModpath(dvRootGetFirstModpath(Root), _Modpath);
    }
    dvRootSetFirstModpath(Root, _Modpath);
    dvModpathSetPrevRootModpath(_Modpath, dvModpathNull);
    if(dvRootGetLastModpath(Root) == dvModpathNull) {
        dvRootSetLastModpath(Root, _Modpath);
    }
    dvModpathSetRoot(_Modpath, Root);
    if(dvModpathGetSym(_Modpath) != utSymNull) {
        addRootModpathToHashTable(Root, _Modpath);
    }
}

/*----------------------------------------------------------------------------------------
  Add the Modpath to the end of the list on the Root.
----------------------------------------------------------------------------------------*/
void dvRootAppendModpath(
    dvRoot Root,
    dvModpath _Modpath)
{
#if defined(DD_DEBUG)
    if(Root == dvRootNull) {
        utExit("Non-existent Root");
    }
    if(_Modpath == dvModpathNull) {
        utExit("Non-existent Modpath");
    }
    if(dvModpathGetRoot(_Modpath) != dvRootNull) {
        utExit("Attempting to add Modpath to Root twice");
    }
#endif
    dvModpathSetPrevRootModpath(_Modpath, dvRootGetLastModpath(Root));
    if(dvRootGetLastModpath(Root) != dvModpathNull) {
        dvModpathSetNextRootModpath(dvRootGetLastModpath(Root), _Modpath);
    }
    dvRootSetLastModpath(Root, _Modpath);
    dvModpathSetNextRootModpath(_Modpath, dvModpathNull);
    if(dvRootGetFirstModpath(Root) == dvModpathNull) {
        dvRootSetFirstModpath(Root, _Modpath);
    }
    dvModpathSetRoot(_Modpath, Root);
    if(dvModpathGetSym(_Modpath) != utSymNull) {
        addRootModpathToHashTable(Root, _Modpath);
    }
}

/*----------------------------------------------------------------------------------------
  Insert the Modpath to the Root after the previous Modpath.
----------------------------------------------------------------------------------------*/
void dvRootInsertAfterModpath(
    dvRoot Root,
    dvModpath prevModpath,
    dvModpath _Modpath)
{
    dvModpath nextModpath = dvModpathGetNextRootModpath(prevModpath);

#if defined(DD_DEBUG)
    if(Root == dvRootNull) {
        utExit("Non-existent Root");
    }
    if(_Modpath == dvModpathNull) {
        utExit("Non-existent Modpath");
    }
    if(dvModpathGetRoot(_Modpath) != dvRootNull) {
        utExit("Attempting to add Modpath to Root twice");
    }
#endif
    dvModpathSetNextRootModpath(_Modpath, nextModpath);
    dvModpathSetNextRootModpath(prevModpath, _Modpath);
    dvModpathSetPrevRootModpath(_Modpath, prevModpath);
    if(nextModpath != dvModpathNull) {
        dvModpathSetPrevRootModpath(nextModpath, _Modpath);
    }
    if(dvRootGetLastModpath(Root) == prevModpath) {
        dvRootSetLastModpath(Root, _Modpath);
    }
    dvModpathSetRoot(_Modpath, Root);
    if(dvModpathGetSym(_Modpath) != utSymNull) {
        addRootModpathToHashTable(Root, _Modpath);
    }
}

/*----------------------------------------------------------------------------------------
 Remove the Modpath from the Root.
----------------------------------------------------------------------------------------*/
void dvRootRemoveModpath(
    dvRoot Root,
    dvModpath _Modpath)
{
    dvModpath pModpath, nModpath;

#if defined(DD_DEBUG)
    if(_Modpath == dvModpathNull) {
        utExit("Non-existent Modpath");
    }
    if(dvModpathGetRoot(_Modpath) != dvRootNull && dvModpathGetRoot(_Modpath) != Root) {
        utExit("Delete Modpath from non-owning Root");
    }
#endif
    nModpath = dvModpathGetNextRootModpath(_Modpath);
    pModpath = dvModpathGetPrevRootModpath(_Modpath);
    if(pModpath != dvModpathNull) {
        dvModpathSetNextRootModpath(pModpath, nModpath);
    } else if(dvRootGetFirstModpath(Root) == _Modpath) {
        dvRootSetFirstModpath(Root, nModpath);
    }
    if(nModpath != dvModpathNull) {
        dvModpathSetPrevRootModpath(nModpath, pModpath);
    } else if(dvRootGetLastModpath(Root) == _Modpath) {
        dvRootSetLastModpath(Root, pModpath);
    }
    dvModpathSetNextRootModpath(_Modpath, dvModpathNull);
    dvModpathSetPrevRootModpath(_Modpath, dvModpathNull);
    dvModpathSetRoot(_Modpath, dvRootNull);
    if(dvModpathGetSym(_Modpath) != utSymNull) {
        removeRootModpathFromHashTable(Root, _Modpath);
    }
}

/*----------------------------------------------------------------------------------------
  Increase the size of the hash table.
----------------------------------------------------------------------------------------*/
static void resizeRootModuleHashTable(
    dvRoot Root)
{
    dvModule _Module, prevModule, nextModule;
    uint32 oldNumModules = dvRootGetNumModuleTable(Root);
    uint32 newNumModules = oldNumModules << 1;
    uint32 xModule, index;

    if(newNumModules == 0) {
        newNumModules = 2;
        dvRootAllocModuleTables(Root, 2);
    } else {
        dvRootResizeModuleTables(Root, newNumModules);
    }
    for(xModule = 0; xModule < oldNumModules; xModule++) {
        _Module = dvRootGetiModuleTable(Root, xModule);
        prevModule = dvModuleNull;
        while(_Module != dvModuleNull) {
            nextModule = dvModuleGetNextTableRootModule(_Module);
            index = (newNumModules - 1) & utSymGetHashValue(dvModuleGetSym(_Module));
            if(index != xModule) {
                if(prevModule == dvModuleNull) {
                    dvRootSetiModuleTable(Root, xModule, nextModule);
                } else {
                    dvModuleSetNextTableRootModule(prevModule, nextModule);
                }
                dvModuleSetNextTableRootModule(_Module, dvRootGetiModuleTable(Root, index));
                dvRootSetiModuleTable(Root, index, _Module);
            } else {
                prevModule = _Module;
            }
            _Module = nextModule;
        }
    }
}

/*----------------------------------------------------------------------------------------
  Add the Module to the Root.  If the table is near full, build a new one twice
  as big, delete the old one, and return the new one.
----------------------------------------------------------------------------------------*/
static void addRootModuleToHashTable(
    dvRoot Root,
    dvModule _Module)
{
    dvModule nextModule;
    uint32 index;

    if(dvRootGetNumModule(Root) >> 1 >= dvRootGetNumModuleTable(Root)) {
        resizeRootModuleHashTable(Root);
    }
    index = (dvRootGetNumModuleTable(Root) - 1) & utSymGetHashValue(dvModuleGetSym(_Module));
    nextModule = dvRootGetiModuleTable(Root, index);
    dvModuleSetNextTableRootModule(_Module, nextModule);
    dvRootSetiModuleTable(Root, index, _Module);
    dvRootSetNumModule(Root, dvRootGetNumModule(Root) + 1);
}

/*----------------------------------------------------------------------------------------
  Remove the Module from the hash table.
----------------------------------------------------------------------------------------*/
static void removeRootModuleFromHashTable(
    dvRoot Root,
    dvModule _Module)
{
    uint32 index = (dvRootGetNumModuleTable(Root) - 1) & utSymGetHashValue(dvModuleGetSym(_Module));
    dvModule prevModule, nextModule;
    
    nextModule = dvRootGetiModuleTable(Root, index);
    if(nextModule == _Module) {
        dvRootSetiModuleTable(Root, index, dvModuleGetNextTableRootModule(nextModule));
    } else {
        do {
            prevModule = nextModule;
            nextModule = dvModuleGetNextTableRootModule(nextModule);
        } while(nextModule != _Module);
        dvModuleSetNextTableRootModule(prevModule, dvModuleGetNextTableRootModule(_Module));
    }
    dvRootSetNumModule(Root, dvRootGetNumModule(Root) - 1);
    dvModuleSetNextTableRootModule(_Module, dvModuleNull);
}

/*----------------------------------------------------------------------------------------
  Find the Module from the Root and its hash key.
----------------------------------------------------------------------------------------*/
dvModule dvRootFindModule(
    dvRoot Root,
    utSym Sym)
{
    uint32 mask = dvRootGetNumModuleTable(Root) - 1;
    dvModule _Module;

    if(mask + 1 != 0) {
        _Module = dvRootGetiModuleTable(Root, utSymGetHashValue(Sym) & mask);
        while(_Module != dvModuleNull) {
            if(dvModuleGetSym(_Module) == Sym) {
                return _Module;
            }
            _Module = dvModuleGetNextTableRootModule(_Module);
        }
    }
    return dvModuleNull;
}

/*----------------------------------------------------------------------------------------
  Find the Module from the Root and its name.
----------------------------------------------------------------------------------------*/
void dvRootRenameModule(
    dvRoot Root,
    dvModule _Module,
    utSym sym)
{
    if(dvModuleGetSym(_Module) != utSymNull) {
        removeRootModuleFromHashTable(Root, _Module);
    }
    dvModuleSetSym(_Module, sym);
    if(sym != utSymNull) {
        addRootModuleToHashTable(Root, _Module);
    }
}

/*----------------------------------------------------------------------------------------
  Add the Module to the head of the list on the Root.
----------------------------------------------------------------------------------------*/
void dvRootInsertModule(
    dvRoot Root,
    dvModule _Module)
{
#if defined(DD_DEBUG)
    if(Root == dvRootNull) {
        utExit("Non-existent Root");
    }
    if(_Module == dvModuleNull) {
        utExit("Non-existent Module");
    }
#endif
    dvModuleSetNextRootModule(_Module, dvRootGetFirstModule(Root));
    if(dvRootGetFirstModule(Root) != dvModuleNull) {
        dvModuleSetPrevRootModule(dvRootGetFirstModule(Root), _Module);
    }
    dvRootSetFirstModule(Root, _Module);
    dvModuleSetPrevRootModule(_Module, dvModuleNull);
    if(dvRootGetLastModule(Root) == dvModuleNull) {
        dvRootSetLastModule(Root, _Module);
    }
    if(dvModuleGetSym(_Module) != utSymNull) {
        addRootModuleToHashTable(Root, _Module);
    }
}

/*----------------------------------------------------------------------------------------
  Add the Module to the end of the list on the Root.
----------------------------------------------------------------------------------------*/
void dvRootAppendModule(
    dvRoot Root,
    dvModule _Module)
{
#if defined(DD_DEBUG)
    if(Root == dvRootNull) {
        utExit("Non-existent Root");
    }
    if(_Module == dvModuleNull) {
        utExit("Non-existent Module");
    }
#endif
    dvModuleSetPrevRootModule(_Module, dvRootGetLastModule(Root));
    if(dvRootGetLastModule(Root) != dvModuleNull) {
        dvModuleSetNextRootModule(dvRootGetLastModule(Root), _Module);
    }
    dvRootSetLastModule(Root, _Module);
    dvModuleSetNextRootModule(_Module, dvModuleNull);
    if(dvRootGetFirstModule(Root) == dvModuleNull) {
        dvRootSetFirstModule(Root, _Module);
    }
    if(dvModuleGetSym(_Module) != utSymNull) {
        addRootModuleToHashTable(Root, _Module);
    }
}

/*----------------------------------------------------------------------------------------
  Insert the Module to the Root after the previous Module.
----------------------------------------------------------------------------------------*/
void dvRootInsertAfterModule(
    dvRoot Root,
    dvModule prevModule,
    dvModule _Module)
{
    dvModule nextModule = dvModuleGetNextRootModule(prevModule);

#if defined(DD_DEBUG)
    if(Root == dvRootNull) {
        utExit("Non-existent Root");
    }
    if(_Module == dvModuleNull) {
        utExit("Non-existent Module");
    }
#endif
    dvModuleSetNextRootModule(_Module, nextModule);
    dvModuleSetNextRootModule(prevModule, _Module);
    dvModuleSetPrevRootModule(_Module, prevModule);
    if(nextModule != dvModuleNull) {
        dvModuleSetPrevRootModule(nextModule, _Module);
    }
    if(dvRootGetLastModule(Root) == prevModule) {
        dvRootSetLastModule(Root, _Module);
    }
    if(dvModuleGetSym(_Module) != utSymNull) {
        addRootModuleToHashTable(Root, _Module);
    }
}

/*----------------------------------------------------------------------------------------
 Remove the Module from the Root.
----------------------------------------------------------------------------------------*/
void dvRootRemoveModule(
    dvRoot Root,
    dvModule _Module)
{
    dvModule pModule, nModule;

#if defined(DD_DEBUG)
    if(_Module == dvModuleNull) {
        utExit("Non-existent Module");
    }
#endif
    nModule = dvModuleGetNextRootModule(_Module);
    pModule = dvModuleGetPrevRootModule(_Module);
    if(pModule != dvModuleNull) {
        dvModuleSetNextRootModule(pModule, nModule);
    } else if(dvRootGetFirstModule(Root) == _Module) {
        dvRootSetFirstModule(Root, nModule);
    }
    if(nModule != dvModuleNull) {
        dvModuleSetPrevRootModule(nModule, pModule);
    } else if(dvRootGetLastModule(Root) == _Module) {
        dvRootSetLastModule(Root, pModule);
    }
    dvModuleSetNextRootModule(_Module, dvModuleNull);
    dvModuleSetPrevRootModule(_Module, dvModuleNull);
    if(dvModuleGetSym(_Module) != utSymNull) {
        removeRootModuleFromHashTable(Root, _Module);
    }
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void dvShowRoot(
    dvRoot Root)
{
    utDatabaseShowObject("dv", "Root", dvRoot2Index(Root));
}
#endif

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocModpath(void)
{
    dvModpath Modpath = dvModpathAlloc();

    return dvModpath2Index(Modpath);
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Modpath.
----------------------------------------------------------------------------------------*/
static void allocModpaths(void)
{
    dvSetAllocatedModpath(2);
    dvSetUsedModpath(1);
    dvModpaths.Sym = utNewAInitFirst(utSym, (dvAllocatedModpath()));
    dvModpaths.Root = utNewAInitFirst(dvRoot, (dvAllocatedModpath()));
    dvModpaths.NextRootModpath = utNewAInitFirst(dvModpath, (dvAllocatedModpath()));
    dvModpaths.PrevRootModpath = utNewAInitFirst(dvModpath, (dvAllocatedModpath()));
    dvModpaths.NextTableRootModpath = utNewAInitFirst(dvModpath, (dvAllocatedModpath()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Modpath.
----------------------------------------------------------------------------------------*/
static void reallocModpaths(
    uint32 newSize)
{
    utResizeArray(dvModpaths.Sym, (newSize));
    utResizeArray(dvModpaths.Root, (newSize));
    utResizeArray(dvModpaths.NextRootModpath, (newSize));
    utResizeArray(dvModpaths.PrevRootModpath, (newSize));
    utResizeArray(dvModpaths.NextTableRootModpath, (newSize));
    dvSetAllocatedModpath(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Modpaths.
----------------------------------------------------------------------------------------*/
void dvModpathAllocMore(void)
{
    reallocModpaths((uint32)(dvAllocatedModpath() + (dvAllocatedModpath() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Modpath.
----------------------------------------------------------------------------------------*/
void dvModpathCopyProps(
    dvModpath oldModpath,
    dvModpath newModpath)
{
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void dvShowModpath(
    dvModpath Modpath)
{
    utDatabaseShowObject("dv", "Modpath", dvModpath2Index(Modpath));
}
#endif

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocModule(void)
{
    dvModule Module = dvModuleAlloc();

    return dvModule2Index(Module);
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Module.
----------------------------------------------------------------------------------------*/
static void allocModules(void)
{
    dvSetAllocatedModule(2);
    dvSetUsedModule(1);
    dvModules.Sym = utNewAInitFirst(utSym, (dvAllocatedModule()));
    dvModules.PrefixSym = utNewAInitFirst(utSym, (dvAllocatedModule()));
    dvModules.Persistent = utNewAInitFirst(uint8, (dvAllocatedModule() + 7) >> 3);
    dvModules.UndoRedo = utNewAInitFirst(uint8, (dvAllocatedModule() + 7) >> 3);
    dvModules.HasSparseData = utNewAInitFirst(uint8, (dvAllocatedModule() + 7) >> 3);
    dvModules.NumFields = utNewAInitFirst(uint16, (dvAllocatedModule()));
    dvModules.NumClasses = utNewAInitFirst(uint32, (dvAllocatedModule()));
    dvModules.NumEnums = utNewAInitFirst(uint32, (dvAllocatedModule()));
    dvModules.Elaborated = utNewAInitFirst(uint8, (dvAllocatedModule() + 7) >> 3);
    dvModules.NextRootModule = utNewAInitFirst(dvModule, (dvAllocatedModule()));
    dvModules.PrevRootModule = utNewAInitFirst(dvModule, (dvAllocatedModule()));
    dvModules.NextTableRootModule = utNewAInitFirst(dvModule, (dvAllocatedModule()));
    dvModules.FirstClass = utNewAInitFirst(dvClass, (dvAllocatedModule()));
    dvModules.LastClass = utNewAInitFirst(dvClass, (dvAllocatedModule()));
    dvModules.ClassTableIndex_ = utNewAInitFirst(uint32, (dvAllocatedModule()));
    dvModules.NumClassTable = utNewAInitFirst(uint32, (dvAllocatedModule()));
    dvSetUsedModuleClassTable(0);
    dvSetAllocatedModuleClassTable(2);
    dvSetFreeModuleClassTable(0);
    dvModules.ClassTable = utNewAInitFirst(dvClass, dvAllocatedModuleClassTable());
    dvModules.NumClass = utNewAInitFirst(uint32, (dvAllocatedModule()));
    dvModules.FirstEnum = utNewAInitFirst(dvEnum, (dvAllocatedModule()));
    dvModules.LastEnum = utNewAInitFirst(dvEnum, (dvAllocatedModule()));
    dvModules.EnumTableIndex_ = utNewAInitFirst(uint32, (dvAllocatedModule()));
    dvModules.NumEnumTable = utNewAInitFirst(uint32, (dvAllocatedModule()));
    dvSetUsedModuleEnumTable(0);
    dvSetAllocatedModuleEnumTable(2);
    dvSetFreeModuleEnumTable(0);
    dvModules.EnumTable = utNewAInitFirst(dvEnum, dvAllocatedModuleEnumTable());
    dvModules.NumEnum = utNewAInitFirst(uint32, (dvAllocatedModule()));
    dvModules.FirstTypedef = utNewAInitFirst(dvTypedef, (dvAllocatedModule()));
    dvModules.LastTypedef = utNewAInitFirst(dvTypedef, (dvAllocatedModule()));
    dvModules.TypedefTableIndex_ = utNewAInitFirst(uint32, (dvAllocatedModule()));
    dvModules.NumTypedefTable = utNewAInitFirst(uint32, (dvAllocatedModule()));
    dvSetUsedModuleTypedefTable(0);
    dvSetAllocatedModuleTypedefTable(2);
    dvSetFreeModuleTypedefTable(0);
    dvModules.TypedefTable = utNewAInitFirst(dvTypedef, dvAllocatedModuleTypedefTable());
    dvModules.NumTypedef = utNewAInitFirst(uint32, (dvAllocatedModule()));
    dvModules.FirstSchema = utNewAInitFirst(dvSchema, (dvAllocatedModule()));
    dvModules.LastSchema = utNewAInitFirst(dvSchema, (dvAllocatedModule()));
    dvModules.SchemaTableIndex_ = utNewAInitFirst(uint32, (dvAllocatedModule()));
    dvModules.NumSchemaTable = utNewAInitFirst(uint32, (dvAllocatedModule()));
    dvSetUsedModuleSchemaTable(0);
    dvSetAllocatedModuleSchemaTable(2);
    dvSetFreeModuleSchemaTable(0);
    dvModules.SchemaTable = utNewAInitFirst(dvSchema, dvAllocatedModuleSchemaTable());
    dvModules.NumSchema = utNewAInitFirst(uint32, (dvAllocatedModule()));
    dvModules.FirstImportLink = utNewAInitFirst(dvLink, (dvAllocatedModule()));
    dvModules.LastImportLink = utNewAInitFirst(dvLink, (dvAllocatedModule()));
    dvModules.FirstExportLink = utNewAInitFirst(dvLink, (dvAllocatedModule()));
    dvModules.LastExportLink = utNewAInitFirst(dvLink, (dvAllocatedModule()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Module.
----------------------------------------------------------------------------------------*/
static void reallocModules(
    uint32 newSize)
{
    utResizeArray(dvModules.Sym, (newSize));
    utResizeArray(dvModules.PrefixSym, (newSize));
    utResizeArray(dvModules.Persistent, (newSize + 7) >> 3);
    utResizeArray(dvModules.UndoRedo, (newSize + 7) >> 3);
    utResizeArray(dvModules.HasSparseData, (newSize + 7) >> 3);
    utResizeArray(dvModules.NumFields, (newSize));
    utResizeArray(dvModules.NumClasses, (newSize));
    utResizeArray(dvModules.NumEnums, (newSize));
    utResizeArray(dvModules.Elaborated, (newSize + 7) >> 3);
    utResizeArray(dvModules.NextRootModule, (newSize));
    utResizeArray(dvModules.PrevRootModule, (newSize));
    utResizeArray(dvModules.NextTableRootModule, (newSize));
    utResizeArray(dvModules.FirstClass, (newSize));
    utResizeArray(dvModules.LastClass, (newSize));
    utResizeArray(dvModules.ClassTableIndex_, (newSize));
    utResizeArray(dvModules.NumClassTable, (newSize));
    utResizeArray(dvModules.NumClass, (newSize));
    utResizeArray(dvModules.FirstEnum, (newSize));
    utResizeArray(dvModules.LastEnum, (newSize));
    utResizeArray(dvModules.EnumTableIndex_, (newSize));
    utResizeArray(dvModules.NumEnumTable, (newSize));
    utResizeArray(dvModules.NumEnum, (newSize));
    utResizeArray(dvModules.FirstTypedef, (newSize));
    utResizeArray(dvModules.LastTypedef, (newSize));
    utResizeArray(dvModules.TypedefTableIndex_, (newSize));
    utResizeArray(dvModules.NumTypedefTable, (newSize));
    utResizeArray(dvModules.NumTypedef, (newSize));
    utResizeArray(dvModules.FirstSchema, (newSize));
    utResizeArray(dvModules.LastSchema, (newSize));
    utResizeArray(dvModules.SchemaTableIndex_, (newSize));
    utResizeArray(dvModules.NumSchemaTable, (newSize));
    utResizeArray(dvModules.NumSchema, (newSize));
    utResizeArray(dvModules.FirstImportLink, (newSize));
    utResizeArray(dvModules.LastImportLink, (newSize));
    utResizeArray(dvModules.FirstExportLink, (newSize));
    utResizeArray(dvModules.LastExportLink, (newSize));
    dvSetAllocatedModule(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Modules.
----------------------------------------------------------------------------------------*/
void dvModuleAllocMore(void)
{
    reallocModules((uint32)(dvAllocatedModule() + (dvAllocatedModule() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Compact the Module.ClassTable heap to free memory.
----------------------------------------------------------------------------------------*/
void dvCompactModuleClassTables(void)
{
    uint32 elementSize = sizeof(dvClass);
    uint32 usedHeaderSize = (sizeof(dvModule) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvModule) + sizeof(uint32) + elementSize - 1)/elementSize;
    dvClass *toPtr = dvModules.ClassTable;
    dvClass *fromPtr = toPtr;
    dvModule Module;
    uint32 size;

    while(fromPtr < dvModules.ClassTable + dvUsedModuleClassTable()) {
        Module = *(dvModule *)(void *)fromPtr;
        if(Module != dvModuleNull) {
            /* Need to move it to toPtr */
            size = utMax(dvModuleGetNumClassTable(Module) + usedHeaderSize, freeHeaderSize);
            memmove((void *)toPtr, (void *)fromPtr, size*elementSize);
            dvModuleSetClassTableIndex_(Module, toPtr - dvModules.ClassTable + usedHeaderSize);
            toPtr += size;
        } else {
            /* Just skip it */
            size = *(uint32 *)(void *)(((dvModule *)(void *)fromPtr) + 1);
        }
        fromPtr += size;
    }
    dvSetUsedModuleClassTable(toPtr - dvModules.ClassTable);
    dvSetFreeModuleClassTable(0);
}

/*----------------------------------------------------------------------------------------
  Allocate more memory for the Module.ClassTable heap.
----------------------------------------------------------------------------------------*/
static void allocMoreModuleClassTables(
    uint32 spaceNeeded)
{
    uint32 freeSpace = dvAllocatedModuleClassTable() - dvUsedModuleClassTable();

    if((dvFreeModuleClassTable() << 2) > dvUsedModuleClassTable()) {
        dvCompactModuleClassTables();
        freeSpace = dvAllocatedModuleClassTable() - dvUsedModuleClassTable();
    }
    if(freeSpace < spaceNeeded) {
        dvSetAllocatedModuleClassTable(dvAllocatedModuleClassTable() + spaceNeeded - freeSpace +
            (dvAllocatedModuleClassTable() >> 1));
        utResizeArray(dvModules.ClassTable, dvAllocatedModuleClassTable());
    }
}

/*----------------------------------------------------------------------------------------
  Allocate memory for a new Module.ClassTable array.
----------------------------------------------------------------------------------------*/
void dvModuleAllocClassTables(
    dvModule Module,
    uint32 numClassTables)
{
    uint32 freeSpace = dvAllocatedModuleClassTable() - dvUsedModuleClassTable();
    uint32 elementSize = sizeof(dvClass);
    uint32 usedHeaderSize = (sizeof(dvModule) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvModule) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 spaceNeeded = utMax(numClassTables + usedHeaderSize, freeHeaderSize);

#if defined(DD_DEBUG)
    utAssert(dvModuleGetNumClassTable(Module) == 0);
#endif
    if(numClassTables == 0) {
        return;
    }
    if(freeSpace < spaceNeeded) {
        allocMoreModuleClassTables(spaceNeeded);
    }
    dvModuleSetClassTableIndex_(Module, dvUsedModuleClassTable() + usedHeaderSize);
    dvModuleSetNumClassTable(Module, numClassTables);
    *(dvModule *)(void *)(dvModules.ClassTable + dvUsedModuleClassTable()) = Module;
    {
        uint32 xValue;
        for(xValue = (uint32)(dvModuleGetClassTableIndex_(Module)); xValue < dvModuleGetClassTableIndex_(Module) + numClassTables; xValue++) {
            dvModules.ClassTable[xValue] = dvClassNull;
        }
    }
    dvSetUsedModuleClassTable(dvUsedModuleClassTable() + spaceNeeded);
}

/*----------------------------------------------------------------------------------------
  Wrapper around dvModuleGetClassTables for the database manager.
----------------------------------------------------------------------------------------*/
static void *getModuleClassTables(
    uint64 objectNumber,
    uint32 *numValues)
{
    dvModule Module = dvIndex2Module((uint32)objectNumber);

    *numValues = dvModuleGetNumClassTable(Module);
    return dvModuleGetClassTables(Module);
}

/*----------------------------------------------------------------------------------------
  Wrapper around dvModuleAllocClassTables for the database manager.
----------------------------------------------------------------------------------------*/
static void *allocModuleClassTables(
    uint64 objectNumber,
    uint32 numValues)
{
    dvModule Module = dvIndex2Module((uint32)objectNumber);

    dvModuleSetClassTableIndex_(Module, 0);
    dvModuleSetNumClassTable(Module, 0);
    if(numValues == 0) {
        return NULL;
    }
    dvModuleAllocClassTables(Module, numValues);
    return dvModuleGetClassTables(Module);
}

/*----------------------------------------------------------------------------------------
  Free memory used by the Module.ClassTable array.
----------------------------------------------------------------------------------------*/
void dvModuleFreeClassTables(
    dvModule Module)
{
    uint32 elementSize = sizeof(dvClass);
    uint32 usedHeaderSize = (sizeof(dvModule) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvModule) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 size = utMax(dvModuleGetNumClassTable(Module) + usedHeaderSize, freeHeaderSize);
    dvClass *dataPtr = dvModuleGetClassTables(Module) - usedHeaderSize;

    if(dvModuleGetNumClassTable(Module) == 0) {
        return;
    }
    *(dvModule *)(void *)(dataPtr) = dvModuleNull;
    *(uint32 *)(void *)(((dvModule *)(void *)dataPtr) + 1) = size;
    dvModuleSetNumClassTable(Module, 0);
    dvSetFreeModuleClassTable(dvFreeModuleClassTable() + size);
}

/*----------------------------------------------------------------------------------------
  Resize the Module.ClassTable array.
----------------------------------------------------------------------------------------*/
void dvModuleResizeClassTables(
    dvModule Module,
    uint32 numClassTables)
{
    uint32 freeSpace;
    uint32 elementSize = sizeof(dvClass);
    uint32 usedHeaderSize = (sizeof(dvModule) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvModule) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 newSize = utMax(numClassTables + usedHeaderSize, freeHeaderSize);
    uint32 oldSize = utMax(dvModuleGetNumClassTable(Module) + usedHeaderSize, freeHeaderSize);
    dvClass *dataPtr;

    if(numClassTables == 0) {
        if(dvModuleGetNumClassTable(Module) != 0) {
            dvModuleFreeClassTables(Module);
        }
        return;
    }
    if(dvModuleGetNumClassTable(Module) == 0) {
        dvModuleAllocClassTables(Module, numClassTables);
        return;
    }
    freeSpace = dvAllocatedModuleClassTable() - dvUsedModuleClassTable();
    if(freeSpace < newSize) {
        allocMoreModuleClassTables(newSize);
    }
    dataPtr = dvModuleGetClassTables(Module) - usedHeaderSize;
    memcpy((void *)(dvModules.ClassTable + dvUsedModuleClassTable()), dataPtr,
        elementSize*utMin(oldSize, newSize));
    if(newSize > oldSize) {
        {
            uint32 xValue;
            for(xValue = (uint32)(dvUsedModuleClassTable() + oldSize); xValue < dvUsedModuleClassTable() + oldSize + newSize - oldSize; xValue++) {
                dvModules.ClassTable[xValue] = dvClassNull;
            }
        }
    }
    *(dvModule *)(void *)dataPtr = dvModuleNull;
    *(uint32 *)(void *)(((dvModule *)(void *)dataPtr) + 1) = oldSize;
    dvSetFreeModuleClassTable(dvFreeModuleClassTable() + oldSize);
    dvModuleSetClassTableIndex_(Module, dvUsedModuleClassTable() + usedHeaderSize);
    dvModuleSetNumClassTable(Module, numClassTables);
    dvSetUsedModuleClassTable(dvUsedModuleClassTable() + newSize);
}

/*----------------------------------------------------------------------------------------
  Compact the Module.EnumTable heap to free memory.
----------------------------------------------------------------------------------------*/
void dvCompactModuleEnumTables(void)
{
    uint32 elementSize = sizeof(dvEnum);
    uint32 usedHeaderSize = (sizeof(dvModule) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvModule) + sizeof(uint32) + elementSize - 1)/elementSize;
    dvEnum *toPtr = dvModules.EnumTable;
    dvEnum *fromPtr = toPtr;
    dvModule Module;
    uint32 size;

    while(fromPtr < dvModules.EnumTable + dvUsedModuleEnumTable()) {
        Module = *(dvModule *)(void *)fromPtr;
        if(Module != dvModuleNull) {
            /* Need to move it to toPtr */
            size = utMax(dvModuleGetNumEnumTable(Module) + usedHeaderSize, freeHeaderSize);
            memmove((void *)toPtr, (void *)fromPtr, size*elementSize);
            dvModuleSetEnumTableIndex_(Module, toPtr - dvModules.EnumTable + usedHeaderSize);
            toPtr += size;
        } else {
            /* Just skip it */
            size = *(uint32 *)(void *)(((dvModule *)(void *)fromPtr) + 1);
        }
        fromPtr += size;
    }
    dvSetUsedModuleEnumTable(toPtr - dvModules.EnumTable);
    dvSetFreeModuleEnumTable(0);
}

/*----------------------------------------------------------------------------------------
  Allocate more memory for the Module.EnumTable heap.
----------------------------------------------------------------------------------------*/
static void allocMoreModuleEnumTables(
    uint32 spaceNeeded)
{
    uint32 freeSpace = dvAllocatedModuleEnumTable() - dvUsedModuleEnumTable();

    if((dvFreeModuleEnumTable() << 2) > dvUsedModuleEnumTable()) {
        dvCompactModuleEnumTables();
        freeSpace = dvAllocatedModuleEnumTable() - dvUsedModuleEnumTable();
    }
    if(freeSpace < spaceNeeded) {
        dvSetAllocatedModuleEnumTable(dvAllocatedModuleEnumTable() + spaceNeeded - freeSpace +
            (dvAllocatedModuleEnumTable() >> 1));
        utResizeArray(dvModules.EnumTable, dvAllocatedModuleEnumTable());
    }
}

/*----------------------------------------------------------------------------------------
  Allocate memory for a new Module.EnumTable array.
----------------------------------------------------------------------------------------*/
void dvModuleAllocEnumTables(
    dvModule Module,
    uint32 numEnumTables)
{
    uint32 freeSpace = dvAllocatedModuleEnumTable() - dvUsedModuleEnumTable();
    uint32 elementSize = sizeof(dvEnum);
    uint32 usedHeaderSize = (sizeof(dvModule) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvModule) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 spaceNeeded = utMax(numEnumTables + usedHeaderSize, freeHeaderSize);

#if defined(DD_DEBUG)
    utAssert(dvModuleGetNumEnumTable(Module) == 0);
#endif
    if(numEnumTables == 0) {
        return;
    }
    if(freeSpace < spaceNeeded) {
        allocMoreModuleEnumTables(spaceNeeded);
    }
    dvModuleSetEnumTableIndex_(Module, dvUsedModuleEnumTable() + usedHeaderSize);
    dvModuleSetNumEnumTable(Module, numEnumTables);
    *(dvModule *)(void *)(dvModules.EnumTable + dvUsedModuleEnumTable()) = Module;
    {
        uint32 xValue;
        for(xValue = (uint32)(dvModuleGetEnumTableIndex_(Module)); xValue < dvModuleGetEnumTableIndex_(Module) + numEnumTables; xValue++) {
            dvModules.EnumTable[xValue] = dvEnumNull;
        }
    }
    dvSetUsedModuleEnumTable(dvUsedModuleEnumTable() + spaceNeeded);
}

/*----------------------------------------------------------------------------------------
  Wrapper around dvModuleGetEnumTables for the database manager.
----------------------------------------------------------------------------------------*/
static void *getModuleEnumTables(
    uint64 objectNumber,
    uint32 *numValues)
{
    dvModule Module = dvIndex2Module((uint32)objectNumber);

    *numValues = dvModuleGetNumEnumTable(Module);
    return dvModuleGetEnumTables(Module);
}

/*----------------------------------------------------------------------------------------
  Wrapper around dvModuleAllocEnumTables for the database manager.
----------------------------------------------------------------------------------------*/
static void *allocModuleEnumTables(
    uint64 objectNumber,
    uint32 numValues)
{
    dvModule Module = dvIndex2Module((uint32)objectNumber);

    dvModuleSetEnumTableIndex_(Module, 0);
    dvModuleSetNumEnumTable(Module, 0);
    if(numValues == 0) {
        return NULL;
    }
    dvModuleAllocEnumTables(Module, numValues);
    return dvModuleGetEnumTables(Module);
}

/*----------------------------------------------------------------------------------------
  Free memory used by the Module.EnumTable array.
----------------------------------------------------------------------------------------*/
void dvModuleFreeEnumTables(
    dvModule Module)
{
    uint32 elementSize = sizeof(dvEnum);
    uint32 usedHeaderSize = (sizeof(dvModule) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvModule) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 size = utMax(dvModuleGetNumEnumTable(Module) + usedHeaderSize, freeHeaderSize);
    dvEnum *dataPtr = dvModuleGetEnumTables(Module) - usedHeaderSize;

    if(dvModuleGetNumEnumTable(Module) == 0) {
        return;
    }
    *(dvModule *)(void *)(dataPtr) = dvModuleNull;
    *(uint32 *)(void *)(((dvModule *)(void *)dataPtr) + 1) = size;
    dvModuleSetNumEnumTable(Module, 0);
    dvSetFreeModuleEnumTable(dvFreeModuleEnumTable() + size);
}

/*----------------------------------------------------------------------------------------
  Resize the Module.EnumTable array.
----------------------------------------------------------------------------------------*/
void dvModuleResizeEnumTables(
    dvModule Module,
    uint32 numEnumTables)
{
    uint32 freeSpace;
    uint32 elementSize = sizeof(dvEnum);
    uint32 usedHeaderSize = (sizeof(dvModule) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvModule) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 newSize = utMax(numEnumTables + usedHeaderSize, freeHeaderSize);
    uint32 oldSize = utMax(dvModuleGetNumEnumTable(Module) + usedHeaderSize, freeHeaderSize);
    dvEnum *dataPtr;

    if(numEnumTables == 0) {
        if(dvModuleGetNumEnumTable(Module) != 0) {
            dvModuleFreeEnumTables(Module);
        }
        return;
    }
    if(dvModuleGetNumEnumTable(Module) == 0) {
        dvModuleAllocEnumTables(Module, numEnumTables);
        return;
    }
    freeSpace = dvAllocatedModuleEnumTable() - dvUsedModuleEnumTable();
    if(freeSpace < newSize) {
        allocMoreModuleEnumTables(newSize);
    }
    dataPtr = dvModuleGetEnumTables(Module) - usedHeaderSize;
    memcpy((void *)(dvModules.EnumTable + dvUsedModuleEnumTable()), dataPtr,
        elementSize*utMin(oldSize, newSize));
    if(newSize > oldSize) {
        {
            uint32 xValue;
            for(xValue = (uint32)(dvUsedModuleEnumTable() + oldSize); xValue < dvUsedModuleEnumTable() + oldSize + newSize - oldSize; xValue++) {
                dvModules.EnumTable[xValue] = dvEnumNull;
            }
        }
    }
    *(dvModule *)(void *)dataPtr = dvModuleNull;
    *(uint32 *)(void *)(((dvModule *)(void *)dataPtr) + 1) = oldSize;
    dvSetFreeModuleEnumTable(dvFreeModuleEnumTable() + oldSize);
    dvModuleSetEnumTableIndex_(Module, dvUsedModuleEnumTable() + usedHeaderSize);
    dvModuleSetNumEnumTable(Module, numEnumTables);
    dvSetUsedModuleEnumTable(dvUsedModuleEnumTable() + newSize);
}

/*----------------------------------------------------------------------------------------
  Compact the Module.TypedefTable heap to free memory.
----------------------------------------------------------------------------------------*/
void dvCompactModuleTypedefTables(void)
{
    uint32 elementSize = sizeof(dvTypedef);
    uint32 usedHeaderSize = (sizeof(dvModule) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvModule) + sizeof(uint32) + elementSize - 1)/elementSize;
    dvTypedef *toPtr = dvModules.TypedefTable;
    dvTypedef *fromPtr = toPtr;
    dvModule Module;
    uint32 size;

    while(fromPtr < dvModules.TypedefTable + dvUsedModuleTypedefTable()) {
        Module = *(dvModule *)(void *)fromPtr;
        if(Module != dvModuleNull) {
            /* Need to move it to toPtr */
            size = utMax(dvModuleGetNumTypedefTable(Module) + usedHeaderSize, freeHeaderSize);
            memmove((void *)toPtr, (void *)fromPtr, size*elementSize);
            dvModuleSetTypedefTableIndex_(Module, toPtr - dvModules.TypedefTable + usedHeaderSize);
            toPtr += size;
        } else {
            /* Just skip it */
            size = *(uint32 *)(void *)(((dvModule *)(void *)fromPtr) + 1);
        }
        fromPtr += size;
    }
    dvSetUsedModuleTypedefTable(toPtr - dvModules.TypedefTable);
    dvSetFreeModuleTypedefTable(0);
}

/*----------------------------------------------------------------------------------------
  Allocate more memory for the Module.TypedefTable heap.
----------------------------------------------------------------------------------------*/
static void allocMoreModuleTypedefTables(
    uint32 spaceNeeded)
{
    uint32 freeSpace = dvAllocatedModuleTypedefTable() - dvUsedModuleTypedefTable();

    if((dvFreeModuleTypedefTable() << 2) > dvUsedModuleTypedefTable()) {
        dvCompactModuleTypedefTables();
        freeSpace = dvAllocatedModuleTypedefTable() - dvUsedModuleTypedefTable();
    }
    if(freeSpace < spaceNeeded) {
        dvSetAllocatedModuleTypedefTable(dvAllocatedModuleTypedefTable() + spaceNeeded - freeSpace +
            (dvAllocatedModuleTypedefTable() >> 1));
        utResizeArray(dvModules.TypedefTable, dvAllocatedModuleTypedefTable());
    }
}

/*----------------------------------------------------------------------------------------
  Allocate memory for a new Module.TypedefTable array.
----------------------------------------------------------------------------------------*/
void dvModuleAllocTypedefTables(
    dvModule Module,
    uint32 numTypedefTables)
{
    uint32 freeSpace = dvAllocatedModuleTypedefTable() - dvUsedModuleTypedefTable();
    uint32 elementSize = sizeof(dvTypedef);
    uint32 usedHeaderSize = (sizeof(dvModule) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvModule) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 spaceNeeded = utMax(numTypedefTables + usedHeaderSize, freeHeaderSize);

#if defined(DD_DEBUG)
    utAssert(dvModuleGetNumTypedefTable(Module) == 0);
#endif
    if(numTypedefTables == 0) {
        return;
    }
    if(freeSpace < spaceNeeded) {
        allocMoreModuleTypedefTables(spaceNeeded);
    }
    dvModuleSetTypedefTableIndex_(Module, dvUsedModuleTypedefTable() + usedHeaderSize);
    dvModuleSetNumTypedefTable(Module, numTypedefTables);
    *(dvModule *)(void *)(dvModules.TypedefTable + dvUsedModuleTypedefTable()) = Module;
    {
        uint32 xValue;
        for(xValue = (uint32)(dvModuleGetTypedefTableIndex_(Module)); xValue < dvModuleGetTypedefTableIndex_(Module) + numTypedefTables; xValue++) {
            dvModules.TypedefTable[xValue] = dvTypedefNull;
        }
    }
    dvSetUsedModuleTypedefTable(dvUsedModuleTypedefTable() + spaceNeeded);
}

/*----------------------------------------------------------------------------------------
  Wrapper around dvModuleGetTypedefTables for the database manager.
----------------------------------------------------------------------------------------*/
static void *getModuleTypedefTables(
    uint64 objectNumber,
    uint32 *numValues)
{
    dvModule Module = dvIndex2Module((uint32)objectNumber);

    *numValues = dvModuleGetNumTypedefTable(Module);
    return dvModuleGetTypedefTables(Module);
}

/*----------------------------------------------------------------------------------------
  Wrapper around dvModuleAllocTypedefTables for the database manager.
----------------------------------------------------------------------------------------*/
static void *allocModuleTypedefTables(
    uint64 objectNumber,
    uint32 numValues)
{
    dvModule Module = dvIndex2Module((uint32)objectNumber);

    dvModuleSetTypedefTableIndex_(Module, 0);
    dvModuleSetNumTypedefTable(Module, 0);
    if(numValues == 0) {
        return NULL;
    }
    dvModuleAllocTypedefTables(Module, numValues);
    return dvModuleGetTypedefTables(Module);
}

/*----------------------------------------------------------------------------------------
  Free memory used by the Module.TypedefTable array.
----------------------------------------------------------------------------------------*/
void dvModuleFreeTypedefTables(
    dvModule Module)
{
    uint32 elementSize = sizeof(dvTypedef);
    uint32 usedHeaderSize = (sizeof(dvModule) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvModule) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 size = utMax(dvModuleGetNumTypedefTable(Module) + usedHeaderSize, freeHeaderSize);
    dvTypedef *dataPtr = dvModuleGetTypedefTables(Module) - usedHeaderSize;

    if(dvModuleGetNumTypedefTable(Module) == 0) {
        return;
    }
    *(dvModule *)(void *)(dataPtr) = dvModuleNull;
    *(uint32 *)(void *)(((dvModule *)(void *)dataPtr) + 1) = size;
    dvModuleSetNumTypedefTable(Module, 0);
    dvSetFreeModuleTypedefTable(dvFreeModuleTypedefTable() + size);
}

/*----------------------------------------------------------------------------------------
  Resize the Module.TypedefTable array.
----------------------------------------------------------------------------------------*/
void dvModuleResizeTypedefTables(
    dvModule Module,
    uint32 numTypedefTables)
{
    uint32 freeSpace;
    uint32 elementSize = sizeof(dvTypedef);
    uint32 usedHeaderSize = (sizeof(dvModule) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvModule) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 newSize = utMax(numTypedefTables + usedHeaderSize, freeHeaderSize);
    uint32 oldSize = utMax(dvModuleGetNumTypedefTable(Module) + usedHeaderSize, freeHeaderSize);
    dvTypedef *dataPtr;

    if(numTypedefTables == 0) {
        if(dvModuleGetNumTypedefTable(Module) != 0) {
            dvModuleFreeTypedefTables(Module);
        }
        return;
    }
    if(dvModuleGetNumTypedefTable(Module) == 0) {
        dvModuleAllocTypedefTables(Module, numTypedefTables);
        return;
    }
    freeSpace = dvAllocatedModuleTypedefTable() - dvUsedModuleTypedefTable();
    if(freeSpace < newSize) {
        allocMoreModuleTypedefTables(newSize);
    }
    dataPtr = dvModuleGetTypedefTables(Module) - usedHeaderSize;
    memcpy((void *)(dvModules.TypedefTable + dvUsedModuleTypedefTable()), dataPtr,
        elementSize*utMin(oldSize, newSize));
    if(newSize > oldSize) {
        {
            uint32 xValue;
            for(xValue = (uint32)(dvUsedModuleTypedefTable() + oldSize); xValue < dvUsedModuleTypedefTable() + oldSize + newSize - oldSize; xValue++) {
                dvModules.TypedefTable[xValue] = dvTypedefNull;
            }
        }
    }
    *(dvModule *)(void *)dataPtr = dvModuleNull;
    *(uint32 *)(void *)(((dvModule *)(void *)dataPtr) + 1) = oldSize;
    dvSetFreeModuleTypedefTable(dvFreeModuleTypedefTable() + oldSize);
    dvModuleSetTypedefTableIndex_(Module, dvUsedModuleTypedefTable() + usedHeaderSize);
    dvModuleSetNumTypedefTable(Module, numTypedefTables);
    dvSetUsedModuleTypedefTable(dvUsedModuleTypedefTable() + newSize);
}

/*----------------------------------------------------------------------------------------
  Compact the Module.SchemaTable heap to free memory.
----------------------------------------------------------------------------------------*/
void dvCompactModuleSchemaTables(void)
{
    uint32 elementSize = sizeof(dvSchema);
    uint32 usedHeaderSize = (sizeof(dvModule) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvModule) + sizeof(uint32) + elementSize - 1)/elementSize;
    dvSchema *toPtr = dvModules.SchemaTable;
    dvSchema *fromPtr = toPtr;
    dvModule Module;
    uint32 size;

    while(fromPtr < dvModules.SchemaTable + dvUsedModuleSchemaTable()) {
        Module = *(dvModule *)(void *)fromPtr;
        if(Module != dvModuleNull) {
            /* Need to move it to toPtr */
            size = utMax(dvModuleGetNumSchemaTable(Module) + usedHeaderSize, freeHeaderSize);
            memmove((void *)toPtr, (void *)fromPtr, size*elementSize);
            dvModuleSetSchemaTableIndex_(Module, toPtr - dvModules.SchemaTable + usedHeaderSize);
            toPtr += size;
        } else {
            /* Just skip it */
            size = *(uint32 *)(void *)(((dvModule *)(void *)fromPtr) + 1);
        }
        fromPtr += size;
    }
    dvSetUsedModuleSchemaTable(toPtr - dvModules.SchemaTable);
    dvSetFreeModuleSchemaTable(0);
}

/*----------------------------------------------------------------------------------------
  Allocate more memory for the Module.SchemaTable heap.
----------------------------------------------------------------------------------------*/
static void allocMoreModuleSchemaTables(
    uint32 spaceNeeded)
{
    uint32 freeSpace = dvAllocatedModuleSchemaTable() - dvUsedModuleSchemaTable();

    if((dvFreeModuleSchemaTable() << 2) > dvUsedModuleSchemaTable()) {
        dvCompactModuleSchemaTables();
        freeSpace = dvAllocatedModuleSchemaTable() - dvUsedModuleSchemaTable();
    }
    if(freeSpace < spaceNeeded) {
        dvSetAllocatedModuleSchemaTable(dvAllocatedModuleSchemaTable() + spaceNeeded - freeSpace +
            (dvAllocatedModuleSchemaTable() >> 1));
        utResizeArray(dvModules.SchemaTable, dvAllocatedModuleSchemaTable());
    }
}

/*----------------------------------------------------------------------------------------
  Allocate memory for a new Module.SchemaTable array.
----------------------------------------------------------------------------------------*/
void dvModuleAllocSchemaTables(
    dvModule Module,
    uint32 numSchemaTables)
{
    uint32 freeSpace = dvAllocatedModuleSchemaTable() - dvUsedModuleSchemaTable();
    uint32 elementSize = sizeof(dvSchema);
    uint32 usedHeaderSize = (sizeof(dvModule) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvModule) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 spaceNeeded = utMax(numSchemaTables + usedHeaderSize, freeHeaderSize);

#if defined(DD_DEBUG)
    utAssert(dvModuleGetNumSchemaTable(Module) == 0);
#endif
    if(numSchemaTables == 0) {
        return;
    }
    if(freeSpace < spaceNeeded) {
        allocMoreModuleSchemaTables(spaceNeeded);
    }
    dvModuleSetSchemaTableIndex_(Module, dvUsedModuleSchemaTable() + usedHeaderSize);
    dvModuleSetNumSchemaTable(Module, numSchemaTables);
    *(dvModule *)(void *)(dvModules.SchemaTable + dvUsedModuleSchemaTable()) = Module;
    {
        uint32 xValue;
        for(xValue = (uint32)(dvModuleGetSchemaTableIndex_(Module)); xValue < dvModuleGetSchemaTableIndex_(Module) + numSchemaTables; xValue++) {
            dvModules.SchemaTable[xValue] = dvSchemaNull;
        }
    }
    dvSetUsedModuleSchemaTable(dvUsedModuleSchemaTable() + spaceNeeded);
}

/*----------------------------------------------------------------------------------------
  Wrapper around dvModuleGetSchemaTables for the database manager.
----------------------------------------------------------------------------------------*/
static void *getModuleSchemaTables(
    uint64 objectNumber,
    uint32 *numValues)
{
    dvModule Module = dvIndex2Module((uint32)objectNumber);

    *numValues = dvModuleGetNumSchemaTable(Module);
    return dvModuleGetSchemaTables(Module);
}

/*----------------------------------------------------------------------------------------
  Wrapper around dvModuleAllocSchemaTables for the database manager.
----------------------------------------------------------------------------------------*/
static void *allocModuleSchemaTables(
    uint64 objectNumber,
    uint32 numValues)
{
    dvModule Module = dvIndex2Module((uint32)objectNumber);

    dvModuleSetSchemaTableIndex_(Module, 0);
    dvModuleSetNumSchemaTable(Module, 0);
    if(numValues == 0) {
        return NULL;
    }
    dvModuleAllocSchemaTables(Module, numValues);
    return dvModuleGetSchemaTables(Module);
}

/*----------------------------------------------------------------------------------------
  Free memory used by the Module.SchemaTable array.
----------------------------------------------------------------------------------------*/
void dvModuleFreeSchemaTables(
    dvModule Module)
{
    uint32 elementSize = sizeof(dvSchema);
    uint32 usedHeaderSize = (sizeof(dvModule) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvModule) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 size = utMax(dvModuleGetNumSchemaTable(Module) + usedHeaderSize, freeHeaderSize);
    dvSchema *dataPtr = dvModuleGetSchemaTables(Module) - usedHeaderSize;

    if(dvModuleGetNumSchemaTable(Module) == 0) {
        return;
    }
    *(dvModule *)(void *)(dataPtr) = dvModuleNull;
    *(uint32 *)(void *)(((dvModule *)(void *)dataPtr) + 1) = size;
    dvModuleSetNumSchemaTable(Module, 0);
    dvSetFreeModuleSchemaTable(dvFreeModuleSchemaTable() + size);
}

/*----------------------------------------------------------------------------------------
  Resize the Module.SchemaTable array.
----------------------------------------------------------------------------------------*/
void dvModuleResizeSchemaTables(
    dvModule Module,
    uint32 numSchemaTables)
{
    uint32 freeSpace;
    uint32 elementSize = sizeof(dvSchema);
    uint32 usedHeaderSize = (sizeof(dvModule) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvModule) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 newSize = utMax(numSchemaTables + usedHeaderSize, freeHeaderSize);
    uint32 oldSize = utMax(dvModuleGetNumSchemaTable(Module) + usedHeaderSize, freeHeaderSize);
    dvSchema *dataPtr;

    if(numSchemaTables == 0) {
        if(dvModuleGetNumSchemaTable(Module) != 0) {
            dvModuleFreeSchemaTables(Module);
        }
        return;
    }
    if(dvModuleGetNumSchemaTable(Module) == 0) {
        dvModuleAllocSchemaTables(Module, numSchemaTables);
        return;
    }
    freeSpace = dvAllocatedModuleSchemaTable() - dvUsedModuleSchemaTable();
    if(freeSpace < newSize) {
        allocMoreModuleSchemaTables(newSize);
    }
    dataPtr = dvModuleGetSchemaTables(Module) - usedHeaderSize;
    memcpy((void *)(dvModules.SchemaTable + dvUsedModuleSchemaTable()), dataPtr,
        elementSize*utMin(oldSize, newSize));
    if(newSize > oldSize) {
        {
            uint32 xValue;
            for(xValue = (uint32)(dvUsedModuleSchemaTable() + oldSize); xValue < dvUsedModuleSchemaTable() + oldSize + newSize - oldSize; xValue++) {
                dvModules.SchemaTable[xValue] = dvSchemaNull;
            }
        }
    }
    *(dvModule *)(void *)dataPtr = dvModuleNull;
    *(uint32 *)(void *)(((dvModule *)(void *)dataPtr) + 1) = oldSize;
    dvSetFreeModuleSchemaTable(dvFreeModuleSchemaTable() + oldSize);
    dvModuleSetSchemaTableIndex_(Module, dvUsedModuleSchemaTable() + usedHeaderSize);
    dvModuleSetNumSchemaTable(Module, numSchemaTables);
    dvSetUsedModuleSchemaTable(dvUsedModuleSchemaTable() + newSize);
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Module.
----------------------------------------------------------------------------------------*/
void dvModuleCopyProps(
    dvModule oldModule,
    dvModule newModule)
{
    dvModuleSetPrefixSym(newModule, dvModuleGetPrefixSym(oldModule));
    dvModuleSetPersistent(newModule, dvModulePersistent(oldModule));
    dvModuleSetUndoRedo(newModule, dvModuleUndoRedo(oldModule));
    dvModuleSetHasSparseData(newModule, dvModuleHasSparseData(oldModule));
    dvModuleSetNumFields(newModule, dvModuleGetNumFields(oldModule));
    dvModuleSetNumClasses(newModule, dvModuleGetNumClasses(oldModule));
    dvModuleSetNumEnums(newModule, dvModuleGetNumEnums(oldModule));
    dvModuleSetElaborated(newModule, dvModuleElaborated(oldModule));
}

/*----------------------------------------------------------------------------------------
  Return the integer equivalent for the bit fields in Module.
----------------------------------------------------------------------------------------*/
uint32 dvModuleGetBitfield(
    dvModule _Module)
{
    uint32 bitfield = 0;
    uint8 xLevel = 0;

    bitfield |= dvModulePersistent(_Module) << xLevel++;
    bitfield |= dvModuleUndoRedo(_Module) << xLevel++;
    bitfield |= dvModuleHasSparseData(_Module) << xLevel++;
    bitfield |= dvModuleElaborated(_Module) << xLevel++;
    return bitfield;
}

/*----------------------------------------------------------------------------------------
  Set bit fields in Module using bitfield.
----------------------------------------------------------------------------------------*/
void dvModuleSetBitfield(
    dvModule _Module,
     uint32 bitfield)
{
    dvModuleSetPersistent(_Module, bitfield & 1);
    bitfield >>= 1;
    dvModuleSetUndoRedo(_Module, bitfield & 1);
    bitfield >>= 1;
    dvModuleSetHasSparseData(_Module, bitfield & 1);
    bitfield >>= 1;
    dvModuleSetElaborated(_Module, bitfield & 1);
    bitfield >>= 1;
}

/*----------------------------------------------------------------------------------------
  Increase the size of the hash table.
----------------------------------------------------------------------------------------*/
static void resizeModuleClassHashTable(
    dvModule Module)
{
    dvClass _Class, prevClass, nextClass;
    uint32 oldNumClasss = dvModuleGetNumClassTable(Module);
    uint32 newNumClasss = oldNumClasss << 1;
    uint32 xClass, index;

    if(newNumClasss == 0) {
        newNumClasss = 2;
        dvModuleAllocClassTables(Module, 2);
    } else {
        dvModuleResizeClassTables(Module, newNumClasss);
    }
    for(xClass = 0; xClass < oldNumClasss; xClass++) {
        _Class = dvModuleGetiClassTable(Module, xClass);
        prevClass = dvClassNull;
        while(_Class != dvClassNull) {
            nextClass = dvClassGetNextTableModuleClass(_Class);
            index = (newNumClasss - 1) & utSymGetHashValue(dvClassGetSym(_Class));
            if(index != xClass) {
                if(prevClass == dvClassNull) {
                    dvModuleSetiClassTable(Module, xClass, nextClass);
                } else {
                    dvClassSetNextTableModuleClass(prevClass, nextClass);
                }
                dvClassSetNextTableModuleClass(_Class, dvModuleGetiClassTable(Module, index));
                dvModuleSetiClassTable(Module, index, _Class);
            } else {
                prevClass = _Class;
            }
            _Class = nextClass;
        }
    }
}

/*----------------------------------------------------------------------------------------
  Add the Class to the Module.  If the table is near full, build a new one twice
  as big, delete the old one, and return the new one.
----------------------------------------------------------------------------------------*/
static void addModuleClassToHashTable(
    dvModule Module,
    dvClass _Class)
{
    dvClass nextClass;
    uint32 index;

    if(dvModuleGetNumClass(Module) >> 1 >= dvModuleGetNumClassTable(Module)) {
        resizeModuleClassHashTable(Module);
    }
    index = (dvModuleGetNumClassTable(Module) - 1) & utSymGetHashValue(dvClassGetSym(_Class));
    nextClass = dvModuleGetiClassTable(Module, index);
    dvClassSetNextTableModuleClass(_Class, nextClass);
    dvModuleSetiClassTable(Module, index, _Class);
    dvModuleSetNumClass(Module, dvModuleGetNumClass(Module) + 1);
}

/*----------------------------------------------------------------------------------------
  Remove the Class from the hash table.
----------------------------------------------------------------------------------------*/
static void removeModuleClassFromHashTable(
    dvModule Module,
    dvClass _Class)
{
    uint32 index = (dvModuleGetNumClassTable(Module) - 1) & utSymGetHashValue(dvClassGetSym(_Class));
    dvClass prevClass, nextClass;
    
    nextClass = dvModuleGetiClassTable(Module, index);
    if(nextClass == _Class) {
        dvModuleSetiClassTable(Module, index, dvClassGetNextTableModuleClass(nextClass));
    } else {
        do {
            prevClass = nextClass;
            nextClass = dvClassGetNextTableModuleClass(nextClass);
        } while(nextClass != _Class);
        dvClassSetNextTableModuleClass(prevClass, dvClassGetNextTableModuleClass(_Class));
    }
    dvModuleSetNumClass(Module, dvModuleGetNumClass(Module) - 1);
    dvClassSetNextTableModuleClass(_Class, dvClassNull);
}

/*----------------------------------------------------------------------------------------
  Find the Class from the Module and its hash key.
----------------------------------------------------------------------------------------*/
dvClass dvModuleFindClass(
    dvModule Module,
    utSym Sym)
{
    uint32 mask = dvModuleGetNumClassTable(Module) - 1;
    dvClass _Class;

    if(mask + 1 != 0) {
        _Class = dvModuleGetiClassTable(Module, utSymGetHashValue(Sym) & mask);
        while(_Class != dvClassNull) {
            if(dvClassGetSym(_Class) == Sym) {
                return _Class;
            }
            _Class = dvClassGetNextTableModuleClass(_Class);
        }
    }
    return dvClassNull;
}

/*----------------------------------------------------------------------------------------
  Find the Class from the Module and its name.
----------------------------------------------------------------------------------------*/
void dvModuleRenameClass(
    dvModule Module,
    dvClass _Class,
    utSym sym)
{
    if(dvClassGetSym(_Class) != utSymNull) {
        removeModuleClassFromHashTable(Module, _Class);
    }
    dvClassSetSym(_Class, sym);
    if(sym != utSymNull) {
        addModuleClassToHashTable(Module, _Class);
    }
}

/*----------------------------------------------------------------------------------------
  Add the Class to the head of the list on the Module.
----------------------------------------------------------------------------------------*/
void dvModuleInsertClass(
    dvModule Module,
    dvClass _Class)
{
#if defined(DD_DEBUG)
    if(Module == dvModuleNull) {
        utExit("Non-existent Module");
    }
    if(_Class == dvClassNull) {
        utExit("Non-existent Class");
    }
    if(dvClassGetModule(_Class) != dvModuleNull) {
        utExit("Attempting to add Class to Module twice");
    }
#endif
    dvClassSetNextModuleClass(_Class, dvModuleGetFirstClass(Module));
    if(dvModuleGetFirstClass(Module) != dvClassNull) {
        dvClassSetPrevModuleClass(dvModuleGetFirstClass(Module), _Class);
    }
    dvModuleSetFirstClass(Module, _Class);
    dvClassSetPrevModuleClass(_Class, dvClassNull);
    if(dvModuleGetLastClass(Module) == dvClassNull) {
        dvModuleSetLastClass(Module, _Class);
    }
    dvClassSetModule(_Class, Module);
    if(dvClassGetSym(_Class) != utSymNull) {
        addModuleClassToHashTable(Module, _Class);
    }
}

/*----------------------------------------------------------------------------------------
  Add the Class to the end of the list on the Module.
----------------------------------------------------------------------------------------*/
void dvModuleAppendClass(
    dvModule Module,
    dvClass _Class)
{
#if defined(DD_DEBUG)
    if(Module == dvModuleNull) {
        utExit("Non-existent Module");
    }
    if(_Class == dvClassNull) {
        utExit("Non-existent Class");
    }
    if(dvClassGetModule(_Class) != dvModuleNull) {
        utExit("Attempting to add Class to Module twice");
    }
#endif
    dvClassSetPrevModuleClass(_Class, dvModuleGetLastClass(Module));
    if(dvModuleGetLastClass(Module) != dvClassNull) {
        dvClassSetNextModuleClass(dvModuleGetLastClass(Module), _Class);
    }
    dvModuleSetLastClass(Module, _Class);
    dvClassSetNextModuleClass(_Class, dvClassNull);
    if(dvModuleGetFirstClass(Module) == dvClassNull) {
        dvModuleSetFirstClass(Module, _Class);
    }
    dvClassSetModule(_Class, Module);
    if(dvClassGetSym(_Class) != utSymNull) {
        addModuleClassToHashTable(Module, _Class);
    }
}

/*----------------------------------------------------------------------------------------
  Insert the Class to the Module after the previous Class.
----------------------------------------------------------------------------------------*/
void dvModuleInsertAfterClass(
    dvModule Module,
    dvClass prevClass,
    dvClass _Class)
{
    dvClass nextClass = dvClassGetNextModuleClass(prevClass);

#if defined(DD_DEBUG)
    if(Module == dvModuleNull) {
        utExit("Non-existent Module");
    }
    if(_Class == dvClassNull) {
        utExit("Non-existent Class");
    }
    if(dvClassGetModule(_Class) != dvModuleNull) {
        utExit("Attempting to add Class to Module twice");
    }
#endif
    dvClassSetNextModuleClass(_Class, nextClass);
    dvClassSetNextModuleClass(prevClass, _Class);
    dvClassSetPrevModuleClass(_Class, prevClass);
    if(nextClass != dvClassNull) {
        dvClassSetPrevModuleClass(nextClass, _Class);
    }
    if(dvModuleGetLastClass(Module) == prevClass) {
        dvModuleSetLastClass(Module, _Class);
    }
    dvClassSetModule(_Class, Module);
    if(dvClassGetSym(_Class) != utSymNull) {
        addModuleClassToHashTable(Module, _Class);
    }
}

/*----------------------------------------------------------------------------------------
 Remove the Class from the Module.
----------------------------------------------------------------------------------------*/
void dvModuleRemoveClass(
    dvModule Module,
    dvClass _Class)
{
    dvClass pClass, nClass;

#if defined(DD_DEBUG)
    if(_Class == dvClassNull) {
        utExit("Non-existent Class");
    }
    if(dvClassGetModule(_Class) != dvModuleNull && dvClassGetModule(_Class) != Module) {
        utExit("Delete Class from non-owning Module");
    }
#endif
    nClass = dvClassGetNextModuleClass(_Class);
    pClass = dvClassGetPrevModuleClass(_Class);
    if(pClass != dvClassNull) {
        dvClassSetNextModuleClass(pClass, nClass);
    } else if(dvModuleGetFirstClass(Module) == _Class) {
        dvModuleSetFirstClass(Module, nClass);
    }
    if(nClass != dvClassNull) {
        dvClassSetPrevModuleClass(nClass, pClass);
    } else if(dvModuleGetLastClass(Module) == _Class) {
        dvModuleSetLastClass(Module, pClass);
    }
    dvClassSetNextModuleClass(_Class, dvClassNull);
    dvClassSetPrevModuleClass(_Class, dvClassNull);
    dvClassSetModule(_Class, dvModuleNull);
    if(dvClassGetSym(_Class) != utSymNull) {
        removeModuleClassFromHashTable(Module, _Class);
    }
}

/*----------------------------------------------------------------------------------------
  Increase the size of the hash table.
----------------------------------------------------------------------------------------*/
static void resizeModuleEnumHashTable(
    dvModule Module)
{
    dvEnum _Enum, prevEnum, nextEnum;
    uint32 oldNumEnums = dvModuleGetNumEnumTable(Module);
    uint32 newNumEnums = oldNumEnums << 1;
    uint32 xEnum, index;

    if(newNumEnums == 0) {
        newNumEnums = 2;
        dvModuleAllocEnumTables(Module, 2);
    } else {
        dvModuleResizeEnumTables(Module, newNumEnums);
    }
    for(xEnum = 0; xEnum < oldNumEnums; xEnum++) {
        _Enum = dvModuleGetiEnumTable(Module, xEnum);
        prevEnum = dvEnumNull;
        while(_Enum != dvEnumNull) {
            nextEnum = dvEnumGetNextTableModuleEnum(_Enum);
            index = (newNumEnums - 1) & utSymGetHashValue(dvEnumGetSym(_Enum));
            if(index != xEnum) {
                if(prevEnum == dvEnumNull) {
                    dvModuleSetiEnumTable(Module, xEnum, nextEnum);
                } else {
                    dvEnumSetNextTableModuleEnum(prevEnum, nextEnum);
                }
                dvEnumSetNextTableModuleEnum(_Enum, dvModuleGetiEnumTable(Module, index));
                dvModuleSetiEnumTable(Module, index, _Enum);
            } else {
                prevEnum = _Enum;
            }
            _Enum = nextEnum;
        }
    }
}

/*----------------------------------------------------------------------------------------
  Add the Enum to the Module.  If the table is near full, build a new one twice
  as big, delete the old one, and return the new one.
----------------------------------------------------------------------------------------*/
static void addModuleEnumToHashTable(
    dvModule Module,
    dvEnum _Enum)
{
    dvEnum nextEnum;
    uint32 index;

    if(dvModuleGetNumEnum(Module) >> 1 >= dvModuleGetNumEnumTable(Module)) {
        resizeModuleEnumHashTable(Module);
    }
    index = (dvModuleGetNumEnumTable(Module) - 1) & utSymGetHashValue(dvEnumGetSym(_Enum));
    nextEnum = dvModuleGetiEnumTable(Module, index);
    dvEnumSetNextTableModuleEnum(_Enum, nextEnum);
    dvModuleSetiEnumTable(Module, index, _Enum);
    dvModuleSetNumEnum(Module, dvModuleGetNumEnum(Module) + 1);
}

/*----------------------------------------------------------------------------------------
  Remove the Enum from the hash table.
----------------------------------------------------------------------------------------*/
static void removeModuleEnumFromHashTable(
    dvModule Module,
    dvEnum _Enum)
{
    uint32 index = (dvModuleGetNumEnumTable(Module) - 1) & utSymGetHashValue(dvEnumGetSym(_Enum));
    dvEnum prevEnum, nextEnum;
    
    nextEnum = dvModuleGetiEnumTable(Module, index);
    if(nextEnum == _Enum) {
        dvModuleSetiEnumTable(Module, index, dvEnumGetNextTableModuleEnum(nextEnum));
    } else {
        do {
            prevEnum = nextEnum;
            nextEnum = dvEnumGetNextTableModuleEnum(nextEnum);
        } while(nextEnum != _Enum);
        dvEnumSetNextTableModuleEnum(prevEnum, dvEnumGetNextTableModuleEnum(_Enum));
    }
    dvModuleSetNumEnum(Module, dvModuleGetNumEnum(Module) - 1);
    dvEnumSetNextTableModuleEnum(_Enum, dvEnumNull);
}

/*----------------------------------------------------------------------------------------
  Find the Enum from the Module and its hash key.
----------------------------------------------------------------------------------------*/
dvEnum dvModuleFindEnum(
    dvModule Module,
    utSym Sym)
{
    uint32 mask = dvModuleGetNumEnumTable(Module) - 1;
    dvEnum _Enum;

    if(mask + 1 != 0) {
        _Enum = dvModuleGetiEnumTable(Module, utSymGetHashValue(Sym) & mask);
        while(_Enum != dvEnumNull) {
            if(dvEnumGetSym(_Enum) == Sym) {
                return _Enum;
            }
            _Enum = dvEnumGetNextTableModuleEnum(_Enum);
        }
    }
    return dvEnumNull;
}

/*----------------------------------------------------------------------------------------
  Find the Enum from the Module and its name.
----------------------------------------------------------------------------------------*/
void dvModuleRenameEnum(
    dvModule Module,
    dvEnum _Enum,
    utSym sym)
{
    if(dvEnumGetSym(_Enum) != utSymNull) {
        removeModuleEnumFromHashTable(Module, _Enum);
    }
    dvEnumSetSym(_Enum, sym);
    if(sym != utSymNull) {
        addModuleEnumToHashTable(Module, _Enum);
    }
}

/*----------------------------------------------------------------------------------------
  Add the Enum to the head of the list on the Module.
----------------------------------------------------------------------------------------*/
void dvModuleInsertEnum(
    dvModule Module,
    dvEnum _Enum)
{
#if defined(DD_DEBUG)
    if(Module == dvModuleNull) {
        utExit("Non-existent Module");
    }
    if(_Enum == dvEnumNull) {
        utExit("Non-existent Enum");
    }
    if(dvEnumGetModule(_Enum) != dvModuleNull) {
        utExit("Attempting to add Enum to Module twice");
    }
#endif
    dvEnumSetNextModuleEnum(_Enum, dvModuleGetFirstEnum(Module));
    if(dvModuleGetFirstEnum(Module) != dvEnumNull) {
        dvEnumSetPrevModuleEnum(dvModuleGetFirstEnum(Module), _Enum);
    }
    dvModuleSetFirstEnum(Module, _Enum);
    dvEnumSetPrevModuleEnum(_Enum, dvEnumNull);
    if(dvModuleGetLastEnum(Module) == dvEnumNull) {
        dvModuleSetLastEnum(Module, _Enum);
    }
    dvEnumSetModule(_Enum, Module);
    if(dvEnumGetSym(_Enum) != utSymNull) {
        addModuleEnumToHashTable(Module, _Enum);
    }
}

/*----------------------------------------------------------------------------------------
  Add the Enum to the end of the list on the Module.
----------------------------------------------------------------------------------------*/
void dvModuleAppendEnum(
    dvModule Module,
    dvEnum _Enum)
{
#if defined(DD_DEBUG)
    if(Module == dvModuleNull) {
        utExit("Non-existent Module");
    }
    if(_Enum == dvEnumNull) {
        utExit("Non-existent Enum");
    }
    if(dvEnumGetModule(_Enum) != dvModuleNull) {
        utExit("Attempting to add Enum to Module twice");
    }
#endif
    dvEnumSetPrevModuleEnum(_Enum, dvModuleGetLastEnum(Module));
    if(dvModuleGetLastEnum(Module) != dvEnumNull) {
        dvEnumSetNextModuleEnum(dvModuleGetLastEnum(Module), _Enum);
    }
    dvModuleSetLastEnum(Module, _Enum);
    dvEnumSetNextModuleEnum(_Enum, dvEnumNull);
    if(dvModuleGetFirstEnum(Module) == dvEnumNull) {
        dvModuleSetFirstEnum(Module, _Enum);
    }
    dvEnumSetModule(_Enum, Module);
    if(dvEnumGetSym(_Enum) != utSymNull) {
        addModuleEnumToHashTable(Module, _Enum);
    }
}

/*----------------------------------------------------------------------------------------
  Insert the Enum to the Module after the previous Enum.
----------------------------------------------------------------------------------------*/
void dvModuleInsertAfterEnum(
    dvModule Module,
    dvEnum prevEnum,
    dvEnum _Enum)
{
    dvEnum nextEnum = dvEnumGetNextModuleEnum(prevEnum);

#if defined(DD_DEBUG)
    if(Module == dvModuleNull) {
        utExit("Non-existent Module");
    }
    if(_Enum == dvEnumNull) {
        utExit("Non-existent Enum");
    }
    if(dvEnumGetModule(_Enum) != dvModuleNull) {
        utExit("Attempting to add Enum to Module twice");
    }
#endif
    dvEnumSetNextModuleEnum(_Enum, nextEnum);
    dvEnumSetNextModuleEnum(prevEnum, _Enum);
    dvEnumSetPrevModuleEnum(_Enum, prevEnum);
    if(nextEnum != dvEnumNull) {
        dvEnumSetPrevModuleEnum(nextEnum, _Enum);
    }
    if(dvModuleGetLastEnum(Module) == prevEnum) {
        dvModuleSetLastEnum(Module, _Enum);
    }
    dvEnumSetModule(_Enum, Module);
    if(dvEnumGetSym(_Enum) != utSymNull) {
        addModuleEnumToHashTable(Module, _Enum);
    }
}

/*----------------------------------------------------------------------------------------
 Remove the Enum from the Module.
----------------------------------------------------------------------------------------*/
void dvModuleRemoveEnum(
    dvModule Module,
    dvEnum _Enum)
{
    dvEnum pEnum, nEnum;

#if defined(DD_DEBUG)
    if(_Enum == dvEnumNull) {
        utExit("Non-existent Enum");
    }
    if(dvEnumGetModule(_Enum) != dvModuleNull && dvEnumGetModule(_Enum) != Module) {
        utExit("Delete Enum from non-owning Module");
    }
#endif
    nEnum = dvEnumGetNextModuleEnum(_Enum);
    pEnum = dvEnumGetPrevModuleEnum(_Enum);
    if(pEnum != dvEnumNull) {
        dvEnumSetNextModuleEnum(pEnum, nEnum);
    } else if(dvModuleGetFirstEnum(Module) == _Enum) {
        dvModuleSetFirstEnum(Module, nEnum);
    }
    if(nEnum != dvEnumNull) {
        dvEnumSetPrevModuleEnum(nEnum, pEnum);
    } else if(dvModuleGetLastEnum(Module) == _Enum) {
        dvModuleSetLastEnum(Module, pEnum);
    }
    dvEnumSetNextModuleEnum(_Enum, dvEnumNull);
    dvEnumSetPrevModuleEnum(_Enum, dvEnumNull);
    dvEnumSetModule(_Enum, dvModuleNull);
    if(dvEnumGetSym(_Enum) != utSymNull) {
        removeModuleEnumFromHashTable(Module, _Enum);
    }
}

/*----------------------------------------------------------------------------------------
  Increase the size of the hash table.
----------------------------------------------------------------------------------------*/
static void resizeModuleTypedefHashTable(
    dvModule Module)
{
    dvTypedef _Typedef, prevTypedef, nextTypedef;
    uint32 oldNumTypedefs = dvModuleGetNumTypedefTable(Module);
    uint32 newNumTypedefs = oldNumTypedefs << 1;
    uint32 xTypedef, index;

    if(newNumTypedefs == 0) {
        newNumTypedefs = 2;
        dvModuleAllocTypedefTables(Module, 2);
    } else {
        dvModuleResizeTypedefTables(Module, newNumTypedefs);
    }
    for(xTypedef = 0; xTypedef < oldNumTypedefs; xTypedef++) {
        _Typedef = dvModuleGetiTypedefTable(Module, xTypedef);
        prevTypedef = dvTypedefNull;
        while(_Typedef != dvTypedefNull) {
            nextTypedef = dvTypedefGetNextTableModuleTypedef(_Typedef);
            index = (newNumTypedefs - 1) & utSymGetHashValue(dvTypedefGetSym(_Typedef));
            if(index != xTypedef) {
                if(prevTypedef == dvTypedefNull) {
                    dvModuleSetiTypedefTable(Module, xTypedef, nextTypedef);
                } else {
                    dvTypedefSetNextTableModuleTypedef(prevTypedef, nextTypedef);
                }
                dvTypedefSetNextTableModuleTypedef(_Typedef, dvModuleGetiTypedefTable(Module, index));
                dvModuleSetiTypedefTable(Module, index, _Typedef);
            } else {
                prevTypedef = _Typedef;
            }
            _Typedef = nextTypedef;
        }
    }
}

/*----------------------------------------------------------------------------------------
  Add the Typedef to the Module.  If the table is near full, build a new one twice
  as big, delete the old one, and return the new one.
----------------------------------------------------------------------------------------*/
static void addModuleTypedefToHashTable(
    dvModule Module,
    dvTypedef _Typedef)
{
    dvTypedef nextTypedef;
    uint32 index;

    if(dvModuleGetNumTypedef(Module) >> 1 >= dvModuleGetNumTypedefTable(Module)) {
        resizeModuleTypedefHashTable(Module);
    }
    index = (dvModuleGetNumTypedefTable(Module) - 1) & utSymGetHashValue(dvTypedefGetSym(_Typedef));
    nextTypedef = dvModuleGetiTypedefTable(Module, index);
    dvTypedefSetNextTableModuleTypedef(_Typedef, nextTypedef);
    dvModuleSetiTypedefTable(Module, index, _Typedef);
    dvModuleSetNumTypedef(Module, dvModuleGetNumTypedef(Module) + 1);
}

/*----------------------------------------------------------------------------------------
  Remove the Typedef from the hash table.
----------------------------------------------------------------------------------------*/
static void removeModuleTypedefFromHashTable(
    dvModule Module,
    dvTypedef _Typedef)
{
    uint32 index = (dvModuleGetNumTypedefTable(Module) - 1) & utSymGetHashValue(dvTypedefGetSym(_Typedef));
    dvTypedef prevTypedef, nextTypedef;
    
    nextTypedef = dvModuleGetiTypedefTable(Module, index);
    if(nextTypedef == _Typedef) {
        dvModuleSetiTypedefTable(Module, index, dvTypedefGetNextTableModuleTypedef(nextTypedef));
    } else {
        do {
            prevTypedef = nextTypedef;
            nextTypedef = dvTypedefGetNextTableModuleTypedef(nextTypedef);
        } while(nextTypedef != _Typedef);
        dvTypedefSetNextTableModuleTypedef(prevTypedef, dvTypedefGetNextTableModuleTypedef(_Typedef));
    }
    dvModuleSetNumTypedef(Module, dvModuleGetNumTypedef(Module) - 1);
    dvTypedefSetNextTableModuleTypedef(_Typedef, dvTypedefNull);
}

/*----------------------------------------------------------------------------------------
  Find the Typedef from the Module and its hash key.
----------------------------------------------------------------------------------------*/
dvTypedef dvModuleFindTypedef(
    dvModule Module,
    utSym Sym)
{
    uint32 mask = dvModuleGetNumTypedefTable(Module) - 1;
    dvTypedef _Typedef;

    if(mask + 1 != 0) {
        _Typedef = dvModuleGetiTypedefTable(Module, utSymGetHashValue(Sym) & mask);
        while(_Typedef != dvTypedefNull) {
            if(dvTypedefGetSym(_Typedef) == Sym) {
                return _Typedef;
            }
            _Typedef = dvTypedefGetNextTableModuleTypedef(_Typedef);
        }
    }
    return dvTypedefNull;
}

/*----------------------------------------------------------------------------------------
  Find the Typedef from the Module and its name.
----------------------------------------------------------------------------------------*/
void dvModuleRenameTypedef(
    dvModule Module,
    dvTypedef _Typedef,
    utSym sym)
{
    if(dvTypedefGetSym(_Typedef) != utSymNull) {
        removeModuleTypedefFromHashTable(Module, _Typedef);
    }
    dvTypedefSetSym(_Typedef, sym);
    if(sym != utSymNull) {
        addModuleTypedefToHashTable(Module, _Typedef);
    }
}

/*----------------------------------------------------------------------------------------
  Add the Typedef to the head of the list on the Module.
----------------------------------------------------------------------------------------*/
void dvModuleInsertTypedef(
    dvModule Module,
    dvTypedef _Typedef)
{
#if defined(DD_DEBUG)
    if(Module == dvModuleNull) {
        utExit("Non-existent Module");
    }
    if(_Typedef == dvTypedefNull) {
        utExit("Non-existent Typedef");
    }
    if(dvTypedefGetModule(_Typedef) != dvModuleNull) {
        utExit("Attempting to add Typedef to Module twice");
    }
#endif
    dvTypedefSetNextModuleTypedef(_Typedef, dvModuleGetFirstTypedef(Module));
    if(dvModuleGetFirstTypedef(Module) != dvTypedefNull) {
        dvTypedefSetPrevModuleTypedef(dvModuleGetFirstTypedef(Module), _Typedef);
    }
    dvModuleSetFirstTypedef(Module, _Typedef);
    dvTypedefSetPrevModuleTypedef(_Typedef, dvTypedefNull);
    if(dvModuleGetLastTypedef(Module) == dvTypedefNull) {
        dvModuleSetLastTypedef(Module, _Typedef);
    }
    dvTypedefSetModule(_Typedef, Module);
    if(dvTypedefGetSym(_Typedef) != utSymNull) {
        addModuleTypedefToHashTable(Module, _Typedef);
    }
}

/*----------------------------------------------------------------------------------------
  Add the Typedef to the end of the list on the Module.
----------------------------------------------------------------------------------------*/
void dvModuleAppendTypedef(
    dvModule Module,
    dvTypedef _Typedef)
{
#if defined(DD_DEBUG)
    if(Module == dvModuleNull) {
        utExit("Non-existent Module");
    }
    if(_Typedef == dvTypedefNull) {
        utExit("Non-existent Typedef");
    }
    if(dvTypedefGetModule(_Typedef) != dvModuleNull) {
        utExit("Attempting to add Typedef to Module twice");
    }
#endif
    dvTypedefSetPrevModuleTypedef(_Typedef, dvModuleGetLastTypedef(Module));
    if(dvModuleGetLastTypedef(Module) != dvTypedefNull) {
        dvTypedefSetNextModuleTypedef(dvModuleGetLastTypedef(Module), _Typedef);
    }
    dvModuleSetLastTypedef(Module, _Typedef);
    dvTypedefSetNextModuleTypedef(_Typedef, dvTypedefNull);
    if(dvModuleGetFirstTypedef(Module) == dvTypedefNull) {
        dvModuleSetFirstTypedef(Module, _Typedef);
    }
    dvTypedefSetModule(_Typedef, Module);
    if(dvTypedefGetSym(_Typedef) != utSymNull) {
        addModuleTypedefToHashTable(Module, _Typedef);
    }
}

/*----------------------------------------------------------------------------------------
  Insert the Typedef to the Module after the previous Typedef.
----------------------------------------------------------------------------------------*/
void dvModuleInsertAfterTypedef(
    dvModule Module,
    dvTypedef prevTypedef,
    dvTypedef _Typedef)
{
    dvTypedef nextTypedef = dvTypedefGetNextModuleTypedef(prevTypedef);

#if defined(DD_DEBUG)
    if(Module == dvModuleNull) {
        utExit("Non-existent Module");
    }
    if(_Typedef == dvTypedefNull) {
        utExit("Non-existent Typedef");
    }
    if(dvTypedefGetModule(_Typedef) != dvModuleNull) {
        utExit("Attempting to add Typedef to Module twice");
    }
#endif
    dvTypedefSetNextModuleTypedef(_Typedef, nextTypedef);
    dvTypedefSetNextModuleTypedef(prevTypedef, _Typedef);
    dvTypedefSetPrevModuleTypedef(_Typedef, prevTypedef);
    if(nextTypedef != dvTypedefNull) {
        dvTypedefSetPrevModuleTypedef(nextTypedef, _Typedef);
    }
    if(dvModuleGetLastTypedef(Module) == prevTypedef) {
        dvModuleSetLastTypedef(Module, _Typedef);
    }
    dvTypedefSetModule(_Typedef, Module);
    if(dvTypedefGetSym(_Typedef) != utSymNull) {
        addModuleTypedefToHashTable(Module, _Typedef);
    }
}

/*----------------------------------------------------------------------------------------
 Remove the Typedef from the Module.
----------------------------------------------------------------------------------------*/
void dvModuleRemoveTypedef(
    dvModule Module,
    dvTypedef _Typedef)
{
    dvTypedef pTypedef, nTypedef;

#if defined(DD_DEBUG)
    if(_Typedef == dvTypedefNull) {
        utExit("Non-existent Typedef");
    }
    if(dvTypedefGetModule(_Typedef) != dvModuleNull && dvTypedefGetModule(_Typedef) != Module) {
        utExit("Delete Typedef from non-owning Module");
    }
#endif
    nTypedef = dvTypedefGetNextModuleTypedef(_Typedef);
    pTypedef = dvTypedefGetPrevModuleTypedef(_Typedef);
    if(pTypedef != dvTypedefNull) {
        dvTypedefSetNextModuleTypedef(pTypedef, nTypedef);
    } else if(dvModuleGetFirstTypedef(Module) == _Typedef) {
        dvModuleSetFirstTypedef(Module, nTypedef);
    }
    if(nTypedef != dvTypedefNull) {
        dvTypedefSetPrevModuleTypedef(nTypedef, pTypedef);
    } else if(dvModuleGetLastTypedef(Module) == _Typedef) {
        dvModuleSetLastTypedef(Module, pTypedef);
    }
    dvTypedefSetNextModuleTypedef(_Typedef, dvTypedefNull);
    dvTypedefSetPrevModuleTypedef(_Typedef, dvTypedefNull);
    dvTypedefSetModule(_Typedef, dvModuleNull);
    if(dvTypedefGetSym(_Typedef) != utSymNull) {
        removeModuleTypedefFromHashTable(Module, _Typedef);
    }
}

/*----------------------------------------------------------------------------------------
  Increase the size of the hash table.
----------------------------------------------------------------------------------------*/
static void resizeModuleSchemaHashTable(
    dvModule Module)
{
    dvSchema _Schema, prevSchema, nextSchema;
    uint32 oldNumSchemas = dvModuleGetNumSchemaTable(Module);
    uint32 newNumSchemas = oldNumSchemas << 1;
    uint32 xSchema, index;

    if(newNumSchemas == 0) {
        newNumSchemas = 2;
        dvModuleAllocSchemaTables(Module, 2);
    } else {
        dvModuleResizeSchemaTables(Module, newNumSchemas);
    }
    for(xSchema = 0; xSchema < oldNumSchemas; xSchema++) {
        _Schema = dvModuleGetiSchemaTable(Module, xSchema);
        prevSchema = dvSchemaNull;
        while(_Schema != dvSchemaNull) {
            nextSchema = dvSchemaGetNextTableModuleSchema(_Schema);
            index = (newNumSchemas - 1) & utSymGetHashValue(dvSchemaGetSym(_Schema));
            if(index != xSchema) {
                if(prevSchema == dvSchemaNull) {
                    dvModuleSetiSchemaTable(Module, xSchema, nextSchema);
                } else {
                    dvSchemaSetNextTableModuleSchema(prevSchema, nextSchema);
                }
                dvSchemaSetNextTableModuleSchema(_Schema, dvModuleGetiSchemaTable(Module, index));
                dvModuleSetiSchemaTable(Module, index, _Schema);
            } else {
                prevSchema = _Schema;
            }
            _Schema = nextSchema;
        }
    }
}

/*----------------------------------------------------------------------------------------
  Add the Schema to the Module.  If the table is near full, build a new one twice
  as big, delete the old one, and return the new one.
----------------------------------------------------------------------------------------*/
static void addModuleSchemaToHashTable(
    dvModule Module,
    dvSchema _Schema)
{
    dvSchema nextSchema;
    uint32 index;

    if(dvModuleGetNumSchema(Module) >> 1 >= dvModuleGetNumSchemaTable(Module)) {
        resizeModuleSchemaHashTable(Module);
    }
    index = (dvModuleGetNumSchemaTable(Module) - 1) & utSymGetHashValue(dvSchemaGetSym(_Schema));
    nextSchema = dvModuleGetiSchemaTable(Module, index);
    dvSchemaSetNextTableModuleSchema(_Schema, nextSchema);
    dvModuleSetiSchemaTable(Module, index, _Schema);
    dvModuleSetNumSchema(Module, dvModuleGetNumSchema(Module) + 1);
}

/*----------------------------------------------------------------------------------------
  Remove the Schema from the hash table.
----------------------------------------------------------------------------------------*/
static void removeModuleSchemaFromHashTable(
    dvModule Module,
    dvSchema _Schema)
{
    uint32 index = (dvModuleGetNumSchemaTable(Module) - 1) & utSymGetHashValue(dvSchemaGetSym(_Schema));
    dvSchema prevSchema, nextSchema;
    
    nextSchema = dvModuleGetiSchemaTable(Module, index);
    if(nextSchema == _Schema) {
        dvModuleSetiSchemaTable(Module, index, dvSchemaGetNextTableModuleSchema(nextSchema));
    } else {
        do {
            prevSchema = nextSchema;
            nextSchema = dvSchemaGetNextTableModuleSchema(nextSchema);
        } while(nextSchema != _Schema);
        dvSchemaSetNextTableModuleSchema(prevSchema, dvSchemaGetNextTableModuleSchema(_Schema));
    }
    dvModuleSetNumSchema(Module, dvModuleGetNumSchema(Module) - 1);
    dvSchemaSetNextTableModuleSchema(_Schema, dvSchemaNull);
}

/*----------------------------------------------------------------------------------------
  Find the Schema from the Module and its hash key.
----------------------------------------------------------------------------------------*/
dvSchema dvModuleFindSchema(
    dvModule Module,
    utSym Sym)
{
    uint32 mask = dvModuleGetNumSchemaTable(Module) - 1;
    dvSchema _Schema;

    if(mask + 1 != 0) {
        _Schema = dvModuleGetiSchemaTable(Module, utSymGetHashValue(Sym) & mask);
        while(_Schema != dvSchemaNull) {
            if(dvSchemaGetSym(_Schema) == Sym) {
                return _Schema;
            }
            _Schema = dvSchemaGetNextTableModuleSchema(_Schema);
        }
    }
    return dvSchemaNull;
}

/*----------------------------------------------------------------------------------------
  Find the Schema from the Module and its name.
----------------------------------------------------------------------------------------*/
void dvModuleRenameSchema(
    dvModule Module,
    dvSchema _Schema,
    utSym sym)
{
    if(dvSchemaGetSym(_Schema) != utSymNull) {
        removeModuleSchemaFromHashTable(Module, _Schema);
    }
    dvSchemaSetSym(_Schema, sym);
    if(sym != utSymNull) {
        addModuleSchemaToHashTable(Module, _Schema);
    }
}

/*----------------------------------------------------------------------------------------
  Add the Schema to the head of the list on the Module.
----------------------------------------------------------------------------------------*/
void dvModuleInsertSchema(
    dvModule Module,
    dvSchema _Schema)
{
#if defined(DD_DEBUG)
    if(Module == dvModuleNull) {
        utExit("Non-existent Module");
    }
    if(_Schema == dvSchemaNull) {
        utExit("Non-existent Schema");
    }
    if(dvSchemaGetModule(_Schema) != dvModuleNull) {
        utExit("Attempting to add Schema to Module twice");
    }
#endif
    dvSchemaSetNextModuleSchema(_Schema, dvModuleGetFirstSchema(Module));
    if(dvModuleGetFirstSchema(Module) != dvSchemaNull) {
        dvSchemaSetPrevModuleSchema(dvModuleGetFirstSchema(Module), _Schema);
    }
    dvModuleSetFirstSchema(Module, _Schema);
    dvSchemaSetPrevModuleSchema(_Schema, dvSchemaNull);
    if(dvModuleGetLastSchema(Module) == dvSchemaNull) {
        dvModuleSetLastSchema(Module, _Schema);
    }
    dvSchemaSetModule(_Schema, Module);
    if(dvSchemaGetSym(_Schema) != utSymNull) {
        addModuleSchemaToHashTable(Module, _Schema);
    }
}

/*----------------------------------------------------------------------------------------
  Add the Schema to the end of the list on the Module.
----------------------------------------------------------------------------------------*/
void dvModuleAppendSchema(
    dvModule Module,
    dvSchema _Schema)
{
#if defined(DD_DEBUG)
    if(Module == dvModuleNull) {
        utExit("Non-existent Module");
    }
    if(_Schema == dvSchemaNull) {
        utExit("Non-existent Schema");
    }
    if(dvSchemaGetModule(_Schema) != dvModuleNull) {
        utExit("Attempting to add Schema to Module twice");
    }
#endif
    dvSchemaSetPrevModuleSchema(_Schema, dvModuleGetLastSchema(Module));
    if(dvModuleGetLastSchema(Module) != dvSchemaNull) {
        dvSchemaSetNextModuleSchema(dvModuleGetLastSchema(Module), _Schema);
    }
    dvModuleSetLastSchema(Module, _Schema);
    dvSchemaSetNextModuleSchema(_Schema, dvSchemaNull);
    if(dvModuleGetFirstSchema(Module) == dvSchemaNull) {
        dvModuleSetFirstSchema(Module, _Schema);
    }
    dvSchemaSetModule(_Schema, Module);
    if(dvSchemaGetSym(_Schema) != utSymNull) {
        addModuleSchemaToHashTable(Module, _Schema);
    }
}

/*----------------------------------------------------------------------------------------
  Insert the Schema to the Module after the previous Schema.
----------------------------------------------------------------------------------------*/
void dvModuleInsertAfterSchema(
    dvModule Module,
    dvSchema prevSchema,
    dvSchema _Schema)
{
    dvSchema nextSchema = dvSchemaGetNextModuleSchema(prevSchema);

#if defined(DD_DEBUG)
    if(Module == dvModuleNull) {
        utExit("Non-existent Module");
    }
    if(_Schema == dvSchemaNull) {
        utExit("Non-existent Schema");
    }
    if(dvSchemaGetModule(_Schema) != dvModuleNull) {
        utExit("Attempting to add Schema to Module twice");
    }
#endif
    dvSchemaSetNextModuleSchema(_Schema, nextSchema);
    dvSchemaSetNextModuleSchema(prevSchema, _Schema);
    dvSchemaSetPrevModuleSchema(_Schema, prevSchema);
    if(nextSchema != dvSchemaNull) {
        dvSchemaSetPrevModuleSchema(nextSchema, _Schema);
    }
    if(dvModuleGetLastSchema(Module) == prevSchema) {
        dvModuleSetLastSchema(Module, _Schema);
    }
    dvSchemaSetModule(_Schema, Module);
    if(dvSchemaGetSym(_Schema) != utSymNull) {
        addModuleSchemaToHashTable(Module, _Schema);
    }
}

/*----------------------------------------------------------------------------------------
 Remove the Schema from the Module.
----------------------------------------------------------------------------------------*/
void dvModuleRemoveSchema(
    dvModule Module,
    dvSchema _Schema)
{
    dvSchema pSchema, nSchema;

#if defined(DD_DEBUG)
    if(_Schema == dvSchemaNull) {
        utExit("Non-existent Schema");
    }
    if(dvSchemaGetModule(_Schema) != dvModuleNull && dvSchemaGetModule(_Schema) != Module) {
        utExit("Delete Schema from non-owning Module");
    }
#endif
    nSchema = dvSchemaGetNextModuleSchema(_Schema);
    pSchema = dvSchemaGetPrevModuleSchema(_Schema);
    if(pSchema != dvSchemaNull) {
        dvSchemaSetNextModuleSchema(pSchema, nSchema);
    } else if(dvModuleGetFirstSchema(Module) == _Schema) {
        dvModuleSetFirstSchema(Module, nSchema);
    }
    if(nSchema != dvSchemaNull) {
        dvSchemaSetPrevModuleSchema(nSchema, pSchema);
    } else if(dvModuleGetLastSchema(Module) == _Schema) {
        dvModuleSetLastSchema(Module, pSchema);
    }
    dvSchemaSetNextModuleSchema(_Schema, dvSchemaNull);
    dvSchemaSetPrevModuleSchema(_Schema, dvSchemaNull);
    dvSchemaSetModule(_Schema, dvModuleNull);
    if(dvSchemaGetSym(_Schema) != utSymNull) {
        removeModuleSchemaFromHashTable(Module, _Schema);
    }
}

/*----------------------------------------------------------------------------------------
  Add the ImportLink to the head of the list on the Module.
----------------------------------------------------------------------------------------*/
void dvModuleInsertImportLink(
    dvModule Module,
    dvLink _Link)
{
#if defined(DD_DEBUG)
    if(Module == dvModuleNull) {
        utExit("Non-existent Module");
    }
    if(_Link == dvLinkNull) {
        utExit("Non-existent Link");
    }
    if(dvLinkGetImportModule(_Link) != dvModuleNull) {
        utExit("Attempting to add Link to Module twice");
    }
#endif
    dvLinkSetNextModuleImportLink(_Link, dvModuleGetFirstImportLink(Module));
    dvModuleSetFirstImportLink(Module, _Link);
    if(dvModuleGetLastImportLink(Module) == dvLinkNull) {
        dvModuleSetLastImportLink(Module, _Link);
    }
    dvLinkSetImportModule(_Link, Module);
}

/*----------------------------------------------------------------------------------------
  Add the ImportLink to the end of the list on the Module.
----------------------------------------------------------------------------------------*/
void dvModuleAppendImportLink(
    dvModule Module,
    dvLink _Link)
{
#if defined(DD_DEBUG)
    if(Module == dvModuleNull) {
        utExit("Non-existent Module");
    }
    if(_Link == dvLinkNull) {
        utExit("Non-existent Link");
    }
    if(dvLinkGetImportModule(_Link) != dvModuleNull) {
        utExit("Attempting to add Link to Module twice");
    }
#endif
    if(dvModuleGetLastImportLink(Module) != dvLinkNull) {
        dvLinkSetNextModuleImportLink(dvModuleGetLastImportLink(Module), _Link);
    } else {
        dvModuleSetFirstImportLink(Module, _Link);
    }
    dvModuleSetLastImportLink(Module, _Link);
    dvLinkSetNextModuleImportLink(_Link, dvLinkNull);
    dvLinkSetImportModule(_Link, Module);
}

/*----------------------------------------------------------------------------------------
  Insert the ImportLink to the Module after the previous ImportLink.
----------------------------------------------------------------------------------------*/
void dvModuleInsertAfterImportLink(
    dvModule Module,
    dvLink prevLink,
    dvLink _Link)
{
    dvLink nextLink = dvLinkGetNextModuleImportLink(prevLink);

#if defined(DD_DEBUG)
    if(Module == dvModuleNull) {
        utExit("Non-existent Module");
    }
    if(_Link == dvLinkNull) {
        utExit("Non-existent Link");
    }
    if(dvLinkGetImportModule(_Link) != dvModuleNull) {
        utExit("Attempting to add Link to Module twice");
    }
#endif
    dvLinkSetNextModuleImportLink(_Link, nextLink);
    dvLinkSetNextModuleImportLink(prevLink, _Link);
    if(dvModuleGetLastImportLink(Module) == prevLink) {
        dvModuleSetLastImportLink(Module, _Link);
    }
    dvLinkSetImportModule(_Link, Module);
}

/*----------------------------------------------------------------------------------------
 Remove the ImportLink from the Module.
----------------------------------------------------------------------------------------*/
void dvModuleRemoveImportLink(
    dvModule Module,
    dvLink _Link)
{
    dvLink pLink, nLink;

#if defined(DD_DEBUG)
    if(_Link == dvLinkNull) {
        utExit("Non-existent Link");
    }
    if(dvLinkGetImportModule(_Link) != dvModuleNull && dvLinkGetImportModule(_Link) != Module) {
        utExit("Delete Link from non-owning Module");
    }
#endif
    pLink = dvLinkNull;
    for(nLink = dvModuleGetFirstImportLink(Module); nLink != dvLinkNull && nLink != _Link;
            nLink = dvLinkGetNextModuleImportLink(nLink)) {
        pLink = nLink;
    }
    if(pLink != dvLinkNull) {
        dvLinkSetNextModuleImportLink(pLink, dvLinkGetNextModuleImportLink(_Link));
    } else {
        dvModuleSetFirstImportLink(Module, dvLinkGetNextModuleImportLink(_Link));
    }
    dvLinkSetNextModuleImportLink(_Link, dvLinkNull);
    if(dvModuleGetLastImportLink(Module) == _Link) {
        dvModuleSetLastImportLink(Module, pLink);
    }
    dvLinkSetImportModule(_Link, dvModuleNull);
}

/*----------------------------------------------------------------------------------------
  Add the ExportLink to the head of the list on the Module.
----------------------------------------------------------------------------------------*/
void dvModuleInsertExportLink(
    dvModule Module,
    dvLink _Link)
{
#if defined(DD_DEBUG)
    if(Module == dvModuleNull) {
        utExit("Non-existent Module");
    }
    if(_Link == dvLinkNull) {
        utExit("Non-existent Link");
    }
    if(dvLinkGetExportModule(_Link) != dvModuleNull) {
        utExit("Attempting to add Link to Module twice");
    }
#endif
    dvLinkSetNextModuleExportLink(_Link, dvModuleGetFirstExportLink(Module));
    dvModuleSetFirstExportLink(Module, _Link);
    if(dvModuleGetLastExportLink(Module) == dvLinkNull) {
        dvModuleSetLastExportLink(Module, _Link);
    }
    dvLinkSetExportModule(_Link, Module);
}

/*----------------------------------------------------------------------------------------
  Add the ExportLink to the end of the list on the Module.
----------------------------------------------------------------------------------------*/
void dvModuleAppendExportLink(
    dvModule Module,
    dvLink _Link)
{
#if defined(DD_DEBUG)
    if(Module == dvModuleNull) {
        utExit("Non-existent Module");
    }
    if(_Link == dvLinkNull) {
        utExit("Non-existent Link");
    }
    if(dvLinkGetExportModule(_Link) != dvModuleNull) {
        utExit("Attempting to add Link to Module twice");
    }
#endif
    if(dvModuleGetLastExportLink(Module) != dvLinkNull) {
        dvLinkSetNextModuleExportLink(dvModuleGetLastExportLink(Module), _Link);
    } else {
        dvModuleSetFirstExportLink(Module, _Link);
    }
    dvModuleSetLastExportLink(Module, _Link);
    dvLinkSetNextModuleExportLink(_Link, dvLinkNull);
    dvLinkSetExportModule(_Link, Module);
}

/*----------------------------------------------------------------------------------------
  Insert the ExportLink to the Module after the previous ExportLink.
----------------------------------------------------------------------------------------*/
void dvModuleInsertAfterExportLink(
    dvModule Module,
    dvLink prevLink,
    dvLink _Link)
{
    dvLink nextLink = dvLinkGetNextModuleExportLink(prevLink);

#if defined(DD_DEBUG)
    if(Module == dvModuleNull) {
        utExit("Non-existent Module");
    }
    if(_Link == dvLinkNull) {
        utExit("Non-existent Link");
    }
    if(dvLinkGetExportModule(_Link) != dvModuleNull) {
        utExit("Attempting to add Link to Module twice");
    }
#endif
    dvLinkSetNextModuleExportLink(_Link, nextLink);
    dvLinkSetNextModuleExportLink(prevLink, _Link);
    if(dvModuleGetLastExportLink(Module) == prevLink) {
        dvModuleSetLastExportLink(Module, _Link);
    }
    dvLinkSetExportModule(_Link, Module);
}

/*----------------------------------------------------------------------------------------
 Remove the ExportLink from the Module.
----------------------------------------------------------------------------------------*/
void dvModuleRemoveExportLink(
    dvModule Module,
    dvLink _Link)
{
    dvLink pLink, nLink;

#if defined(DD_DEBUG)
    if(_Link == dvLinkNull) {
        utExit("Non-existent Link");
    }
    if(dvLinkGetExportModule(_Link) != dvModuleNull && dvLinkGetExportModule(_Link) != Module) {
        utExit("Delete Link from non-owning Module");
    }
#endif
    pLink = dvLinkNull;
    for(nLink = dvModuleGetFirstExportLink(Module); nLink != dvLinkNull && nLink != _Link;
            nLink = dvLinkGetNextModuleExportLink(nLink)) {
        pLink = nLink;
    }
    if(pLink != dvLinkNull) {
        dvLinkSetNextModuleExportLink(pLink, dvLinkGetNextModuleExportLink(_Link));
    } else {
        dvModuleSetFirstExportLink(Module, dvLinkGetNextModuleExportLink(_Link));
    }
    dvLinkSetNextModuleExportLink(_Link, dvLinkNull);
    if(dvModuleGetLastExportLink(Module) == _Link) {
        dvModuleSetLastExportLink(Module, pLink);
    }
    dvLinkSetExportModule(_Link, dvModuleNull);
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void dvShowModule(
    dvModule Module)
{
    utDatabaseShowObject("dv", "Module", dvModule2Index(Module));
}
#endif

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocLink(void)
{
    dvLink Link = dvLinkAlloc();

    return dvLink2Index(Link);
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Link.
----------------------------------------------------------------------------------------*/
static void allocLinks(void)
{
    dvSetAllocatedLink(2);
    dvSetUsedLink(1);
    dvLinks.ImportModule = utNewAInitFirst(dvModule, (dvAllocatedLink()));
    dvLinks.NextModuleImportLink = utNewAInitFirst(dvLink, (dvAllocatedLink()));
    dvLinks.ExportModule = utNewAInitFirst(dvModule, (dvAllocatedLink()));
    dvLinks.NextModuleExportLink = utNewAInitFirst(dvLink, (dvAllocatedLink()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Link.
----------------------------------------------------------------------------------------*/
static void reallocLinks(
    uint32 newSize)
{
    utResizeArray(dvLinks.ImportModule, (newSize));
    utResizeArray(dvLinks.NextModuleImportLink, (newSize));
    utResizeArray(dvLinks.ExportModule, (newSize));
    utResizeArray(dvLinks.NextModuleExportLink, (newSize));
    dvSetAllocatedLink(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Links.
----------------------------------------------------------------------------------------*/
void dvLinkAllocMore(void)
{
    reallocLinks((uint32)(dvAllocatedLink() + (dvAllocatedLink() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Link.
----------------------------------------------------------------------------------------*/
void dvLinkCopyProps(
    dvLink oldLink,
    dvLink newLink)
{
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void dvShowLink(
    dvLink Link)
{
    utDatabaseShowObject("dv", "Link", dvLink2Index(Link));
}
#endif

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocSchema(void)
{
    dvSchema Schema = dvSchemaAlloc();

    return dvSchema2Index(Schema);
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Schema.
----------------------------------------------------------------------------------------*/
static void allocSchemas(void)
{
    dvSetAllocatedSchema(2);
    dvSetUsedSchema(1);
    dvSchemas.Sym = utNewAInitFirst(utSym, (dvAllocatedSchema()));
    dvSchemas.Module = utNewAInitFirst(dvModule, (dvAllocatedSchema()));
    dvSchemas.NextModuleSchema = utNewAInitFirst(dvSchema, (dvAllocatedSchema()));
    dvSchemas.PrevModuleSchema = utNewAInitFirst(dvSchema, (dvAllocatedSchema()));
    dvSchemas.NextTableModuleSchema = utNewAInitFirst(dvSchema, (dvAllocatedSchema()));
    dvSchemas.FirstRelationship = utNewAInitFirst(dvRelationship, (dvAllocatedSchema()));
    dvSchemas.LastRelationship = utNewAInitFirst(dvRelationship, (dvAllocatedSchema()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Schema.
----------------------------------------------------------------------------------------*/
static void reallocSchemas(
    uint32 newSize)
{
    utResizeArray(dvSchemas.Sym, (newSize));
    utResizeArray(dvSchemas.Module, (newSize));
    utResizeArray(dvSchemas.NextModuleSchema, (newSize));
    utResizeArray(dvSchemas.PrevModuleSchema, (newSize));
    utResizeArray(dvSchemas.NextTableModuleSchema, (newSize));
    utResizeArray(dvSchemas.FirstRelationship, (newSize));
    utResizeArray(dvSchemas.LastRelationship, (newSize));
    dvSetAllocatedSchema(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Schemas.
----------------------------------------------------------------------------------------*/
void dvSchemaAllocMore(void)
{
    reallocSchemas((uint32)(dvAllocatedSchema() + (dvAllocatedSchema() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Schema.
----------------------------------------------------------------------------------------*/
void dvSchemaCopyProps(
    dvSchema oldSchema,
    dvSchema newSchema)
{
}

/*----------------------------------------------------------------------------------------
  Add the Relationship to the head of the list on the Schema.
----------------------------------------------------------------------------------------*/
void dvSchemaInsertRelationship(
    dvSchema Schema,
    dvRelationship _Relationship)
{
#if defined(DD_DEBUG)
    if(Schema == dvSchemaNull) {
        utExit("Non-existent Schema");
    }
    if(_Relationship == dvRelationshipNull) {
        utExit("Non-existent Relationship");
    }
    if(dvRelationshipGetSchema(_Relationship) != dvSchemaNull) {
        utExit("Attempting to add Relationship to Schema twice");
    }
#endif
    dvRelationshipSetNextSchemaRelationship(_Relationship, dvSchemaGetFirstRelationship(Schema));
    dvSchemaSetFirstRelationship(Schema, _Relationship);
    if(dvSchemaGetLastRelationship(Schema) == dvRelationshipNull) {
        dvSchemaSetLastRelationship(Schema, _Relationship);
    }
    dvRelationshipSetSchema(_Relationship, Schema);
}

/*----------------------------------------------------------------------------------------
  Add the Relationship to the end of the list on the Schema.
----------------------------------------------------------------------------------------*/
void dvSchemaAppendRelationship(
    dvSchema Schema,
    dvRelationship _Relationship)
{
#if defined(DD_DEBUG)
    if(Schema == dvSchemaNull) {
        utExit("Non-existent Schema");
    }
    if(_Relationship == dvRelationshipNull) {
        utExit("Non-existent Relationship");
    }
    if(dvRelationshipGetSchema(_Relationship) != dvSchemaNull) {
        utExit("Attempting to add Relationship to Schema twice");
    }
#endif
    if(dvSchemaGetLastRelationship(Schema) != dvRelationshipNull) {
        dvRelationshipSetNextSchemaRelationship(dvSchemaGetLastRelationship(Schema), _Relationship);
    } else {
        dvSchemaSetFirstRelationship(Schema, _Relationship);
    }
    dvSchemaSetLastRelationship(Schema, _Relationship);
    dvRelationshipSetNextSchemaRelationship(_Relationship, dvRelationshipNull);
    dvRelationshipSetSchema(_Relationship, Schema);
}

/*----------------------------------------------------------------------------------------
  Insert the Relationship to the Schema after the previous Relationship.
----------------------------------------------------------------------------------------*/
void dvSchemaInsertAfterRelationship(
    dvSchema Schema,
    dvRelationship prevRelationship,
    dvRelationship _Relationship)
{
    dvRelationship nextRelationship = dvRelationshipGetNextSchemaRelationship(prevRelationship);

#if defined(DD_DEBUG)
    if(Schema == dvSchemaNull) {
        utExit("Non-existent Schema");
    }
    if(_Relationship == dvRelationshipNull) {
        utExit("Non-existent Relationship");
    }
    if(dvRelationshipGetSchema(_Relationship) != dvSchemaNull) {
        utExit("Attempting to add Relationship to Schema twice");
    }
#endif
    dvRelationshipSetNextSchemaRelationship(_Relationship, nextRelationship);
    dvRelationshipSetNextSchemaRelationship(prevRelationship, _Relationship);
    if(dvSchemaGetLastRelationship(Schema) == prevRelationship) {
        dvSchemaSetLastRelationship(Schema, _Relationship);
    }
    dvRelationshipSetSchema(_Relationship, Schema);
}

/*----------------------------------------------------------------------------------------
 Remove the Relationship from the Schema.
----------------------------------------------------------------------------------------*/
void dvSchemaRemoveRelationship(
    dvSchema Schema,
    dvRelationship _Relationship)
{
    dvRelationship pRelationship, nRelationship;

#if defined(DD_DEBUG)
    if(_Relationship == dvRelationshipNull) {
        utExit("Non-existent Relationship");
    }
    if(dvRelationshipGetSchema(_Relationship) != dvSchemaNull && dvRelationshipGetSchema(_Relationship) != Schema) {
        utExit("Delete Relationship from non-owning Schema");
    }
#endif
    pRelationship = dvRelationshipNull;
    for(nRelationship = dvSchemaGetFirstRelationship(Schema); nRelationship != dvRelationshipNull && nRelationship != _Relationship;
            nRelationship = dvRelationshipGetNextSchemaRelationship(nRelationship)) {
        pRelationship = nRelationship;
    }
    if(pRelationship != dvRelationshipNull) {
        dvRelationshipSetNextSchemaRelationship(pRelationship, dvRelationshipGetNextSchemaRelationship(_Relationship));
    } else {
        dvSchemaSetFirstRelationship(Schema, dvRelationshipGetNextSchemaRelationship(_Relationship));
    }
    dvRelationshipSetNextSchemaRelationship(_Relationship, dvRelationshipNull);
    if(dvSchemaGetLastRelationship(Schema) == _Relationship) {
        dvSchemaSetLastRelationship(Schema, pRelationship);
    }
    dvRelationshipSetSchema(_Relationship, dvSchemaNull);
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void dvShowSchema(
    dvSchema Schema)
{
    utDatabaseShowObject("dv", "Schema", dvSchema2Index(Schema));
}
#endif

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocEnum(void)
{
    dvEnum Enum = dvEnumAlloc();

    return dvEnum2Index(Enum);
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Enum.
----------------------------------------------------------------------------------------*/
static void allocEnums(void)
{
    dvSetAllocatedEnum(2);
    dvSetUsedEnum(1);
    dvEnums.Sym = utNewAInitFirst(utSym, (dvAllocatedEnum()));
    dvEnums.PrefixSym = utNewAInitFirst(utSym, (dvAllocatedEnum()));
    dvEnums.NumEntries = utNewAInitFirst(uint16, (dvAllocatedEnum()));
    dvEnums.Module = utNewAInitFirst(dvModule, (dvAllocatedEnum()));
    dvEnums.NextModuleEnum = utNewAInitFirst(dvEnum, (dvAllocatedEnum()));
    dvEnums.PrevModuleEnum = utNewAInitFirst(dvEnum, (dvAllocatedEnum()));
    dvEnums.NextTableModuleEnum = utNewAInitFirst(dvEnum, (dvAllocatedEnum()));
    dvEnums.FirstEntry = utNewAInitFirst(dvEntry, (dvAllocatedEnum()));
    dvEnums.LastEntry = utNewAInitFirst(dvEntry, (dvAllocatedEnum()));
    dvEnums.EntryTableIndex_ = utNewAInitFirst(uint32, (dvAllocatedEnum()));
    dvEnums.NumEntryTable = utNewAInitFirst(uint32, (dvAllocatedEnum()));
    dvSetUsedEnumEntryTable(0);
    dvSetAllocatedEnumEntryTable(2);
    dvSetFreeEnumEntryTable(0);
    dvEnums.EntryTable = utNewAInitFirst(dvEntry, dvAllocatedEnumEntryTable());
    dvEnums.NumEntry = utNewAInitFirst(uint32, (dvAllocatedEnum()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Enum.
----------------------------------------------------------------------------------------*/
static void reallocEnums(
    uint32 newSize)
{
    utResizeArray(dvEnums.Sym, (newSize));
    utResizeArray(dvEnums.PrefixSym, (newSize));
    utResizeArray(dvEnums.NumEntries, (newSize));
    utResizeArray(dvEnums.Module, (newSize));
    utResizeArray(dvEnums.NextModuleEnum, (newSize));
    utResizeArray(dvEnums.PrevModuleEnum, (newSize));
    utResizeArray(dvEnums.NextTableModuleEnum, (newSize));
    utResizeArray(dvEnums.FirstEntry, (newSize));
    utResizeArray(dvEnums.LastEntry, (newSize));
    utResizeArray(dvEnums.EntryTableIndex_, (newSize));
    utResizeArray(dvEnums.NumEntryTable, (newSize));
    utResizeArray(dvEnums.NumEntry, (newSize));
    dvSetAllocatedEnum(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Enums.
----------------------------------------------------------------------------------------*/
void dvEnumAllocMore(void)
{
    reallocEnums((uint32)(dvAllocatedEnum() + (dvAllocatedEnum() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Compact the Enum.EntryTable heap to free memory.
----------------------------------------------------------------------------------------*/
void dvCompactEnumEntryTables(void)
{
    uint32 elementSize = sizeof(dvEntry);
    uint32 usedHeaderSize = (sizeof(dvEnum) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvEnum) + sizeof(uint32) + elementSize - 1)/elementSize;
    dvEntry *toPtr = dvEnums.EntryTable;
    dvEntry *fromPtr = toPtr;
    dvEnum Enum;
    uint32 size;

    while(fromPtr < dvEnums.EntryTable + dvUsedEnumEntryTable()) {
        Enum = *(dvEnum *)(void *)fromPtr;
        if(Enum != dvEnumNull) {
            /* Need to move it to toPtr */
            size = utMax(dvEnumGetNumEntryTable(Enum) + usedHeaderSize, freeHeaderSize);
            memmove((void *)toPtr, (void *)fromPtr, size*elementSize);
            dvEnumSetEntryTableIndex_(Enum, toPtr - dvEnums.EntryTable + usedHeaderSize);
            toPtr += size;
        } else {
            /* Just skip it */
            size = *(uint32 *)(void *)(((dvEnum *)(void *)fromPtr) + 1);
        }
        fromPtr += size;
    }
    dvSetUsedEnumEntryTable(toPtr - dvEnums.EntryTable);
    dvSetFreeEnumEntryTable(0);
}

/*----------------------------------------------------------------------------------------
  Allocate more memory for the Enum.EntryTable heap.
----------------------------------------------------------------------------------------*/
static void allocMoreEnumEntryTables(
    uint32 spaceNeeded)
{
    uint32 freeSpace = dvAllocatedEnumEntryTable() - dvUsedEnumEntryTable();

    if((dvFreeEnumEntryTable() << 2) > dvUsedEnumEntryTable()) {
        dvCompactEnumEntryTables();
        freeSpace = dvAllocatedEnumEntryTable() - dvUsedEnumEntryTable();
    }
    if(freeSpace < spaceNeeded) {
        dvSetAllocatedEnumEntryTable(dvAllocatedEnumEntryTable() + spaceNeeded - freeSpace +
            (dvAllocatedEnumEntryTable() >> 1));
        utResizeArray(dvEnums.EntryTable, dvAllocatedEnumEntryTable());
    }
}

/*----------------------------------------------------------------------------------------
  Allocate memory for a new Enum.EntryTable array.
----------------------------------------------------------------------------------------*/
void dvEnumAllocEntryTables(
    dvEnum Enum,
    uint32 numEntryTables)
{
    uint32 freeSpace = dvAllocatedEnumEntryTable() - dvUsedEnumEntryTable();
    uint32 elementSize = sizeof(dvEntry);
    uint32 usedHeaderSize = (sizeof(dvEnum) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvEnum) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 spaceNeeded = utMax(numEntryTables + usedHeaderSize, freeHeaderSize);

#if defined(DD_DEBUG)
    utAssert(dvEnumGetNumEntryTable(Enum) == 0);
#endif
    if(numEntryTables == 0) {
        return;
    }
    if(freeSpace < spaceNeeded) {
        allocMoreEnumEntryTables(spaceNeeded);
    }
    dvEnumSetEntryTableIndex_(Enum, dvUsedEnumEntryTable() + usedHeaderSize);
    dvEnumSetNumEntryTable(Enum, numEntryTables);
    *(dvEnum *)(void *)(dvEnums.EntryTable + dvUsedEnumEntryTable()) = Enum;
    {
        uint32 xValue;
        for(xValue = (uint32)(dvEnumGetEntryTableIndex_(Enum)); xValue < dvEnumGetEntryTableIndex_(Enum) + numEntryTables; xValue++) {
            dvEnums.EntryTable[xValue] = dvEntryNull;
        }
    }
    dvSetUsedEnumEntryTable(dvUsedEnumEntryTable() + spaceNeeded);
}

/*----------------------------------------------------------------------------------------
  Wrapper around dvEnumGetEntryTables for the database manager.
----------------------------------------------------------------------------------------*/
static void *getEnumEntryTables(
    uint64 objectNumber,
    uint32 *numValues)
{
    dvEnum Enum = dvIndex2Enum((uint32)objectNumber);

    *numValues = dvEnumGetNumEntryTable(Enum);
    return dvEnumGetEntryTables(Enum);
}

/*----------------------------------------------------------------------------------------
  Wrapper around dvEnumAllocEntryTables for the database manager.
----------------------------------------------------------------------------------------*/
static void *allocEnumEntryTables(
    uint64 objectNumber,
    uint32 numValues)
{
    dvEnum Enum = dvIndex2Enum((uint32)objectNumber);

    dvEnumSetEntryTableIndex_(Enum, 0);
    dvEnumSetNumEntryTable(Enum, 0);
    if(numValues == 0) {
        return NULL;
    }
    dvEnumAllocEntryTables(Enum, numValues);
    return dvEnumGetEntryTables(Enum);
}

/*----------------------------------------------------------------------------------------
  Free memory used by the Enum.EntryTable array.
----------------------------------------------------------------------------------------*/
void dvEnumFreeEntryTables(
    dvEnum Enum)
{
    uint32 elementSize = sizeof(dvEntry);
    uint32 usedHeaderSize = (sizeof(dvEnum) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvEnum) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 size = utMax(dvEnumGetNumEntryTable(Enum) + usedHeaderSize, freeHeaderSize);
    dvEntry *dataPtr = dvEnumGetEntryTables(Enum) - usedHeaderSize;

    if(dvEnumGetNumEntryTable(Enum) == 0) {
        return;
    }
    *(dvEnum *)(void *)(dataPtr) = dvEnumNull;
    *(uint32 *)(void *)(((dvEnum *)(void *)dataPtr) + 1) = size;
    dvEnumSetNumEntryTable(Enum, 0);
    dvSetFreeEnumEntryTable(dvFreeEnumEntryTable() + size);
}

/*----------------------------------------------------------------------------------------
  Resize the Enum.EntryTable array.
----------------------------------------------------------------------------------------*/
void dvEnumResizeEntryTables(
    dvEnum Enum,
    uint32 numEntryTables)
{
    uint32 freeSpace;
    uint32 elementSize = sizeof(dvEntry);
    uint32 usedHeaderSize = (sizeof(dvEnum) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvEnum) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 newSize = utMax(numEntryTables + usedHeaderSize, freeHeaderSize);
    uint32 oldSize = utMax(dvEnumGetNumEntryTable(Enum) + usedHeaderSize, freeHeaderSize);
    dvEntry *dataPtr;

    if(numEntryTables == 0) {
        if(dvEnumGetNumEntryTable(Enum) != 0) {
            dvEnumFreeEntryTables(Enum);
        }
        return;
    }
    if(dvEnumGetNumEntryTable(Enum) == 0) {
        dvEnumAllocEntryTables(Enum, numEntryTables);
        return;
    }
    freeSpace = dvAllocatedEnumEntryTable() - dvUsedEnumEntryTable();
    if(freeSpace < newSize) {
        allocMoreEnumEntryTables(newSize);
    }
    dataPtr = dvEnumGetEntryTables(Enum) - usedHeaderSize;
    memcpy((void *)(dvEnums.EntryTable + dvUsedEnumEntryTable()), dataPtr,
        elementSize*utMin(oldSize, newSize));
    if(newSize > oldSize) {
        {
            uint32 xValue;
            for(xValue = (uint32)(dvUsedEnumEntryTable() + oldSize); xValue < dvUsedEnumEntryTable() + oldSize + newSize - oldSize; xValue++) {
                dvEnums.EntryTable[xValue] = dvEntryNull;
            }
        }
    }
    *(dvEnum *)(void *)dataPtr = dvEnumNull;
    *(uint32 *)(void *)(((dvEnum *)(void *)dataPtr) + 1) = oldSize;
    dvSetFreeEnumEntryTable(dvFreeEnumEntryTable() + oldSize);
    dvEnumSetEntryTableIndex_(Enum, dvUsedEnumEntryTable() + usedHeaderSize);
    dvEnumSetNumEntryTable(Enum, numEntryTables);
    dvSetUsedEnumEntryTable(dvUsedEnumEntryTable() + newSize);
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Enum.
----------------------------------------------------------------------------------------*/
void dvEnumCopyProps(
    dvEnum oldEnum,
    dvEnum newEnum)
{
    dvEnumSetPrefixSym(newEnum, dvEnumGetPrefixSym(oldEnum));
    dvEnumSetNumEntries(newEnum, dvEnumGetNumEntries(oldEnum));
}

/*----------------------------------------------------------------------------------------
  Increase the size of the hash table.
----------------------------------------------------------------------------------------*/
static void resizeEnumEntryHashTable(
    dvEnum Enum)
{
    dvEntry _Entry, prevEntry, nextEntry;
    uint32 oldNumEntrys = dvEnumGetNumEntryTable(Enum);
    uint32 newNumEntrys = oldNumEntrys << 1;
    uint32 xEntry, index;

    if(newNumEntrys == 0) {
        newNumEntrys = 2;
        dvEnumAllocEntryTables(Enum, 2);
    } else {
        dvEnumResizeEntryTables(Enum, newNumEntrys);
    }
    for(xEntry = 0; xEntry < oldNumEntrys; xEntry++) {
        _Entry = dvEnumGetiEntryTable(Enum, xEntry);
        prevEntry = dvEntryNull;
        while(_Entry != dvEntryNull) {
            nextEntry = dvEntryGetNextTableEnumEntry(_Entry);
            index = (newNumEntrys - 1) & utSymGetHashValue(dvEntryGetSym(_Entry));
            if(index != xEntry) {
                if(prevEntry == dvEntryNull) {
                    dvEnumSetiEntryTable(Enum, xEntry, nextEntry);
                } else {
                    dvEntrySetNextTableEnumEntry(prevEntry, nextEntry);
                }
                dvEntrySetNextTableEnumEntry(_Entry, dvEnumGetiEntryTable(Enum, index));
                dvEnumSetiEntryTable(Enum, index, _Entry);
            } else {
                prevEntry = _Entry;
            }
            _Entry = nextEntry;
        }
    }
}

/*----------------------------------------------------------------------------------------
  Add the Entry to the Enum.  If the table is near full, build a new one twice
  as big, delete the old one, and return the new one.
----------------------------------------------------------------------------------------*/
static void addEnumEntryToHashTable(
    dvEnum Enum,
    dvEntry _Entry)
{
    dvEntry nextEntry;
    uint32 index;

    if(dvEnumGetNumEntry(Enum) >> 1 >= dvEnumGetNumEntryTable(Enum)) {
        resizeEnumEntryHashTable(Enum);
    }
    index = (dvEnumGetNumEntryTable(Enum) - 1) & utSymGetHashValue(dvEntryGetSym(_Entry));
    nextEntry = dvEnumGetiEntryTable(Enum, index);
    dvEntrySetNextTableEnumEntry(_Entry, nextEntry);
    dvEnumSetiEntryTable(Enum, index, _Entry);
    dvEnumSetNumEntry(Enum, dvEnumGetNumEntry(Enum) + 1);
}

/*----------------------------------------------------------------------------------------
  Remove the Entry from the hash table.
----------------------------------------------------------------------------------------*/
static void removeEnumEntryFromHashTable(
    dvEnum Enum,
    dvEntry _Entry)
{
    uint32 index = (dvEnumGetNumEntryTable(Enum) - 1) & utSymGetHashValue(dvEntryGetSym(_Entry));
    dvEntry prevEntry, nextEntry;
    
    nextEntry = dvEnumGetiEntryTable(Enum, index);
    if(nextEntry == _Entry) {
        dvEnumSetiEntryTable(Enum, index, dvEntryGetNextTableEnumEntry(nextEntry));
    } else {
        do {
            prevEntry = nextEntry;
            nextEntry = dvEntryGetNextTableEnumEntry(nextEntry);
        } while(nextEntry != _Entry);
        dvEntrySetNextTableEnumEntry(prevEntry, dvEntryGetNextTableEnumEntry(_Entry));
    }
    dvEnumSetNumEntry(Enum, dvEnumGetNumEntry(Enum) - 1);
    dvEntrySetNextTableEnumEntry(_Entry, dvEntryNull);
}

/*----------------------------------------------------------------------------------------
  Find the Entry from the Enum and its hash key.
----------------------------------------------------------------------------------------*/
dvEntry dvEnumFindEntry(
    dvEnum Enum,
    utSym Sym)
{
    uint32 mask = dvEnumGetNumEntryTable(Enum) - 1;
    dvEntry _Entry;

    if(mask + 1 != 0) {
        _Entry = dvEnumGetiEntryTable(Enum, utSymGetHashValue(Sym) & mask);
        while(_Entry != dvEntryNull) {
            if(dvEntryGetSym(_Entry) == Sym) {
                return _Entry;
            }
            _Entry = dvEntryGetNextTableEnumEntry(_Entry);
        }
    }
    return dvEntryNull;
}

/*----------------------------------------------------------------------------------------
  Find the Entry from the Enum and its name.
----------------------------------------------------------------------------------------*/
void dvEnumRenameEntry(
    dvEnum Enum,
    dvEntry _Entry,
    utSym sym)
{
    if(dvEntryGetSym(_Entry) != utSymNull) {
        removeEnumEntryFromHashTable(Enum, _Entry);
    }
    dvEntrySetSym(_Entry, sym);
    if(sym != utSymNull) {
        addEnumEntryToHashTable(Enum, _Entry);
    }
}

/*----------------------------------------------------------------------------------------
  Add the Entry to the head of the list on the Enum.
----------------------------------------------------------------------------------------*/
void dvEnumInsertEntry(
    dvEnum Enum,
    dvEntry _Entry)
{
#if defined(DD_DEBUG)
    if(Enum == dvEnumNull) {
        utExit("Non-existent Enum");
    }
    if(_Entry == dvEntryNull) {
        utExit("Non-existent Entry");
    }
    if(dvEntryGetEnum(_Entry) != dvEnumNull) {
        utExit("Attempting to add Entry to Enum twice");
    }
#endif
    dvEntrySetNextEnumEntry(_Entry, dvEnumGetFirstEntry(Enum));
    if(dvEnumGetFirstEntry(Enum) != dvEntryNull) {
        dvEntrySetPrevEnumEntry(dvEnumGetFirstEntry(Enum), _Entry);
    }
    dvEnumSetFirstEntry(Enum, _Entry);
    dvEntrySetPrevEnumEntry(_Entry, dvEntryNull);
    if(dvEnumGetLastEntry(Enum) == dvEntryNull) {
        dvEnumSetLastEntry(Enum, _Entry);
    }
    dvEntrySetEnum(_Entry, Enum);
    if(dvEntryGetSym(_Entry) != utSymNull) {
        addEnumEntryToHashTable(Enum, _Entry);
    }
}

/*----------------------------------------------------------------------------------------
  Add the Entry to the end of the list on the Enum.
----------------------------------------------------------------------------------------*/
void dvEnumAppendEntry(
    dvEnum Enum,
    dvEntry _Entry)
{
#if defined(DD_DEBUG)
    if(Enum == dvEnumNull) {
        utExit("Non-existent Enum");
    }
    if(_Entry == dvEntryNull) {
        utExit("Non-existent Entry");
    }
    if(dvEntryGetEnum(_Entry) != dvEnumNull) {
        utExit("Attempting to add Entry to Enum twice");
    }
#endif
    dvEntrySetPrevEnumEntry(_Entry, dvEnumGetLastEntry(Enum));
    if(dvEnumGetLastEntry(Enum) != dvEntryNull) {
        dvEntrySetNextEnumEntry(dvEnumGetLastEntry(Enum), _Entry);
    }
    dvEnumSetLastEntry(Enum, _Entry);
    dvEntrySetNextEnumEntry(_Entry, dvEntryNull);
    if(dvEnumGetFirstEntry(Enum) == dvEntryNull) {
        dvEnumSetFirstEntry(Enum, _Entry);
    }
    dvEntrySetEnum(_Entry, Enum);
    if(dvEntryGetSym(_Entry) != utSymNull) {
        addEnumEntryToHashTable(Enum, _Entry);
    }
}

/*----------------------------------------------------------------------------------------
  Insert the Entry to the Enum after the previous Entry.
----------------------------------------------------------------------------------------*/
void dvEnumInsertAfterEntry(
    dvEnum Enum,
    dvEntry prevEntry,
    dvEntry _Entry)
{
    dvEntry nextEntry = dvEntryGetNextEnumEntry(prevEntry);

#if defined(DD_DEBUG)
    if(Enum == dvEnumNull) {
        utExit("Non-existent Enum");
    }
    if(_Entry == dvEntryNull) {
        utExit("Non-existent Entry");
    }
    if(dvEntryGetEnum(_Entry) != dvEnumNull) {
        utExit("Attempting to add Entry to Enum twice");
    }
#endif
    dvEntrySetNextEnumEntry(_Entry, nextEntry);
    dvEntrySetNextEnumEntry(prevEntry, _Entry);
    dvEntrySetPrevEnumEntry(_Entry, prevEntry);
    if(nextEntry != dvEntryNull) {
        dvEntrySetPrevEnumEntry(nextEntry, _Entry);
    }
    if(dvEnumGetLastEntry(Enum) == prevEntry) {
        dvEnumSetLastEntry(Enum, _Entry);
    }
    dvEntrySetEnum(_Entry, Enum);
    if(dvEntryGetSym(_Entry) != utSymNull) {
        addEnumEntryToHashTable(Enum, _Entry);
    }
}

/*----------------------------------------------------------------------------------------
 Remove the Entry from the Enum.
----------------------------------------------------------------------------------------*/
void dvEnumRemoveEntry(
    dvEnum Enum,
    dvEntry _Entry)
{
    dvEntry pEntry, nEntry;

#if defined(DD_DEBUG)
    if(_Entry == dvEntryNull) {
        utExit("Non-existent Entry");
    }
    if(dvEntryGetEnum(_Entry) != dvEnumNull && dvEntryGetEnum(_Entry) != Enum) {
        utExit("Delete Entry from non-owning Enum");
    }
#endif
    nEntry = dvEntryGetNextEnumEntry(_Entry);
    pEntry = dvEntryGetPrevEnumEntry(_Entry);
    if(pEntry != dvEntryNull) {
        dvEntrySetNextEnumEntry(pEntry, nEntry);
    } else if(dvEnumGetFirstEntry(Enum) == _Entry) {
        dvEnumSetFirstEntry(Enum, nEntry);
    }
    if(nEntry != dvEntryNull) {
        dvEntrySetPrevEnumEntry(nEntry, pEntry);
    } else if(dvEnumGetLastEntry(Enum) == _Entry) {
        dvEnumSetLastEntry(Enum, pEntry);
    }
    dvEntrySetNextEnumEntry(_Entry, dvEntryNull);
    dvEntrySetPrevEnumEntry(_Entry, dvEntryNull);
    dvEntrySetEnum(_Entry, dvEnumNull);
    if(dvEntryGetSym(_Entry) != utSymNull) {
        removeEnumEntryFromHashTable(Enum, _Entry);
    }
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void dvShowEnum(
    dvEnum Enum)
{
    utDatabaseShowObject("dv", "Enum", dvEnum2Index(Enum));
}
#endif

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocEntry(void)
{
    dvEntry Entry = dvEntryAlloc();

    return dvEntry2Index(Entry);
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Entry.
----------------------------------------------------------------------------------------*/
static void allocEntrys(void)
{
    dvSetAllocatedEntry(2);
    dvSetUsedEntry(1);
    dvEntrys.Sym = utNewAInitFirst(utSym, (dvAllocatedEntry()));
    dvEntrys.Value = utNewAInitFirst(uint32, (dvAllocatedEntry()));
    dvEntrys.Enum = utNewAInitFirst(dvEnum, (dvAllocatedEntry()));
    dvEntrys.NextEnumEntry = utNewAInitFirst(dvEntry, (dvAllocatedEntry()));
    dvEntrys.PrevEnumEntry = utNewAInitFirst(dvEntry, (dvAllocatedEntry()));
    dvEntrys.NextTableEnumEntry = utNewAInitFirst(dvEntry, (dvAllocatedEntry()));
    dvEntrys.FirstCase = utNewAInitFirst(dvCase, (dvAllocatedEntry()));
    dvEntrys.LastCase = utNewAInitFirst(dvCase, (dvAllocatedEntry()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Entry.
----------------------------------------------------------------------------------------*/
static void reallocEntrys(
    uint32 newSize)
{
    utResizeArray(dvEntrys.Sym, (newSize));
    utResizeArray(dvEntrys.Value, (newSize));
    utResizeArray(dvEntrys.Enum, (newSize));
    utResizeArray(dvEntrys.NextEnumEntry, (newSize));
    utResizeArray(dvEntrys.PrevEnumEntry, (newSize));
    utResizeArray(dvEntrys.NextTableEnumEntry, (newSize));
    utResizeArray(dvEntrys.FirstCase, (newSize));
    utResizeArray(dvEntrys.LastCase, (newSize));
    dvSetAllocatedEntry(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Entrys.
----------------------------------------------------------------------------------------*/
void dvEntryAllocMore(void)
{
    reallocEntrys((uint32)(dvAllocatedEntry() + (dvAllocatedEntry() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Entry.
----------------------------------------------------------------------------------------*/
void dvEntryCopyProps(
    dvEntry oldEntry,
    dvEntry newEntry)
{
    dvEntrySetValue(newEntry, dvEntryGetValue(oldEntry));
}

/*----------------------------------------------------------------------------------------
  Add the Case to the head of the list on the Entry.
----------------------------------------------------------------------------------------*/
void dvEntryInsertCase(
    dvEntry Entry,
    dvCase _Case)
{
#if defined(DD_DEBUG)
    if(Entry == dvEntryNull) {
        utExit("Non-existent Entry");
    }
    if(_Case == dvCaseNull) {
        utExit("Non-existent Case");
    }
    if(dvCaseGetEntry(_Case) != dvEntryNull) {
        utExit("Attempting to add Case to Entry twice");
    }
#endif
    dvCaseSetNextEntryCase(_Case, dvEntryGetFirstCase(Entry));
    dvEntrySetFirstCase(Entry, _Case);
    if(dvEntryGetLastCase(Entry) == dvCaseNull) {
        dvEntrySetLastCase(Entry, _Case);
    }
    dvCaseSetEntry(_Case, Entry);
}

/*----------------------------------------------------------------------------------------
  Add the Case to the end of the list on the Entry.
----------------------------------------------------------------------------------------*/
void dvEntryAppendCase(
    dvEntry Entry,
    dvCase _Case)
{
#if defined(DD_DEBUG)
    if(Entry == dvEntryNull) {
        utExit("Non-existent Entry");
    }
    if(_Case == dvCaseNull) {
        utExit("Non-existent Case");
    }
    if(dvCaseGetEntry(_Case) != dvEntryNull) {
        utExit("Attempting to add Case to Entry twice");
    }
#endif
    if(dvEntryGetLastCase(Entry) != dvCaseNull) {
        dvCaseSetNextEntryCase(dvEntryGetLastCase(Entry), _Case);
    } else {
        dvEntrySetFirstCase(Entry, _Case);
    }
    dvEntrySetLastCase(Entry, _Case);
    dvCaseSetNextEntryCase(_Case, dvCaseNull);
    dvCaseSetEntry(_Case, Entry);
}

/*----------------------------------------------------------------------------------------
  Insert the Case to the Entry after the previous Case.
----------------------------------------------------------------------------------------*/
void dvEntryInsertAfterCase(
    dvEntry Entry,
    dvCase prevCase,
    dvCase _Case)
{
    dvCase nextCase = dvCaseGetNextEntryCase(prevCase);

#if defined(DD_DEBUG)
    if(Entry == dvEntryNull) {
        utExit("Non-existent Entry");
    }
    if(_Case == dvCaseNull) {
        utExit("Non-existent Case");
    }
    if(dvCaseGetEntry(_Case) != dvEntryNull) {
        utExit("Attempting to add Case to Entry twice");
    }
#endif
    dvCaseSetNextEntryCase(_Case, nextCase);
    dvCaseSetNextEntryCase(prevCase, _Case);
    if(dvEntryGetLastCase(Entry) == prevCase) {
        dvEntrySetLastCase(Entry, _Case);
    }
    dvCaseSetEntry(_Case, Entry);
}

/*----------------------------------------------------------------------------------------
 Remove the Case from the Entry.
----------------------------------------------------------------------------------------*/
void dvEntryRemoveCase(
    dvEntry Entry,
    dvCase _Case)
{
    dvCase pCase, nCase;

#if defined(DD_DEBUG)
    if(_Case == dvCaseNull) {
        utExit("Non-existent Case");
    }
    if(dvCaseGetEntry(_Case) != dvEntryNull && dvCaseGetEntry(_Case) != Entry) {
        utExit("Delete Case from non-owning Entry");
    }
#endif
    pCase = dvCaseNull;
    for(nCase = dvEntryGetFirstCase(Entry); nCase != dvCaseNull && nCase != _Case;
            nCase = dvCaseGetNextEntryCase(nCase)) {
        pCase = nCase;
    }
    if(pCase != dvCaseNull) {
        dvCaseSetNextEntryCase(pCase, dvCaseGetNextEntryCase(_Case));
    } else {
        dvEntrySetFirstCase(Entry, dvCaseGetNextEntryCase(_Case));
    }
    dvCaseSetNextEntryCase(_Case, dvCaseNull);
    if(dvEntryGetLastCase(Entry) == _Case) {
        dvEntrySetLastCase(Entry, pCase);
    }
    dvCaseSetEntry(_Case, dvEntryNull);
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void dvShowEntry(
    dvEntry Entry)
{
    utDatabaseShowObject("dv", "Entry", dvEntry2Index(Entry));
}
#endif

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocTypedef(void)
{
    dvTypedef Typedef = dvTypedefAlloc();

    return dvTypedef2Index(Typedef);
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Typedef.
----------------------------------------------------------------------------------------*/
static void allocTypedefs(void)
{
    dvSetAllocatedTypedef(2);
    dvSetUsedTypedef(1);
    dvTypedefs.Sym = utNewAInitFirst(utSym, (dvAllocatedTypedef()));
    dvTypedefs.InitializerIndex_ = utNewAInitFirst(uint32, (dvAllocatedTypedef()));
    dvTypedefs.NumInitializer = utNewAInitFirst(uint32, (dvAllocatedTypedef()));
    dvSetUsedTypedefInitializer(0);
    dvSetAllocatedTypedefInitializer(2);
    dvSetFreeTypedefInitializer(0);
    dvTypedefs.Initializer = utNewAInitFirst(char, dvAllocatedTypedefInitializer());
    dvTypedefs.Module = utNewAInitFirst(dvModule, (dvAllocatedTypedef()));
    dvTypedefs.NextModuleTypedef = utNewAInitFirst(dvTypedef, (dvAllocatedTypedef()));
    dvTypedefs.PrevModuleTypedef = utNewAInitFirst(dvTypedef, (dvAllocatedTypedef()));
    dvTypedefs.NextTableModuleTypedef = utNewAInitFirst(dvTypedef, (dvAllocatedTypedef()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Typedef.
----------------------------------------------------------------------------------------*/
static void reallocTypedefs(
    uint32 newSize)
{
    utResizeArray(dvTypedefs.Sym, (newSize));
    utResizeArray(dvTypedefs.InitializerIndex_, (newSize));
    utResizeArray(dvTypedefs.NumInitializer, (newSize));
    utResizeArray(dvTypedefs.Module, (newSize));
    utResizeArray(dvTypedefs.NextModuleTypedef, (newSize));
    utResizeArray(dvTypedefs.PrevModuleTypedef, (newSize));
    utResizeArray(dvTypedefs.NextTableModuleTypedef, (newSize));
    dvSetAllocatedTypedef(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Typedefs.
----------------------------------------------------------------------------------------*/
void dvTypedefAllocMore(void)
{
    reallocTypedefs((uint32)(dvAllocatedTypedef() + (dvAllocatedTypedef() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Compact the Typedef.Initializer heap to free memory.
----------------------------------------------------------------------------------------*/
void dvCompactTypedefInitializers(void)
{
    uint32 elementSize = sizeof(char);
    uint32 usedHeaderSize = (sizeof(dvTypedef) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvTypedef) + sizeof(uint32) + elementSize - 1)/elementSize;
    char *toPtr = dvTypedefs.Initializer;
    char *fromPtr = toPtr;
    dvTypedef Typedef;
    uint32 size;

    while(fromPtr < dvTypedefs.Initializer + dvUsedTypedefInitializer()) {
        Typedef = *(dvTypedef *)(void *)fromPtr;
        if(Typedef != dvTypedefNull) {
            /* Need to move it to toPtr */
            size = utMax(dvTypedefGetNumInitializer(Typedef) + usedHeaderSize, freeHeaderSize);
            memmove((void *)toPtr, (void *)fromPtr, size*elementSize);
            dvTypedefSetInitializerIndex_(Typedef, toPtr - dvTypedefs.Initializer + usedHeaderSize);
            toPtr += size;
        } else {
            /* Just skip it */
            size = *(uint32 *)(void *)(((dvTypedef *)(void *)fromPtr) + 1);
        }
        fromPtr += size;
    }
    dvSetUsedTypedefInitializer(toPtr - dvTypedefs.Initializer);
    dvSetFreeTypedefInitializer(0);
}

/*----------------------------------------------------------------------------------------
  Allocate more memory for the Typedef.Initializer heap.
----------------------------------------------------------------------------------------*/
static void allocMoreTypedefInitializers(
    uint32 spaceNeeded)
{
    uint32 freeSpace = dvAllocatedTypedefInitializer() - dvUsedTypedefInitializer();

    if((dvFreeTypedefInitializer() << 2) > dvUsedTypedefInitializer()) {
        dvCompactTypedefInitializers();
        freeSpace = dvAllocatedTypedefInitializer() - dvUsedTypedefInitializer();
    }
    if(freeSpace < spaceNeeded) {
        dvSetAllocatedTypedefInitializer(dvAllocatedTypedefInitializer() + spaceNeeded - freeSpace +
            (dvAllocatedTypedefInitializer() >> 1));
        utResizeArray(dvTypedefs.Initializer, dvAllocatedTypedefInitializer());
    }
}

/*----------------------------------------------------------------------------------------
  Allocate memory for a new Typedef.Initializer array.
----------------------------------------------------------------------------------------*/
void dvTypedefAllocInitializers(
    dvTypedef Typedef,
    uint32 numInitializers)
{
    uint32 freeSpace = dvAllocatedTypedefInitializer() - dvUsedTypedefInitializer();
    uint32 elementSize = sizeof(char);
    uint32 usedHeaderSize = (sizeof(dvTypedef) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvTypedef) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 spaceNeeded = utMax(numInitializers + usedHeaderSize, freeHeaderSize);

#if defined(DD_DEBUG)
    utAssert(dvTypedefGetNumInitializer(Typedef) == 0);
#endif
    if(numInitializers == 0) {
        return;
    }
    if(freeSpace < spaceNeeded) {
        allocMoreTypedefInitializers(spaceNeeded);
    }
    dvTypedefSetInitializerIndex_(Typedef, dvUsedTypedefInitializer() + usedHeaderSize);
    dvTypedefSetNumInitializer(Typedef, numInitializers);
    *(dvTypedef *)(void *)(dvTypedefs.Initializer + dvUsedTypedefInitializer()) = Typedef;
    memset(dvTypedefs.Initializer + dvTypedefGetInitializerIndex_(Typedef), 0, ((numInitializers))*sizeof(char));
    dvSetUsedTypedefInitializer(dvUsedTypedefInitializer() + spaceNeeded);
}

/*----------------------------------------------------------------------------------------
  Wrapper around dvTypedefGetInitializers for the database manager.
----------------------------------------------------------------------------------------*/
static void *getTypedefInitializers(
    uint64 objectNumber,
    uint32 *numValues)
{
    dvTypedef Typedef = dvIndex2Typedef((uint32)objectNumber);

    *numValues = dvTypedefGetNumInitializer(Typedef);
    return dvTypedefGetInitializers(Typedef);
}

/*----------------------------------------------------------------------------------------
  Wrapper around dvTypedefAllocInitializers for the database manager.
----------------------------------------------------------------------------------------*/
static void *allocTypedefInitializers(
    uint64 objectNumber,
    uint32 numValues)
{
    dvTypedef Typedef = dvIndex2Typedef((uint32)objectNumber);

    dvTypedefSetInitializerIndex_(Typedef, 0);
    dvTypedefSetNumInitializer(Typedef, 0);
    if(numValues == 0) {
        return NULL;
    }
    dvTypedefAllocInitializers(Typedef, numValues);
    return dvTypedefGetInitializers(Typedef);
}

/*----------------------------------------------------------------------------------------
  Free memory used by the Typedef.Initializer array.
----------------------------------------------------------------------------------------*/
void dvTypedefFreeInitializers(
    dvTypedef Typedef)
{
    uint32 elementSize = sizeof(char);
    uint32 usedHeaderSize = (sizeof(dvTypedef) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvTypedef) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 size = utMax(dvTypedefGetNumInitializer(Typedef) + usedHeaderSize, freeHeaderSize);
    char *dataPtr = dvTypedefGetInitializers(Typedef) - usedHeaderSize;

    if(dvTypedefGetNumInitializer(Typedef) == 0) {
        return;
    }
    *(dvTypedef *)(void *)(dataPtr) = dvTypedefNull;
    *(uint32 *)(void *)(((dvTypedef *)(void *)dataPtr) + 1) = size;
    dvTypedefSetNumInitializer(Typedef, 0);
    dvSetFreeTypedefInitializer(dvFreeTypedefInitializer() + size);
}

/*----------------------------------------------------------------------------------------
  Resize the Typedef.Initializer array.
----------------------------------------------------------------------------------------*/
void dvTypedefResizeInitializers(
    dvTypedef Typedef,
    uint32 numInitializers)
{
    uint32 freeSpace;
    uint32 elementSize = sizeof(char);
    uint32 usedHeaderSize = (sizeof(dvTypedef) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvTypedef) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 newSize = utMax(numInitializers + usedHeaderSize, freeHeaderSize);
    uint32 oldSize = utMax(dvTypedefGetNumInitializer(Typedef) + usedHeaderSize, freeHeaderSize);
    char *dataPtr;

    if(numInitializers == 0) {
        if(dvTypedefGetNumInitializer(Typedef) != 0) {
            dvTypedefFreeInitializers(Typedef);
        }
        return;
    }
    if(dvTypedefGetNumInitializer(Typedef) == 0) {
        dvTypedefAllocInitializers(Typedef, numInitializers);
        return;
    }
    freeSpace = dvAllocatedTypedefInitializer() - dvUsedTypedefInitializer();
    if(freeSpace < newSize) {
        allocMoreTypedefInitializers(newSize);
    }
    dataPtr = dvTypedefGetInitializers(Typedef) - usedHeaderSize;
    memcpy((void *)(dvTypedefs.Initializer + dvUsedTypedefInitializer()), dataPtr,
        elementSize*utMin(oldSize, newSize));
    if(newSize > oldSize) {
        memset(dvTypedefs.Initializer + dvUsedTypedefInitializer() + oldSize, 0, ((newSize - oldSize))*sizeof(char));
    }
    *(dvTypedef *)(void *)dataPtr = dvTypedefNull;
    *(uint32 *)(void *)(((dvTypedef *)(void *)dataPtr) + 1) = oldSize;
    dvSetFreeTypedefInitializer(dvFreeTypedefInitializer() + oldSize);
    dvTypedefSetInitializerIndex_(Typedef, dvUsedTypedefInitializer() + usedHeaderSize);
    dvTypedefSetNumInitializer(Typedef, numInitializers);
    dvSetUsedTypedefInitializer(dvUsedTypedefInitializer() + newSize);
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Typedef.
----------------------------------------------------------------------------------------*/
void dvTypedefCopyProps(
    dvTypedef oldTypedef,
    dvTypedef newTypedef)
{
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void dvShowTypedef(
    dvTypedef Typedef)
{
    utDatabaseShowObject("dv", "Typedef", dvTypedef2Index(Typedef));
}
#endif

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocClass(void)
{
    dvClass Class = dvClassAlloc();

    return dvClass2Index(Class);
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Class.
----------------------------------------------------------------------------------------*/
static void allocClasss(void)
{
    dvSetAllocatedClass(2);
    dvSetUsedClass(1);
    dvClasss.Sym = utNewAInitFirst(utSym, (dvAllocatedClass()));
    dvClasss.MemoryStyle = utNewAInitFirst(dvMemoryStyle, (dvAllocatedClass()));
    dvClasss.ReferenceSize = utNewAInitFirst(uint8, (dvAllocatedClass()));
    dvClasss.GenerateArrayClass = utNewAInitFirst(uint8, (dvAllocatedClass() + 7) >> 3);
    dvClasss.GenerateAttributes = utNewAInitFirst(uint8, (dvAllocatedClass() + 7) >> 3);
    dvClasss.Sparse = utNewAInitFirst(uint8, (dvAllocatedClass() + 7) >> 3);
    dvClasss.NumFields = utNewAInitFirst(uint16, (dvAllocatedClass()));
    dvClasss.Number = utNewAInitFirst(uint16, (dvAllocatedClass()));
    dvClasss.BaseClassSym = utNewAInitFirst(utSym, (dvAllocatedClass()));
    dvClasss.Module = utNewAInitFirst(dvModule, (dvAllocatedClass()));
    dvClasss.NextModuleClass = utNewAInitFirst(dvClass, (dvAllocatedClass()));
    dvClasss.PrevModuleClass = utNewAInitFirst(dvClass, (dvAllocatedClass()));
    dvClasss.NextTableModuleClass = utNewAInitFirst(dvClass, (dvAllocatedClass()));
    dvClasss.FirstProperty = utNewAInitFirst(dvProperty, (dvAllocatedClass()));
    dvClasss.LastProperty = utNewAInitFirst(dvProperty, (dvAllocatedClass()));
    dvClasss.PropertyTableIndex_ = utNewAInitFirst(uint32, (dvAllocatedClass()));
    dvClasss.NumPropertyTable = utNewAInitFirst(uint32, (dvAllocatedClass()));
    dvSetUsedClassPropertyTable(0);
    dvSetAllocatedClassPropertyTable(2);
    dvSetFreeClassPropertyTable(0);
    dvClasss.PropertyTable = utNewAInitFirst(dvProperty, dvAllocatedClassPropertyTable());
    dvClasss.NumProperty = utNewAInitFirst(uint32, (dvAllocatedClass()));
    dvClasss.FreeListProperty = utNewAInitFirst(dvProperty, (dvAllocatedClass()));
    dvClasss.FirstSparsegroup = utNewAInitFirst(dvSparsegroup, (dvAllocatedClass()));
    dvClasss.LastSparsegroup = utNewAInitFirst(dvSparsegroup, (dvAllocatedClass()));
    dvClasss.SparsegroupTableIndex_ = utNewAInitFirst(uint32, (dvAllocatedClass()));
    dvClasss.NumSparsegroupTable = utNewAInitFirst(uint32, (dvAllocatedClass()));
    dvSetUsedClassSparsegroupTable(0);
    dvSetAllocatedClassSparsegroupTable(2);
    dvSetFreeClassSparsegroupTable(0);
    dvClasss.SparsegroupTable = utNewAInitFirst(dvSparsegroup, dvAllocatedClassSparsegroupTable());
    dvClasss.NumSparsegroup = utNewAInitFirst(uint32, (dvAllocatedClass()));
    dvClasss.BaseClass = utNewAInitFirst(dvClass, (dvAllocatedClass()));
    dvClasss.FirstDerivedClass = utNewAInitFirst(dvClass, (dvAllocatedClass()));
    dvClasss.NextClassDerivedClass = utNewAInitFirst(dvClass, (dvAllocatedClass()));
    dvClasss.LastDerivedClass = utNewAInitFirst(dvClass, (dvAllocatedClass()));
    dvClasss.FirstChildRelationship = utNewAInitFirst(dvRelationship, (dvAllocatedClass()));
    dvClasss.LastChildRelationship = utNewAInitFirst(dvRelationship, (dvAllocatedClass()));
    dvClasss.FirstParentRelationship = utNewAInitFirst(dvRelationship, (dvAllocatedClass()));
    dvClasss.LastParentRelationship = utNewAInitFirst(dvRelationship, (dvAllocatedClass()));
    dvClasss.FirstUnion = utNewAInitFirst(dvUnion, (dvAllocatedClass()));
    dvClasss.LastUnion = utNewAInitFirst(dvUnion, (dvAllocatedClass()));
    dvClasss.FirstCache = utNewAInitFirst(dvCache, (dvAllocatedClass()));
    dvClasss.LastCache = utNewAInitFirst(dvCache, (dvAllocatedClass()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Class.
----------------------------------------------------------------------------------------*/
static void reallocClasss(
    uint32 newSize)
{
    utResizeArray(dvClasss.Sym, (newSize));
    utResizeArray(dvClasss.MemoryStyle, (newSize));
    utResizeArray(dvClasss.ReferenceSize, (newSize));
    utResizeArray(dvClasss.GenerateArrayClass, (newSize + 7) >> 3);
    utResizeArray(dvClasss.GenerateAttributes, (newSize + 7) >> 3);
    utResizeArray(dvClasss.Sparse, (newSize + 7) >> 3);
    utResizeArray(dvClasss.NumFields, (newSize));
    utResizeArray(dvClasss.Number, (newSize));
    utResizeArray(dvClasss.BaseClassSym, (newSize));
    utResizeArray(dvClasss.Module, (newSize));
    utResizeArray(dvClasss.NextModuleClass, (newSize));
    utResizeArray(dvClasss.PrevModuleClass, (newSize));
    utResizeArray(dvClasss.NextTableModuleClass, (newSize));
    utResizeArray(dvClasss.FirstProperty, (newSize));
    utResizeArray(dvClasss.LastProperty, (newSize));
    utResizeArray(dvClasss.PropertyTableIndex_, (newSize));
    utResizeArray(dvClasss.NumPropertyTable, (newSize));
    utResizeArray(dvClasss.NumProperty, (newSize));
    utResizeArray(dvClasss.FreeListProperty, (newSize));
    utResizeArray(dvClasss.FirstSparsegroup, (newSize));
    utResizeArray(dvClasss.LastSparsegroup, (newSize));
    utResizeArray(dvClasss.SparsegroupTableIndex_, (newSize));
    utResizeArray(dvClasss.NumSparsegroupTable, (newSize));
    utResizeArray(dvClasss.NumSparsegroup, (newSize));
    utResizeArray(dvClasss.BaseClass, (newSize));
    utResizeArray(dvClasss.FirstDerivedClass, (newSize));
    utResizeArray(dvClasss.NextClassDerivedClass, (newSize));
    utResizeArray(dvClasss.LastDerivedClass, (newSize));
    utResizeArray(dvClasss.FirstChildRelationship, (newSize));
    utResizeArray(dvClasss.LastChildRelationship, (newSize));
    utResizeArray(dvClasss.FirstParentRelationship, (newSize));
    utResizeArray(dvClasss.LastParentRelationship, (newSize));
    utResizeArray(dvClasss.FirstUnion, (newSize));
    utResizeArray(dvClasss.LastUnion, (newSize));
    utResizeArray(dvClasss.FirstCache, (newSize));
    utResizeArray(dvClasss.LastCache, (newSize));
    dvSetAllocatedClass(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Classs.
----------------------------------------------------------------------------------------*/
void dvClassAllocMore(void)
{
    reallocClasss((uint32)(dvAllocatedClass() + (dvAllocatedClass() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Compact the Class.PropertyTable heap to free memory.
----------------------------------------------------------------------------------------*/
void dvCompactClassPropertyTables(void)
{
    uint32 elementSize = sizeof(dvProperty);
    uint32 usedHeaderSize = (sizeof(dvClass) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvClass) + sizeof(uint32) + elementSize - 1)/elementSize;
    dvProperty *toPtr = dvClasss.PropertyTable;
    dvProperty *fromPtr = toPtr;
    dvClass Class;
    uint32 size;

    while(fromPtr < dvClasss.PropertyTable + dvUsedClassPropertyTable()) {
        Class = *(dvClass *)(void *)fromPtr;
        if(Class != dvClassNull) {
            /* Need to move it to toPtr */
            size = utMax(dvClassGetNumPropertyTable(Class) + usedHeaderSize, freeHeaderSize);
            memmove((void *)toPtr, (void *)fromPtr, size*elementSize);
            dvClassSetPropertyTableIndex_(Class, toPtr - dvClasss.PropertyTable + usedHeaderSize);
            toPtr += size;
        } else {
            /* Just skip it */
            size = *(uint32 *)(void *)(((dvClass *)(void *)fromPtr) + 1);
        }
        fromPtr += size;
    }
    dvSetUsedClassPropertyTable(toPtr - dvClasss.PropertyTable);
    dvSetFreeClassPropertyTable(0);
}

/*----------------------------------------------------------------------------------------
  Allocate more memory for the Class.PropertyTable heap.
----------------------------------------------------------------------------------------*/
static void allocMoreClassPropertyTables(
    uint32 spaceNeeded)
{
    uint32 freeSpace = dvAllocatedClassPropertyTable() - dvUsedClassPropertyTable();

    if((dvFreeClassPropertyTable() << 2) > dvUsedClassPropertyTable()) {
        dvCompactClassPropertyTables();
        freeSpace = dvAllocatedClassPropertyTable() - dvUsedClassPropertyTable();
    }
    if(freeSpace < spaceNeeded) {
        dvSetAllocatedClassPropertyTable(dvAllocatedClassPropertyTable() + spaceNeeded - freeSpace +
            (dvAllocatedClassPropertyTable() >> 1));
        utResizeArray(dvClasss.PropertyTable, dvAllocatedClassPropertyTable());
    }
}

/*----------------------------------------------------------------------------------------
  Allocate memory for a new Class.PropertyTable array.
----------------------------------------------------------------------------------------*/
void dvClassAllocPropertyTables(
    dvClass Class,
    uint32 numPropertyTables)
{
    uint32 freeSpace = dvAllocatedClassPropertyTable() - dvUsedClassPropertyTable();
    uint32 elementSize = sizeof(dvProperty);
    uint32 usedHeaderSize = (sizeof(dvClass) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvClass) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 spaceNeeded = utMax(numPropertyTables + usedHeaderSize, freeHeaderSize);

#if defined(DD_DEBUG)
    utAssert(dvClassGetNumPropertyTable(Class) == 0);
#endif
    if(numPropertyTables == 0) {
        return;
    }
    if(freeSpace < spaceNeeded) {
        allocMoreClassPropertyTables(spaceNeeded);
    }
    dvClassSetPropertyTableIndex_(Class, dvUsedClassPropertyTable() + usedHeaderSize);
    dvClassSetNumPropertyTable(Class, numPropertyTables);
    *(dvClass *)(void *)(dvClasss.PropertyTable + dvUsedClassPropertyTable()) = Class;
    {
        uint32 xValue;
        for(xValue = (uint32)(dvClassGetPropertyTableIndex_(Class)); xValue < dvClassGetPropertyTableIndex_(Class) + numPropertyTables; xValue++) {
            dvClasss.PropertyTable[xValue] = dvPropertyNull;
        }
    }
    dvSetUsedClassPropertyTable(dvUsedClassPropertyTable() + spaceNeeded);
}

/*----------------------------------------------------------------------------------------
  Wrapper around dvClassGetPropertyTables for the database manager.
----------------------------------------------------------------------------------------*/
static void *getClassPropertyTables(
    uint64 objectNumber,
    uint32 *numValues)
{
    dvClass Class = dvIndex2Class((uint32)objectNumber);

    *numValues = dvClassGetNumPropertyTable(Class);
    return dvClassGetPropertyTables(Class);
}

/*----------------------------------------------------------------------------------------
  Wrapper around dvClassAllocPropertyTables for the database manager.
----------------------------------------------------------------------------------------*/
static void *allocClassPropertyTables(
    uint64 objectNumber,
    uint32 numValues)
{
    dvClass Class = dvIndex2Class((uint32)objectNumber);

    dvClassSetPropertyTableIndex_(Class, 0);
    dvClassSetNumPropertyTable(Class, 0);
    if(numValues == 0) {
        return NULL;
    }
    dvClassAllocPropertyTables(Class, numValues);
    return dvClassGetPropertyTables(Class);
}

/*----------------------------------------------------------------------------------------
  Free memory used by the Class.PropertyTable array.
----------------------------------------------------------------------------------------*/
void dvClassFreePropertyTables(
    dvClass Class)
{
    uint32 elementSize = sizeof(dvProperty);
    uint32 usedHeaderSize = (sizeof(dvClass) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvClass) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 size = utMax(dvClassGetNumPropertyTable(Class) + usedHeaderSize, freeHeaderSize);
    dvProperty *dataPtr = dvClassGetPropertyTables(Class) - usedHeaderSize;

    if(dvClassGetNumPropertyTable(Class) == 0) {
        return;
    }
    *(dvClass *)(void *)(dataPtr) = dvClassNull;
    *(uint32 *)(void *)(((dvClass *)(void *)dataPtr) + 1) = size;
    dvClassSetNumPropertyTable(Class, 0);
    dvSetFreeClassPropertyTable(dvFreeClassPropertyTable() + size);
}

/*----------------------------------------------------------------------------------------
  Resize the Class.PropertyTable array.
----------------------------------------------------------------------------------------*/
void dvClassResizePropertyTables(
    dvClass Class,
    uint32 numPropertyTables)
{
    uint32 freeSpace;
    uint32 elementSize = sizeof(dvProperty);
    uint32 usedHeaderSize = (sizeof(dvClass) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvClass) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 newSize = utMax(numPropertyTables + usedHeaderSize, freeHeaderSize);
    uint32 oldSize = utMax(dvClassGetNumPropertyTable(Class) + usedHeaderSize, freeHeaderSize);
    dvProperty *dataPtr;

    if(numPropertyTables == 0) {
        if(dvClassGetNumPropertyTable(Class) != 0) {
            dvClassFreePropertyTables(Class);
        }
        return;
    }
    if(dvClassGetNumPropertyTable(Class) == 0) {
        dvClassAllocPropertyTables(Class, numPropertyTables);
        return;
    }
    freeSpace = dvAllocatedClassPropertyTable() - dvUsedClassPropertyTable();
    if(freeSpace < newSize) {
        allocMoreClassPropertyTables(newSize);
    }
    dataPtr = dvClassGetPropertyTables(Class) - usedHeaderSize;
    memcpy((void *)(dvClasss.PropertyTable + dvUsedClassPropertyTable()), dataPtr,
        elementSize*utMin(oldSize, newSize));
    if(newSize > oldSize) {
        {
            uint32 xValue;
            for(xValue = (uint32)(dvUsedClassPropertyTable() + oldSize); xValue < dvUsedClassPropertyTable() + oldSize + newSize - oldSize; xValue++) {
                dvClasss.PropertyTable[xValue] = dvPropertyNull;
            }
        }
    }
    *(dvClass *)(void *)dataPtr = dvClassNull;
    *(uint32 *)(void *)(((dvClass *)(void *)dataPtr) + 1) = oldSize;
    dvSetFreeClassPropertyTable(dvFreeClassPropertyTable() + oldSize);
    dvClassSetPropertyTableIndex_(Class, dvUsedClassPropertyTable() + usedHeaderSize);
    dvClassSetNumPropertyTable(Class, numPropertyTables);
    dvSetUsedClassPropertyTable(dvUsedClassPropertyTable() + newSize);
}

/*----------------------------------------------------------------------------------------
  Compact the Class.SparsegroupTable heap to free memory.
----------------------------------------------------------------------------------------*/
void dvCompactClassSparsegroupTables(void)
{
    uint32 elementSize = sizeof(dvSparsegroup);
    uint32 usedHeaderSize = (sizeof(dvClass) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvClass) + sizeof(uint32) + elementSize - 1)/elementSize;
    dvSparsegroup *toPtr = dvClasss.SparsegroupTable;
    dvSparsegroup *fromPtr = toPtr;
    dvClass Class;
    uint32 size;

    while(fromPtr < dvClasss.SparsegroupTable + dvUsedClassSparsegroupTable()) {
        Class = *(dvClass *)(void *)fromPtr;
        if(Class != dvClassNull) {
            /* Need to move it to toPtr */
            size = utMax(dvClassGetNumSparsegroupTable(Class) + usedHeaderSize, freeHeaderSize);
            memmove((void *)toPtr, (void *)fromPtr, size*elementSize);
            dvClassSetSparsegroupTableIndex_(Class, toPtr - dvClasss.SparsegroupTable + usedHeaderSize);
            toPtr += size;
        } else {
            /* Just skip it */
            size = *(uint32 *)(void *)(((dvClass *)(void *)fromPtr) + 1);
        }
        fromPtr += size;
    }
    dvSetUsedClassSparsegroupTable(toPtr - dvClasss.SparsegroupTable);
    dvSetFreeClassSparsegroupTable(0);
}

/*----------------------------------------------------------------------------------------
  Allocate more memory for the Class.SparsegroupTable heap.
----------------------------------------------------------------------------------------*/
static void allocMoreClassSparsegroupTables(
    uint32 spaceNeeded)
{
    uint32 freeSpace = dvAllocatedClassSparsegroupTable() - dvUsedClassSparsegroupTable();

    if((dvFreeClassSparsegroupTable() << 2) > dvUsedClassSparsegroupTable()) {
        dvCompactClassSparsegroupTables();
        freeSpace = dvAllocatedClassSparsegroupTable() - dvUsedClassSparsegroupTable();
    }
    if(freeSpace < spaceNeeded) {
        dvSetAllocatedClassSparsegroupTable(dvAllocatedClassSparsegroupTable() + spaceNeeded - freeSpace +
            (dvAllocatedClassSparsegroupTable() >> 1));
        utResizeArray(dvClasss.SparsegroupTable, dvAllocatedClassSparsegroupTable());
    }
}

/*----------------------------------------------------------------------------------------
  Allocate memory for a new Class.SparsegroupTable array.
----------------------------------------------------------------------------------------*/
void dvClassAllocSparsegroupTables(
    dvClass Class,
    uint32 numSparsegroupTables)
{
    uint32 freeSpace = dvAllocatedClassSparsegroupTable() - dvUsedClassSparsegroupTable();
    uint32 elementSize = sizeof(dvSparsegroup);
    uint32 usedHeaderSize = (sizeof(dvClass) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvClass) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 spaceNeeded = utMax(numSparsegroupTables + usedHeaderSize, freeHeaderSize);

#if defined(DD_DEBUG)
    utAssert(dvClassGetNumSparsegroupTable(Class) == 0);
#endif
    if(numSparsegroupTables == 0) {
        return;
    }
    if(freeSpace < spaceNeeded) {
        allocMoreClassSparsegroupTables(spaceNeeded);
    }
    dvClassSetSparsegroupTableIndex_(Class, dvUsedClassSparsegroupTable() + usedHeaderSize);
    dvClassSetNumSparsegroupTable(Class, numSparsegroupTables);
    *(dvClass *)(void *)(dvClasss.SparsegroupTable + dvUsedClassSparsegroupTable()) = Class;
    {
        uint32 xValue;
        for(xValue = (uint32)(dvClassGetSparsegroupTableIndex_(Class)); xValue < dvClassGetSparsegroupTableIndex_(Class) + numSparsegroupTables; xValue++) {
            dvClasss.SparsegroupTable[xValue] = dvSparsegroupNull;
        }
    }
    dvSetUsedClassSparsegroupTable(dvUsedClassSparsegroupTable() + spaceNeeded);
}

/*----------------------------------------------------------------------------------------
  Wrapper around dvClassGetSparsegroupTables for the database manager.
----------------------------------------------------------------------------------------*/
static void *getClassSparsegroupTables(
    uint64 objectNumber,
    uint32 *numValues)
{
    dvClass Class = dvIndex2Class((uint32)objectNumber);

    *numValues = dvClassGetNumSparsegroupTable(Class);
    return dvClassGetSparsegroupTables(Class);
}

/*----------------------------------------------------------------------------------------
  Wrapper around dvClassAllocSparsegroupTables for the database manager.
----------------------------------------------------------------------------------------*/
static void *allocClassSparsegroupTables(
    uint64 objectNumber,
    uint32 numValues)
{
    dvClass Class = dvIndex2Class((uint32)objectNumber);

    dvClassSetSparsegroupTableIndex_(Class, 0);
    dvClassSetNumSparsegroupTable(Class, 0);
    if(numValues == 0) {
        return NULL;
    }
    dvClassAllocSparsegroupTables(Class, numValues);
    return dvClassGetSparsegroupTables(Class);
}

/*----------------------------------------------------------------------------------------
  Free memory used by the Class.SparsegroupTable array.
----------------------------------------------------------------------------------------*/
void dvClassFreeSparsegroupTables(
    dvClass Class)
{
    uint32 elementSize = sizeof(dvSparsegroup);
    uint32 usedHeaderSize = (sizeof(dvClass) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvClass) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 size = utMax(dvClassGetNumSparsegroupTable(Class) + usedHeaderSize, freeHeaderSize);
    dvSparsegroup *dataPtr = dvClassGetSparsegroupTables(Class) - usedHeaderSize;

    if(dvClassGetNumSparsegroupTable(Class) == 0) {
        return;
    }
    *(dvClass *)(void *)(dataPtr) = dvClassNull;
    *(uint32 *)(void *)(((dvClass *)(void *)dataPtr) + 1) = size;
    dvClassSetNumSparsegroupTable(Class, 0);
    dvSetFreeClassSparsegroupTable(dvFreeClassSparsegroupTable() + size);
}

/*----------------------------------------------------------------------------------------
  Resize the Class.SparsegroupTable array.
----------------------------------------------------------------------------------------*/
void dvClassResizeSparsegroupTables(
    dvClass Class,
    uint32 numSparsegroupTables)
{
    uint32 freeSpace;
    uint32 elementSize = sizeof(dvSparsegroup);
    uint32 usedHeaderSize = (sizeof(dvClass) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvClass) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 newSize = utMax(numSparsegroupTables + usedHeaderSize, freeHeaderSize);
    uint32 oldSize = utMax(dvClassGetNumSparsegroupTable(Class) + usedHeaderSize, freeHeaderSize);
    dvSparsegroup *dataPtr;

    if(numSparsegroupTables == 0) {
        if(dvClassGetNumSparsegroupTable(Class) != 0) {
            dvClassFreeSparsegroupTables(Class);
        }
        return;
    }
    if(dvClassGetNumSparsegroupTable(Class) == 0) {
        dvClassAllocSparsegroupTables(Class, numSparsegroupTables);
        return;
    }
    freeSpace = dvAllocatedClassSparsegroupTable() - dvUsedClassSparsegroupTable();
    if(freeSpace < newSize) {
        allocMoreClassSparsegroupTables(newSize);
    }
    dataPtr = dvClassGetSparsegroupTables(Class) - usedHeaderSize;
    memcpy((void *)(dvClasss.SparsegroupTable + dvUsedClassSparsegroupTable()), dataPtr,
        elementSize*utMin(oldSize, newSize));
    if(newSize > oldSize) {
        {
            uint32 xValue;
            for(xValue = (uint32)(dvUsedClassSparsegroupTable() + oldSize); xValue < dvUsedClassSparsegroupTable() + oldSize + newSize - oldSize; xValue++) {
                dvClasss.SparsegroupTable[xValue] = dvSparsegroupNull;
            }
        }
    }
    *(dvClass *)(void *)dataPtr = dvClassNull;
    *(uint32 *)(void *)(((dvClass *)(void *)dataPtr) + 1) = oldSize;
    dvSetFreeClassSparsegroupTable(dvFreeClassSparsegroupTable() + oldSize);
    dvClassSetSparsegroupTableIndex_(Class, dvUsedClassSparsegroupTable() + usedHeaderSize);
    dvClassSetNumSparsegroupTable(Class, numSparsegroupTables);
    dvSetUsedClassSparsegroupTable(dvUsedClassSparsegroupTable() + newSize);
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Class.
----------------------------------------------------------------------------------------*/
void dvClassCopyProps(
    dvClass oldClass,
    dvClass newClass)
{
    dvClassSetMemoryStyle(newClass, dvClassGetMemoryStyle(oldClass));
    dvClassSetReferenceSize(newClass, dvClassGetReferenceSize(oldClass));
    dvClassSetGenerateArrayClass(newClass, dvClassGenerateArrayClass(oldClass));
    dvClassSetGenerateAttributes(newClass, dvClassGenerateAttributes(oldClass));
    dvClassSetSparse(newClass, dvClassSparse(oldClass));
    dvClassSetNumFields(newClass, dvClassGetNumFields(oldClass));
    dvClassSetNumber(newClass, dvClassGetNumber(oldClass));
    dvClassSetBaseClassSym(newClass, dvClassGetBaseClassSym(oldClass));
}

/*----------------------------------------------------------------------------------------
  Return the integer equivalent for the bit fields in Class.
----------------------------------------------------------------------------------------*/
uint32 dvClassGetBitfield(
    dvClass _Class)
{
    uint32 bitfield = 0;
    uint8 xLevel = 0;

    bitfield |= dvClassGenerateArrayClass(_Class) << xLevel++;
    bitfield |= dvClassGenerateAttributes(_Class) << xLevel++;
    bitfield |= dvClassSparse(_Class) << xLevel++;
    return bitfield;
}

/*----------------------------------------------------------------------------------------
  Set bit fields in Class using bitfield.
----------------------------------------------------------------------------------------*/
void dvClassSetBitfield(
    dvClass _Class,
     uint32 bitfield)
{
    dvClassSetGenerateArrayClass(_Class, bitfield & 1);
    bitfield >>= 1;
    dvClassSetGenerateAttributes(_Class, bitfield & 1);
    bitfield >>= 1;
    dvClassSetSparse(_Class, bitfield & 1);
    bitfield >>= 1;
}

/*----------------------------------------------------------------------------------------
  Increase the size of the hash table.
----------------------------------------------------------------------------------------*/
static void resizeClassPropertyHashTable(
    dvClass Class)
{
    dvProperty _Property, prevProperty, nextProperty;
    uint32 oldNumPropertys = dvClassGetNumPropertyTable(Class);
    uint32 newNumPropertys = oldNumPropertys << 1;
    uint32 xProperty, index;

    if(newNumPropertys == 0) {
        newNumPropertys = 2;
        dvClassAllocPropertyTables(Class, 2);
    } else {
        dvClassResizePropertyTables(Class, newNumPropertys);
    }
    for(xProperty = 0; xProperty < oldNumPropertys; xProperty++) {
        _Property = dvClassGetiPropertyTable(Class, xProperty);
        prevProperty = dvPropertyNull;
        while(_Property != dvPropertyNull) {
            nextProperty = dvPropertyGetNextTableClassProperty(_Property);
            index = (newNumPropertys - 1) & utSymGetHashValue(dvPropertyGetSym(_Property));
            if(index != xProperty) {
                if(prevProperty == dvPropertyNull) {
                    dvClassSetiPropertyTable(Class, xProperty, nextProperty);
                } else {
                    dvPropertySetNextTableClassProperty(prevProperty, nextProperty);
                }
                dvPropertySetNextTableClassProperty(_Property, dvClassGetiPropertyTable(Class, index));
                dvClassSetiPropertyTable(Class, index, _Property);
            } else {
                prevProperty = _Property;
            }
            _Property = nextProperty;
        }
    }
}

/*----------------------------------------------------------------------------------------
  Add the Property to the Class.  If the table is near full, build a new one twice
  as big, delete the old one, and return the new one.
----------------------------------------------------------------------------------------*/
static void addClassPropertyToHashTable(
    dvClass Class,
    dvProperty _Property)
{
    dvProperty nextProperty;
    uint32 index;

    if(dvClassGetNumProperty(Class) >> 1 >= dvClassGetNumPropertyTable(Class)) {
        resizeClassPropertyHashTable(Class);
    }
    index = (dvClassGetNumPropertyTable(Class) - 1) & utSymGetHashValue(dvPropertyGetSym(_Property));
    nextProperty = dvClassGetiPropertyTable(Class, index);
    dvPropertySetNextTableClassProperty(_Property, nextProperty);
    dvClassSetiPropertyTable(Class, index, _Property);
    dvClassSetNumProperty(Class, dvClassGetNumProperty(Class) + 1);
}

/*----------------------------------------------------------------------------------------
  Remove the Property from the hash table.
----------------------------------------------------------------------------------------*/
static void removeClassPropertyFromHashTable(
    dvClass Class,
    dvProperty _Property)
{
    uint32 index = (dvClassGetNumPropertyTable(Class) - 1) & utSymGetHashValue(dvPropertyGetSym(_Property));
    dvProperty prevProperty, nextProperty;
    
    nextProperty = dvClassGetiPropertyTable(Class, index);
    if(nextProperty == _Property) {
        dvClassSetiPropertyTable(Class, index, dvPropertyGetNextTableClassProperty(nextProperty));
    } else {
        do {
            prevProperty = nextProperty;
            nextProperty = dvPropertyGetNextTableClassProperty(nextProperty);
        } while(nextProperty != _Property);
        dvPropertySetNextTableClassProperty(prevProperty, dvPropertyGetNextTableClassProperty(_Property));
    }
    dvClassSetNumProperty(Class, dvClassGetNumProperty(Class) - 1);
    dvPropertySetNextTableClassProperty(_Property, dvPropertyNull);
}

/*----------------------------------------------------------------------------------------
  Find the Property from the Class and its hash key.
----------------------------------------------------------------------------------------*/
dvProperty dvClassFindProperty(
    dvClass Class,
    utSym Sym)
{
    uint32 mask = dvClassGetNumPropertyTable(Class) - 1;
    dvProperty _Property;

    if(mask + 1 != 0) {
        _Property = dvClassGetiPropertyTable(Class, utSymGetHashValue(Sym) & mask);
        while(_Property != dvPropertyNull) {
            if(dvPropertyGetSym(_Property) == Sym) {
                return _Property;
            }
            _Property = dvPropertyGetNextTableClassProperty(_Property);
        }
    }
    return dvPropertyNull;
}

/*----------------------------------------------------------------------------------------
  Find the Property from the Class and its name.
----------------------------------------------------------------------------------------*/
void dvClassRenameProperty(
    dvClass Class,
    dvProperty _Property,
    utSym sym)
{
    if(dvPropertyGetSym(_Property) != utSymNull) {
        removeClassPropertyFromHashTable(Class, _Property);
    }
    dvPropertySetSym(_Property, sym);
    if(sym != utSymNull) {
        addClassPropertyToHashTable(Class, _Property);
    }
}

/*----------------------------------------------------------------------------------------
  Add the Property to the head of the list on the Class.
----------------------------------------------------------------------------------------*/
void dvClassInsertProperty(
    dvClass Class,
    dvProperty _Property)
{
#if defined(DD_DEBUG)
    if(Class == dvClassNull) {
        utExit("Non-existent Class");
    }
    if(_Property == dvPropertyNull) {
        utExit("Non-existent Property");
    }
    if(dvPropertyGetClass(_Property) != dvClassNull) {
        utExit("Attempting to add Property to Class twice");
    }
#endif
    dvPropertySetNextClassProperty(_Property, dvClassGetFirstProperty(Class));
    if(dvClassGetFirstProperty(Class) != dvPropertyNull) {
        dvPropertySetPrevClassProperty(dvClassGetFirstProperty(Class), _Property);
    }
    dvClassSetFirstProperty(Class, _Property);
    dvPropertySetPrevClassProperty(_Property, dvPropertyNull);
    if(dvClassGetLastProperty(Class) == dvPropertyNull) {
        dvClassSetLastProperty(Class, _Property);
    }
    dvPropertySetClass(_Property, Class);
    if(dvPropertyGetSym(_Property) != utSymNull) {
        addClassPropertyToHashTable(Class, _Property);
    }
}

/*----------------------------------------------------------------------------------------
  Add the Property to the end of the list on the Class.
----------------------------------------------------------------------------------------*/
void dvClassAppendProperty(
    dvClass Class,
    dvProperty _Property)
{
#if defined(DD_DEBUG)
    if(Class == dvClassNull) {
        utExit("Non-existent Class");
    }
    if(_Property == dvPropertyNull) {
        utExit("Non-existent Property");
    }
    if(dvPropertyGetClass(_Property) != dvClassNull) {
        utExit("Attempting to add Property to Class twice");
    }
#endif
    dvPropertySetPrevClassProperty(_Property, dvClassGetLastProperty(Class));
    if(dvClassGetLastProperty(Class) != dvPropertyNull) {
        dvPropertySetNextClassProperty(dvClassGetLastProperty(Class), _Property);
    }
    dvClassSetLastProperty(Class, _Property);
    dvPropertySetNextClassProperty(_Property, dvPropertyNull);
    if(dvClassGetFirstProperty(Class) == dvPropertyNull) {
        dvClassSetFirstProperty(Class, _Property);
    }
    dvPropertySetClass(_Property, Class);
    if(dvPropertyGetSym(_Property) != utSymNull) {
        addClassPropertyToHashTable(Class, _Property);
    }
}

/*----------------------------------------------------------------------------------------
  Insert the Property to the Class after the previous Property.
----------------------------------------------------------------------------------------*/
void dvClassInsertAfterProperty(
    dvClass Class,
    dvProperty prevProperty,
    dvProperty _Property)
{
    dvProperty nextProperty = dvPropertyGetNextClassProperty(prevProperty);

#if defined(DD_DEBUG)
    if(Class == dvClassNull) {
        utExit("Non-existent Class");
    }
    if(_Property == dvPropertyNull) {
        utExit("Non-existent Property");
    }
    if(dvPropertyGetClass(_Property) != dvClassNull) {
        utExit("Attempting to add Property to Class twice");
    }
#endif
    dvPropertySetNextClassProperty(_Property, nextProperty);
    dvPropertySetNextClassProperty(prevProperty, _Property);
    dvPropertySetPrevClassProperty(_Property, prevProperty);
    if(nextProperty != dvPropertyNull) {
        dvPropertySetPrevClassProperty(nextProperty, _Property);
    }
    if(dvClassGetLastProperty(Class) == prevProperty) {
        dvClassSetLastProperty(Class, _Property);
    }
    dvPropertySetClass(_Property, Class);
    if(dvPropertyGetSym(_Property) != utSymNull) {
        addClassPropertyToHashTable(Class, _Property);
    }
}

/*----------------------------------------------------------------------------------------
 Remove the Property from the Class.
----------------------------------------------------------------------------------------*/
void dvClassRemoveProperty(
    dvClass Class,
    dvProperty _Property)
{
    dvProperty pProperty, nProperty;

#if defined(DD_DEBUG)
    if(_Property == dvPropertyNull) {
        utExit("Non-existent Property");
    }
    if(dvPropertyGetClass(_Property) != dvClassNull && dvPropertyGetClass(_Property) != Class) {
        utExit("Delete Property from non-owning Class");
    }
#endif
    nProperty = dvPropertyGetNextClassProperty(_Property);
    pProperty = dvPropertyGetPrevClassProperty(_Property);
    if(pProperty != dvPropertyNull) {
        dvPropertySetNextClassProperty(pProperty, nProperty);
    } else if(dvClassGetFirstProperty(Class) == _Property) {
        dvClassSetFirstProperty(Class, nProperty);
    }
    if(nProperty != dvPropertyNull) {
        dvPropertySetPrevClassProperty(nProperty, pProperty);
    } else if(dvClassGetLastProperty(Class) == _Property) {
        dvClassSetLastProperty(Class, pProperty);
    }
    dvPropertySetNextClassProperty(_Property, dvPropertyNull);
    dvPropertySetPrevClassProperty(_Property, dvPropertyNull);
    dvPropertySetClass(_Property, dvClassNull);
    if(dvPropertyGetSym(_Property) != utSymNull) {
        removeClassPropertyFromHashTable(Class, _Property);
    }
}

/*----------------------------------------------------------------------------------------
  Increase the size of the hash table.
----------------------------------------------------------------------------------------*/
static void resizeClassSparsegroupHashTable(
    dvClass Class)
{
    dvSparsegroup _Sparsegroup, prevSparsegroup, nextSparsegroup;
    uint32 oldNumSparsegroups = dvClassGetNumSparsegroupTable(Class);
    uint32 newNumSparsegroups = oldNumSparsegroups << 1;
    uint32 xSparsegroup, index;

    if(newNumSparsegroups == 0) {
        newNumSparsegroups = 2;
        dvClassAllocSparsegroupTables(Class, 2);
    } else {
        dvClassResizeSparsegroupTables(Class, newNumSparsegroups);
    }
    for(xSparsegroup = 0; xSparsegroup < oldNumSparsegroups; xSparsegroup++) {
        _Sparsegroup = dvClassGetiSparsegroupTable(Class, xSparsegroup);
        prevSparsegroup = dvSparsegroupNull;
        while(_Sparsegroup != dvSparsegroupNull) {
            nextSparsegroup = dvSparsegroupGetNextTableClassSparsegroup(_Sparsegroup);
            index = (newNumSparsegroups - 1) & utSymGetHashValue(dvSparsegroupGetSym(_Sparsegroup));
            if(index != xSparsegroup) {
                if(prevSparsegroup == dvSparsegroupNull) {
                    dvClassSetiSparsegroupTable(Class, xSparsegroup, nextSparsegroup);
                } else {
                    dvSparsegroupSetNextTableClassSparsegroup(prevSparsegroup, nextSparsegroup);
                }
                dvSparsegroupSetNextTableClassSparsegroup(_Sparsegroup, dvClassGetiSparsegroupTable(Class, index));
                dvClassSetiSparsegroupTable(Class, index, _Sparsegroup);
            } else {
                prevSparsegroup = _Sparsegroup;
            }
            _Sparsegroup = nextSparsegroup;
        }
    }
}

/*----------------------------------------------------------------------------------------
  Add the Sparsegroup to the Class.  If the table is near full, build a new one twice
  as big, delete the old one, and return the new one.
----------------------------------------------------------------------------------------*/
static void addClassSparsegroupToHashTable(
    dvClass Class,
    dvSparsegroup _Sparsegroup)
{
    dvSparsegroup nextSparsegroup;
    uint32 index;

    if(dvClassGetNumSparsegroup(Class) >> 1 >= dvClassGetNumSparsegroupTable(Class)) {
        resizeClassSparsegroupHashTable(Class);
    }
    index = (dvClassGetNumSparsegroupTable(Class) - 1) & utSymGetHashValue(dvSparsegroupGetSym(_Sparsegroup));
    nextSparsegroup = dvClassGetiSparsegroupTable(Class, index);
    dvSparsegroupSetNextTableClassSparsegroup(_Sparsegroup, nextSparsegroup);
    dvClassSetiSparsegroupTable(Class, index, _Sparsegroup);
    dvClassSetNumSparsegroup(Class, dvClassGetNumSparsegroup(Class) + 1);
}

/*----------------------------------------------------------------------------------------
  Remove the Sparsegroup from the hash table.
----------------------------------------------------------------------------------------*/
static void removeClassSparsegroupFromHashTable(
    dvClass Class,
    dvSparsegroup _Sparsegroup)
{
    uint32 index = (dvClassGetNumSparsegroupTable(Class) - 1) & utSymGetHashValue(dvSparsegroupGetSym(_Sparsegroup));
    dvSparsegroup prevSparsegroup, nextSparsegroup;
    
    nextSparsegroup = dvClassGetiSparsegroupTable(Class, index);
    if(nextSparsegroup == _Sparsegroup) {
        dvClassSetiSparsegroupTable(Class, index, dvSparsegroupGetNextTableClassSparsegroup(nextSparsegroup));
    } else {
        do {
            prevSparsegroup = nextSparsegroup;
            nextSparsegroup = dvSparsegroupGetNextTableClassSparsegroup(nextSparsegroup);
        } while(nextSparsegroup != _Sparsegroup);
        dvSparsegroupSetNextTableClassSparsegroup(prevSparsegroup, dvSparsegroupGetNextTableClassSparsegroup(_Sparsegroup));
    }
    dvClassSetNumSparsegroup(Class, dvClassGetNumSparsegroup(Class) - 1);
    dvSparsegroupSetNextTableClassSparsegroup(_Sparsegroup, dvSparsegroupNull);
}

/*----------------------------------------------------------------------------------------
  Find the Sparsegroup from the Class and its hash key.
----------------------------------------------------------------------------------------*/
dvSparsegroup dvClassFindSparsegroup(
    dvClass Class,
    utSym Sym)
{
    uint32 mask = dvClassGetNumSparsegroupTable(Class) - 1;
    dvSparsegroup _Sparsegroup;

    if(mask + 1 != 0) {
        _Sparsegroup = dvClassGetiSparsegroupTable(Class, utSymGetHashValue(Sym) & mask);
        while(_Sparsegroup != dvSparsegroupNull) {
            if(dvSparsegroupGetSym(_Sparsegroup) == Sym) {
                return _Sparsegroup;
            }
            _Sparsegroup = dvSparsegroupGetNextTableClassSparsegroup(_Sparsegroup);
        }
    }
    return dvSparsegroupNull;
}

/*----------------------------------------------------------------------------------------
  Find the Sparsegroup from the Class and its name.
----------------------------------------------------------------------------------------*/
void dvClassRenameSparsegroup(
    dvClass Class,
    dvSparsegroup _Sparsegroup,
    utSym sym)
{
    if(dvSparsegroupGetSym(_Sparsegroup) != utSymNull) {
        removeClassSparsegroupFromHashTable(Class, _Sparsegroup);
    }
    dvSparsegroupSetSym(_Sparsegroup, sym);
    if(sym != utSymNull) {
        addClassSparsegroupToHashTable(Class, _Sparsegroup);
    }
}

/*----------------------------------------------------------------------------------------
  Add the Sparsegroup to the head of the list on the Class.
----------------------------------------------------------------------------------------*/
void dvClassInsertSparsegroup(
    dvClass Class,
    dvSparsegroup _Sparsegroup)
{
#if defined(DD_DEBUG)
    if(Class == dvClassNull) {
        utExit("Non-existent Class");
    }
    if(_Sparsegroup == dvSparsegroupNull) {
        utExit("Non-existent Sparsegroup");
    }
    if(dvSparsegroupGetClass(_Sparsegroup) != dvClassNull) {
        utExit("Attempting to add Sparsegroup to Class twice");
    }
#endif
    dvSparsegroupSetNextClassSparsegroup(_Sparsegroup, dvClassGetFirstSparsegroup(Class));
    if(dvClassGetFirstSparsegroup(Class) != dvSparsegroupNull) {
        dvSparsegroupSetPrevClassSparsegroup(dvClassGetFirstSparsegroup(Class), _Sparsegroup);
    }
    dvClassSetFirstSparsegroup(Class, _Sparsegroup);
    dvSparsegroupSetPrevClassSparsegroup(_Sparsegroup, dvSparsegroupNull);
    if(dvClassGetLastSparsegroup(Class) == dvSparsegroupNull) {
        dvClassSetLastSparsegroup(Class, _Sparsegroup);
    }
    dvSparsegroupSetClass(_Sparsegroup, Class);
    if(dvSparsegroupGetSym(_Sparsegroup) != utSymNull) {
        addClassSparsegroupToHashTable(Class, _Sparsegroup);
    }
}

/*----------------------------------------------------------------------------------------
  Add the Sparsegroup to the end of the list on the Class.
----------------------------------------------------------------------------------------*/
void dvClassAppendSparsegroup(
    dvClass Class,
    dvSparsegroup _Sparsegroup)
{
#if defined(DD_DEBUG)
    if(Class == dvClassNull) {
        utExit("Non-existent Class");
    }
    if(_Sparsegroup == dvSparsegroupNull) {
        utExit("Non-existent Sparsegroup");
    }
    if(dvSparsegroupGetClass(_Sparsegroup) != dvClassNull) {
        utExit("Attempting to add Sparsegroup to Class twice");
    }
#endif
    dvSparsegroupSetPrevClassSparsegroup(_Sparsegroup, dvClassGetLastSparsegroup(Class));
    if(dvClassGetLastSparsegroup(Class) != dvSparsegroupNull) {
        dvSparsegroupSetNextClassSparsegroup(dvClassGetLastSparsegroup(Class), _Sparsegroup);
    }
    dvClassSetLastSparsegroup(Class, _Sparsegroup);
    dvSparsegroupSetNextClassSparsegroup(_Sparsegroup, dvSparsegroupNull);
    if(dvClassGetFirstSparsegroup(Class) == dvSparsegroupNull) {
        dvClassSetFirstSparsegroup(Class, _Sparsegroup);
    }
    dvSparsegroupSetClass(_Sparsegroup, Class);
    if(dvSparsegroupGetSym(_Sparsegroup) != utSymNull) {
        addClassSparsegroupToHashTable(Class, _Sparsegroup);
    }
}

/*----------------------------------------------------------------------------------------
  Insert the Sparsegroup to the Class after the previous Sparsegroup.
----------------------------------------------------------------------------------------*/
void dvClassInsertAfterSparsegroup(
    dvClass Class,
    dvSparsegroup prevSparsegroup,
    dvSparsegroup _Sparsegroup)
{
    dvSparsegroup nextSparsegroup = dvSparsegroupGetNextClassSparsegroup(prevSparsegroup);

#if defined(DD_DEBUG)
    if(Class == dvClassNull) {
        utExit("Non-existent Class");
    }
    if(_Sparsegroup == dvSparsegroupNull) {
        utExit("Non-existent Sparsegroup");
    }
    if(dvSparsegroupGetClass(_Sparsegroup) != dvClassNull) {
        utExit("Attempting to add Sparsegroup to Class twice");
    }
#endif
    dvSparsegroupSetNextClassSparsegroup(_Sparsegroup, nextSparsegroup);
    dvSparsegroupSetNextClassSparsegroup(prevSparsegroup, _Sparsegroup);
    dvSparsegroupSetPrevClassSparsegroup(_Sparsegroup, prevSparsegroup);
    if(nextSparsegroup != dvSparsegroupNull) {
        dvSparsegroupSetPrevClassSparsegroup(nextSparsegroup, _Sparsegroup);
    }
    if(dvClassGetLastSparsegroup(Class) == prevSparsegroup) {
        dvClassSetLastSparsegroup(Class, _Sparsegroup);
    }
    dvSparsegroupSetClass(_Sparsegroup, Class);
    if(dvSparsegroupGetSym(_Sparsegroup) != utSymNull) {
        addClassSparsegroupToHashTable(Class, _Sparsegroup);
    }
}

/*----------------------------------------------------------------------------------------
 Remove the Sparsegroup from the Class.
----------------------------------------------------------------------------------------*/
void dvClassRemoveSparsegroup(
    dvClass Class,
    dvSparsegroup _Sparsegroup)
{
    dvSparsegroup pSparsegroup, nSparsegroup;

#if defined(DD_DEBUG)
    if(_Sparsegroup == dvSparsegroupNull) {
        utExit("Non-existent Sparsegroup");
    }
    if(dvSparsegroupGetClass(_Sparsegroup) != dvClassNull && dvSparsegroupGetClass(_Sparsegroup) != Class) {
        utExit("Delete Sparsegroup from non-owning Class");
    }
#endif
    nSparsegroup = dvSparsegroupGetNextClassSparsegroup(_Sparsegroup);
    pSparsegroup = dvSparsegroupGetPrevClassSparsegroup(_Sparsegroup);
    if(pSparsegroup != dvSparsegroupNull) {
        dvSparsegroupSetNextClassSparsegroup(pSparsegroup, nSparsegroup);
    } else if(dvClassGetFirstSparsegroup(Class) == _Sparsegroup) {
        dvClassSetFirstSparsegroup(Class, nSparsegroup);
    }
    if(nSparsegroup != dvSparsegroupNull) {
        dvSparsegroupSetPrevClassSparsegroup(nSparsegroup, pSparsegroup);
    } else if(dvClassGetLastSparsegroup(Class) == _Sparsegroup) {
        dvClassSetLastSparsegroup(Class, pSparsegroup);
    }
    dvSparsegroupSetNextClassSparsegroup(_Sparsegroup, dvSparsegroupNull);
    dvSparsegroupSetPrevClassSparsegroup(_Sparsegroup, dvSparsegroupNull);
    dvSparsegroupSetClass(_Sparsegroup, dvClassNull);
    if(dvSparsegroupGetSym(_Sparsegroup) != utSymNull) {
        removeClassSparsegroupFromHashTable(Class, _Sparsegroup);
    }
}

/*----------------------------------------------------------------------------------------
  Add the DerivedClass to the head of the list on the Class.
----------------------------------------------------------------------------------------*/
void dvClassInsertDerivedClass(
    dvClass Class,
    dvClass _Class)
{
#if defined(DD_DEBUG)
    if(Class == dvClassNull) {
        utExit("Non-existent Class");
    }
    if(_Class == dvClassNull) {
        utExit("Non-existent Class");
    }
    if(dvClassGetBaseClass(_Class) != dvClassNull) {
        utExit("Attempting to add Class to Class twice");
    }
#endif
    dvClassSetNextClassDerivedClass(_Class, dvClassGetFirstDerivedClass(Class));
    dvClassSetFirstDerivedClass(Class, _Class);
    if(dvClassGetLastDerivedClass(Class) == dvClassNull) {
        dvClassSetLastDerivedClass(Class, _Class);
    }
    dvClassSetBaseClass(_Class, Class);
}

/*----------------------------------------------------------------------------------------
  Add the DerivedClass to the end of the list on the Class.
----------------------------------------------------------------------------------------*/
void dvClassAppendDerivedClass(
    dvClass Class,
    dvClass _Class)
{
#if defined(DD_DEBUG)
    if(Class == dvClassNull) {
        utExit("Non-existent Class");
    }
    if(_Class == dvClassNull) {
        utExit("Non-existent Class");
    }
    if(dvClassGetBaseClass(_Class) != dvClassNull) {
        utExit("Attempting to add Class to Class twice");
    }
#endif
    if(dvClassGetLastDerivedClass(Class) != dvClassNull) {
        dvClassSetNextClassDerivedClass(dvClassGetLastDerivedClass(Class), _Class);
    } else {
        dvClassSetFirstDerivedClass(Class, _Class);
    }
    dvClassSetLastDerivedClass(Class, _Class);
    dvClassSetNextClassDerivedClass(_Class, dvClassNull);
    dvClassSetBaseClass(_Class, Class);
}

/*----------------------------------------------------------------------------------------
  Insert the DerivedClass to the Class after the previous DerivedClass.
----------------------------------------------------------------------------------------*/
void dvClassInsertAfterDerivedClass(
    dvClass Class,
    dvClass prevClass,
    dvClass _Class)
{
    dvClass nextClass = dvClassGetNextClassDerivedClass(prevClass);

#if defined(DD_DEBUG)
    if(Class == dvClassNull) {
        utExit("Non-existent Class");
    }
    if(_Class == dvClassNull) {
        utExit("Non-existent Class");
    }
    if(dvClassGetBaseClass(_Class) != dvClassNull) {
        utExit("Attempting to add Class to Class twice");
    }
#endif
    dvClassSetNextClassDerivedClass(_Class, nextClass);
    dvClassSetNextClassDerivedClass(prevClass, _Class);
    if(dvClassGetLastDerivedClass(Class) == prevClass) {
        dvClassSetLastDerivedClass(Class, _Class);
    }
    dvClassSetBaseClass(_Class, Class);
}

/*----------------------------------------------------------------------------------------
 Remove the DerivedClass from the Class.
----------------------------------------------------------------------------------------*/
void dvClassRemoveDerivedClass(
    dvClass Class,
    dvClass _Class)
{
    dvClass pClass, nClass;

#if defined(DD_DEBUG)
    if(_Class == dvClassNull) {
        utExit("Non-existent Class");
    }
    if(dvClassGetBaseClass(_Class) != dvClassNull && dvClassGetBaseClass(_Class) != Class) {
        utExit("Delete Class from non-owning Class");
    }
#endif
    pClass = dvClassNull;
    for(nClass = dvClassGetFirstDerivedClass(Class); nClass != dvClassNull && nClass != _Class;
            nClass = dvClassGetNextClassDerivedClass(nClass)) {
        pClass = nClass;
    }
    if(pClass != dvClassNull) {
        dvClassSetNextClassDerivedClass(pClass, dvClassGetNextClassDerivedClass(_Class));
    } else {
        dvClassSetFirstDerivedClass(Class, dvClassGetNextClassDerivedClass(_Class));
    }
    dvClassSetNextClassDerivedClass(_Class, dvClassNull);
    if(dvClassGetLastDerivedClass(Class) == _Class) {
        dvClassSetLastDerivedClass(Class, pClass);
    }
    dvClassSetBaseClass(_Class, dvClassNull);
}

/*----------------------------------------------------------------------------------------
  Add the ChildRelationship to the head of the list on the Class.
----------------------------------------------------------------------------------------*/
void dvClassInsertChildRelationship(
    dvClass Class,
    dvRelationship _Relationship)
{
#if defined(DD_DEBUG)
    if(Class == dvClassNull) {
        utExit("Non-existent Class");
    }
    if(_Relationship == dvRelationshipNull) {
        utExit("Non-existent Relationship");
    }
    if(dvRelationshipGetParentClass(_Relationship) != dvClassNull) {
        utExit("Attempting to add Relationship to Class twice");
    }
#endif
    dvRelationshipSetNextClassChildRelationship(_Relationship, dvClassGetFirstChildRelationship(Class));
    dvClassSetFirstChildRelationship(Class, _Relationship);
    if(dvClassGetLastChildRelationship(Class) == dvRelationshipNull) {
        dvClassSetLastChildRelationship(Class, _Relationship);
    }
    dvRelationshipSetParentClass(_Relationship, Class);
}

/*----------------------------------------------------------------------------------------
  Add the ChildRelationship to the end of the list on the Class.
----------------------------------------------------------------------------------------*/
void dvClassAppendChildRelationship(
    dvClass Class,
    dvRelationship _Relationship)
{
#if defined(DD_DEBUG)
    if(Class == dvClassNull) {
        utExit("Non-existent Class");
    }
    if(_Relationship == dvRelationshipNull) {
        utExit("Non-existent Relationship");
    }
    if(dvRelationshipGetParentClass(_Relationship) != dvClassNull) {
        utExit("Attempting to add Relationship to Class twice");
    }
#endif
    if(dvClassGetLastChildRelationship(Class) != dvRelationshipNull) {
        dvRelationshipSetNextClassChildRelationship(dvClassGetLastChildRelationship(Class), _Relationship);
    } else {
        dvClassSetFirstChildRelationship(Class, _Relationship);
    }
    dvClassSetLastChildRelationship(Class, _Relationship);
    dvRelationshipSetNextClassChildRelationship(_Relationship, dvRelationshipNull);
    dvRelationshipSetParentClass(_Relationship, Class);
}

/*----------------------------------------------------------------------------------------
  Insert the ChildRelationship to the Class after the previous ChildRelationship.
----------------------------------------------------------------------------------------*/
void dvClassInsertAfterChildRelationship(
    dvClass Class,
    dvRelationship prevRelationship,
    dvRelationship _Relationship)
{
    dvRelationship nextRelationship = dvRelationshipGetNextClassChildRelationship(prevRelationship);

#if defined(DD_DEBUG)
    if(Class == dvClassNull) {
        utExit("Non-existent Class");
    }
    if(_Relationship == dvRelationshipNull) {
        utExit("Non-existent Relationship");
    }
    if(dvRelationshipGetParentClass(_Relationship) != dvClassNull) {
        utExit("Attempting to add Relationship to Class twice");
    }
#endif
    dvRelationshipSetNextClassChildRelationship(_Relationship, nextRelationship);
    dvRelationshipSetNextClassChildRelationship(prevRelationship, _Relationship);
    if(dvClassGetLastChildRelationship(Class) == prevRelationship) {
        dvClassSetLastChildRelationship(Class, _Relationship);
    }
    dvRelationshipSetParentClass(_Relationship, Class);
}

/*----------------------------------------------------------------------------------------
 Remove the ChildRelationship from the Class.
----------------------------------------------------------------------------------------*/
void dvClassRemoveChildRelationship(
    dvClass Class,
    dvRelationship _Relationship)
{
    dvRelationship pRelationship, nRelationship;

#if defined(DD_DEBUG)
    if(_Relationship == dvRelationshipNull) {
        utExit("Non-existent Relationship");
    }
    if(dvRelationshipGetParentClass(_Relationship) != dvClassNull && dvRelationshipGetParentClass(_Relationship) != Class) {
        utExit("Delete Relationship from non-owning Class");
    }
#endif
    pRelationship = dvRelationshipNull;
    for(nRelationship = dvClassGetFirstChildRelationship(Class); nRelationship != dvRelationshipNull && nRelationship != _Relationship;
            nRelationship = dvRelationshipGetNextClassChildRelationship(nRelationship)) {
        pRelationship = nRelationship;
    }
    if(pRelationship != dvRelationshipNull) {
        dvRelationshipSetNextClassChildRelationship(pRelationship, dvRelationshipGetNextClassChildRelationship(_Relationship));
    } else {
        dvClassSetFirstChildRelationship(Class, dvRelationshipGetNextClassChildRelationship(_Relationship));
    }
    dvRelationshipSetNextClassChildRelationship(_Relationship, dvRelationshipNull);
    if(dvClassGetLastChildRelationship(Class) == _Relationship) {
        dvClassSetLastChildRelationship(Class, pRelationship);
    }
    dvRelationshipSetParentClass(_Relationship, dvClassNull);
}

/*----------------------------------------------------------------------------------------
  Add the ParentRelationship to the head of the list on the Class.
----------------------------------------------------------------------------------------*/
void dvClassInsertParentRelationship(
    dvClass Class,
    dvRelationship _Relationship)
{
#if defined(DD_DEBUG)
    if(Class == dvClassNull) {
        utExit("Non-existent Class");
    }
    if(_Relationship == dvRelationshipNull) {
        utExit("Non-existent Relationship");
    }
    if(dvRelationshipGetChildClass(_Relationship) != dvClassNull) {
        utExit("Attempting to add Relationship to Class twice");
    }
#endif
    dvRelationshipSetNextClassParentRelationship(_Relationship, dvClassGetFirstParentRelationship(Class));
    dvClassSetFirstParentRelationship(Class, _Relationship);
    if(dvClassGetLastParentRelationship(Class) == dvRelationshipNull) {
        dvClassSetLastParentRelationship(Class, _Relationship);
    }
    dvRelationshipSetChildClass(_Relationship, Class);
}

/*----------------------------------------------------------------------------------------
  Add the ParentRelationship to the end of the list on the Class.
----------------------------------------------------------------------------------------*/
void dvClassAppendParentRelationship(
    dvClass Class,
    dvRelationship _Relationship)
{
#if defined(DD_DEBUG)
    if(Class == dvClassNull) {
        utExit("Non-existent Class");
    }
    if(_Relationship == dvRelationshipNull) {
        utExit("Non-existent Relationship");
    }
    if(dvRelationshipGetChildClass(_Relationship) != dvClassNull) {
        utExit("Attempting to add Relationship to Class twice");
    }
#endif
    if(dvClassGetLastParentRelationship(Class) != dvRelationshipNull) {
        dvRelationshipSetNextClassParentRelationship(dvClassGetLastParentRelationship(Class), _Relationship);
    } else {
        dvClassSetFirstParentRelationship(Class, _Relationship);
    }
    dvClassSetLastParentRelationship(Class, _Relationship);
    dvRelationshipSetNextClassParentRelationship(_Relationship, dvRelationshipNull);
    dvRelationshipSetChildClass(_Relationship, Class);
}

/*----------------------------------------------------------------------------------------
  Insert the ParentRelationship to the Class after the previous ParentRelationship.
----------------------------------------------------------------------------------------*/
void dvClassInsertAfterParentRelationship(
    dvClass Class,
    dvRelationship prevRelationship,
    dvRelationship _Relationship)
{
    dvRelationship nextRelationship = dvRelationshipGetNextClassParentRelationship(prevRelationship);

#if defined(DD_DEBUG)
    if(Class == dvClassNull) {
        utExit("Non-existent Class");
    }
    if(_Relationship == dvRelationshipNull) {
        utExit("Non-existent Relationship");
    }
    if(dvRelationshipGetChildClass(_Relationship) != dvClassNull) {
        utExit("Attempting to add Relationship to Class twice");
    }
#endif
    dvRelationshipSetNextClassParentRelationship(_Relationship, nextRelationship);
    dvRelationshipSetNextClassParentRelationship(prevRelationship, _Relationship);
    if(dvClassGetLastParentRelationship(Class) == prevRelationship) {
        dvClassSetLastParentRelationship(Class, _Relationship);
    }
    dvRelationshipSetChildClass(_Relationship, Class);
}

/*----------------------------------------------------------------------------------------
 Remove the ParentRelationship from the Class.
----------------------------------------------------------------------------------------*/
void dvClassRemoveParentRelationship(
    dvClass Class,
    dvRelationship _Relationship)
{
    dvRelationship pRelationship, nRelationship;

#if defined(DD_DEBUG)
    if(_Relationship == dvRelationshipNull) {
        utExit("Non-existent Relationship");
    }
    if(dvRelationshipGetChildClass(_Relationship) != dvClassNull && dvRelationshipGetChildClass(_Relationship) != Class) {
        utExit("Delete Relationship from non-owning Class");
    }
#endif
    pRelationship = dvRelationshipNull;
    for(nRelationship = dvClassGetFirstParentRelationship(Class); nRelationship != dvRelationshipNull && nRelationship != _Relationship;
            nRelationship = dvRelationshipGetNextClassParentRelationship(nRelationship)) {
        pRelationship = nRelationship;
    }
    if(pRelationship != dvRelationshipNull) {
        dvRelationshipSetNextClassParentRelationship(pRelationship, dvRelationshipGetNextClassParentRelationship(_Relationship));
    } else {
        dvClassSetFirstParentRelationship(Class, dvRelationshipGetNextClassParentRelationship(_Relationship));
    }
    dvRelationshipSetNextClassParentRelationship(_Relationship, dvRelationshipNull);
    if(dvClassGetLastParentRelationship(Class) == _Relationship) {
        dvClassSetLastParentRelationship(Class, pRelationship);
    }
    dvRelationshipSetChildClass(_Relationship, dvClassNull);
}

/*----------------------------------------------------------------------------------------
  Add the Union to the head of the list on the Class.
----------------------------------------------------------------------------------------*/
void dvClassInsertUnion(
    dvClass Class,
    dvUnion _Union)
{
#if defined(DD_DEBUG)
    if(Class == dvClassNull) {
        utExit("Non-existent Class");
    }
    if(_Union == dvUnionNull) {
        utExit("Non-existent Union");
    }
    if(dvUnionGetClass(_Union) != dvClassNull) {
        utExit("Attempting to add Union to Class twice");
    }
#endif
    dvUnionSetNextClassUnion(_Union, dvClassGetFirstUnion(Class));
    dvClassSetFirstUnion(Class, _Union);
    if(dvClassGetLastUnion(Class) == dvUnionNull) {
        dvClassSetLastUnion(Class, _Union);
    }
    dvUnionSetClass(_Union, Class);
}

/*----------------------------------------------------------------------------------------
  Add the Union to the end of the list on the Class.
----------------------------------------------------------------------------------------*/
void dvClassAppendUnion(
    dvClass Class,
    dvUnion _Union)
{
#if defined(DD_DEBUG)
    if(Class == dvClassNull) {
        utExit("Non-existent Class");
    }
    if(_Union == dvUnionNull) {
        utExit("Non-existent Union");
    }
    if(dvUnionGetClass(_Union) != dvClassNull) {
        utExit("Attempting to add Union to Class twice");
    }
#endif
    if(dvClassGetLastUnion(Class) != dvUnionNull) {
        dvUnionSetNextClassUnion(dvClassGetLastUnion(Class), _Union);
    } else {
        dvClassSetFirstUnion(Class, _Union);
    }
    dvClassSetLastUnion(Class, _Union);
    dvUnionSetNextClassUnion(_Union, dvUnionNull);
    dvUnionSetClass(_Union, Class);
}

/*----------------------------------------------------------------------------------------
  Insert the Union to the Class after the previous Union.
----------------------------------------------------------------------------------------*/
void dvClassInsertAfterUnion(
    dvClass Class,
    dvUnion prevUnion,
    dvUnion _Union)
{
    dvUnion nextUnion = dvUnionGetNextClassUnion(prevUnion);

#if defined(DD_DEBUG)
    if(Class == dvClassNull) {
        utExit("Non-existent Class");
    }
    if(_Union == dvUnionNull) {
        utExit("Non-existent Union");
    }
    if(dvUnionGetClass(_Union) != dvClassNull) {
        utExit("Attempting to add Union to Class twice");
    }
#endif
    dvUnionSetNextClassUnion(_Union, nextUnion);
    dvUnionSetNextClassUnion(prevUnion, _Union);
    if(dvClassGetLastUnion(Class) == prevUnion) {
        dvClassSetLastUnion(Class, _Union);
    }
    dvUnionSetClass(_Union, Class);
}

/*----------------------------------------------------------------------------------------
 Remove the Union from the Class.
----------------------------------------------------------------------------------------*/
void dvClassRemoveUnion(
    dvClass Class,
    dvUnion _Union)
{
    dvUnion pUnion, nUnion;

#if defined(DD_DEBUG)
    if(_Union == dvUnionNull) {
        utExit("Non-existent Union");
    }
    if(dvUnionGetClass(_Union) != dvClassNull && dvUnionGetClass(_Union) != Class) {
        utExit("Delete Union from non-owning Class");
    }
#endif
    pUnion = dvUnionNull;
    for(nUnion = dvClassGetFirstUnion(Class); nUnion != dvUnionNull && nUnion != _Union;
            nUnion = dvUnionGetNextClassUnion(nUnion)) {
        pUnion = nUnion;
    }
    if(pUnion != dvUnionNull) {
        dvUnionSetNextClassUnion(pUnion, dvUnionGetNextClassUnion(_Union));
    } else {
        dvClassSetFirstUnion(Class, dvUnionGetNextClassUnion(_Union));
    }
    dvUnionSetNextClassUnion(_Union, dvUnionNull);
    if(dvClassGetLastUnion(Class) == _Union) {
        dvClassSetLastUnion(Class, pUnion);
    }
    dvUnionSetClass(_Union, dvClassNull);
}

/*----------------------------------------------------------------------------------------
  Add the Cache to the head of the list on the Class.
----------------------------------------------------------------------------------------*/
void dvClassInsertCache(
    dvClass Class,
    dvCache _Cache)
{
#if defined(DD_DEBUG)
    if(Class == dvClassNull) {
        utExit("Non-existent Class");
    }
    if(_Cache == dvCacheNull) {
        utExit("Non-existent Cache");
    }
    if(dvCacheGetClass(_Cache) != dvClassNull) {
        utExit("Attempting to add Cache to Class twice");
    }
#endif
    dvCacheSetNextClassCache(_Cache, dvClassGetFirstCache(Class));
    dvClassSetFirstCache(Class, _Cache);
    if(dvClassGetLastCache(Class) == dvCacheNull) {
        dvClassSetLastCache(Class, _Cache);
    }
    dvCacheSetClass(_Cache, Class);
}

/*----------------------------------------------------------------------------------------
  Add the Cache to the end of the list on the Class.
----------------------------------------------------------------------------------------*/
void dvClassAppendCache(
    dvClass Class,
    dvCache _Cache)
{
#if defined(DD_DEBUG)
    if(Class == dvClassNull) {
        utExit("Non-existent Class");
    }
    if(_Cache == dvCacheNull) {
        utExit("Non-existent Cache");
    }
    if(dvCacheGetClass(_Cache) != dvClassNull) {
        utExit("Attempting to add Cache to Class twice");
    }
#endif
    if(dvClassGetLastCache(Class) != dvCacheNull) {
        dvCacheSetNextClassCache(dvClassGetLastCache(Class), _Cache);
    } else {
        dvClassSetFirstCache(Class, _Cache);
    }
    dvClassSetLastCache(Class, _Cache);
    dvCacheSetNextClassCache(_Cache, dvCacheNull);
    dvCacheSetClass(_Cache, Class);
}

/*----------------------------------------------------------------------------------------
  Insert the Cache to the Class after the previous Cache.
----------------------------------------------------------------------------------------*/
void dvClassInsertAfterCache(
    dvClass Class,
    dvCache prevCache,
    dvCache _Cache)
{
    dvCache nextCache = dvCacheGetNextClassCache(prevCache);

#if defined(DD_DEBUG)
    if(Class == dvClassNull) {
        utExit("Non-existent Class");
    }
    if(_Cache == dvCacheNull) {
        utExit("Non-existent Cache");
    }
    if(dvCacheGetClass(_Cache) != dvClassNull) {
        utExit("Attempting to add Cache to Class twice");
    }
#endif
    dvCacheSetNextClassCache(_Cache, nextCache);
    dvCacheSetNextClassCache(prevCache, _Cache);
    if(dvClassGetLastCache(Class) == prevCache) {
        dvClassSetLastCache(Class, _Cache);
    }
    dvCacheSetClass(_Cache, Class);
}

/*----------------------------------------------------------------------------------------
 Remove the Cache from the Class.
----------------------------------------------------------------------------------------*/
void dvClassRemoveCache(
    dvClass Class,
    dvCache _Cache)
{
    dvCache pCache, nCache;

#if defined(DD_DEBUG)
    if(_Cache == dvCacheNull) {
        utExit("Non-existent Cache");
    }
    if(dvCacheGetClass(_Cache) != dvClassNull && dvCacheGetClass(_Cache) != Class) {
        utExit("Delete Cache from non-owning Class");
    }
#endif
    pCache = dvCacheNull;
    for(nCache = dvClassGetFirstCache(Class); nCache != dvCacheNull && nCache != _Cache;
            nCache = dvCacheGetNextClassCache(nCache)) {
        pCache = nCache;
    }
    if(pCache != dvCacheNull) {
        dvCacheSetNextClassCache(pCache, dvCacheGetNextClassCache(_Cache));
    } else {
        dvClassSetFirstCache(Class, dvCacheGetNextClassCache(_Cache));
    }
    dvCacheSetNextClassCache(_Cache, dvCacheNull);
    if(dvClassGetLastCache(Class) == _Cache) {
        dvClassSetLastCache(Class, pCache);
    }
    dvCacheSetClass(_Cache, dvClassNull);
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void dvShowClass(
    dvClass Class)
{
    utDatabaseShowObject("dv", "Class", dvClass2Index(Class));
}
#endif

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocProperty(void)
{
    dvProperty Property = dvPropertyAlloc();

    return dvProperty2Index(Property);
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Property.
----------------------------------------------------------------------------------------*/
static void allocPropertys(void)
{
    dvSetAllocatedProperty(2);
    dvSetUsedProperty(1);
    dvPropertys.Sym = utNewAInitFirst(utSym, (dvAllocatedProperty()));
    dvPropertys.Type = utNewAInitFirst(dvPropertyType, (dvAllocatedProperty()));
    dvPropertys.Array = utNewAInitFirst(uint8, (dvAllocatedProperty() + 7) >> 3);
    dvPropertys.Cascade = utNewAInitFirst(uint8, (dvAllocatedProperty() + 7) >> 3);
    dvPropertys.Sparse = utNewAInitFirst(uint8, (dvAllocatedProperty() + 7) >> 3);
    dvPropertys.View = utNewAInitFirst(uint8, (dvAllocatedProperty() + 7) >> 3);
    dvPropertys.Expanded = utNewAInitFirst(uint8, (dvAllocatedProperty() + 7) >> 3);
    dvPropertys.FieldNumber = utNewAInitFirst(uint32, (dvAllocatedProperty()));
    dvPropertys.FirstElementProp = utNewAInitFirst(dvProperty, (dvAllocatedProperty()));
    dvPropertys.NumElementsProp = utNewAInitFirst(dvProperty, (dvAllocatedProperty()));
    dvPropertys.Hidden = utNewAInitFirst(uint8, (dvAllocatedProperty() + 7) >> 3);
    dvPropertys.InitializerIndex_ = utNewAInitFirst(uint32, (dvAllocatedProperty()));
    dvPropertys.NumInitializer = utNewAInitFirst(uint32, (dvAllocatedProperty()));
    dvSetUsedPropertyInitializer(0);
    dvSetAllocatedPropertyInitializer(2);
    dvSetFreePropertyInitializer(0);
    dvPropertys.Initializer = utNewAInitFirst(char, dvAllocatedPropertyInitializer());
    dvPropertys.FixedSize = utNewAInitFirst(uint8, (dvAllocatedProperty() + 7) >> 3);
    dvPropertys.IndexIndex_ = utNewAInitFirst(uint32, (dvAllocatedProperty()));
    dvPropertys.NumIndex = utNewAInitFirst(uint32, (dvAllocatedProperty()));
    dvSetUsedPropertyIndex(0);
    dvSetAllocatedPropertyIndex(2);
    dvSetFreePropertyIndex(0);
    dvPropertys.Index = utNewAInitFirst(char, dvAllocatedPropertyIndex());
    dvPropertys.Line = utNewAInitFirst(uint32, (dvAllocatedProperty()));
    dvPropertys.Class = utNewAInitFirst(dvClass, (dvAllocatedProperty()));
    dvPropertys.NextClassProperty = utNewAInitFirst(dvProperty, (dvAllocatedProperty()));
    dvPropertys.PrevClassProperty = utNewAInitFirst(dvProperty, (dvAllocatedProperty()));
    dvPropertys.NextTableClassProperty = utNewAInitFirst(dvProperty, (dvAllocatedProperty()));
    dvPropertys.FirstCase = utNewAInitFirst(dvCase, (dvAllocatedProperty()));
    dvPropertys.LastCase = utNewAInitFirst(dvCase, (dvAllocatedProperty()));
    dvPropertys.FirstKeyproperty = utNewAInitFirst(dvKeyproperty, (dvAllocatedProperty()));
    dvPropertys.LastKeyproperty = utNewAInitFirst(dvKeyproperty, (dvAllocatedProperty()));
    dvPropertys.Sparsegroup = utNewAInitFirst(dvSparsegroup, (dvAllocatedProperty()));
    dvPropertys.NextSparsegroupProperty = utNewAInitFirst(dvProperty, (dvAllocatedProperty()));
    dvPropertys.Relationship = utNewAInitFirst(dvRelationship, (dvAllocatedProperty()));
    dvPropertys.NextRelationshipProperty = utNewAInitFirst(dvProperty, (dvAllocatedProperty()));
    dvPropertys.Union = utNewAInitFirst(dvUnion, (dvAllocatedProperty()));
    dvPropertys.NextUnionProperty = utNewAInitFirst(dvProperty, (dvAllocatedProperty()));
    dvPropertys.Cache = utNewAInitFirst(dvCache, (dvAllocatedProperty()));
    dvPropertys.NextCacheProperty = utNewAInitFirst(dvProperty, (dvAllocatedProperty()));
    dvPropertys.union1 = utNewAInitFirst(dvPropertyUnion1, dvAllocatedProperty());
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Property.
----------------------------------------------------------------------------------------*/
static void reallocPropertys(
    uint32 newSize)
{
    utResizeArray(dvPropertys.Sym, (newSize));
    utResizeArray(dvPropertys.Type, (newSize));
    utResizeArray(dvPropertys.Array, (newSize + 7) >> 3);
    utResizeArray(dvPropertys.Cascade, (newSize + 7) >> 3);
    utResizeArray(dvPropertys.Sparse, (newSize + 7) >> 3);
    utResizeArray(dvPropertys.View, (newSize + 7) >> 3);
    utResizeArray(dvPropertys.Expanded, (newSize + 7) >> 3);
    utResizeArray(dvPropertys.FieldNumber, (newSize));
    utResizeArray(dvPropertys.FirstElementProp, (newSize));
    utResizeArray(dvPropertys.NumElementsProp, (newSize));
    utResizeArray(dvPropertys.Hidden, (newSize + 7) >> 3);
    utResizeArray(dvPropertys.InitializerIndex_, (newSize));
    utResizeArray(dvPropertys.NumInitializer, (newSize));
    utResizeArray(dvPropertys.FixedSize, (newSize + 7) >> 3);
    utResizeArray(dvPropertys.IndexIndex_, (newSize));
    utResizeArray(dvPropertys.NumIndex, (newSize));
    utResizeArray(dvPropertys.Line, (newSize));
    utResizeArray(dvPropertys.Class, (newSize));
    utResizeArray(dvPropertys.NextClassProperty, (newSize));
    utResizeArray(dvPropertys.PrevClassProperty, (newSize));
    utResizeArray(dvPropertys.NextTableClassProperty, (newSize));
    utResizeArray(dvPropertys.FirstCase, (newSize));
    utResizeArray(dvPropertys.LastCase, (newSize));
    utResizeArray(dvPropertys.FirstKeyproperty, (newSize));
    utResizeArray(dvPropertys.LastKeyproperty, (newSize));
    utResizeArray(dvPropertys.Sparsegroup, (newSize));
    utResizeArray(dvPropertys.NextSparsegroupProperty, (newSize));
    utResizeArray(dvPropertys.Relationship, (newSize));
    utResizeArray(dvPropertys.NextRelationshipProperty, (newSize));
    utResizeArray(dvPropertys.Union, (newSize));
    utResizeArray(dvPropertys.NextUnionProperty, (newSize));
    utResizeArray(dvPropertys.Cache, (newSize));
    utResizeArray(dvPropertys.NextCacheProperty, (newSize));
    utResizeArray(dvPropertys.union1, newSize);
    dvSetAllocatedProperty(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Propertys.
----------------------------------------------------------------------------------------*/
void dvPropertyAllocMore(void)
{
    reallocPropertys((uint32)(dvAllocatedProperty() + (dvAllocatedProperty() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Compact the Property.Initializer heap to free memory.
----------------------------------------------------------------------------------------*/
void dvCompactPropertyInitializers(void)
{
    uint32 elementSize = sizeof(char);
    uint32 usedHeaderSize = (sizeof(dvProperty) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvProperty) + sizeof(uint32) + elementSize - 1)/elementSize;
    char *toPtr = dvPropertys.Initializer;
    char *fromPtr = toPtr;
    dvProperty Property;
    uint32 size;

    while(fromPtr < dvPropertys.Initializer + dvUsedPropertyInitializer()) {
        Property = *(dvProperty *)(void *)fromPtr;
        if(Property != dvPropertyNull) {
            /* Need to move it to toPtr */
            size = utMax(dvPropertyGetNumInitializer(Property) + usedHeaderSize, freeHeaderSize);
            memmove((void *)toPtr, (void *)fromPtr, size*elementSize);
            dvPropertySetInitializerIndex_(Property, toPtr - dvPropertys.Initializer + usedHeaderSize);
            toPtr += size;
        } else {
            /* Just skip it */
            size = *(uint32 *)(void *)(((dvProperty *)(void *)fromPtr) + 1);
        }
        fromPtr += size;
    }
    dvSetUsedPropertyInitializer(toPtr - dvPropertys.Initializer);
    dvSetFreePropertyInitializer(0);
}

/*----------------------------------------------------------------------------------------
  Allocate more memory for the Property.Initializer heap.
----------------------------------------------------------------------------------------*/
static void allocMorePropertyInitializers(
    uint32 spaceNeeded)
{
    uint32 freeSpace = dvAllocatedPropertyInitializer() - dvUsedPropertyInitializer();

    if((dvFreePropertyInitializer() << 2) > dvUsedPropertyInitializer()) {
        dvCompactPropertyInitializers();
        freeSpace = dvAllocatedPropertyInitializer() - dvUsedPropertyInitializer();
    }
    if(freeSpace < spaceNeeded) {
        dvSetAllocatedPropertyInitializer(dvAllocatedPropertyInitializer() + spaceNeeded - freeSpace +
            (dvAllocatedPropertyInitializer() >> 1));
        utResizeArray(dvPropertys.Initializer, dvAllocatedPropertyInitializer());
    }
}

/*----------------------------------------------------------------------------------------
  Allocate memory for a new Property.Initializer array.
----------------------------------------------------------------------------------------*/
void dvPropertyAllocInitializers(
    dvProperty Property,
    uint32 numInitializers)
{
    uint32 freeSpace = dvAllocatedPropertyInitializer() - dvUsedPropertyInitializer();
    uint32 elementSize = sizeof(char);
    uint32 usedHeaderSize = (sizeof(dvProperty) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvProperty) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 spaceNeeded = utMax(numInitializers + usedHeaderSize, freeHeaderSize);

#if defined(DD_DEBUG)
    utAssert(dvPropertyGetNumInitializer(Property) == 0);
#endif
    if(numInitializers == 0) {
        return;
    }
    if(freeSpace < spaceNeeded) {
        allocMorePropertyInitializers(spaceNeeded);
    }
    dvPropertySetInitializerIndex_(Property, dvUsedPropertyInitializer() + usedHeaderSize);
    dvPropertySetNumInitializer(Property, numInitializers);
    *(dvProperty *)(void *)(dvPropertys.Initializer + dvUsedPropertyInitializer()) = Property;
    memset(dvPropertys.Initializer + dvPropertyGetInitializerIndex_(Property), 0, ((numInitializers))*sizeof(char));
    dvSetUsedPropertyInitializer(dvUsedPropertyInitializer() + spaceNeeded);
}

/*----------------------------------------------------------------------------------------
  Wrapper around dvPropertyGetInitializers for the database manager.
----------------------------------------------------------------------------------------*/
static void *getPropertyInitializers(
    uint64 objectNumber,
    uint32 *numValues)
{
    dvProperty Property = dvIndex2Property((uint32)objectNumber);

    *numValues = dvPropertyGetNumInitializer(Property);
    return dvPropertyGetInitializers(Property);
}

/*----------------------------------------------------------------------------------------
  Wrapper around dvPropertyAllocInitializers for the database manager.
----------------------------------------------------------------------------------------*/
static void *allocPropertyInitializers(
    uint64 objectNumber,
    uint32 numValues)
{
    dvProperty Property = dvIndex2Property((uint32)objectNumber);

    dvPropertySetInitializerIndex_(Property, 0);
    dvPropertySetNumInitializer(Property, 0);
    if(numValues == 0) {
        return NULL;
    }
    dvPropertyAllocInitializers(Property, numValues);
    return dvPropertyGetInitializers(Property);
}

/*----------------------------------------------------------------------------------------
  Free memory used by the Property.Initializer array.
----------------------------------------------------------------------------------------*/
void dvPropertyFreeInitializers(
    dvProperty Property)
{
    uint32 elementSize = sizeof(char);
    uint32 usedHeaderSize = (sizeof(dvProperty) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvProperty) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 size = utMax(dvPropertyGetNumInitializer(Property) + usedHeaderSize, freeHeaderSize);
    char *dataPtr = dvPropertyGetInitializers(Property) - usedHeaderSize;

    if(dvPropertyGetNumInitializer(Property) == 0) {
        return;
    }
    *(dvProperty *)(void *)(dataPtr) = dvPropertyNull;
    *(uint32 *)(void *)(((dvProperty *)(void *)dataPtr) + 1) = size;
    dvPropertySetNumInitializer(Property, 0);
    dvSetFreePropertyInitializer(dvFreePropertyInitializer() + size);
}

/*----------------------------------------------------------------------------------------
  Resize the Property.Initializer array.
----------------------------------------------------------------------------------------*/
void dvPropertyResizeInitializers(
    dvProperty Property,
    uint32 numInitializers)
{
    uint32 freeSpace;
    uint32 elementSize = sizeof(char);
    uint32 usedHeaderSize = (sizeof(dvProperty) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvProperty) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 newSize = utMax(numInitializers + usedHeaderSize, freeHeaderSize);
    uint32 oldSize = utMax(dvPropertyGetNumInitializer(Property) + usedHeaderSize, freeHeaderSize);
    char *dataPtr;

    if(numInitializers == 0) {
        if(dvPropertyGetNumInitializer(Property) != 0) {
            dvPropertyFreeInitializers(Property);
        }
        return;
    }
    if(dvPropertyGetNumInitializer(Property) == 0) {
        dvPropertyAllocInitializers(Property, numInitializers);
        return;
    }
    freeSpace = dvAllocatedPropertyInitializer() - dvUsedPropertyInitializer();
    if(freeSpace < newSize) {
        allocMorePropertyInitializers(newSize);
    }
    dataPtr = dvPropertyGetInitializers(Property) - usedHeaderSize;
    memcpy((void *)(dvPropertys.Initializer + dvUsedPropertyInitializer()), dataPtr,
        elementSize*utMin(oldSize, newSize));
    if(newSize > oldSize) {
        memset(dvPropertys.Initializer + dvUsedPropertyInitializer() + oldSize, 0, ((newSize - oldSize))*sizeof(char));
    }
    *(dvProperty *)(void *)dataPtr = dvPropertyNull;
    *(uint32 *)(void *)(((dvProperty *)(void *)dataPtr) + 1) = oldSize;
    dvSetFreePropertyInitializer(dvFreePropertyInitializer() + oldSize);
    dvPropertySetInitializerIndex_(Property, dvUsedPropertyInitializer() + usedHeaderSize);
    dvPropertySetNumInitializer(Property, numInitializers);
    dvSetUsedPropertyInitializer(dvUsedPropertyInitializer() + newSize);
}

/*----------------------------------------------------------------------------------------
  Compact the Property.Index heap to free memory.
----------------------------------------------------------------------------------------*/
void dvCompactPropertyIndexs(void)
{
    uint32 elementSize = sizeof(char);
    uint32 usedHeaderSize = (sizeof(dvProperty) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvProperty) + sizeof(uint32) + elementSize - 1)/elementSize;
    char *toPtr = dvPropertys.Index;
    char *fromPtr = toPtr;
    dvProperty Property;
    uint32 size;

    while(fromPtr < dvPropertys.Index + dvUsedPropertyIndex()) {
        Property = *(dvProperty *)(void *)fromPtr;
        if(Property != dvPropertyNull) {
            /* Need to move it to toPtr */
            size = utMax(dvPropertyGetNumIndex(Property) + usedHeaderSize, freeHeaderSize);
            memmove((void *)toPtr, (void *)fromPtr, size*elementSize);
            dvPropertySetIndexIndex_(Property, toPtr - dvPropertys.Index + usedHeaderSize);
            toPtr += size;
        } else {
            /* Just skip it */
            size = *(uint32 *)(void *)(((dvProperty *)(void *)fromPtr) + 1);
        }
        fromPtr += size;
    }
    dvSetUsedPropertyIndex(toPtr - dvPropertys.Index);
    dvSetFreePropertyIndex(0);
}

/*----------------------------------------------------------------------------------------
  Allocate more memory for the Property.Index heap.
----------------------------------------------------------------------------------------*/
static void allocMorePropertyIndexs(
    uint32 spaceNeeded)
{
    uint32 freeSpace = dvAllocatedPropertyIndex() - dvUsedPropertyIndex();

    if((dvFreePropertyIndex() << 2) > dvUsedPropertyIndex()) {
        dvCompactPropertyIndexs();
        freeSpace = dvAllocatedPropertyIndex() - dvUsedPropertyIndex();
    }
    if(freeSpace < spaceNeeded) {
        dvSetAllocatedPropertyIndex(dvAllocatedPropertyIndex() + spaceNeeded - freeSpace +
            (dvAllocatedPropertyIndex() >> 1));
        utResizeArray(dvPropertys.Index, dvAllocatedPropertyIndex());
    }
}

/*----------------------------------------------------------------------------------------
  Allocate memory for a new Property.Index array.
----------------------------------------------------------------------------------------*/
void dvPropertyAllocIndexs(
    dvProperty Property,
    uint32 numIndexs)
{
    uint32 freeSpace = dvAllocatedPropertyIndex() - dvUsedPropertyIndex();
    uint32 elementSize = sizeof(char);
    uint32 usedHeaderSize = (sizeof(dvProperty) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvProperty) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 spaceNeeded = utMax(numIndexs + usedHeaderSize, freeHeaderSize);

#if defined(DD_DEBUG)
    utAssert(dvPropertyGetNumIndex(Property) == 0);
#endif
    if(numIndexs == 0) {
        return;
    }
    if(freeSpace < spaceNeeded) {
        allocMorePropertyIndexs(spaceNeeded);
    }
    dvPropertySetIndexIndex_(Property, dvUsedPropertyIndex() + usedHeaderSize);
    dvPropertySetNumIndex(Property, numIndexs);
    *(dvProperty *)(void *)(dvPropertys.Index + dvUsedPropertyIndex()) = Property;
    memset(dvPropertys.Index + dvPropertyGetIndexIndex_(Property), 0, ((numIndexs))*sizeof(char));
    dvSetUsedPropertyIndex(dvUsedPropertyIndex() + spaceNeeded);
}

/*----------------------------------------------------------------------------------------
  Wrapper around dvPropertyGetIndexs for the database manager.
----------------------------------------------------------------------------------------*/
static void *getPropertyIndexs(
    uint64 objectNumber,
    uint32 *numValues)
{
    dvProperty Property = dvIndex2Property((uint32)objectNumber);

    *numValues = dvPropertyGetNumIndex(Property);
    return dvPropertyGetIndexs(Property);
}

/*----------------------------------------------------------------------------------------
  Wrapper around dvPropertyAllocIndexs for the database manager.
----------------------------------------------------------------------------------------*/
static void *allocPropertyIndexs(
    uint64 objectNumber,
    uint32 numValues)
{
    dvProperty Property = dvIndex2Property((uint32)objectNumber);

    dvPropertySetIndexIndex_(Property, 0);
    dvPropertySetNumIndex(Property, 0);
    if(numValues == 0) {
        return NULL;
    }
    dvPropertyAllocIndexs(Property, numValues);
    return dvPropertyGetIndexs(Property);
}

/*----------------------------------------------------------------------------------------
  Free memory used by the Property.Index array.
----------------------------------------------------------------------------------------*/
void dvPropertyFreeIndexs(
    dvProperty Property)
{
    uint32 elementSize = sizeof(char);
    uint32 usedHeaderSize = (sizeof(dvProperty) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvProperty) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 size = utMax(dvPropertyGetNumIndex(Property) + usedHeaderSize, freeHeaderSize);
    char *dataPtr = dvPropertyGetIndexs(Property) - usedHeaderSize;

    if(dvPropertyGetNumIndex(Property) == 0) {
        return;
    }
    *(dvProperty *)(void *)(dataPtr) = dvPropertyNull;
    *(uint32 *)(void *)(((dvProperty *)(void *)dataPtr) + 1) = size;
    dvPropertySetNumIndex(Property, 0);
    dvSetFreePropertyIndex(dvFreePropertyIndex() + size);
}

/*----------------------------------------------------------------------------------------
  Resize the Property.Index array.
----------------------------------------------------------------------------------------*/
void dvPropertyResizeIndexs(
    dvProperty Property,
    uint32 numIndexs)
{
    uint32 freeSpace;
    uint32 elementSize = sizeof(char);
    uint32 usedHeaderSize = (sizeof(dvProperty) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(dvProperty) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 newSize = utMax(numIndexs + usedHeaderSize, freeHeaderSize);
    uint32 oldSize = utMax(dvPropertyGetNumIndex(Property) + usedHeaderSize, freeHeaderSize);
    char *dataPtr;

    if(numIndexs == 0) {
        if(dvPropertyGetNumIndex(Property) != 0) {
            dvPropertyFreeIndexs(Property);
        }
        return;
    }
    if(dvPropertyGetNumIndex(Property) == 0) {
        dvPropertyAllocIndexs(Property, numIndexs);
        return;
    }
    freeSpace = dvAllocatedPropertyIndex() - dvUsedPropertyIndex();
    if(freeSpace < newSize) {
        allocMorePropertyIndexs(newSize);
    }
    dataPtr = dvPropertyGetIndexs(Property) - usedHeaderSize;
    memcpy((void *)(dvPropertys.Index + dvUsedPropertyIndex()), dataPtr,
        elementSize*utMin(oldSize, newSize));
    if(newSize > oldSize) {
        memset(dvPropertys.Index + dvUsedPropertyIndex() + oldSize, 0, ((newSize - oldSize))*sizeof(char));
    }
    *(dvProperty *)(void *)dataPtr = dvPropertyNull;
    *(uint32 *)(void *)(((dvProperty *)(void *)dataPtr) + 1) = oldSize;
    dvSetFreePropertyIndex(dvFreePropertyIndex() + oldSize);
    dvPropertySetIndexIndex_(Property, dvUsedPropertyIndex() + usedHeaderSize);
    dvPropertySetNumIndex(Property, numIndexs);
    dvSetUsedPropertyIndex(dvUsedPropertyIndex() + newSize);
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Property.
----------------------------------------------------------------------------------------*/
void dvPropertyCopyProps(
    dvProperty oldProperty,
    dvProperty newProperty)
{
    dvPropertySetType(newProperty, dvPropertyGetType(oldProperty));
    dvPropertySetArray(newProperty, dvPropertyArray(oldProperty));
    dvPropertySetCascade(newProperty, dvPropertyCascade(oldProperty));
    dvPropertySetSparse(newProperty, dvPropertySparse(oldProperty));
    dvPropertySetView(newProperty, dvPropertyView(oldProperty));
    dvPropertySetExpanded(newProperty, dvPropertyExpanded(oldProperty));
    dvPropertySetFieldNumber(newProperty, dvPropertyGetFieldNumber(oldProperty));
    dvPropertySetHidden(newProperty, dvPropertyHidden(oldProperty));
    dvPropertySetFixedSize(newProperty, dvPropertyFixedSize(oldProperty));
    dvPropertySetLine(newProperty, dvPropertyGetLine(oldProperty));
}

/*----------------------------------------------------------------------------------------
  Return the integer equivalent for the bit fields in Property.
----------------------------------------------------------------------------------------*/
uint32 dvPropertyGetBitfield(
    dvProperty _Property)
{
    uint32 bitfield = 0;
    uint8 xLevel = 0;

    bitfield |= dvPropertyArray(_Property) << xLevel++;
    bitfield |= dvPropertyCascade(_Property) << xLevel++;
    bitfield |= dvPropertySparse(_Property) << xLevel++;
    bitfield |= dvPropertyView(_Property) << xLevel++;
    bitfield |= dvPropertyExpanded(_Property) << xLevel++;
    bitfield |= dvPropertyHidden(_Property) << xLevel++;
    bitfield |= dvPropertyFixedSize(_Property) << xLevel++;
    return bitfield;
}

/*----------------------------------------------------------------------------------------
  Set bit fields in Property using bitfield.
----------------------------------------------------------------------------------------*/
void dvPropertySetBitfield(
    dvProperty _Property,
     uint32 bitfield)
{
    dvPropertySetArray(_Property, bitfield & 1);
    bitfield >>= 1;
    dvPropertySetCascade(_Property, bitfield & 1);
    bitfield >>= 1;
    dvPropertySetSparse(_Property, bitfield & 1);
    bitfield >>= 1;
    dvPropertySetView(_Property, bitfield & 1);
    bitfield >>= 1;
    dvPropertySetExpanded(_Property, bitfield & 1);
    bitfield >>= 1;
    dvPropertySetHidden(_Property, bitfield & 1);
    bitfield >>= 1;
    dvPropertySetFixedSize(_Property, bitfield & 1);
    bitfield >>= 1;
}

/*----------------------------------------------------------------------------------------
  Add the Case to the head of the list on the Property.
----------------------------------------------------------------------------------------*/
void dvPropertyInsertCase(
    dvProperty Property,
    dvCase _Case)
{
#if defined(DD_DEBUG)
    if(Property == dvPropertyNull) {
        utExit("Non-existent Property");
    }
    if(_Case == dvCaseNull) {
        utExit("Non-existent Case");
    }
    if(dvCaseGetProperty(_Case) != dvPropertyNull) {
        utExit("Attempting to add Case to Property twice");
    }
#endif
    dvCaseSetNextPropertyCase(_Case, dvPropertyGetFirstCase(Property));
    dvPropertySetFirstCase(Property, _Case);
    if(dvPropertyGetLastCase(Property) == dvCaseNull) {
        dvPropertySetLastCase(Property, _Case);
    }
    dvCaseSetProperty(_Case, Property);
}

/*----------------------------------------------------------------------------------------
  Add the Case to the end of the list on the Property.
----------------------------------------------------------------------------------------*/
void dvPropertyAppendCase(
    dvProperty Property,
    dvCase _Case)
{
#if defined(DD_DEBUG)
    if(Property == dvPropertyNull) {
        utExit("Non-existent Property");
    }
    if(_Case == dvCaseNull) {
        utExit("Non-existent Case");
    }
    if(dvCaseGetProperty(_Case) != dvPropertyNull) {
        utExit("Attempting to add Case to Property twice");
    }
#endif
    if(dvPropertyGetLastCase(Property) != dvCaseNull) {
        dvCaseSetNextPropertyCase(dvPropertyGetLastCase(Property), _Case);
    } else {
        dvPropertySetFirstCase(Property, _Case);
    }
    dvPropertySetLastCase(Property, _Case);
    dvCaseSetNextPropertyCase(_Case, dvCaseNull);
    dvCaseSetProperty(_Case, Property);
}

/*----------------------------------------------------------------------------------------
  Insert the Case to the Property after the previous Case.
----------------------------------------------------------------------------------------*/
void dvPropertyInsertAfterCase(
    dvProperty Property,
    dvCase prevCase,
    dvCase _Case)
{
    dvCase nextCase = dvCaseGetNextPropertyCase(prevCase);

#if defined(DD_DEBUG)
    if(Property == dvPropertyNull) {
        utExit("Non-existent Property");
    }
    if(_Case == dvCaseNull) {
        utExit("Non-existent Case");
    }
    if(dvCaseGetProperty(_Case) != dvPropertyNull) {
        utExit("Attempting to add Case to Property twice");
    }
#endif
    dvCaseSetNextPropertyCase(_Case, nextCase);
    dvCaseSetNextPropertyCase(prevCase, _Case);
    if(dvPropertyGetLastCase(Property) == prevCase) {
        dvPropertySetLastCase(Property, _Case);
    }
    dvCaseSetProperty(_Case, Property);
}

/*----------------------------------------------------------------------------------------
 Remove the Case from the Property.
----------------------------------------------------------------------------------------*/
void dvPropertyRemoveCase(
    dvProperty Property,
    dvCase _Case)
{
    dvCase pCase, nCase;

#if defined(DD_DEBUG)
    if(_Case == dvCaseNull) {
        utExit("Non-existent Case");
    }
    if(dvCaseGetProperty(_Case) != dvPropertyNull && dvCaseGetProperty(_Case) != Property) {
        utExit("Delete Case from non-owning Property");
    }
#endif
    pCase = dvCaseNull;
    for(nCase = dvPropertyGetFirstCase(Property); nCase != dvCaseNull && nCase != _Case;
            nCase = dvCaseGetNextPropertyCase(nCase)) {
        pCase = nCase;
    }
    if(pCase != dvCaseNull) {
        dvCaseSetNextPropertyCase(pCase, dvCaseGetNextPropertyCase(_Case));
    } else {
        dvPropertySetFirstCase(Property, dvCaseGetNextPropertyCase(_Case));
    }
    dvCaseSetNextPropertyCase(_Case, dvCaseNull);
    if(dvPropertyGetLastCase(Property) == _Case) {
        dvPropertySetLastCase(Property, pCase);
    }
    dvCaseSetProperty(_Case, dvPropertyNull);
}

/*----------------------------------------------------------------------------------------
  Add the Keyproperty to the head of the list on the Property.
----------------------------------------------------------------------------------------*/
void dvPropertyInsertKeyproperty(
    dvProperty Property,
    dvKeyproperty _Keyproperty)
{
#if defined(DD_DEBUG)
    if(Property == dvPropertyNull) {
        utExit("Non-existent Property");
    }
    if(_Keyproperty == dvKeypropertyNull) {
        utExit("Non-existent Keyproperty");
    }
    if(dvKeypropertyGetProperty(_Keyproperty) != dvPropertyNull) {
        utExit("Attempting to add Keyproperty to Property twice");
    }
#endif
    dvKeypropertySetNextPropertyKeyproperty(_Keyproperty, dvPropertyGetFirstKeyproperty(Property));
    dvPropertySetFirstKeyproperty(Property, _Keyproperty);
    if(dvPropertyGetLastKeyproperty(Property) == dvKeypropertyNull) {
        dvPropertySetLastKeyproperty(Property, _Keyproperty);
    }
    dvKeypropertySetProperty(_Keyproperty, Property);
}

/*----------------------------------------------------------------------------------------
  Add the Keyproperty to the end of the list on the Property.
----------------------------------------------------------------------------------------*/
void dvPropertyAppendKeyproperty(
    dvProperty Property,
    dvKeyproperty _Keyproperty)
{
#if defined(DD_DEBUG)
    if(Property == dvPropertyNull) {
        utExit("Non-existent Property");
    }
    if(_Keyproperty == dvKeypropertyNull) {
        utExit("Non-existent Keyproperty");
    }
    if(dvKeypropertyGetProperty(_Keyproperty) != dvPropertyNull) {
        utExit("Attempting to add Keyproperty to Property twice");
    }
#endif
    if(dvPropertyGetLastKeyproperty(Property) != dvKeypropertyNull) {
        dvKeypropertySetNextPropertyKeyproperty(dvPropertyGetLastKeyproperty(Property), _Keyproperty);
    } else {
        dvPropertySetFirstKeyproperty(Property, _Keyproperty);
    }
    dvPropertySetLastKeyproperty(Property, _Keyproperty);
    dvKeypropertySetNextPropertyKeyproperty(_Keyproperty, dvKeypropertyNull);
    dvKeypropertySetProperty(_Keyproperty, Property);
}

/*----------------------------------------------------------------------------------------
  Insert the Keyproperty to the Property after the previous Keyproperty.
----------------------------------------------------------------------------------------*/
void dvPropertyInsertAfterKeyproperty(
    dvProperty Property,
    dvKeyproperty prevKeyproperty,
    dvKeyproperty _Keyproperty)
{
    dvKeyproperty nextKeyproperty = dvKeypropertyGetNextPropertyKeyproperty(prevKeyproperty);

#if defined(DD_DEBUG)
    if(Property == dvPropertyNull) {
        utExit("Non-existent Property");
    }
    if(_Keyproperty == dvKeypropertyNull) {
        utExit("Non-existent Keyproperty");
    }
    if(dvKeypropertyGetProperty(_Keyproperty) != dvPropertyNull) {
        utExit("Attempting to add Keyproperty to Property twice");
    }
#endif
    dvKeypropertySetNextPropertyKeyproperty(_Keyproperty, nextKeyproperty);
    dvKeypropertySetNextPropertyKeyproperty(prevKeyproperty, _Keyproperty);
    if(dvPropertyGetLastKeyproperty(Property) == prevKeyproperty) {
        dvPropertySetLastKeyproperty(Property, _Keyproperty);
    }
    dvKeypropertySetProperty(_Keyproperty, Property);
}

/*----------------------------------------------------------------------------------------
 Remove the Keyproperty from the Property.
----------------------------------------------------------------------------------------*/
void dvPropertyRemoveKeyproperty(
    dvProperty Property,
    dvKeyproperty _Keyproperty)
{
    dvKeyproperty pKeyproperty, nKeyproperty;

#if defined(DD_DEBUG)
    if(_Keyproperty == dvKeypropertyNull) {
        utExit("Non-existent Keyproperty");
    }
    if(dvKeypropertyGetProperty(_Keyproperty) != dvPropertyNull && dvKeypropertyGetProperty(_Keyproperty) != Property) {
        utExit("Delete Keyproperty from non-owning Property");
    }
#endif
    pKeyproperty = dvKeypropertyNull;
    for(nKeyproperty = dvPropertyGetFirstKeyproperty(Property); nKeyproperty != dvKeypropertyNull && nKeyproperty != _Keyproperty;
            nKeyproperty = dvKeypropertyGetNextPropertyKeyproperty(nKeyproperty)) {
        pKeyproperty = nKeyproperty;
    }
    if(pKeyproperty != dvKeypropertyNull) {
        dvKeypropertySetNextPropertyKeyproperty(pKeyproperty, dvKeypropertyGetNextPropertyKeyproperty(_Keyproperty));
    } else {
        dvPropertySetFirstKeyproperty(Property, dvKeypropertyGetNextPropertyKeyproperty(_Keyproperty));
    }
    dvKeypropertySetNextPropertyKeyproperty(_Keyproperty, dvKeypropertyNull);
    if(dvPropertyGetLastKeyproperty(Property) == _Keyproperty) {
        dvPropertySetLastKeyproperty(Property, pKeyproperty);
    }
    dvKeypropertySetProperty(_Keyproperty, dvPropertyNull);
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void dvShowProperty(
    dvProperty Property)
{
    utDatabaseShowObject("dv", "Property", dvProperty2Index(Property));
}
#endif

/*----------------------------------------------------------------------------------------
  Destroy Sparsegroup including everything in it. Remove from parents.
----------------------------------------------------------------------------------------*/
void dvSparsegroupDestroy(
    dvSparsegroup Sparsegroup)
{
    dvProperty Property_;
    dvClass owningClass = dvSparsegroupGetClass(Sparsegroup);
    dvRelationship owningRelationship = dvSparsegroupGetRelationship(Sparsegroup);

    if(dvSparsegroupDestructorCallback != NULL) {
        dvSparsegroupDestructorCallback(Sparsegroup);
    }
    dvSafeForeachSparsegroupProperty(Sparsegroup, Property_) {
        dvPropertySetSparsegroup(Property_, dvSparsegroupNull);
    } dvEndSafeSparsegroupProperty;
    if(owningClass != dvClassNull) {
        dvClassRemoveSparsegroup(owningClass, Sparsegroup);
#if defined(DD_DEBUG)
    } else {
        utExit("Sparsegroup without owning Class");
#endif
    }
    if(owningRelationship != dvRelationshipNull) {
        dvRelationshipSetParentSparsegroup(owningRelationship, dvSparsegroupNull);
    }
    if(owningRelationship != dvRelationshipNull) {
        dvRelationshipSetChildSparsegroup(owningRelationship, dvSparsegroupNull);
    }
    dvSparsegroupFree(Sparsegroup);
}

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocSparsegroup(void)
{
    dvSparsegroup Sparsegroup = dvSparsegroupAlloc();

    return dvSparsegroup2Index(Sparsegroup);
}

/*----------------------------------------------------------------------------------------
  Destructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static void destroySparsegroup(
    uint64 objectIndex)
{
    dvSparsegroupDestroy(dvIndex2Sparsegroup((uint32)objectIndex));
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Sparsegroup.
----------------------------------------------------------------------------------------*/
static void allocSparsegroups(void)
{
    dvSetAllocatedSparsegroup(2);
    dvSetUsedSparsegroup(1);
    dvSetFirstFreeSparsegroup(dvSparsegroupNull);
    dvSparsegroups.Sym = utNewAInitFirst(utSym, (dvAllocatedSparsegroup()));
    dvSparsegroups.Class = utNewAInitFirst(dvClass, (dvAllocatedSparsegroup()));
    dvSparsegroups.NextClassSparsegroup = utNewAInitFirst(dvSparsegroup, (dvAllocatedSparsegroup()));
    dvSparsegroups.PrevClassSparsegroup = utNewAInitFirst(dvSparsegroup, (dvAllocatedSparsegroup()));
    dvSparsegroups.NextTableClassSparsegroup = utNewAInitFirst(dvSparsegroup, (dvAllocatedSparsegroup()));
    dvSparsegroups.FirstProperty = utNewAInitFirst(dvProperty, (dvAllocatedSparsegroup()));
    dvSparsegroups.LastProperty = utNewAInitFirst(dvProperty, (dvAllocatedSparsegroup()));
    dvSparsegroups.Relationship = utNewAInitFirst(dvRelationship, (dvAllocatedSparsegroup()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Sparsegroup.
----------------------------------------------------------------------------------------*/
static void reallocSparsegroups(
    uint32 newSize)
{
    utResizeArray(dvSparsegroups.Sym, (newSize));
    utResizeArray(dvSparsegroups.Class, (newSize));
    utResizeArray(dvSparsegroups.NextClassSparsegroup, (newSize));
    utResizeArray(dvSparsegroups.PrevClassSparsegroup, (newSize));
    utResizeArray(dvSparsegroups.NextTableClassSparsegroup, (newSize));
    utResizeArray(dvSparsegroups.FirstProperty, (newSize));
    utResizeArray(dvSparsegroups.LastProperty, (newSize));
    utResizeArray(dvSparsegroups.Relationship, (newSize));
    dvSetAllocatedSparsegroup(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Sparsegroups.
----------------------------------------------------------------------------------------*/
void dvSparsegroupAllocMore(void)
{
    reallocSparsegroups((uint32)(dvAllocatedSparsegroup() + (dvAllocatedSparsegroup() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Sparsegroup.
----------------------------------------------------------------------------------------*/
void dvSparsegroupCopyProps(
    dvSparsegroup oldSparsegroup,
    dvSparsegroup newSparsegroup)
{
}

/*----------------------------------------------------------------------------------------
  Add the Property to the head of the list on the Sparsegroup.
----------------------------------------------------------------------------------------*/
void dvSparsegroupInsertProperty(
    dvSparsegroup Sparsegroup,
    dvProperty _Property)
{
#if defined(DD_DEBUG)
    if(Sparsegroup == dvSparsegroupNull) {
        utExit("Non-existent Sparsegroup");
    }
    if(_Property == dvPropertyNull) {
        utExit("Non-existent Property");
    }
    if(dvPropertyGetSparsegroup(_Property) != dvSparsegroupNull) {
        utExit("Attempting to add Property to Sparsegroup twice");
    }
#endif
    dvPropertySetNextSparsegroupProperty(_Property, dvSparsegroupGetFirstProperty(Sparsegroup));
    dvSparsegroupSetFirstProperty(Sparsegroup, _Property);
    if(dvSparsegroupGetLastProperty(Sparsegroup) == dvPropertyNull) {
        dvSparsegroupSetLastProperty(Sparsegroup, _Property);
    }
    dvPropertySetSparsegroup(_Property, Sparsegroup);
}

/*----------------------------------------------------------------------------------------
  Add the Property to the end of the list on the Sparsegroup.
----------------------------------------------------------------------------------------*/
void dvSparsegroupAppendProperty(
    dvSparsegroup Sparsegroup,
    dvProperty _Property)
{
#if defined(DD_DEBUG)
    if(Sparsegroup == dvSparsegroupNull) {
        utExit("Non-existent Sparsegroup");
    }
    if(_Property == dvPropertyNull) {
        utExit("Non-existent Property");
    }
    if(dvPropertyGetSparsegroup(_Property) != dvSparsegroupNull) {
        utExit("Attempting to add Property to Sparsegroup twice");
    }
#endif
    if(dvSparsegroupGetLastProperty(Sparsegroup) != dvPropertyNull) {
        dvPropertySetNextSparsegroupProperty(dvSparsegroupGetLastProperty(Sparsegroup), _Property);
    } else {
        dvSparsegroupSetFirstProperty(Sparsegroup, _Property);
    }
    dvSparsegroupSetLastProperty(Sparsegroup, _Property);
    dvPropertySetNextSparsegroupProperty(_Property, dvPropertyNull);
    dvPropertySetSparsegroup(_Property, Sparsegroup);
}

/*----------------------------------------------------------------------------------------
  Insert the Property to the Sparsegroup after the previous Property.
----------------------------------------------------------------------------------------*/
void dvSparsegroupInsertAfterProperty(
    dvSparsegroup Sparsegroup,
    dvProperty prevProperty,
    dvProperty _Property)
{
    dvProperty nextProperty = dvPropertyGetNextSparsegroupProperty(prevProperty);

#if defined(DD_DEBUG)
    if(Sparsegroup == dvSparsegroupNull) {
        utExit("Non-existent Sparsegroup");
    }
    if(_Property == dvPropertyNull) {
        utExit("Non-existent Property");
    }
    if(dvPropertyGetSparsegroup(_Property) != dvSparsegroupNull) {
        utExit("Attempting to add Property to Sparsegroup twice");
    }
#endif
    dvPropertySetNextSparsegroupProperty(_Property, nextProperty);
    dvPropertySetNextSparsegroupProperty(prevProperty, _Property);
    if(dvSparsegroupGetLastProperty(Sparsegroup) == prevProperty) {
        dvSparsegroupSetLastProperty(Sparsegroup, _Property);
    }
    dvPropertySetSparsegroup(_Property, Sparsegroup);
}

/*----------------------------------------------------------------------------------------
 Remove the Property from the Sparsegroup.
----------------------------------------------------------------------------------------*/
void dvSparsegroupRemoveProperty(
    dvSparsegroup Sparsegroup,
    dvProperty _Property)
{
    dvProperty pProperty, nProperty;

#if defined(DD_DEBUG)
    if(_Property == dvPropertyNull) {
        utExit("Non-existent Property");
    }
    if(dvPropertyGetSparsegroup(_Property) != dvSparsegroupNull && dvPropertyGetSparsegroup(_Property) != Sparsegroup) {
        utExit("Delete Property from non-owning Sparsegroup");
    }
#endif
    pProperty = dvPropertyNull;
    for(nProperty = dvSparsegroupGetFirstProperty(Sparsegroup); nProperty != dvPropertyNull && nProperty != _Property;
            nProperty = dvPropertyGetNextSparsegroupProperty(nProperty)) {
        pProperty = nProperty;
    }
    if(pProperty != dvPropertyNull) {
        dvPropertySetNextSparsegroupProperty(pProperty, dvPropertyGetNextSparsegroupProperty(_Property));
    } else {
        dvSparsegroupSetFirstProperty(Sparsegroup, dvPropertyGetNextSparsegroupProperty(_Property));
    }
    dvPropertySetNextSparsegroupProperty(_Property, dvPropertyNull);
    if(dvSparsegroupGetLastProperty(Sparsegroup) == _Property) {
        dvSparsegroupSetLastProperty(Sparsegroup, pProperty);
    }
    dvPropertySetSparsegroup(_Property, dvSparsegroupNull);
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void dvShowSparsegroup(
    dvSparsegroup Sparsegroup)
{
    utDatabaseShowObject("dv", "Sparsegroup", dvSparsegroup2Index(Sparsegroup));
}
#endif

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocRelationship(void)
{
    dvRelationship Relationship = dvRelationshipAlloc();

    return dvRelationship2Index(Relationship);
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Relationship.
----------------------------------------------------------------------------------------*/
static void allocRelationships(void)
{
    dvSetAllocatedRelationship(2);
    dvSetUsedRelationship(1);
    dvRelationships.Type = utNewAInitFirst(dvRelationshipType, (dvAllocatedRelationship()));
    dvRelationships.ParentLabelSym = utNewAInitFirst(utSym, (dvAllocatedRelationship()));
    dvRelationships.ChildLabelSym = utNewAInitFirst(utSym, (dvAllocatedRelationship()));
    dvRelationships.Mandatory = utNewAInitFirst(uint8, (dvAllocatedRelationship() + 7) >> 3);
    dvRelationships.Cascade = utNewAInitFirst(uint8, (dvAllocatedRelationship() + 7) >> 3);
    dvRelationships.AccessChild = utNewAInitFirst(uint8, (dvAllocatedRelationship() + 7) >> 3);
    dvRelationships.AccessParent = utNewAInitFirst(uint8, (dvAllocatedRelationship() + 7) >> 3);
    dvRelationships.SharedParent = utNewAInitFirst(uint8, (dvAllocatedRelationship() + 7) >> 3);
    dvRelationships.Sparse = utNewAInitFirst(uint8, (dvAllocatedRelationship() + 7) >> 3);
    dvRelationships.Expanded = utNewAInitFirst(uint8, (dvAllocatedRelationship() + 7) >> 3);
    dvRelationships.Unordered = utNewAInitFirst(uint8, (dvAllocatedRelationship() + 7) >> 3);
    dvRelationships.Schema = utNewAInitFirst(dvSchema, (dvAllocatedRelationship()));
    dvRelationships.NextSchemaRelationship = utNewAInitFirst(dvRelationship, (dvAllocatedRelationship()));
    dvRelationships.ParentClass = utNewAInitFirst(dvClass, (dvAllocatedRelationship()));
    dvRelationships.NextClassChildRelationship = utNewAInitFirst(dvRelationship, (dvAllocatedRelationship()));
    dvRelationships.ChildClass = utNewAInitFirst(dvClass, (dvAllocatedRelationship()));
    dvRelationships.NextClassParentRelationship = utNewAInitFirst(dvRelationship, (dvAllocatedRelationship()));
    dvRelationships.FirstProperty = utNewAInitFirst(dvProperty, (dvAllocatedRelationship()));
    dvRelationships.LastProperty = utNewAInitFirst(dvProperty, (dvAllocatedRelationship()));
    dvRelationships.FirstKey = utNewAInitFirst(dvKey, (dvAllocatedRelationship()));
    dvRelationships.LastKey = utNewAInitFirst(dvKey, (dvAllocatedRelationship()));
    dvRelationships.ParentSparsegroup = utNewAInitFirst(dvSparsegroup, (dvAllocatedRelationship()));
    dvRelationships.ChildSparsegroup = utNewAInitFirst(dvSparsegroup, (dvAllocatedRelationship()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Relationship.
----------------------------------------------------------------------------------------*/
static void reallocRelationships(
    uint32 newSize)
{
    utResizeArray(dvRelationships.Type, (newSize));
    utResizeArray(dvRelationships.ParentLabelSym, (newSize));
    utResizeArray(dvRelationships.ChildLabelSym, (newSize));
    utResizeArray(dvRelationships.Mandatory, (newSize + 7) >> 3);
    utResizeArray(dvRelationships.Cascade, (newSize + 7) >> 3);
    utResizeArray(dvRelationships.AccessChild, (newSize + 7) >> 3);
    utResizeArray(dvRelationships.AccessParent, (newSize + 7) >> 3);
    utResizeArray(dvRelationships.SharedParent, (newSize + 7) >> 3);
    utResizeArray(dvRelationships.Sparse, (newSize + 7) >> 3);
    utResizeArray(dvRelationships.Expanded, (newSize + 7) >> 3);
    utResizeArray(dvRelationships.Unordered, (newSize + 7) >> 3);
    utResizeArray(dvRelationships.Schema, (newSize));
    utResizeArray(dvRelationships.NextSchemaRelationship, (newSize));
    utResizeArray(dvRelationships.ParentClass, (newSize));
    utResizeArray(dvRelationships.NextClassChildRelationship, (newSize));
    utResizeArray(dvRelationships.ChildClass, (newSize));
    utResizeArray(dvRelationships.NextClassParentRelationship, (newSize));
    utResizeArray(dvRelationships.FirstProperty, (newSize));
    utResizeArray(dvRelationships.LastProperty, (newSize));
    utResizeArray(dvRelationships.FirstKey, (newSize));
    utResizeArray(dvRelationships.LastKey, (newSize));
    utResizeArray(dvRelationships.ParentSparsegroup, (newSize));
    utResizeArray(dvRelationships.ChildSparsegroup, (newSize));
    dvSetAllocatedRelationship(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Relationships.
----------------------------------------------------------------------------------------*/
void dvRelationshipAllocMore(void)
{
    reallocRelationships((uint32)(dvAllocatedRelationship() + (dvAllocatedRelationship() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Relationship.
----------------------------------------------------------------------------------------*/
void dvRelationshipCopyProps(
    dvRelationship oldRelationship,
    dvRelationship newRelationship)
{
    dvRelationshipSetType(newRelationship, dvRelationshipGetType(oldRelationship));
    dvRelationshipSetParentLabelSym(newRelationship, dvRelationshipGetParentLabelSym(oldRelationship));
    dvRelationshipSetChildLabelSym(newRelationship, dvRelationshipGetChildLabelSym(oldRelationship));
    dvRelationshipSetMandatory(newRelationship, dvRelationshipMandatory(oldRelationship));
    dvRelationshipSetCascade(newRelationship, dvRelationshipCascade(oldRelationship));
    dvRelationshipSetAccessChild(newRelationship, dvRelationshipAccessChild(oldRelationship));
    dvRelationshipSetAccessParent(newRelationship, dvRelationshipAccessParent(oldRelationship));
    dvRelationshipSetSharedParent(newRelationship, dvRelationshipSharedParent(oldRelationship));
    dvRelationshipSetSparse(newRelationship, dvRelationshipSparse(oldRelationship));
    dvRelationshipSetExpanded(newRelationship, dvRelationshipExpanded(oldRelationship));
    dvRelationshipSetUnordered(newRelationship, dvRelationshipUnordered(oldRelationship));
}

/*----------------------------------------------------------------------------------------
  Return the integer equivalent for the bit fields in Relationship.
----------------------------------------------------------------------------------------*/
uint32 dvRelationshipGetBitfield(
    dvRelationship _Relationship)
{
    uint32 bitfield = 0;
    uint8 xLevel = 0;

    bitfield |= dvRelationshipMandatory(_Relationship) << xLevel++;
    bitfield |= dvRelationshipCascade(_Relationship) << xLevel++;
    bitfield |= dvRelationshipAccessChild(_Relationship) << xLevel++;
    bitfield |= dvRelationshipAccessParent(_Relationship) << xLevel++;
    bitfield |= dvRelationshipSharedParent(_Relationship) << xLevel++;
    bitfield |= dvRelationshipSparse(_Relationship) << xLevel++;
    bitfield |= dvRelationshipExpanded(_Relationship) << xLevel++;
    bitfield |= dvRelationshipUnordered(_Relationship) << xLevel++;
    return bitfield;
}

/*----------------------------------------------------------------------------------------
  Set bit fields in Relationship using bitfield.
----------------------------------------------------------------------------------------*/
void dvRelationshipSetBitfield(
    dvRelationship _Relationship,
     uint32 bitfield)
{
    dvRelationshipSetMandatory(_Relationship, bitfield & 1);
    bitfield >>= 1;
    dvRelationshipSetCascade(_Relationship, bitfield & 1);
    bitfield >>= 1;
    dvRelationshipSetAccessChild(_Relationship, bitfield & 1);
    bitfield >>= 1;
    dvRelationshipSetAccessParent(_Relationship, bitfield & 1);
    bitfield >>= 1;
    dvRelationshipSetSharedParent(_Relationship, bitfield & 1);
    bitfield >>= 1;
    dvRelationshipSetSparse(_Relationship, bitfield & 1);
    bitfield >>= 1;
    dvRelationshipSetExpanded(_Relationship, bitfield & 1);
    bitfield >>= 1;
    dvRelationshipSetUnordered(_Relationship, bitfield & 1);
    bitfield >>= 1;
}

/*----------------------------------------------------------------------------------------
  Add the Property to the head of the list on the Relationship.
----------------------------------------------------------------------------------------*/
void dvRelationshipInsertProperty(
    dvRelationship Relationship,
    dvProperty _Property)
{
#if defined(DD_DEBUG)
    if(Relationship == dvRelationshipNull) {
        utExit("Non-existent Relationship");
    }
    if(_Property == dvPropertyNull) {
        utExit("Non-existent Property");
    }
    if(dvPropertyGetRelationship(_Property) != dvRelationshipNull) {
        utExit("Attempting to add Property to Relationship twice");
    }
#endif
    dvPropertySetNextRelationshipProperty(_Property, dvRelationshipGetFirstProperty(Relationship));
    dvRelationshipSetFirstProperty(Relationship, _Property);
    if(dvRelationshipGetLastProperty(Relationship) == dvPropertyNull) {
        dvRelationshipSetLastProperty(Relationship, _Property);
    }
    dvPropertySetRelationship(_Property, Relationship);
}

/*----------------------------------------------------------------------------------------
  Add the Property to the end of the list on the Relationship.
----------------------------------------------------------------------------------------*/
void dvRelationshipAppendProperty(
    dvRelationship Relationship,
    dvProperty _Property)
{
#if defined(DD_DEBUG)
    if(Relationship == dvRelationshipNull) {
        utExit("Non-existent Relationship");
    }
    if(_Property == dvPropertyNull) {
        utExit("Non-existent Property");
    }
    if(dvPropertyGetRelationship(_Property) != dvRelationshipNull) {
        utExit("Attempting to add Property to Relationship twice");
    }
#endif
    if(dvRelationshipGetLastProperty(Relationship) != dvPropertyNull) {
        dvPropertySetNextRelationshipProperty(dvRelationshipGetLastProperty(Relationship), _Property);
    } else {
        dvRelationshipSetFirstProperty(Relationship, _Property);
    }
    dvRelationshipSetLastProperty(Relationship, _Property);
    dvPropertySetNextRelationshipProperty(_Property, dvPropertyNull);
    dvPropertySetRelationship(_Property, Relationship);
}

/*----------------------------------------------------------------------------------------
  Insert the Property to the Relationship after the previous Property.
----------------------------------------------------------------------------------------*/
void dvRelationshipInsertAfterProperty(
    dvRelationship Relationship,
    dvProperty prevProperty,
    dvProperty _Property)
{
    dvProperty nextProperty = dvPropertyGetNextRelationshipProperty(prevProperty);

#if defined(DD_DEBUG)
    if(Relationship == dvRelationshipNull) {
        utExit("Non-existent Relationship");
    }
    if(_Property == dvPropertyNull) {
        utExit("Non-existent Property");
    }
    if(dvPropertyGetRelationship(_Property) != dvRelationshipNull) {
        utExit("Attempting to add Property to Relationship twice");
    }
#endif
    dvPropertySetNextRelationshipProperty(_Property, nextProperty);
    dvPropertySetNextRelationshipProperty(prevProperty, _Property);
    if(dvRelationshipGetLastProperty(Relationship) == prevProperty) {
        dvRelationshipSetLastProperty(Relationship, _Property);
    }
    dvPropertySetRelationship(_Property, Relationship);
}

/*----------------------------------------------------------------------------------------
 Remove the Property from the Relationship.
----------------------------------------------------------------------------------------*/
void dvRelationshipRemoveProperty(
    dvRelationship Relationship,
    dvProperty _Property)
{
    dvProperty pProperty, nProperty;

#if defined(DD_DEBUG)
    if(_Property == dvPropertyNull) {
        utExit("Non-existent Property");
    }
    if(dvPropertyGetRelationship(_Property) != dvRelationshipNull && dvPropertyGetRelationship(_Property) != Relationship) {
        utExit("Delete Property from non-owning Relationship");
    }
#endif
    pProperty = dvPropertyNull;
    for(nProperty = dvRelationshipGetFirstProperty(Relationship); nProperty != dvPropertyNull && nProperty != _Property;
            nProperty = dvPropertyGetNextRelationshipProperty(nProperty)) {
        pProperty = nProperty;
    }
    if(pProperty != dvPropertyNull) {
        dvPropertySetNextRelationshipProperty(pProperty, dvPropertyGetNextRelationshipProperty(_Property));
    } else {
        dvRelationshipSetFirstProperty(Relationship, dvPropertyGetNextRelationshipProperty(_Property));
    }
    dvPropertySetNextRelationshipProperty(_Property, dvPropertyNull);
    if(dvRelationshipGetLastProperty(Relationship) == _Property) {
        dvRelationshipSetLastProperty(Relationship, pProperty);
    }
    dvPropertySetRelationship(_Property, dvRelationshipNull);
}

/*----------------------------------------------------------------------------------------
  Add the Key to the head of the list on the Relationship.
----------------------------------------------------------------------------------------*/
void dvRelationshipInsertKey(
    dvRelationship Relationship,
    dvKey _Key)
{
#if defined(DD_DEBUG)
    if(Relationship == dvRelationshipNull) {
        utExit("Non-existent Relationship");
    }
    if(_Key == dvKeyNull) {
        utExit("Non-existent Key");
    }
    if(dvKeyGetRelationship(_Key) != dvRelationshipNull) {
        utExit("Attempting to add Key to Relationship twice");
    }
#endif
    dvKeySetNextRelationshipKey(_Key, dvRelationshipGetFirstKey(Relationship));
    dvRelationshipSetFirstKey(Relationship, _Key);
    if(dvRelationshipGetLastKey(Relationship) == dvKeyNull) {
        dvRelationshipSetLastKey(Relationship, _Key);
    }
    dvKeySetRelationship(_Key, Relationship);
}

/*----------------------------------------------------------------------------------------
  Add the Key to the end of the list on the Relationship.
----------------------------------------------------------------------------------------*/
void dvRelationshipAppendKey(
    dvRelationship Relationship,
    dvKey _Key)
{
#if defined(DD_DEBUG)
    if(Relationship == dvRelationshipNull) {
        utExit("Non-existent Relationship");
    }
    if(_Key == dvKeyNull) {
        utExit("Non-existent Key");
    }
    if(dvKeyGetRelationship(_Key) != dvRelationshipNull) {
        utExit("Attempting to add Key to Relationship twice");
    }
#endif
    if(dvRelationshipGetLastKey(Relationship) != dvKeyNull) {
        dvKeySetNextRelationshipKey(dvRelationshipGetLastKey(Relationship), _Key);
    } else {
        dvRelationshipSetFirstKey(Relationship, _Key);
    }
    dvRelationshipSetLastKey(Relationship, _Key);
    dvKeySetNextRelationshipKey(_Key, dvKeyNull);
    dvKeySetRelationship(_Key, Relationship);
}

/*----------------------------------------------------------------------------------------
  Insert the Key to the Relationship after the previous Key.
----------------------------------------------------------------------------------------*/
void dvRelationshipInsertAfterKey(
    dvRelationship Relationship,
    dvKey prevKey,
    dvKey _Key)
{
    dvKey nextKey = dvKeyGetNextRelationshipKey(prevKey);

#if defined(DD_DEBUG)
    if(Relationship == dvRelationshipNull) {
        utExit("Non-existent Relationship");
    }
    if(_Key == dvKeyNull) {
        utExit("Non-existent Key");
    }
    if(dvKeyGetRelationship(_Key) != dvRelationshipNull) {
        utExit("Attempting to add Key to Relationship twice");
    }
#endif
    dvKeySetNextRelationshipKey(_Key, nextKey);
    dvKeySetNextRelationshipKey(prevKey, _Key);
    if(dvRelationshipGetLastKey(Relationship) == prevKey) {
        dvRelationshipSetLastKey(Relationship, _Key);
    }
    dvKeySetRelationship(_Key, Relationship);
}

/*----------------------------------------------------------------------------------------
 Remove the Key from the Relationship.
----------------------------------------------------------------------------------------*/
void dvRelationshipRemoveKey(
    dvRelationship Relationship,
    dvKey _Key)
{
    dvKey pKey, nKey;

#if defined(DD_DEBUG)
    if(_Key == dvKeyNull) {
        utExit("Non-existent Key");
    }
    if(dvKeyGetRelationship(_Key) != dvRelationshipNull && dvKeyGetRelationship(_Key) != Relationship) {
        utExit("Delete Key from non-owning Relationship");
    }
#endif
    pKey = dvKeyNull;
    for(nKey = dvRelationshipGetFirstKey(Relationship); nKey != dvKeyNull && nKey != _Key;
            nKey = dvKeyGetNextRelationshipKey(nKey)) {
        pKey = nKey;
    }
    if(pKey != dvKeyNull) {
        dvKeySetNextRelationshipKey(pKey, dvKeyGetNextRelationshipKey(_Key));
    } else {
        dvRelationshipSetFirstKey(Relationship, dvKeyGetNextRelationshipKey(_Key));
    }
    dvKeySetNextRelationshipKey(_Key, dvKeyNull);
    if(dvRelationshipGetLastKey(Relationship) == _Key) {
        dvRelationshipSetLastKey(Relationship, pKey);
    }
    dvKeySetRelationship(_Key, dvRelationshipNull);
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void dvShowRelationship(
    dvRelationship Relationship)
{
    utDatabaseShowObject("dv", "Relationship", dvRelationship2Index(Relationship));
}
#endif

/*----------------------------------------------------------------------------------------
  Destroy Key including everything in it. Remove from parents.
----------------------------------------------------------------------------------------*/
void dvKeyDestroy(
    dvKey Key)
{
    dvKeyproperty Keyproperty_;
    dvRelationship owningRelationship = dvKeyGetRelationship(Key);

    if(dvKeyDestructorCallback != NULL) {
        dvKeyDestructorCallback(Key);
    }
    dvSafeForeachKeyKeyproperty(Key, Keyproperty_) {
        dvKeypropertyDestroy(Keyproperty_);
    } dvEndSafeKeyKeyproperty;
    if(owningRelationship != dvRelationshipNull) {
        dvRelationshipRemoveKey(owningRelationship, Key);
#if defined(DD_DEBUG)
    } else {
        utExit("Key without owning Relationship");
#endif
    }
    dvKeyFree(Key);
}

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocKey(void)
{
    dvKey Key = dvKeyAlloc();

    return dvKey2Index(Key);
}

/*----------------------------------------------------------------------------------------
  Destructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static void destroyKey(
    uint64 objectIndex)
{
    dvKeyDestroy(dvIndex2Key((uint32)objectIndex));
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Key.
----------------------------------------------------------------------------------------*/
static void allocKeys(void)
{
    dvSetAllocatedKey(2);
    dvSetUsedKey(1);
    dvSetFirstFreeKey(dvKeyNull);
    dvKeys.LineNum = utNewAInitFirst(uint32, (dvAllocatedKey()));
    dvKeys.Relationship = utNewAInitFirst(dvRelationship, (dvAllocatedKey()));
    dvKeys.NextRelationshipKey = utNewAInitFirst(dvKey, (dvAllocatedKey()));
    dvKeys.FirstKeyproperty = utNewAInitFirst(dvKeyproperty, (dvAllocatedKey()));
    dvKeys.LastKeyproperty = utNewAInitFirst(dvKeyproperty, (dvAllocatedKey()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Key.
----------------------------------------------------------------------------------------*/
static void reallocKeys(
    uint32 newSize)
{
    utResizeArray(dvKeys.LineNum, (newSize));
    utResizeArray(dvKeys.Relationship, (newSize));
    utResizeArray(dvKeys.NextRelationshipKey, (newSize));
    utResizeArray(dvKeys.FirstKeyproperty, (newSize));
    utResizeArray(dvKeys.LastKeyproperty, (newSize));
    dvSetAllocatedKey(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Keys.
----------------------------------------------------------------------------------------*/
void dvKeyAllocMore(void)
{
    reallocKeys((uint32)(dvAllocatedKey() + (dvAllocatedKey() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Key.
----------------------------------------------------------------------------------------*/
void dvKeyCopyProps(
    dvKey oldKey,
    dvKey newKey)
{
    dvKeySetLineNum(newKey, dvKeyGetLineNum(oldKey));
}

/*----------------------------------------------------------------------------------------
  Add the Keyproperty to the head of the list on the Key.
----------------------------------------------------------------------------------------*/
void dvKeyInsertKeyproperty(
    dvKey Key,
    dvKeyproperty _Keyproperty)
{
#if defined(DD_DEBUG)
    if(Key == dvKeyNull) {
        utExit("Non-existent Key");
    }
    if(_Keyproperty == dvKeypropertyNull) {
        utExit("Non-existent Keyproperty");
    }
    if(dvKeypropertyGetKey(_Keyproperty) != dvKeyNull) {
        utExit("Attempting to add Keyproperty to Key twice");
    }
#endif
    dvKeypropertySetNextKeyKeyproperty(_Keyproperty, dvKeyGetFirstKeyproperty(Key));
    dvKeySetFirstKeyproperty(Key, _Keyproperty);
    if(dvKeyGetLastKeyproperty(Key) == dvKeypropertyNull) {
        dvKeySetLastKeyproperty(Key, _Keyproperty);
    }
    dvKeypropertySetKey(_Keyproperty, Key);
}

/*----------------------------------------------------------------------------------------
  Add the Keyproperty to the end of the list on the Key.
----------------------------------------------------------------------------------------*/
void dvKeyAppendKeyproperty(
    dvKey Key,
    dvKeyproperty _Keyproperty)
{
#if defined(DD_DEBUG)
    if(Key == dvKeyNull) {
        utExit("Non-existent Key");
    }
    if(_Keyproperty == dvKeypropertyNull) {
        utExit("Non-existent Keyproperty");
    }
    if(dvKeypropertyGetKey(_Keyproperty) != dvKeyNull) {
        utExit("Attempting to add Keyproperty to Key twice");
    }
#endif
    if(dvKeyGetLastKeyproperty(Key) != dvKeypropertyNull) {
        dvKeypropertySetNextKeyKeyproperty(dvKeyGetLastKeyproperty(Key), _Keyproperty);
    } else {
        dvKeySetFirstKeyproperty(Key, _Keyproperty);
    }
    dvKeySetLastKeyproperty(Key, _Keyproperty);
    dvKeypropertySetNextKeyKeyproperty(_Keyproperty, dvKeypropertyNull);
    dvKeypropertySetKey(_Keyproperty, Key);
}

/*----------------------------------------------------------------------------------------
  Insert the Keyproperty to the Key after the previous Keyproperty.
----------------------------------------------------------------------------------------*/
void dvKeyInsertAfterKeyproperty(
    dvKey Key,
    dvKeyproperty prevKeyproperty,
    dvKeyproperty _Keyproperty)
{
    dvKeyproperty nextKeyproperty = dvKeypropertyGetNextKeyKeyproperty(prevKeyproperty);

#if defined(DD_DEBUG)
    if(Key == dvKeyNull) {
        utExit("Non-existent Key");
    }
    if(_Keyproperty == dvKeypropertyNull) {
        utExit("Non-existent Keyproperty");
    }
    if(dvKeypropertyGetKey(_Keyproperty) != dvKeyNull) {
        utExit("Attempting to add Keyproperty to Key twice");
    }
#endif
    dvKeypropertySetNextKeyKeyproperty(_Keyproperty, nextKeyproperty);
    dvKeypropertySetNextKeyKeyproperty(prevKeyproperty, _Keyproperty);
    if(dvKeyGetLastKeyproperty(Key) == prevKeyproperty) {
        dvKeySetLastKeyproperty(Key, _Keyproperty);
    }
    dvKeypropertySetKey(_Keyproperty, Key);
}

/*----------------------------------------------------------------------------------------
 Remove the Keyproperty from the Key.
----------------------------------------------------------------------------------------*/
void dvKeyRemoveKeyproperty(
    dvKey Key,
    dvKeyproperty _Keyproperty)
{
    dvKeyproperty pKeyproperty, nKeyproperty;

#if defined(DD_DEBUG)
    if(_Keyproperty == dvKeypropertyNull) {
        utExit("Non-existent Keyproperty");
    }
    if(dvKeypropertyGetKey(_Keyproperty) != dvKeyNull && dvKeypropertyGetKey(_Keyproperty) != Key) {
        utExit("Delete Keyproperty from non-owning Key");
    }
#endif
    pKeyproperty = dvKeypropertyNull;
    for(nKeyproperty = dvKeyGetFirstKeyproperty(Key); nKeyproperty != dvKeypropertyNull && nKeyproperty != _Keyproperty;
            nKeyproperty = dvKeypropertyGetNextKeyKeyproperty(nKeyproperty)) {
        pKeyproperty = nKeyproperty;
    }
    if(pKeyproperty != dvKeypropertyNull) {
        dvKeypropertySetNextKeyKeyproperty(pKeyproperty, dvKeypropertyGetNextKeyKeyproperty(_Keyproperty));
    } else {
        dvKeySetFirstKeyproperty(Key, dvKeypropertyGetNextKeyKeyproperty(_Keyproperty));
    }
    dvKeypropertySetNextKeyKeyproperty(_Keyproperty, dvKeypropertyNull);
    if(dvKeyGetLastKeyproperty(Key) == _Keyproperty) {
        dvKeySetLastKeyproperty(Key, pKeyproperty);
    }
    dvKeypropertySetKey(_Keyproperty, dvKeyNull);
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void dvShowKey(
    dvKey Key)
{
    utDatabaseShowObject("dv", "Key", dvKey2Index(Key));
}
#endif

/*----------------------------------------------------------------------------------------
  Destroy Keyproperty including everything in it. Remove from parents.
----------------------------------------------------------------------------------------*/
void dvKeypropertyDestroy(
    dvKeyproperty Keyproperty)
{
    dvKey owningKey = dvKeypropertyGetKey(Keyproperty);
    dvProperty owningProperty = dvKeypropertyGetProperty(Keyproperty);

    if(dvKeypropertyDestructorCallback != NULL) {
        dvKeypropertyDestructorCallback(Keyproperty);
    }
    if(owningKey != dvKeyNull) {
        dvKeyRemoveKeyproperty(owningKey, Keyproperty);
#if defined(DD_DEBUG)
    } else {
        utExit("Keyproperty without owning Key");
#endif
    }
    if(owningProperty != dvPropertyNull) {
        dvPropertyRemoveKeyproperty(owningProperty, Keyproperty);
#if defined(DD_DEBUG)
    } else {
        utExit("Keyproperty without owning Property");
#endif
    }
    dvKeypropertyFree(Keyproperty);
}

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocKeyproperty(void)
{
    dvKeyproperty Keyproperty = dvKeypropertyAlloc();

    return dvKeyproperty2Index(Keyproperty);
}

/*----------------------------------------------------------------------------------------
  Destructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static void destroyKeyproperty(
    uint64 objectIndex)
{
    dvKeypropertyDestroy(dvIndex2Keyproperty((uint32)objectIndex));
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Keyproperty.
----------------------------------------------------------------------------------------*/
static void allocKeypropertys(void)
{
    dvSetAllocatedKeyproperty(2);
    dvSetUsedKeyproperty(1);
    dvSetFirstFreeKeyproperty(dvKeypropertyNull);
    dvKeypropertys.PropertySym = utNewAInitFirst(utSym, (dvAllocatedKeyproperty()));
    dvKeypropertys.Property = utNewAInitFirst(dvProperty, (dvAllocatedKeyproperty()));
    dvKeypropertys.NextPropertyKeyproperty = utNewAInitFirst(dvKeyproperty, (dvAllocatedKeyproperty()));
    dvKeypropertys.Key = utNewAInitFirst(dvKey, (dvAllocatedKeyproperty()));
    dvKeypropertys.NextKeyKeyproperty = utNewAInitFirst(dvKeyproperty, (dvAllocatedKeyproperty()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Keyproperty.
----------------------------------------------------------------------------------------*/
static void reallocKeypropertys(
    uint32 newSize)
{
    utResizeArray(dvKeypropertys.PropertySym, (newSize));
    utResizeArray(dvKeypropertys.Property, (newSize));
    utResizeArray(dvKeypropertys.NextPropertyKeyproperty, (newSize));
    utResizeArray(dvKeypropertys.Key, (newSize));
    utResizeArray(dvKeypropertys.NextKeyKeyproperty, (newSize));
    dvSetAllocatedKeyproperty(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Keypropertys.
----------------------------------------------------------------------------------------*/
void dvKeypropertyAllocMore(void)
{
    reallocKeypropertys((uint32)(dvAllocatedKeyproperty() + (dvAllocatedKeyproperty() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Keyproperty.
----------------------------------------------------------------------------------------*/
void dvKeypropertyCopyProps(
    dvKeyproperty oldKeyproperty,
    dvKeyproperty newKeyproperty)
{
    dvKeypropertySetPropertySym(newKeyproperty, dvKeypropertyGetPropertySym(oldKeyproperty));
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void dvShowKeyproperty(
    dvKeyproperty Keyproperty)
{
    utDatabaseShowObject("dv", "Keyproperty", dvKeyproperty2Index(Keyproperty));
}
#endif

/*----------------------------------------------------------------------------------------
  Destroy Union including everything in it. Remove from parents.
----------------------------------------------------------------------------------------*/
void dvUnionDestroy(
    dvUnion Union)
{
    dvProperty Property_;
    dvClass owningClass = dvUnionGetClass(Union);

    if(dvUnionDestructorCallback != NULL) {
        dvUnionDestructorCallback(Union);
    }
    dvSafeForeachUnionProperty(Union, Property_) {
        dvPropertySetUnion(Property_, dvUnionNull);
    } dvEndSafeUnionProperty;
    if(owningClass != dvClassNull) {
        dvClassRemoveUnion(owningClass, Union);
#if defined(DD_DEBUG)
    } else {
        utExit("Union without owning Class");
#endif
    }
    dvUnionFree(Union);
}

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocUnion(void)
{
    dvUnion Union = dvUnionAlloc();

    return dvUnion2Index(Union);
}

/*----------------------------------------------------------------------------------------
  Destructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static void destroyUnion(
    uint64 objectIndex)
{
    dvUnionDestroy(dvIndex2Union((uint32)objectIndex));
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Union.
----------------------------------------------------------------------------------------*/
static void allocUnions(void)
{
    dvSetAllocatedUnion(2);
    dvSetUsedUnion(1);
    dvSetFirstFreeUnion(dvUnionNull);
    dvUnions.PropertySym = utNewAInitFirst(utSym, (dvAllocatedUnion()));
    dvUnions.TypeProperty = utNewAInitFirst(dvProperty, (dvAllocatedUnion()));
    dvUnions.Line = utNewAInitFirst(uint32, (dvAllocatedUnion()));
    dvUnions.Number = utNewAInitFirst(uint16, (dvAllocatedUnion()));
    dvUnions.FieldNumber = utNewAInitFirst(uint32, (dvAllocatedUnion()));
    dvUnions.NumCases = utNewAInitFirst(uint16, (dvAllocatedUnion()));
    dvUnions.Class = utNewAInitFirst(dvClass, (dvAllocatedUnion()));
    dvUnions.NextClassUnion = utNewAInitFirst(dvUnion, (dvAllocatedUnion()));
    dvUnions.FirstProperty = utNewAInitFirst(dvProperty, (dvAllocatedUnion()));
    dvUnions.LastProperty = utNewAInitFirst(dvProperty, (dvAllocatedUnion()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Union.
----------------------------------------------------------------------------------------*/
static void reallocUnions(
    uint32 newSize)
{
    utResizeArray(dvUnions.PropertySym, (newSize));
    utResizeArray(dvUnions.TypeProperty, (newSize));
    utResizeArray(dvUnions.Line, (newSize));
    utResizeArray(dvUnions.Number, (newSize));
    utResizeArray(dvUnions.FieldNumber, (newSize));
    utResizeArray(dvUnions.NumCases, (newSize));
    utResizeArray(dvUnions.Class, (newSize));
    utResizeArray(dvUnions.NextClassUnion, (newSize));
    utResizeArray(dvUnions.FirstProperty, (newSize));
    utResizeArray(dvUnions.LastProperty, (newSize));
    dvSetAllocatedUnion(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Unions.
----------------------------------------------------------------------------------------*/
void dvUnionAllocMore(void)
{
    reallocUnions((uint32)(dvAllocatedUnion() + (dvAllocatedUnion() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Union.
----------------------------------------------------------------------------------------*/
void dvUnionCopyProps(
    dvUnion oldUnion,
    dvUnion newUnion)
{
    dvUnionSetPropertySym(newUnion, dvUnionGetPropertySym(oldUnion));
    dvUnionSetLine(newUnion, dvUnionGetLine(oldUnion));
    dvUnionSetNumber(newUnion, dvUnionGetNumber(oldUnion));
    dvUnionSetFieldNumber(newUnion, dvUnionGetFieldNumber(oldUnion));
    dvUnionSetNumCases(newUnion, dvUnionGetNumCases(oldUnion));
}

/*----------------------------------------------------------------------------------------
  Add the Property to the head of the list on the Union.
----------------------------------------------------------------------------------------*/
void dvUnionInsertProperty(
    dvUnion Union,
    dvProperty _Property)
{
#if defined(DD_DEBUG)
    if(Union == dvUnionNull) {
        utExit("Non-existent Union");
    }
    if(_Property == dvPropertyNull) {
        utExit("Non-existent Property");
    }
    if(dvPropertyGetUnion(_Property) != dvUnionNull) {
        utExit("Attempting to add Property to Union twice");
    }
#endif
    dvPropertySetNextUnionProperty(_Property, dvUnionGetFirstProperty(Union));
    dvUnionSetFirstProperty(Union, _Property);
    if(dvUnionGetLastProperty(Union) == dvPropertyNull) {
        dvUnionSetLastProperty(Union, _Property);
    }
    dvPropertySetUnion(_Property, Union);
}

/*----------------------------------------------------------------------------------------
  Add the Property to the end of the list on the Union.
----------------------------------------------------------------------------------------*/
void dvUnionAppendProperty(
    dvUnion Union,
    dvProperty _Property)
{
#if defined(DD_DEBUG)
    if(Union == dvUnionNull) {
        utExit("Non-existent Union");
    }
    if(_Property == dvPropertyNull) {
        utExit("Non-existent Property");
    }
    if(dvPropertyGetUnion(_Property) != dvUnionNull) {
        utExit("Attempting to add Property to Union twice");
    }
#endif
    if(dvUnionGetLastProperty(Union) != dvPropertyNull) {
        dvPropertySetNextUnionProperty(dvUnionGetLastProperty(Union), _Property);
    } else {
        dvUnionSetFirstProperty(Union, _Property);
    }
    dvUnionSetLastProperty(Union, _Property);
    dvPropertySetNextUnionProperty(_Property, dvPropertyNull);
    dvPropertySetUnion(_Property, Union);
}

/*----------------------------------------------------------------------------------------
  Insert the Property to the Union after the previous Property.
----------------------------------------------------------------------------------------*/
void dvUnionInsertAfterProperty(
    dvUnion Union,
    dvProperty prevProperty,
    dvProperty _Property)
{
    dvProperty nextProperty = dvPropertyGetNextUnionProperty(prevProperty);

#if defined(DD_DEBUG)
    if(Union == dvUnionNull) {
        utExit("Non-existent Union");
    }
    if(_Property == dvPropertyNull) {
        utExit("Non-existent Property");
    }
    if(dvPropertyGetUnion(_Property) != dvUnionNull) {
        utExit("Attempting to add Property to Union twice");
    }
#endif
    dvPropertySetNextUnionProperty(_Property, nextProperty);
    dvPropertySetNextUnionProperty(prevProperty, _Property);
    if(dvUnionGetLastProperty(Union) == prevProperty) {
        dvUnionSetLastProperty(Union, _Property);
    }
    dvPropertySetUnion(_Property, Union);
}

/*----------------------------------------------------------------------------------------
 Remove the Property from the Union.
----------------------------------------------------------------------------------------*/
void dvUnionRemoveProperty(
    dvUnion Union,
    dvProperty _Property)
{
    dvProperty pProperty, nProperty;

#if defined(DD_DEBUG)
    if(_Property == dvPropertyNull) {
        utExit("Non-existent Property");
    }
    if(dvPropertyGetUnion(_Property) != dvUnionNull && dvPropertyGetUnion(_Property) != Union) {
        utExit("Delete Property from non-owning Union");
    }
#endif
    pProperty = dvPropertyNull;
    for(nProperty = dvUnionGetFirstProperty(Union); nProperty != dvPropertyNull && nProperty != _Property;
            nProperty = dvPropertyGetNextUnionProperty(nProperty)) {
        pProperty = nProperty;
    }
    if(pProperty != dvPropertyNull) {
        dvPropertySetNextUnionProperty(pProperty, dvPropertyGetNextUnionProperty(_Property));
    } else {
        dvUnionSetFirstProperty(Union, dvPropertyGetNextUnionProperty(_Property));
    }
    dvPropertySetNextUnionProperty(_Property, dvPropertyNull);
    if(dvUnionGetLastProperty(Union) == _Property) {
        dvUnionSetLastProperty(Union, pProperty);
    }
    dvPropertySetUnion(_Property, dvUnionNull);
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void dvShowUnion(
    dvUnion Union)
{
    utDatabaseShowObject("dv", "Union", dvUnion2Index(Union));
}
#endif

/*----------------------------------------------------------------------------------------
  Destroy Case including everything in it. Remove from parents.
----------------------------------------------------------------------------------------*/
void dvCaseDestroy(
    dvCase Case)
{
    dvEntry owningEntry = dvCaseGetEntry(Case);
    dvProperty owningProperty = dvCaseGetProperty(Case);

    if(dvCaseDestructorCallback != NULL) {
        dvCaseDestructorCallback(Case);
    }
    if(owningEntry != dvEntryNull) {
        dvEntryRemoveCase(owningEntry, Case);
#if defined(DD_DEBUG)
    } else {
        utExit("Case without owning Entry");
#endif
    }
    if(owningProperty != dvPropertyNull) {
        dvPropertyRemoveCase(owningProperty, Case);
#if defined(DD_DEBUG)
    } else {
        utExit("Case without owning Property");
#endif
    }
    dvCaseFree(Case);
}

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocCase(void)
{
    dvCase Case = dvCaseAlloc();

    return dvCase2Index(Case);
}

/*----------------------------------------------------------------------------------------
  Destructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static void destroyCase(
    uint64 objectIndex)
{
    dvCaseDestroy(dvIndex2Case((uint32)objectIndex));
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Case.
----------------------------------------------------------------------------------------*/
static void allocCases(void)
{
    dvSetAllocatedCase(2);
    dvSetUsedCase(1);
    dvSetFirstFreeCase(dvCaseNull);
    dvCases.EntrySym = utNewAInitFirst(utSym, (dvAllocatedCase()));
    dvCases.Entry = utNewAInitFirst(dvEntry, (dvAllocatedCase()));
    dvCases.NextEntryCase = utNewAInitFirst(dvCase, (dvAllocatedCase()));
    dvCases.Property = utNewAInitFirst(dvProperty, (dvAllocatedCase()));
    dvCases.NextPropertyCase = utNewAInitFirst(dvCase, (dvAllocatedCase()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Case.
----------------------------------------------------------------------------------------*/
static void reallocCases(
    uint32 newSize)
{
    utResizeArray(dvCases.EntrySym, (newSize));
    utResizeArray(dvCases.Entry, (newSize));
    utResizeArray(dvCases.NextEntryCase, (newSize));
    utResizeArray(dvCases.Property, (newSize));
    utResizeArray(dvCases.NextPropertyCase, (newSize));
    dvSetAllocatedCase(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Cases.
----------------------------------------------------------------------------------------*/
void dvCaseAllocMore(void)
{
    reallocCases((uint32)(dvAllocatedCase() + (dvAllocatedCase() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Case.
----------------------------------------------------------------------------------------*/
void dvCaseCopyProps(
    dvCase oldCase,
    dvCase newCase)
{
    dvCaseSetEntrySym(newCase, dvCaseGetEntrySym(oldCase));
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void dvShowCase(
    dvCase Case)
{
    utDatabaseShowObject("dv", "Case", dvCase2Index(Case));
}
#endif

/*----------------------------------------------------------------------------------------
  Destroy Cache including everything in it. Remove from parents.
----------------------------------------------------------------------------------------*/
void dvCacheDestroy(
    dvCache Cache)
{
    dvProperty Property_;
    dvPropident Propident_;
    dvClass owningClass = dvCacheGetClass(Cache);

    if(dvCacheDestructorCallback != NULL) {
        dvCacheDestructorCallback(Cache);
    }
    dvSafeForeachCacheProperty(Cache, Property_) {
        dvPropertySetCache(Property_, dvCacheNull);
    } dvEndSafeCacheProperty;
    dvSafeForeachCachePropident(Cache, Propident_) {
        dvPropidentDestroy(Propident_);
    } dvEndSafeCachePropident;
    if(owningClass != dvClassNull) {
        dvClassRemoveCache(owningClass, Cache);
#if defined(DD_DEBUG)
    } else {
        utExit("Cache without owning Class");
#endif
    }
    dvCacheFree(Cache);
}

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocCache(void)
{
    dvCache Cache = dvCacheAlloc();

    return dvCache2Index(Cache);
}

/*----------------------------------------------------------------------------------------
  Destructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static void destroyCache(
    uint64 objectIndex)
{
    dvCacheDestroy(dvIndex2Cache((uint32)objectIndex));
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Cache.
----------------------------------------------------------------------------------------*/
static void allocCaches(void)
{
    dvSetAllocatedCache(2);
    dvSetUsedCache(1);
    dvSetFirstFreeCache(dvCacheNull);
    dvCaches.Number = utNewAInitFirst(uint16, (dvAllocatedCache()));
    dvCaches.Line = utNewAInitFirst(uint32, (dvAllocatedCache()));
    dvCaches.Class = utNewAInitFirst(dvClass, (dvAllocatedCache()));
    dvCaches.NextClassCache = utNewAInitFirst(dvCache, (dvAllocatedCache()));
    dvCaches.FirstProperty = utNewAInitFirst(dvProperty, (dvAllocatedCache()));
    dvCaches.LastProperty = utNewAInitFirst(dvProperty, (dvAllocatedCache()));
    dvCaches.FirstPropident = utNewAInitFirst(dvPropident, (dvAllocatedCache()));
    dvCaches.LastPropident = utNewAInitFirst(dvPropident, (dvAllocatedCache()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Cache.
----------------------------------------------------------------------------------------*/
static void reallocCaches(
    uint32 newSize)
{
    utResizeArray(dvCaches.Number, (newSize));
    utResizeArray(dvCaches.Line, (newSize));
    utResizeArray(dvCaches.Class, (newSize));
    utResizeArray(dvCaches.NextClassCache, (newSize));
    utResizeArray(dvCaches.FirstProperty, (newSize));
    utResizeArray(dvCaches.LastProperty, (newSize));
    utResizeArray(dvCaches.FirstPropident, (newSize));
    utResizeArray(dvCaches.LastPropident, (newSize));
    dvSetAllocatedCache(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Caches.
----------------------------------------------------------------------------------------*/
void dvCacheAllocMore(void)
{
    reallocCaches((uint32)(dvAllocatedCache() + (dvAllocatedCache() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Cache.
----------------------------------------------------------------------------------------*/
void dvCacheCopyProps(
    dvCache oldCache,
    dvCache newCache)
{
    dvCacheSetNumber(newCache, dvCacheGetNumber(oldCache));
    dvCacheSetLine(newCache, dvCacheGetLine(oldCache));
}

/*----------------------------------------------------------------------------------------
  Add the Property to the head of the list on the Cache.
----------------------------------------------------------------------------------------*/
void dvCacheInsertProperty(
    dvCache Cache,
    dvProperty _Property)
{
#if defined(DD_DEBUG)
    if(Cache == dvCacheNull) {
        utExit("Non-existent Cache");
    }
    if(_Property == dvPropertyNull) {
        utExit("Non-existent Property");
    }
    if(dvPropertyGetCache(_Property) != dvCacheNull) {
        utExit("Attempting to add Property to Cache twice");
    }
#endif
    dvPropertySetNextCacheProperty(_Property, dvCacheGetFirstProperty(Cache));
    dvCacheSetFirstProperty(Cache, _Property);
    if(dvCacheGetLastProperty(Cache) == dvPropertyNull) {
        dvCacheSetLastProperty(Cache, _Property);
    }
    dvPropertySetCache(_Property, Cache);
}

/*----------------------------------------------------------------------------------------
  Add the Property to the end of the list on the Cache.
----------------------------------------------------------------------------------------*/
void dvCacheAppendProperty(
    dvCache Cache,
    dvProperty _Property)
{
#if defined(DD_DEBUG)
    if(Cache == dvCacheNull) {
        utExit("Non-existent Cache");
    }
    if(_Property == dvPropertyNull) {
        utExit("Non-existent Property");
    }
    if(dvPropertyGetCache(_Property) != dvCacheNull) {
        utExit("Attempting to add Property to Cache twice");
    }
#endif
    if(dvCacheGetLastProperty(Cache) != dvPropertyNull) {
        dvPropertySetNextCacheProperty(dvCacheGetLastProperty(Cache), _Property);
    } else {
        dvCacheSetFirstProperty(Cache, _Property);
    }
    dvCacheSetLastProperty(Cache, _Property);
    dvPropertySetNextCacheProperty(_Property, dvPropertyNull);
    dvPropertySetCache(_Property, Cache);
}

/*----------------------------------------------------------------------------------------
  Insert the Property to the Cache after the previous Property.
----------------------------------------------------------------------------------------*/
void dvCacheInsertAfterProperty(
    dvCache Cache,
    dvProperty prevProperty,
    dvProperty _Property)
{
    dvProperty nextProperty = dvPropertyGetNextCacheProperty(prevProperty);

#if defined(DD_DEBUG)
    if(Cache == dvCacheNull) {
        utExit("Non-existent Cache");
    }
    if(_Property == dvPropertyNull) {
        utExit("Non-existent Property");
    }
    if(dvPropertyGetCache(_Property) != dvCacheNull) {
        utExit("Attempting to add Property to Cache twice");
    }
#endif
    dvPropertySetNextCacheProperty(_Property, nextProperty);
    dvPropertySetNextCacheProperty(prevProperty, _Property);
    if(dvCacheGetLastProperty(Cache) == prevProperty) {
        dvCacheSetLastProperty(Cache, _Property);
    }
    dvPropertySetCache(_Property, Cache);
}

/*----------------------------------------------------------------------------------------
 Remove the Property from the Cache.
----------------------------------------------------------------------------------------*/
void dvCacheRemoveProperty(
    dvCache Cache,
    dvProperty _Property)
{
    dvProperty pProperty, nProperty;

#if defined(DD_DEBUG)
    if(_Property == dvPropertyNull) {
        utExit("Non-existent Property");
    }
    if(dvPropertyGetCache(_Property) != dvCacheNull && dvPropertyGetCache(_Property) != Cache) {
        utExit("Delete Property from non-owning Cache");
    }
#endif
    pProperty = dvPropertyNull;
    for(nProperty = dvCacheGetFirstProperty(Cache); nProperty != dvPropertyNull && nProperty != _Property;
            nProperty = dvPropertyGetNextCacheProperty(nProperty)) {
        pProperty = nProperty;
    }
    if(pProperty != dvPropertyNull) {
        dvPropertySetNextCacheProperty(pProperty, dvPropertyGetNextCacheProperty(_Property));
    } else {
        dvCacheSetFirstProperty(Cache, dvPropertyGetNextCacheProperty(_Property));
    }
    dvPropertySetNextCacheProperty(_Property, dvPropertyNull);
    if(dvCacheGetLastProperty(Cache) == _Property) {
        dvCacheSetLastProperty(Cache, pProperty);
    }
    dvPropertySetCache(_Property, dvCacheNull);
}

/*----------------------------------------------------------------------------------------
  Add the Propident to the head of the list on the Cache.
----------------------------------------------------------------------------------------*/
void dvCacheInsertPropident(
    dvCache Cache,
    dvPropident _Propident)
{
#if defined(DD_DEBUG)
    if(Cache == dvCacheNull) {
        utExit("Non-existent Cache");
    }
    if(_Propident == dvPropidentNull) {
        utExit("Non-existent Propident");
    }
    if(dvPropidentGetCache(_Propident) != dvCacheNull) {
        utExit("Attempting to add Propident to Cache twice");
    }
#endif
    dvPropidentSetNextCachePropident(_Propident, dvCacheGetFirstPropident(Cache));
    dvCacheSetFirstPropident(Cache, _Propident);
    if(dvCacheGetLastPropident(Cache) == dvPropidentNull) {
        dvCacheSetLastPropident(Cache, _Propident);
    }
    dvPropidentSetCache(_Propident, Cache);
}

/*----------------------------------------------------------------------------------------
  Add the Propident to the end of the list on the Cache.
----------------------------------------------------------------------------------------*/
void dvCacheAppendPropident(
    dvCache Cache,
    dvPropident _Propident)
{
#if defined(DD_DEBUG)
    if(Cache == dvCacheNull) {
        utExit("Non-existent Cache");
    }
    if(_Propident == dvPropidentNull) {
        utExit("Non-existent Propident");
    }
    if(dvPropidentGetCache(_Propident) != dvCacheNull) {
        utExit("Attempting to add Propident to Cache twice");
    }
#endif
    if(dvCacheGetLastPropident(Cache) != dvPropidentNull) {
        dvPropidentSetNextCachePropident(dvCacheGetLastPropident(Cache), _Propident);
    } else {
        dvCacheSetFirstPropident(Cache, _Propident);
    }
    dvCacheSetLastPropident(Cache, _Propident);
    dvPropidentSetNextCachePropident(_Propident, dvPropidentNull);
    dvPropidentSetCache(_Propident, Cache);
}

/*----------------------------------------------------------------------------------------
  Insert the Propident to the Cache after the previous Propident.
----------------------------------------------------------------------------------------*/
void dvCacheInsertAfterPropident(
    dvCache Cache,
    dvPropident prevPropident,
    dvPropident _Propident)
{
    dvPropident nextPropident = dvPropidentGetNextCachePropident(prevPropident);

#if defined(DD_DEBUG)
    if(Cache == dvCacheNull) {
        utExit("Non-existent Cache");
    }
    if(_Propident == dvPropidentNull) {
        utExit("Non-existent Propident");
    }
    if(dvPropidentGetCache(_Propident) != dvCacheNull) {
        utExit("Attempting to add Propident to Cache twice");
    }
#endif
    dvPropidentSetNextCachePropident(_Propident, nextPropident);
    dvPropidentSetNextCachePropident(prevPropident, _Propident);
    if(dvCacheGetLastPropident(Cache) == prevPropident) {
        dvCacheSetLastPropident(Cache, _Propident);
    }
    dvPropidentSetCache(_Propident, Cache);
}

/*----------------------------------------------------------------------------------------
 Remove the Propident from the Cache.
----------------------------------------------------------------------------------------*/
void dvCacheRemovePropident(
    dvCache Cache,
    dvPropident _Propident)
{
    dvPropident pPropident, nPropident;

#if defined(DD_DEBUG)
    if(_Propident == dvPropidentNull) {
        utExit("Non-existent Propident");
    }
    if(dvPropidentGetCache(_Propident) != dvCacheNull && dvPropidentGetCache(_Propident) != Cache) {
        utExit("Delete Propident from non-owning Cache");
    }
#endif
    pPropident = dvPropidentNull;
    for(nPropident = dvCacheGetFirstPropident(Cache); nPropident != dvPropidentNull && nPropident != _Propident;
            nPropident = dvPropidentGetNextCachePropident(nPropident)) {
        pPropident = nPropident;
    }
    if(pPropident != dvPropidentNull) {
        dvPropidentSetNextCachePropident(pPropident, dvPropidentGetNextCachePropident(_Propident));
    } else {
        dvCacheSetFirstPropident(Cache, dvPropidentGetNextCachePropident(_Propident));
    }
    dvPropidentSetNextCachePropident(_Propident, dvPropidentNull);
    if(dvCacheGetLastPropident(Cache) == _Propident) {
        dvCacheSetLastPropident(Cache, pPropident);
    }
    dvPropidentSetCache(_Propident, dvCacheNull);
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void dvShowCache(
    dvCache Cache)
{
    utDatabaseShowObject("dv", "Cache", dvCache2Index(Cache));
}
#endif

/*----------------------------------------------------------------------------------------
  Destroy Propident including everything in it. Remove from parents.
----------------------------------------------------------------------------------------*/
void dvPropidentDestroy(
    dvPropident Propident)
{
    dvCache owningCache = dvPropidentGetCache(Propident);

    if(dvPropidentDestructorCallback != NULL) {
        dvPropidentDestructorCallback(Propident);
    }
    if(owningCache != dvCacheNull) {
        dvCacheRemovePropident(owningCache, Propident);
#if defined(DD_DEBUG)
    } else {
        utExit("Propident without owning Cache");
#endif
    }
    dvPropidentFree(Propident);
}

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocPropident(void)
{
    dvPropident Propident = dvPropidentAlloc();

    return dvPropident2Index(Propident);
}

/*----------------------------------------------------------------------------------------
  Destructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static void destroyPropident(
    uint64 objectIndex)
{
    dvPropidentDestroy(dvIndex2Propident((uint32)objectIndex));
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Propident.
----------------------------------------------------------------------------------------*/
static void allocPropidents(void)
{
    dvSetAllocatedPropident(2);
    dvSetUsedPropident(1);
    dvSetFirstFreePropident(dvPropidentNull);
    dvPropidents.Sym = utNewAInitFirst(utSym, (dvAllocatedPropident()));
    dvPropidents.Cache = utNewAInitFirst(dvCache, (dvAllocatedPropident()));
    dvPropidents.NextCachePropident = utNewAInitFirst(dvPropident, (dvAllocatedPropident()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Propident.
----------------------------------------------------------------------------------------*/
static void reallocPropidents(
    uint32 newSize)
{
    utResizeArray(dvPropidents.Sym, (newSize));
    utResizeArray(dvPropidents.Cache, (newSize));
    utResizeArray(dvPropidents.NextCachePropident, (newSize));
    dvSetAllocatedPropident(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Propidents.
----------------------------------------------------------------------------------------*/
void dvPropidentAllocMore(void)
{
    reallocPropidents((uint32)(dvAllocatedPropident() + (dvAllocatedPropident() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Propident.
----------------------------------------------------------------------------------------*/
void dvPropidentCopyProps(
    dvPropident oldPropident,
    dvPropident newPropident)
{
    dvPropidentSetSym(newPropident, dvPropidentGetSym(oldPropident));
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void dvShowPropident(
    dvPropident Propident)
{
    utDatabaseShowObject("dv", "Propident", dvPropident2Index(Propident));
}
#endif

/*----------------------------------------------------------------------------------------
  Free memory used by the dv database.
----------------------------------------------------------------------------------------*/
void dvDatabaseStop(void)
{
    utFree(dvRoots.FirstModpath);
    utFree(dvRoots.LastModpath);
    utFree(dvRoots.ModpathTableIndex_);
    utFree(dvRoots.NumModpathTable);
    utFree(dvRoots.ModpathTable);
    utFree(dvRoots.NumModpath);
    utFree(dvRoots.FirstModule);
    utFree(dvRoots.LastModule);
    utFree(dvRoots.ModuleTableIndex_);
    utFree(dvRoots.NumModuleTable);
    utFree(dvRoots.ModuleTable);
    utFree(dvRoots.NumModule);
    utFree(dvModpaths.Sym);
    utFree(dvModpaths.Root);
    utFree(dvModpaths.NextRootModpath);
    utFree(dvModpaths.PrevRootModpath);
    utFree(dvModpaths.NextTableRootModpath);
    utFree(dvModules.Sym);
    utFree(dvModules.PrefixSym);
    utFree(dvModules.Persistent);
    utFree(dvModules.UndoRedo);
    utFree(dvModules.HasSparseData);
    utFree(dvModules.NumFields);
    utFree(dvModules.NumClasses);
    utFree(dvModules.NumEnums);
    utFree(dvModules.Elaborated);
    utFree(dvModules.NextRootModule);
    utFree(dvModules.PrevRootModule);
    utFree(dvModules.NextTableRootModule);
    utFree(dvModules.FirstClass);
    utFree(dvModules.LastClass);
    utFree(dvModules.ClassTableIndex_);
    utFree(dvModules.NumClassTable);
    utFree(dvModules.ClassTable);
    utFree(dvModules.NumClass);
    utFree(dvModules.FirstEnum);
    utFree(dvModules.LastEnum);
    utFree(dvModules.EnumTableIndex_);
    utFree(dvModules.NumEnumTable);
    utFree(dvModules.EnumTable);
    utFree(dvModules.NumEnum);
    utFree(dvModules.FirstTypedef);
    utFree(dvModules.LastTypedef);
    utFree(dvModules.TypedefTableIndex_);
    utFree(dvModules.NumTypedefTable);
    utFree(dvModules.TypedefTable);
    utFree(dvModules.NumTypedef);
    utFree(dvModules.FirstSchema);
    utFree(dvModules.LastSchema);
    utFree(dvModules.SchemaTableIndex_);
    utFree(dvModules.NumSchemaTable);
    utFree(dvModules.SchemaTable);
    utFree(dvModules.NumSchema);
    utFree(dvModules.FirstImportLink);
    utFree(dvModules.LastImportLink);
    utFree(dvModules.FirstExportLink);
    utFree(dvModules.LastExportLink);
    utFree(dvLinks.ImportModule);
    utFree(dvLinks.NextModuleImportLink);
    utFree(dvLinks.ExportModule);
    utFree(dvLinks.NextModuleExportLink);
    utFree(dvSchemas.Sym);
    utFree(dvSchemas.Module);
    utFree(dvSchemas.NextModuleSchema);
    utFree(dvSchemas.PrevModuleSchema);
    utFree(dvSchemas.NextTableModuleSchema);
    utFree(dvSchemas.FirstRelationship);
    utFree(dvSchemas.LastRelationship);
    utFree(dvEnums.Sym);
    utFree(dvEnums.PrefixSym);
    utFree(dvEnums.NumEntries);
    utFree(dvEnums.Module);
    utFree(dvEnums.NextModuleEnum);
    utFree(dvEnums.PrevModuleEnum);
    utFree(dvEnums.NextTableModuleEnum);
    utFree(dvEnums.FirstEntry);
    utFree(dvEnums.LastEntry);
    utFree(dvEnums.EntryTableIndex_);
    utFree(dvEnums.NumEntryTable);
    utFree(dvEnums.EntryTable);
    utFree(dvEnums.NumEntry);
    utFree(dvEntrys.Sym);
    utFree(dvEntrys.Value);
    utFree(dvEntrys.Enum);
    utFree(dvEntrys.NextEnumEntry);
    utFree(dvEntrys.PrevEnumEntry);
    utFree(dvEntrys.NextTableEnumEntry);
    utFree(dvEntrys.FirstCase);
    utFree(dvEntrys.LastCase);
    utFree(dvTypedefs.Sym);
    utFree(dvTypedefs.InitializerIndex_);
    utFree(dvTypedefs.NumInitializer);
    utFree(dvTypedefs.Initializer);
    utFree(dvTypedefs.Module);
    utFree(dvTypedefs.NextModuleTypedef);
    utFree(dvTypedefs.PrevModuleTypedef);
    utFree(dvTypedefs.NextTableModuleTypedef);
    utFree(dvClasss.Sym);
    utFree(dvClasss.MemoryStyle);
    utFree(dvClasss.ReferenceSize);
    utFree(dvClasss.GenerateArrayClass);
    utFree(dvClasss.GenerateAttributes);
    utFree(dvClasss.Sparse);
    utFree(dvClasss.NumFields);
    utFree(dvClasss.Number);
    utFree(dvClasss.BaseClassSym);
    utFree(dvClasss.Module);
    utFree(dvClasss.NextModuleClass);
    utFree(dvClasss.PrevModuleClass);
    utFree(dvClasss.NextTableModuleClass);
    utFree(dvClasss.FirstProperty);
    utFree(dvClasss.LastProperty);
    utFree(dvClasss.PropertyTableIndex_);
    utFree(dvClasss.NumPropertyTable);
    utFree(dvClasss.PropertyTable);
    utFree(dvClasss.NumProperty);
    utFree(dvClasss.FreeListProperty);
    utFree(dvClasss.FirstSparsegroup);
    utFree(dvClasss.LastSparsegroup);
    utFree(dvClasss.SparsegroupTableIndex_);
    utFree(dvClasss.NumSparsegroupTable);
    utFree(dvClasss.SparsegroupTable);
    utFree(dvClasss.NumSparsegroup);
    utFree(dvClasss.BaseClass);
    utFree(dvClasss.FirstDerivedClass);
    utFree(dvClasss.NextClassDerivedClass);
    utFree(dvClasss.LastDerivedClass);
    utFree(dvClasss.FirstChildRelationship);
    utFree(dvClasss.LastChildRelationship);
    utFree(dvClasss.FirstParentRelationship);
    utFree(dvClasss.LastParentRelationship);
    utFree(dvClasss.FirstUnion);
    utFree(dvClasss.LastUnion);
    utFree(dvClasss.FirstCache);
    utFree(dvClasss.LastCache);
    utFree(dvPropertys.Sym);
    utFree(dvPropertys.Type);
    utFree(dvPropertys.Array);
    utFree(dvPropertys.Cascade);
    utFree(dvPropertys.Sparse);
    utFree(dvPropertys.View);
    utFree(dvPropertys.Expanded);
    utFree(dvPropertys.FieldNumber);
    utFree(dvPropertys.FirstElementProp);
    utFree(dvPropertys.NumElementsProp);
    utFree(dvPropertys.Hidden);
    utFree(dvPropertys.InitializerIndex_);
    utFree(dvPropertys.NumInitializer);
    utFree(dvPropertys.Initializer);
    utFree(dvPropertys.FixedSize);
    utFree(dvPropertys.IndexIndex_);
    utFree(dvPropertys.NumIndex);
    utFree(dvPropertys.Index);
    utFree(dvPropertys.Line);
    utFree(dvPropertys.Class);
    utFree(dvPropertys.NextClassProperty);
    utFree(dvPropertys.PrevClassProperty);
    utFree(dvPropertys.NextTableClassProperty);
    utFree(dvPropertys.FirstCase);
    utFree(dvPropertys.LastCase);
    utFree(dvPropertys.FirstKeyproperty);
    utFree(dvPropertys.LastKeyproperty);
    utFree(dvPropertys.Sparsegroup);
    utFree(dvPropertys.NextSparsegroupProperty);
    utFree(dvPropertys.Relationship);
    utFree(dvPropertys.NextRelationshipProperty);
    utFree(dvPropertys.Union);
    utFree(dvPropertys.NextUnionProperty);
    utFree(dvPropertys.Cache);
    utFree(dvPropertys.NextCacheProperty);
    utFree(dvPropertys.union1);
    utFree(dvSparsegroups.Sym);
    utFree(dvSparsegroups.Class);
    utFree(dvSparsegroups.NextClassSparsegroup);
    utFree(dvSparsegroups.PrevClassSparsegroup);
    utFree(dvSparsegroups.NextTableClassSparsegroup);
    utFree(dvSparsegroups.FirstProperty);
    utFree(dvSparsegroups.LastProperty);
    utFree(dvSparsegroups.Relationship);
    utFree(dvRelationships.Type);
    utFree(dvRelationships.ParentLabelSym);
    utFree(dvRelationships.ChildLabelSym);
    utFree(dvRelationships.Mandatory);
    utFree(dvRelationships.Cascade);
    utFree(dvRelationships.AccessChild);
    utFree(dvRelationships.AccessParent);
    utFree(dvRelationships.SharedParent);
    utFree(dvRelationships.Sparse);
    utFree(dvRelationships.Expanded);
    utFree(dvRelationships.Unordered);
    utFree(dvRelationships.Schema);
    utFree(dvRelationships.NextSchemaRelationship);
    utFree(dvRelationships.ParentClass);
    utFree(dvRelationships.NextClassChildRelationship);
    utFree(dvRelationships.ChildClass);
    utFree(dvRelationships.NextClassParentRelationship);
    utFree(dvRelationships.FirstProperty);
    utFree(dvRelationships.LastProperty);
    utFree(dvRelationships.FirstKey);
    utFree(dvRelationships.LastKey);
    utFree(dvRelationships.ParentSparsegroup);
    utFree(dvRelationships.ChildSparsegroup);
    utFree(dvKeys.LineNum);
    utFree(dvKeys.Relationship);
    utFree(dvKeys.NextRelationshipKey);
    utFree(dvKeys.FirstKeyproperty);
    utFree(dvKeys.LastKeyproperty);
    utFree(dvKeypropertys.PropertySym);
    utFree(dvKeypropertys.Property);
    utFree(dvKeypropertys.NextPropertyKeyproperty);
    utFree(dvKeypropertys.Key);
    utFree(dvKeypropertys.NextKeyKeyproperty);
    utFree(dvUnions.PropertySym);
    utFree(dvUnions.TypeProperty);
    utFree(dvUnions.Line);
    utFree(dvUnions.Number);
    utFree(dvUnions.FieldNumber);
    utFree(dvUnions.NumCases);
    utFree(dvUnions.Class);
    utFree(dvUnions.NextClassUnion);
    utFree(dvUnions.FirstProperty);
    utFree(dvUnions.LastProperty);
    utFree(dvCases.EntrySym);
    utFree(dvCases.Entry);
    utFree(dvCases.NextEntryCase);
    utFree(dvCases.Property);
    utFree(dvCases.NextPropertyCase);
    utFree(dvCaches.Number);
    utFree(dvCaches.Line);
    utFree(dvCaches.Class);
    utFree(dvCaches.NextClassCache);
    utFree(dvCaches.FirstProperty);
    utFree(dvCaches.LastProperty);
    utFree(dvCaches.FirstPropident);
    utFree(dvCaches.LastPropident);
    utFree(dvPropidents.Sym);
    utFree(dvPropidents.Cache);
    utFree(dvPropidents.NextCachePropident);
    utUnregisterModule(dvModuleID);
}

/*----------------------------------------------------------------------------------------
  Allocate memory used by the dv database.
----------------------------------------------------------------------------------------*/
void dvDatabaseStart(void)
{
    if(!utInitialized()) {
        utStart();
    }
    dvRootData.hash = 0xf96c7007;
    dvModuleID = utRegisterModule("dv", false, dvHash(), 18, 238, 3, sizeof(struct dvRootType_),
        &dvRootData, dvDatabaseStart, dvDatabaseStop);
    utRegisterEnum("RelationshipType", 9);
    utRegisterEntry("LINKED_LIST", 0);
    utRegisterEntry("DOUBLY_LINKED", 1);
    utRegisterEntry("TAIL_LINKED", 2);
    utRegisterEntry("POINTER", 3);
    utRegisterEntry("ARRAY", 4);
    utRegisterEntry("HEAP", 5);
    utRegisterEntry("HASHED", 6);
    utRegisterEntry("ORDERED_LIST", 7);
    utRegisterEntry("UNBOUND", 8);
    utRegisterEnum("PropertyType", 12);
    utRegisterEntry("INT", 0);
    utRegisterEntry("UINT", 1);
    utRegisterEntry("FLOAT", 2);
    utRegisterEntry("DOUBLE", 3);
    utRegisterEntry("BIT", 4);
    utRegisterEntry("BOOL", 5);
    utRegisterEntry("CHAR", 6);
    utRegisterEntry("ENUM", 7);
    utRegisterEntry("TYPEDEF", 8);
    utRegisterEntry("POINTER", 9);
    utRegisterEntry("SYM", 10);
    utRegisterEntry("UNBOUND", 11);
    utRegisterEnum("MemoryStyle", 2);
    utRegisterEntry("CREATE_ONLY", 0);
    utRegisterEntry("FREE_LIST", 1);
    utRegisterClass("Root", 12, &dvRootData.usedRoot, &dvRootData.allocatedRoot,
        NULL, 65535, 4, allocRoot, NULL);
    utRegisterField("FirstModpath", &dvRoots.FirstModpath, sizeof(dvModpath), UT_POINTER, "Modpath");
    utRegisterField("LastModpath", &dvRoots.LastModpath, sizeof(dvModpath), UT_POINTER, "Modpath");
    utRegisterField("ModpathTableIndex_", &dvRoots.ModpathTableIndex_, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("NumModpathTable", &dvRoots.NumModpathTable, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("ModpathTable", &dvRoots.ModpathTable, sizeof(dvModpath), UT_POINTER, "Modpath");
    utRegisterArray(&dvRootData.usedRootModpathTable, &dvRootData.allocatedRootModpathTable,
        getRootModpathTables, allocRootModpathTables, dvCompactRootModpathTables);
    utRegisterField("NumModpath", &dvRoots.NumModpath, sizeof(uint32), UT_UINT, NULL);
    utRegisterField("FirstModule", &dvRoots.FirstModule, sizeof(dvModule), UT_POINTER, "Module");
    utRegisterField("LastModule", &dvRoots.LastModule, sizeof(dvModule), UT_POINTER, "Module");
    utRegisterField("ModuleTableIndex_", &dvRoots.ModuleTableIndex_, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("NumModuleTable", &dvRoots.NumModuleTable, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("ModuleTable", &dvRoots.ModuleTable, sizeof(dvModule), UT_POINTER, "Module");
    utRegisterArray(&dvRootData.usedRootModuleTable, &dvRootData.allocatedRootModuleTable,
        getRootModuleTables, allocRootModuleTables, dvCompactRootModuleTables);
    utRegisterField("NumModule", &dvRoots.NumModule, sizeof(uint32), UT_UINT, NULL);
    utRegisterClass("Modpath", 5, &dvRootData.usedModpath, &dvRootData.allocatedModpath,
        NULL, 65535, 4, allocModpath, NULL);
    utRegisterField("Sym", &dvModpaths.Sym, sizeof(utSym), UT_SYM, NULL);
    utRegisterField("Root", &dvModpaths.Root, sizeof(dvRoot), UT_POINTER, "Root");
    utRegisterField("NextRootModpath", &dvModpaths.NextRootModpath, sizeof(dvModpath), UT_POINTER, "Modpath");
    utRegisterField("PrevRootModpath", &dvModpaths.PrevRootModpath, sizeof(dvModpath), UT_POINTER, "Modpath");
    utRegisterField("NextTableRootModpath", &dvModpaths.NextTableRootModpath, sizeof(dvModpath), UT_POINTER, "Modpath");
    utRegisterClass("Module", 40, &dvRootData.usedModule, &dvRootData.allocatedModule,
        NULL, 65535, 4, allocModule, NULL);
    utRegisterField("Sym", &dvModules.Sym, sizeof(utSym), UT_SYM, NULL);
    utRegisterField("PrefixSym", &dvModules.PrefixSym, sizeof(utSym), UT_SYM, NULL);
    utRegisterField("Persistent", &dvModules.Persistent, sizeof(uint8), UT_BIT, NULL);
    utRegisterField("UndoRedo", &dvModules.UndoRedo, sizeof(uint8), UT_BIT, NULL);
    utRegisterField("HasSparseData", &dvModules.HasSparseData, sizeof(uint8), UT_BIT, NULL);
    utRegisterField("NumFields", &dvModules.NumFields, sizeof(uint16), UT_UINT, NULL);
    utRegisterField("NumClasses", &dvModules.NumClasses, sizeof(uint32), UT_UINT, NULL);
    utRegisterField("NumEnums", &dvModules.NumEnums, sizeof(uint32), UT_UINT, NULL);
    utRegisterField("Elaborated", &dvModules.Elaborated, sizeof(uint8), UT_BIT, NULL);
    utRegisterField("NextRootModule", &dvModules.NextRootModule, sizeof(dvModule), UT_POINTER, "Module");
    utRegisterField("PrevRootModule", &dvModules.PrevRootModule, sizeof(dvModule), UT_POINTER, "Module");
    utRegisterField("NextTableRootModule", &dvModules.NextTableRootModule, sizeof(dvModule), UT_POINTER, "Module");
    utRegisterField("FirstClass", &dvModules.FirstClass, sizeof(dvClass), UT_POINTER, "Class");
    utRegisterField("LastClass", &dvModules.LastClass, sizeof(dvClass), UT_POINTER, "Class");
    utRegisterField("ClassTableIndex_", &dvModules.ClassTableIndex_, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("NumClassTable", &dvModules.NumClassTable, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("ClassTable", &dvModules.ClassTable, sizeof(dvClass), UT_POINTER, "Class");
    utRegisterArray(&dvRootData.usedModuleClassTable, &dvRootData.allocatedModuleClassTable,
        getModuleClassTables, allocModuleClassTables, dvCompactModuleClassTables);
    utRegisterField("NumClass", &dvModules.NumClass, sizeof(uint32), UT_UINT, NULL);
    utRegisterField("FirstEnum", &dvModules.FirstEnum, sizeof(dvEnum), UT_POINTER, "Enum");
    utRegisterField("LastEnum", &dvModules.LastEnum, sizeof(dvEnum), UT_POINTER, "Enum");
    utRegisterField("EnumTableIndex_", &dvModules.EnumTableIndex_, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("NumEnumTable", &dvModules.NumEnumTable, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("EnumTable", &dvModules.EnumTable, sizeof(dvEnum), UT_POINTER, "Enum");
    utRegisterArray(&dvRootData.usedModuleEnumTable, &dvRootData.allocatedModuleEnumTable,
        getModuleEnumTables, allocModuleEnumTables, dvCompactModuleEnumTables);
    utRegisterField("NumEnum", &dvModules.NumEnum, sizeof(uint32), UT_UINT, NULL);
    utRegisterField("FirstTypedef", &dvModules.FirstTypedef, sizeof(dvTypedef), UT_POINTER, "Typedef");
    utRegisterField("LastTypedef", &dvModules.LastTypedef, sizeof(dvTypedef), UT_POINTER, "Typedef");
    utRegisterField("TypedefTableIndex_", &dvModules.TypedefTableIndex_, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("NumTypedefTable", &dvModules.NumTypedefTable, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("TypedefTable", &dvModules.TypedefTable, sizeof(dvTypedef), UT_POINTER, "Typedef");
    utRegisterArray(&dvRootData.usedModuleTypedefTable, &dvRootData.allocatedModuleTypedefTable,
        getModuleTypedefTables, allocModuleTypedefTables, dvCompactModuleTypedefTables);
    utRegisterField("NumTypedef", &dvModules.NumTypedef, sizeof(uint32), UT_UINT, NULL);
    utRegisterField("FirstSchema", &dvModules.FirstSchema, sizeof(dvSchema), UT_POINTER, "Schema");
    utRegisterField("LastSchema", &dvModules.LastSchema, sizeof(dvSchema), UT_POINTER, "Schema");
    utRegisterField("SchemaTableIndex_", &dvModules.SchemaTableIndex_, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("NumSchemaTable", &dvModules.NumSchemaTable, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("SchemaTable", &dvModules.SchemaTable, sizeof(dvSchema), UT_POINTER, "Schema");
    utRegisterArray(&dvRootData.usedModuleSchemaTable, &dvRootData.allocatedModuleSchemaTable,
        getModuleSchemaTables, allocModuleSchemaTables, dvCompactModuleSchemaTables);
    utRegisterField("NumSchema", &dvModules.NumSchema, sizeof(uint32), UT_UINT, NULL);
    utRegisterField("FirstImportLink", &dvModules.FirstImportLink, sizeof(dvLink), UT_POINTER, "Link");
    utRegisterField("LastImportLink", &dvModules.LastImportLink, sizeof(dvLink), UT_POINTER, "Link");
    utRegisterField("FirstExportLink", &dvModules.FirstExportLink, sizeof(dvLink), UT_POINTER, "Link");
    utRegisterField("LastExportLink", &dvModules.LastExportLink, sizeof(dvLink), UT_POINTER, "Link");
    utRegisterClass("Link", 4, &dvRootData.usedLink, &dvRootData.allocatedLink,
        NULL, 65535, 4, allocLink, NULL);
    utRegisterField("ImportModule", &dvLinks.ImportModule, sizeof(dvModule), UT_POINTER, "Module");
    utRegisterField("NextModuleImportLink", &dvLinks.NextModuleImportLink, sizeof(dvLink), UT_POINTER, "Link");
    utRegisterField("ExportModule", &dvLinks.ExportModule, sizeof(dvModule), UT_POINTER, "Module");
    utRegisterField("NextModuleExportLink", &dvLinks.NextModuleExportLink, sizeof(dvLink), UT_POINTER, "Link");
    utRegisterClass("Schema", 7, &dvRootData.usedSchema, &dvRootData.allocatedSchema,
        NULL, 65535, 4, allocSchema, NULL);
    utRegisterField("Sym", &dvSchemas.Sym, sizeof(utSym), UT_SYM, NULL);
    utRegisterField("Module", &dvSchemas.Module, sizeof(dvModule), UT_POINTER, "Module");
    utRegisterField("NextModuleSchema", &dvSchemas.NextModuleSchema, sizeof(dvSchema), UT_POINTER, "Schema");
    utRegisterField("PrevModuleSchema", &dvSchemas.PrevModuleSchema, sizeof(dvSchema), UT_POINTER, "Schema");
    utRegisterField("NextTableModuleSchema", &dvSchemas.NextTableModuleSchema, sizeof(dvSchema), UT_POINTER, "Schema");
    utRegisterField("FirstRelationship", &dvSchemas.FirstRelationship, sizeof(dvRelationship), UT_POINTER, "Relationship");
    utRegisterField("LastRelationship", &dvSchemas.LastRelationship, sizeof(dvRelationship), UT_POINTER, "Relationship");
    utRegisterClass("Enum", 13, &dvRootData.usedEnum, &dvRootData.allocatedEnum,
        NULL, 65535, 4, allocEnum, NULL);
    utRegisterField("Sym", &dvEnums.Sym, sizeof(utSym), UT_SYM, NULL);
    utRegisterField("PrefixSym", &dvEnums.PrefixSym, sizeof(utSym), UT_SYM, NULL);
    utRegisterField("NumEntries", &dvEnums.NumEntries, sizeof(uint16), UT_UINT, NULL);
    utRegisterField("Module", &dvEnums.Module, sizeof(dvModule), UT_POINTER, "Module");
    utRegisterField("NextModuleEnum", &dvEnums.NextModuleEnum, sizeof(dvEnum), UT_POINTER, "Enum");
    utRegisterField("PrevModuleEnum", &dvEnums.PrevModuleEnum, sizeof(dvEnum), UT_POINTER, "Enum");
    utRegisterField("NextTableModuleEnum", &dvEnums.NextTableModuleEnum, sizeof(dvEnum), UT_POINTER, "Enum");
    utRegisterField("FirstEntry", &dvEnums.FirstEntry, sizeof(dvEntry), UT_POINTER, "Entry");
    utRegisterField("LastEntry", &dvEnums.LastEntry, sizeof(dvEntry), UT_POINTER, "Entry");
    utRegisterField("EntryTableIndex_", &dvEnums.EntryTableIndex_, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("NumEntryTable", &dvEnums.NumEntryTable, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("EntryTable", &dvEnums.EntryTable, sizeof(dvEntry), UT_POINTER, "Entry");
    utRegisterArray(&dvRootData.usedEnumEntryTable, &dvRootData.allocatedEnumEntryTable,
        getEnumEntryTables, allocEnumEntryTables, dvCompactEnumEntryTables);
    utRegisterField("NumEntry", &dvEnums.NumEntry, sizeof(uint32), UT_UINT, NULL);
    utRegisterClass("Entry", 8, &dvRootData.usedEntry, &dvRootData.allocatedEntry,
        NULL, 65535, 4, allocEntry, NULL);
    utRegisterField("Sym", &dvEntrys.Sym, sizeof(utSym), UT_SYM, NULL);
    utRegisterField("Value", &dvEntrys.Value, sizeof(uint32), UT_UINT, NULL);
    utRegisterField("Enum", &dvEntrys.Enum, sizeof(dvEnum), UT_POINTER, "Enum");
    utRegisterField("NextEnumEntry", &dvEntrys.NextEnumEntry, sizeof(dvEntry), UT_POINTER, "Entry");
    utRegisterField("PrevEnumEntry", &dvEntrys.PrevEnumEntry, sizeof(dvEntry), UT_POINTER, "Entry");
    utRegisterField("NextTableEnumEntry", &dvEntrys.NextTableEnumEntry, sizeof(dvEntry), UT_POINTER, "Entry");
    utRegisterField("FirstCase", &dvEntrys.FirstCase, sizeof(dvCase), UT_POINTER, "Case");
    utRegisterField("LastCase", &dvEntrys.LastCase, sizeof(dvCase), UT_POINTER, "Case");
    utRegisterClass("Typedef", 8, &dvRootData.usedTypedef, &dvRootData.allocatedTypedef,
        NULL, 65535, 4, allocTypedef, NULL);
    utRegisterField("Sym", &dvTypedefs.Sym, sizeof(utSym), UT_SYM, NULL);
    utRegisterField("InitializerIndex_", &dvTypedefs.InitializerIndex_, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("NumInitializer", &dvTypedefs.NumInitializer, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("Initializer", &dvTypedefs.Initializer, sizeof(char), UT_CHAR, NULL);
    utRegisterArray(&dvRootData.usedTypedefInitializer, &dvRootData.allocatedTypedefInitializer,
        getTypedefInitializers, allocTypedefInitializers, dvCompactTypedefInitializers);
    utRegisterField("Module", &dvTypedefs.Module, sizeof(dvModule), UT_POINTER, "Module");
    utRegisterField("NextModuleTypedef", &dvTypedefs.NextModuleTypedef, sizeof(dvTypedef), UT_POINTER, "Typedef");
    utRegisterField("PrevModuleTypedef", &dvTypedefs.PrevModuleTypedef, sizeof(dvTypedef), UT_POINTER, "Typedef");
    utRegisterField("NextTableModuleTypedef", &dvTypedefs.NextTableModuleTypedef, sizeof(dvTypedef), UT_POINTER, "Typedef");
    utRegisterClass("Class", 38, &dvRootData.usedClass, &dvRootData.allocatedClass,
        NULL, 65535, 4, allocClass, NULL);
    utRegisterField("Sym", &dvClasss.Sym, sizeof(utSym), UT_SYM, NULL);
    utRegisterField("MemoryStyle", &dvClasss.MemoryStyle, sizeof(dvMemoryStyle), UT_ENUM, "MemoryStyle");
    utRegisterField("ReferenceSize", &dvClasss.ReferenceSize, sizeof(uint8), UT_UINT, NULL);
    utRegisterField("GenerateArrayClass", &dvClasss.GenerateArrayClass, sizeof(uint8), UT_BIT, NULL);
    utRegisterField("GenerateAttributes", &dvClasss.GenerateAttributes, sizeof(uint8), UT_BIT, NULL);
    utRegisterField("Sparse", &dvClasss.Sparse, sizeof(uint8), UT_BIT, NULL);
    utRegisterField("NumFields", &dvClasss.NumFields, sizeof(uint16), UT_UINT, NULL);
    utRegisterField("Number", &dvClasss.Number, sizeof(uint16), UT_UINT, NULL);
    utRegisterField("BaseClassSym", &dvClasss.BaseClassSym, sizeof(utSym), UT_SYM, NULL);
    utRegisterField("Module", &dvClasss.Module, sizeof(dvModule), UT_POINTER, "Module");
    utRegisterField("NextModuleClass", &dvClasss.NextModuleClass, sizeof(dvClass), UT_POINTER, "Class");
    utRegisterField("PrevModuleClass", &dvClasss.PrevModuleClass, sizeof(dvClass), UT_POINTER, "Class");
    utRegisterField("NextTableModuleClass", &dvClasss.NextTableModuleClass, sizeof(dvClass), UT_POINTER, "Class");
    utRegisterField("FirstProperty", &dvClasss.FirstProperty, sizeof(dvProperty), UT_POINTER, "Property");
    utRegisterField("LastProperty", &dvClasss.LastProperty, sizeof(dvProperty), UT_POINTER, "Property");
    utRegisterField("PropertyTableIndex_", &dvClasss.PropertyTableIndex_, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("NumPropertyTable", &dvClasss.NumPropertyTable, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("PropertyTable", &dvClasss.PropertyTable, sizeof(dvProperty), UT_POINTER, "Property");
    utRegisterArray(&dvRootData.usedClassPropertyTable, &dvRootData.allocatedClassPropertyTable,
        getClassPropertyTables, allocClassPropertyTables, dvCompactClassPropertyTables);
    utRegisterField("NumProperty", &dvClasss.NumProperty, sizeof(uint32), UT_UINT, NULL);
    utRegisterField("FreeListProperty", &dvClasss.FreeListProperty, sizeof(dvProperty), UT_POINTER, "Property");
    utRegisterField("FirstSparsegroup", &dvClasss.FirstSparsegroup, sizeof(dvSparsegroup), UT_POINTER, "Sparsegroup");
    utRegisterField("LastSparsegroup", &dvClasss.LastSparsegroup, sizeof(dvSparsegroup), UT_POINTER, "Sparsegroup");
    utRegisterField("SparsegroupTableIndex_", &dvClasss.SparsegroupTableIndex_, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("NumSparsegroupTable", &dvClasss.NumSparsegroupTable, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("SparsegroupTable", &dvClasss.SparsegroupTable, sizeof(dvSparsegroup), UT_POINTER, "Sparsegroup");
    utRegisterArray(&dvRootData.usedClassSparsegroupTable, &dvRootData.allocatedClassSparsegroupTable,
        getClassSparsegroupTables, allocClassSparsegroupTables, dvCompactClassSparsegroupTables);
    utRegisterField("NumSparsegroup", &dvClasss.NumSparsegroup, sizeof(uint32), UT_UINT, NULL);
    utRegisterField("BaseClass", &dvClasss.BaseClass, sizeof(dvClass), UT_POINTER, "Class");
    utRegisterField("FirstDerivedClass", &dvClasss.FirstDerivedClass, sizeof(dvClass), UT_POINTER, "Class");
    utRegisterField("NextClassDerivedClass", &dvClasss.NextClassDerivedClass, sizeof(dvClass), UT_POINTER, "Class");
    utRegisterField("LastDerivedClass", &dvClasss.LastDerivedClass, sizeof(dvClass), UT_POINTER, "Class");
    utRegisterField("FirstChildRelationship", &dvClasss.FirstChildRelationship, sizeof(dvRelationship), UT_POINTER, "Relationship");
    utRegisterField("LastChildRelationship", &dvClasss.LastChildRelationship, sizeof(dvRelationship), UT_POINTER, "Relationship");
    utRegisterField("FirstParentRelationship", &dvClasss.FirstParentRelationship, sizeof(dvRelationship), UT_POINTER, "Relationship");
    utRegisterField("LastParentRelationship", &dvClasss.LastParentRelationship, sizeof(dvRelationship), UT_POINTER, "Relationship");
    utRegisterField("FirstUnion", &dvClasss.FirstUnion, sizeof(dvUnion), UT_POINTER, "Union");
    utRegisterField("LastUnion", &dvClasss.LastUnion, sizeof(dvUnion), UT_POINTER, "Union");
    utRegisterField("FirstCache", &dvClasss.FirstCache, sizeof(dvCache), UT_POINTER, "Cache");
    utRegisterField("LastCache", &dvClasss.LastCache, sizeof(dvCache), UT_POINTER, "Cache");
    utRegisterClass("Property", 36, &dvRootData.usedProperty, &dvRootData.allocatedProperty,
        NULL, 65535, 4, allocProperty, NULL);
    utRegisterField("Sym", &dvPropertys.Sym, sizeof(utSym), UT_SYM, NULL);
    utRegisterField("Type", &dvPropertys.Type, sizeof(dvPropertyType), UT_ENUM, "PropertyType");
    utRegisterField("Array", &dvPropertys.Array, sizeof(uint8), UT_BIT, NULL);
    utRegisterField("Cascade", &dvPropertys.Cascade, sizeof(uint8), UT_BIT, NULL);
    utRegisterField("Sparse", &dvPropertys.Sparse, sizeof(uint8), UT_BIT, NULL);
    utRegisterField("View", &dvPropertys.View, sizeof(uint8), UT_BIT, NULL);
    utRegisterField("Expanded", &dvPropertys.Expanded, sizeof(uint8), UT_BIT, NULL);
    utRegisterField("FieldNumber", &dvPropertys.FieldNumber, sizeof(uint32), UT_UINT, NULL);
    utRegisterField("FirstElementProp", &dvPropertys.FirstElementProp, sizeof(dvProperty), UT_POINTER, "Property");
    utRegisterField("NumElementsProp", &dvPropertys.NumElementsProp, sizeof(dvProperty), UT_POINTER, "Property");
    utRegisterField("Hidden", &dvPropertys.Hidden, sizeof(uint8), UT_BIT, NULL);
    utRegisterField("InitializerIndex_", &dvPropertys.InitializerIndex_, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("NumInitializer", &dvPropertys.NumInitializer, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("Initializer", &dvPropertys.Initializer, sizeof(char), UT_CHAR, NULL);
    utRegisterArray(&dvRootData.usedPropertyInitializer, &dvRootData.allocatedPropertyInitializer,
        getPropertyInitializers, allocPropertyInitializers, dvCompactPropertyInitializers);
    utRegisterField("FixedSize", &dvPropertys.FixedSize, sizeof(uint8), UT_BIT, NULL);
    utRegisterField("IndexIndex_", &dvPropertys.IndexIndex_, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("NumIndex", &dvPropertys.NumIndex, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("Index", &dvPropertys.Index, sizeof(char), UT_CHAR, NULL);
    utRegisterArray(&dvRootData.usedPropertyIndex, &dvRootData.allocatedPropertyIndex,
        getPropertyIndexs, allocPropertyIndexs, dvCompactPropertyIndexs);
    utRegisterField("Line", &dvPropertys.Line, sizeof(uint32), UT_UINT, NULL);
    utRegisterField("Class", &dvPropertys.Class, sizeof(dvClass), UT_POINTER, "Class");
    utRegisterField("NextClassProperty", &dvPropertys.NextClassProperty, sizeof(dvProperty), UT_POINTER, "Property");
    utRegisterField("PrevClassProperty", &dvPropertys.PrevClassProperty, sizeof(dvProperty), UT_POINTER, "Property");
    utRegisterField("NextTableClassProperty", &dvPropertys.NextTableClassProperty, sizeof(dvProperty), UT_POINTER, "Property");
    utRegisterField("FirstCase", &dvPropertys.FirstCase, sizeof(dvCase), UT_POINTER, "Case");
    utRegisterField("LastCase", &dvPropertys.LastCase, sizeof(dvCase), UT_POINTER, "Case");
    utRegisterField("FirstKeyproperty", &dvPropertys.FirstKeyproperty, sizeof(dvKeyproperty), UT_POINTER, "Keyproperty");
    utRegisterField("LastKeyproperty", &dvPropertys.LastKeyproperty, sizeof(dvKeyproperty), UT_POINTER, "Keyproperty");
    utRegisterField("Sparsegroup", &dvPropertys.Sparsegroup, sizeof(dvSparsegroup), UT_POINTER, "Sparsegroup");
    utRegisterField("NextSparsegroupProperty", &dvPropertys.NextSparsegroupProperty, sizeof(dvProperty), UT_POINTER, "Property");
    utRegisterField("Relationship", &dvPropertys.Relationship, sizeof(dvRelationship), UT_POINTER, "Relationship");
    utRegisterField("NextRelationshipProperty", &dvPropertys.NextRelationshipProperty, sizeof(dvProperty), UT_POINTER, "Property");
    utRegisterField("Union", &dvPropertys.Union, sizeof(dvUnion), UT_POINTER, "Union");
    utRegisterField("NextUnionProperty", &dvPropertys.NextUnionProperty, sizeof(dvProperty), UT_POINTER, "Property");
    utRegisterField("Cache", &dvPropertys.Cache, sizeof(dvCache), UT_POINTER, "Cache");
    utRegisterField("NextCacheProperty", &dvPropertys.NextCacheProperty, sizeof(dvProperty), UT_POINTER, "Property");
    utRegisterField("union1", &dvPropertys.union1, sizeof(dvPropertyUnion1), UT_UNION, "Type");
    utRegisterUnion("Type", 6);
    utRegisterUnionCase(7, UT_POINTER, sizeof(dvEnum));
    utRegisterUnionCase(8, UT_POINTER, sizeof(dvTypedef));
    utRegisterUnionCase(9, UT_POINTER, sizeof(dvClass));
    utRegisterUnionCase(10, UT_SYM, sizeof(utSym));
    utRegisterUnionCase(0, UT_UINT, sizeof(uint8));
    utRegisterUnionCase(1, UT_UINT, sizeof(uint8));
    utRegisterClass("Sparsegroup", 8, &dvRootData.usedSparsegroup, &dvRootData.allocatedSparsegroup,
        &dvRootData.firstFreeSparsegroup, 171, 4, allocSparsegroup, destroySparsegroup);
    utRegisterField("Sym", &dvSparsegroups.Sym, sizeof(utSym), UT_SYM, NULL);
    utRegisterField("Class", &dvSparsegroups.Class, sizeof(dvClass), UT_POINTER, "Class");
    utRegisterField("NextClassSparsegroup", &dvSparsegroups.NextClassSparsegroup, sizeof(dvSparsegroup), UT_POINTER, "Sparsegroup");
    utRegisterField("PrevClassSparsegroup", &dvSparsegroups.PrevClassSparsegroup, sizeof(dvSparsegroup), UT_POINTER, "Sparsegroup");
    utRegisterField("NextTableClassSparsegroup", &dvSparsegroups.NextTableClassSparsegroup, sizeof(dvSparsegroup), UT_POINTER, "Sparsegroup");
    utRegisterField("FirstProperty", &dvSparsegroups.FirstProperty, sizeof(dvProperty), UT_POINTER, "Property");
    utRegisterField("LastProperty", &dvSparsegroups.LastProperty, sizeof(dvProperty), UT_POINTER, "Property");
    utRegisterField("Relationship", &dvSparsegroups.Relationship, sizeof(dvRelationship), UT_POINTER, "Relationship");
    utRegisterClass("Relationship", 23, &dvRootData.usedRelationship, &dvRootData.allocatedRelationship,
        NULL, 65535, 4, allocRelationship, NULL);
    utRegisterField("Type", &dvRelationships.Type, sizeof(dvRelationshipType), UT_ENUM, "RelationshipType");
    utRegisterField("ParentLabelSym", &dvRelationships.ParentLabelSym, sizeof(utSym), UT_SYM, NULL);
    utRegisterField("ChildLabelSym", &dvRelationships.ChildLabelSym, sizeof(utSym), UT_SYM, NULL);
    utRegisterField("Mandatory", &dvRelationships.Mandatory, sizeof(uint8), UT_BIT, NULL);
    utRegisterField("Cascade", &dvRelationships.Cascade, sizeof(uint8), UT_BIT, NULL);
    utRegisterField("AccessChild", &dvRelationships.AccessChild, sizeof(uint8), UT_BIT, NULL);
    utRegisterField("AccessParent", &dvRelationships.AccessParent, sizeof(uint8), UT_BIT, NULL);
    utRegisterField("SharedParent", &dvRelationships.SharedParent, sizeof(uint8), UT_BIT, NULL);
    utRegisterField("Sparse", &dvRelationships.Sparse, sizeof(uint8), UT_BIT, NULL);
    utRegisterField("Expanded", &dvRelationships.Expanded, sizeof(uint8), UT_BIT, NULL);
    utRegisterField("Unordered", &dvRelationships.Unordered, sizeof(uint8), UT_BIT, NULL);
    utRegisterField("Schema", &dvRelationships.Schema, sizeof(dvSchema), UT_POINTER, "Schema");
    utRegisterField("NextSchemaRelationship", &dvRelationships.NextSchemaRelationship, sizeof(dvRelationship), UT_POINTER, "Relationship");
    utRegisterField("ParentClass", &dvRelationships.ParentClass, sizeof(dvClass), UT_POINTER, "Class");
    utRegisterField("NextClassChildRelationship", &dvRelationships.NextClassChildRelationship, sizeof(dvRelationship), UT_POINTER, "Relationship");
    utRegisterField("ChildClass", &dvRelationships.ChildClass, sizeof(dvClass), UT_POINTER, "Class");
    utRegisterField("NextClassParentRelationship", &dvRelationships.NextClassParentRelationship, sizeof(dvRelationship), UT_POINTER, "Relationship");
    utRegisterField("FirstProperty", &dvRelationships.FirstProperty, sizeof(dvProperty), UT_POINTER, "Property");
    utRegisterField("LastProperty", &dvRelationships.LastProperty, sizeof(dvProperty), UT_POINTER, "Property");
    utRegisterField("FirstKey", &dvRelationships.FirstKey, sizeof(dvKey), UT_POINTER, "Key");
    utRegisterField("LastKey", &dvRelationships.LastKey, sizeof(dvKey), UT_POINTER, "Key");
    utRegisterField("ParentSparsegroup", &dvRelationships.ParentSparsegroup, sizeof(dvSparsegroup), UT_POINTER, "Sparsegroup");
    utRegisterField("ChildSparsegroup", &dvRelationships.ChildSparsegroup, sizeof(dvSparsegroup), UT_POINTER, "Sparsegroup");
    utRegisterClass("Key", 5, &dvRootData.usedKey, &dvRootData.allocatedKey,
        &dvRootData.firstFreeKey, 203, 4, allocKey, destroyKey);
    utRegisterField("LineNum", &dvKeys.LineNum, sizeof(uint32), UT_UINT, NULL);
    utRegisterField("Relationship", &dvKeys.Relationship, sizeof(dvRelationship), UT_POINTER, "Relationship");
    utRegisterField("NextRelationshipKey", &dvKeys.NextRelationshipKey, sizeof(dvKey), UT_POINTER, "Key");
    utRegisterField("FirstKeyproperty", &dvKeys.FirstKeyproperty, sizeof(dvKeyproperty), UT_POINTER, "Keyproperty");
    utRegisterField("LastKeyproperty", &dvKeys.LastKeyproperty, sizeof(dvKeyproperty), UT_POINTER, "Keyproperty");
    utRegisterClass("Keyproperty", 5, &dvRootData.usedKeyproperty, &dvRootData.allocatedKeyproperty,
        &dvRootData.firstFreeKeyproperty, 207, 4, allocKeyproperty, destroyKeyproperty);
    utRegisterField("PropertySym", &dvKeypropertys.PropertySym, sizeof(utSym), UT_SYM, NULL);
    utRegisterField("Property", &dvKeypropertys.Property, sizeof(dvProperty), UT_POINTER, "Property");
    utRegisterField("NextPropertyKeyproperty", &dvKeypropertys.NextPropertyKeyproperty, sizeof(dvKeyproperty), UT_POINTER, "Keyproperty");
    utRegisterField("Key", &dvKeypropertys.Key, sizeof(dvKey), UT_POINTER, "Key");
    utRegisterField("NextKeyKeyproperty", &dvKeypropertys.NextKeyKeyproperty, sizeof(dvKeyproperty), UT_POINTER, "Keyproperty");
    utRegisterClass("Union", 10, &dvRootData.usedUnion, &dvRootData.allocatedUnion,
        &dvRootData.firstFreeUnion, 212, 4, allocUnion, destroyUnion);
    utRegisterField("PropertySym", &dvUnions.PropertySym, sizeof(utSym), UT_SYM, NULL);
    utRegisterField("TypeProperty", &dvUnions.TypeProperty, sizeof(dvProperty), UT_POINTER, "Property");
    utRegisterField("Line", &dvUnions.Line, sizeof(uint32), UT_UINT, NULL);
    utRegisterField("Number", &dvUnions.Number, sizeof(uint16), UT_UINT, NULL);
    utRegisterField("FieldNumber", &dvUnions.FieldNumber, sizeof(uint32), UT_UINT, NULL);
    utRegisterField("NumCases", &dvUnions.NumCases, sizeof(uint16), UT_UINT, NULL);
    utRegisterField("Class", &dvUnions.Class, sizeof(dvClass), UT_POINTER, "Class");
    utRegisterField("NextClassUnion", &dvUnions.NextClassUnion, sizeof(dvUnion), UT_POINTER, "Union");
    utRegisterField("FirstProperty", &dvUnions.FirstProperty, sizeof(dvProperty), UT_POINTER, "Property");
    utRegisterField("LastProperty", &dvUnions.LastProperty, sizeof(dvProperty), UT_POINTER, "Property");
    utRegisterClass("Case", 5, &dvRootData.usedCase, &dvRootData.allocatedCase,
        &dvRootData.firstFreeCase, 222, 4, allocCase, destroyCase);
    utRegisterField("EntrySym", &dvCases.EntrySym, sizeof(utSym), UT_SYM, NULL);
    utRegisterField("Entry", &dvCases.Entry, sizeof(dvEntry), UT_POINTER, "Entry");
    utRegisterField("NextEntryCase", &dvCases.NextEntryCase, sizeof(dvCase), UT_POINTER, "Case");
    utRegisterField("Property", &dvCases.Property, sizeof(dvProperty), UT_POINTER, "Property");
    utRegisterField("NextPropertyCase", &dvCases.NextPropertyCase, sizeof(dvCase), UT_POINTER, "Case");
    utRegisterClass("Cache", 8, &dvRootData.usedCache, &dvRootData.allocatedCache,
        &dvRootData.firstFreeCache, 229, 4, allocCache, destroyCache);
    utRegisterField("Number", &dvCaches.Number, sizeof(uint16), UT_UINT, NULL);
    utRegisterField("Line", &dvCaches.Line, sizeof(uint32), UT_UINT, NULL);
    utRegisterField("Class", &dvCaches.Class, sizeof(dvClass), UT_POINTER, "Class");
    utRegisterField("NextClassCache", &dvCaches.NextClassCache, sizeof(dvCache), UT_POINTER, "Cache");
    utRegisterField("FirstProperty", &dvCaches.FirstProperty, sizeof(dvProperty), UT_POINTER, "Property");
    utRegisterField("LastProperty", &dvCaches.LastProperty, sizeof(dvProperty), UT_POINTER, "Property");
    utRegisterField("FirstPropident", &dvCaches.FirstPropident, sizeof(dvPropident), UT_POINTER, "Propident");
    utRegisterField("LastPropident", &dvCaches.LastPropident, sizeof(dvPropident), UT_POINTER, "Propident");
    utRegisterClass("Propident", 3, &dvRootData.usedPropident, &dvRootData.allocatedPropident,
        &dvRootData.firstFreePropident, 235, 4, allocPropident, destroyPropident);
    utRegisterField("Sym", &dvPropidents.Sym, sizeof(utSym), UT_SYM, NULL);
    utRegisterField("Cache", &dvPropidents.Cache, sizeof(dvCache), UT_POINTER, "Cache");
    utRegisterField("NextCachePropident", &dvPropidents.NextCachePropident, sizeof(dvPropident), UT_POINTER, "Propident");
    allocRoots();
    allocModpaths();
    allocModules();
    allocLinks();
    allocSchemas();
    allocEnums();
    allocEntrys();
    allocTypedefs();
    allocClasss();
    allocPropertys();
    allocSparsegroups();
    allocRelationships();
    allocKeys();
    allocKeypropertys();
    allocUnions();
    allocCases();
    allocCaches();
    allocPropidents();
}

