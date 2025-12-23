#
/*
 * v7sh-amd64 (UNIX shell)
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include	"defs.h"

STRING		exitadr;
INT		exitval;

/* ========	error handling	======== */

VOID	exitset()
{
	assnum(&exitadr,exitval);
}

VOID	sigchk()
{
	/* Find out if it is time to go away.
	 * `trapnote' is set to SIGSET when fault is seen and
	 * no trap has been set.
	 */
	IF trapnote&SIGSET
	THEN	exitsh(SIGFAIL);
		/*NOTREACHED*/
	FI
}

VOID	failed(s1,s2)
	CSTRING	s1, s2;
{
	prp(); prs(s1); 
	IF s2
	THEN	prs(colon); prs(s2);
	FI
	newline(); exitsh(ERROR);
	/*NOTREACHED*/
}

VOID	error(s)
	CSTRING	s;
{
	failed(s,NIL);
	/*NOTREACHED*/
}

VOID	exitsh(xno)
	INT	xno;
{
	/* Arrive here from `FATAL' errors
	 *  a) exit command,
	 *  b) default trap,
	 *  c) fault with no trap set.
	 *
	 * Action is to return to command level or exit.
	 */
	exitval=xno;
	IF (flags & (forked|errflg|ttyflg)) != ttyflg
	THEN	done();
		/*NOTREACHED*/
	ELSE	clearup();
#if defined(SYSIII)
		execbrk = breakcnt = 0;
#endif
		longjmp(errshell,1);
	FI
}

VOID	done()
{
	REG STRING	t;

	IF (t=trapcom[0])!=NIL			/* GCC */
	THEN	trapcom[0]=0; /*should free but not long */
		execexp(t,0);
	FI
	rmtemp(0);
	exit(exitval);
	/*NOTREACHED*/
}

VOID	rmtemp(base)
	IOPTR		base;
{
	WHILE iotemp>base
	DO  unlink(iotemp->ioname);
	    iotemp=iotemp->iolst;
	OD
}
