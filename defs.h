#
/*
 *	v7sh-amd64 (UNIX shell)
 *
 *	S. R. Bourne
 *	Bell Telephone Laboratories
 *
 * This port (v7sh-amd64) was developed with the assistance of Gemini and 
 * Claude and is free software; you can redistribute it and/or modify it 
 * under the terms of the GNU General Public License as published by the 
 * Free Software Foundation; either version 2 of the License.
 */
#include <compat.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/times.h>
#include	<sys/wait.h>
#include	<dirent.h>
#include	<errno.h>
#include	<fcntl.h>
#include	<setjmp.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>
#if defined(ULTRIX)
#if (__FreeBSD_version - 0) >= 500005
#include	<ulimit.h>
#else
#include	"ulimit.h"
#endif
#endif
#include	<unistd.h>

/* error exits from various parts of shell */
#define ERROR	1
#define SYNBAD	2
#define SIGFAIL 3
#define SIGFLG	0200

/* command tree */
#define FPRS	020
#define FINT	040
#define FAMP	0100
#define FPIN	0400
#define FPOU	01000
#define FPCL	02000
#define FCMD	04000
#define COMMSK	017

#define TCOM	0
#define TPAR	1
#define TFIL	2
#define TLST	3
#define TIF	4
#define TWH	5
#define TUN	6
#define TSW	7
#define TAND	8
#define TORF	9
#define TFORK	10
#define TFOR	11

/* execute table */
#define SYSSET	1
#define SYSCD	2
#define SYSEXEC	3
#define SYSLOGIN 4
#define SYSTRAP	5
#define SYSEXIT	6
#define SYSSHFT 7
#define SYSWAIT	8
#define SYSCONT 9
#define SYSBREAK 10
#define SYSEVAL 11
#define SYSDOT	12
#define SYSRDONLY 13
#define SYSTIMES 14
#define SYSXPORT 15
#define SYSNULL 16
#define SYSREAD 17
#define SYSTST	18
#define	SYSUMASK 19
#if defined(SYSIII)
#define SYSNEWGRP 20
#endif
#if defined(ULTRIX)
#define SYSULIMIT 21
#endif

#if defined(SYSIII)
/*	builtin table	*/
#define TEST	127
#endif

/* used for input and output of shell */
#define INIO	18	/* V7 - 10 */
#define OTIO	19	/* V7 - 11 */

/*io nodes*/
#define USERIO	10
#define IOUFD	15
#define IODOC	16
#define IOPUT	32
#define IOAPP	64
#define IOMOV	128
#define IORDW	256
#define INPIPE	0
#define OTPIPE	1

/* arg list terminator */
#define ENDARGS	0

#include	"mac.h"
#include	"mode.h"
#include	"name.h"


/* result type declarations */
/* args.c */
PROC INT	options(INT, STRING *);
PROC VOID	setargs(STRING *);
PROC DOLPTR	freeargs(DOLPTR);
PROC VOID	clearup(VOID);
PROC DOLPTR	useargs(VOID);
/* blok.c */
PROC ADDRESS	alloc(POS);
PROC VOID	addblok(POS);
PROC VOID	shfree(BLKPTR);
PROC VOID	blok_init(VOID);
/* builtin.c */
#if defined(SYSIII)
PROC VOID	builtin(INT, INT, STRING *);
#else /* V7 */
PROC INT	builtin(INT, STRING *);
#endif
PROC VOID	bfailed(CSTRING, CSTRING, CSTRING);
/* cmd.c */
PROC TREPTR	makefork(INT, TREPTR);
PROC TREPTR	cmd(INT, INT);
/* error.c */
PROC VOID	exitset(VOID);
PROC VOID	sigchk(VOID);
PROC VOID	failed(CSTRING, CSTRING);
PROC VOID	error(CSTRING);
PROC VOID	exitsh(INT);
PROC VOID	done(VOID);
PROC VOID	rmtemp(IOPTR);
/* expand.c */
PROC INT	expand(STRING, INT);
PROC INT	gmatch(STRING, STRING);
PROC VOID	makearg(ARGPTR);
/* expr.c */
/* PROC REAL	expr(); */
/* fault.c */
PROC VOID	fault(INT);
PROC VOID	stdsigs(VOID);
PROC SIGPTR	ignsig(INT);
PROC VOID	getsig(INT);
PROC VOID	oldsigs(VOID);
PROC VOID	clrsig(INT);
PROC VOID	chktrap(VOID);
/* io.c */
PROC VOID	initf(UFD);
PROC INT	estabf(STRING);
PROC VOID	push(FILEPTR);
PROC INT	pop(VOID);
PROC VOID	chkpipe(INT *);
PROC INT	chkopen(CSTRING);
#define rename	rename_
PROC VOID	rename(INT, INT);
PROC INT	create(STRING);
PROC INT	tmpfil(VOID);
PROC VOID	copy(IOPTR);
/* macro.c */
PROC STRING	macro(STRING);
PROC VOID	subst(INT, INT);
/* main.c */
PROC VOID	chkpr(INT);
PROC VOID	settmp(VOID);
/* name.c */
PROC INT	syslook(STRING, SYSTAB);
PROC VOID	setlist(ARGPTR, INT);
#if defined(SYSIII)
PROC INT	setname(STRING, INT);
#else /* V7 */
PROC VOID	setname(STRING, INT);
#endif
PROC VOID	replace(STRING *, CSTRING);
PROC VOID	dfault(NAMPTR, CSTRING);
PROC VOID	assign(NAMPTR, CSTRING);
PROC INT	readvar(STRING *);
PROC VOID	assnum(STRING *, INT);
PROC STRING	make(CSTRING);
PROC NAMPTR	lookup(CSTRING);
PROC VOID	namscan(VOID(*)(NAMPTR));
PROC VOID	freenames(VOID);
PROC VOID	printnam(NAMPTR);
PROC VOID	exname(NAMPTR);
PROC VOID	printflg(NAMPTR);
#define getenv getenv_
#if defined(SYSIII)
PROC INT	getenv(VOID);
#else /* V7 */
PROC VOID	getenv(VOID);
#endif
#define setenv setenv_
PROC STRING	*setenv(VOID);
/* print.c */
PROC VOID	newline(VOID);
PROC VOID	blank(VOID);
PROC VOID	prp(VOID);
PROC VOID	prs(CSTRING);
PROC VOID	prc(INT);
PROC VOID	prt(CLOCK);
PROC VOID	prn(INT);
PROC VOID	prl(LONG);
PROC VOID	itos(INT);
PROC VOID	ltos(LONG);
PROC INT	stoi(CSTRING);
/* service.c */
PROC VOID	initio(IOPTR);
#if defined(SYSIII)
PROC CSTRING	simple(CSTRING);
#endif
PROC CSTRING	getpath(CSTRING);
PROC INT	pathopen(CSTRING, CSTRING);
PROC CSTRING	catpath(CSTRING, CSTRING);
PROC VOID	execa(STRING *);
PROC VOID	postclr(VOID);
PROC VOID	post(INT);
#if defined(SYSIII)
PROC VOID	await(INT, INT);
#else /* V7 */
PROC VOID	await(INT);
#endif
PROC VOID	trim(STRING);
PROC STRING	mactrim(STRING);
PROC STRING	*scan(INT);
PROC INT	getarg(COMPTR);
/* setbrk.c */
PROC BYTPTR	setbrk(INT);
/* stak.c -> stak.h */
/* string.c */
PROC STRING	movstr(CSTRING, STRING);
PROC INT	any(INT, CSTRING);
PROC INT	cf(CSTRING, CSTRING);
PROC INT	length(CSTRING);
/* test.c */
#if defined(SYSIII)
PROC INT	test(INT, STRING *);
#endif
/* word.c */
PROC INT	word(VOID);
PROC INT	nextc(INT);
PROC INT	readc(VOID);
/* xec.c */
PROC INT	execute(TREPTR, INT, INT *, INT *);
PROC VOID	execexp(STRING, void *);

#define attrib(n,f)	(n->namflg |= f)
#define round(a,b)	(((intptr_t)((ADR(a)+b)-1))&~((intptr_t)(b)-1))
#define closepipe(x)	(close(x[INPIPE]), close(x[OTPIPE]))
#define eq(a,b)		(cf(a,b)==0)
#define max(a,b)	((a)>(b)?(a):(b))
#define assert(x)	;

/* temp files and io */
EXTERN UFD	output;
EXTERN INT	ioset;
EXTERN IOPTR	iotemp;		/* files to be deleted sometime */
EXTERN IOPTR	iopend;		/* documents waiting to be read at NL */
#if defined(SYSIII)
EXTERN INT	stripflg;
#endif
EXTERN BOOL	nosubst;

/* substitution */
EXTERN INT	dolc;
EXTERN STRING	*dolv;
EXTERN DOLPTR	argfor;
EXTERN ARGPTR	gchain;

/* stack */
#define		BLK(x)	((BLKPTR)(x))
#define		BYT(x)	((BYTPTR)(x))
#define		STK(x)	((STKPTR)(x))
#define		ADR(x)	((char*)(x))

/* stak stuff */
#include	"stak.h"

/* string constants */
EXTERN CMSG	atline;
EXTERN CMSG	readmsg;
EXTERN CMSG	colon;
EXTERN CMSG	minus;
EXTERN CMSG	nullstr;
EXTERN CMSG	sptbnl;
EXTERN CMSG	unexpected;
EXTERN CMSG	endoffile;
EXTERN CMSG	endofline;
EXTERN CMSG	synmsg;

/* name tree and words */
EXTERN SYSTAB	reserved;
EXTERN SYSTAB	commands;
#if defined(SYSIII)
EXTERN SYSTAB	builtins;
#endif
EXTERN INT	wdval;
EXTERN INT	wdnum;
EXTERN ARGPTR	wdarg;
EXTERN INT	wdset;
EXTERN BOOL	reserv;

/* prompting */
EXTERN CMSG	stdprompt;
EXTERN CMSG	supprompt;
EXTERN CMSG	profile;
#if defined(SYSIII)
EXTERN CMSG	sysprofile;
#endif

/* built in names */
EXTERN NAMNOD	fngnod;
EXTERN NAMNOD	ifsnod;
EXTERN NAMNOD	homenod;
EXTERN NAMNOD	mailnod;
EXTERN NAMNOD	pathnod;
EXTERN NAMNOD	ps1nod;
EXTERN NAMNOD	ps2nod;

/* special names */
EXTERN MSG	flagadr;
EXTERN STRING	cmdadr;
EXTERN STRING	exitadr;
EXTERN STRING	dolladr;
EXTERN STRING	pcsadr;
EXTERN STRING	pidadr;

EXTERN CMSG	defpath;

/* names always present */
EXTERN CMSG	mailname;
EXTERN CMSG	homename;
EXTERN CMSG	pathname;
EXTERN CMSG	fngname;
EXTERN CMSG	ifsname;
EXTERN CMSG	ps1name;
EXTERN CMSG	ps2name;

/* transput */
EXTERN CHAR	tmpout[];
#define tmpnam	tmpnam_
EXTERN STRING	tmpnam;
EXTERN INT	serial;
#define		TMPNAM 7
EXTERN FILEPTR	standin;
#define input	(standin->fdes)
#define eof	(standin->feof)
EXTERN INT	peekc;
EXTERN STRING	comdiv;
EXTERN CMSG	devnull;

/* flags */
#define		noexec	01
#define		intflg	02
#define		prompt	04
#define		setflg	010
#define		errflg	020
#define		ttyflg	040
#define		forked	0100
#define		oneflg	0200
#define		rshflg	0400
#define		waiting	01000
#define		stdflg	02000
#define		STDFLG	's'
#define		STDFLGLOC 4
#define		execpr	04000
#define		readpr	010000
#define		keyflg	020000
EXTERN INT	flags;
#if defined(SYSIII)
EXTERN BOOL	rwait;
EXTERN BOOL	wasintr;	/*	used to tell if break or delete is hit
					while executing a wait	*/
#endif

/* error exits from various parts of shell */
EXTERN jmp_buf	subshell;
EXTERN jmp_buf	errshell;
#if defined(RENO)
EXTERN jmp_buf	INTbuf;
#endif

/* fault handling */
#include	"brkincr.h"
EXTERN INT	brkincr;

#define MINTRAP	0
#define MAXTRAP	33

#define INTR	SIGINT
#define QUIT	SIGQUIT
#define MEMF	SIGSEGV
#define ALARM	SIGALRM
#define KILL	SIGTERM
#define TRAPSET	2
#define SIGSET	4
#define SIGMOD	8
#if defined(SYSIII)
#define SIGCAUGHT 16
#endif

EXTERN BOOL	trapnote;
EXTERN STRING	trapcom[];
EXTERN BOOL	trapflg[];
#if defined(RENO)
EXTERN BOOL	trapjmp[];
#endif

/* name tree and words */
EXTERN STRING	*environ;
EXTERN CHAR	numbuf[];
EXTERN CMSG	export;
EXTERN CMSG	readonly;

/* execflgs */
EXTERN INT	exitval;
EXTERN BOOL	execbrk;
EXTERN INT	loopcnt;
EXTERN INT	breakcnt;

/* messages */
EXTERN CMSG	mailmsg;
EXTERN CMSG	coredump;
EXTERN CMSG	badopt;
EXTERN CMSG	badparam;
#if defined(SYSIII)
EXTERN CMSG	unset;
#endif
EXTERN CMSG	badsub;
EXTERN CMSG	nospace;
EXTERN CMSG	badtrap;
EXTERN CMSG	memfault;
EXTERN CMSG	baddir;
EXTERN CMSG	badshift;
EXTERN CMSG	illegal;
EXTERN CMSG	restricted;
EXTERN CMSG	execpmsg;
EXTERN CMSG	notid;
EXTERN CMSG	badulimit;
EXTERN CMSG	wtfailed;
EXTERN CMSG	badcreate;
#if defined(SYSIII)
EXTERN CMSG	nofork;
EXTERN CMSG	noswap;
#endif
EXTERN CMSG	piperr;
EXTERN CMSG	badopen;
EXTERN CMSG	badnum;
EXTERN CMSG	arglist;
#if defined(SYSIII)
EXTERN CMSG	argcount;
#endif
EXTERN CMSG	txtbsy;
EXTERN CMSG	toobig;
EXTERN CMSG	badexec;
EXTERN CMSG	notfound;
EXTERN CMSG	badfile;
EXTERN CSTRING	sysmsg[];
#if defined(RENO)
EXTERN INT	num_sysmsg;
#endif

#if defined(SYSIII)
/*	'builtin' error messages	*/
EXTERN CMSG	btest;
EXTERN CMSG	badop;
#endif

#if defined(SYSIII)
/*	fork constant	*/
#define FORKLIM	32
#endif

#if defined(SYSIII) || defined(RENO)
/*	comment delimeter	*/
#define COMCHAR	'#'
#endif

EXTERN address	end[];

#include	"ctype.h"

