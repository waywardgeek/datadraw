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
  This module routes the schema.  There is a grid consisting of "Bundles", or bunds representing
  locations in the grid.  There are horizontal and vertical tracks of bunds that overlap.  In
  horizontal tracks, adjacencies are build horizontally, but not vertically.  When horizontal
  tracks cross vertical tracks, there are adjacencies between the overlapping horizontal and
  vertical bundles.
--------------------------------------------------------------------------------------------------*/
#include "dw.h"

static uint32 dwOverconstraints;
static uint32 dwRows, dwCols;
static uint32 dwOverconstraintPenalty;
static uint32 dwMaxBucket;
static dvClass dwSourceClass, dwTargetClass;
static dwBundArray dwRandBunds;
static dwRelationshipArray dwRandRelationships;

#define dwIndexHBund(x, y) dwIndex2Bund(dwCols*(y) + (x) + 1)
#define dwIndexVBund(x, y) dwIndex2Bund(dwRows*dwCols + dwRows*(x) + (y) + 1)

/* Bund adj iterator */
#define dwFindAdjOtherBund(adj, bund) (dwAdjGetFromBund(adj) == (bund) ? \
    dwAdjGetToBund(adj) : dwAdjGetFromBund(adj))
#define dwBundGetFirstAdj(bund) (dwBundGetFirstInAdj(bund) != dwAdjNull? \
    dwBundGetFirstInAdj(bund) : dwBundGetFirstOutAdj(bund))
#define dwBundGetNextBundAdj(bund, adj) (dwAdjGetToBund(adj) == (bund)? \
    (dwAdjGetNextBundInAdj(adj) != dwAdjNull? \
    dwAdjGetNextBundInAdj(adj) : dwBundGetFirstOutAdj(bund)) : \
    dwAdjGetNextBundOutAdj(adj))
#define dwForeachBundAdj(bund, adj) \
    for(adj = dwBundGetFirstAdj(bund); (adj != dwAdjNull); \
    adj = dwBundGetNextBundAdj(bund, adj))
#define dwEndBundAdj

/*--------------------------------------------------------------------------------------------------
  Create a new bundle.
--------------------------------------------------------------------------------------------------*/
static dwBund dwBundCreate(
    uint32 x,
    uint32 y)
{
    dwBund bund = dwBundAlloc();

    dwBundSetX(bund, x);
    dwBundSetY(bund, y);
    return bund;
}

/*--------------------------------------------------------------------------------------------------
  Create a new adj.
--------------------------------------------------------------------------------------------------*/
static dwAdj dwAdjCreate(
    dwBund bund1,
    dwBund bund2)
{
    dwAdj adj = dwAdjAlloc();

    dwBundInsertOutAdj(bund1, adj);
    dwBundInsertInAdj(bund2, adj);
    return adj;
}

/*--------------------------------------------------------------------------------------------------
  Build the routing graph of horizontal and vertical tracks of bundles.  We create first horizontal
  bundles from lower left to the top, then vertical tracks from lower left to the right.

    gridsPerCol = dwSchemaGetColWidth(schema)
    gridsPerRow = dwSchemaGetRowHeight(schema)
    cols = (dwSchemaGetCols(schema) + 1)*dwGridsPerCol
    rows = (dwSchemaGetRows(schema) + 1)*dwGridsPerRow
    hBundIndex = bundY*cols + bundX
    vBundIndex = rows*cols + bundX*rows + bundY
--------------------------------------------------------------------------------------------------*/
static void buildRoutingGraph(
    dvSchema schema)
{
    dwBund bund, prevBund, hBund;
    uint32 gridsPerCol = dwSchemaGetColWidth(schema);
    uint32 gridsPerRow = dwSchemaGetRowHeight(schema);
    uint32 x, y;

    dwCols = (dwSchemaGetCols(schema) + 1)*gridsPerCol;
    dwRows = (dwSchemaGetRows(schema) + 1)*gridsPerRow;
    /* Build horizontal tracks */
    for(y = 0; y < dwRows; y++) {
        prevBund = dwBundNull;
        for(x = 0; x < dwCols; x++) {
            bund = dwBundCreate(x, y);
            if(prevBund != dwBundNull) {
                dwAdjCreate(prevBund, bund);
            }
            prevBund = bund;
        }
    }
    /* Build vertical tracks */
    for(x = 0; x < dwCols; x++) {
        prevBund = dwBundNull;
        for(y = 0; y < dwRows; y++) {
            bund = dwBundCreate(x, y);
            if(prevBund != dwBundNull) {
                dwAdjCreate(prevBund, bund);
            }
            /* Connect this vertical bund to it's overlapping horizontal bund */
            hBund = dwIndexHBund(x, y);
            dwAdjCreate(bund, hBund);
            dwBundSetOverlappingBund(bund, hBund);
            dwBundSetOverlappingBund(hBund, bund);
            prevBund = bund;
        }
    }
}

/*--------------------------------------------------------------------------------------------------
  Add the bundle to the class, and then block all adjacencies to the bundle, exept in the direction
  specified.
--------------------------------------------------------------------------------------------------*/
static void addClassBund(
    dvClass theClass,
    dwBund bund,
    uint32 x,
    uint32 y)
{
    dwBund otherBund;
    dwAdj adj;

    dwClassInsertBund(theClass, bund);
    dwForeachBundAdj(bund, adj) {
        otherBund = dwFindAdjOtherBund(adj, bund);
        if(dwBundGetX(otherBund) != x || dwBundGetY(otherBund) != y) {
            dwAdjSetBlocked(adj, true);
        }
    } dwEndBundAdj;
}

/*--------------------------------------------------------------------------------------------------
  Add the Class --> Bund relationship for this class.  Also set the class's bounding box in grid
  units.
--------------------------------------------------------------------------------------------------*/
static void addClassToGraph(
    dvSchema schema,
    dvClass theClass)
{
    dwBund bund;
    uint32 width = dwClassGetWidth(theClass);
    uint32 height = dwClassGetHeight(theClass);
    uint32 gridsPerCol = dwSchemaGetColWidth(schema);
    uint32 gridsPerRow = dwSchemaGetRowHeight(schema);
    uint32 x = (dwClassGetX(theClass) + 1)*gridsPerCol - (width >> 1);
    uint32 y = (dwClassGetY(theClass) + 1)*gridsPerRow - (height >> 1);
    uint32 dX, dY;

    /* In case we had a previous run, the class could be pointing to a deleted bundle */
    dwClassSetFirstBund(theClass, dwBundNull);
    dwClassSetLeft(theClass, x);
    dwClassSetBottom(theClass, y);
    dwClassSetRight(theClass, x + width);
    dwClassSetTop(theClass, y + height);
    /* Add the top and bottom to the vertical grid. */
    for(dX = 0; dX <= width; dX++) {
        bund = dwIndexVBund(x + dX, y);
        addClassBund(theClass, bund, x + dX, y - 1);
        bund = dwIndexVBund(x + dX, y + height);
        addClassBund(theClass, bund, x + dX, y + height + 1);
    }
    /* Add the left and right to the horizontal grid. */
    for(dY = 0; dY <= height; dY++) {
        bund = dwIndexHBund(x, y + dY);
        addClassBund(theClass, bund, x - 1, y + dY);
        bund = dwIndexHBund(x + width, y + dY);
        addClassBund(theClass, bund, x + width + 1, y + dY);
    }
}

/*--------------------------------------------------------------------------------------------------
  Mark bundles within 2 adjs of a class as nearby.  This effects the cost for routing.
--------------------------------------------------------------------------------------------------*/
static void markBundsNearClass(
    dvClass theClass)
{
    dwBund bund1, bund2;
    dwAdj adj1;

    dwForeachClassBund(theClass, bund1) {
        dwForeachBundAdj(bund1, adj1) {
            bund2 = dwFindAdjOtherBund(adj1, bund1);
            dwBundSetNearClass(bund2, true);
        } dwEndBundAdj;
    } dwEndClassBund;
}

/*--------------------------------------------------------------------------------------------------
  Build the Class --> Bund relationship for all classes in the schema.
--------------------------------------------------------------------------------------------------*/
static void addClassesToGraph(
    dvSchema schema)
{
    dvClass theClass;

    dwForeachClassArrayClass(dwClasses, theClass) {
        addClassToGraph(schema, theClass);
        markBundsNearClass(theClass);
    } dwEndClassArrayClass;
}

/*--------------------------------------------------------------------------------------------------
  Add the bundle to the bunket with the given cost.
--------------------------------------------------------------------------------------------------*/
static void addBucketBund(
    uint32 cost,
    dwBund bund)
{
    dwBucket bucket;

    utAssert(dwBundGetClass(bund) == dwSourceClass || dwBundGetPrevBund(bund) != dwBundNull);
    while(cost + 1 >= dwUsedBucket()) {
        (void)dwBucketAlloc();
    }
    bucket = dwIndex2Bucket(cost + 1);
    dwBucketInsertBund(bucket, bund);
    dwMaxBucket = utMax(cost, dwMaxBucket);
}

/*--------------------------------------------------------------------------------------------------
  Find a bundle cost.  It's more expensive to be overconstrained, or close to a class or another
  relationship.
--------------------------------------------------------------------------------------------------*/
static uint32 findBundCost(
    dwBund bund)
{
    dwBund overlappingBund = dwBundGetOverlappingBund(bund);
    uint32 cost = 0;

    if(dwBundGetFirstRoute(bund) != dwRouteNull) {
        cost = dwOverconstraintPenalty;
    }
    if(dwBundGetFirstRoute(overlappingBund) != dwRouteNull) {
        cost++;
    }
    if(dwBundNearClass(bund) || dwBundNearClass(overlappingBund)) {
        cost += 4;
    }
    return cost;
}

/*--------------------------------------------------------------------------------------------------
  Initialize the wave on the source class.
--------------------------------------------------------------------------------------------------*/
static void initWave(void)
{
    dwBund bund;
    uint32 cost;

    dwForeachClassBund(dwSourceClass, bund) {
        cost = findBundCost(bund);
        dwBundSetPrevBund(bund, dwBundNull);
        addBucketBund(cost, bund);
    } dwEndClassBund;
}

/*--------------------------------------------------------------------------------------------------
  Initialize the wave on the source class for a self-relationship.  Just find a random starting
  point.
--------------------------------------------------------------------------------------------------*/
static void initSelfWave(void)
{
    dwBund bund, randSourceBund;
    uint32 xRandBund;

    dwBundArraySetUsedBund(dwRandBunds, 0);
    dwForeachClassBund(dwSourceClass, bund) {
        dwBundSetPrevBund(bund, dwBundNull);
        if(dwBundGetFirstRoute(bund) == dwRouteNull) {
            dwBundArrayAppendBund(dwRandBunds, bund);
        }
    } dwEndClassBund;
    xRandBund = utRandN(dwBundArrayGetUsedBund(dwRandBunds));
    randSourceBund = dwBundArrayGetiBund(dwRandBunds, xRandBund);
    addBucketBund(0, randSourceBund);
}

/*--------------------------------------------------------------------------------------------------
  Randomize bundles in the bucket, and add them to the dwRandBunds array.
--------------------------------------------------------------------------------------------------*/
static void randomizeBundArray(
    dwBundArray bunds)
{
    dwBund bund, otherBund;
    uint32 xBund, xOtherBund;
    uint32 numBunds = dwBundArrayGetUsedBund(bunds);

    for(xBund = 0; xBund < numBunds; xBund++) {
        xOtherBund = xBund + utRandN(numBunds - xBund);
        bund = dwBundArrayGetiBund(bunds, xBund);
        otherBund = dwBundArrayGetiBund(bunds, xOtherBund);
        dwBundArraySetiBund(bunds, xBund, otherBund);
        dwBundArraySetiBund(bunds, xOtherBund, bund);
    }
}

/*--------------------------------------------------------------------------------------------------
  Randomize bundles in the bucket, and add them to the dwRandBunds array.
--------------------------------------------------------------------------------------------------*/
static void randomizeBunds(
    dwBucket bucket)
{
    dwBund bund;

    dwBundArraySetUsedBund(dwRandBunds, 0);
    dwForeachBucketBund(bucket, bund) {
        dwBundArrayAppendBund(dwRandBunds, bund);
    } dwEndBucketBund;
    randomizeBundArray(dwRandBunds);
}

/*--------------------------------------------------------------------------------------------------
  Expand the bundle into nearest neighbors randomly.  If we find a target bund, return it.
--------------------------------------------------------------------------------------------------*/
static dwBund expandBund(
    dwBund bund)
{
    dvClass theClass;
    dwBund otherBund;
    dwAdj adj;
    uint32 cost = dwBucket2Index(dwBundGetBucket(bund)) + 2;
    uint32 bundCost;

    dwForeachBundAdj(bund, adj) {
        otherBund = dwFindAdjOtherBund(adj, bund);
        if(dwBundGetBucket(otherBund) == dwBucketNull && !dwAdjBlocked(adj)) {
            theClass = dwBundGetClass(otherBund);
            if(theClass == dwTargetClass) {
                dwBundSetPrevBund(otherBund, bund);
                return otherBund;
            }
            if(theClass == dvClassNull) {
                bundCost = cost + findBundCost(otherBund);
                dwBundSetPrevBund(otherBund, bund);
                addBucketBund(bundCost, otherBund);
            }
        }
    } dwEndBundAdj;
    return dwBundNull;
}

/*--------------------------------------------------------------------------------------------------
  Expand the bucket of bundles into nearest neighbors randomly.  Return any target we find.
--------------------------------------------------------------------------------------------------*/
static dwBund expandBucket(
    dwBucket bucket)
{
    dwBund bund, targetBund;

    randomizeBunds(bucket);
    dwForeachBundArrayBund(dwRandBunds, bund) {
        targetBund = expandBund(bund);
        if(targetBund != dwBundNull) {
            return targetBund;
        }
    } dwEndBundArrayBund;
    return dwBundNull;
}

/*--------------------------------------------------------------------------------------------------
  Expand the wave to the target.  Return the target we hit.
--------------------------------------------------------------------------------------------------*/
static dwBund expandToTarget(void)
{
    dwBucket bucket;
    dwBund target;

    uint32 xBucket = 1;

    while(true) {
        bucket = dwIndex2Bucket(xBucket);
        target = expandBucket(bucket);
        if(target != dwBundNull) {
            return target;
        }
        xBucket++;
    }
    return dwBundNull; /* Dummy return */
}

/*--------------------------------------------------------------------------------------------------
  Follow the back pointers from the target to source, building routes.
--------------------------------------------------------------------------------------------------*/
static dwRoute dwBuildRoute(
    dvRelationship relationship,
    dwBund bund)
{
    dwRoute route = dwRouteAlloc();

    if(dwBundGetFirstRoute(bund) != dwRouteNull) {
        dwOverconstraints++;
    }
    dwRelationshipInsertRoute(relationship, route);
    dwBundInsertRoute(bund, route);
    return route;
}

/*--------------------------------------------------------------------------------------------------
  Follow the back pointers from the target to source, building routes.
--------------------------------------------------------------------------------------------------*/
static void backtrace(
    dvRelationship relationship,
    dwBund bund)
{
    do {
        dwBuildRoute(relationship, bund);
        bund = dwBundGetPrevBund(bund);
    } while(bund != dwBundNull);
}

/*--------------------------------------------------------------------------------------------------
  Empty buckets so we can do another expansion.
--------------------------------------------------------------------------------------------------*/
static void emptyBuckets(void)
{
    dwBucket bucket;
    dwBund bund;
    uint32 xBucket;

    for(xBucket = 0; xBucket <= dwMaxBucket; xBucket++) {
        bucket = dwIndex2Bucket(xBucket + 1);
        utDo {
            bund = dwBucketGetFirstBund(bucket);
        } utWhile(bund != dwBundNull) {
            dwBucketRemoveBund(bucket, bund);
        } utRepeat;
    }
}

/*--------------------------------------------------------------------------------------------------
  Route a relationship.
--------------------------------------------------------------------------------------------------*/
static void routeRelationship(
    dvRelationship relationship)
{
    dwBund target;

    dwSourceClass = dvRelationshipGetParentClass(relationship);
    dwTargetClass = dvRelationshipGetChildClass(relationship);
    dwMaxBucket = 0;
    if(dwSourceClass != dwTargetClass) {
        initWave();
    } else {
        initSelfWave();
    }
    target = expandToTarget();
    backtrace(relationship, target);
    emptyBuckets();
}

/*--------------------------------------------------------------------------------------------------
  Create an initial solution, where routes just overlap without penalty.
--------------------------------------------------------------------------------------------------*/
static void initialSolution(
    dvSchema schema)
{
    dvRelationship relationship;


    dvForeachSchemaRelationship(schema, relationship) {
        routeRelationship(relationship);
    } dvEndSchemaRelationship;
}

/*--------------------------------------------------------------------------------------------------
  Rip a route
--------------------------------------------------------------------------------------------------*/
static void dwRipRoute(
    dwRoute route)
{
    dwBund bund = dwRouteGetBund(route);
    dwRoute firstRoute;

    dwRouteDestroy(route);
    firstRoute = dwBundGetFirstRoute(bund);
    if(firstRoute != dwRouteNull) {
        dwOverconstraints--;
    }
}

/*--------------------------------------------------------------------------------------------------
  Randomize the relationships order
--------------------------------------------------------------------------------------------------*/
static void randomizeRelationships()
{
    dvRelationship relationship1, relationship2;
    uint32 temp1, temp2;
    uint32 numRelationships = dwRelationshipArrayGetUsedRelationship(dwRandRelationships);

    for(temp1 = 0; temp1 < numRelationships; temp1++) {
        temp2 = temp1 + utRandN(numRelationships - temp1);
        relationship1 = dwRelationshipArrayGetiRelationship(dwRandRelationships, temp1);
        relationship2 = dwRelationshipArrayGetiRelationship(dwRandRelationships, temp2);
        dwRelationshipArraySetiRelationship(dwRandRelationships, temp1, relationship2);
        dwRelationshipArraySetiRelationship(dwRandRelationships, temp2, relationship1);
    }
}

/*--------------------------------------------------------------------------------------------------
  Rip all routes of a bundle
--------------------------------------------------------------------------------------------------*/
static void dwRipRelationshipRoutes(
    dvRelationship relationship)
{
    dwRoute route;

    dwSafeForeachRelationshipRoute(relationship, route) {
        dwRipRoute(route);
    } dwEndSafeRelationshipRoute;
}

/*--------------------------------------------------------------------------------------------------
  Rip-up and reroute at least a few times to improve readability, or continue until DRCs are gone
  or we give up.
--------------------------------------------------------------------------------------------------*/
static void fillRelationshipArray(
    dvSchema schema)
{
    dvRelationship relationship;

    dvForeachSchemaRelationship(schema, relationship) {
        dwRelationshipArrayAppendRelationship(dwRandRelationships, relationship);
    } dvEndSchemaRelationship;

}

/*--------------------------------------------------------------------------------------------------
  Rip-up and reroute at least a few times to improve readability, or continue until DRCs are gone
  or we give up.
--------------------------------------------------------------------------------------------------*/
static void ripUpandReroute(
    dvSchema schema)
{
    dvRelationship relationship;

    randomizeRelationships();
    dwForeachRelationshipArrayRelationship(dwRandRelationships, relationship) {
        dwRipRelationshipRoutes(relationship);
        routeRelationship(relationship);
    } dwEndRelationshipArrayRelationship;
}

/*--------------------------------------------------------------------------------------------------
  Route Until Completion
--------------------------------------------------------------------------------------------------*/
static void routeUntilComplete(
    dvSchema schema)
{
    uint32 dwCycle = 0;

    fillRelationshipArray(schema);
    do{
        utIfDebug(1) {
            utLogDebug("cycles = %d overconstraints = %d", dwCycle, dwOverconstraints);
        }
        dwOverconstraintPenalty++;
        ripUpandReroute(schema);
        dwCycle++;
    } while((dwCycle < 10 || dwOverconstraints != 0) && dwCycle < 100);
}

/*--------------------------------------------------------------------------------------------------
  Route relationships on the schema.  Return the number of DRCs.
--------------------------------------------------------------------------------------------------*/
uint32 dwRouteSchema(
    dvSchema schema)
{
    dwRandRelationships = dwRelationshipArrayAlloc();
    dwBucketFreeAll();
    dwBundFreeAll();
    dwAdjFreeAll();
    dwRandBunds = dwBundArrayAlloc();
    buildRoutingGraph(schema);
    addClassesToGraph(schema);
    dwOverconstraints = 0;
    dwOverconstraintPenalty = 0;
    initialSolution(schema);
    routeUntilComplete(schema);
    dwBundArrayFree(dwRandBunds);
    dwRelationshipArrayFree(dwRandRelationships);
    return dwOverconstraints;
}
