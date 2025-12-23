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

LOCAL TREPTR	makelist(INT,TREPTR,TREPTR);
LOCAL TREPTR	list(INT);
LOCAL TREPTR	term(INT);
LOCAL REGPTR	syncase(INT);
LOCAL TREPTR	item(BOOL);
LOCAL INT	skipnl(VOID);
LOCAL IOPTR	inout(IOPTR);
LOCAL VOID	chkword(VOID);
LOCAL VOID	chksym(INT);
LOCAL VOID	prsym(INT);
LOCAL VOID	synbad(VOID);


/* ========	command line decoding	========*/




TREPTR	makefork(flgs, i)
	INT		flgs;
	TREPTR		i;
{
	REG TREPTR	t;

	t=(TREPTR) getstak(FORKTYPE);
	t->forknod.forktyp=flgs|TFORK;
	t->forknod.forktre=i;
	t->forknod.forkio=0;
	return(t);
}

LOCAL TREPTR	makelist(type,i,r)
	INT		type;
	TREPTR		i, r;
{
	REG TREPTR	t=NIL;			/* GCC */

	IF i==0 ORF r==0
	THEN	synbad();
	ELSE	t=(TREPTR) getstak(LSTTYPE);
		t->lstnod.lsttyp = type;
		t->lstnod.lstlef = i;
		t->lstnod.lstrit = r;
	FI
	return(t);
}

/*
 * cmd
 *	empty
 *	list
 *	list & [ cmd ]
 *	list [ ; cmd ]
 */

TREPTR	cmd(sym,flg)
	REG INT		sym;
	INT		flg;
{
	REG TREPTR	i, e;

	i = list(flg);

	IF wdval==NL
	THEN	IF flg&NLFLG
		THEN	wdval=';'; chkpr(NL);
		FI
	ELIF i==0 ANDF (flg&MTFLG)==0
	THEN	synbad();
	FI

	SWITCH wdval IN

	    case '&':
		IF i
		THEN	i = makefork(FINT|FPRS|FAMP, i);
		ELSE	synbad();
		FI
		/*FALLTHROUGH*/

	    case ';':
		IF (e=cmd(sym,flg|MTFLG))!=NIL	/* GCC */
		THEN	i=makelist(TLST, i, e);
#if defined(SYSIII)
		ELIF	i == 0
		THEN	synbad();
#endif
		FI
		break;

	    case EOFSYM:
		IF sym==NL
		THEN	break;
		FI
		/*FALLTHROUGH*/

	    default:
		IF sym
		THEN	chksym(sym);
		FI

	ENDSW
	return(i);
}

/*
 * list
 *	term
 *	list && term
 *	list || term
 */

LOCAL TREPTR	list(flg)
	INT		flg;
{
	REG TREPTR	r;
	REG INT		b;

	r = term(flg);
	WHILE r ANDF ((b=(wdval==ANDFSYM)) ORF wdval==ORFSYM)
	DO	r = makelist((b ? TAND : TORF), r, term(NLFLG));
	OD
	return(r);
}

/*
 * term
 *	item
 *	item |^ term
 */

LOCAL TREPTR	term(flg)
	INT		flg;
{
	REG TREPTR	t;

	reserv++;
	IF flg&NLFLG
	THEN	skipnl();
	ELSE	word();
	FI

	IF (t=item(TRUE)) ANDF (wdval=='^' ORF wdval=='|')
	THEN	return(makelist(TFIL, makefork(FPOU,t), makefork(FPIN|FPCL,term(NLFLG))));
	ELSE	return(t);
	FI
}

LOCAL REGPTR	syncase(esym)
	REG INT	esym;
{
	skipnl();
	IF wdval==esym
	THEN	return(0);
	ELSE	REG REGPTR	r=(REGPTR) getstak(REGTYPE);
		r->regptr=0;
		LOOP wdarg->argnxt=r->regptr;
		     r->regptr=wdarg;
		     IF wdval ORF ( word()!=')' ANDF wdval!='|' )
		     THEN synbad();
		     FI
		     IF wdval=='|'
		     THEN word();
		     ELSE break;
		     FI
		POOL
		r->regcom=cmd(0,NLFLG|MTFLG);
		IF wdval==ECSYM
		THEN	r->regnxt=syncase(esym);
		ELSE	chksym(esym);
			r->regnxt=0;
		FI
		return(r);
	FI
}

/*
 * item
 *
 *	( cmd ) [ < in  ] [ > out ]
 *	word word* [ < in ] [ > out ]
 *	if ... then ... else ... fi
 *	for ... while ... do ... done
 *	case ... in ... esac
 *	begin ... end
 */

LOCAL TREPTR	item(flag)
	BOOL		flag;
{
	REG TREPTR	t;
	REG IOPTR	io;

	IF flag
	THEN	io=inout((IOPTR)0);
	ELSE	io=0;
	FI

	SWITCH wdval IN

	    case CASYM:
		BEGIN
		   t=(TREPTR) getstak(SWTYPE);
		   chkword();
		   t->swnod.swarg=wdarg->argval;
		   skipnl(); chksym(INSYM|BRSYM);
		   t->swnod.swlst=syncase(wdval==INSYM?ESSYM:KTSYM);
		   t->swnod.swtyp=TSW;
		   break;
		END

	    case IFSYM:
		BEGIN
		   REG INT	w;
		   t=(TREPTR) getstak(IFTYPE);
		   t->ifnod.iftyp=TIF;
		   t->ifnod.iftre=cmd(THSYM,NLFLG);
		   t->ifnod.thtre=cmd(ELSYM|FISYM|EFSYM,NLFLG);
		   t->ifnod.eltre=((w=wdval)==ELSYM ? cmd(FISYM,NLFLG) : (w==EFSYM ? (wdval=IFSYM, item(0)) : 0));
		   IF w==EFSYM THEN return(t) FI
		   break;
		END

	    case FORSYM:
		BEGIN
		   t=(TREPTR) getstak(FORTYPE);
		   t->fornod.fortyp=TFOR;
		   t->fornod.forlst=0;
		   chkword();
		   t->fornod.fornam=wdarg->argval;
		   IF skipnl()==INSYM
		   THEN	chkword();
			t->fornod.forlst=(COMPTR) item(0);
			IF wdval!=NL ANDF wdval!=';'
			THEN	synbad();
			FI
			chkpr(wdval); skipnl();
		   FI
		   chksym(DOSYM|BRSYM);
		   t->fornod.fortre=cmd(wdval==DOSYM?ODSYM:KTSYM,NLFLG);
		   break;
		END

	    case WHSYM:
	    case UNSYM:
		BEGIN
		   t=(TREPTR) getstak(WHTYPE);
		   t->whnod.whtyp=(wdval==WHSYM ? TWH : TUN);
		   t->whnod.whtre = cmd(DOSYM,NLFLG);
		   t->whnod.dotre = cmd(ODSYM,NLFLG);
		   break;
		END

	    case BRSYM:
		t=cmd(KTSYM,NLFLG);
		break;

	    case '(':
		BEGIN
		   REG PARPTR	 p;
		   p=(PARPTR) getstak(PARTYPE);
		   p->partre=cmd(')',NLFLG);
		   p->partyp=TPAR;
		   t=makefork(0,(TREPTR) p);
		   break;
		END

	    default:
		IF io==0
		THEN	return(0);
		FI
		/*FALLTHROUGH*/

	    case 0:
		BEGIN
		   REG ARGPTR	argp;
		   REG ARGPTR	*argtail;
		   REG ARGPTR	*argset=0;
		   INT		keywd=1;
		   t=(TREPTR) getstak(COMTYPE);
		   t->comnod.comio=io; /*initial io chain*/
		   argtail = &(t->comnod.comarg);
		   WHILE wdval==0
		   DO	argp = wdarg;
			IF wdset ANDF keywd
			THEN	argp->argnxt=(ARGPTR) argset; argset=(ARGPTR *) argp;
			ELSE	*argtail=argp; argtail = &(argp->argnxt); keywd=flags&keyflg;
			FI
			word();
			IF flag
			THEN t->comnod.comio=inout(t->comnod.comio);
			FI
		   OD

		   t->comnod.comtyp=TCOM;
		   t->comnod.comset=(ARGPTR) argset;
		   *argtail=0;
		   return(t);
		END

	ENDSW
	reserv++; word();
	IF (io=inout(io))!=NIL
	THEN	t=makefork(0,t); t->treio.treio=io;
	FI
	return(t);
}


LOCAL INT	skipnl()
{
	WHILE (reserv++, word()==NL) DO chkpr(NL) OD
	return(wdval);
}

LOCAL IOPTR	inout(lastio)
	IOPTR		lastio;
{
	REG INT		iof;
	REG IOPTR	iop;
	REG INT		c;

	iof=wdnum;

	SWITCH wdval IN

	    case DOCSYM:
		/*	<<	*/
		iof |= IODOC; break;
		/*FALLTHROUGH*/

	    case APPSYM:
	        /*	>>	*/
	    case '>':
		IF wdnum==0 THEN iof |= 1 FI
		iof |= IOPUT;
		IF wdval==APPSYM
		THEN	iof |= IOAPP; break;
		FI
		/*FALLTHROUGH*/

	    case '<':
		IF (c=nextc(0))=='&'
		THEN	iof |= IOMOV;
		ELIF c=='>'
		/*	<> is open for read and write	*/
		/*	unadvertised feature		*/
		THEN	iof |= IORDW;
		ELSE	peekc=c|MARK;
		FI
		break;

	    default:
		return(lastio);
	ENDSW

	chkword();
	iop=(IOPTR) getstak(IOTYPE); iop->ioname=wdarg->argval; iop->iofile=iof;
	IF iof&IODOC
	THEN iop->iolst=iopend; iopend=iop;
	FI
	word(); iop->ionxt=inout(lastio);
	return(iop);
}

LOCAL VOID	chkword()
{
	IF word()
	THEN	synbad();
	FI
}

LOCAL VOID	chksym(sym)
	INT		sym;
{
	REG INT		x = sym&wdval;
	IF ((x&SYMFLG) ? x : sym) != wdval
	THEN	synbad();
	FI
}

LOCAL VOID	prsym(sym)
	INT		sym;
{
	IF sym&SYMFLG
	THEN	REG SYSPTR	sp=reserved;
		WHILE sp->sysval
			ANDF sp->sysval!=sym
		DO sp++ OD
		prs(sp->sysnam);
	ELIF sym==EOFSYM
	THEN	prs(endoffile);
	ELSE	IF sym&SYMREP THEN prc(sym) FI
		IF sym==NL
		THEN	prs(endofline);
		ELSE	prc(sym);
		FI
	FI
}

LOCAL VOID	synbad()
{
	prp(); prs(synmsg);
	IF (flags&ttyflg)==0
	THEN	prs(atline); prn((INT) standin->flin);
	FI
	prs(colon);
	prc(LQ);
	IF wdval
	THEN	prsym(wdval);
	ELSE	prs(wdarg->argval);
	FI
	prc(RQ); prs(unexpected);
	newline();
	exitsh(SYNBAD);
	/*NOTREACHED*/
}
