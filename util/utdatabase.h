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

static utInlineC uint32 utHash(void) {return utRootData.hash;}
static utInlineC uint32 utUsedSymtab(void) {return utRootData.usedSymtab;}
static utInlineC uint32 utAllocatedSymtab(void) {return utRootData.allocatedSymtab;}
static utInlineC void utSetUsedSymtab(uint32 value) {utRootData.usedSymtab = value;}
static utInlineC void utSetAllocatedSymtab(uint32 value) {utRootData.allocatedSymtab = value;}
static utInlineC uint32 utUsedSymtabTable(void) {return utRootData.usedSymtabTable;}
static utInlineC uint32 utAllocatedSymtabTable(void) {return utRootData.allocatedSymtabTable;}
static utInlineC uint32 utFreeSymtabTable(void) {return utRootData.freeSymtabTable;}
static utInlineC void utSetUsedSymtabTable(uint32 value) {utRootData.usedSymtabTable = value;}
static utInlineC void utSetAllocatedSymtabTable(uint32 value) {utRootData.allocatedSymtabTable = value;}
static utInlineC void utSetFreeSymtabTable(int32 value) {utRootData.freeSymtabTable = value;}
static utInlineC uint32 utUsedSym(void) {return utRootData.usedSym;}
static utInlineC uint32 utAllocatedSym(void) {return utRootData.allocatedSym;}
static utInlineC void utSetUsedSym(uint32 value) {utRootData.usedSym = value;}
static utInlineC void utSetAllocatedSym(uint32 value) {utRootData.allocatedSym = value;}
static utInlineC uint32 utUsedSymName(void) {return utRootData.usedSymName;}
static utInlineC uint32 utAllocatedSymName(void) {return utRootData.allocatedSymName;}
static utInlineC uint32 utFreeSymName(void) {return utRootData.freeSymName;}
static utInlineC void utSetUsedSymName(uint32 value) {utRootData.usedSymName = value;}
static utInlineC void utSetAllocatedSymName(uint32 value) {utRootData.allocatedSymName = value;}
static utInlineC void utSetFreeSymName(int32 value) {utRootData.freeSymName = value;}
static utInlineC utDynarray utFirstFreeDynarray(void) {return utRootData.firstFreeDynarray;}
static utInlineC void utSetFirstFreeDynarray(utDynarray value) {utRootData.firstFreeDynarray = (value);}
static utInlineC uint32 utUsedDynarray(void) {return utRootData.usedDynarray;}
static utInlineC uint32 utAllocatedDynarray(void) {return utRootData.allocatedDynarray;}
static utInlineC void utSetUsedDynarray(uint32 value) {utRootData.usedDynarray = value;}
static utInlineC void utSetAllocatedDynarray(uint32 value) {utRootData.allocatedDynarray = value;}
static utInlineC uint32 utUsedDynarrayValue(void) {return utRootData.usedDynarrayValue;}
static utInlineC uint32 utAllocatedDynarrayValue(void) {return utRootData.allocatedDynarrayValue;}
static utInlineC uint32 utFreeDynarrayValue(void) {return utRootData.freeDynarrayValue;}
static utInlineC void utSetUsedDynarrayValue(uint32 value) {utRootData.usedDynarrayValue = value;}
static utInlineC void utSetAllocatedDynarrayValue(uint32 value) {utRootData.allocatedDynarrayValue = value;}
static utInlineC void utSetFreeDynarrayValue(int32 value) {utRootData.freeDynarrayValue = value;}
static utInlineC utSymArray utFirstFreeSymArray(void) {return utRootData.firstFreeSymArray;}
static utInlineC void utSetFirstFreeSymArray(utSymArray value) {utRootData.firstFreeSymArray = (value);}
static utInlineC uint32 utUsedSymArray(void) {return utRootData.usedSymArray;}
static utInlineC uint32 utAllocatedSymArray(void) {return utRootData.allocatedSymArray;}
static utInlineC void utSetUsedSymArray(uint32 value) {utRootData.usedSymArray = value;}
static utInlineC void utSetAllocatedSymArray(uint32 value) {utRootData.allocatedSymArray = value;}
static utInlineC uint32 utUsedSymArraySym(void) {return utRootData.usedSymArraySym;}
static utInlineC uint32 utAllocatedSymArraySym(void) {return utRootData.allocatedSymArraySym;}
static utInlineC uint32 utFreeSymArraySym(void) {return utRootData.freeSymArraySym;}
static utInlineC void utSetUsedSymArraySym(uint32 value) {utRootData.usedSymArraySym = value;}
static utInlineC void utSetAllocatedSymArraySym(uint32 value) {utRootData.allocatedSymArraySym = value;}
static utInlineC void utSetFreeSymArraySym(int32 value) {utRootData.freeSymArraySym = value;}

/* Validate macros */
#if defined(DD_DEBUG)
static utInlineC utSymtab utValidSymtab(utSymtab Symtab) {
    utAssert(utLikely(Symtab != utSymtabNull && (uint32)(Symtab - (utSymtab)0) < utRootData.usedSymtab));
    return Symtab;}
static utInlineC utSym utValidSym(utSym Sym) {
    utAssert(utLikely(Sym != utSymNull && (uint32)(Sym - (utSym)0) < utRootData.usedSym));
    return Sym;}
static utInlineC utDynarray utValidDynarray(utDynarray Dynarray) {
    utAssert(utLikely(Dynarray != utDynarrayNull && (uint32)(Dynarray - (utDynarray)0) < utRootData.usedDynarray));
    return Dynarray;}
static utInlineC utSymArray utValidSymArray(utSymArray SymArray) {
    utAssert(utLikely(SymArray != utSymArrayNull && (uint32)(SymArray - (utSymArray)0) < utRootData.usedSymArray));
    return SymArray;}
#else
static utInlineC utSymtab utValidSymtab(utSymtab Symtab) {return Symtab;}
static utInlineC utSym utValidSym(utSym Sym) {return Sym;}
static utInlineC utDynarray utValidDynarray(utDynarray Dynarray) {return Dynarray;}
static utInlineC utSymArray utValidSymArray(utSymArray SymArray) {return SymArray;}
#endif

/* Object ref to integer conversions */
#if (defined(DD_DEBUG) && !defined(DD_NOSTRICT)) || defined(DD_STRICT)
static utInlineC uint32 utSymtab2Index(utSymtab Symtab) {return Symtab - (utSymtab)0;}
static uint32 utSymtab2ValidIndex(utSymtab Symtab) {return utValidSymtab(Symtab) - (utSymtab)0;}
static utSymtab utIndex2Symtab(uint32 xSymtab) {return (utSymtab)(xSymtab + (utSymtab)(0));}
static utInlineC uint32 utSym2Index(utSym Sym) {return Sym - (utSym)0;}
static uint32 utSym2ValidIndex(utSym Sym) {return utValidSym(Sym) - (utSym)0;}
static utSym utIndex2Sym(uint32 xSym) {return (utSym)(xSym + (utSym)(0));}
static utInlineC uint32 utDynarray2Index(utDynarray Dynarray) {return Dynarray - (utDynarray)0;}
static uint32 utDynarray2ValidIndex(utDynarray Dynarray) {return utValidDynarray(Dynarray) - (utDynarray)0;}
static utDynarray utIndex2Dynarray(uint32 xDynarray) {return (utDynarray)(xDynarray + (utDynarray)(0));}
static utInlineC uint32 utSymArray2Index(utSymArray SymArray) {return SymArray - (utSymArray)0;}
static uint32 utSymArray2ValidIndex(utSymArray SymArray) {return utValidSymArray(SymArray) - (utSymArray)0;}
static utSymArray utIndex2SymArray(uint32 xSymArray) {return (utSymArray)(xSymArray + (utSymArray)(0));}
#else
static utInlineC uint32 utSymtab2Index(utSymtab Symtab) {return Symtab;}
static utInlineC uint32 utSymtab2ValidIndex(utSymtab Symtab) {return utValidSymtab(Symtab);}
static utInlineC utSymtab utIndex2Symtab(uint32 xSymtab) {return xSymtab;}
static utInlineC uint32 utSym2Index(utSym Sym) {return Sym;}
static utInlineC uint32 utSym2ValidIndex(utSym Sym) {return utValidSym(Sym);}
static utInlineC utSym utIndex2Sym(uint32 xSym) {return xSym;}
static utInlineC uint32 utDynarray2Index(utDynarray Dynarray) {return Dynarray;}
static utInlineC uint32 utDynarray2ValidIndex(utDynarray Dynarray) {return utValidDynarray(Dynarray);}
static utInlineC utDynarray utIndex2Dynarray(uint32 xDynarray) {return xDynarray;}
static utInlineC uint32 utSymArray2Index(utSymArray SymArray) {return SymArray;}
static utInlineC uint32 utSymArray2ValidIndex(utSymArray SymArray) {return utValidSymArray(SymArray);}
static utInlineC utSymArray utIndex2SymArray(uint32 xSymArray) {return xSymArray;}
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
static utInlineC uint32 utSymtabGetTableIndex(utSymtab Symtab) {return utSymtabs.TableIndex[utSymtab2ValidIndex(Symtab)];}
static utInlineC void utSymtabSetTableIndex(utSymtab Symtab, uint32 value) {utSymtabs.TableIndex[utSymtab2ValidIndex(Symtab)] = value;}
static utInlineC uint32 utSymtabGetNumTable(utSymtab Symtab) {return utSymtabs.NumTable[utSymtab2ValidIndex(Symtab)];}
static utInlineC void utSymtabSetNumTable(utSymtab Symtab, uint32 value) {utSymtabs.NumTable[utSymtab2ValidIndex(Symtab)] = value;}
#if defined(DD_DEBUG)
static utInlineC uint32 utSymtabCheckTableIndex(utSymtab Symtab, uint32 x) {utAssert(x < utSymtabGetNumTable(Symtab)); return x;}
#else
static utInlineC uint32 utSymtabCheckTableIndex(utSymtab Symtab, uint32 x) {return x;}
#endif
static utInlineC utSym utSymtabGetiTable(utSymtab Symtab, uint32 x) {return utSymtabs.Table[
    utSymtabGetTableIndex(Symtab) + utSymtabCheckTableIndex(Symtab, x)];}
static utInlineC utSym *utSymtabGetTable(utSymtab Symtab) {return utSymtabs.Table + utSymtabGetTableIndex(Symtab);}
#define utSymtabGetTables utSymtabGetTable
static utInlineC void utSymtabSetTable(utSymtab Symtab, utSym *valuePtr, uint32 numTable) {
    utSymtabResizeTables(Symtab, numTable);
    memcpy(utSymtabGetTables(Symtab), valuePtr, numTable*sizeof(utSym));}
static utInlineC void utSymtabSetiTable(utSymtab Symtab, uint32 x, utSym value) {
    utSymtabs.Table[utSymtabGetTableIndex(Symtab) + utSymtabCheckTableIndex(Symtab, (x))] = value;}
static utInlineC uint32 utSymtabGetNumSym(utSymtab Symtab) {return utSymtabs.NumSym[utSymtab2ValidIndex(Symtab)];}
static utInlineC void utSymtabSetNumSym(utSymtab Symtab, uint32 value) {utSymtabs.NumSym[utSymtab2ValidIndex(Symtab)] = value;}
static utInlineC void utSymtabSetConstructorCallback(void(*func)(utSymtab)) {utSymtabConstructorCallback = func;}
static utInlineC utSymtabCallbackType utSymtabGetConstructorCallback(void) {return utSymtabConstructorCallback;}
static utInlineC utSymtab utFirstSymtab(void) {return utRootData.usedSymtab == 1? utSymtabNull : utIndex2Symtab(1);}
static utInlineC utSymtab utLastSymtab(void) {return utRootData.usedSymtab == 1? utSymtabNull :
    utIndex2Symtab(utRootData.usedSymtab - 1);}
static utSymtab utNextSymtab(utSymtab Symtab) {return utSymtab2ValidIndex(Symtab) + 1 == utRootData.usedSymtab? utSymtabNull :
    Symtab + 1;}
static utInlineC utSymtab utPrevSymtab(utSymtab Symtab) {return utSymtab2ValidIndex(Symtab) == 1? utSymtabNull : Symtab - 1;}
#define utForeachSymtab(var) \
    for(var = utIndex2Symtab(1); utSymtab2Index(var) != utRootData.usedSymtab; var++)
#define utEndSymtab
static utInlineC void utSymtabFreeAll(void) {utSetUsedSymtab(1); utSetUsedSymtabTable(0);}
static utInlineC utSymtab utSymtabAllocRaw(void) {
    utSymtab Symtab;
    if(utRootData.usedSymtab == utRootData.allocatedSymtab) {
        utSymtabAllocMore();
    }
    Symtab = utIndex2Symtab(utRootData.usedSymtab);
    utSetUsedSymtab(utUsedSymtab() + 1);
    return Symtab;}
static utInlineC utSymtab utSymtabAlloc(void) {
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
static utInlineC uint32 utSymGetNameIndex(utSym Sym) {return utSyms.NameIndex[utSym2ValidIndex(Sym)];}
static utInlineC void utSymSetNameIndex(utSym Sym, uint32 value) {utSyms.NameIndex[utSym2ValidIndex(Sym)] = value;}
static utInlineC uint32 utSymGetNumName(utSym Sym) {return utSyms.NumName[utSym2ValidIndex(Sym)];}
static utInlineC void utSymSetNumName(utSym Sym, uint32 value) {utSyms.NumName[utSym2ValidIndex(Sym)] = value;}
#if defined(DD_DEBUG)
static utInlineC uint32 utSymCheckNameIndex(utSym Sym, uint32 x) {utAssert(x < utSymGetNumName(Sym)); return x;}
#else
static utInlineC uint32 utSymCheckNameIndex(utSym Sym, uint32 x) {return x;}
#endif
static utInlineC char utSymGetiName(utSym Sym, uint32 x) {return utSyms.Name[
    utSymGetNameIndex(Sym) + utSymCheckNameIndex(Sym, x)];}
static utInlineC char *utSymGetName(utSym Sym) {return utSyms.Name + utSymGetNameIndex(Sym);}
#define utSymGetNames utSymGetName
static utInlineC void utSymSetName(utSym Sym, char *valuePtr, uint32 numName) {
    utSymResizeNames(Sym, numName);
    memcpy(utSymGetNames(Sym), valuePtr, numName*sizeof(char));}
static utInlineC void utSymSetiName(utSym Sym, uint32 x, char value) {
    utSyms.Name[utSymGetNameIndex(Sym) + utSymCheckNameIndex(Sym, (x))] = value;}
static utInlineC uint32 utSymGetHashValue(utSym Sym) {return utSyms.HashValue[utSym2ValidIndex(Sym)];}
static utInlineC void utSymSetHashValue(utSym Sym, uint32 value) {utSyms.HashValue[utSym2ValidIndex(Sym)] = value;}
static utInlineC utSym utSymGetNext(utSym Sym) {return utSyms.Next[utSym2ValidIndex(Sym)];}
static utInlineC void utSymSetNext(utSym Sym, utSym value) {utSyms.Next[utSym2ValidIndex(Sym)] = value;}
static utInlineC void utSymSetConstructorCallback(void(*func)(utSym)) {utSymConstructorCallback = func;}
static utInlineC utSymCallbackType utSymGetConstructorCallback(void) {return utSymConstructorCallback;}
static utInlineC utSym utFirstSym(void) {return utRootData.usedSym == 1? utSymNull : utIndex2Sym(1);}
static utInlineC utSym utLastSym(void) {return utRootData.usedSym == 1? utSymNull :
    utIndex2Sym(utRootData.usedSym - 1);}
static utSym utNextSym(utSym Sym) {return utSym2ValidIndex(Sym) + 1 == utRootData.usedSym? utSymNull :
    Sym + 1;}
static utInlineC utSym utPrevSym(utSym Sym) {return utSym2ValidIndex(Sym) == 1? utSymNull : Sym - 1;}
#define utForeachSym(var) \
    for(var = utIndex2Sym(1); utSym2Index(var) != utRootData.usedSym; var++)
#define utEndSym
static utInlineC void utSymFreeAll(void) {utSetUsedSym(1); utSetUsedSymName(0);}
static utInlineC utSym utSymAllocRaw(void) {
    utSym Sym;
    if(utRootData.usedSym == utRootData.allocatedSym) {
        utSymAllocMore();
    }
    Sym = utIndex2Sym(utRootData.usedSym);
    utSetUsedSym(utUsedSym() + 1);
    return Sym;}
static utInlineC utSym utSymAlloc(void) {
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
static utInlineC uint32 utDynarrayGetValueIndex(utDynarray Dynarray) {return utDynarrays.ValueIndex[utDynarray2ValidIndex(Dynarray)];}
static utInlineC void utDynarraySetValueIndex(utDynarray Dynarray, uint32 value) {utDynarrays.ValueIndex[utDynarray2ValidIndex(Dynarray)] = value;}
static utInlineC uint32 utDynarrayGetNumValue(utDynarray Dynarray) {return utDynarrays.NumValue[utDynarray2ValidIndex(Dynarray)];}
static utInlineC void utDynarraySetNumValue(utDynarray Dynarray, uint32 value) {utDynarrays.NumValue[utDynarray2ValidIndex(Dynarray)] = value;}
#if defined(DD_DEBUG)
static utInlineC uint32 utDynarrayCheckValueIndex(utDynarray Dynarray, uint32 x) {utAssert(x < utDynarrayGetNumValue(Dynarray)); return x;}
#else
static utInlineC uint32 utDynarrayCheckValueIndex(utDynarray Dynarray, uint32 x) {return x;}
#endif
static utInlineC uint8 utDynarrayGetiValue(utDynarray Dynarray, uint32 x) {return utDynarrays.Value[
    utDynarrayGetValueIndex(Dynarray) + utDynarrayCheckValueIndex(Dynarray, x)];}
static utInlineC uint8 *utDynarrayGetValue(utDynarray Dynarray) {return utDynarrays.Value + utDynarrayGetValueIndex(Dynarray);}
#define utDynarrayGetValues utDynarrayGetValue
static utInlineC void utDynarraySetValue(utDynarray Dynarray, uint8 *valuePtr, uint32 numValue) {
    utDynarrayResizeValues(Dynarray, numValue);
    memcpy(utDynarrayGetValues(Dynarray), valuePtr, numValue*sizeof(uint8));}
static utInlineC void utDynarraySetiValue(utDynarray Dynarray, uint32 x, uint8 value) {
    utDynarrays.Value[utDynarrayGetValueIndex(Dynarray) + utDynarrayCheckValueIndex(Dynarray, (x))] = value;}
static utInlineC uint16 utDynarrayGetValueSize(utDynarray Dynarray) {return utDynarrays.ValueSize[utDynarray2ValidIndex(Dynarray)];}
static utInlineC void utDynarraySetValueSize(utDynarray Dynarray, uint16 value) {utDynarrays.ValueSize[utDynarray2ValidIndex(Dynarray)] = value;}
static utInlineC uint32 utDynarrayGetUsedValue(utDynarray Dynarray) {return utDynarrays.UsedValue[utDynarray2ValidIndex(Dynarray)];}
static utInlineC void utDynarraySetUsedValue(utDynarray Dynarray, uint32 value) {utDynarrays.UsedValue[utDynarray2ValidIndex(Dynarray)] = value;}
static utInlineC uint32 utDynarrayGetSize(utDynarray Dynarray) {return utDynarrays.Size[utDynarray2ValidIndex(Dynarray)];}
static utInlineC void utDynarraySetSize(utDynarray Dynarray, uint32 value) {utDynarrays.Size[utDynarray2ValidIndex(Dynarray)] = value;}
static utInlineC utDynarray utDynarrayGetFreeList(utDynarray Dynarray) {return utDynarrays.FreeList[utDynarray2ValidIndex(Dynarray)];}
static utInlineC void utDynarraySetFreeList(utDynarray Dynarray, utDynarray value) {utDynarrays.FreeList[utDynarray2ValidIndex(Dynarray)] = value;}
static utInlineC void utDynarraySetConstructorCallback(void(*func)(utDynarray)) {utDynarrayConstructorCallback = func;}
static utInlineC utDynarrayCallbackType utDynarrayGetConstructorCallback(void) {return utDynarrayConstructorCallback;}
static utInlineC void utDynarraySetDestructorCallback(void(*func)(utDynarray)) {utDynarrayDestructorCallback = func;}
static utInlineC utDynarrayCallbackType utDynarrayGetDestructorCallback(void) {return utDynarrayDestructorCallback;}
static utInlineC utDynarray utDynarrayNextFree(utDynarray Dynarray) {return ((utDynarray *)(void *)(utDynarrays.FreeList))[utDynarray2ValidIndex(Dynarray)];}
static utInlineC void utDynarraySetNextFree(utDynarray Dynarray, utDynarray value) {
    ((utDynarray *)(void *)(utDynarrays.FreeList))[utDynarray2ValidIndex(Dynarray)] = value;}
static utInlineC void utDynarrayFree(utDynarray Dynarray) {
    utDynarrayFreeValues(Dynarray);
    utDynarraySetNextFree(Dynarray, utRootData.firstFreeDynarray);
    utSetFirstFreeDynarray(Dynarray);}
void utDynarrayDestroy(utDynarray Dynarray);
static utInlineC utDynarray utDynarrayAllocRaw(void) {
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
static utInlineC utDynarray utDynarrayAlloc(void) {
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
static utInlineC uint32 utSymArrayGetSymIndex(utSymArray SymArray) {return utSymArrays.SymIndex[utSymArray2ValidIndex(SymArray)];}
static utInlineC void utSymArraySetSymIndex(utSymArray SymArray, uint32 value) {utSymArrays.SymIndex[utSymArray2ValidIndex(SymArray)] = value;}
static utInlineC uint32 utSymArrayGetNumSym(utSymArray SymArray) {return utSymArrays.NumSym[utSymArray2ValidIndex(SymArray)];}
static utInlineC void utSymArraySetNumSym(utSymArray SymArray, uint32 value) {utSymArrays.NumSym[utSymArray2ValidIndex(SymArray)] = value;}
#if defined(DD_DEBUG)
static utInlineC uint32 utSymArrayCheckSymIndex(utSymArray SymArray, uint32 x) {utAssert(x < utSymArrayGetNumSym(SymArray)); return x;}
#else
static utInlineC uint32 utSymArrayCheckSymIndex(utSymArray SymArray, uint32 x) {return x;}
#endif
static utInlineC utSym utSymArrayGetiSym(utSymArray SymArray, uint32 x) {return utSymArrays.Sym[
    utSymArrayGetSymIndex(SymArray) + utSymArrayCheckSymIndex(SymArray, x)];}
static utInlineC utSym *utSymArrayGetSym(utSymArray SymArray) {return utSymArrays.Sym + utSymArrayGetSymIndex(SymArray);}
#define utSymArrayGetSyms utSymArrayGetSym
static utInlineC void utSymArraySetSym(utSymArray SymArray, utSym *valuePtr, uint32 numSym) {
    utSymArrayResizeSyms(SymArray, numSym);
    memcpy(utSymArrayGetSyms(SymArray), valuePtr, numSym*sizeof(utSym));}
static utInlineC void utSymArraySetiSym(utSymArray SymArray, uint32 x, utSym value) {
    utSymArrays.Sym[utSymArrayGetSymIndex(SymArray) + utSymArrayCheckSymIndex(SymArray, (x))] = value;}
static utInlineC uint32 utSymArrayGetUsedSym(utSymArray SymArray) {return utSymArrays.UsedSym[utSymArray2ValidIndex(SymArray)];}
static utInlineC void utSymArraySetUsedSym(utSymArray SymArray, uint32 value) {utSymArrays.UsedSym[utSymArray2ValidIndex(SymArray)] = value;}
static utInlineC utSymArray utSymArrayGetFreeList(utSymArray SymArray) {return utSymArrays.FreeList[utSymArray2ValidIndex(SymArray)];}
static utInlineC void utSymArraySetFreeList(utSymArray SymArray, utSymArray value) {utSymArrays.FreeList[utSymArray2ValidIndex(SymArray)] = value;}
static utInlineC void utSymArraySetConstructorCallback(void(*func)(utSymArray)) {utSymArrayConstructorCallback = func;}
static utInlineC utSymArrayCallbackType utSymArrayGetConstructorCallback(void) {return utSymArrayConstructorCallback;}
static utInlineC void utSymArraySetDestructorCallback(void(*func)(utSymArray)) {utSymArrayDestructorCallback = func;}
static utInlineC utSymArrayCallbackType utSymArrayGetDestructorCallback(void) {return utSymArrayDestructorCallback;}
static utInlineC utSymArray utSymArrayNextFree(utSymArray SymArray) {return ((utSymArray *)(void *)(utSymArrays.FreeList))[utSymArray2ValidIndex(SymArray)];}
static utInlineC void utSymArraySetNextFree(utSymArray SymArray, utSymArray value) {
    ((utSymArray *)(void *)(utSymArrays.FreeList))[utSymArray2ValidIndex(SymArray)] = value;}
static utInlineC void utSymArrayFree(utSymArray SymArray) {
    utSymArrayFreeSyms(SymArray);
    utSymArraySetNextFree(SymArray, utRootData.firstFreeSymArray);
    utSetFirstFreeSymArray(SymArray);}
void utSymArrayDestroy(utSymArray SymArray);
static utInlineC utSymArray utSymArrayAllocRaw(void) {
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
static utInlineC utSymArray utSymArrayAlloc(void) {
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
