#
/*
 *	v7sh-amd64 (UNIX shell)
 *
 *	S. R. Bourne
 *	Bell Telephone Laboratories
 *
 */

#include	"defs.h"



/* globals (file name generation)
 *
 * "*" in params matches r.e ".*"
 * "?" in params matches r.e. "."
 * "[...]" in params matches character class
 * "[...a-z...]" in params matches a through z.
 *
 */

LOCAL VOID	addg(STRING, STRING, STRING);
#if defined(SYSIII)
LOCAL VOID	chgquot(STRING, INT);
#endif

ARGPTR		gchain;


INT	expand(as,rflg)
	STRING		as;
	INT		rflg;
{
	INT		count;
	DIR		*dirf;
	BOOL		dir=0;
	STRING		rescan = 0;
	REG STRING	s, cs;
	ARGPTR		schain = gchain;
	DIRPTR		dp;
	STATBUF		statb;
	CHAR		buf[2];

	IF trapnote&SIGSET THEN return(0); FI

	s=cs=as;

	/* check for meta chars */
	BEGIN
#if defined(SYSIII)
#define open open_
	   REG BOOL slash, open; slash=0;open=0;
	   LOOP
		SWITCH *cs++ IN
		case 0:		IF rflg ANDF slash THEN break;
				ELSE return(0);
				FI
		case '/':	slash++;
				open = 0;
				continue;
		case '[':	open++;
				continue;
		case ']':	IF open THEN break FI
				continue;
		case '?':
		case '*':	cs--;
				break;
		default:	continue;
		ENDSW
		break;
	   POOL
#undef open
#else /* V7 */
	   REG BOOL slash; slash=0;
	   WHILE !fngchar((INT) *cs)
	   DO	IF *cs++==0
		THEN	IF rflg ANDF slash THEN break; ELSE return(0) FI
		ELIF *cs=='/'
		THEN	slash++;
		FI
	   OD
#endif
	END

	LOOP	IF cs==s
		THEN	movstr(nullstr,s=buf);
			break;
		ELIF *--cs == '/'
		THEN	*cs=0;
			IF s==cs THEN movstr("/",s=buf) FI
			break;
		FI
	POOL
#if defined(SYSIII)
	chgquot(s, 0);
	IF stat(*s?s:".",&statb)>=0
#else /* V7 */
	IF stat(s,&statb)>=0
#endif
	    ANDF (statb.st_mode&S_IFMT)==S_IFDIR
#if defined(SYSIII)
	    ANDF (dirf=opendir(*s?s:".")) != NULL
#else /* V7 */
	    ANDF (dirf=opendir(s)) != NULL
#endif
	THEN	dir++;
	ELSE	dirf=NULL;
	FI
#if defined(SYSIII)
	chgquot(s, 1);
#endif
	count=0;
	IF *cs==0 THEN *cs++=QUOTE FI
	IF dir
	THEN	/* check for rescan */
		REG STRING rs; rs=cs;

		REP	IF *rs=='/' THEN rescan=rs; *rs=0; gchain=0 FI
		PER	*rs++ DONE

#if defined(RENO)
		IF setjmp(INTbuf) == 0 THEN trapjmp[INTR] = 1; FI
		WHILE (trapnote&SIGSET) == 0 ANDF (dp = readdir(dirf)) != NULL
#else /* V7 */
		WHILE (dp = readdir(dirf)) != NULL ANDF (trapnote&SIGSET) == 0
#endif
		DO	IF (*dp->d_name=='.' ANDF *cs!='.')
			THEN	continue;
			FI
#if defined(SYSIII)
/*
 *	Here lies the fix for the "echo * ^H/." problem when
 *	there are files with metacharacters in there names.
 */
			chgquot(dp->d_name, 1);
#endif
			IF gmatch(dp->d_name, cs)
			THEN	addg(s,dp->d_name,rescan); count++;
			FI
#if defined(SYSIII)
			chgquot(dp->d_name, 0);
#endif
		OD
		closedir(dirf);
#if defined(RENO)
		trapjmp[INTR] = 0;
#endif

		IF rescan
		THEN	REG ARGPTR	rchain;
			rchain=gchain; gchain=schain;
			IF count
			THEN	count=0;
				WHILE rchain
				DO	count += expand(rchain->argval,1);
					rchain=rchain->argnxt;
				OD
			FI
			*rescan='/';
		FI
	FI

	BEGIN
	   REG INT	c;
	   s=as;
	   WHILE (c = *s)!=0			/* GCC */
	   DO	*s++=(c&STRIP?c:'/') OD
	END
	return(count);
}

INT	gmatch(s, p)
	REG STRING	s, p;
{
	REG INT		scc;
	INT		c;

	IF (scc = *s++)!=0			/* GCC */
	THEN	IF (scc &= STRIP)==0
		THEN	scc=QUOTE;
		FI
	FI
	SWITCH c = *p++ IN

	    case '[':
		BEGIN
		BOOL ok; INT lc;
#if defined(SYSIII)
		INT notflag=0;
#endif
		ok=0; lc=077777;
#if defined(SYSIII)
		IF *p == '!' ORF *p == '^' THEN notflag=1; p++; FI
#endif
		WHILE (c = *p++)!=0		/* GCC */
		DO	IF c==']'
			THEN	return(ok?gmatch(s,p):0);
			ELIF c==MINUS
#if defined(SYSIII)
			THEN	IF notflag
				THEN	IF lc>scc ORF scc>*(p++)
					THEN ok++;
					ELSE return(0)
					FI
				ELSE IF lc<=scc ANDF scc<=(*p++) THEN ok++ FI
				FI
			ELSE	IF notflag
				THEN	IF scc!=(lc=(c&STRIP))
					THEN ok++;
					ELSE return(0)
					FI
				ELSE	IF scc==(lc=(c&STRIP)) THEN ok++ FI
				FI
#else /* V7 */
			THEN	IF lc<=scc ANDF scc<=(*p++) THEN ok++ FI
			ELSE	IF scc==(lc=(c&STRIP)) THEN ok++ FI
#endif
			FI
		OD
		return(0);
		END

	    default:
		IF (c&STRIP)!=scc THEN return(0) FI
		/*FALLTHROUGH*/

	    case '?':
		return(scc?gmatch(s,p):0);

	    case '*':
		IF *p==0 THEN return(1) FI
		--s;
		WHILE *s
		DO  IF gmatch(s++,p) THEN return(1) FI OD
		return(0);

	    case 0:
		return(scc==0);
	ENDSW
}

LOCAL VOID	addg(as1,as2,as3)
	STRING		as1, as2, as3;
{
	REG STRING	s1, s2;
	REG INT		c;

	s2 = locstak()+BYTESPERWORD;

	s1=as1;
	WHILE (c = *s1++)!=0			/* GCC */
	DO	IF (c &= STRIP)==0
		THEN	*s2++='/';
			break;
		FI
		*s2++=c;
	OD
	s1=as2;
	WHILE (*s2 = *s1++)!=0 DO s2++ OD	/* GCC */
	IF (s1=as3)!=NIL			/* GCC */
	THEN	*s2++='/';
		WHILE (*s2++ = *++s1)!=0 DONE	/* GCC */
	FI
	makearg((ARGPTR) endstak(s2));
}

VOID	makearg(args)
	REG ARGPTR	args;
{
	args->argnxt=gchain;
	gchain=(ARGPTR) args;
}

#if defined(SYSIII)
LOCAL VOID	chgquot(str, flg)
	REG STRING	str;
	REG INT		flg;
{
	REG INT i;
 
	FOR i=0;str[i];i++
	DO
		SWITCH str[i] IN
		case '*':
		case '?':
		case '[':
		case '*'|QUOTE:
		case '?'|QUOTE:
		case '['|QUOTE:
			IF flg==0
			THEN	str[i] &= (~QUOTE);
			ELSE	str[i] |= QUOTE;
			FI
		ENDSW
	OD
}
#endif
