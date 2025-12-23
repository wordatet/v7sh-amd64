#
/*
 * v7sh-amd64 (UNIX shell)
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include	"defs.h"
#include	"sym.h"

LOCAL VOID	copyto(INT);
LOCAL VOID	skipto(INT);
LOCAL INT	getch(INT);
LOCAL VOID	comsubst(VOID);
#define flush	flush_
LOCAL VOID	flush(INT);

LOCAL INT	quote;	/* used locally */
LOCAL BOOL	quoted;	/* used locally */


LOCAL VOID	copyto(endch)
	REG INT		endch;
{
	REG INT		c;

	WHILE (c=getch(endch))!=endch ANDF c
	DO pushstak(c|quote) OD
	zerostak();
	IF c!=endch
	THEN	error(badsub);
		/*NOTREACHED*/
	FI
}

LOCAL VOID	skipto(endch)
	REG INT		endch;
{
	/* skip chars up to } */
	REG INT		c;

	WHILE (c=readc()) ANDF c!=endch
	DO	SWITCH c IN

		case SQUOTE:	skipto(SQUOTE); break;

		case DQUOTE:	skipto(DQUOTE); break;

		case DOLLAR:	IF readc()==BRACE
				THEN	skipto('}');
				FI
		ENDSW
	OD
	IF c!=endch
	THEN	error(badsub);
		/*NOTREACHED*/
	FI
}

LOCAL INT	getch(endch)
	CHAR		endch;
{
	REG INT		d;

retry:
	d=readc();
	IF !subchar(d)
	THEN	return(d);
	FI
	IF d==DOLLAR
	THEN	REG INT	c;
		IF (c=readc(), dolchar(c))
		THEN	NAMPTR		n=NIL;
			INT		dolg=0;
			BOOL		bra;
			REG STRING	argp, v=NIL;	/* GCC */
#if defined(SYSIII)
			BOOL		nulflg;
#endif
			CHAR		idb[2];
			STRING		id=idb;

			IF (bra=c==BRACE)!=0 THEN c=readc() FI	/* GCC */
			IF letter(c)
			THEN	argp=(STRING) relstak();
				WHILE alphanum(c) DO pushstak(c); c=readc() OD
				zerostak();
				n=lookup(absstak(argp)); setstak(argp);
				v = n->namval; id = (STRING) n->namid;
				peekc = c|MARK;;
			ELIF digchar(c)
			THEN	*id=c; idb[1]=0;
				IF astchar(c)
				THEN	dolg=1; c='1';
				FI
				c -= '0';
				v=((c==0) ? cmdadr : (c<=dolc) ? dolv[c] : (STRING) (dolg=0));
			ELIF c=='$'
			THEN	v=pidadr;
			ELIF c=='!'
			THEN	v=pcsadr;
			ELIF c=='#'
			THEN	v=dolladr;
			ELIF c=='?'
			THEN	v=exitadr;
			ELIF c=='-'
			THEN	v=flagadr;
			ELIF bra
			THEN	error(badsub);
				/*NOTREACHED*/
			ELSE	goto retry;
			FI
			c = readc();
#if defined(SYSIII)
			IF c==':' ANDF bra	/* null and unset fix */
			THEN	nulflg=1; c=readc();
			ELSE	nulflg=0;
			FI
#endif
			IF !defchar(c) ANDF bra
			THEN	error(badsub);
				/*NOTREACHED*/
			FI
			argp=0;
			IF bra
			THEN	IF c!='}'
				THEN	argp=(STRING) relstak();
#if defined(SYSIII)
					IF (v==0 ORF (nulflg ANDF *v==0)) NEQ (setchar(c))
#else /* V7 */
					IF (v==0)NEQ(setchar(c))
#endif
					THEN	copyto('}');
					ELSE	skipto('}');
					FI
					argp=absstak(argp);
				FI
			ELSE	peekc = c|MARK; c = 0;
			FI
#if defined(SYSIII)
			IF v ANDF (!nulflg ORF *v)
#else /* V7 */
			IF v
#endif
			THEN	IF c!='+'
				THEN	LOOP WHILE (c = *v++)!=0 /* GCC */
					     DO pushstak(c|quote); OD
					     IF dolg==0 ORF (++dolg>dolc)
					     THEN break;
					     ELSE v=dolv[dolg]; pushstak(SP|(*id=='*' ? quote : 0));
					     FI
					POOL
				FI
			ELIF argp
			THEN	IF c=='?'
				THEN	failed(id,*argp?argp:badparam);
					/*NOTREACHED*/
				ELIF c=='='
				THEN	IF n
					THEN	assign(n,argp);
					ELSE	error(badsub);
						/*NOTREACHED*/
					FI
				FI
			ELIF flags&setflg
#if defined(SYSIII)
			THEN	failed(id,unset);
#else /* V7 */
			THEN	failed(id,badparam);
#endif
				/*NOTREACHED*/
			FI
			goto retry;
		ELSE	peekc=c|MARK;
		FI
	ELIF d==endch
	THEN	return(d);
	ELIF d==SQUOTE
	THEN	comsubst(); goto retry;
	ELIF d==DQUOTE
	THEN	quoted++; quote^=QUOTE; goto retry;
	FI
	return(d);
}

STRING	macro(as)
	STRING		as;
{
	/* Strip "" and do $ substitution
	 * Leaves result on top of stack
	 */
	REG BOOL	savqu =quoted;
	REG INT		savq = quote;
	FILEHDR		fb;

	push((FILEPTR) &fb); estabf(as);
	usestak();
	quote=0; quoted=0;
	copyto(0);
	pop();
	IF quoted ANDF (stakbot==staktop) THEN pushstak(QUOTE) FI
	/* above is the fix for *'.c' bug */
	quote=savq; quoted=savqu;
	return(fixstak());
}

LOCAL VOID	comsubst()
{
	/* command substn */
	FILEBLK		cb;
	REG INT		d;
	REG STKPTR	savptr = fixstak();

	usestak();
	WHILE (d=readc())!=SQUOTE ANDF d
	DO pushstak(d) OD

	BEGIN
	   REG STRING	argc;
	   trim(argc=fixstak());
	   push(&cb); estabf(argc);
	END
	BEGIN
	   REG TREPTR	t = makefork(FPOU,cmd(EOFSYM,MTFLG|NLFLG));
	   INT		pv[2];

	   /* this is done like this so that the pipe
	    * is open only when needed
	    */
	   chkpipe(pv);
	   initf(pv[INPIPE]);
	   execute(t, 0, 0, pv);
	   close(pv[OTPIPE]);
	END
	tdystak(savptr); staktop=movstr(savptr,stakbot);
#if defined(RENO)
	WHILE (d=readc())!=0 DO locstak(); pushstak(d|quote) OD
#else /* V7 */
	WHILE (d=readc())!=0 DO pushstak(d|quote) OD	/* GCC */
#endif
#if defined(SYSIII)
	await(0,0);
#else /* V7 */
	await(0);
#endif
	WHILE stakbot!=staktop
	DO	IF (*--staktop&STRIP)!=NL
		THEN	++staktop; break;
		FI
	OD
	pop();
}

#define CPYSIZ	512

VOID	subst(in,ot)
	INT		in, ot;
{
	REG INT		c;
	FILEBLK		fb;
	REG INT		count=CPYSIZ;

	push(&fb); initf(in);
	/* DQUOTE used to stop it from quoting */
	WHILE (c=(getch(DQUOTE)&STRIP))!=0		/* GCC */
	DO pushstak(c);
	   IF --count == 0
	   THEN	flush(ot); count=CPYSIZ;
	   FI
	OD
	flush(ot);
	pop();
}

LOCAL VOID	flush(ot)
	INT		ot;
{
	write(ot,stakbot,(SIZE) (staktop-stakbot));
	IF flags&execpr THEN write(output,stakbot,(SIZE) (staktop-stakbot)) FI
	staktop=stakbot;
}
