#
/*
 * v7sh-amd64 (UNIX shell)
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include	"defs.h"



#define ARGMK	01

LOCAL CSTRING	execs(CSTRING, STRING *);
LOCAL VOID	gsort(STRING *, STRING *);
LOCAL INT	split(STRING);
#if defined(GOCSH) /* RENO */
LOCAL VOID	gocsh(STRING *, STRING, STRING *);
#endif


/* service routines for `execute' */

VOID	initio(iop)
	IOPTR		iop;
{
	REG STRING	ion;
	REG INT		iof, fd=-1;		/* GCC */

	IF iop
	THEN	iof=iop->iofile;
		ion=mactrim(iop->ioname);
		IF *ion ANDF (flags&noexec)==0
		THEN	IF iof&IODOC
			THEN	subst(chkopen(ion),(fd=tmpfil()));
				close(fd); fd=chkopen(tmpout); unlink(tmpout);
			ELIF iof&IOMOV
			THEN	IF eq(minus,ion)
				THEN	fd = -1;
					close(iof&IOUFD);
				ELIF (fd=stoi(ion))>=USERIO
				THEN	failed(ion,badfile);
					/*NOTREACHED*/
				ELSE	fd=dup(fd);
				FI
			ELIF (iof&IOPUT)==0
			THEN	fd=chkopen(ion);
			ELIF flags&rshflg
			THEN	failed(ion,restricted);
				/*NOTREACHED*/
#if defined(RENO)
			ELIF (iof&IOAPP)==0 ORF
			     (fd=open(ion,O_WRONLY|O_APPEND))<0
			THEN	fd=create(ion);
#else /* V7 */
			ELIF iof&IOAPP ANDF (fd=open(ion,O_WRONLY))>=0
			THEN	lseek(fd, (OFFSET) 0, SEEK_END);
			ELSE	fd=create(ion);
#endif
			FI
			IF fd>=0
			THEN	rename(fd,iof&IOUFD);
			FI
		FI
		initio(iop->ionxt);
	FI
}

CSTRING	getpath(s)
	CSTRING		s;
{
	REG CSTRING	path;

#if defined(SYSIII)
	IF any('/',s) ORF any(('/'|QUOTE),s)
#else /* V7 */
	IF any('/',s)
#endif
	THEN	IF flags&rshflg
		THEN	failed(s, restricted);
			/*NOTREACHED*/
		FI
	ELIF (path = pathnod.namval)==0
	THEN	return(defpath);
	ELSE	return(cpystak(path));
	FI
	return(nullstr);
}

INT	pathopen(path, name)
	REG CSTRING	path, name;
{
	REG UFD		f;

	REP path=catpath(path,name);
	PER (f=open(curstak(),O_RDONLY))<0 ANDF path DONE
	return(f);
}

CSTRING	catpath(path,name)
	REG CSTRING	path;
	CSTRING		name;
{
	/* leaves result on top of stack */
	REG CSTRING	scanp = path;
	REG STRING	argp = locstak();

	WHILE *scanp ANDF *scanp!=COLON DO *argp++ = *scanp++ OD
	IF scanp!=path THEN *argp++='/' FI
	IF *scanp==COLON THEN scanp++ FI
	path=(*scanp ? scanp : 0); scanp=name;
	WHILE (*argp++ = *scanp++) DONE
	return(path);
}

LOCAL CSTRING	xecmsg;
LOCAL STRING	*xecenv;

VOID	execa(at)
	STRING		at[];
{
	REG CSTRING	path;
	REG STRING	*t = at;

	IF (flags&noexec)==0
	THEN	xecmsg=notfound; path=getpath(*t);
		namscan(exname);
		xecenv=setenv();
		WHILE (path=execs(path,t))!=NIL DONE	/* GCC */
		failed(*t,xecmsg);
		/*NOTREACHED*/
	FI
}


LOCAL CSTRING	execs(ap,t)
	CSTRING		ap;
	REG STRING	t[];
{
	REG STRING	p;
	REG CSTRING	prefix;

	prefix=catpath(ap,t[0]);
	trim(p=curstak());

	sigchk();
	execve(p, &t[0] ,xecenv);
	SWITCH errno IN

	    case ENOEXEC:
		flags=0;
		comdiv=0; ioset=0;
		clearup(); /* remove open files and for loop junk */
		IF input THEN close(input) FI
		close(output); output=2;
		input=chkopen(p);

#if defined(GOCSH) /* RENO */
		/* band aid to get csh... 2/26/79 */
		BEGIN
			CHAR c;

			IF !isatty(input)
			THEN	read(input, &c, 1);
				IF c == '#' THEN gocsh(t, p, xecenv) FI
				lseek(input, (OFFSET) 0, SEEK_SET);
			FI
		END
#endif

		/* set up new args */
		setargs(t);
		longjmp(subshell,1);
		/*NOTREACHED*/

	    case ENOMEM:
		failed(p,toobig);
		/*NOTREACHED*/

	    case E2BIG:
		failed(p,arglist);
		/*NOTREACHED*/

	    case ETXTBSY:
		failed(p,txtbsy);
		/*NOTREACHED*/

	    default:
		xecmsg=badexec;
		/*FALLTHROUGH*/

	    case ENOENT:
		return(prefix);
	ENDSW
}

#if defined(GOCSH) /* RENO */
LOCAL VOID	gocsh(t, cp, xecenv)
	REG STRING *t, cp, *xecenv;
{
	STRING *newt[1000];
	REG STRING *p;
	REG INT i;

	FOR i = 0; t[i]; i++ DO newt[i+1] = t[i] OD
	newt[i+1] = 0;
	newt[0] = _PATH_CSHELL;
	newt[1] = cp;
	execve(_PATH_CSHELL, newt, xecenv);
}
#endif

/* for processes to be waited for */
#define MAXP 20
LOCAL INT	pwlist[MAXP];
LOCAL INT	pwc;

VOID	postclr()
{
	REG INT		*pw = pwlist;

	WHILE pw <= &pwlist[pwc]
	DO *pw++ = 0 OD
	pwc=0;
}

VOID	post(pcsid)
	INT		pcsid;
{
	REG INT		*pw = pwlist;

	IF pcsid
	THEN	WHILE *pw DO pw++ OD
		IF pwc >= MAXP-1
		THEN	pw--;
		ELSE	pwc++;
		FI
		*pw = pcsid;
	FI
}

#if defined(SYSIII)
VOID	await(i, bckg)
	INT		i, bckg;
#else /* V7 */
VOID	await(i)
	INT		i;
#endif
{
	INT		rc=0, wx=0;
	INT		w;
	INT		ipwc = pwc;

	post(i);
	WHILE pwc
	DO	REG INT		p;
		REG INT		sig;
		INT		w_hi;
#if defined(SYSIII)
		INT		found = 0;
#endif

		BEGIN
		   REG INT	*pw=pwlist;
#if defined(RENO)
		   IF setjmp(INTbuf) == 0
		   THEN	trapjmp[INTR] = 1;
#endif
		   p=wait(&w);
#if defined(SYSIII)
			IF wasintr THEN
				wasintr = 0;
				IF bckg THEN break; FI
			FI
#endif
#if defined(RENO)
		   ELSE	p = -1;
		   FI
		   trapjmp[INTR] = 0;
#endif
		   WHILE pw <= &pwlist[ipwc]
		   DO IF *pw==p
		      THEN *pw=0; pwc--;
#if defined(SYSIII)
			   found++;
#endif
		      ELSE pw++;
		      FI
		   OD
		END

#if defined(SYSIII)
		IF p == -1
		THEN	IF bckg THEN
				REG INT *pw =pwlist;
				WHILE pw <= &pwlist[ipwc] ANDF i != *pw
				DO pw++; OD
				IF i == *pw THEN *pw = 0; pwc-- FI
			FI
			continue
		FI
#else /* V7 */
		IF p == -1 THEN continue FI
#endif

		w_hi = (w>>8)&LOBYTE;

		IF (sig = w&0177)!=0		/* GCC */
		THEN	IF sig == 0177	/* ptrace! return */
			THEN	prs("ptrace: ");
				sig = w_hi;
			FI
#if defined(RENO)
			IF sig < num_sysmsg ANDF sysmsg[sig]
#else /* V7 */
			IF sysmsg[sig]
#endif
			THEN	IF i!=p ORF (flags&prompt)==0 THEN prp(); prn(p); blank() FI
				prs(sysmsg[sig]);
				IF w&0200 THEN prs(coredump) FI
			FI
			newline();
		FI

#if defined(SYSIII)
		IF rc==0 ANDF found != 0
#else /* V7 */
		IF rc==0
#endif
		THEN	rc = (sig ? sig|SIGFLG : w_hi);
		FI
		wx |= w;
#if defined(SYSIII)
		IF p == i THEN break FI
#endif
	OD

	IF wx ANDF flags&errflg
	THEN	exitsh(rc);
		/*NOTREACHED*/
	FI
	exitval=rc; exitset();
}

VOID	trim(at)
	STRING		at;
{
	REG STRING	p;
	REG INT		c;
	REG INT		q=0;

	IF (p=at)!=NIL				/* GCC */
	THEN	WHILE (c = *p)!=0		/* GCC */
		DO *p++=c&STRIP; q |= c OD
	FI
	nosubst=q&QUOTE;
}

STRING	mactrim(s)
	STRING		s;
{
	REG STRING	t=macro(s);
	trim(t);
	return(t);
}

STRING	*scan(argn)
	INT		argn;
{
	REG ARGPTR	argp = (ARGPTR) (Rcheat(gchain)&~ARGMK);
	REG STRING	*comargn, *comargm;

	comargn=(STRING *) getstak(BYTESPERWORD*argn+BYTESPERWORD);
	comargm = comargn += argn; *comargn = ENDARGS;

	WHILE argp
	DO	*--comargn = argp->argval;
		IF (argp = argp->argnxt)!=NIL	/* GCC */
		THEN trim(*comargn);
		FI
		IF argp==0 ORF Rcheat(argp)&ARGMK
		THEN	gsort(comargn,comargm);
			comargm = comargn;
		FI
		/* Lcheat(argp) &= ~ARGMK; */
		argp = (ARGPTR) (Rcheat(argp)&~ARGMK);
	OD
	return(comargn);
}

LOCAL VOID	gsort(from,to)
	STRING		from[], to[];
{
	INT		k, m, n;
	REG INT		i, j;

	IF (n=to-from)<=1 THEN return FI

	FOR j=1; j<=n; j*=2 DONE

	FOR m=2*j-1; m/=2;
	DO  k=n-m;
	    FOR j=0; j<k; j++
	    DO	FOR i=j; i>=0; i-=m
		DO  REG STRING *fromi; fromi = &from[i];
		    IF cf(fromi[m],fromi[0])>0
		    THEN break;
		    ELSE STRING s; s=fromi[m]; fromi[m]=fromi[0]; fromi[0]=s;
		    FI
		OD
	    OD
	OD
}

/* Argument list generation */

INT	getarg(ac)
	COMPTR		ac;
{
	REG ARGPTR	argp;
	REG INT		count=0;
	REG COMPTR	c;

	IF (c=ac)!=NIL				/* GCC */
	THEN	argp=c->comarg;
		WHILE argp
		DO	count += split(macro(argp->argval));
			argp=argp->argnxt;
		OD
	FI
	return(count);
}

LOCAL INT	split(s)
	/* blank interpretation routine */
	REG STRING	s;
{
	REG STRING	argp;
	REG INT		c;
	INT		count=0;

	LOOP	sigchk(); argp=(STRING) (locstak()+BYTESPERWORD);
		WHILE (c = *s++, !any(c,ifsnod.namval) && c)
		DO *argp++ = c OD
		IF argp==(STRING)(staktop+BYTESPERWORD)
		THEN	IF c
			THEN	continue;
			ELSE	return(count);
			FI
		ELIF c==0
		THEN	s--;
		FI
		/* file name generation */
		IF (c=expand(((ARGPTR) (argp=(STRING) endstak(argp)))->argval,0))!=0 /* GCC */
		THEN	count += c;
		ELSE	/* assign(&fngnod, argp->argval); */
			makearg((ARGPTR) argp); count++;
		FI
		Lcheat(gchain) |= ARGMK;
	POOL
}

#if defined(SYSIII)
CSTRING	simple(s)
	CSTRING		s;
{
	CSTRING		sname=s;

	LOOP
		IF any('/', sname)
		THEN	WHILE *sname++ != '/' DONE
		ELSE	return(sname);
		FI
	POOL
}
#endif
