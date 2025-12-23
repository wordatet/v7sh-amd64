#
/*
 * v7sh-amd64 (UNIX shell)
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include	"defs.h"


BOOL		trapnote;
STRING		trapcom[MAXTRAP];
#if defined(SYSIII)
BOOL		trapflg[MAXTRAP] = {
                0,
                0,		/* Hangup */
                SIGCAUGHT,      /* Interrupt */
                SIGCAUGHT,	/* Quit */
                0,		/* Illegal instruction */
                0,		/* Trace/BPT trap */
                0,		/* IOT trap */
                0,		/* EMT trap */
                0,		/* Floating exception */
                0,		/* Killed */
                0,		/* Bus error */
                0,		/* Memory fault */
                0,		/* Bad system call */
                0,      	/* Broken pipe */
                SIGCAUGHT,	/* Alarm call */
                SIGCAUGHT,	/* Terminated */
                0,		/* Urgent condition */
                0,		/* Stopped */
                0,		/* Stopped from terminal */
                0,		/* Continued */
                0,		/* Child terminated */
                0,		/* Stopped on terminal input */
                0,		/* Stopped on terminal output */
                0,		/* Asynchronous I/O */
                0,		/* Exceeded cpu time limit */
                0,		/* Exceeded file size limit */
                0,		/* Virtual time alarm */
                0,		/* Profiling time alarm */
                0,		/* Window size changed */
                0,		/* Information request */
                0,		/* User defined signal 1 */
                0,		/* User defined signal 2 */
                0		/* Thread interrupt */
};
#else /* V7 */
BOOL		trapflg[MAXTRAP];
#endif
#if defined(RENO)
BOOL		trapjmp[MAXTRAP];
jmp_buf		INTbuf;
#endif
#if defined(SYSIII)
BOOL		wasintr;
#endif

/* ========	fault handling routines	   ======== */


#include <signal.h>

VOID	fault(sig)
	INT		sig;
{
	REG INT		flag;
	IF sig==MEMF
	THEN	/* With malloc-based memory, SIGSEGV is always fatal */
		write(2, "Segmentation fault\n", 19);
		_exit(139);  /* 128 + SIGSEGV(11) */
	ELIF sig==ALARM
	THEN	IF flags&waiting
		THEN	done();
			/*NOTREACHED*/
		FI
	ELSE	flag = (trapcom[sig] ? TRAPSET : SIGSET);
		trapnote |= flag;
		trapflg[sig] |= flag;
#if defined(SYSIII)
		IF sig == INTR THEN wasintr++ FI
#endif
	FI
#if defined(RENO)
	IF trapjmp[sig] ANDF sig==INTR
	THEN	trapjmp[sig] = 0;
		longjmp(INTbuf, 1);
	FI
#endif
}

VOID	stdsigs()
{
	ignsig(QUIT);
	getsig(INTR);
	/* Don't catch SIGSEGV - let it use default handler for debugging */
	getsig(ALARM);
}

SIGPTR	ignsig(n)
	INT		n;
{
	REG INT		i;
	REG SIGPTR	s=SIG_DFL;		/* GCC */

#if defined(SYSIII)
	IF (i=n)==MEMF
	THEN	clrsig(i);
		failed(badtrap, memfault);
		/*NOTREACHED*/
	ELIF (s=signal(i=n,SIG_IGN))==SIG_DFL
#else /* V7 */
	IF (s=signal(i=n,SIG_IGN))==SIG_DFL
#endif
	THEN	trapflg[i] |= SIGMOD;
	FI
	return(s);
}

VOID	getsig(n)
	INT		n;
{
	REG INT		i;

	IF trapflg[i=n]&SIGMOD ORF ignsig(i)==SIG_DFL
	THEN	signal(i,fault);
	FI
}

VOID	oldsigs()
{
	REG INT		i;
	REG STRING	t;

	i=MAXTRAP;
	WHILE i--
	DO  t=trapcom[i];
	    IF t==0 ORF *t
	    THEN clrsig(i);
	    FI
	    trapflg[i]=0;
	OD
	trapnote=0;
}

VOID	clrsig(i)
	INT		i;
{
	shfree((BLKPTR) trapcom[i]); trapcom[i]=0;
	IF trapflg[i]&SIGMOD
#if defined(SYSIII)
	THEN	IF trapflg[i]&SIGCAUGHT
		THEN signal(i, fault);
		ELSE signal(i, SIG_DFL);
		FI
		trapflg[i] &= ~SIGMOD;
#else /* V7 */
	THEN	signal(i,fault);
#endif
	FI
}

VOID	chktrap()
{
	/* check for traps */
	REG INT		i=MAXTRAP;
	REG STRING	t;

	trapnote &= ~TRAPSET;
	WHILE --i
	DO IF trapflg[i]&TRAPSET
	   THEN trapflg[i] &= ~TRAPSET;
		IF (t=trapcom[i])!=NIL		/* GCC */
		THEN	INT	savxit=exitval;
			execexp(t,0);
			exitval=savxit; exitset();
		FI
	   FI
	OD
}
