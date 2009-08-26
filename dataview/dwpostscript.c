#include "dw.h"

#define DW_DEFAULT_PAGE_WIDTH 612 /* Default letter width */
#define DW_DEFAULT_PAGE_HEIGHT 792 /* Default letter width */

static uint32 dwPageHeight;
static uint32 dwPageWidth;
static uint32 dwPageHeight;
static FILE *dwFile;
static uint32 dwGridSize;
static uint32 dwGridsPerRow, dwGridsPerCol;
static uint32 dwXOffset;
static uint32 dwYOffset;

#define dwColToPageX(col) ((col)*dwGridSize*dwGridsPerCol + dwXOffset)
#define dwRowToPageY(row) ((row)*dwGridSize*dwGridsPerRow + dwYOffset)
#define dwGridXToPageX(x) ((x)*dwGridSize + dwXOffset)
#define dwGridYToPageY(y) ((y)*dwGridSize + dwYOffset)

/*--------------------------------------------------------------------------------------------------
  Return the name of the class the way it will be printed.
--------------------------------------------------------------------------------------------------*/
char *dwClassGetName(
    dvClass class)
{
    utSym baseClassSym = dvClassGetBaseClassSym(class);

    if(baseClassSym == utSymNull) {
        return dvClassGetName(class);
    }
    return utSprintf("%s:%s", dvClassGetName(class), utStringToLowerCase(utSymGetName(baseClassSym)));
}

/*--------------------------------------------------------------------------------------------------
  We need to scale the image and center it.  This routing finds the scaling factors and X and Y
  offsets.  If optimizeForPrinting is set, we assume letter size, and rotate if it's better
  for landscape viewing.  Otherwise, we assume a page size equal to our grid size.
--------------------------------------------------------------------------------------------------*/
static void findScalingFactors(
    dvSchema schema,
    bool optimizeForPrinting)
{
    uint32 rows, cols, horizGrid, vertGrid;

    dwGridsPerCol = dwSchemaGetColWidth(schema);
    dwGridsPerRow = dwSchemaGetRowHeight(schema);
    cols = (dwSchemaGetCols(schema) + 1)*dwGridsPerCol;
    rows = (dwSchemaGetRows(schema) + 1)*dwGridsPerRow;
    if(optimizeForPrinting) {
        if(cols <= rows) {
            /* Portrait view */
            dwPageWidth = DW_DEFAULT_PAGE_WIDTH;
            dwPageHeight = DW_DEFAULT_PAGE_HEIGHT;
        } else {
            /* Landscape view */
            dwPageWidth = DW_DEFAULT_PAGE_HEIGHT;
            dwPageHeight = DW_DEFAULT_PAGE_WIDTH;
        }
    } else {
        dwPageWidth = DW_DEFAULT_PAGE_WIDTH;
        dwPageHeight = DW_DEFAULT_PAGE_WIDTH*rows/cols;
    }
    /* Make grid a multiple of 4 */
    horizGrid = (dwPageWidth/cols) << 2;
    vertGrid = (dwPageHeight/rows) << 2;
    if(horizGrid <= vertGrid) {
        dwGridSize = horizGrid;
        dwXOffset = 0;
        dwYOffset = ((dwPageHeight << 2) - (dwSchemaGetRows(schema) + 1)*dwGridSize*dwGridsPerRow) >> 1;
    } else {
        dwGridSize = vertGrid;
        dwYOffset = 0;
        dwXOffset = ((dwPageWidth << 2) - (dwSchemaGetCols(schema) + 1)*dwGridSize*dwGridsPerCol) >> 1;
    }
}

/*--------------------------------------------------------------------------------------------------
  Print the beginning of the postscript file.  If optimizeForPrinting is not set, we set the page
  size to dwPageWidth and dwPageHieght.  This works for viewing, but not printing.
--------------------------------------------------------------------------------------------------*/
static void printHeader(
    dvSchema schema,
    bool optimizeForPrinting)
{
    fprintf(dwFile, "%%!PS\n");
    if(!optimizeForPrinting) {
        fprintf(dwFile, "%%%%DocumentMedia: CUSTOM %u %u 0 () ()\n", dwPageWidth, dwPageHeight);
    }
    fprintf(dwFile,"%%%%BoundingBox %u %u\n", dwPageWidth, dwPageHeight);
    if(optimizeForPrinting && dwPageWidth > dwPageHeight) {
        fprintf(dwFile, "90 rotate 0 -%u translate\n", dwPageHeight);
    }
    fprintf(dwFile, "0.25 0.25 scale\n");
    fprintf(dwFile, "/Times-Roman findfont\n");
    fprintf(dwFile, "%u scalefont\n", dwGridSize);
    fprintf(dwFile, "setfont\n");
    fprintf(dwFile, "2.5 setlinewidth\n");
}

/*--------------------------------------------------------------------------------------------------
  Print the class.
--------------------------------------------------------------------------------------------------*/
static void printClass(
    dvClass theClass)
{
    char *name = dwClassGetName(theClass);
    int32 left = dwGridXToPageX(dwClassGetLeft(theClass)) - (dwGridSize >> 1);
    int32 bottom = dwGridYToPageY(dwClassGetBottom(theClass)) - (dwGridSize >> 1);
    int32 right = dwGridXToPageX(dwClassGetRight(theClass)) + (dwGridSize >> 1);
    int32 top = dwGridYToPageY(dwClassGetTop(theClass)) + (dwGridSize >> 1);
    int32 x = (right + left) >> 1;
    int32 y = (top + bottom) >> 1;

    /* Find the string dimensions so we can center the text */
    fprintf(dwFile, "%d %d moveto\n (%s) dup stringwidth pop 2 div neg 0 rmoveto show\n", x, y - (dwGridSize >> 2), name);
    fprintf(dwFile, "newpath\n %d %d moveto\n %d %d lineto\n %d %d lineto\n %d %d lineto\n closepath\n stroke\n",
        left, bottom, right, bottom, right, top, left, top);
}

/*--------------------------------------------------------------------------------------------------
  Set the relationship color.
--------------------------------------------------------------------------------------------------*/
static void setRelationshipColor(
    dvRelationship relationship)
{
    float red = 0;
    float green = 0;
    float blue = 0;

    if(dvRelationshipMandatory(relationship)) {
        green = 0.85f;
    } else if(dvRelationshipCascade(relationship)) {
        red = 0.4f;
        green = 0.4f;
        blue = 1.0f;
    }
    fprintf(dwFile, " %.1f %.1f %.1f setrgbcolor\n", red, green, blue);
}

/*--------------------------------------------------------------------------------------------------
  Find the rotation required to have the upper bund be directly above the lower bund.
--------------------------------------------------------------------------------------------------*/
static int32 findRotation(
    dwBund upperBund,
    dwBund lowerBund)
{
    int32 upperX = dwBundGetX(upperBund);
    int32 upperY = dwBundGetY(upperBund);
    int32 lowerX = dwBundGetX(lowerBund);
    int32 lowerY = dwBundGetY(lowerBund);

    if(upperX == lowerX) {
        if(upperY > lowerY) {
            return 0;
        }
        return 180;
    } else {
        if(upperX > lowerX) {
            return -90;
        }
        return 90;
    }
}

/*--------------------------------------------------------------------------------------------------
  Draw the first route, including an optional bubble and label.  We assume the class is above us,
  and rotate/translate if needed.
--------------------------------------------------------------------------------------------------*/
static void drawFirstRoute(
    dvRelationship relationship,
    dwBund classBund,
    dwBund routeBund)
{
    int32 x = dwGridXToPageX(dwBundGetX(routeBund));
    int32 y = dwGridYToPageY(dwBundGetY(routeBund));
    int32 rotation = findRotation(classBund, routeBund);

    fprintf(dwFile, " gsave\n%d %d translate\n", x, y);
    if(rotation != 0) {
        fprintf(dwFile, " %d rotate\n", rotation);
    }
    if(dvRelationshipAccessParent(relationship)) {
        fprintf(dwFile, " newpath\n 0 %d %d 0 360 arc\n closepath\n", dwGridSize >> 2, dwGridSize >> 2);
    } else {
        fprintf(dwFile, " newpath\n 0 0 moveto\n 0 %d lineto\n", dwGridSize >> 1);
    }
    fprintf(dwFile, " stroke\n");
    fprintf(dwFile, " grestore\n");
}

/*--------------------------------------------------------------------------------------------------
  Draw a heavy arrow, which has enclosed area.
--------------------------------------------------------------------------------------------------*/
static void drawHeavyArrow(void)
{
    uint32 delta = dwGridSize >> 2;

    fprintf(dwFile, " newpath\n 0 0 moveto\n 0 %d lineto\n stroke\n newpath\n"
        " 0 %d moveto\n %d 0 lineto\n 0 %d lineto\n %d 0 lineto\n closepath\n fill\n",
        -delta, -delta, delta, -(delta << 1), -delta);
}

/*--------------------------------------------------------------------------------------------------
  Draw a light arrow, which has no enclosed area.
--------------------------------------------------------------------------------------------------*/
static void drawLightArrow(void)
{
    uint32 delta = dwGridSize >> 2;

    fprintf(dwFile, " newpath\n 0 0 moveto\n 0 %d lineto\n stroke\n newpath\n"
        " %d 0 moveto\n 0 %d lineto\n %d 0 lineto\n stroke\n",
        -(delta << 1), -delta, -(delta << 1), delta);
}

/*--------------------------------------------------------------------------------------------------
  Draw a box.
--------------------------------------------------------------------------------------------------*/
static void drawBox(void)
{
    uint32 delta = dwGridSize >> 2;

    fprintf(dwFile, " newpath\n %d 0 moveto\n %d %d lineto\n %d %d lineto\n %d 0 lineto\n closepath\n stroke\n",
        -delta, -delta, -(delta << 1), delta, -(delta << 1), delta);
}

/*--------------------------------------------------------------------------------------------------
  Draw an X inside the box.
--------------------------------------------------------------------------------------------------*/
static void drawX(void)
{
    uint32 delta = dwGridSize >> 2;

    fprintf(dwFile, " newpath\n %d 0 moveto\n %d %d lineto\n stroke\n newpath\n %d 0 moveto\n"
        " %d %d lineto\n stroke\n", -delta, delta, -(delta << 1), delta, -delta, -(delta << 1));
}

/*--------------------------------------------------------------------------------------------------
  Draw the last route, including an arrow and label.
--------------------------------------------------------------------------------------------------*/
static void drawLastRoute(
    dvRelationship relationship,
    dwBund classBund,
    dwBund routeBund)
{
    int32 x = dwGridXToPageX(dwBundGetX(routeBund));
    int32 y = dwGridYToPageY(dwBundGetY(routeBund));
    int32 rotation = findRotation(routeBund, classBund);

    fprintf(dwFile, " gsave\n %d %d translate\n", x, y);
    if(rotation != 0) {
        fprintf(dwFile, " %d rotate\n", rotation);
    }
    switch(dvRelationshipGetType(relationship)) {
    case REL_LINKED_LIST:
    case REL_DOUBLY_LINKED:
    case REL_TAIL_LINKED:
        drawHeavyArrow();
        break;
    case REL_POINTER:
        drawLightArrow();
        break;
    case REL_ARRAY:
    case REL_HEAP:
        drawBox();
        drawX();
        break;
    case REL_HASHED:
        drawBox();
        break;
    default:
        utExit("Unknown type");
    }
    fprintf(dwFile, " grestore\n");
}

/*--------------------------------------------------------------------------------------------------
  Print the relationship.
--------------------------------------------------------------------------------------------------*/
static void printRelationship(
    dvRelationship relationship)
{
    dwBund bund, prevBund = dwBundNull;
    dwRoute route;
    bool firstRoute = true;

    fprintf(dwFile, "gsave\n");
    setRelationshipColor(relationship);
    dwForeachRelationshipRoute(relationship, route) {
        bund = dwRouteGetBund(route);
        if(prevBund != dwBundNull) {
            if(firstRoute) {
                firstRoute = false;
                drawFirstRoute(relationship, prevBund, bund);
                fprintf(dwFile, " newpath\n %d %d moveto\n",
                    dwGridXToPageX(dwBundGetX(bund)), dwGridYToPageY(dwBundGetY(bund)));
            } else if(dwRouteGetNextRelationshipRoute(route) == dwRouteNull) {
                fprintf(dwFile, " stroke\n");
                drawLastRoute(relationship, bund, prevBund);
            } else {
                fprintf(dwFile, " %d %d lineto \n", dwGridXToPageX(dwBundGetX(bund)), dwGridYToPageY(dwBundGetY(bund)));
            }
        }
        prevBund = bund;
    } dwEndRelationshipRoute;
    fprintf(dwFile, "grestore\n");
}

/*--------------------------------------------------------------------------------------------------
  Draw the first route, including an optional bubble and label.  We assume the class is above us,
  and rotate/translate if needed.
--------------------------------------------------------------------------------------------------*/
static void drawParentLabel(
    dvRelationship relationship)
{
    dwRoute route = dwRelationshipGetFirstRoute(relationship);
    dwBund classBund = dwRouteGetBund(route);
    dwBund routeBund = dwRouteGetBund(dwRouteGetNextRelationshipRoute(route));
    int32 x = dwGridXToPageX(dwBundGetX(routeBund));
    int32 y = dwGridYToPageY(dwBundGetY(routeBund));
    int32 rotation = findRotation(classBund, routeBund);

    fprintf(dwFile, " gsave\n%d %d translate\n", x, y);
    if(rotation != 0) {
        fprintf(dwFile, " %d rotate\n", rotation);
    }
    fprintf(dwFile, " 0 0 0 setrgbcolor\n -90 rotate\n");
    fprintf(dwFile, " /Times-Roman findfont\n %u scalefont\n setfont\n", dwGridSize >> 1);
    fprintf(dwFile, " 0 1 moveto\n (%s) show\n", dvRelationshipGetParentLabel(relationship));
    fprintf(dwFile, " grestore\n");
}

/*--------------------------------------------------------------------------------------------------
  Draw the last route, including an arrow and label.
--------------------------------------------------------------------------------------------------*/
static void drawChildLabel(
    dvRelationship relationship)
{
    dwRoute route = dwRelationshipGetLastRoute(relationship);
    dwBund classBund = dwRouteGetBund(route);
    dwBund routeBund = dwRouteGetBund(dwRouteGetPrevRelationshipRoute(route));
    int32 x = dwGridXToPageX(dwBundGetX(routeBund));
    int32 y = dwGridYToPageY(dwBundGetY(routeBund));
    int32 rotation = findRotation(routeBund, classBund);

    fprintf(dwFile, " gsave\n %d %d translate\n", x, y);
    if(rotation != 0) {
        fprintf(dwFile, " %d rotate\n", rotation);
    }
    fprintf(dwFile, " 0 0 0 setrgbcolor\n -90 rotate\n");
    fprintf(dwFile, " /Times-Roman findfont\n %u scalefont\n setfont\n", dwGridSize >> 1);
    fprintf(dwFile, " 0 1 moveto\n (%s) dup stringwidth pop neg 0 rmoveto show\n",
        dvRelationshipGetChildLabel(relationship));
    fprintf(dwFile, " grestore\n");
}

/*--------------------------------------------------------------------------------------------------
  Print the relationship.
--------------------------------------------------------------------------------------------------*/
static void printRelationshipLabels(
    dvRelationship relationship)
{
    if(dvRelationshipGetParentLabelSym(relationship) != utSymNull) {
        drawParentLabel(relationship);
    }
    if(dvRelationshipGetChildLabelSym(relationship) != utSymNull) {
        drawChildLabel(relationship);
    }
}

/*--------------------------------------------------------------------------------------------------
  Print the end of the postscript file.
--------------------------------------------------------------------------------------------------*/
static void printTrailer(void)
{       
    fprintf(dwFile,"showpage");
}

/*--------------------------------------------------------------------------------------------------
  Print the schema.  By default, we make up a page size that is scaled to the schema, however,
  that does not work for printing.  If optimizeForPrinting is set, we choose letter page size.
--------------------------------------------------------------------------------------------------*/
void dwPrintSchemaClasses(
    dvSchema schema,
    bool optimizeForPrinting)
{
    char *schemaName = utSprintf("%s.ps", dvSchemaGetName(schema));
    dvClass theClass;    
    dvRelationship relationship;

    dwFile = fopen(schemaName, "w");
    utLogMessage("Writing schema %s", schemaName);
    findScalingFactors(schema, optimizeForPrinting);
    printHeader(schema, optimizeForPrinting);
    dwForeachClassArrayClass(dwClasses, theClass) {
        printClass(theClass);
    } dwEndClassArrayClass;
    dvForeachSchemaRelationship(schema, relationship) {
        printRelationship(relationship);
    } dvEndSchemaRelationship;
    dvForeachSchemaRelationship(schema, relationship) {
        printRelationshipLabels(relationship);
    } dvEndSchemaRelationship;
    printTrailer();
    fclose(dwFile);
}

