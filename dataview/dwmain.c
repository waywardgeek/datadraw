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
#include <stdlib.h>
#include "dw.h"

/*--------------------------------------------------------------------------------------------------
  Place and route the schema.  If the viewer is not NULL, launch it on the output.
--------------------------------------------------------------------------------------------------*/
static void placeAndRouteSchema(
    dvSchema schema,
    bool optimizeForPrinting,
    char *viewer)
{
    dwPlaceSchema(schema);
    dwRouteSchema(schema);
    dwPrintSchemaClasses(schema, optimizeForPrinting);
}

/*--------------------------------------------------------------------------------------------------
  This tool generates a class diagram showing classes and their relationships.
--------------------------------------------------------------------------------------------------*/
static void placeAndRouteSchemas(
    utSym schemaSym,
    bool optimizeForPrinting,
    char *viewer)
{
    dvModule module;
    dvSchema schema;
    bool foundSchema = false;

    dvForeachRootModule(dvTheRoot, module) {
        if(schemaSym == utSymNull) {
            dvForeachModuleSchema(module, schema) {
                placeAndRouteSchema(schema, optimizeForPrinting, viewer);
            } dvEndModuleSchema;
        } else {
            schema = dvModuleFindSchema(module, schemaSym);
            if(schema != dvSchemaNull) {
                foundSchema = true;
                placeAndRouteSchema(schema, optimizeForPrinting, viewer);
            }
        }
    } dvEndRootModule;
    if(schemaSym != utSymNull && !foundSchema) {
        utError("Schema %s not found", utSymGetName(schemaSym));
    }
}

/*--------------------------------------------------------------------------------------------------
  Initialize databases.
--------------------------------------------------------------------------------------------------*/
void dwStart(void)
{
    utStart();
    dvDatabaseStart();
    dwDatabaseStart();
    dvTheRoot = dvRootAlloc();
    dwClasses = dwClassArrayAlloc();
}

/*--------------------------------------------------------------------------------------------------
  Shut down databases.
--------------------------------------------------------------------------------------------------*/
void dwStop(void)
{
    dwDatabaseStop();
    dvDatabaseStop();
    utStop(false);
}

/*--------------------------------------------------------------------------------------------------
  Report the usage and exit.
--------------------------------------------------------------------------------------------------*/
static void usage(
    char *message)
{
    printf("%s\nUsage: dataview [-p] file [schema]\n"
        "    -d value  -- set debug level to value\n"
        "    -p        -- Optimize for printing, rather than viewing.  Don't launch a viewer.\n"
        "    -v viewer -- Launch the specified postscript viewer, rather than the default of evince\n", message);
    exit(1);
}

/*--------------------------------------------------------------------------------------------------
  Return a string that is a list of the schemas we generated.
--------------------------------------------------------------------------------------------------*/
static char *findSchemaNames(
    utSym schemaSym)
{
    dvModule module;
    dvSchema schema;
    char *schemaList = "";

    dvForeachRootModule(dvTheRoot, module) {
        if(schemaSym == utSymNull) {
            dvForeachModuleSchema(module, schema) {
                schemaList = utSprintf("%s %s.ps", schemaList, dvSchemaGetName(schema));
            } dvEndModuleSchema;
        } else {
            schema = dvModuleFindSchema(module, schemaSym);
            if(schema != dvSchemaNull) {
                schemaList = utSprintf("%s %s.ps", schemaList, dvSchemaGetName(schema));
            }
        }
    } dvEndRootModule;
    return schemaList + 1;
}

/*--------------------------------------------------------------------------------------------------
  This tool generates a class diagram showing classes and their relationships.
--------------------------------------------------------------------------------------------------*/
int main(
    int argc,
    char *argv[])
{
    utSym schemaSym = utSymNull;
    int xArg = 1;
    bool optimizeForPrinting = false;
    char *viewer = "evince";

    while(xArg < argc && *(argv[xArg]) == '-') {
        if(!strcmp(argv[xArg], "-p")) {
            optimizeForPrinting = true;
            viewer = NULL;
        } else if(!strcmp(argv[xArg], "-d")) {
            xArg++;
            if(xArg < argc) {
                utDebugVal = atoi(argv[xArg]);
            }
            if(utDebugVal == 0) {
                usage("Expecting debug level after -d");
            }
        } else if(!strcmp(argv[xArg], "-v")) {
            xArg++;
            if(xArg == argc) {
                usage("Expecting viewer name after -v");
            }
            viewer = argv[xArg];
        } else {
            usage("Invalid argument");
        }
        xArg++;
    }
    if(argc - xArg != 1 && argc - xArg != 2) {
        return 1;
    }
    dwStart();
    if(!dvReadFile(argv[xArg], false)) {
        usage("Unable to parse datatabase description");
    }
    xArg++;
    if(argc > xArg) {
        schemaSym = utSymCreate(argv[xArg]);
    }
    placeAndRouteSchemas(schemaSym, optimizeForPrinting, viewer);
    if(viewer != NULL) {
        utLaunchApp(utSprintf("%s %s", viewer, findSchemaNames(schemaSym)), ".");
    }
    dwStop();
    return 0;
}
