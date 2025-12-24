#
/*
 *	v7sh-amd64 (UNIX shell)
 *
 *	S. R. Bourne
 *	Bell Telephone Laboratories
 *
 */

#include	"defs.h"
#include	<stdlib.h>

#include	<sys/mman.h>

#define STAK_BASE_ADDR (void*)0x700000000000ULL
#define STAK_MAX_SIZE (8 * 1024 * 1024)

BLKPTR		stakbsy;
STKPTR		stakbas;
STKPTR		brkend;
STKPTR		stakbot;
STKPTR		staktop;

LOCAL STKPTR	stak_buffer;

STKPTR	getstak(asize)
	POS		asize;
{
	REG POS		size=round(asize,BYTESPERWORD);
	if (stak_buffer == NIL) {
		stak_buffer = (STKPTR) mmap(STAK_BASE_ADDR, STAK_MAX_SIZE, 
		                           PROT_READ|PROT_WRITE, 
		                           MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED, -1, 0);
		if (stak_buffer == MAP_FAILED) {
			/* Fallback to non-fixed if fixed fails */
			stak_buffer = (STKPTR) mmap(NULL, STAK_MAX_SIZE, 
			                           PROT_READ|PROT_WRITE, 
			                           MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
		}
		if (stak_buffer == MAP_FAILED) error(nospace);
		stakbas = stakbot = staktop = stak_buffer;
		brkend = stak_buffer + STAK_MAX_SIZE;
	}
	if (stakbot + size > brkend) error(nospace);
	{ REG STKPTR oldstak = stakbot;
	  staktop = stakbot += size;
	  return(oldstak);
	}
}

STKPTR	locstak()
{
	if (stak_buffer == NIL) {
		getstak(BRKINCR);
	}
	return(stakbot);
}

STKPTR	savstak()
{
	return(stakbot);
}

STKPTR	endstak(argp)
	REG STRING	argp;
{
	REG STKPTR	oldstak;
	*argp++=0;
	oldstak=stakbot; stakbot=staktop=(STKPTR) round(argp,BYTESPERWORD);
	return(oldstak);
}

VOID	tdystak(x)
	REG STKPTR 	x;
{
	staktop=stakbot=max(ADR(x),ADR(stakbas));
	rmtemp((IOPTR) x);
}

VOID	stakchk()
{}

STKPTR	cpystak(x)
	CSTKPTR		x;
{
	return(endstak(movstr(x,locstak())));
}
