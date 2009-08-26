/*--------------------------------------------------------------------------------------------------
  Anneal a system to a low cost.
--------------------------------------------------------------------------------------------------*/
#include <math.h>
#include <stdlib.h>
#include "dv.h"
#include "dw.h"
#define AN_RANDOM_SYSTEMS 32
#define AN_MOVES_PER_ELEMENT 20
#define AN_FREEZE_TEMP 0.1

double anTemperature;
int64 anCost;

/*--------------------------------------------------------------------------------------------------
  Find the standard deviation of random systems.
--------------------------------------------------------------------------------------------------*/
static double findInitialTemperature(void) {
    int64 costs[AN_RANDOM_SYSTEMS];
    int64 totalCost = 0;
    int64 totalOfSquares = 0;
    int64 averageCost, difference;
    uint16 xRandomSystems;
    
    for(xRandomSystems = 0; xRandomSystems < AN_RANDOM_SYSTEMS;
        xRandomSystems++) {
        anCost = anCost + anRandomizeSystem(); 
    }
    for(xRandomSystems = 0; xRandomSystems < AN_RANDOM_SYSTEMS; xRandomSystems++) {
        anCost = anCost + anRandomizeSystem(); 
        costs[xRandomSystems] = anCost;
        totalCost = totalCost + costs[xRandomSystems];
    }
    averageCost = totalCost/AN_RANDOM_SYSTEMS;
    for(xRandomSystems = 0; xRandomSystems < AN_RANDOM_SYSTEMS; xRandomSystems++) {
        difference = costs[xRandomSystems] - averageCost;
        totalOfSquares = totalOfSquares + difference*difference;
    }
    return sqrt(totalOfSquares/AN_RANDOM_SYSTEMS);
}

/*--------------------------------------------------------------------------------------------------
  Anneal a system to a low cost.
--------------------------------------------------------------------------------------------------*/
void anAnneal(
    double coolingFactor)
{
    uint32 xMove, movesPerCycle;
    uint32 numElements;
    int64 deltaCost;

    anCost = anInitializeSystem(&numElements);
    movesPerCycle = numElements*AN_MOVES_PER_ELEMENT;
    anTemperature = findInitialTemperature();
    if(numElements > 0) {
        do {
            anInitializeCycle(anTemperature);
            for (xMove = 0; xMove < movesPerCycle; xMove++) {
                deltaCost = anMakeRandomMove();
                if (deltaCost > 0 && (int32)(UINT16_MAX*exp(-deltaCost/anTemperature)) <= (int32)utRandN(UINT16_MAX)) {
                    anUndoMove();
                } else {
                    anCost = anCost + deltaCost;
                }
            }
            anTemperature *= coolingFactor; 
        } while(anTemperature > 0.1);
    }
}
