/*----------------------------------------------------------------------------------------
  Database ut
----------------------------------------------------------------------------------------*/

#include "ddutil.h"

struct utRootType_ utRootData;
uint8 utModuleID;
struct utSymtabFields utSymtabs;
struct utSymFields utSyms;
struct utDynarrayFields utDynarrays;
struct utSymArrayFields utSymArrays;

/*----------------------------------------------------------------------------------------
  Constructor/Destructor hooks.
----------------------------------------------------------------------------------------*/
utSymtabCallbackType utSymtabConstructorCallback;
utSymCallbackType utSymConstructorCallback;
utDynarrayCallbackType utDynarrayConstructorCallback;
utDynarrayCallbackType utDynarrayDestructorCallback;
utSymArrayCallbackType utSymArrayConstructorCallback;
utSymArrayCallbackType utSymArrayDestructorCallback;

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocSymtab(void)
{
    utSymtab Symtab = utSymtabAlloc();

    return utSymtab2Index(Symtab);
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Symtab.
----------------------------------------------------------------------------------------*/
static void allocSymtabs(void)
{
    utSetAllocatedSymtab(2);
    utSetUsedSymtab(1);
    utSymtabs.TableIndex = utNewA(uint32, (utAllocatedSymtab()));
    utSymtabs.NumTable = utNewA(uint32, (utAllocatedSymtab()));
    utSetUsedSymtabTable(0);
    utSetAllocatedSymtabTable(2);
    utSetFreeSymtabTable(0);
    utSymtabs.Table = utNewA(utSym, utAllocatedSymtabTable());
    utSymtabs.NumSym = utNewA(uint32, (utAllocatedSymtab()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Symtab.
----------------------------------------------------------------------------------------*/
static void reallocSymtabs(
    uint32 newSize)
{
    utRecordResize(utModuleID, 0, (utAllocatedSymtab()), true);
    utResizeArray(utSymtabs.TableIndex, (newSize));
    utRecordResize(utModuleID, 0, (newSize), false);
    utRecordResize(utModuleID, 1, (utAllocatedSymtab()), true);
    utResizeArray(utSymtabs.NumTable, (newSize));
    utRecordResize(utModuleID, 1, (newSize), false);
    utRecordResize(utModuleID, 3, (utAllocatedSymtab()), true);
    utResizeArray(utSymtabs.NumSym, (newSize));
    utRecordResize(utModuleID, 3, (newSize), false);
    utSetAllocatedSymtab(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Symtabs.
----------------------------------------------------------------------------------------*/
void utSymtabAllocMore(void)
{
    reallocSymtabs((uint32)(utAllocatedSymtab() + (utAllocatedSymtab() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Compact the Symtab.Table heap to free memory.
----------------------------------------------------------------------------------------*/
void utCompactSymtabTables(void)
{
    uint32 elementSize = sizeof(utSym);
    uint32 usedHeaderSize = (sizeof(utSymtab) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(utSymtab) + sizeof(uint32) + elementSize - 1)/elementSize;
    utSym *toPtr = utSymtabs.Table;
    utSym *fromPtr = toPtr;
    utSymtab Symtab;
    uint32 size;

    while(fromPtr < utSymtabs.Table + utUsedSymtabTable()) {
        Symtab = *(utSymtab *)(void *)fromPtr;
        if(Symtab != utSymtabNull) {
            /* Need to move it to toPtr */
            size = utMax(utSymtabGetNumTable(Symtab) + usedHeaderSize, freeHeaderSize);
            utRecordArray(utModuleID, 2, toPtr - utSymtabs.Table, size, true);
            memmove((void *)toPtr, (void *)fromPtr, size*elementSize);
            utRecordArray(utModuleID, 2, toPtr - utSymtabs.Table, size, false);
            utSymtabSetTableIndex(Symtab, toPtr - utSymtabs.Table + usedHeaderSize);
            toPtr += size;
        } else {
            /* Just skip it */
            size = *(uint32 *)(void *)(((utSymtab *)(void *)fromPtr) + 1);
        }
        fromPtr += size;
    }
    utSetUsedSymtabTable(toPtr - utSymtabs.Table);
    utSetFreeSymtabTable(0);
}

/*----------------------------------------------------------------------------------------
  Allocate more memory for the Symtab.Table heap.
----------------------------------------------------------------------------------------*/
static void allocMoreSymtabTables(
    uint32 spaceNeeded)
{
    uint32 freeSpace = utAllocatedSymtabTable() - utUsedSymtabTable();

    if((utFreeSymtabTable() << 2) > utUsedSymtabTable()) {
        utCompactSymtabTables();
        freeSpace = utAllocatedSymtabTable() - utUsedSymtabTable();
    }
    if(freeSpace < spaceNeeded) {
        utRecordResize(utModuleID, 2, utAllocatedSymtabTable(), true);
        utSetAllocatedSymtabTable(utAllocatedSymtabTable() + spaceNeeded - freeSpace +
            (utAllocatedSymtabTable() >> 1));
        utResizeArray(utSymtabs.Table, utAllocatedSymtabTable());
        utRecordResize(utModuleID, 2, utAllocatedSymtabTable(), false);
    }
}

/*----------------------------------------------------------------------------------------
  Allocate memory for a new Symtab.Table array.
----------------------------------------------------------------------------------------*/
void utSymtabAllocTables(
    utSymtab Symtab,
    uint32 numTables)
{
    uint32 freeSpace = utAllocatedSymtabTable() - utUsedSymtabTable();
    uint32 elementSize = sizeof(utSym);
    uint32 usedHeaderSize = (sizeof(utSymtab) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(utSymtab) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 spaceNeeded = utMax(numTables + usedHeaderSize, freeHeaderSize);

#if defined(DD_DEBUG)
    utAssert(utSymtabGetNumTable(Symtab) == 0);
#endif
    if(numTables == 0) {
        return;
    }
    if(freeSpace < spaceNeeded) {
        allocMoreSymtabTables(spaceNeeded);
    }
    utSymtabSetTableIndex(Symtab, utUsedSymtabTable() + usedHeaderSize);
    utSymtabSetNumTable(Symtab, numTables);
    utRecordArray(utModuleID, 2, utUsedSymtabTable(), numTables + usedHeaderSize, true);
    *(utSymtab *)(void *)(utSymtabs.Table + utUsedSymtabTable()) = Symtab;
    {
        uint32 xSymtab;
        for(xSymtab = (uint32)(utSymtabGetTableIndex(Symtab)); xSymtab < utSymtabGetTableIndex(Symtab) + numTables; xSymtab++) {
            utSymtabs.Table[xSymtab] = utSymNull;
        }
    }
    utRecordArray(utModuleID, 2, utUsedSymtabTable(), numTables + usedHeaderSize, false);
    utSetUsedSymtabTable(utUsedSymtabTable() + spaceNeeded);
}

/*----------------------------------------------------------------------------------------
  Wrapper around utSymtabGetTables for the database manager.
----------------------------------------------------------------------------------------*/
static void *getSymtabTables(
    uint64 objectNumber,
    uint32 *numValues)
{
    utSymtab Symtab = utIndex2Symtab((uint32)objectNumber);

    *numValues = utSymtabGetNumTable(Symtab);
    return utSymtabGetTables(Symtab);
}

/*----------------------------------------------------------------------------------------
  Wrapper around utSymtabAllocTables for the database manager.
----------------------------------------------------------------------------------------*/
static void *allocSymtabTables(
    uint64 objectNumber,
    uint32 numValues)
{
    utSymtab Symtab = utIndex2Symtab((uint32)objectNumber);

    utSymtabSetTableIndex(Symtab, 0);
    utSymtabSetNumTable(Symtab, 0);
    if(numValues == 0) {
        return NULL;
    }
    utSymtabAllocTables(Symtab, numValues);
    return utSymtabGetTables(Symtab);
}

/*----------------------------------------------------------------------------------------
  Free memory used by the Symtab.Table array.
----------------------------------------------------------------------------------------*/
void utSymtabFreeTables(
    utSymtab Symtab)
{
    uint32 elementSize = sizeof(utSym);
    uint32 usedHeaderSize = (sizeof(utSymtab) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(utSymtab) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 size = utMax(utSymtabGetNumTable(Symtab) + usedHeaderSize, freeHeaderSize);
    utSym *dataPtr = utSymtabGetTables(Symtab) - usedHeaderSize;

    if(utSymtabGetNumTable(Symtab) == 0) {
        return;
    }
    utRecordArray(utModuleID, 2, dataPtr - utSymtabs.Table, freeHeaderSize, true);
    *(utSymtab *)(void *)(dataPtr) = utSymtabNull;
    *(uint32 *)(void *)(((utSymtab *)(void *)dataPtr) + 1) = size;
    utRecordArray(utModuleID, 2, dataPtr - utSymtabs.Table, freeHeaderSize, false);
    utSymtabSetNumTable(Symtab, 0);
    utSetFreeSymtabTable(utFreeSymtabTable() + size);
}

/*----------------------------------------------------------------------------------------
  Resize the Symtab.Table array.
----------------------------------------------------------------------------------------*/
void utSymtabResizeTables(
    utSymtab Symtab,
    uint32 numTables)
{
    uint32 freeSpace;
    uint32 elementSize = sizeof(utSym);
    uint32 usedHeaderSize = (sizeof(utSymtab) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(utSymtab) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 newSize = utMax(numTables + usedHeaderSize, freeHeaderSize);
    uint32 oldSize = utMax(utSymtabGetNumTable(Symtab) + usedHeaderSize, freeHeaderSize);
    utSym *dataPtr;

    if(numTables == 0) {
        if(utSymtabGetNumTable(Symtab) != 0) {
            utSymtabFreeTables(Symtab);
        }
        return;
    }
    if(utSymtabGetNumTable(Symtab) == 0) {
        utSymtabAllocTables(Symtab, numTables);
        return;
    }
    freeSpace = utAllocatedSymtabTable() - utUsedSymtabTable();
    if(freeSpace < newSize) {
        allocMoreSymtabTables(newSize);
    }
    dataPtr = utSymtabGetTables(Symtab) - usedHeaderSize;
    utRecordArray(utModuleID, 2, utUsedSymtabTable(), newSize, true);
    utRecordArray(utModuleID, 2, dataPtr - utSymtabs.Table, freeHeaderSize, true);
    memcpy((void *)(utSymtabs.Table + utUsedSymtabTable()), dataPtr,
        elementSize*utMin(oldSize, newSize));
    if(newSize > oldSize) {
        {
            uint32 xSymtab;
            for(xSymtab = (uint32)(utUsedSymtabTable() + oldSize); xSymtab < utUsedSymtabTable() + oldSize + newSize - oldSize; xSymtab++) {
                utSymtabs.Table[xSymtab] = utSymNull;
            }
        }
    }
    *(utSymtab *)(void *)dataPtr = utSymtabNull;
    *(uint32 *)(void *)(((utSymtab *)(void *)dataPtr) + 1) = oldSize;
    utRecordArray(utModuleID, 2, utUsedSymtabTable(), newSize, false);
    utRecordArray(utModuleID, 2, dataPtr - utSymtabs.Table, freeHeaderSize, false);
    utSetFreeSymtabTable(utFreeSymtabTable() + oldSize);
    utSymtabSetTableIndex(Symtab, utUsedSymtabTable() + usedHeaderSize);
    utSymtabSetNumTable(Symtab, numTables);
    utSetUsedSymtabTable(utUsedSymtabTable() + newSize);
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Symtab.
----------------------------------------------------------------------------------------*/
void utSymtabCopyProps(
    utSymtab oldSymtab,
    utSymtab newSymtab)
{
    utSymtabSetNumSym(newSymtab, utSymtabGetNumSym(oldSymtab));
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void utShowSymtab(
    utSymtab Symtab)
{
    utDatabaseShowObject("ut", "Symtab", utSymtab2Index(Symtab));
}
#endif

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocSym(void)
{
    utSym Sym = utSymAlloc();

    return utSym2Index(Sym);
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Sym.
----------------------------------------------------------------------------------------*/
static void allocSyms(void)
{
    utSetAllocatedSym(2);
    utSetUsedSym(1);
    utSyms.NameIndex = utNewA(uint32, (utAllocatedSym()));
    utSyms.NumName = utNewA(uint32, (utAllocatedSym()));
    utSetUsedSymName(0);
    utSetAllocatedSymName(2);
    utSetFreeSymName(0);
    utSyms.Name = utNewA(char, utAllocatedSymName());
    utSyms.HashValue = utNewA(uint32, (utAllocatedSym()));
    utSyms.Next = utNewA(utSym, (utAllocatedSym()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Sym.
----------------------------------------------------------------------------------------*/
static void reallocSyms(
    uint32 newSize)
{
    utRecordResize(utModuleID, 4, (utAllocatedSym()), true);
    utResizeArray(utSyms.NameIndex, (newSize));
    utRecordResize(utModuleID, 4, (newSize), false);
    utRecordResize(utModuleID, 5, (utAllocatedSym()), true);
    utResizeArray(utSyms.NumName, (newSize));
    utRecordResize(utModuleID, 5, (newSize), false);
    utRecordResize(utModuleID, 7, (utAllocatedSym()), true);
    utResizeArray(utSyms.HashValue, (newSize));
    utRecordResize(utModuleID, 7, (newSize), false);
    utRecordResize(utModuleID, 8, (utAllocatedSym()), true);
    utResizeArray(utSyms.Next, (newSize));
    utRecordResize(utModuleID, 8, (newSize), false);
    utSetAllocatedSym(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Syms.
----------------------------------------------------------------------------------------*/
void utSymAllocMore(void)
{
    reallocSyms((uint32)(utAllocatedSym() + (utAllocatedSym() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Compact the Sym.Name heap to free memory.
----------------------------------------------------------------------------------------*/
void utCompactSymNames(void)
{
    uint32 elementSize = sizeof(char);
    uint32 usedHeaderSize = (sizeof(utSym) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(utSym) + sizeof(uint32) + elementSize - 1)/elementSize;
    char *toPtr = utSyms.Name;
    char *fromPtr = toPtr;
    utSym Sym;
    uint32 size;

    while(fromPtr < utSyms.Name + utUsedSymName()) {
        Sym = *(utSym *)(void *)fromPtr;
        if(Sym != utSymNull) {
            /* Need to move it to toPtr */
            size = utMax(utSymGetNumName(Sym) + usedHeaderSize, freeHeaderSize);
            utRecordArray(utModuleID, 6, toPtr - utSyms.Name, size, true);
            memmove((void *)toPtr, (void *)fromPtr, size*elementSize);
            utRecordArray(utModuleID, 6, toPtr - utSyms.Name, size, false);
            utSymSetNameIndex(Sym, toPtr - utSyms.Name + usedHeaderSize);
            toPtr += size;
        } else {
            /* Just skip it */
            size = *(uint32 *)(void *)(((utSym *)(void *)fromPtr) + 1);
        }
        fromPtr += size;
    }
    utSetUsedSymName(toPtr - utSyms.Name);
    utSetFreeSymName(0);
}

/*----------------------------------------------------------------------------------------
  Allocate more memory for the Sym.Name heap.
----------------------------------------------------------------------------------------*/
static void allocMoreSymNames(
    uint32 spaceNeeded)
{
    uint32 freeSpace = utAllocatedSymName() - utUsedSymName();

    if((utFreeSymName() << 2) > utUsedSymName()) {
        utCompactSymNames();
        freeSpace = utAllocatedSymName() - utUsedSymName();
    }
    if(freeSpace < spaceNeeded) {
        utRecordResize(utModuleID, 6, utAllocatedSymName(), true);
        utSetAllocatedSymName(utAllocatedSymName() + spaceNeeded - freeSpace +
            (utAllocatedSymName() >> 1));
        utResizeArray(utSyms.Name, utAllocatedSymName());
        utRecordResize(utModuleID, 6, utAllocatedSymName(), false);
    }
}

/*----------------------------------------------------------------------------------------
  Allocate memory for a new Sym.Name array.
----------------------------------------------------------------------------------------*/
void utSymAllocNames(
    utSym Sym,
    uint32 numNames)
{
    uint32 freeSpace = utAllocatedSymName() - utUsedSymName();
    uint32 elementSize = sizeof(char);
    uint32 usedHeaderSize = (sizeof(utSym) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(utSym) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 spaceNeeded = utMax(numNames + usedHeaderSize, freeHeaderSize);

#if defined(DD_DEBUG)
    utAssert(utSymGetNumName(Sym) == 0);
#endif
    if(numNames == 0) {
        return;
    }
    if(freeSpace < spaceNeeded) {
        allocMoreSymNames(spaceNeeded);
    }
    utSymSetNameIndex(Sym, utUsedSymName() + usedHeaderSize);
    utSymSetNumName(Sym, numNames);
    utRecordArray(utModuleID, 6, utUsedSymName(), numNames + usedHeaderSize, true);
    *(utSym *)(void *)(utSyms.Name + utUsedSymName()) = Sym;
    memset(utSyms.Name + utSymGetNameIndex(Sym), 0, ((numNames))*sizeof(char));
    utRecordArray(utModuleID, 6, utUsedSymName(), numNames + usedHeaderSize, false);
    utSetUsedSymName(utUsedSymName() + spaceNeeded);
}

/*----------------------------------------------------------------------------------------
  Wrapper around utSymGetNames for the database manager.
----------------------------------------------------------------------------------------*/
static void *getSymNames(
    uint64 objectNumber,
    uint32 *numValues)
{
    utSym Sym = utIndex2Sym((uint32)objectNumber);

    *numValues = utSymGetNumName(Sym);
    return utSymGetNames(Sym);
}

/*----------------------------------------------------------------------------------------
  Wrapper around utSymAllocNames for the database manager.
----------------------------------------------------------------------------------------*/
static void *allocSymNames(
    uint64 objectNumber,
    uint32 numValues)
{
    utSym Sym = utIndex2Sym((uint32)objectNumber);

    utSymSetNameIndex(Sym, 0);
    utSymSetNumName(Sym, 0);
    if(numValues == 0) {
        return NULL;
    }
    utSymAllocNames(Sym, numValues);
    return utSymGetNames(Sym);
}

/*----------------------------------------------------------------------------------------
  Free memory used by the Sym.Name array.
----------------------------------------------------------------------------------------*/
void utSymFreeNames(
    utSym Sym)
{
    uint32 elementSize = sizeof(char);
    uint32 usedHeaderSize = (sizeof(utSym) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(utSym) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 size = utMax(utSymGetNumName(Sym) + usedHeaderSize, freeHeaderSize);
    char *dataPtr = utSymGetNames(Sym) - usedHeaderSize;

    if(utSymGetNumName(Sym) == 0) {
        return;
    }
    utRecordArray(utModuleID, 6, dataPtr - utSyms.Name, freeHeaderSize, true);
    *(utSym *)(void *)(dataPtr) = utSymNull;
    *(uint32 *)(void *)(((utSym *)(void *)dataPtr) + 1) = size;
    utRecordArray(utModuleID, 6, dataPtr - utSyms.Name, freeHeaderSize, false);
    utSymSetNumName(Sym, 0);
    utSetFreeSymName(utFreeSymName() + size);
}

/*----------------------------------------------------------------------------------------
  Resize the Sym.Name array.
----------------------------------------------------------------------------------------*/
void utSymResizeNames(
    utSym Sym,
    uint32 numNames)
{
    uint32 freeSpace;
    uint32 elementSize = sizeof(char);
    uint32 usedHeaderSize = (sizeof(utSym) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(utSym) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 newSize = utMax(numNames + usedHeaderSize, freeHeaderSize);
    uint32 oldSize = utMax(utSymGetNumName(Sym) + usedHeaderSize, freeHeaderSize);
    char *dataPtr;

    if(numNames == 0) {
        if(utSymGetNumName(Sym) != 0) {
            utSymFreeNames(Sym);
        }
        return;
    }
    if(utSymGetNumName(Sym) == 0) {
        utSymAllocNames(Sym, numNames);
        return;
    }
    freeSpace = utAllocatedSymName() - utUsedSymName();
    if(freeSpace < newSize) {
        allocMoreSymNames(newSize);
    }
    dataPtr = utSymGetNames(Sym) - usedHeaderSize;
    utRecordArray(utModuleID, 6, utUsedSymName(), newSize, true);
    utRecordArray(utModuleID, 6, dataPtr - utSyms.Name, freeHeaderSize, true);
    memcpy((void *)(utSyms.Name + utUsedSymName()), dataPtr,
        elementSize*utMin(oldSize, newSize));
    if(newSize > oldSize) {
        memset(utSyms.Name + utUsedSymName() + oldSize, 0, ((newSize - oldSize))*sizeof(char));
    }
    *(utSym *)(void *)dataPtr = utSymNull;
    *(uint32 *)(void *)(((utSym *)(void *)dataPtr) + 1) = oldSize;
    utRecordArray(utModuleID, 6, utUsedSymName(), newSize, false);
    utRecordArray(utModuleID, 6, dataPtr - utSyms.Name, freeHeaderSize, false);
    utSetFreeSymName(utFreeSymName() + oldSize);
    utSymSetNameIndex(Sym, utUsedSymName() + usedHeaderSize);
    utSymSetNumName(Sym, numNames);
    utSetUsedSymName(utUsedSymName() + newSize);
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Sym.
----------------------------------------------------------------------------------------*/
void utSymCopyProps(
    utSym oldSym,
    utSym newSym)
{
    utSymSetHashValue(newSym, utSymGetHashValue(oldSym));
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void utShowSym(
    utSym Sym)
{
    utDatabaseShowObject("ut", "Sym", utSym2Index(Sym));
}
#endif

/*----------------------------------------------------------------------------------------
  Destroy Dynarray including everything in it. Remove from parents.
----------------------------------------------------------------------------------------*/
void utDynarrayDestroy(
    utDynarray Dynarray)
{
    if(utDynarrayDestructorCallback != NULL) {
        utDynarrayDestructorCallback(Dynarray);
    }
    utDynarrayFree(Dynarray);
}

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocDynarray(void)
{
    utDynarray Dynarray = utDynarrayAlloc();

    return utDynarray2Index(Dynarray);
}

/*----------------------------------------------------------------------------------------
  Destructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static void destroyDynarray(
    uint64 objectIndex)
{
    utDynarrayDestroy(utIndex2Dynarray((uint32)objectIndex));
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Dynarray.
----------------------------------------------------------------------------------------*/
static void allocDynarrays(void)
{
    utSetAllocatedDynarray(2);
    utSetUsedDynarray(1);
    utSetFirstFreeDynarray(utDynarrayNull);
    utDynarrays.ValueIndex = utNewA(uint32, (utAllocatedDynarray()));
    utDynarrays.NumValue = utNewA(uint32, (utAllocatedDynarray()));
    utSetUsedDynarrayValue(0);
    utSetAllocatedDynarrayValue(2);
    utSetFreeDynarrayValue(0);
    utDynarrays.Value = utNewA(uint8, utAllocatedDynarrayValue());
    utDynarrays.ValueSize = utNewA(uint16, (utAllocatedDynarray()));
    utDynarrays.UsedValue = utNewA(uint32, (utAllocatedDynarray()));
    utDynarrays.Size = utNewA(uint32, (utAllocatedDynarray()));
    utDynarrays.FreeList = utNewA(utDynarray, (utAllocatedDynarray()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Dynarray.
----------------------------------------------------------------------------------------*/
static void reallocDynarrays(
    uint32 newSize)
{
    utRecordResize(utModuleID, 9, (utAllocatedDynarray()), true);
    utResizeArray(utDynarrays.ValueIndex, (newSize));
    utRecordResize(utModuleID, 9, (newSize), false);
    utRecordResize(utModuleID, 10, (utAllocatedDynarray()), true);
    utResizeArray(utDynarrays.NumValue, (newSize));
    utRecordResize(utModuleID, 10, (newSize), false);
    utRecordResize(utModuleID, 12, (utAllocatedDynarray()), true);
    utResizeArray(utDynarrays.ValueSize, (newSize));
    utRecordResize(utModuleID, 12, (newSize), false);
    utRecordResize(utModuleID, 13, (utAllocatedDynarray()), true);
    utResizeArray(utDynarrays.UsedValue, (newSize));
    utRecordResize(utModuleID, 13, (newSize), false);
    utRecordResize(utModuleID, 14, (utAllocatedDynarray()), true);
    utResizeArray(utDynarrays.Size, (newSize));
    utRecordResize(utModuleID, 14, (newSize), false);
    utRecordResize(utModuleID, 15, (utAllocatedDynarray()), true);
    utResizeArray(utDynarrays.FreeList, (newSize));
    utRecordResize(utModuleID, 15, (newSize), false);
    utSetAllocatedDynarray(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Dynarrays.
----------------------------------------------------------------------------------------*/
void utDynarrayAllocMore(void)
{
    reallocDynarrays((uint32)(utAllocatedDynarray() + (utAllocatedDynarray() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Compact the Dynarray.Value heap to free memory.
----------------------------------------------------------------------------------------*/
void utCompactDynarrayValues(void)
{
    uint32 elementSize = sizeof(uint8);
    uint32 usedHeaderSize = (sizeof(utDynarray) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(utDynarray) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint8 *toPtr = utDynarrays.Value;
    uint8 *fromPtr = toPtr;
    utDynarray Dynarray;
    uint32 size;

    while(fromPtr < utDynarrays.Value + utUsedDynarrayValue()) {
        Dynarray = *(utDynarray *)(void *)fromPtr;
        if(Dynarray != utDynarrayNull) {
            /* Need to move it to toPtr */
            size = utMax(utDynarrayGetNumValue(Dynarray) + usedHeaderSize, freeHeaderSize);
            utRecordArray(utModuleID, 11, toPtr - utDynarrays.Value, size, true);
            memmove((void *)toPtr, (void *)fromPtr, size*elementSize);
            utRecordArray(utModuleID, 11, toPtr - utDynarrays.Value, size, false);
            utDynarraySetValueIndex(Dynarray, toPtr - utDynarrays.Value + usedHeaderSize);
            toPtr += size;
        } else {
            /* Just skip it */
            size = *(uint32 *)(void *)(((utDynarray *)(void *)fromPtr) + 1);
        }
        fromPtr += size;
    }
    utSetUsedDynarrayValue(toPtr - utDynarrays.Value);
    utSetFreeDynarrayValue(0);
}

/*----------------------------------------------------------------------------------------
  Allocate more memory for the Dynarray.Value heap.
----------------------------------------------------------------------------------------*/
static void allocMoreDynarrayValues(
    uint32 spaceNeeded)
{
    uint32 freeSpace = utAllocatedDynarrayValue() - utUsedDynarrayValue();

    if((utFreeDynarrayValue() << 2) > utUsedDynarrayValue()) {
        utCompactDynarrayValues();
        freeSpace = utAllocatedDynarrayValue() - utUsedDynarrayValue();
    }
    if(freeSpace < spaceNeeded) {
        utRecordResize(utModuleID, 11, utAllocatedDynarrayValue(), true);
        utSetAllocatedDynarrayValue(utAllocatedDynarrayValue() + spaceNeeded - freeSpace +
            (utAllocatedDynarrayValue() >> 1));
        utResizeArray(utDynarrays.Value, utAllocatedDynarrayValue());
        utRecordResize(utModuleID, 11, utAllocatedDynarrayValue(), false);
    }
}

/*----------------------------------------------------------------------------------------
  Allocate memory for a new Dynarray.Value array.
----------------------------------------------------------------------------------------*/
void utDynarrayAllocValues(
    utDynarray Dynarray,
    uint32 numValues)
{
    uint32 freeSpace = utAllocatedDynarrayValue() - utUsedDynarrayValue();
    uint32 elementSize = sizeof(uint8);
    uint32 usedHeaderSize = (sizeof(utDynarray) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(utDynarray) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 spaceNeeded = utMax(numValues + usedHeaderSize, freeHeaderSize);

#if defined(DD_DEBUG)
    utAssert(utDynarrayGetNumValue(Dynarray) == 0);
#endif
    if(numValues == 0) {
        return;
    }
    if(freeSpace < spaceNeeded) {
        allocMoreDynarrayValues(spaceNeeded);
    }
    utDynarraySetValueIndex(Dynarray, utUsedDynarrayValue() + usedHeaderSize);
    utDynarraySetNumValue(Dynarray, numValues);
    utRecordArray(utModuleID, 11, utUsedDynarrayValue(), numValues + usedHeaderSize, true);
    *(utDynarray *)(void *)(utDynarrays.Value + utUsedDynarrayValue()) = Dynarray;
    memset(utDynarrays.Value + utDynarrayGetValueIndex(Dynarray), 0, ((numValues))*sizeof(uint8));
    utRecordArray(utModuleID, 11, utUsedDynarrayValue(), numValues + usedHeaderSize, false);
    utSetUsedDynarrayValue(utUsedDynarrayValue() + spaceNeeded);
}

/*----------------------------------------------------------------------------------------
  Wrapper around utDynarrayGetValues for the database manager.
----------------------------------------------------------------------------------------*/
static void *getDynarrayValues(
    uint64 objectNumber,
    uint32 *numValues)
{
    utDynarray Dynarray = utIndex2Dynarray((uint32)objectNumber);

    *numValues = utDynarrayGetNumValue(Dynarray);
    return utDynarrayGetValues(Dynarray);
}

/*----------------------------------------------------------------------------------------
  Wrapper around utDynarrayAllocValues for the database manager.
----------------------------------------------------------------------------------------*/
static void *allocDynarrayValues(
    uint64 objectNumber,
    uint32 numValues)
{
    utDynarray Dynarray = utIndex2Dynarray((uint32)objectNumber);

    utDynarraySetValueIndex(Dynarray, 0);
    utDynarraySetNumValue(Dynarray, 0);
    if(numValues == 0) {
        return NULL;
    }
    utDynarrayAllocValues(Dynarray, numValues);
    return utDynarrayGetValues(Dynarray);
}

/*----------------------------------------------------------------------------------------
  Free memory used by the Dynarray.Value array.
----------------------------------------------------------------------------------------*/
void utDynarrayFreeValues(
    utDynarray Dynarray)
{
    uint32 elementSize = sizeof(uint8);
    uint32 usedHeaderSize = (sizeof(utDynarray) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(utDynarray) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 size = utMax(utDynarrayGetNumValue(Dynarray) + usedHeaderSize, freeHeaderSize);
    uint8 *dataPtr = utDynarrayGetValues(Dynarray) - usedHeaderSize;

    if(utDynarrayGetNumValue(Dynarray) == 0) {
        return;
    }
    utRecordArray(utModuleID, 11, dataPtr - utDynarrays.Value, freeHeaderSize, true);
    *(utDynarray *)(void *)(dataPtr) = utDynarrayNull;
    *(uint32 *)(void *)(((utDynarray *)(void *)dataPtr) + 1) = size;
    utRecordArray(utModuleID, 11, dataPtr - utDynarrays.Value, freeHeaderSize, false);
    utDynarraySetNumValue(Dynarray, 0);
    utSetFreeDynarrayValue(utFreeDynarrayValue() + size);
}

/*----------------------------------------------------------------------------------------
  Resize the Dynarray.Value array.
----------------------------------------------------------------------------------------*/
void utDynarrayResizeValues(
    utDynarray Dynarray,
    uint32 numValues)
{
    uint32 freeSpace;
    uint32 elementSize = sizeof(uint8);
    uint32 usedHeaderSize = (sizeof(utDynarray) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(utDynarray) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 newSize = utMax(numValues + usedHeaderSize, freeHeaderSize);
    uint32 oldSize = utMax(utDynarrayGetNumValue(Dynarray) + usedHeaderSize, freeHeaderSize);
    uint8 *dataPtr;

    if(numValues == 0) {
        if(utDynarrayGetNumValue(Dynarray) != 0) {
            utDynarrayFreeValues(Dynarray);
        }
        return;
    }
    if(utDynarrayGetNumValue(Dynarray) == 0) {
        utDynarrayAllocValues(Dynarray, numValues);
        return;
    }
    freeSpace = utAllocatedDynarrayValue() - utUsedDynarrayValue();
    if(freeSpace < newSize) {
        allocMoreDynarrayValues(newSize);
    }
    dataPtr = utDynarrayGetValues(Dynarray) - usedHeaderSize;
    utRecordArray(utModuleID, 11, utUsedDynarrayValue(), newSize, true);
    utRecordArray(utModuleID, 11, dataPtr - utDynarrays.Value, freeHeaderSize, true);
    memcpy((void *)(utDynarrays.Value + utUsedDynarrayValue()), dataPtr,
        elementSize*utMin(oldSize, newSize));
    if(newSize > oldSize) {
        memset(utDynarrays.Value + utUsedDynarrayValue() + oldSize, 0, ((newSize - oldSize))*sizeof(uint8));
    }
    *(utDynarray *)(void *)dataPtr = utDynarrayNull;
    *(uint32 *)(void *)(((utDynarray *)(void *)dataPtr) + 1) = oldSize;
    utRecordArray(utModuleID, 11, utUsedDynarrayValue(), newSize, false);
    utRecordArray(utModuleID, 11, dataPtr - utDynarrays.Value, freeHeaderSize, false);
    utSetFreeDynarrayValue(utFreeDynarrayValue() + oldSize);
    utDynarraySetValueIndex(Dynarray, utUsedDynarrayValue() + usedHeaderSize);
    utDynarraySetNumValue(Dynarray, numValues);
    utSetUsedDynarrayValue(utUsedDynarrayValue() + newSize);
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Dynarray.
----------------------------------------------------------------------------------------*/
void utDynarrayCopyProps(
    utDynarray oldDynarray,
    utDynarray newDynarray)
{
    utDynarraySetValueSize(newDynarray, utDynarrayGetValueSize(oldDynarray));
    utDynarraySetUsedValue(newDynarray, utDynarrayGetUsedValue(oldDynarray));
    utDynarraySetSize(newDynarray, utDynarrayGetSize(oldDynarray));
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void utShowDynarray(
    utDynarray Dynarray)
{
    utDatabaseShowObject("ut", "Dynarray", utDynarray2Index(Dynarray));
}
#endif

/*----------------------------------------------------------------------------------------
  Destroy SymArray including everything in it. Remove from parents.
----------------------------------------------------------------------------------------*/
void utSymArrayDestroy(
    utSymArray SymArray)
{
    if(utSymArrayDestructorCallback != NULL) {
        utSymArrayDestructorCallback(SymArray);
    }
    utSymArrayFree(SymArray);
}

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocSymArray(void)
{
    utSymArray SymArray = utSymArrayAlloc();

    return utSymArray2Index(SymArray);
}

/*----------------------------------------------------------------------------------------
  Destructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static void destroySymArray(
    uint64 objectIndex)
{
    utSymArrayDestroy(utIndex2SymArray((uint32)objectIndex));
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of SymArray.
----------------------------------------------------------------------------------------*/
static void allocSymArrays(void)
{
    utSetAllocatedSymArray(2);
    utSetUsedSymArray(1);
    utSetFirstFreeSymArray(utSymArrayNull);
    utSymArrays.SymIndex = utNewA(uint32, (utAllocatedSymArray()));
    utSymArrays.NumSym = utNewA(uint32, (utAllocatedSymArray()));
    utSetUsedSymArraySym(0);
    utSetAllocatedSymArraySym(2);
    utSetFreeSymArraySym(0);
    utSymArrays.Sym = utNewA(utSym, utAllocatedSymArraySym());
    utSymArrays.UsedSym = utNewA(uint32, (utAllocatedSymArray()));
    utSymArrays.FreeList = utNewA(utSymArray, (utAllocatedSymArray()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class SymArray.
----------------------------------------------------------------------------------------*/
static void reallocSymArrays(
    uint32 newSize)
{
    utRecordResize(utModuleID, 16, (utAllocatedSymArray()), true);
    utResizeArray(utSymArrays.SymIndex, (newSize));
    utRecordResize(utModuleID, 16, (newSize), false);
    utRecordResize(utModuleID, 17, (utAllocatedSymArray()), true);
    utResizeArray(utSymArrays.NumSym, (newSize));
    utRecordResize(utModuleID, 17, (newSize), false);
    utRecordResize(utModuleID, 19, (utAllocatedSymArray()), true);
    utResizeArray(utSymArrays.UsedSym, (newSize));
    utRecordResize(utModuleID, 19, (newSize), false);
    utRecordResize(utModuleID, 20, (utAllocatedSymArray()), true);
    utResizeArray(utSymArrays.FreeList, (newSize));
    utRecordResize(utModuleID, 20, (newSize), false);
    utSetAllocatedSymArray(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more SymArrays.
----------------------------------------------------------------------------------------*/
void utSymArrayAllocMore(void)
{
    reallocSymArrays((uint32)(utAllocatedSymArray() + (utAllocatedSymArray() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Compact the SymArray.Sym heap to free memory.
----------------------------------------------------------------------------------------*/
void utCompactSymArraySyms(void)
{
    uint32 elementSize = sizeof(utSym);
    uint32 usedHeaderSize = (sizeof(utSymArray) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(utSymArray) + sizeof(uint32) + elementSize - 1)/elementSize;
    utSym *toPtr = utSymArrays.Sym;
    utSym *fromPtr = toPtr;
    utSymArray SymArray;
    uint32 size;

    while(fromPtr < utSymArrays.Sym + utUsedSymArraySym()) {
        SymArray = *(utSymArray *)(void *)fromPtr;
        if(SymArray != utSymArrayNull) {
            /* Need to move it to toPtr */
            size = utMax(utSymArrayGetNumSym(SymArray) + usedHeaderSize, freeHeaderSize);
            utRecordArray(utModuleID, 18, toPtr - utSymArrays.Sym, size, true);
            memmove((void *)toPtr, (void *)fromPtr, size*elementSize);
            utRecordArray(utModuleID, 18, toPtr - utSymArrays.Sym, size, false);
            utSymArraySetSymIndex(SymArray, toPtr - utSymArrays.Sym + usedHeaderSize);
            toPtr += size;
        } else {
            /* Just skip it */
            size = *(uint32 *)(void *)(((utSymArray *)(void *)fromPtr) + 1);
        }
        fromPtr += size;
    }
    utSetUsedSymArraySym(toPtr - utSymArrays.Sym);
    utSetFreeSymArraySym(0);
}

/*----------------------------------------------------------------------------------------
  Allocate more memory for the SymArray.Sym heap.
----------------------------------------------------------------------------------------*/
static void allocMoreSymArraySyms(
    uint32 spaceNeeded)
{
    uint32 freeSpace = utAllocatedSymArraySym() - utUsedSymArraySym();

    if((utFreeSymArraySym() << 2) > utUsedSymArraySym()) {
        utCompactSymArraySyms();
        freeSpace = utAllocatedSymArraySym() - utUsedSymArraySym();
    }
    if(freeSpace < spaceNeeded) {
        utRecordResize(utModuleID, 18, utAllocatedSymArraySym(), true);
        utSetAllocatedSymArraySym(utAllocatedSymArraySym() + spaceNeeded - freeSpace +
            (utAllocatedSymArraySym() >> 1));
        utResizeArray(utSymArrays.Sym, utAllocatedSymArraySym());
        utRecordResize(utModuleID, 18, utAllocatedSymArraySym(), false);
    }
}

/*----------------------------------------------------------------------------------------
  Allocate memory for a new SymArray.Sym array.
----------------------------------------------------------------------------------------*/
void utSymArrayAllocSyms(
    utSymArray SymArray,
    uint32 numSyms)
{
    uint32 freeSpace = utAllocatedSymArraySym() - utUsedSymArraySym();
    uint32 elementSize = sizeof(utSym);
    uint32 usedHeaderSize = (sizeof(utSymArray) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(utSymArray) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 spaceNeeded = utMax(numSyms + usedHeaderSize, freeHeaderSize);

#if defined(DD_DEBUG)
    utAssert(utSymArrayGetNumSym(SymArray) == 0);
#endif
    if(numSyms == 0) {
        return;
    }
    if(freeSpace < spaceNeeded) {
        allocMoreSymArraySyms(spaceNeeded);
    }
    utSymArraySetSymIndex(SymArray, utUsedSymArraySym() + usedHeaderSize);
    utSymArraySetNumSym(SymArray, numSyms);
    utRecordArray(utModuleID, 18, utUsedSymArraySym(), numSyms + usedHeaderSize, true);
    *(utSymArray *)(void *)(utSymArrays.Sym + utUsedSymArraySym()) = SymArray;
    {
        uint32 xSymArray;
        for(xSymArray = (uint32)(utSymArrayGetSymIndex(SymArray)); xSymArray < utSymArrayGetSymIndex(SymArray) + numSyms; xSymArray++) {
            utSymArrays.Sym[xSymArray] = utSymNull;
        }
    }
    utRecordArray(utModuleID, 18, utUsedSymArraySym(), numSyms + usedHeaderSize, false);
    utSetUsedSymArraySym(utUsedSymArraySym() + spaceNeeded);
}

/*----------------------------------------------------------------------------------------
  Wrapper around utSymArrayGetSyms for the database manager.
----------------------------------------------------------------------------------------*/
static void *getSymArraySyms(
    uint64 objectNumber,
    uint32 *numValues)
{
    utSymArray SymArray = utIndex2SymArray((uint32)objectNumber);

    *numValues = utSymArrayGetNumSym(SymArray);
    return utSymArrayGetSyms(SymArray);
}

/*----------------------------------------------------------------------------------------
  Wrapper around utSymArrayAllocSyms for the database manager.
----------------------------------------------------------------------------------------*/
static void *allocSymArraySyms(
    uint64 objectNumber,
    uint32 numValues)
{
    utSymArray SymArray = utIndex2SymArray((uint32)objectNumber);

    utSymArraySetSymIndex(SymArray, 0);
    utSymArraySetNumSym(SymArray, 0);
    if(numValues == 0) {
        return NULL;
    }
    utSymArrayAllocSyms(SymArray, numValues);
    return utSymArrayGetSyms(SymArray);
}

/*----------------------------------------------------------------------------------------
  Free memory used by the SymArray.Sym array.
----------------------------------------------------------------------------------------*/
void utSymArrayFreeSyms(
    utSymArray SymArray)
{
    uint32 elementSize = sizeof(utSym);
    uint32 usedHeaderSize = (sizeof(utSymArray) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(utSymArray) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 size = utMax(utSymArrayGetNumSym(SymArray) + usedHeaderSize, freeHeaderSize);
    utSym *dataPtr = utSymArrayGetSyms(SymArray) - usedHeaderSize;

    if(utSymArrayGetNumSym(SymArray) == 0) {
        return;
    }
    utRecordArray(utModuleID, 18, dataPtr - utSymArrays.Sym, freeHeaderSize, true);
    *(utSymArray *)(void *)(dataPtr) = utSymArrayNull;
    *(uint32 *)(void *)(((utSymArray *)(void *)dataPtr) + 1) = size;
    utRecordArray(utModuleID, 18, dataPtr - utSymArrays.Sym, freeHeaderSize, false);
    utSymArraySetNumSym(SymArray, 0);
    utSetFreeSymArraySym(utFreeSymArraySym() + size);
}

/*----------------------------------------------------------------------------------------
  Resize the SymArray.Sym array.
----------------------------------------------------------------------------------------*/
void utSymArrayResizeSyms(
    utSymArray SymArray,
    uint32 numSyms)
{
    uint32 freeSpace;
    uint32 elementSize = sizeof(utSym);
    uint32 usedHeaderSize = (sizeof(utSymArray) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(utSymArray) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 newSize = utMax(numSyms + usedHeaderSize, freeHeaderSize);
    uint32 oldSize = utMax(utSymArrayGetNumSym(SymArray) + usedHeaderSize, freeHeaderSize);
    utSym *dataPtr;

    if(numSyms == 0) {
        if(utSymArrayGetNumSym(SymArray) != 0) {
            utSymArrayFreeSyms(SymArray);
        }
        return;
    }
    if(utSymArrayGetNumSym(SymArray) == 0) {
        utSymArrayAllocSyms(SymArray, numSyms);
        return;
    }
    freeSpace = utAllocatedSymArraySym() - utUsedSymArraySym();
    if(freeSpace < newSize) {
        allocMoreSymArraySyms(newSize);
    }
    dataPtr = utSymArrayGetSyms(SymArray) - usedHeaderSize;
    utRecordArray(utModuleID, 18, utUsedSymArraySym(), newSize, true);
    utRecordArray(utModuleID, 18, dataPtr - utSymArrays.Sym, freeHeaderSize, true);
    memcpy((void *)(utSymArrays.Sym + utUsedSymArraySym()), dataPtr,
        elementSize*utMin(oldSize, newSize));
    if(newSize > oldSize) {
        {
            uint32 xSymArray;
            for(xSymArray = (uint32)(utUsedSymArraySym() + oldSize); xSymArray < utUsedSymArraySym() + oldSize + newSize - oldSize; xSymArray++) {
                utSymArrays.Sym[xSymArray] = utSymNull;
            }
        }
    }
    *(utSymArray *)(void *)dataPtr = utSymArrayNull;
    *(uint32 *)(void *)(((utSymArray *)(void *)dataPtr) + 1) = oldSize;
    utRecordArray(utModuleID, 18, utUsedSymArraySym(), newSize, false);
    utRecordArray(utModuleID, 18, dataPtr - utSymArrays.Sym, freeHeaderSize, false);
    utSetFreeSymArraySym(utFreeSymArraySym() + oldSize);
    utSymArraySetSymIndex(SymArray, utUsedSymArraySym() + usedHeaderSize);
    utSymArraySetNumSym(SymArray, numSyms);
    utSetUsedSymArraySym(utUsedSymArraySym() + newSize);
}

/*----------------------------------------------------------------------------------------
  Copy the properties of SymArray.
----------------------------------------------------------------------------------------*/
void utSymArrayCopyProps(
    utSymArray oldSymArray,
    utSymArray newSymArray)
{
}

/*----------------------------------------------------------------------------------------
  Add the indexed Sym to the SymArray.
----------------------------------------------------------------------------------------*/
void utSymArrayInsertSym(
    utSymArray SymArray,
    uint32 x,
    utSym _Sym)
{
#if defined(DD_DEBUG)
    if(SymArray == utSymArrayNull) {
        utExit("Non existent SymArray");
    }
#endif
    utSymArraySetiSym(SymArray, x, _Sym);
    utSymArraySetUsedSym(SymArray, utMax(utSymArrayGetUsedSym(SymArray), x + 1));
}

/*----------------------------------------------------------------------------------------
  Add the Sym to the end of the SymArraySym array.
----------------------------------------------------------------------------------------*/
void utSymArrayAppendSym(
    utSymArray SymArray,
    utSym _Sym)
{
    uint32 usedSym = utSymArrayGetUsedSym(SymArray);

#if defined(DD_DEBUG)
    if(SymArray == utSymArrayNull) {
        utExit("Non existent SymArray");
    }
#endif
    if(usedSym >= utSymArrayGetNumSym(SymArray)) {
        utSymArrayResizeSyms(SymArray, usedSym + (usedSym << 1) + 1);
    }
    utSymArraySetiSym(SymArray, usedSym, _Sym);
    utSymArraySetUsedSym(SymArray, usedSym + 1);
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void utShowSymArray(
    utSymArray SymArray)
{
    utDatabaseShowObject("ut", "SymArray", utSymArray2Index(SymArray));
}
#endif

/*----------------------------------------------------------------------------------------
  Free memory used by the ut database.
----------------------------------------------------------------------------------------*/
void utDatabaseStop(void)
{
    utFree(utSymtabs.TableIndex);
    utFree(utSymtabs.NumTable);
    utFree(utSymtabs.Table);
    utFree(utSymtabs.NumSym);
    utFree(utSyms.NameIndex);
    utFree(utSyms.NumName);
    utFree(utSyms.Name);
    utFree(utSyms.HashValue);
    utFree(utSyms.Next);
    utFree(utDynarrays.ValueIndex);
    utFree(utDynarrays.NumValue);
    utFree(utDynarrays.Value);
    utFree(utDynarrays.ValueSize);
    utFree(utDynarrays.UsedValue);
    utFree(utDynarrays.Size);
    utFree(utDynarrays.FreeList);
    utFree(utSymArrays.SymIndex);
    utFree(utSymArrays.NumSym);
    utFree(utSymArrays.Sym);
    utFree(utSymArrays.UsedSym);
    utFree(utSymArrays.FreeList);
    utUnregisterModule(utModuleID);
}

/*----------------------------------------------------------------------------------------
  Allocate memory used by the ut database.
----------------------------------------------------------------------------------------*/
void utDatabaseStart(void)
{
    if(!utInitialized()) {
        utStart();
    }
    utRootData.hash = 0x60fafcf;
    utModuleID = utRegisterModule("ut", true, utHash(), 4, 21, 1, sizeof(struct utRootType_),
        &utRootData, utDatabaseStart, utDatabaseStop);
    utRegisterEnum("FieldType", 12);
    utRegisterEntry("UT_BIT", 0);
    utRegisterEntry("UT_BOOL", 1);
    utRegisterEntry("UT_INT", 2);
    utRegisterEntry("UT_UINT", 3);
    utRegisterEntry("UT_CHAR", 4);
    utRegisterEntry("UT_FLOAT", 5);
    utRegisterEntry("UT_DOUBLE", 6);
    utRegisterEntry("UT_POINTER", 7);
    utRegisterEntry("UT_TYPEDEF", 8);
    utRegisterEntry("UT_ENUM", 9);
    utRegisterEntry("UT_SYM", 10);
    utRegisterEntry("UT_UNION", 11);
    utRegisterClass("Symtab", 4, &utRootData.usedSymtab, &utRootData.allocatedSymtab,
        NULL, 65535, 4, allocSymtab, NULL);
    utRegisterField("TableIndex", &utSymtabs.TableIndex, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("NumTable", &utSymtabs.NumTable, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("Table", &utSymtabs.Table, sizeof(utSym), UT_SYM, NULL);
    utRegisterArray(&utRootData.usedSymtabTable, &utRootData.allocatedSymtabTable,
        getSymtabTables, allocSymtabTables, utCompactSymtabTables);
    utRegisterField("NumSym", &utSymtabs.NumSym, sizeof(uint32), UT_UINT, NULL);
    utRegisterClass("Sym", 5, &utRootData.usedSym, &utRootData.allocatedSym,
        NULL, 65535, 4, allocSym, NULL);
    utRegisterField("NameIndex", &utSyms.NameIndex, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("NumName", &utSyms.NumName, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("Name", &utSyms.Name, sizeof(char), UT_CHAR, NULL);
    utRegisterArray(&utRootData.usedSymName, &utRootData.allocatedSymName,
        getSymNames, allocSymNames, utCompactSymNames);
    utRegisterField("HashValue", &utSyms.HashValue, sizeof(uint32), UT_UINT, NULL);
    utRegisterField("Next", &utSyms.Next, sizeof(utSym), UT_POINTER, "Sym");
    utRegisterClass("Dynarray", 7, &utRootData.usedDynarray, &utRootData.allocatedDynarray,
        &utRootData.firstFreeDynarray, 15, 4, allocDynarray, destroyDynarray);
    utRegisterField("ValueIndex", &utDynarrays.ValueIndex, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("NumValue", &utDynarrays.NumValue, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("Value", &utDynarrays.Value, sizeof(uint8), UT_UINT, NULL);
    utRegisterArray(&utRootData.usedDynarrayValue, &utRootData.allocatedDynarrayValue,
        getDynarrayValues, allocDynarrayValues, utCompactDynarrayValues);
    utRegisterField("ValueSize", &utDynarrays.ValueSize, sizeof(uint16), UT_UINT, NULL);
    utRegisterField("UsedValue", &utDynarrays.UsedValue, sizeof(uint32), UT_UINT, NULL);
    utRegisterField("Size", &utDynarrays.Size, sizeof(uint32), UT_UINT, NULL);
    utRegisterField("FreeList", &utDynarrays.FreeList, sizeof(utDynarray), UT_POINTER, "Dynarray");
    utSetFieldHidden();
    utRegisterClass("SymArray", 5, &utRootData.usedSymArray, &utRootData.allocatedSymArray,
        &utRootData.firstFreeSymArray, 20, 4, allocSymArray, destroySymArray);
    utRegisterField("SymIndex", &utSymArrays.SymIndex, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("NumSym", &utSymArrays.NumSym, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("Sym", &utSymArrays.Sym, sizeof(utSym), UT_POINTER, "Sym");
    utRegisterArray(&utRootData.usedSymArraySym, &utRootData.allocatedSymArraySym,
        getSymArraySyms, allocSymArraySyms, utCompactSymArraySyms);
    utRegisterField("UsedSym", &utSymArrays.UsedSym, sizeof(uint32), UT_UINT, NULL);
    utRegisterField("FreeList", &utSymArrays.FreeList, sizeof(utSymArray), UT_POINTER, "SymArray");
    utSetFieldHidden();
    allocSymtabs();
    allocSyms();
    allocDynarrays();
    allocSymArrays();
}

