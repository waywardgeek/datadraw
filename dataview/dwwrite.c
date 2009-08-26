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

#include "dw.h"

static dwClassArray dwGrid = dwClassArrayNull;
static uint32 dwNumCols = UINT32_MAX; 
static dwClassArray dwClasses = dwClassArrayNull;
static uint32 dwCost;
static uint32 dwUndoX1 = UINT32_MAX;
static uint32 dwUndoX2 = UINT32_MAX;
static uint32 dwUndoY1 = UINT32_MAX;
static uint32 dwUndoY2 = UINT32_MAX;

/*--------------------------------------------------------------------------------------------------
  Initialize globals.
--------------------------------------------------------------------------------------------------*/
static void init(
    dvSchema schema)
{
    dvClass class;
    uint32 size;
    uint32 x;
    
    dwCost=0;
    dwClasses = dwClassArrayAlloc();
    dvForeachSchemaClass(schema, class) {
        dwClassArrayAppendClass(dwClasses, class);
    }  dvEndSchemaClass;
    size = dwClassArrayGetUsedClass(dwClasses);
    size = size*size;
    dwNumCols = dwClassArrayGetUsedClass(dwClasses);;
    dwGrid = dwClassArrayAlloc();
    for(x = 0; x < size; x++) {
        dwClassArrayAppendClass(dwGrid, dvClassNull);
    }
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
    dwClassArrayFree(dwClasses);
    dwClasses = dwClassArrayNull;
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
uint32 findRelationshipCost(
    dvRelationship relationship)
{
    dvClass parentClass = dvRelationshipGetParentClass(relationship);
    dvClass childClass = dvRelationshipGetChildClass(relationship);
    int32 parentX = dwClassGetX(parentClass);
    int32 parentY = dwClassGetY(parentClass);
    int32 childX = dwClassGetX(childClass);
    int32 childY = dwClassGetY(childClass);
    uint32 relationshipCost = utAbs(parentX - childX) + utAbs(parentY - childY);

    if(!dwClassAdded(parentClass)) {
        return 0;
    }
    if(!dwClassAdded(childClass)) {
        return 0;
    }
    return relationshipCost;
}

/*--------------------------------------------------------------------------------------------------
  Find the cost of all the  classes in relationship with a class
--------------------------------------------------------------------------------------------------*/
uint32 findClassCost(
    dvClass class)
{
    uint32 cost = 0;
    dvRelationship relationship;

    dvForeachClassParentRelationship(class,relationship) {
        cost += findRelationshipCost(relationship);
    } dvEndClassParentRelationship;
    dvForeachClassChildRelationship(class,relationship) {
        cost += findRelationshipCost(relationship);
    } dvEndClassChildRelationship;
    return cost;
}

/*--------------------------------------------------------------------------------------------------
  Find the class at location X and Y
--------------------------------------------------------------------------------------------------*/
dvClass getGridXYClass(
    uint32 x, 
    uint32 y)
{
    uint32 index = y*dwNumCols + x;
    dvClass class = dwClassArrayGetiClass(dwGrid, index);

    return class;
}

/*--------------------------------------------------------------------------------------------------
  Set the class at location X and Y
--------------------------------------------------------------------------------------------------*/
void setGridXYClass(
    uint32 x, 
    uint32 y,
    dvClass class)
{
    uint32 index = y*dwNumCols + x; 

    dwClassArraySetiClass(dwGrid, index, class);
}

/*--------------------------------------------------------------------------------------------------
  Updating cost while removing Class from a Grid
--------------------------------------------------------------------------------------------------*/

void removeGridClass(
     dvClass class)
{
    uint32 classCost = findClassCost(class);
    uint32 x = dwClassGetX(class);
    uint32 y = dwClassGetY(class);

    utAssert(getGridXYClass(x, y) == class); 
    utAssert(classCost <= dwCost);
    utAssert(dwClassAdded(class));
    setGridXYClass(x, y, dvClassNull);
    dwClassSetAdded(class, false);
    dwCost -= classCost;
  
}

/*--------------------------------------------------------------------------------------------------
  Updating cost while adding Class to a Grid
--------------------------------------------------------------------------------------------------*/
void addGridClass(
                dvClass class, 
                uint32 x, 
                uint32 y)
{
    utAssert(dwClassAdded(class) == false);
    utAssert(getGridXYClass(x,y) == dvClassNull);
    setGridXYClass(x, y, class);
    dwClassSetX(class, x);
    dwClassSetY(class, y);
    dwClassSetAdded(class, true);
    dwCost += findClassCost(class); 
}

/*--------------------------------------------------------------------------------------------------
  Swapping classes between coordinates X1,Y1 and X2,Y2
--------------------------------------------------------------------------------------------------*/
void SwapGridXY(
    uint32 x1,
    uint32 y1,
    uint32 x2,
    uint32 y2)
{
    dvClass temp1 = getGridXYClass(x1,y1);
    dvClass temp2 = getGridXYClass(x2,y1);

    if(temp1!=dvClassNull && temp2 == dvClassNull) {
        removeGridClass(temp1);
        addGridClass(temp1,x2,y2);
    } else if(temp1==dvClassNull && temp2 != dvClassNull) {
            removeGridClass(temp2);
            addGridClass(temp2,x1,y1);
    } else if(temp1!=dvClassNull && temp2 != dvClassNull) {
        removeGridClass(temp1);
        removeGridClass(temp2);
        addGridClass(temp1,x2,y2);
        addGridClass(temp2,x1,y1);
    } else if (temp1==dvClassNull && temp2 == dvClassNull) {
    }
}

/*--------------------------------------------------------------------------------------------------
  Picking a Random Class
--------------------------------------------------------------------------------------------------*/
dvClass pickRandomClass(void)
{
    uint32 classnum=dwClassArrayGetUsedClass(dwClasses);
    dvClass class=getGridXYClass(utRandN(classnum),utRandN(classnum));
 
    return class;
}

/*--------------------------------------------------------------------------------------------------
  Function to make a Random Move  
--------------------------------------------------------------------------------------------------*/
int32 makeRandomMoveWithClass(
     dvClass class1)
{
    uint32 x1 = dwClassGetX(class1);
    uint32 y1 = dwClassGetY(class1);
    uint32 x2 = utRandN(dwNumCols);
    uint32 y2 = utRandN(dwNumCols);
    int32 initialTemp=dwCost;
    int32 deltaTemp = 0;

    dwUndoX1 = x1;
    dwUndoX2 = x2;
    dwUndoY1 = y1;
    dwUndoY2 = y2;
    if ((x1 == x2) && (y1 == y2)) {
    } else {         
          SwapGridXY(x1,y1,x2,y2);
          deltaTemp = dwCost - initialTemp;
    }
    return deltaTemp;
}

/*--------------------------------------------------------------------------------------------------
  Function to make a Random Move  
--------------------------------------------------------------------------------------------------*/
int32 anMakeRandomMove(void)
{
    dvClass class = pickRandomClass();
    
    return makeRandomMoveWithClass(class);
}

/*--------------------------------------------------------------------------------------------------
  Undoing a Move
--------------------------------------------------------------------------------------------------*/
void anUndoMove(void)
{

    if (dwUndoX1 == UINT32_MAX || dwUndoX2 == UINT32_MAX || dwUndoY1==UINT32_MAX || dwUndoY2==UINT32_MAX) {
    }
    else{
    SwapGridXY(dwUndoX2,dwUndoX1,dwUndoY1,dwUndoY2);
    dwUndoX1 = UINT32_MAX;
    dwUndoX2 = UINT32_MAX;
    dwUndoY1 = UINT32_MAX;
    dwUndoY2 = UINT32_MAX; }
}

/*--------------------------------------------------------------------------------------------------
  Initialize Systems
--------------------------------------------------------------------------------------------------*/
int32 anInitializeSystem(
    uint32 *numElements)
{
    uint32 i=0;
    dvClass class; 

    for (i=0; i<dwNumCols; i++) {
       setGridXYClass(i,0,class);
    }
    *numElements=i;
    return dwCost;
}

/*--------------------------------------------------------------------------------------------------
  Function to randomize system
--------------------------------------------------------------------------------------------------*/
int32 anRandomizeSystem(void)
{
    dvClass class;
    uint32 i;
    
    for(i = 0;i < dwNumCols;i++) {
        class = dwClassArrayGetiClass(dwGrid, i);
        makeRandomMoveWithClass(class);
   }
    return dwCost; 
}

/*--------------------------------------------------------------------------------------------------
  Place classes on the schema.
--------------------------------------------------------------------------------------------------*/
void dwPlaceSchema(
    dvSchema schema)
{
    dvClass class;
    
    init(schema);
    anAnneal(0.9);
    close();
}
