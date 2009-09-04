/*----------------------------------------------------------------------------------------
  Module header file for: ut module
----------------------------------------------------------------------------------------*/
#ifndef UTDATABASE_H

#define UTDATABASE_H

#if defined __cplusplus
extern "C" {
#endif

#ifndef DD_UTIL_H
#include "ddutil.h"
#endif

extern uint8 utModuleID;
/* Class reference definitions */
#if (defined(DD_DEBUG) && !defined(DD_NOSTRICT)) || defined(DD_STRICT)
typedef struct _struct_utSymtab{char val;} *utSymtab;
#define utSymtabNull ((utSymtab)0)
typedef struct _struct_utSym{char val;} *utSym;
#define utSymNull ((utSym)0)
typedef struct _struct_utDynarray{char val;} *utDynarray;
#define utDynarrayNull ((utDynarray)0)
typedef struct _struct_utSymArray{char val;} *utSymArray;
#define utSymArrayNull ((utSymArray)0)
#else
typedef uint32 utSymtab;
#define utSymtabNull 0
typedef uint32 utSym;
#define utSymNull 0
typedef uint32 utDynarray;
#define utDynarrayNull 0
typedef uint32 utSymArray;
#define utSymArrayNull 0
#endif

/* FieldType enumerated type */
typedef enum {
    UT_BIT = 0,
    UT_BOOL = 1,
    UT_INT = 2,
    UT_UINT = 3,
    UT_CHAR = 4,
    UT_FLOAT = 5,
    UT_DOUBLE = 6,
    UT_POINTER = 7,
    UT_TYPEDEF = 8,
    UT_ENUM = 9,
    UT_SYM = 10,
    UT_UNION = 11
} utFieldType;

/* Constructor/Destructor hooks. */
typedef void (*utSymtabCallbackType)(utSymtab);
extern utSymtabCallbackType utSymtabConstructorCallback;
typedef void (*utSymCallbackType)(utSym);
extern utSymCallbackType utSymConstructorCallback;
typedef void (*utDynarrayCallbackType)(utDynarray);
extern utDynarrayCallbackType utDynarrayConstructorCallback;
extern utDynarrayCallbackType utDynarrayDestructorCallback;
typedef void (*utSymArrayCallbackType)(utSymArray);
extern utSymArrayCallbackType utSymArrayConstructorCallback;
extern utSymArrayCallbackType utSymArrayDestructorCallback;

/*----------------------------------------------------------------------------------------
  Root structure
----------------------------------------------------------------------------------------*/
struct utRootType_ {
    uint32 hash; /* This depends only on the structure of the database */
    uint32 usedSymtab, allocatedSymtab;
    uint32 usedSymtabTable, allocatedSymtabTable, freeSymtabTable;
    uint32 usedSym, allocatedSym;
    uint32 usedSymName, allocatedSymName, freeSymName;
    utDynarray firstFreeDynarray;
    uint32 usedDynarray, allocatedDynarray;
    uint32 usedDynarrayValue, allocatedDynarrayValue, freeDynarrayValue;
    utSymArray firstFreeSymArray;
    uint32 usedSymArray, allocatedSymArray;
    uint32 usedSymArraySym, allocatedSymArraySym, freeSymArraySym;
};
extern struct utRootType_ utRootData;

utInlineC uint32 utHash(void) {return utRootData.hash;}
utInlineC uint32 utUsedSymtab(void) {return utRootData.usedSymtab;}
utInlineC uint32 utAllocatedSymtab(void) {return utRootData.allocatedSymtab;}
utInlineC void utSetUsedSymtab(uint32 value) {utRootData.usedSymtab = value;
    utRecordGlobal(utModuleID, 4, &utRootData.usedSymtab, false);}
utInlineC void utSetAllocatedSymtab(uint32 value) {utRootData.allocatedSymtab = value;
    utRecordGlobal(utModuleID, 4, &utRootData.allocatedSymtab, false);}
utInlineC uint32 utUsedSymtabTable(void) {return utRootData.usedSymtabTable;}
utInlineC uint32 utAllocatedSymtabTable(void) {return utRootData.allocatedSymtabTable;}
utInlineC uint32 utFreeSymtabTable(void) {return utRootData.freeSymtabTable;}
utInlineC void utSetUsedSymtabTable(uint32 value) {utRootData.usedSymtabTable = value;
    utRecordGlobal(utModuleID, sizeof(uint32), &utRootData.usedSymtabTable, false);}
utInlineC void utSetAllocatedSymtabTable(uint32 value) {utRootData.allocatedSymtabTable = value;
    utRecordGlobal(utModuleID, sizeof(uint32), &utRootData.allocatedSymtabTable, false);}
utInlineC void utSetFreeSymtabTable(int32 value) {utRootData.freeSymtabTable = value;
    utRecordGlobal(utModuleID, sizeof(uint32), &utRootData.freeSymtabTable, false);}
utInlineC uint32 utUsedSym(void) {return utRootData.usedSym;}
utInlineC uint32 utAllocatedSym(void) {return utRootData.allocatedSym;}
utInlineC void utSetUsedSym(uint32 value) {utRootData.usedSym = value;
    utRecordGlobal(utModuleID, 4, &utRootData.usedSym, false);}
utInlineC void utSetAllocatedSym(uint32 value) {utRootData.allocatedSym = value;
    utRecordGlobal(utModuleID, 4, &utRootData.allocatedSym, false);}
utInlineC uint32 utUsedSymName(void) {return utRootData.usedSymName;}
utInlineC uint32 utAllocatedSymName(void) {return utRootData.allocatedSymName;}
utInlineC uint32 utFreeSymName(void) {return utRootData.freeSymName;}
utInlineC void utSetUsedSymName(uint32 value) {utRootData.usedSymName = value;
    utRecordGlobal(utModuleID, sizeof(uint32), &utRootData.usedSymName, false);}
utInlineC void utSetAllocatedSymName(uint32 value) {utRootData.allocatedSymName = value;
    utRecordGlobal(utModuleID, sizeof(uint32), &utRootData.allocatedSymName, false);}
utInlineC void utSetFreeSymName(int32 value) {utRootData.freeSymName = value;
    utRecordGlobal(utModuleID, sizeof(uint32), &utRootData.freeSymName, false);}
utInlineC utDynarray utFirstFreeDynarray(void) {return utRootData.firstFreeDynarray;}
utInlineC void utSetFirstFreeDynarray(utDynarray value) {utRootData.firstFreeDynarray = (value);
    utRecordGlobal(utModuleID, 4, &utRootData.firstFreeDynarray, false);}
utInlineC uint32 utUsedDynarray(void) {return utRootData.usedDynarray;}
utInlineC uint32 utAllocatedDynarray(void) {return utRootData.allocatedDynarray;}
utInlineC void utSetUsedDynarray(uint32 value) {utRootData.usedDynarray = value;
    utRecordGlobal(utModuleID, 4, &utRootData.usedDynarray, false);}
utInlineC void utSetAllocatedDynarray(uint32 value) {utRootData.allocatedDynarray = value;
    utRecordGlobal(utModuleID, 4, &utRootData.allocatedDynarray, false);}
utInlineC uint32 utUsedDynarrayValue(void) {return utRootData.usedDynarrayValue;}
utInlineC uint32 utAllocatedDynarrayValue(void) {return utRootData.allocatedDynarrayValue;}
utInlineC uint32 utFreeDynarrayValue(void) {return utRootData.freeDynarrayValue;}
utInlineC void utSetUsedDynarrayValue(uint32 value) {utRootData.usedDynarrayValue = value;
    utRecordGlobal(utModuleID, sizeof(uint32), &utRootData.usedDynarrayValue, false);}
utInlineC void utSetAllocatedDynarrayValue(uint32 value) {utRootData.allocatedDynarrayValue = value;
    utRecordGlobal(utModuleID, sizeof(uint32), &utRootData.allocatedDynarrayValue, false);}
utInlineC void utSetFreeDynarrayValue(int32 value) {utRootData.freeDynarrayValue = value;
    utRecordGlobal(utModuleID, sizeof(uint32), &utRootData.freeDynarrayValue, false);}
utInlineC utSymArray utFirstFreeSymArray(void) {return utRootData.firstFreeSymArray;}
utInlineC void utSetFirstFreeSymArray(utSymArray value) {utRootData.firstFreeSymArray = (value);
    utRecordGlobal(utModuleID, 4, &utRootData.firstFreeSymArray, false);}
utInlineC uint32 utUsedSymArray(void) {return utRootData.usedSymArray;}
utInlineC uint32 utAllocatedSymArray(void) {return utRootData.allocatedSymArray;}
utInlineC void utSetUsedSymArray(uint32 value) {utRootData.usedSymArray = value;
    utRecordGlobal(utModuleID, 4, &utRootData.usedSymArray, false);}
utInlineC void utSetAllocatedSymArray(uint32 value) {utRootData.allocatedSymArray = value;
    utRecordGlobal(utModuleID, 4, &utRootData.allocatedSymArray, false);}
utInlineC uint32 utUsedSymArraySym(void) {return utRootData.usedSymArraySym;}
utInlineC uint32 utAllocatedSymArraySym(void) {return utRootData.allocatedSymArraySym;}
utInlineC uint32 utFreeSymArraySym(void) {return utRootData.freeSymArraySym;}
utInlineC void utSetUsedSymArraySym(uint32 value) {utRootData.usedSymArraySym = value;
    utRecordGlobal(utModuleID, sizeof(uint32), &utRootData.usedSymArraySym, false);}
utInlineC void utSetAllocatedSymArraySym(uint32 value) {utRootData.allocatedSymArraySym = value;
    utRecordGlobal(utModuleID, sizeof(uint32), &utRootData.allocatedSymArraySym, false);}
utInlineC void utSetFreeSymArraySym(int32 value) {utRootData.freeSymArraySym = value;
    utRecordGlobal(utModuleID, sizeof(uint32), &utRootData.freeSymArraySym, false);}

/* Validate macros */
#if defined(DD_DEBUG)
utInlineC utSymtab utValidSymtab(utSymtab Symtab) {
    utAssert(utLikely(Symtab != utSymtabNull && (uint32)(Symtab - (utSymtab)0) < utRootData.usedSymtab));
    return Symtab;}
utInlineC utSym utValidSym(utSym Sym) {
    utAssert(utLikely(Sym != utSymNull && (uint32)(Sym - (utSym)0) < utRootData.usedSym));
    return Sym;}
utInlineC utDynarray utValidDynarray(utDynarray Dynarray) {
    utAssert(utLikely(Dynarray != utDynarrayNull && (uint32)(Dynarray - (utDynarray)0) < utRootData.usedDynarray));
    return Dynarray;}
utInlineC utSymArray utValidSymArray(utSymArray SymArray) {
    utAssert(utLikely(SymArray != utSymArrayNull && (uint32)(SymArray - (utSymArray)0) < utRootData.usedSymArray));
    return SymArray;}
#else
utInlineC utSymtab utValidSymtab(utSymtab Symtab) {return Symtab;}
utInlineC utSym utValidSym(utSym Sym) {return Sym;}
utInlineC utDynarray utValidDynarray(utDynarray Dynarray) {return Dynarray;}
utInlineC utSymArray utValidSymArray(utSymArray SymArray) {return SymArray;}
#endif

/* Object ref to integer conversions */
#if (defined(DD_DEBUG) && !defined(DD_NOSTRICT)) || defined(DD_STRICT)
utInlineC uint32 utSymtab2Index(utSymtab Symtab) {return Symtab - (utSymtab)0;}
utInlineC uint32 utSymtab2ValidIndex(utSymtab Symtab) {return utValidSymtab(Symtab) - (utSymtab)0;}
utInlineC utSymtab utIndex2Symtab(uint32 xSymtab) {return (utSymtab)(xSymtab + (utSymtab)(0));}
utInlineC uint32 utSym2Index(utSym Sym) {return Sym - (utSym)0;}
utInlineC uint32 utSym2ValidIndex(utSym Sym) {return utValidSym(Sym) - (utSym)0;}
utInlineC utSym utIndex2Sym(uint32 xSym) {return (utSym)(xSym + (utSym)(0));}
utInlineC uint32 utDynarray2Index(utDynarray Dynarray) {return Dynarray - (utDynarray)0;}
utInlineC uint32 utDynarray2ValidIndex(utDynarray Dynarray) {return utValidDynarray(Dynarray) - (utDynarray)0;}
utInlineC utDynarray utIndex2Dynarray(uint32 xDynarray) {return (utDynarray)(xDynarray + (utDynarray)(0));}
utInlineC uint32 utSymArray2Index(utSymArray SymArray) {return SymArray - (utSymArray)0;}
utInlineC uint32 utSymArray2ValidIndex(utSymArray SymArray) {return utValidSymArray(SymArray) - (utSymArray)0;}
utInlineC utSymArray utIndex2SymArray(uint32 xSymArray) {return (utSymArray)(xSymArray + (utSymArray)(0));}
#else
utInlineC uint32 utSymtab2Index(utSymtab Symtab) {return Symtab;}
utInlineC uint32 utSymtab2ValidIndex(utSymtab Symtab) {return utValidSymtab(Symtab);}
utInlineC utSymtab utIndex2Symtab(uint32 xSymtab) {return xSymtab;}
utInlineC uint32 utSym2Index(utSym Sym) {return Sym;}
utInlineC uint32 utSym2ValidIndex(utSym Sym) {return utValidSym(Sym);}
utInlineC utSym utIndex2Sym(uint32 xSym) {return xSym;}
utInlineC uint32 utDynarray2Index(utDynarray Dynarray) {return Dynarray;}
utInlineC uint32 utDynarray2ValidIndex(utDynarray Dynarray) {return utValidDynarray(Dynarray);}
utInlineC utDynarray utIndex2Dynarray(uint32 xDynarray) {return xDynarray;}
utInlineC uint32 utSymArray2Index(utSymArray SymArray) {return SymArray;}
utInlineC uint32 utSymArray2ValidIndex(utSymArray SymArray) {return utValidSymArray(SymArray);}
utInlineC utSymArray utIndex2SymArray(uint32 xSymArray) {return xSymArray;}
#endif

/*----------------------------------------------------------------------------------------
  Fields for class Symtab.
----------------------------------------------------------------------------------------*/
struct utSymtabFields {
    uint32 *TableIndex;
    uint32 *NumTable;
    utSym *Table;
    uint32 *NumSym;
};
extern struct utSymtabFields utSymtabs;

void utSymtabAllocMore(void);
void utSymtabCopyProps(utSymtab utOldSymtab, utSymtab utNewSymtab);
void utSymtabAllocTables(utSymtab Symtab, uint32 numTables);
void utSymtabResizeTables(utSymtab Symtab, uint32 numTables);
void utSymtabFreeTables(utSymtab Symtab);
void utCompactSymtabTables(void);
utInlineC uint32 utSymtabGetTableIndex(utSymtab Symtab) {return utSymtabs.TableIndex[utSymtab2ValidIndex(Symtab)];}
utInlineC void utSymtabSetTableIndex(utSymtab Symtab, uint32 value) {utSymtabs.TableIndex[utSymtab2ValidIndex(Symtab)] = value;
    utRecordField(utModuleID, 0, utSymtab2ValidIndex(Symtab), false);}
utInlineC uint32 utSymtabGetNumTable(utSymtab Symtab) {return utSymtabs.NumTable[utSymtab2ValidIndex(Symtab)];}
utInlineC void utSymtabSetNumTable(utSymtab Symtab, uint32 value) {utSymtabs.NumTable[utSymtab2ValidIndex(Symtab)] = value;
    utRecordField(utModuleID, 1, utSymtab2ValidIndex(Symtab), false);}
#if defined(DD_DEBUG)
utInlineC uint32 utSymtabCheckTableIndex(utSymtab Symtab, uint32 x) {utAssert(x < utSymtabGetNumTable(Symtab)); return x;}
#else
utInlineC uint32 utSymtabCheckTableIndex(utSymtab Symtab, uint32 x) {return x;}
#endif
utInlineC utSym utSymtabGetiTable(utSymtab Symtab, uint32 x) {return utSymtabs.Table[
    utSymtabGetTableIndex(Symtab) + utSymtabCheckTableIndex(Symtab, x)];}
utInlineC utSym *utSymtabGetTable(utSymtab Symtab) {return utSymtabs.Table + utSymtabGetTableIndex(Symtab);}
#define utSymtabGetTables utSymtabGetTable
utInlineC void utSymtabSetTable(utSymtab Symtab, utSym *valuePtr, uint32 numTable) {
    utSymtabResizeTables(Symtab, numTable);
    memcpy(utSymtabGetTables(Symtab), valuePtr, numTable*sizeof(utSym));
    utRecordArray(utModuleID, 2, utSymtabGetTableIndex(Symtab), utSymtabGetNumTable(Symtab), false);
}
utInlineC void utSymtabSetiTable(utSymtab Symtab, uint32 x, utSym value) {
    utSymtabs.Table[utSymtabGetTableIndex(Symtab) + utSymtabCheckTableIndex(Symtab, (x))] = value;
    utRecordField(utModuleID, 2, utSymtabGetTableIndex(Symtab) + (x), false);}
utInlineC void utSymtabMoveTables(utSymtab Symtab, uint32 from, uint32 to, uint32 count) {
    utAssert((to+count) <= utSymtabGetNumTable(Symtab));
    utAssert((from+count) <= utSymtabGetNumTable(Symtab));
    memmove(utSymtabGetTables(Symtab)+to,utSymtabGetTables(Symtab)+from,((int32)count)*sizeof(utSym));
}
utInlineC void utSymtabCopyTables(utSymtab Symtab, uint32 x, utSym * values, uint32 count) {
    utAssert((x+count) <= utSymtabGetNumTable(Symtab));
    memcpy(utSymtabGetTables(Symtab)+x, values, count*sizeof(utSym));
}
utInlineC void utSymtabAppendTables(utSymtab Symtab, utSym * values, uint32 count) {
    uint32 num = utSymtabGetNumTable(Symtab);
    utSymtabResizeTables(Symtab, num+count);
    utSymtabCopyTables(Symtab, num, values, count);
}
utInlineC void utSymtabAppendTable(utSymtab Symtab, utSym Table) {
    utSymtabResizeTables(Symtab, utSymtabGetNumTable(Symtab)+1);
    utSymtabSetiTable(Symtab, utSymtabGetNumTable(Symtab)-1, Table);
}
utInlineC void utSymtabInsertTables(utSymtab Symtab, uint32 x, utSym *Table, uint32 count) {
    utAssert(x <= utSymtabGetNumTable(Symtab));
    if(x < utSymtabGetNumTable(Symtab)) {
        utSymtabResizeTables(Symtab, utSymtabGetNumTable(Symtab)+count);
        utSymtabMoveTables(Symtab, x, x+count, utSymtabGetNumTable(Symtab)-x-count);
        utSymtabCopyTables(Symtab, x, Table, count);
    }
    else {
        utSymtabAppendTables(Symtab, Table, count);
    }
}
utInlineC void utSymtabInsertTable(utSymtab Symtab, uint32 x, utSym Table) {
    utSymtabInsertTables(Symtab, x, &Table, 1);
}
utInlineC void utSymtabRemoveTables(utSymtab Symtab, uint32 x, uint32 count) {
    utAssert((x+count) <= utSymtabGetNumTable(Symtab));
    if((x+count) < utSymtabGetNumTable(Symtab)) {
        utSymtabMoveTables(Symtab, x+count,x,utSymtabGetNumTable(Symtab)-x-count);
    }
    utSymtabResizeTables(Symtab, utSymtabGetNumTable(Symtab)-(int32)count);
}
utInlineC void utSymtabRemoveTable(utSymtab Symtab, uint32 x) {
    utSymtabRemoveTables(Symtab, x, 1);
}
utInlineC void utSymtabSwapTable(utSymtab Symtab, uint32 from, uint32 to) {
    utAssert(from <= utSymtabGetNumTable(Symtab));
    utAssert(to <= utSymtabGetNumTable(Symtab));
    utSym tmp = utSymtabGetiTable(Symtab, from);
    utSymtabSetiTable(Symtab, from, utSymtabGetiTable(Symtab, to));
    utSymtabSetiTable(Symtab, to, tmp);
}
utInlineC void utSymtabSwapTables(utSymtab Symtab, uint32 from, uint32 to, uint32 count) {
    utAssert((from+count) < utSymtabGetNumTable(Symtab));
    utAssert((to+count) < utSymtabGetNumTable(Symtab));
    utSym tmp[count];
    memcpy(tmp, utSymtabGetTables(Symtab)+from, count*sizeof(utSym));
    memcpy(utSymtabGetTables(Symtab)+from, utSymtabGetTables(Symtab)+to, count*sizeof(utSym));
    memcpy(utSymtabGetTables(Symtab)+to, tmp, count*sizeof(utSym));
}
#define utForeachSymtabTable(pVar, cVar) { \
    uint32 _xTable; \
    for(_xTable = 0; _xTable < utSymtabGetNumTable(pVar); _xTable++) { \
        cVar = utSymtabGetiTable(pVar, _xTable);
#define utEndSymtabTable }}
utInlineC uint32 utSymtabGetNumSym(utSymtab Symtab) {return utSymtabs.NumSym[utSymtab2ValidIndex(Symtab)];}
utInlineC void utSymtabSetNumSym(utSymtab Symtab, uint32 value) {utSymtabs.NumSym[utSymtab2ValidIndex(Symtab)] = value;
    utRecordField(utModuleID, 3, utSymtab2ValidIndex(Symtab), false);}
utInlineC void utSymtabSetConstructorCallback(void(*func)(utSymtab)) {utSymtabConstructorCallback = func;}
utInlineC utSymtabCallbackType utSymtabGetConstructorCallback(void) {return utSymtabConstructorCallback;}
utInlineC utSymtab utFirstSymtab(void) {return utRootData.usedSymtab == 1? utSymtabNull : utIndex2Symtab(1);}
utInlineC utSymtab utLastSymtab(void) {return utRootData.usedSymtab == 1? utSymtabNull :
    utIndex2Symtab(utRootData.usedSymtab - 1);}
utInlineC utSymtab utNextSymtab(utSymtab Symtab) {return utSymtab2ValidIndex(Symtab) + 1 == utRootData.usedSymtab? utSymtabNull :
    Symtab + 1;}
utInlineC utSymtab utPrevSymtab(utSymtab Symtab) {return utSymtab2ValidIndex(Symtab) == 1? utSymtabNull : Symtab - 1;}
#define utForeachSymtab(var) \
    for(var = utIndex2Symtab(1); utSymtab2Index(var) != utRootData.usedSymtab; var++)
#define utEndSymtab
utInlineC void utSymtabFreeAll(void) {utSetUsedSymtab(1); utSetUsedSymtabTable(0);}
utInlineC utSymtab utSymtabAllocRaw(void) {
    utSymtab Symtab;
    if(utRootData.usedSymtab == utRootData.allocatedSymtab) {
        utSymtabAllocMore();
    }
    Symtab = utIndex2Symtab(utRootData.usedSymtab);
    utSetUsedSymtab(utUsedSymtab() + 1);
    return Symtab;}
utInlineC utSymtab utSymtabAlloc(void) {
    utSymtab Symtab = utSymtabAllocRaw();
    utSymtabSetTableIndex(Symtab, 0);
    utSymtabSetNumTable(Symtab, 0);
    utSymtabSetNumTable(Symtab, 0);
    utSymtabSetNumSym(Symtab, 0);
    if(utSymtabConstructorCallback != NULL) {
        utSymtabConstructorCallback(Symtab);
    }
    return Symtab;}

/*----------------------------------------------------------------------------------------
  Fields for class Sym.
----------------------------------------------------------------------------------------*/
struct utSymFields {
    uint32 *NameIndex;
    uint32 *NumName;
    char *Name;
    uint32 *HashValue;
    utSym *Next;
};
extern struct utSymFields utSyms;

void utSymAllocMore(void);
void utSymCopyProps(utSym utOldSym, utSym utNewSym);
void utSymAllocNames(utSym Sym, uint32 numNames);
void utSymResizeNames(utSym Sym, uint32 numNames);
void utSymFreeNames(utSym Sym);
void utCompactSymNames(void);
utInlineC uint32 utSymGetNameIndex(utSym Sym) {return utSyms.NameIndex[utSym2ValidIndex(Sym)];}
utInlineC void utSymSetNameIndex(utSym Sym, uint32 value) {utSyms.NameIndex[utSym2ValidIndex(Sym)] = value;
    utRecordField(utModuleID, 4, utSym2ValidIndex(Sym), false);}
utInlineC uint32 utSymGetNumName(utSym Sym) {return utSyms.NumName[utSym2ValidIndex(Sym)];}
utInlineC void utSymSetNumName(utSym Sym, uint32 value) {utSyms.NumName[utSym2ValidIndex(Sym)] = value;
    utRecordField(utModuleID, 5, utSym2ValidIndex(Sym), false);}
#if defined(DD_DEBUG)
utInlineC uint32 utSymCheckNameIndex(utSym Sym, uint32 x) {utAssert(x < utSymGetNumName(Sym)); return x;}
#else
utInlineC uint32 utSymCheckNameIndex(utSym Sym, uint32 x) {return x;}
#endif
utInlineC char utSymGetiName(utSym Sym, uint32 x) {return utSyms.Name[
    utSymGetNameIndex(Sym) + utSymCheckNameIndex(Sym, x)];}
utInlineC char *utSymGetName(utSym Sym) {return utSyms.Name + utSymGetNameIndex(Sym);}
#define utSymGetNames utSymGetName
utInlineC void utSymSetName(utSym Sym, char *valuePtr, uint32 numName) {
    utSymResizeNames(Sym, numName);
    memcpy(utSymGetNames(Sym), valuePtr, numName*sizeof(char));
    utRecordArray(utModuleID, 6, utSymGetNameIndex(Sym), utSymGetNumName(Sym), false);
}
utInlineC void utSymSetiName(utSym Sym, uint32 x, char value) {
    utSyms.Name[utSymGetNameIndex(Sym) + utSymCheckNameIndex(Sym, (x))] = value;
    utRecordField(utModuleID, 6, utSymGetNameIndex(Sym) + (x), false);}
utInlineC void utSymMoveNames(utSym Sym, uint32 from, uint32 to, uint32 count) {
    utAssert((to+count) <= utSymGetNumName(Sym));
    utAssert((from+count) <= utSymGetNumName(Sym));
    memmove(utSymGetNames(Sym)+to,utSymGetNames(Sym)+from,((int32)count)*sizeof(char));
}
utInlineC void utSymCopyNames(utSym Sym, uint32 x, char * values, uint32 count) {
    utAssert((x+count) <= utSymGetNumName(Sym));
    memcpy(utSymGetNames(Sym)+x, values, count*sizeof(char));
}
utInlineC void utSymAppendNames(utSym Sym, char * values, uint32 count) {
    uint32 num = utSymGetNumName(Sym);
    utSymResizeNames(Sym, num+count);
    utSymCopyNames(Sym, num, values, count);
}
utInlineC void utSymAppendName(utSym Sym, char Name) {
    utSymResizeNames(Sym, utSymGetNumName(Sym)+1);
    utSymSetiName(Sym, utSymGetNumName(Sym)-1, Name);
}
utInlineC void utSymInsertNames(utSym Sym, uint32 x, char *Name, uint32 count) {
    utAssert(x <= utSymGetNumName(Sym));
    if(x < utSymGetNumName(Sym)) {
        utSymResizeNames(Sym, utSymGetNumName(Sym)+count);
        utSymMoveNames(Sym, x, x+count, utSymGetNumName(Sym)-x-count);
        utSymCopyNames(Sym, x, Name, count);
    }
    else {
        utSymAppendNames(Sym, Name, count);
    }
}
utInlineC void utSymInsertName(utSym Sym, uint32 x, char Name) {
    utSymInsertNames(Sym, x, &Name, 1);
}
utInlineC void utSymRemoveNames(utSym Sym, uint32 x, uint32 count) {
    utAssert((x+count) <= utSymGetNumName(Sym));
    if((x+count) < utSymGetNumName(Sym)) {
        utSymMoveNames(Sym, x+count,x,utSymGetNumName(Sym)-x-count);
    }
    utSymResizeNames(Sym, utSymGetNumName(Sym)-(int32)count);
}
utInlineC void utSymRemoveName(utSym Sym, uint32 x) {
    utSymRemoveNames(Sym, x, 1);
}
utInlineC void utSymSwapName(utSym Sym, uint32 from, uint32 to) {
    utAssert(from <= utSymGetNumName(Sym));
    utAssert(to <= utSymGetNumName(Sym));
    char tmp = utSymGetiName(Sym, from);
    utSymSetiName(Sym, from, utSymGetiName(Sym, to));
    utSymSetiName(Sym, to, tmp);
}
utInlineC void utSymSwapNames(utSym Sym, uint32 from, uint32 to, uint32 count) {
    utAssert((from+count) < utSymGetNumName(Sym));
    utAssert((to+count) < utSymGetNumName(Sym));
    char tmp[count];
    memcpy(tmp, utSymGetNames(Sym)+from, count*sizeof(char));
    memcpy(utSymGetNames(Sym)+from, utSymGetNames(Sym)+to, count*sizeof(char));
    memcpy(utSymGetNames(Sym)+to, tmp, count*sizeof(char));
}
#define utForeachSymName(pVar, cVar) { \
    uint32 _xName; \
    for(_xName = 0; _xName < utSymGetNumName(pVar); _xName++) { \
        cVar = utSymGetiName(pVar, _xName);
#define utEndSymName }}
utInlineC uint32 utSymGetHashValue(utSym Sym) {return utSyms.HashValue[utSym2ValidIndex(Sym)];}
utInlineC void utSymSetHashValue(utSym Sym, uint32 value) {utSyms.HashValue[utSym2ValidIndex(Sym)] = value;
    utRecordField(utModuleID, 7, utSym2ValidIndex(Sym), false);}
utInlineC utSym utSymGetNext(utSym Sym) {return utSyms.Next[utSym2ValidIndex(Sym)];}
utInlineC void utSymSetNext(utSym Sym, utSym value) {utSyms.Next[utSym2ValidIndex(Sym)] = value;
    utRecordField(utModuleID, 8, utSym2ValidIndex(Sym), false);}
utInlineC void utSymSetConstructorCallback(void(*func)(utSym)) {utSymConstructorCallback = func;}
utInlineC utSymCallbackType utSymGetConstructorCallback(void) {return utSymConstructorCallback;}
utInlineC utSym utFirstSym(void) {return utRootData.usedSym == 1? utSymNull : utIndex2Sym(1);}
utInlineC utSym utLastSym(void) {return utRootData.usedSym == 1? utSymNull :
    utIndex2Sym(utRootData.usedSym - 1);}
utInlineC utSym utNextSym(utSym Sym) {return utSym2ValidIndex(Sym) + 1 == utRootData.usedSym? utSymNull :
    Sym + 1;}
utInlineC utSym utPrevSym(utSym Sym) {return utSym2ValidIndex(Sym) == 1? utSymNull : Sym - 1;}
#define utForeachSym(var) \
    for(var = utIndex2Sym(1); utSym2Index(var) != utRootData.usedSym; var++)
#define utEndSym
utInlineC void utSymFreeAll(void) {utSetUsedSym(1); utSetUsedSymName(0);}
utInlineC utSym utSymAllocRaw(void) {
    utSym Sym;
    if(utRootData.usedSym == utRootData.allocatedSym) {
        utSymAllocMore();
    }
    Sym = utIndex2Sym(utRootData.usedSym);
    utSetUsedSym(utUsedSym() + 1);
    return Sym;}
utInlineC utSym utSymAlloc(void) {
    utSym Sym = utSymAllocRaw();
    utSymSetNameIndex(Sym, 0);
    utSymSetNumName(Sym, 0);
    utSymSetNumName(Sym, 0);
    utSymSetHashValue(Sym, 0);
    utSymSetNext(Sym, utSymNull);
    if(utSymConstructorCallback != NULL) {
        utSymConstructorCallback(Sym);
    }
    return Sym;}

/*----------------------------------------------------------------------------------------
  Fields for class Dynarray.
----------------------------------------------------------------------------------------*/
struct utDynarrayFields {
    uint32 *ValueIndex;
    uint32 *NumValue;
    uint8 *Value;
    uint16 *ValueSize;
    uint32 *UsedValue;
    uint32 *Size;
    utDynarray *FreeList;
};
extern struct utDynarrayFields utDynarrays;

void utDynarrayAllocMore(void);
void utDynarrayCopyProps(utDynarray utOldDynarray, utDynarray utNewDynarray);
void utDynarrayAllocValues(utDynarray Dynarray, uint32 numValues);
void utDynarrayResizeValues(utDynarray Dynarray, uint32 numValues);
void utDynarrayFreeValues(utDynarray Dynarray);
void utCompactDynarrayValues(void);
utInlineC uint32 utDynarrayGetValueIndex(utDynarray Dynarray) {return utDynarrays.ValueIndex[utDynarray2ValidIndex(Dynarray)];}
utInlineC void utDynarraySetValueIndex(utDynarray Dynarray, uint32 value) {utDynarrays.ValueIndex[utDynarray2ValidIndex(Dynarray)] = value;
    utRecordField(utModuleID, 9, utDynarray2ValidIndex(Dynarray), false);}
utInlineC uint32 utDynarrayGetNumValue(utDynarray Dynarray) {return utDynarrays.NumValue[utDynarray2ValidIndex(Dynarray)];}
utInlineC void utDynarraySetNumValue(utDynarray Dynarray, uint32 value) {utDynarrays.NumValue[utDynarray2ValidIndex(Dynarray)] = value;
    utRecordField(utModuleID, 10, utDynarray2ValidIndex(Dynarray), false);}
#if defined(DD_DEBUG)
utInlineC uint32 utDynarrayCheckValueIndex(utDynarray Dynarray, uint32 x) {utAssert(x < utDynarrayGetNumValue(Dynarray)); return x;}
#else
utInlineC uint32 utDynarrayCheckValueIndex(utDynarray Dynarray, uint32 x) {return x;}
#endif
utInlineC uint8 utDynarrayGetiValue(utDynarray Dynarray, uint32 x) {return utDynarrays.Value[
    utDynarrayGetValueIndex(Dynarray) + utDynarrayCheckValueIndex(Dynarray, x)];}
utInlineC uint8 *utDynarrayGetValue(utDynarray Dynarray) {return utDynarrays.Value + utDynarrayGetValueIndex(Dynarray);}
#define utDynarrayGetValues utDynarrayGetValue
utInlineC void utDynarraySetValue(utDynarray Dynarray, uint8 *valuePtr, uint32 numValue) {
    utDynarrayResizeValues(Dynarray, numValue);
    memcpy(utDynarrayGetValues(Dynarray), valuePtr, numValue*sizeof(uint8));
    utRecordArray(utModuleID, 11, utDynarrayGetValueIndex(Dynarray), utDynarrayGetNumValue(Dynarray), false);
}
utInlineC void utDynarraySetiValue(utDynarray Dynarray, uint32 x, uint8 value) {
    utDynarrays.Value[utDynarrayGetValueIndex(Dynarray) + utDynarrayCheckValueIndex(Dynarray, (x))] = value;
    utRecordField(utModuleID, 11, utDynarrayGetValueIndex(Dynarray) + (x), false);}
utInlineC void utDynarrayMoveValues(utDynarray Dynarray, uint32 from, uint32 to, uint32 count) {
    utAssert((to+count) <= utDynarrayGetNumValue(Dynarray));
    utAssert((from+count) <= utDynarrayGetNumValue(Dynarray));
    memmove(utDynarrayGetValues(Dynarray)+to,utDynarrayGetValues(Dynarray)+from,((int32)count)*sizeof(uint8));
}
utInlineC void utDynarrayCopyValues(utDynarray Dynarray, uint32 x, uint8 * values, uint32 count) {
    utAssert((x+count) <= utDynarrayGetNumValue(Dynarray));
    memcpy(utDynarrayGetValues(Dynarray)+x, values, count*sizeof(uint8));
}
utInlineC void utDynarrayAppendValues(utDynarray Dynarray, uint8 * values, uint32 count) {
    uint32 num = utDynarrayGetNumValue(Dynarray);
    utDynarrayResizeValues(Dynarray, num+count);
    utDynarrayCopyValues(Dynarray, num, values, count);
}
utInlineC void utDynarrayAppendValue(utDynarray Dynarray, uint8 Value) {
    utDynarrayResizeValues(Dynarray, utDynarrayGetNumValue(Dynarray)+1);
    utDynarraySetiValue(Dynarray, utDynarrayGetNumValue(Dynarray)-1, Value);
}
utInlineC void utDynarrayInsertValues(utDynarray Dynarray, uint32 x, uint8 *Value, uint32 count) {
    utAssert(x <= utDynarrayGetNumValue(Dynarray));
    if(x < utDynarrayGetNumValue(Dynarray)) {
        utDynarrayResizeValues(Dynarray, utDynarrayGetNumValue(Dynarray)+count);
        utDynarrayMoveValues(Dynarray, x, x+count, utDynarrayGetNumValue(Dynarray)-x-count);
        utDynarrayCopyValues(Dynarray, x, Value, count);
    }
    else {
        utDynarrayAppendValues(Dynarray, Value, count);
    }
}
utInlineC void utDynarrayInsertValue(utDynarray Dynarray, uint32 x, uint8 Value) {
    utDynarrayInsertValues(Dynarray, x, &Value, 1);
}
utInlineC void utDynarrayRemoveValues(utDynarray Dynarray, uint32 x, uint32 count) {
    utAssert((x+count) <= utDynarrayGetNumValue(Dynarray));
    if((x+count) < utDynarrayGetNumValue(Dynarray)) {
        utDynarrayMoveValues(Dynarray, x+count,x,utDynarrayGetNumValue(Dynarray)-x-count);
    }
    utDynarrayResizeValues(Dynarray, utDynarrayGetNumValue(Dynarray)-(int32)count);
}
utInlineC void utDynarrayRemoveValue(utDynarray Dynarray, uint32 x) {
    utDynarrayRemoveValues(Dynarray, x, 1);
}
utInlineC void utDynarraySwapValue(utDynarray Dynarray, uint32 from, uint32 to) {
    utAssert(from <= utDynarrayGetNumValue(Dynarray));
    utAssert(to <= utDynarrayGetNumValue(Dynarray));
    uint8 tmp = utDynarrayGetiValue(Dynarray, from);
    utDynarraySetiValue(Dynarray, from, utDynarrayGetiValue(Dynarray, to));
    utDynarraySetiValue(Dynarray, to, tmp);
}
utInlineC void utDynarraySwapValues(utDynarray Dynarray, uint32 from, uint32 to, uint32 count) {
    utAssert((from+count) < utDynarrayGetNumValue(Dynarray));
    utAssert((to+count) < utDynarrayGetNumValue(Dynarray));
    uint8 tmp[count];
    memcpy(tmp, utDynarrayGetValues(Dynarray)+from, count*sizeof(uint8));
    memcpy(utDynarrayGetValues(Dynarray)+from, utDynarrayGetValues(Dynarray)+to, count*sizeof(uint8));
    memcpy(utDynarrayGetValues(Dynarray)+to, tmp, count*sizeof(uint8));
}
#define utForeachDynarrayValue(pVar, cVar) { \
    uint32 _xValue; \
    for(_xValue = 0; _xValue < utDynarrayGetNumValue(pVar); _xValue++) { \
        cVar = utDynarrayGetiValue(pVar, _xValue);
#define utEndDynarrayValue }}
utInlineC uint16 utDynarrayGetValueSize(utDynarray Dynarray) {return utDynarrays.ValueSize[utDynarray2ValidIndex(Dynarray)];}
utInlineC void utDynarraySetValueSize(utDynarray Dynarray, uint16 value) {utDynarrays.ValueSize[utDynarray2ValidIndex(Dynarray)] = value;
    utRecordField(utModuleID, 12, utDynarray2ValidIndex(Dynarray), false);}
utInlineC uint32 utDynarrayGetUsedValue(utDynarray Dynarray) {return utDynarrays.UsedValue[utDynarray2ValidIndex(Dynarray)];}
utInlineC void utDynarraySetUsedValue(utDynarray Dynarray, uint32 value) {utDynarrays.UsedValue[utDynarray2ValidIndex(Dynarray)] = value;
    utRecordField(utModuleID, 13, utDynarray2ValidIndex(Dynarray), false);}
utInlineC uint32 utDynarrayGetSize(utDynarray Dynarray) {return utDynarrays.Size[utDynarray2ValidIndex(Dynarray)];}
utInlineC void utDynarraySetSize(utDynarray Dynarray, uint32 value) {utDynarrays.Size[utDynarray2ValidIndex(Dynarray)] = value;
    utRecordField(utModuleID, 14, utDynarray2ValidIndex(Dynarray), false);}
utInlineC utDynarray utDynarrayGetFreeList(utDynarray Dynarray) {return utDynarrays.FreeList[utDynarray2ValidIndex(Dynarray)];}
utInlineC void utDynarraySetFreeList(utDynarray Dynarray, utDynarray value) {utDynarrays.FreeList[utDynarray2ValidIndex(Dynarray)] = value;
    utRecordField(utModuleID, 15, utDynarray2ValidIndex(Dynarray), false);}
utInlineC void utDynarraySetConstructorCallback(void(*func)(utDynarray)) {utDynarrayConstructorCallback = func;}
utInlineC utDynarrayCallbackType utDynarrayGetConstructorCallback(void) {return utDynarrayConstructorCallback;}
utInlineC void utDynarraySetDestructorCallback(void(*func)(utDynarray)) {utDynarrayDestructorCallback = func;}
utInlineC utDynarrayCallbackType utDynarrayGetDestructorCallback(void) {return utDynarrayDestructorCallback;}
utInlineC utDynarray utDynarrayNextFree(utDynarray Dynarray) {return ((utDynarray *)(void *)(utDynarrays.FreeList))[utDynarray2ValidIndex(Dynarray)];}
utInlineC void utDynarraySetNextFree(utDynarray Dynarray, utDynarray value) {
    ((utDynarray *)(void *)(utDynarrays.FreeList))[utDynarray2ValidIndex(Dynarray)] = value;
    utRecordField(utModuleID, 15, utDynarray2ValidIndex(Dynarray), false);}
utInlineC void utDynarrayFree(utDynarray Dynarray) {
    utDynarrayFreeValues(Dynarray);
    utDynarraySetNextFree(Dynarray, utRootData.firstFreeDynarray);
    utSetFirstFreeDynarray(Dynarray);}
void utDynarrayDestroy(utDynarray Dynarray);
utInlineC utDynarray utDynarrayAllocRaw(void) {
    utDynarray Dynarray;
    if(utRootData.firstFreeDynarray != utDynarrayNull) {
        Dynarray = utRootData.firstFreeDynarray;
        utSetFirstFreeDynarray(utDynarrayNextFree(Dynarray));
    } else {
        if(utRootData.usedDynarray == utRootData.allocatedDynarray) {
            utDynarrayAllocMore();
        }
        Dynarray = utIndex2Dynarray(utRootData.usedDynarray);
        utSetUsedDynarray(utUsedDynarray() + 1);
    }
    return Dynarray;}
utInlineC utDynarray utDynarrayAlloc(void) {
    utDynarray Dynarray = utDynarrayAllocRaw();
    utDynarraySetValueIndex(Dynarray, 0);
    utDynarraySetNumValue(Dynarray, 0);
    utDynarraySetNumValue(Dynarray, 0);
    utDynarraySetValueSize(Dynarray, 0);
    utDynarraySetUsedValue(Dynarray, 0);
    utDynarraySetSize(Dynarray, 0);
    utDynarraySetFreeList(Dynarray, utDynarrayNull);
    if(utDynarrayConstructorCallback != NULL) {
        utDynarrayConstructorCallback(Dynarray);
    }
    return Dynarray;}

/*----------------------------------------------------------------------------------------
  Fields for class SymArray.
----------------------------------------------------------------------------------------*/
struct utSymArrayFields {
    uint32 *SymIndex;
    uint32 *NumSym;
    utSym *Sym;
    uint32 *UsedSym;
    utSymArray *FreeList;
};
extern struct utSymArrayFields utSymArrays;

void utSymArrayAllocMore(void);
void utSymArrayCopyProps(utSymArray utOldSymArray, utSymArray utNewSymArray);
void utSymArrayAllocSyms(utSymArray SymArray, uint32 numSyms);
void utSymArrayResizeSyms(utSymArray SymArray, uint32 numSyms);
void utSymArrayFreeSyms(utSymArray SymArray);
void utCompactSymArraySyms(void);
utInlineC uint32 utSymArrayGetSymIndex(utSymArray SymArray) {return utSymArrays.SymIndex[utSymArray2ValidIndex(SymArray)];}
utInlineC void utSymArraySetSymIndex(utSymArray SymArray, uint32 value) {utSymArrays.SymIndex[utSymArray2ValidIndex(SymArray)] = value;
    utRecordField(utModuleID, 16, utSymArray2ValidIndex(SymArray), false);}
utInlineC uint32 utSymArrayGetNumSym(utSymArray SymArray) {return utSymArrays.NumSym[utSymArray2ValidIndex(SymArray)];}
utInlineC void utSymArraySetNumSym(utSymArray SymArray, uint32 value) {utSymArrays.NumSym[utSymArray2ValidIndex(SymArray)] = value;
    utRecordField(utModuleID, 17, utSymArray2ValidIndex(SymArray), false);}
#if defined(DD_DEBUG)
utInlineC uint32 utSymArrayCheckSymIndex(utSymArray SymArray, uint32 x) {utAssert(x < utSymArrayGetNumSym(SymArray)); return x;}
#else
utInlineC uint32 utSymArrayCheckSymIndex(utSymArray SymArray, uint32 x) {return x;}
#endif
utInlineC utSym utSymArrayGetiSym(utSymArray SymArray, uint32 x) {return utSymArrays.Sym[
    utSymArrayGetSymIndex(SymArray) + utSymArrayCheckSymIndex(SymArray, x)];}
utInlineC utSym *utSymArrayGetSym(utSymArray SymArray) {return utSymArrays.Sym + utSymArrayGetSymIndex(SymArray);}
#define utSymArrayGetSyms utSymArrayGetSym
utInlineC void utSymArraySetSym(utSymArray SymArray, utSym *valuePtr, uint32 numSym) {
    utSymArrayResizeSyms(SymArray, numSym);
    memcpy(utSymArrayGetSyms(SymArray), valuePtr, numSym*sizeof(utSym));
    utRecordArray(utModuleID, 18, utSymArrayGetSymIndex(SymArray), utSymArrayGetNumSym(SymArray), false);
}
utInlineC void utSymArraySetiSym(utSymArray SymArray, uint32 x, utSym value) {
    utSymArrays.Sym[utSymArrayGetSymIndex(SymArray) + utSymArrayCheckSymIndex(SymArray, (x))] = value;
    utRecordField(utModuleID, 18, utSymArrayGetSymIndex(SymArray) + (x), false);}
utInlineC uint32 utSymArrayGetUsedSym(utSymArray SymArray) {return utSymArrays.UsedSym[utSymArray2ValidIndex(SymArray)];}
utInlineC void utSymArraySetUsedSym(utSymArray SymArray, uint32 value) {utSymArrays.UsedSym[utSymArray2ValidIndex(SymArray)] = value;
    utRecordField(utModuleID, 19, utSymArray2ValidIndex(SymArray), false);}
utInlineC utSymArray utSymArrayGetFreeList(utSymArray SymArray) {return utSymArrays.FreeList[utSymArray2ValidIndex(SymArray)];}
utInlineC void utSymArraySetFreeList(utSymArray SymArray, utSymArray value) {utSymArrays.FreeList[utSymArray2ValidIndex(SymArray)] = value;
    utRecordField(utModuleID, 20, utSymArray2ValidIndex(SymArray), false);}
utInlineC void utSymArraySetConstructorCallback(void(*func)(utSymArray)) {utSymArrayConstructorCallback = func;}
utInlineC utSymArrayCallbackType utSymArrayGetConstructorCallback(void) {return utSymArrayConstructorCallback;}
utInlineC void utSymArraySetDestructorCallback(void(*func)(utSymArray)) {utSymArrayDestructorCallback = func;}
utInlineC utSymArrayCallbackType utSymArrayGetDestructorCallback(void) {return utSymArrayDestructorCallback;}
utInlineC utSymArray utSymArrayNextFree(utSymArray SymArray) {return ((utSymArray *)(void *)(utSymArrays.FreeList))[utSymArray2ValidIndex(SymArray)];}
utInlineC void utSymArraySetNextFree(utSymArray SymArray, utSymArray value) {
    ((utSymArray *)(void *)(utSymArrays.FreeList))[utSymArray2ValidIndex(SymArray)] = value;
    utRecordField(utModuleID, 20, utSymArray2ValidIndex(SymArray), false);}
utInlineC void utSymArrayFree(utSymArray SymArray) {
    utSymArrayFreeSyms(SymArray);
    utSymArraySetNextFree(SymArray, utRootData.firstFreeSymArray);
    utSetFirstFreeSymArray(SymArray);}
void utSymArrayDestroy(utSymArray SymArray);
utInlineC utSymArray utSymArrayAllocRaw(void) {
    utSymArray SymArray;
    if(utRootData.firstFreeSymArray != utSymArrayNull) {
        SymArray = utRootData.firstFreeSymArray;
        utSetFirstFreeSymArray(utSymArrayNextFree(SymArray));
    } else {
        if(utRootData.usedSymArray == utRootData.allocatedSymArray) {
            utSymArrayAllocMore();
        }
        SymArray = utIndex2SymArray(utRootData.usedSymArray);
        utSetUsedSymArray(utUsedSymArray() + 1);
    }
    return SymArray;}
utInlineC utSymArray utSymArrayAlloc(void) {
    utSymArray SymArray = utSymArrayAllocRaw();
    utSymArraySetSymIndex(SymArray, 0);
    utSymArraySetNumSym(SymArray, 0);
    utSymArraySetNumSym(SymArray, 0);
    utSymArraySetUsedSym(SymArray, 0);
    utSymArraySetFreeList(SymArray, utSymArrayNull);
    if(utSymArrayConstructorCallback != NULL) {
        utSymArrayConstructorCallback(SymArray);
    }
    return SymArray;}

/*----------------------------------------------------------------------------------------
  Relationship macros between classes.
----------------------------------------------------------------------------------------*/
#define utForeachSymArraySym(pVar, cVar) { \
    uint32 _xSym; \
    for(_xSym = 0; _xSym < utSymArrayGetUsedSym(pVar); _xSym++) { \
        cVar = utSymArrayGetiSym(pVar, _xSym); \
        if(cVar != utSymNull) {
#define utEndSymArraySym }}}
void utSymArrayInsertSym(utSymArray SymArray, uint32 x, utSym _Sym);
void utSymArrayAppendSym(utSymArray SymArray, utSym _Sym);
void utDatabaseStart(void);
void utDatabaseStop(void);
#if defined __cplusplus
}
#endif

#endif
