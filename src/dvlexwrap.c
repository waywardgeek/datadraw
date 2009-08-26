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

/*--------------------------------------------------------------------------------------------------
  This is a simple wrapper to allow KWEND to be returned multiple times in a row.
--------------------------------------------------------------------------------------------------*/
#include "dv.h"
#include "dvparse.h"

uint16 dvNumEndsRemaining;

/*--------------------------------------------------------------------------------------------------
  This is a simple wrapper to allow KWEND to be returned multiple times in a row.
--------------------------------------------------------------------------------------------------*/
int dvlex(void)
{
    if(dvNumEndsRemaining > 0) {
        dvNumEndsRemaining--;
        return KWEND;
    }
    return dvlexlex();
}
