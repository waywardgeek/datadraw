/*
 * Copyright (C) 2006 Bill Cox
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

%{

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "dv.h"
#include "dvparse.h"

#define YY_NEVER_INTERACTIVE 1
#define YY_INPUT(buf,result,max_size) \
    if((result = dvRead((char *) buf, max_size )) < 0 ) \
        YY_FATAL_ERROR( "input in flex scanner failed" );

bool dvLastWasReturn;
bool dvEnding;

static int dvRead(char *buf, int maxSize)
{
    int numChars = 0;
    int c;

    do {
        if(dvLastWasReturn) {
            /* Insert a tab in front of each line to force a character to match */
            c = '\t';
            dvLastWasReturn = false;
        } else if(dvEnding) {
            c = EOF;
        } else {
            c = getc(dvFile);
        }
        if(!dvEnding && c == EOF) {
            /* Add a newline at end of file in case the input file is missing it */
            c = '\n';
            dvEnding = true;
        }
        if(c != EOF && c != '\r') {
            *buf++ = c;
            numChars++;
            if(c == '\n') {
                dvLastWasReturn = true;
            }
        }
    } while(numChars < maxSize && c != EOF);
    return numChars;
}

static char *unmunge(
    char *string)
{
    char *buffer = utMakeString(strlen(string));
    char *p = buffer;

    string++; /* Skip " */
    while(*string != '\0') {
        if(*string == '\\') {
            string++;
        }
        *p++ = *string++;
    }
    *--p = '\0'; /* Overwrite terminating " */
    return buffer;
}

static uint16 findIndentDepth(char *string)
{
    uint16 length = 0;

    while(*string) {
        if(*string == ' ') {
            length++;
        } else if(*string == '\t') {
            length += 8;
        }
        string++;
    }
    return length;
}

int dvlexwrap(void)
{
    return 1;
}

#ifdef DV_DEBUG
#define myDebug utDebug
#else
#define myDebug noDebug
#endif
#undef YYLMAX
#define YYLMAX 4096

static uint16 dvCommentDepth, dvIndentLevels = 0, dvIndentDepth[DV_MAX_INDENT] = {8};

static void noDebug(char *foo, ...) {}

%}

%option prefix="dvlex"
%option outfile="dvscan.c"

%Start comment

%%
<INITIAL>^[ \t]*"//".*\n        { dvLineNum++; myDebug("comment line\n"); } /* Single line comment */
<INITIAL>[ \t]*"//".*\n         { dvLineNum++; myDebug("newline\n"); return '\n'; } /* Tail comment */
<INITIAL>"/*"                   { myDebug("Start block comment...\n");
                                  dvCommentDepth = 1;
                                  BEGIN comment; }
<comment>"/*"                   { dvCommentDepth++; }
<comment>"*/"                   { myDebug("End block comment\n");
                                  dvCommentDepth--;
                                  if (dvCommentDepth == 0) {
                                     BEGIN INITIAL;
                                  } }
<comment>.                      ;
<comment>"\n"                   { dvLineNum++; }


<INITIAL>^[ \t]*\n              { dvLineNum++; }
<INITIAL>^[ \t]+                { uint16 length = findIndentDepth(dvlextext);
                                  uint16 oldLevels = dvIndentLevels;
                                  if(length > dvIndentDepth[dvIndentLevels]) {
                                      dvIndentDepth[++dvIndentLevels] = length;
                                      myDebug("KWBEGIN\n");
                                      return KWBEGIN;
                                  } else if(length < dvIndentDepth[dvIndentLevels]) {
                                      while(dvIndentLevels > 0 &&
                                              length != dvIndentDepth[dvIndentLevels]) {
                                          dvIndentLevels--;
                                      }
                                      if(length != 8 && dvIndentLevels == 0) {
                                          dverror("Mismatched indentation");
                                      }
                                      if(dvIndentLevels + 1 != oldLevels) {
                                          dvNumEndsRemaining = oldLevels - dvIndentLevels - 1;
                                          myDebug("Starting multiple ends...\n");
                                      }
                                      myDebug("KWEND\n");
                                      return KWEND;
                                }}

<INITIAL>[ \t]+                 ;

<INITIAL>"array"                {myDebug("KWARRAY\n"); return KWARRAY;}
<INITIAL>"attributes"           {myDebug("KWATTRIBUTES\n"); return KWATTRIBUTES;}
<INITIAL>"bit"                  {myDebug("KWBIT\n"); return KWBIT;}
<INITIAL>"bool"                 {myDebug("KWBOOL\n"); return KWBOOL;}
<INITIAL>"cache_together"       {myDebug("KWCACHE_TOGETHER"); return KWCACHE_TOGETHER;}
<INITIAL>"cascade"              {myDebug("KWCASCADE\n"); return KWCASCADE;}
<INITIAL>"char"                 {myDebug("KWCHAR\n"); return KWCHAR;}
<INITIAL>"child_only"           {myDebug("KWCHILD_ONLY\n"); return KWCHILD_ONLY;}
<INITIAL>"class"                {myDebug("KWCLASS\n"); return KWCLASS;}
<INITIAL>"create_only"          {myDebug("KWCREATE_ONLY\n"); return KWCREATE_ONLY;}
<INITIAL>"double"               {myDebug("KWDOUBLE\n"); return KWDOUBLE;}
<INITIAL>"doubly_linked"        {myDebug("KWDOUBLY_LINKED\n"); return KWDOUBLY_LINKED;}
<INITIAL>"enum"                 {myDebug("KWENUM\n"); return KWENUM;}
<INITIAL>"float"                {myDebug("KWFLOAT\n"); return KWFLOAT;}
<INITIAL>"free_list"            {myDebug("KWFREE_LIST\n"); return KWFREE_LIST;}
<INITIAL>"hashed"               {myDebug("KWHASHED\n"); return KWHASHED;}
<INITIAL>"heap"                 {myDebug("KWHEAP\n"); return KWHEAP;}
<INITIAL>"import"               {myDebug("KIMPORT\n"); return KWIMPORT;}
<INITIAL>"linked_list"          {myDebug("KWLINKED_LIST\n"); return KWLINKED_LIST;}
<INITIAL>"mandatory"            {myDebug("KWMANDATORY\n"); return KWMANDATORY;}
<INITIAL>"module"               {myDebug("KWMODULE\n"); return KWMODULE;}
<INITIAL>"ordered_list"         {myDebug("KWORDERED_LIST\n"); return KWORDERED_LIST;}
<INITIAL>"parent_only"          {myDebug("KWPARENT_ONLY\n"); return KWPARENT_ONLY;}
<INITIAL>"persistent"           {myDebug("KWPERSISTENT\n"); return KWPERSISTENT;}
<INITIAL>"reference_size"       {myDebug("KWREFERENCE_SIZE\n"); return KWREFERENCE_SIZE;}
<INITIAL>"relationship"         {myDebug("KWRELATIONSHIP\n"); return KWRELATIONSHIP;}
<INITIAL>"schema"               {myDebug("KWSCHEMA\n"); return KWSCHEMA;}
<INITIAL>"sparse"               {myDebug("KWSPARSE\n"); return KWSPARSE;}
<INITIAL>"sym"                  {myDebug("KWSYM\n"); return KWSYM;}
<INITIAL>"tail_linked"          {myDebug("KWTAIL_LINKED\n"); return KWTAIL_LINKED;}
<INITIAL>"typedef"              {myDebug("KWTYPEDEF\n"); return KWTYPEDEF;}
<INITIAL>"undo_redo"            {myDebug("KWUNDO_REDO\n"); return KWUNDO_REDO;}
<INITIAL>"union"                {myDebug("KWUNION\n"); return KWUNION;}
<INITIAL>"unordered"            {myDebug("KWUNORDERED\n"); return KWUNORDERED;}
<INITIAL>"view"                 {myDebug("KWVIEW\n"); return KWVIEW;}
<INITIAL>"volatile"             {myDebug("KWVOLATILE\n"); return KWVOLATILE;}

<INITIAL>"."                    { myDebug(".\n"); return '.'; }
<INITIAL>"+"\n                  { myDebug("Line continuation\n"); dvLineNum++; }
<INITIAL>\n                     { myDebug("newline\n"); dvLineNum++; return '\n'; }
<INITIAL>[0-9]+                 { dvlval.intVal = atol(dvlextext);
                                  myDebug("INTEGER %u\n", dvlval.intVal);
                                  return INTEGER; }
<INITIAL>int[0-9]*              { if(strlen(dvlextext) == 3) {
                                      dvlval.intVal = 32;
                                  } else {
                                      dvlval.intVal = atol(dvlextext + 3);
                                  }
                                  myDebug("INTTYPE %u\n", dvlval.intVal);
                                  return INTTYPE; }
<INITIAL>uint[0-9]*             { if(strlen(dvlextext) == 4) {
                                      dvlval.intVal = 32;
                                  } else {
                                      dvlval.intVal = atol(dvlextext + 4);
                                  }
                                  myDebug("UINTTYPE %u\n", dvlval.intVal);
                                  return UINTTYPE; }
<INITIAL>[a-zA-Z][a-zA-Z0-9_]*  { myDebug("IDENT %s\n", dvlextext);
                                  dvlval.symVal = utSymCreate(dvlextext);
                                  return IDENT; }
<INITIAL>\"([^"]|\\.)*\"      { myDebug("STRING %s\n", dvlextext);
                                  dvlval.stringVal = utSymCreate(unmunge(dvlextext));
                                  return STRING; }
<INITIAL>\[([^\[\]]|\\.)*\]      { myDebug("INDEX %s\n", dvlextext);
                                  dvlval.indexVal = utSymCreate(unmunge(dvlextext));
                                  return INDEX; }
<INITIAL>.                      { myDebug("Char '%c'\n", dvlextext[0]);
                                  return dvlextext[0]; }
