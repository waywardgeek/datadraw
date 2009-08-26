/*
 * Copyright (C) 2007 Bill Cox
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

/*--------------------------------------------------------------------------------------------------
  This module places classes into rows in the schema.  The algorithm is basically:

  - Assign classes to rows, starting with classes without parents, and assigning their children
    to the next row, and so on
  - Assign relative positions within rows to reduce crossings as much as possible
  - Assign exact position to minimize total length of horizontal routing
--------------------------------------------------------------------------------------------------*/
#include <stdlib.h>
#include "dw.h"

static dwClassArray dwGrid = dwClassArrayNull;
static uint32 dwNumCols = UINT32_MAX; 
dwClassArray dwClasses = dwClassArrayNull;
static uint32 dwCost;
static uint32 dwUndoX1 = UINT32_MAX;
static uint32 dwUndoX2 = UINT32_MAX;
static uint32 dwUndoY1 = UINT32_MAX;
static uint32 dwUndoY2 = UINT32_MAX;
static uint32 dwRange;
static dvSchema dwCurrentSchema;

/*--------------------------------------------------------------------------------------------------
  Initialize globals.
--------------------------------------------------------------------------------------------------*/
static void init(
    dvSchema schema)
{
    dvRelationship relationship;
    dvClass theClass;
    uint32 size;
    
    dwCurrentSchema = schema;
    dwCost = 0;
    dwClassArraySetUsedClass(dwClasses, 0);
    dvForeachSchemaRelationship(schema, relationship) {
        dwClassSetAdded(dvRelationshipGetParentClass(relationship), false);
        dwClassSetAdded(dvRelationshipGetChildClass(relationship), false);
    } dvEndSchemaRelationship;
    dvForeachSchemaRelationship(schema, relationship) {
        theClass = dvRelationshipGetParentClass(relationship);
        if(!dwClassAdded(theClass)) {
            dwClassSetAdded(theClass, true);
            dwClassArrayAppendClass(dwClasses, theClass);
        }
        theClass = dvRelationshipGetChildClass(relationship);
        if(!dwClassAdded(theClass)) {
            dwClassSetAdded(theClass, true);
            dwClassArrayAppendClass(dwClasses, theClass);
        }
    }  dvEndSchemaRelationship;
    dwForeachClassArrayClass(dwClasses, theClass) {
        dwClassSetAdded(theClass, false);
    } dwEndClassArrayClass;
    size = dwClassArrayGetUsedClass(dwClasses);
    size = size*size;
    dwNumCols = dwClassArrayGetUsedClass(dwClasses);
    dwGrid = dwClassArrayAlloc();
    dwClassArrayAllocClasss(dwGrid, size);
    dwUndoX1 = UINT32_MAX;
    dwUndoX2 = UINT32_MAX;
    dwUndoY1 = UINT32_MAX;
    dwUndoY2 = UINT32_MAX;
}

/*--------------------------------------------------------------------------------------------------
  Free Globals.
--------------------------------------------------------------------------------------------------*/
static void close(void)
{
    dwClassArrayFree(dwGrid);
    dwGrid = dwClassArrayNull;
    dwNumCols = UINT32_MAX;
    dwUndoX1 = UINT32_MAX;
    dwUndoX2 = UINT32_MAX;
    dwUndoY1 = UINT32_MAX;
    dwUndoY2 = UINT32_MAX;
}

/*--------------------------------------------------------------------------------------------------
  Find the cost of relationships for classes within a schema
--------------------------------------------------------------------------------------------------*/
static uint32 findRelationshipCost(
    dvRelationship relationship)
{
    dvClass parentClass = dvRelationshipGetParentClass(relationship);
    dvClass childClass = dvRelationshipGetChildClass(relationship);
    int32 parentX = dwClassGetX(parentClass);
    int32 parentY = dwClassGetY(parentClass);
    int32 childX = dwClassGetX(childClass);
    int32 childY = dwClassGetY(childClass);
    uint32 relationshipCost = (utAbs(parentX - childX) + utAbs(parentY - childY)) << 1;

    if(parentClass == childClass) {
        return 0;
    }
    if(!dwClassAdded(parentClass)) {
        return 0;
    }
    if(!dwClassAdded(childClass)) {
        return 0;
    }
    if(childX < parentX) {
        relationshipCost++;
    }
    if(childY > parentY) {
        relationshipCost++;
    } else if(childX == parentX) {
        relationshipCost--;
    }
    /* Add a slight preference for vertical rather than horizontal layout */
    return relationshipCost;
}

/*--------------------------------------------------------------------------------------------------
  Find the cost of all the  classes in relationship with a class
--------------------------------------------------------------------------------------------------*/
static uint32 findClassCost(
    dvClass theClass)
{
    uint32 cost = 0;
    dvRelationship relationship;

    dvForeachClassParentRelationship(theClass, relationship) {
        if(dvRelationshipGetSchema(relationship) == dwCurrentSchema) {
            cost += findRelationshipCost(relationship);
        }
    } dvEndClassParentRelationship;
    dvForeachClassChildRelationship(theClass, relationship) {
        if(dvRelationshipGetSchema(relationship) == dwCurrentSchema) {
            cost += findRelationshipCost(relationship);
        }
    } dvEndClassChildRelationship;
    return cost;
}

/*--------------------------------------------------------------------------------------------------
  Find the class at location X and Y
--------------------------------------------------------------------------------------------------*/
static dvClass getGridXYClass(
    uint32 x, 
    uint32 y)
{
    uint32 index = y*dwNumCols + x; 
    dvClass theClass = dwClassArrayGetiClass(dwGrid, index); 

    return theClass;
}

/*--------------------------------------------------------------------------------------------------
  Set the class at location X and Y
--------------------------------------------------------------------------------------------------*/
static void setGridXYClass(
    uint32 x, 
    uint32 y,
    dvClass theClass)
{
    uint32 index = y*dwNumCols + x;

    dwClassArraySetiClass(dwGrid, index, theClass);
}

/*--------------------------------------------------------------------------------------------------
  Updating cost while removing Class from a Grid
--------------------------------------------------------------------------------------------------*/
static void removeGridClass(
     dvClass theClass)
{
    uint32 classCost = findClassCost(theClass);
    uint32 x = dwClassGetX(theClass);
    uint32 y = dwClassGetY(theClass); 

    utAssert(getGridXYClass(x, y) == theClass); 
    utAssert(classCost <= dwCost);
    utAssert(dwClassAdded(theClass));
    setGridXYClass(x, y, dvClassNull);
    dwClassSetAdded(theClass, false);
    dwCost -= classCost;  
}

/*--------------------------------------------------------------------------------------------------
  Updating cost while adding Class to a Grid
--------------------------------------------------------------------------------------------------*/
void addGridClass(
    dvClass theClass, 
    uint32 x, 
    uint32 y)
{
    utAssert(dwClassAdded(theClass) == false);
    utAssert(getGridXYClass(x,y) == dvClassNull);
    setGridXYClass(x, y, theClass);
    dwClassSetX(theClass, x);
    dwClassSetY(theClass, y);
    dwClassSetAdded(theClass, true);
    dwCost += findClassCost(theClass); 
}

/*--------------------------------------------------------------------------------------------------
  Swapping classes between coordinates X1,Y1 and X2,Y2
--------------------------------------------------------------------------------------------------*/
static void swapGridXY(
    uint32 x1,
    uint32 y1,
    uint32 x2,
    uint32 y2)
{
    dvClass class1 = getGridXYClass(x1, y1);
    dvClass class2 = getGridXYClass(x2, y2);

    if(class1 != dvClassNull && class2 == dvClassNull) {
        removeGridClass(class1); 
        addGridClass(class1, x2, y2); 
    } else if(class1 == dvClassNull && class2 != dvClassNull) {
        removeGridClass(class2);
        addGridClass(class2, x1, y1);
    } else if(class1!=dvClassNull && class2 != dvClassNull) {
        removeGridClass(class1);
        removeGridClass(class2);
        addGridClass(class1, x2, y2);
        addGridClass(class2, x1, y1);
    }
}

/*--------------------------------------------------------------------------------------------------
  Picking a Random Class
--------------------------------------------------------------------------------------------------*/
static dvClass pickRandomClass(void)
{
    uint32 classnum = dwClassArrayGetUsedClass(dwClasses);
    dvClass theClass = dwClassArrayGetiClass(dwClasses,utRandN(classnum));
 
    return theClass;
}

/*--------------------------------------------------------------------------------------------------
  Function to make a Initialize  
--------------------------------------------------------------------------------------------------*/
void anInitializeCycle(
    double temperature)
{
    dwRange = utMax(temperature, 1);
    utIfDebug(1) {
        printf("temperature = %.2f, cost = %u\n", temperature, dwCost);
    }
}

/*--------------------------------------------------------------------------------------------------
  Function to Find Random dest  
--------------------------------------------------------------------------------------------------*/
static uint32 findRandomDest(
    uint32 x)
{
    int32 position;

    do {
        position = x - dwRange + utRandN((dwRange << 1) + 1);
    } while(position < 0 || position >= (int32)dwNumCols);
    return position;
}

/*--------------------------------------------------------------------------------------------------
  Function to make a Random Move  
--------------------------------------------------------------------------------------------------*/
static int32 makeRandomMoveWithClass(
     dvClass class1)
{
    uint32 x1 = dwClassGetX(class1); 
    uint32 y1 = dwClassGetY(class1); 
    uint32 x2 = findRandomDest(x1);
    uint32 y2 = findRandomDest(y1);
    int32 initialCost = dwCost;
    int32 deltaTemp = 0;

    dwUndoX1 = x1;
    dwUndoX2 = x2;
    dwUndoY1 = y1;
    dwUndoY2 = y2;
    if(x1 != x2 || y1 != y2) {
         swapGridXY(x1, y1, x2, y2); 
         deltaTemp = dwCost - initialCost; 
    }
    return deltaTemp;
}

/*--------------------------------------------------------------------------------------------------
  Function to make a Random Move  
--------------------------------------------------------------------------------------------------*/
int32 anMakeRandomMove(void)
{
    dvClass theClass = pickRandomClass();
    
    return makeRandomMoveWithClass(theClass);
}

/*--------------------------------------------------------------------------------------------------
  Undoing a Move
--------------------------------------------------------------------------------------------------*/
void anUndoMove(void)
{

    if (dwUndoX1 == UINT32_MAX || dwUndoX2 == UINT32_MAX || dwUndoY1==UINT32_MAX || 
        dwUndoY2==UINT32_MAX) {
    } else {
        swapGridXY(dwUndoX1,dwUndoY1,dwUndoX2,dwUndoY2);
        dwUndoX1 = UINT32_MAX;
        dwUndoX2 = UINT32_MAX;
        dwUndoY1 = UINT32_MAX;
        dwUndoY2 = UINT32_MAX; 
    }
}

/*--------------------------------------------------------------------------------------------------
  Initialize Systems
--------------------------------------------------------------------------------------------------*/
int32 anInitializeSystem(uint32 *numElements)
{
    uint32 i = 0;
    dvClass theClass; 

    for(i = 0; i < dwNumCols; i++) {
        theClass = dwClassArrayGetiClass(dwClasses, i);
        addGridClass(theClass, i, 0);
    }
    *numElements = dwNumCols;
    return dwCost;
}

/*--------------------------------------------------------------------------------------------------
  Function to randomize system
--------------------------------------------------------------------------------------------------*/
int32 anRandomizeSystem(void)
{
    dvClass theClass;
    uint32 i;
    
    for(i = 0; i < dwNumCols; i++) {
        theClass = dwClassArrayGetiClass(dwClasses, i);
        makeRandomMoveWithClass(theClass);
    }
    return dwCost; 
}

/*--------------------------------------------------------------------------------------------------
  Compare class X positions.
--------------------------------------------------------------------------------------------------*/
static int compareClassX(
    const void *classPtr1,
    const void *classPtr2)
{
    dvClass class1 = *(dvClass *)classPtr1;
    dvClass class2 = *(dvClass *)classPtr2;

    return dwClassGetX(class1) - dwClassGetX(class2);
}

/*--------------------------------------------------------------------------------------------------
  Compare class Y positions.
--------------------------------------------------------------------------------------------------*/
static int compareClassY(
    const void *classPtr1,
    const void *classPtr2)
{
    dvClass class1 = *(dvClass *)classPtr1;
    dvClass class2 = *(dvClass *)classPtr2;

    return dwClassGetY(class1) - dwClassGetY(class2);
}

/*--------------------------------------------------------------------------------------------------
  Post-process the placement to clean it up a bit.  Remove blank rows and columns, and determine
  class height and widht.
--------------------------------------------------------------------------------------------------*/
static void stripEmptyRowsAndColumns(
    dvSchema schema)
{
    dvClass theClass;
    int32 prevPosition = -1;
    int32 x, y;
    uint32 deltaSpace = 0;

    qsort(dwClassArrayGetClasss(dwClasses), dwClassArrayGetUsedClass(dwClasses), sizeof(dvClass),
        compareClassX);
    dwForeachClassArrayClass(dwClasses, theClass) {
        x = dwClassGetX(theClass);
        if(x > prevPosition + 1) {
            deltaSpace += x - prevPosition - 1;
        }
        dwClassSetX(theClass, x - deltaSpace);
        prevPosition = x;
    } dwEndClassArrayClass;
    dwSchemaSetCols(schema, prevPosition - deltaSpace + 1);
    prevPosition = -1;
    deltaSpace = 0;
    qsort(dwClassArrayGetClasss(dwClasses), dwClassArrayGetUsedClass(dwClasses), sizeof(dvClass),
        compareClassY);
    dwForeachClassArrayClass(dwClasses, theClass) {
        y = dwClassGetY(theClass);
        if(y > prevPosition + 1) {
            deltaSpace += y - prevPosition - 1;
        }
        dwClassSetY(theClass, y - deltaSpace);
        prevPosition = y;
    } dwEndClassArrayClass;
    dwSchemaSetRows(schema, prevPosition - deltaSpace + 1);
}

/*--------------------------------------------------------------------------------------------------
  Count the number of relationships on a class.
--------------------------------------------------------------------------------------------------*/
static uint32 countClassRelationshipsInSchema(
    dvClass theClass,
    dvSchema schema)
{
    dvRelationship relationship;
    uint32 numRelationships = 0;

    dvForeachClassParentRelationship(theClass, relationship) {
        if(dvRelationshipGetSchema(relationship) == schema) {
            numRelationships++;
        }
    } dvEndClassParentRelationship;
    dvForeachClassChildRelationship(theClass, relationship) {
        if(dvRelationshipGetSchema(relationship) == schema) {
            numRelationships++;
        }
    } dvEndClassChildRelationship;
    return numRelationships;
}

/*--------------------------------------------------------------------------------------------------
  Set class heights and widths.  Also, set the schema row height and column width.
--------------------------------------------------------------------------------------------------*/
static void setClassHeightAndWidths(
    dvSchema schema)
{
    dvClass theClass;
    uint32 nameLength, numRelationships;
    uint32 width, relationshipWidth;
    uint32 rowHeight = 1 + DW_ROUTING_TRACKS;
    uint32 colWidth = 3 + DW_ROUTING_TRACKS;

    dwForeachClassArrayClass(dwClasses, theClass) {
        dwClassSetHeight(theClass, 1);
        nameLength = strlen(dwClassGetName(theClass));
        width = utMax(3, nameLength >> 1);
        numRelationships = countClassRelationshipsInSchema(theClass, schema);
        relationshipWidth = (numRelationships + 1) >> 1;
        width = utMax(width, relationshipWidth);
        dwClassSetWidth(theClass, width);
        colWidth = utMax(colWidth, width + DW_ROUTING_TRACKS);
    } dwEndClassArrayClass;
    dwSchemaSetRowHeight(schema, rowHeight);
    dwSchemaSetColWidth(schema, colWidth);
}

/*--------------------------------------------------------------------------------------------------
  Post-process the placement to clean it up a bit.  Remove blank rows and columns, and determine
  class height and width.
--------------------------------------------------------------------------------------------------*/
static void postProcess(
    dvSchema schema)
{
    stripEmptyRowsAndColumns(schema);
    setClassHeightAndWidths(schema);
}

/*--------------------------------------------------------------------------------------------------
  Place classes on the schema.
--------------------------------------------------------------------------------------------------*/
void dwPlaceSchema(
    dvSchema schema)
{
    init(schema);
    anAnneal(0.999);
    postProcess(schema);
    close();
}
