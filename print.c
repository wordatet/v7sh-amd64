#
/*
 * v7sh-amd64 (UNIX shell)
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include	"defs.h"

CHAR		numbuf[12];


/* printing and io conversion */

VOID	newline()
{	prc(NL);
}

VOID	blank()
{	prc(SP);
}

VOID	prp()
{
	IF (flags&prompt)==0 ANDF cmdadr
	THEN	prs(cmdadr); prs(colon);
	FI
}

VOID	prs(as)
	CSTRING		as;
{
	REG CSTRING	s;

	IF (s=as)!=NIL				/* GCC */
	THEN	write(output,s,(SIZE) length(s)-1);
	FI
}

VOID	prc(c)
	INT		c;
{
	IF c
	THEN	write(output,&c,(SIZE) 1);
	FI
}

VOID	prt(t)
	CLOCK		t;
{
	REG CLOCK	hr, min, sec;

	t += 30; t /= 60;
	sec=t%60; t /= 60;
	min=t%60;
	IF (hr=t/60)!=0				/* GCC */
	THEN	prl(hr); prc('h');
	FI
	prl(min); prc('m');
	prl(sec); prc('s');
}

VOID	prn(n)
	INT		n;
{
	itos(n); prs(numbuf);
}

VOID	itos(n)
	INT		n;
{
	REG STRING abuf; REG POS a, i; INT pr, d;

	abuf=numbuf; pr=FALSE; a=n;
	FOR i=10000; i!=1; i/=10
	DO	IF (pr |= (d=a/i)) THEN *abuf++=d+'0' FI
		a %= i;
	OD
	*abuf++=a+'0';
	*abuf++=0;
}

INT	stoi(icp)
	CSTRING	icp;
{
	REG CSTRING	cp = icp;
	REG INT		r = 0;
	REG INT		c;

	WHILE (c = *cp, digit(c)) ANDF c ANDF r>=0
	DO r = r*10 + c - '0'; cp++ OD
	IF r<0 ORF cp==icp
	THEN	failed(icp,badnum);
		/*NOTREACHED*/
	FI
	return(r);
}

VOID	prl(n)	/* for ULIMIT */
	LONG		n;
{
	ltos(n); prs(numbuf);
}

VOID	ltos(n)
	LONG		n;
{
	REG STRING abuf; REG POS a, i; INT pr, d;

	abuf=numbuf; pr=FALSE; a=n;
	FOR i=1000000000; i!=1; i/=10
	DO	IF (pr |= (d=(INT) (a/i))) THEN *abuf++=d+'0' FI
		a %= i;
	OD
	*abuf++=a+'0';
	*abuf++=0;
}
