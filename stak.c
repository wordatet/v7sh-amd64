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

BLKPTR		stakbsy;
STKPTR		stakbas;
STKPTR		brkend;
STKPTR		stakbot;
STKPTR		staktop;

LOCAL STKPTR	stak_buffer;
LOCAL POS	stak_size;

#define INITIAL_STAK_SIZE (1024 * 1024)

/* ========	storage allocation	======== */

LOCAL VOID	stak_grow(needed)
	POS	needed;
{
	REG POS	off_bas = stakbas - stak_buffer;
	REG POS	off_bot = stakbot - stak_buffer;
	REG POS	off_top = staktop - stak_buffer;
	stak_size += max(needed + BRKINCR, stak_size);
	stak_buffer = (STKPTR) realloc(stak_buffer, stak_size);
	IF stak_buffer == NIL THEN error(nospace) FI
	stakbas = stak_buffer + off_bas;
	stakbot = stak_buffer + off_bot;
	staktop = stak_buffer + off_top;
	brkend = stak_buffer + stak_size;
}

STKPTR	getstak(asize)
	POS		asize;
{
	REG POS		size=round(asize,BYTESPERWORD);
	if (stak_buffer == NIL) {
		stak_size = INITIAL_STAK_SIZE;
		stak_buffer = malloc(stak_size);
		stakbas = stakbot = staktop = stak_buffer;
		brkend = stak_buffer + stak_size;
	}
	WHILE stakbot + size > brkend DO stak_grow(size) OD
	{ REG STKPTR oldstak = stakbot;
	  staktop = stakbot += size;
	  return(oldstak);
	}
}

STKPTR	locstak()
{
	if (stak_buffer == NIL) {
		stak_size = INITIAL_STAK_SIZE;
		stak_buffer = malloc(stak_size);
		stakbas = stakbot = staktop = stak_buffer;
		brkend = stak_buffer + stak_size;
	}
	IF stakbot + BRKINCR >= brkend THEN stak_grow((POS)BRKINCR) FI
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
