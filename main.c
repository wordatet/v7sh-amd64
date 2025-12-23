#
/*
 * v7sh-amd64 (UNIX shell)
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 * This port (v7sh-amd64) was developed with the assistance of Gemini and 
 * Claude and is free software; you can redistribute it and/or modify it 
 * under the terms of the GNU General Public License as published by the 
 * Free Software Foundation; either version 2 of the License.
 */

#include	"defs.h"
#include	"sym.h"
#include	"timeout.h"
#include	"pathnames.h"
#ifdef stupid
#include	<execargs.h>
#endif

LOCAL VOID	exfile(BOOL);
LOCAL VOID	Ldup(INT, INT);

LOCAL BOOL	beenhere = FALSE;
LOCAL FILEBLK	stdfile;

UFD		output = 2;
CHAR		tmpout[20] = _PATH_TMPOUT;
STRING		tmpnam;
FILEPTR		standin = &stdfile;
INT		dolc;
STRING		*dolv;
STRING		dolladr;
STRING		cmdadr;
STRING		pidadr;
STRING		comdiv;
INT		flags;
INT		serial;
jmp_buf		subshell;
jmp_buf		errshell;


INT	main(c, v)
	INT		c;
	STRING		v[];
{
	REG INT		rflag=ttyflg;
#if defined(SYSIII)
	INT		rsflag=1;	/* local restricted flag */
#endif

	/* initialise storage allocation */
	stdsigs();

	/* set names from userenv */
#if !defined(SYSIII)
	getenv();
#else /* SYSIII */
	/* 'rsflag' is non-zero if SHELL variable is
	   set in environment and contains an 'r' in
	   the simple file part of the value.	*/
	rsflag=getenv();

	/* look for restricted */
	/* a shell is also restricted if argv(0) has
	   an 'r' in its simple name	*/
	IF c>0 ANDF any('r', simple(*v)) THEN rflag=0 FI
#endif
	/* look for options */
	/* dolc is $# */
	dolc=options(c,v);
	IF dolc<2 THEN flags |= stdflg FI
#if defined(SYSIII)
	IF dolc < 2
	THEN	REG STRING flagc = flagadr;
		WHILE *flagc DO flagc++ OD
		*flagc = STDFLG;
	FI
#endif
	IF (flags&stdflg)==0
	THEN	dolc--;
	FI
	dolv=v+c-dolc; dolc--;

	/* return here for shell file execution */
	/* but not for parenthesis subshells	*/
	setjmp(subshell);

	/* number of positional parameters '$#' */
	assnum(&dolladr,dolc);
	/* comadr is $0 */
	cmdadr=dolv[0];

	/* set pidname '$$' */
	assnum(&pidadr, getpid());

	/* set up temp file names */
	settmp();

	/* default internal field separators - $IFS */
	dfault(&ifsnod, sptbnl);

	IF (beenhere++)==FALSE
	THEN	/* ? profile */
#if defined(SYSIII)
		IF *simple(cmdadr) == '-'
		THEN	IF (input=pathopen(nullstr, sysprofile))>=0
			THEN	exfile(rflag);	/* file exists */
			FI
			IF (input=pathopen(homenod.namenv, profile))>=0
			THEN	exfile(rflag); flags &= ~ttyflg;
			FI
		FI
		IF rsflag==0 ORF rflag==0 THEN flags |= rshflg FI
#else /* V7 */
		IF *cmdadr=='-'
		    ANDF (input=pathopen(nullstr, profile))>=0
		THEN	exfile(rflag); flags &= ~ttyflg;
		FI
		IF rflag==0 THEN flags |= rshflg FI
#endif

		/* open input file if specified */
		IF comdiv
		THEN	estabf(comdiv); input = -1;
		ELSE	input=((flags&stdflg) ? 0 : chkopen(cmdadr));
			comdiv--;
		FI
#ifdef stupid
	ELSE	*execargs=(STRING) dolv;	/* for `ps' cmd */
#endif
	FI

	exfile(0);
	done();
	/*NOTREACHED*/
	return(exitval);			/* GCC */
}

LOCAL VOID	exfile(prof)
	BOOL		prof;
{
	REG TIME	mailtime = 0;
	REG UID		userid;
	struct stat	statb;

	/* move input */
	IF input>0
	THEN	Ldup(input,INIO);
		input=INIO;
	FI

	/* move output to safe place */
	IF output==2
	THEN	Ldup(dup(2),OTIO);
		output=OTIO;
	FI

#if defined(SYSIII)
	userid=geteuid();
#else /* V7 */
	userid=getuid();
#endif

	/* decide whether interactive */
	IF (flags&intflg) ORF ((flags&oneflg)==0 ANDF isatty(output) ANDF isatty(input))
	THEN	dfault(&ps1nod, (userid?stdprompt:supprompt));
		dfault(&ps2nod, readmsg);
		flags |= ttyflg|prompt; ignsig(KILL);
	ELSE	flags |= prof; flags &= ~prompt;
	FI

	IF setjmp(errshell) ANDF prof
	THEN	close(input); return;
	FI

	/* error return here */
	loopcnt=breakcnt=peekc=0; iopend=0;
	IF input>=0 THEN initf(input) FI

	/* command loop */
	LOOP	tdystak(0);
		stakchk(); /* may reduce sbrk */
		exitset();
		IF (flags&prompt) ANDF standin->fstak==0 ANDF !eof
		THEN	IF mailnod.namval
#if defined(SYSIII)
			THEN	IF stat(mailnod.namval,&statb)>=0
				THEN	IF statb.st_size
					   ANDF mailtime
					   ANDF (statb.st_mtime != mailtime)
					THEN	prs(mailmsg)
					FI
					mailtime=statb.st_mtime;
				ELIF mailtime==0
				THEN	mailtime=1
				FI
			FI
#else /* V7 */
			    ANDF stat(mailnod.namval,&statb)>=0 ANDF statb.st_size
			    ANDF (statb.st_mtime != mailtime)
			    ANDF mailtime
			THEN	prs(mailmsg)
			FI
			mailtime=statb.st_mtime;
#endif
#if TIMEOUT > 0
			prs(ps1nod.namval); alarm(TIMEOUT); flags |= waiting;
#else /* !TIMEOUT */
			prs(ps1nod.namval);
#endif
		FI

		trapnote=0; peekc=readc();
		IF eof
		THEN	return;
		FI
#if TIMEOUT > 0
		alarm(0); flags &= ~waiting;
#endif
		execute(cmd(NL,MTFLG),0,0,0);
		eof |= (flags&oneflg);
	POOL
}

VOID	chkpr(eor)
	INT	eor;
{
	IF (flags&prompt) ANDF standin->fstak==0 ANDF eor==NL
	THEN	prs(ps2nod.namval);
	FI
}

VOID	settmp()
{
	itos(getpid()); serial=0;
	tmpnam=movstr(numbuf,&tmpout[TMPNAM]);
}

LOCAL VOID	Ldup(fa, fb)
	REG INT		fa, fb;
{
	dup2(fa, fb);
	close(fa);
	fcntl(fb, F_SETFD, FD_CLOEXEC);
}
