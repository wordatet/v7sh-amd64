#
/*
 * v7sh-amd64 (UNIX shell)
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include	"defs.h"

IOPTR		iotemp;
IOPTR		iopend;
INT		stripflg;
/* set by trim */
BOOL		nosubst;


/* ========	input output and file copying ======== */

VOID	initf(fd)
	UFD		fd;
{
	REG FILEPTR	f=standin;

#if defined(SYSIII)
	f->fdes=fd; f->fsiz=((flags&oneflg)==0 ? BUFSIZ : 1);
#else /* V7 */
	f->fdes=fd; f->fsiz=((flags&(oneflg|ttyflg))==0 ? BUFSIZ : 1);
#endif
	f->fnxt=f->fend=f->fbuf; f->feval=0; f->flin=1;
	f->feof=FALSE;
}

INT	estabf(s)
	REG STRING	s;
{
	REG FILEPTR	f;

	(f=standin)->fdes = -1;
	f->fend=length(s)+(f->fnxt=s);
	f->flin=1;
	return(f->feof=(s==0));
}

VOID	push(af)
	FILEPTR		af;
{
	REG FILEPTR	f;

	(f=af)->fstak=standin;
	f->feof=0; f->feval=0;
	standin=f;
}

INT	pop()
{
	REG FILEPTR	f;

	IF (f=standin)->fstak
	THEN	IF f->fdes>=0 THEN close(f->fdes) FI
		standin=f->fstak;
		return(TRUE);
	ELSE	return(FALSE);
	FI
}

VOID	chkpipe(pv)
	INT		*pv;
{
	IF pipe(pv)<0 ORF pv[INPIPE]<0 ORF pv[OTPIPE]<0
	THEN	error(piperr);
		/*NOTREACHED*/
	FI
}

INT	chkopen(idf)
	CSTRING		idf;
{
	REG INT		rc;

	IF (rc=open(idf,O_RDONLY))<0
	THEN	failed(idf,badopen);
		/*NOTREACHED*/
	FI
	return(rc);
}

VOID	rename(f1,f2)
	REG INT		f1, f2;
{
	IF f1!=f2
	THEN	dup2(f1, f2);
		close(f1);
		IF f2==0 THEN ioset|=1 FI
	FI
}

INT	create(s)
	STRING		s;
{
	REG INT		rc;

	IF (rc=creat(s,0666))<0
	THEN	failed(s,badcreate);
		/*NOTREACHED*/
	FI
	return(rc);
}

INT	tmpfil()
{
	itos(serial++); movstr(numbuf,tmpnam);
	return(create(tmpout));
}


VOID	copy(ioparg)
	IOPTR		ioparg;
{
	INT		c;
	STRING		ends;
	REG STRING	cline, clinep;
	INT		fd;
	REG IOPTR	iop;

	IF (iop=ioparg)!=NIL			/* GCC */
	THEN	copy(iop->iolst);
		ends=mactrim(iop->ioname); IF nosubst THEN iop->iofile &= ~IODOC FI
		fd=tmpfil();
		iop->ioname=cpystak(tmpout);
		iop->iolst=iotemp; iotemp=iop;
		cline=locstak();

#if defined(SYSIII)
		IF stripflg
		THEN	WHILE *ends=='\t' DO ends++ OD
		FI
#endif
		LOOP	clinep=cline; chkpr(NL);
#if defined(SYSIII)
			IF stripflg
			THEN
				WHILE (c=(nosubst ? readc() : nextc(*ends)), !eolchar(c))
				&& cline == clinep && c == '\t' DONE
				WHILE (!eolchar(c))
				DO
					*clinep++=c;
					c=(nosubst ? readc() : nextc(*ends));
				OD
			ELSE
#endif
			WHILE (c = (nosubst ? readc() :  nextc(*ends)),  !eolchar(c)) DO *clinep++ = c OD
#if defined(SYSIII)
			FI
#endif
			*clinep=0;
			IF eof ORF eq(cline,ends) THEN break FI
			*clinep++=NL;
			write(fd,cline,(SIZE) (clinep-cline));
		POOL
#if defined(SYSIII)
		IF stripflg THEN stripflg-- FI
#endif
		close(fd);
	FI
}
