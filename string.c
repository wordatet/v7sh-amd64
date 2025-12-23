#
/*
 * v7sh-amd64 (UNIX shell)
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include	"defs.h"


/* ========	general purpose string handling ======== */


STRING	movstr(a,b)
	REG CSTRING	a;
	REG STRING	b;
{
	WHILE (*b++ = *a++)!=0 DONE		/* GCC */
	return(--b);
}

INT	any(c,s)
	REG INT		c;
	CSTRING		s;
{
	REG INT		d;

	WHILE (d = *s++)!=0			/* GCC */
	DO	IF d==c
		THEN	return(TRUE);
		FI
	OD
	return(FALSE);
}

INT	cf(s1, s2)
	REG CSTRING s1, s2;
{
	WHILE *s1++ == *s2
	DO	IF *s2++==0
		THEN	return(0);
		FI
	OD
	return(*--s1 - *s2);
}

INT	length(as)
	CSTRING as;
{
	REG CSTRING s;

	IF (s=as)!=NIL THEN WHILE *s++ DONE FI	/* GCC */
	return(s-as);
}
