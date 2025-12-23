/*
 * v7sh-amd64 (UNIX shell)
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 * This port (v7sh-amd64) is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as published 
 * by the Free Software Foundation; either version 2 of the License.
 */

#include "defs.h"

BYTPTR setbrk(incr)
	INT incr;
{
	return (BYTPTR)0;
}
