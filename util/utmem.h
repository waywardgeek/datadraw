/*
 * Copyright (C) 1991-2006 Bill Cox
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

/*============================================================================
   Module : Memory checker
   Purpose: This module checks for writing past the end of arrays, and
            memory leaks.  It is used in conjunction with the utCalloc
            functions.  It works for both the local and far versions.
============================================================================*/
#ifndef UTMEM_H
#define UTMEM_H

#include <stddef.h>

#ifndef UTTYPES_H
#include "uttypes.h"
#endif

#if __cplusplus
extern "C" {
#endif

typedef uint32 utMemRef;
typedef uint32 utStackRef;

/*--------------------------------------------------------------------------------------------------
   Object : utMem
   Purpose: Keep track of a block of memory allocated with any ut function.
            When mems are allocated, they are assigned to the current top
            stack.  When they are deleted, they are first checked for
            validity.  Pickets are used to check trashing memory.
--------------------------------------------------------------------------------------------------*/
struct utMem_ {
   utStackRef oStack;
   void *memPtr;
   uint8 picket;
   utMemRef nMem;
   uint32 line;
   char *name;
   size_t size;
   bool used;
};

extern struct utMem_ *utMems;
extern uint32 utfFreeMem, utfVirgMem_, utmMem;

#define ut0Mem UINT32_MAX
#define utmMem() utmMem
#define utfMem() 0
#define utnMem(mem) ((mem) + 1)
#define utfFreeMem() utfFreeMem
#define utnFreeMem(mem) (utMems[mem].nMem)
#define utnStackMem(stack, mem) (utMems[mem].nMem)
#define utfVirgMem() utfVirgMem_
#define uttMemExists(mem) ((mem) < utfVirgMem())
#define utgMemPicket(mem) (utMems[mem].picket)
#define utgMemPtr(mem) (utMems[mem].memPtr)
#define utgMemName(mem) (utMems[mem].name)
#define utgMemLine(mem) (utMems[mem].line)
#define utgMemSize(mem) (utMems[mem].size)
#define utoStackMem(mem) (utMems[mem].oStack)
#define uttMemUsed(mem) (utMems[mem].used)
extern utMemRef utcMem(void);
extern void utdMem(utMemRef mem, char *fileName, uint32 line);
extern utMemRef utBuildMem(void *memPtr, size_t size, char *file, uint32 line);
extern utMemRef utqMemPtr(void *memPtr);

/*--------------------------------------------------------------------------------------------------
   Object : utStack
   Purpose: Memory handle stack.  This allows us to keep track of all memory
            allocations past a mark, and have multiple levels of marks.
            When stacks are created, all new utMems are assigned to them.
            When a stack is deleted, an error is reported if any mems still
            exists for the stack.  Errors are also reported if stacks are
            deleted out of order.
--------------------------------------------------------------------------------------------------*/
struct utStack_ {
   utMemRef fMem;
   char *name;
   uint32 line;
};

extern struct utStack_ *utStacks;
extern uint32 utsStack, utmStack;

#define ut0Stack UINT32_MAX
#define utfStack() 0
#define utlStack() ((utStackRef)((int16)utsStack() - 1))
#define utsStack() utsStack
#define utmStack() utmStack
#define utnStack(stack) ((stack) + 1)
#define uttStackExists(stack) ((stack) < utsStack())
#define utfStackMem(stack) (utStacks[stack].fMem)
#define utgStackName(stack) (utStacks[stack].name)
#define utgStackLine(stack) (utStacks[stack].line)
extern utStackRef utcStackTrace(char *fileName, uint32 line);
extern void utdStackTrace(utStackRef stack, char *fileName,
      uint32 line);
#define utcStack() utcStackTrace(__FILE__, __LINE__)
#define utdStack(stack) utdStackTrace(stack, __FILE__, __LINE__)
extern void utaStackMem(utStackRef stack, utMemRef mem);
extern void utdStackMem(utStackRef stack, utMemRef mem,
      char *fileName, uint32 line);

extern void utMemStart(void);
extern void utMemStop(bool reportMemory);
extern void utMemCheckTrace(char *fileName, uint32 line);
#define utMemCheck() utMemCheckTrace(__FILE__, __LINE__)

#define mtResizeArray(array, num) \
        ((array) = realloc((void *)(array), (num), sizeof(*(array))))
#define mtNew(type) (type *)calloc(1, sizeof(type))
#define mtNewA(type, num) (type *)calloc((num), sizeof(type))

#if __cplusplus
}
#endif

#endif
