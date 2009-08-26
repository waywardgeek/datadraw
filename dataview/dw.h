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

#include "../src/dvdatabase.h"
#include "dv.h"
#include "dwdatabase.h"

#define DW_ROUTING_TRACKS 6

void dwReadDatabases(int argc, char *argv[]);
void dwPlaceSchema(dvSchema schema);
uint32 dwRouteSchema(dvSchema schema);
void dwPrintSchemaClasses(dvSchema schema, bool optimizeForPrinting);
void dwViewSchemas(void);
void anAnneal(double coolingFactor);
void anInitializeCycle(double temperature);
int32 anInitializeSystem(uint32 *numElements);
void anUndoMove(void);
int32 anRandomizeSystem(void);
int32 anMakeRandomMove(void);
char *dwClassGetName(dvClass class);

/* Global variables */
extern dwClassArray dwClasses;
