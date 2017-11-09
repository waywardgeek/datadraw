/*
 * This file was written by Bill Cox, originally in 1991, and maintained since.  It is hereby
 * placed into the public domain.
 */

#include <string.h>
#include <stdlib.h>
#include "ddutil.h"

struct utMem_ *utMems;
uint32 utfFreeMem, utfVirgMem_, utmMem;
struct utStack_ *utStacks;
uint32 utsStack, utmStack;

static utStackRef utBotStack;
static uint64 utMaxMem;
uint64 utUsedMem;
uint32 utMemSeed = 42;

/*--------------------------------------------------------------------------------------------------
  Create a new mem.
--------------------------------------------------------------------------------------------------*/
utMemRef utcMem (void)
{
    utMemRef mem;

    if (uttMemExists(utfFreeMem())) {
        mem = utfFreeMem();
        utfFreeMem() = utnFreeMem(mem);
    } else {
        if (utfVirgMem() == utmMem()) {
            utmMem() += utmMem()/2;
            utMems = (struct utMem_ *)realloc((void *)utMems,
                    utmMem()*sizeof(struct utMem_));
            if (!utMems) {
                utExit("utcMem: Can't allocate memory handle");
            }
        }
        mem = utfVirgMem()++;
    }
    uttMemUsed(mem) = true;
    return mem;
}

/*--------------------------------------------------------------------------------------------------
  Delete a mem (put it on the free list).
--------------------------------------------------------------------------------------------------*/
void utdMem (
    utMemRef mem,
    char *fileName,
    uint32 line)
{
    utStackRef stack = utlStack();
    uint8 rightPicket = *(((uint8 *)utgMemPtr(mem)) + utgMemSize(mem) - 1);

    utUsedMem -= utgMemSize(mem);
    if (!uttMemUsed(mem)) {
        utExit("utdMem: Memory allocated in %s on line %u freed second time "
                "in %s on line %u", utgMemName(mem),
                utgMemLine(mem), fileName, line);
    }
    if (utgMemPicket(mem) != rightPicket) {
        utExit("utdMem: Memory allocated in %s on line %u and freed in %s "
                "line %u lost right picket", utgMemName(mem),
                utgMemLine(mem), fileName, line);
    }
    utdStackMem(stack, mem, fileName, line);
    utnFreeMem(mem) = utfFreeMem();
    utfFreeMem() = mem;
    uttMemUsed(mem) = false;
    free(utgMemName(mem));
}

/*--------------------------------------------------------------------------------------------------
  Set the seed.  Set the signature to 0;
--------------------------------------------------------------------------------------------------*/
void utMemInitSeed (
    uint32 seed)
{
    if (seed == 0) {
        utMemSeed = 1;
    } else {
        utMemSeed = seed;
    }
}

/*--------------------------------------------------------------------------------------------------
  Return a random number between 1 and 2^23 - 1. Specific to
                 utMem functions so that users utSeed is not affected.
--------------------------------------------------------------------------------------------------*/
uint32 utMemRand (void)
{
    utMemSeed = (((((utMemSeed << 5) ^ utMemSeed) & 0x7FFF80) >> 7) |
                     (utMemSeed << 16)) & 0x7FFFFF;
    return utMemSeed;
}

/*--------------------------------------------------------------------------------------------------
  Create a new mem, filling in all fields.  Note that the calling routine is responsible for
  putting the pickets in memory.
--------------------------------------------------------------------------------------------------*/
utMemRef utBuildMem (
    void *memPtr,
    size_t size,
    char *file,
    uint32 line)
{
    utMemRef mem = utcMem();

    utAssert(size > 1);
    utAssert(file != NULL && *file != '\0');
    utgMemPtr(mem) = memPtr;
    utgMemSize(mem) = size;
    utUsedMem += size;
    if(utUsedMem > utMaxMem) {
        utMaxMem = utUsedMem;
    }
    utgMemLine(mem) = line;
    utgMemName(mem) = malloc((strlen(file) + 1)*sizeof(char));
    strcpy(utgMemName(mem), file);
    utaStackMem(utlStack(), mem);
    utgMemPicket(mem) = (uint8)utMemRand();
    *(((uint8 *)memPtr) + size - 1) = utgMemPicket(mem);
    return mem;
}

/*--------------------------------------------------------------------------------------------------
  Find a mem with the same memory pointer.
--------------------------------------------------------------------------------------------------*/
utMemRef utqMemPtr (
    void *memPtr)
{
    utStackRef stack = utlStack();
    utMemRef mem;

    for (mem = utfStackMem(stack); uttMemExists(mem);
            mem = utnStackMem(stack, mem)) {
        if (utgMemPtr(mem) == memPtr) {
            return mem;
        }
    }
    return ut0Mem;
}

/*--------------------------------------------------------------------------------------------------
  Create a new stack.
--------------------------------------------------------------------------------------------------*/
utStackRef utcStackTrace (
    char *fileName,
    uint32 line)
{
    utStackRef stack;

    if (utsStack() == utmStack()) {
        utmStack() += utmStack()/2;
        utStacks = (struct utStack_ *)realloc((void *)utStacks, utmStack()*sizeof(struct utStack_));
        if (!utStacks) {
            utExit("utcStackTrace: Can't allocate a new stack");
        }
    }
    stack = utsStack()++;
    utfStackMem(stack) = ut0Mem;
    utgStackName(stack) = malloc((strlen(fileName) + 1)*sizeof(char));
    strcpy(utgStackName(stack), fileName);
    utgStackLine(stack) = line;
    return stack;
}

/*--------------------------------------------------------------------------------------------------
  Delete a stack.  Check for memory errors.
--------------------------------------------------------------------------------------------------*/
void utdStackTrace (
    utStackRef stack,
    char *fileName,
    uint32 line)
{
    utMemRef mem;
    bool error = false;

    for (mem = utfStackMem(stack); uttMemExists(mem);
            mem = utnStackMem(stack, mem)) {
        error = true;
        utExit("utdStack: Failed to free memory allocated in %s on line %u",
                utgMemName(mem), utgMemLine(mem));
    }
    if (stack != utlStack()) {
        error = true;
        utExit("utdStack: Stack allocated in %s on line %u is freed out of "
                "order in %s on line %u", utgStackName(stack),
                utgStackLine(stack), fileName, line);
    }
    if (error) {
        utExit("utdStack: Punting due to memory errors");
    }
    free(utgStackName(stack));
    utAssert(utsStack() > 0);
    utsStack()--;
}

/*--------------------------------------------------------------------------------------------------
  Add a mem to a stack.
--------------------------------------------------------------------------------------------------*/
void utaStackMem (
    utStackRef stack,
    utMemRef mem)
{
    utoStackMem(mem) = stack;
    utnStackMem(stack, mem) = utfStackMem(stack);
    utfStackMem(stack) = mem;
}

/*--------------------------------------------------------------------------------------------------
  Remove a mem from a stack.
--------------------------------------------------------------------------------------------------*/
void utdStackMem (
    utStackRef stack,
    utMemRef mem,
    char *fileName,
    uint32 line)
{
    utMemRef pMem, nMem;

    pMem = ut0Mem;
    for (nMem = utfStackMem(stack);
          uttMemExists(nMem) && nMem != mem;
          nMem = utnStackMem(stack, nMem)) {
        pMem = nMem;
    }
    if (nMem != mem) {
        utExit("utdStackMem: Memory allocated in %s on line %u does not "
                "belong to the stack allocated in %s on line %u.  Look at "
                "%s line %u", utgMemName(mem), utgMemLine(mem),
                utgStackName(stack), utgStackLine(stack),
                fileName, line);
    }
    if (uttMemExists(pMem)) {
        utnStackMem(stack, pMem) = utnStackMem(stack, mem);
    } else {
        utfStackMem(stack) = utnStackMem(stack, mem);
    }
}

/*--------------------------------------------------------------------------------------------------
  Allocate initail memory for mems.
--------------------------------------------------------------------------------------------------*/
static void allocateMems (void)
{
    utfFreeMem() = ut0Mem;
    utfVirgMem() = 0;
    utmMem() = 100;
    utMems = (struct utMem_ *)calloc(utmMem(), sizeof(struct utMem_));
    if (!utMems) {
        utExit("allocateMems: Out of memory");
    }
}

/*--------------------------------------------------------------------------------------------------
  Allocate initail memory for stacks.
--------------------------------------------------------------------------------------------------*/
static void allocateStacks (void)
{
    utsStack() = 0;
    utmStack() = 100;
    utStacks = (struct utStack_ *)calloc(utmStack(), sizeof(struct utStack_));
    if (!utStacks) {
        utExit("allocateStacks: Out of memory");
    }
}

/*--------------------------------------------------------------------------------------------------
  Initialize the memory checker module;
--------------------------------------------------------------------------------------------------*/
void utMemStart(void)
{
    allocateMems();
    allocateStacks();
    utBotStack = utcStack();
    utUsedMem = 0;
    utMaxMem = 0;
}

/*--------------------------------------------------------------------------------------------------
  Free memory used by the memory checker.
--------------------------------------------------------------------------------------------------*/
void utMemStop(
     bool reportMemory)
{
    utdStack(utBotStack);
    free((void *)utMems);
    free((void *)utStacks);
    if(reportMemory) {
         utLogMessage("Used %s of memory", utMemoryUnits(utMaxMem));
    }
}

/*--------------------------------------------------------------------------------------------------
  Perform a memory check.  Basically, check all the pickets.
--------------------------------------------------------------------------------------------------*/
void utMemCheckTrace (
    char *fileName,
    uint32 line)
{
    utStackRef stack;
    utMemRef mem;
    uint8 picket;
    static uint64 count = 0;

    count++;
    for (stack = utfStack(); uttStackExists(stack); stack = utnStack(stack)) {
        for (mem = utfStackMem(stack); uttMemExists(mem);
                mem = utnStackMem(stack, mem)) {
            picket = *(((uint8 *)utgMemPtr(mem)) + utgMemSize(mem) - 1);
            if (picket != utgMemPicket(mem)) {
                utExit("utMemCheck %lu: Invoked from %s, line %u: Picket hosed for "
                        "memory allocated in %s, " "line %u", count, fileName, 
                        line, utgMemName(mem), utgMemLine(mem));
            }
        }
    }
}

