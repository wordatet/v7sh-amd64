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

LOCAL INT	readb(VOID);

INT		wdval;
INT		wdnum;
ARGPTR		wdarg;
INT		wdset;
BOOL		reserv;
INT		peekc;
BOOL		rwait;


/* ========	character handling for command lines	========*/


INT	word()
{
	REG INT		c, d;
	REG STRING	argp=(STRING) (locstak()+BYTESPERWORD);
	REG ARGPTR	ap;
	INT		alpha=1;

	wdnum=0; wdset=0;

#if defined(SYSIII)
	LOOP
#endif
	WHILE (c=nextc(0), space(c)) DONE
#if defined(RENO)
		IF c==COMCHAR ANDF ((flags&prompt)==0 ORF ((flags&ttyflg) ANDF
		   standin->fstak!=0))
		THEN	WHILE (c=readc())!=EOF ANDF c!=NL DONE
#if defined(SYSIII)
			peekc=c;
		ELSE	break;	/* out of comment - white space loop */
#endif
		FI
#endif
#if defined(SYSIII)
		IF c==COMCHAR
		THEN	WHILE (c=readc())!=EOF ANDF c!=NL DONE
			peekc=c;
		ELSE	break;	/* out of comment - white space loop */
		FI
	POOL
#endif
	IF !eofmeta(c)
	THEN	REP	IF c==LITERAL
			THEN	*argp++=(DQUOTE);
				WHILE (c=readc()) ANDF c!=LITERAL
				DO *argp++=(c|QUOTE); chkpr(c) OD
				*argp++=(DQUOTE);
			ELSE	*argp++=(c);
				IF c=='=' THEN wdset |= alpha FI
				IF !alphanum(c) THEN alpha=0 FI
				IF qotchar(c)
				THEN	d=c;
					WHILE (*argp++=(c=nextc(d))) ANDF c!=d
					DO chkpr(c) OD
				FI
			FI
		PER (c=nextc(0), !eofmeta(c)) DONE
		ap=(ARGPTR) endstak(argp);
		IF !letter((INT) ap->argval[0]) THEN wdset=0 FI

		peekc=c|MARK;
		IF ap->argval[1]==0 ANDF (d=ap->argval[0], digit(d)) ANDF (c=='>' ORF c=='<')
		THEN	word(); wdnum=d-'0';
		ELSE	/*check for reserved words*/
			IF reserv==FALSE ORF (wdval=syslook(ap->argval,reserved))==0
			THEN	wdarg=ap; wdval=0;
			FI
		FI

	ELIF dipchar(c)
	THEN	IF (d=nextc(0))==c
		THEN	wdval = c|SYMREP;
#if defined(SYSIII)
			IF c=='<'
			THEN	IF (d=nextc(0))=='-'
				THEN	stripflg++;
				ELSE	peekc = d|MARK;
				FI
			FI
#endif
		ELSE	peekc = d|MARK; wdval = c;
		FI
	ELSE	IF (wdval=c)==EOF
		THEN	wdval=EOFSYM;
		FI
		IF iopend ANDF eolchar(c)
		THEN	copy(iopend); iopend=0;
		FI
	FI
	reserv=FALSE;
	return(wdval);
}

INT	nextc(quote)
	INT		quote;
{
	REG INT		c, d;
	IF (d=readc())==ESCAPE
	THEN	IF (c=readc())==NL
		THEN	chkpr(NL); d=nextc(quote);
		ELIF quote ANDF c!=quote ANDF !escchar(c)
		THEN	peekc=c|MARK;
		ELSE	d = c|QUOTE;
		FI
	FI
	return(d);
}

INT	readc()
{
	REG INT		c;
	REG INT		len;
	REG FILEPTR	f;

retry:
	IF peekc
	THEN	c=peekc&STRIP; peekc=0;
	ELIF (f=standin, f->fnxt!=f->fend)
	THEN	IF (c = *f->fnxt++)==0
		THEN	IF f->feval
			THEN	IF estabf(*f->feval++)
				THEN	c=EOF;
				ELSE	c=SP;
				FI
			ELSE	goto retry; /* = c=readc(); */
			FI
		FI
		IF flags&readpr ANDF standin->fstak==0 THEN prc(c) FI
		IF c==NL THEN f->flin++ FI
	ELIF f->feof ORF f->fdes<0
	THEN	c=EOF; f->feof++;
	ELIF (len=readb())<=0
	THEN	close(f->fdes); f->fdes = -1; c=EOF; f->feof++;
	ELSE	f->fend = (f->fnxt = f->fbuf)+len;
		goto retry;
	FI
	return(c);
}

LOCAL INT	readb()
{
	REG FILEPTR	f=standin;
	REG INT		len;

#if defined(RENO)
	IF setjmp(INTbuf) == 0 THEN trapjmp[INTR] = 1; FI
#endif
#if defined(SYSIII)
	REP	IF trapnote&SIGSET
		THEN	newline(); sigchk();
		ELIF (trapnote&TRAPSET) ANDF (rwait>0)
		THEN	newline(); chktrap(); clearup();
		FI
#else /* V7 */
	REP	IF trapnote&SIGSET THEN newline(); sigchk() FI
#endif
	PER (len=read(f->fdes,f->fbuf,(SIZE) f->fsiz))<0 ANDF trapnote DONE
#if defined(RENO)
	trapjmp[INTR] = 0;
#endif
	return(len);
}
