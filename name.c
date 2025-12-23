#
/*
 * v7sh-amd64 (UNIX shell)
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include	"defs.h"

LOCAL BOOL	chkid(CSTRING);
LOCAL VOID	namwalk(NAMPTR);
LOCAL STRING	staknam(NAMPTR);
LOCAL VOID	countnam(NAMPTR);
LOCAL VOID	pushnam(NAMPTR);

NAMNOD	ps2nod	= {	NIL,		NIL,		ps2name,  NIL,NIL,0},
	fngnod	= {	NIL,		NIL,		fngname,  NIL,NIL,0},
	pathnod = {	NIL,		NIL,		pathname, NIL,NIL,0},
	ifsnod	= {	NIL,		NIL,		ifsname,  NIL,NIL,0},
	ps1nod	= {	&pathnod,	&ps2nod,	ps1name,  NIL,NIL,0},
	homenod = {	&fngnod,	&ifsnod,	homename, NIL,NIL,0},
	mailnod = {	&homenod,	&ps1nod,	mailname, NIL,NIL,0};

LOCAL NAMPTR	namep = &mailnod;


/* ========	variable and string handling	======== */

INT	syslook(w,syswds)
	STRING		w;
	SYSTAB		syswds;
{
	REG INT		first;
	REG CSTRING	s;
	REG SYSPTR	syscan;

	syscan=syswds; first = *w;

	WHILE (s=syscan->sysnam)!=NIL		/* GCC */
	DO  IF first == *s
		ANDF eq(w,s)
	    THEN return(syscan->sysval);
	    FI
	    syscan++;
	OD
	return(0);
}

VOID	setlist(arg,xp)
	REG ARGPTR	arg;
	INT		xp;
{
	WHILE arg
	DO REG STRING	s=mactrim(arg->argval);
	   setname(s, xp);
	   arg=arg->argnxt;
	   IF flags&execpr
	   THEN prs(s);
		IF arg THEN blank(); ELSE newline(); FI
	   FI
	OD
}

#if defined(SYSIII)
INT	setname(argi, xp)
#else /* V7 */
VOID	setname(argi, xp)
#endif
	STRING		argi;
	INT		xp;
{
#if defined(SYSIII)
	INT		rsflag=1;	/* local restricted flag */
#endif
	REG STRING	argscan=argi;
	REG NAMPTR	n;

	IF letter((INT) *argscan)
	THEN	WHILE alphanum((INT) *argscan) DO argscan++ OD
		IF *argscan=='='
		/* make name a cohesive string */
		THEN	*argscan = 0;
#if defined(SYSIII)
		    /*	restricted stuff excluded from research	*/
		   IF eq(argi, "SHELL") ANDF !(flags&rshflg)
		   THEN	argscan++;
			IF any('r', simple(argscan))
			THEN	rsflag=0;	/* restricted shell */
			FI
			argscan--;
		   FI
		   IF eq(argi,pathname) ANDF (flags&rshflg) /* cannot set PATH */
		   THEN	failed(argi,restricted);
			/*NOTREACHED*/
		   ELIF eq(argi, "SHELL") ANDF (flags&rshflg)
		   THEN	failed(argi, restricted);
			/*NOTREACHED*/
		   ELSE
#endif
			n=lookup(argi);
			*argscan++ = '=';
			attrib(n, xp);
			IF xp&N_ENVNAM
#if defined(RENO)
			THEN
				/*
				 * Importing IFS can be very dangerous
				 */
				IF !bcmp(argi, "IFS=", sizeof("IFS=") - 1)
				THEN
					UID uid;
					IF (uid = getuid())!=geteuid() ORF !uid
					THEN
#if defined(SYSIII)
						return(0);
#else /* V7 */
						return;
#endif
					FI
				FI
				n->namenv = n->namval = argscan;
#else /* V7 */
			THEN	n->namenv = n->namval = argscan;
#endif
			ELSE	assign(n, argscan);
			FI
#if defined(SYSIII)
			return(rsflag);
		    FI
#else /* V7 */
			return;
#endif
		FI
	FI
	failed(argi,notid);
	/*NOTREACHED*/
#if defined(SYSIII)
	return(0);
#endif
}

VOID	replace(a, v)
	REG STRING	*a;
	CSTRING		v;
{
	shfree((BLKPTR) *a); *a=make(v);
}

VOID	dfault(n,v)
	NAMPTR		n;
	CSTRING		v;
{
	IF n->namval==0
	THEN	assign(n,v)
	FI
}

VOID	assign(n,v)
	NAMPTR		n;
	CSTRING		v;
{
	IF n->namflg&N_RDONLY
	THEN	failed(n->namid,wtfailed);
		/*NOTREACHED*/
	ELSE	replace(&n->namval,v);
	FI
}

INT	readvar(names)
	STRING		*names;
{
	FILEBLK		fb;
	REG FILEPTR	f = &fb;
	REG INT		c;
	REG INT		rc=0;
	NAMPTR		n=lookup(*names++); /* done now to avoid storage mess */
	STKPTR		rel=(STKPTR) relstak();

	push(f); initf(dup(0));
	IF lseek(0,(OFFSET) 0,SEEK_CUR)==-1
	THEN	f->fsiz=1;
	FI

#if defined(SYSIII)
	/*	strip leading IFS characters */
	WHILE (any((c=nextc(0)), ifsnod.namval)) ANDF !(eolchar(c)) DONE
	LOOP
#else /* V7 */
	LOOP	c=nextc(0);
#endif
		IF (*names ANDF any(c, ifsnod.namval)) ORF eolchar(c)
		THEN	zerostak();
			assign(n,absstak(rel)); setstak(rel);
			IF *names
			THEN	n=lookup(*names++);
			ELSE	n=0;
			FI
			IF eolchar(c)
			THEN	break;
#if defined(SYSIII)
			ELSE	/*	strip imbedded IFS characters	*/
				WHILE (any((c=nextc(0)), ifsnod.namval)) ANDF
				      !(eolchar(c)) DONE
#endif
			FI
		ELSE	pushstak(c);
#if defined(SYSIII)
			c=nextc(0);
#endif
		FI
	POOL
	WHILE n
	DO assign(n, nullstr);
	   IF *names THEN n=lookup(*names++); ELSE n=0; FI
	OD

	IF eof THEN rc=1 FI
	lseek(0, (OFFSET) (f->fnxt-f->fend), SEEK_CUR);
	pop();
	return(rc);
}

VOID	assnum(p, i)
	STRING		*p;
	INT		i;
{
	itos(i); replace(p,numbuf);
}

STRING	make(v)
	CSTRING		v;
{
	REG STRING	p;

	IF v
	THEN	movstr(v,p=(STRING) alloc((POS) length(v)));
		return(p);
	ELSE	return(0);
	FI
}


NAMPTR	lookup(nam)
	REG CSTRING	nam;
{
	REG NAMPTR	nscan=namep;
	REG NAMPTR	*prev=NIL;		/* GCC */
	INT		LR;

	IF !chkid(nam)
	THEN	failed(nam,notid);
		/*NOTREACHED*/
	FI
	WHILE nscan
	DO	IF (LR=cf(nam,nscan->namid))==0
		THEN	return(nscan);
		ELIF LR<0
		THEN	prev = &(nscan->namlft);
		ELSE	prev = &(nscan->namrgt);
		FI
		nscan = *prev;
	OD

	/* add name node */
	nscan=(NAMPTR) alloc(sizeof *nscan);
	nscan->namlft=nscan->namrgt=NIL;
	nscan->namid=make(nam);
	nscan->namval=0; nscan->namflg=N_DEFAULT; nscan->namenv=0;
	return(*prev = nscan);
}

LOCAL BOOL	chkid(nam)
	CSTRING		nam;
{
	REG CSTRING	cp=nam;

	IF !letter((INT) *cp)
	THEN	return(FALSE);
	ELSE	WHILE *++cp
		DO IF !alphanum((INT) *cp)
		   THEN	return(FALSE);
		   FI
		OD
	FI
	return(TRUE);
}

LOCAL VOID (*namfn)(NAMPTR);

VOID	namscan(fn)
	VOID		(*fn)(NAMPTR);
{
	namfn=fn;
	namwalk(namep);
}

LOCAL VOID	namwalk(np)
	REG NAMPTR	np;
{
	IF np
	THEN	namwalk(np->namlft);
		(*namfn)(np);
		namwalk(np->namrgt);
	FI
}

VOID	printnam(n)
	NAMPTR		n;
{
	REG STRING	s;

	sigchk();
	IF (s=n->namval)!=NIL			/* GCC */
	THEN	prs(n->namid);
		prc('='); prs(s);
		newline();
	FI
}

LOCAL STRING	staknam(n)
	REG NAMPTR	n;
{
	REG STRING	p;

	p=movstr(n->namid,staktop);
	p=movstr("=",p);
	p=movstr(n->namval,p);
	return(getstak((POS) (p+1-ADR(stakbot))));
}

VOID	exname(n)
	REG NAMPTR	n;
{
	IF n->namflg&N_EXPORT
	THEN	shfree((BLKPTR) n->namenv);
		n->namenv = make(n->namval);
	ELSE	shfree((BLKPTR) n->namval);
		n->namval = make(n->namenv);
	FI
}

VOID	printflg(n)
	REG NAMPTR		n;
{
	IF n->namflg&N_EXPORT
	THEN	prs(export); blank();
	FI
	IF n->namflg&N_RDONLY
	THEN	prs(readonly); blank();
	FI
	IF n->namflg&(N_EXPORT|N_RDONLY)
	THEN	prs(n->namid); newline();
	FI
}

#if defined(SYSIII)
INT	getenv()
{
	INT	rsflag=1;	/* local restricted flag */
	REG STRING	*e=environ;

	WHILE *e DO rsflag=setname(*e++, N_ENVNAM) & rsflag OD
	return(rsflag);
}
#else /* V7 */
VOID	getenv()
{
	REG STRING	*e=environ;

	WHILE *e
	DO setname(*e++, N_ENVNAM) OD
}
#endif

LOCAL POS	namec;

LOCAL VOID	countnam(n)
	NAMPTR		n;
{
	n=n;					/* GCC */
	namec++;
}

LOCAL STRING 	*argnam;

LOCAL VOID	pushnam(n)
	NAMPTR		n;
{
	IF n->namval
	THEN	*argnam++ = staknam(n);
	FI
}

STRING	*setenv()
{
	REG STRING	*er;

	namec=0;
	namscan(countnam);
	argnam = er = (STRING *) getstak(namec*BYTESPERWORD+BYTESPERWORD);
	namscan(pushnam);
	*argnam++ = 0;
	return(er);
}
