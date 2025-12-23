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

LOCAL INT	parent;

INT		ioset;
STRING		pcsadr;
INT		loopcnt;
INT		breakcnt;
BOOL		execbrk;


/* ========	command execution	========*/


INT	execute(argt, execflg, pf1, pf2)
	TREPTR		argt;
	INT		execflg;
	INT		*pf1, *pf2;
{
	/* `stakbot' is preserved by this routine */
	REG TREPTR	t;
	STKPTR		sav=savstak();
#if defined(SYSIII)
	INT		bltflg=0;
#endif

	sigchk();

	IF (t=argt) ANDF execbrk==0
	THEN	REG INT		treeflgs;
		INT		oldexit, type;
		REG STRING	*com=NIL;	/* GCC */

		treeflgs = t->treio.tretyp; type = treeflgs&COMMSK;
		oldexit=exitval; exitval=0;

		SWITCH type IN

		case TCOM:
			BEGIN
			STRING		a1;
			INT		argn, internal=0; /* GCC */
			ARGPTR		schain=gchain;
			IOPTR		io=t->treio.treio;
			gchain=0;
			argn = getarg((COMPTR) t);
			com=scan(argn);
			a1=com[1]; gchain=schain;

#ifdef RENO
			IF argn==0 ORF (internal=syslook(com[0],commands))
#else /* V7 */
			/* Fixed: check argn first to avoid NULL dereference */
			IF argn==0 ORF (internal=syslook(com[0],commands))
#endif
			THEN	setlist(t->comnod.comset, 0);
			FI

			IF argn ANDF (flags&noexec)==0
			THEN	/* print command if execpr */
				IF flags&execpr
				THEN	argn=0;	prs(execpmsg);
					WHILE com[argn]!=ENDARGS
					DO prs(com[argn++]); blank() OD
					newline();
				FI

				SWITCH internal IN

				case SYSDOT:
					IF a1
					THEN	REG INT		f;
	
						IF (f=pathopen(getpath(a1), a1)) < 0
						THEN failed(a1,notfound);
							/*NOTREACHED*/
						ELSE execexp(0,f);
						FI
					FI
					break;
	
				case SYSTIMES:
					BEGIN
					TIMEBUF	timeb; times(&timeb);
					prt(((CLOCK *)&timeb)[2]); blank();
					prt(((CLOCK *)&timeb)[3]); newline();
					END
					break;
	
				case SYSEXIT:
					flags |= forked;	/* force exit even if interactive */
					exitsh(a1?stoi(a1):oldexit);
					/*NOTREACHED*/
	
				case SYSNULL:
					io=0;
					break;
	
				case SYSCONT:
#if defined(ULTRIX)
					IF loopcnt
					THEN	execbrk = breakcnt = 1;
						IF a1
						THEN breakcnt = stoi(a1) FI
						breakcnt = breakcnt > loopcnt ?
						loopcnt : -breakcnt;
					FI
					break;
#elif defined(SYSIII)
					IF (execbrk=-loopcnt)!=0 ANDF a1
					THEN breakcnt=stoi(a1);
					FI
					break;
#else /* V7 */
					execbrk = -loopcnt; break;
#endif
	
				case SYSBREAK:
#if defined(ULTRIX)
					IF loopcnt
					THEN	execbrk = breakcnt = 1;
						IF a1
						THEN breakcnt = stoi(a1) FI
						IF breakcnt > loopcnt
						THEN breakcnt = loopcnt FI
					FI
#else /* V7 */
					IF (execbrk=loopcnt)!=0 ANDF a1 /*GCC*/
					THEN breakcnt=stoi(a1);
					FI
#endif
					break;
	
				case SYSTRAP:
					IF a1
					THEN	BOOL	clear;
						IF (clear=digit((INT) *a1))==0
						THEN	++com;
						FI
						WHILE *++com
						DO INT	i;
						   IF (i=stoi(*com))>=MAXTRAP ORF i<MINTRAP
						   THEN	failed(*com,badtrap);
							/*NOTREACHED*/
						   ELIF clear
						   THEN	clrsig(i);
						   ELSE	replace(&trapcom[i],a1);
							IF *a1
							THEN	getsig(i);
							ELSE	ignsig(i);
							FI
						   FI
						OD
					ELSE	/* print out current traps */
						INT		i;
	
						FOR i=0; i<MAXTRAP; i++
						DO IF trapcom[i]
						   THEN	prn(i); prs(colon); prs(trapcom[i]); newline();
						   FI
						OD
					FI
					break;
	
				case SYSEXEC:
					com++;
					initio(io); ioset=0; io=0;
					IF a1==0 THEN break FI
					/*FALLTHROUGH*/
	
				case SYSLOGIN:
#if !defined(SYSIII)
					flags |= forked;
#endif
					oldsigs(); execa(com); done();
					/*NOTREACHED*/
#if defined(SYSIII)
				case SYSNEWGRP:
					IF flags&rshflg
					THEN	failed(com[0],restricted);
						/*NOTREACHED*/
					ELSE	/* force bad exec to terminate shell */
						flags |= forked;
						oldsigs(); execa(com); done();
						/*NOTREACHED*/
					FI
#endif
				case SYSCD:
					IF flags&rshflg
					THEN	failed(com[0],restricted);
						/*NOTREACHED*/
#if defined(SYSIII)
					ELIF	(argn >2)
					THEN	failed(com[0],argcount);
						/*NOTREACHED*/
#endif
					ELIF (a1==0 ANDF (a1=homenod.namval)==0) ORF chdir(a1)<0
					THEN	failed(a1,baddir);
						/*NOTREACHED*/
					FI
					break;
	
				case SYSSHFT:
					IF dolc<1
					THEN	error(badshift);
						/*NOTREACHED*/
					ELSE	dolv++; dolc--;
					FI
					assnum(&dolladr, dolc);
					break;
	
				case SYSWAIT:
#if defined(SYSIII)
					await(a1?stoi(a1):-1,1);
#else /* V7 */
					await(-1);
#endif
					break;
	
				case SYSREAD:
#if defined(SYSIII)
					rwait=1;
#endif
					exitval=readvar(&com[1]);
#if defined(SYSIII)
					rwait=0;
#endif
					break;

				case SYSSET:
					IF a1
					THEN	INT	argc;
						argc = options(argn,com);
						IF argc>1
						THEN	setargs(com+argn-argc);
						FI
					ELIF ((COMPTR) t)->comset==0
					THEN	/*scan name chain and print*/
						namscan(printnam);
					FI
					break;
	
				case SYSRDONLY:
					exitval=N_RDONLY;
					/*FALLTHROUGH*/

				case SYSXPORT:
					IF exitval==0 THEN exitval=N_EXPORT; FI
	
					IF a1
					THEN	WHILE *++com
						DO attrib(lookup(*com), exitval) OD
					ELSE	namscan(printflg);
					FI
					exitval=0;
					break;
	
				case SYSEVAL:
					IF a1
					THEN	execexp(a1,(UFD) &com[2]);
					FI
					break;
#if defined(ULTRIX)
                                case SYSULIMIT:
					BEGIN
					LONG i = 0;
					INT command = 2;

					IF **++com == '-'
					THEN	IF *(*com+1) != 'f'
						THEN error(badopt) FI
						com++;
					FI
					IF *com
					THEN	WHILE **com >= '0' && **com <= '9'
						DO	i *= 10;
							i += **com - '0';
							IF i < 0
							THEN error(badulimit) FI
							(*com)++;
						OD
						IF **com
						THEN error(badulimit) FI
					ELSE	command--;
					FI
					i = ulimit(command,i);
					IF i < 0 THEN error(badulimit) FI
					IF command == i
					THEN prl(i); newline(); FI
					END
					break;
#endif
                                case SYSUMASK:
					IF a1
					THEN	INT c, i;
                                                i = 0;
						WHILE ((c = *a1++) >= '0' ANDF
                                                        c <= '7')
						DO	i = (i << 3) + c - '0';
						OD
                                                umask(i);
                                        ELSE	MODE i; INT j;
                                                umask(i = umask(0));
                                                prc('0');
						FOR j = 6; j >= 0; j -= 3
						DO	prc(((i>>j)&07) + '0');
						OD
                                                newline();
                                        FI
                                        break;
	
				default:
#if defined(SYSIII)
					IF (internal=syslook(com[0],builtins))>0
					THEN	builtin(internal, argn, com);
						bltflg=1;
					FI
#else /* V7 */
					internal=builtin(argn,com);
#endif
				ENDSW

				IF internal
#if defined(SYSIII)
				THEN	IF io ANDF !bltflg
					THEN	error(illegal);
						/*NOTREACHED*/
					FI
#else /* V7 */
				THEN	IF io THEN error(illegal) FI
#endif
					chktrap();
					break;
				FI
			ELIF t->treio.treio==0
#if defined(SYSIII)
			THEN	chktrap();
				break;
#else /* V7 */
			THEN	break;
#endif
			FI
			END
			/*FALLTHROUGH*/
	
		case TFORK:
			IF execflg ANDF (treeflgs&(FAMP|FPOU))==0
			THEN	parent=0;
#if defined(SYSIII)
			ELSE	UINT forkcnt=1;
			/* FORKLIM is the max period between forks -
			   power of 2 usually.  Currently shell tries
			   after 2,4,8,16, and 32 seconds and then quits */
				WHILE (parent=fork()) == -1
				DO	IF (forkcnt=(forkcnt*2)) > FORKLIM /* 32 */
					THEN	SWITCH errno IN
						case ENOMEM:
							error(noswap);
							/*NOTREACHED*/
							break;
						default:
						/* case EAGAIN: */
							error(nofork);
							/*NOTREACHED*/
							break;
						ENDSW
					FI
					sigchk(); alarm(forkcnt); pause();
				OD
#else /* V7 */
			ELSE	WHILE (parent=fork()) == -1
				DO sigchk(); alarm(10); pause() OD
#endif
			FI

			IF parent
			THEN	/* This is the parent branch of fork;    */
				/* it may or may not wait for the child. */
				IF treeflgs&FPRS ANDF flags&ttyflg
				THEN	prn(parent); newline();
				FI
				IF treeflgs&FPCL THEN closepipe(pf1) FI
				IF (treeflgs&(FAMP|FPOU))==0
#if defined(SYSIII)
				THEN	await(parent,0);
#else /* V7 */
				THEN	await(parent);
#endif
				ELIF (treeflgs&FAMP)==0
				THEN	post(parent);
				ELSE	assnum(&pcsadr, parent);
				FI

				chktrap();
				break;


			ELSE	/* this is the forked branch (child) of execute */
				flags |= forked; iotemp=0;
				postclr();
				settmp();

				/* Turn off INTR and QUIT if `FINT'  */
				/* Reset ramaining signals to parent */
				/* except for those `lost' by trap   */
				oldsigs();
				IF treeflgs&FINT
				THEN	signal(INTR,SIG_IGN);
					signal(QUIT,SIG_IGN);
				FI

				/* pipe in or out */
				IF treeflgs&FPIN
				THEN	rename(pf1[INPIPE],0);
					close(pf1[OTPIPE]);
				FI
				IF treeflgs&FPOU
				THEN	rename(pf2[OTPIPE],1);
					close(pf2[INPIPE]);
				FI

				/* default std input for & */
				IF treeflgs&FINT ANDF ioset==0
				THEN	rename(chkopen(devnull),0);
				FI

				/* io redirection */
				initio(t->treio.treio);
				IF type!=TCOM
				THEN	execute(t->forknod.forktre,1,0,0);
				ELIF com[0]!=ENDARGS
				THEN	setlist(t->comnod.comset,N_EXPORT);
					execa(com);
				FI
				done();
				/*NOTREACHED*/
			FI

		case TPAR:
			rename(dup(2),output);
			execute(t->parnod.partre,execflg,0,0);
			done();
			/*NOTREACHED*/

		case TFIL:
			BEGIN
			   INT pv[2]; chkpipe(pv);
			   IF execute(t->lstnod.lstlef, 0, pf1, pv)==0
			   THEN	execute(t->lstnod.lstrit, execflg, pv, pf2);
			   ELSE	closepipe(pv);
			   FI
			END
			break;

		case TLST:
			execute(t->lstnod.lstlef,0,0,0);
			execute(t->lstnod.lstrit,execflg,0,0);
			break;

		case TAND:
			IF execute(t->lstnod.lstlef,0,0,0)==0
			THEN	execute(t->lstnod.lstrit,execflg,0,0);
			FI
			break;

		case TORF:
			IF execute(t->lstnod.lstlef,0,0,0)!=0
			THEN	execute(t->lstnod.lstrit,execflg,0,0);
			FI
			break;

		case TFOR:
			/* for.. do */
			BEGIN
			   NAMPTR	n = lookup(t->fornod.fornam);
			   STRING	*args;
			   DOLPTR	argsav=0;

			   IF t->fornod.forlst==0
			   THEN    args=dolv+1;
				   argsav=useargs();
			   ELSE	   ARGPTR	schain=gchain;
				   gchain=0;
				   trim((args=scan(getarg(t->fornod.forlst)))[0]);
				   gchain=schain;
			   FI
			   loopcnt++;
#if defined(SYSIII)
			   WHILE *args!=ENDARGS ANDF execbrk<=0
#else /* V7 */
			   WHILE *args!=ENDARGS ANDF execbrk==0
#endif
			   DO	assign(n,*args++);
				execute(t->fornod.fortre,0,0,0);
#if defined(ULTRIX)
				IF breakcnt < 0
				THEN execbrk = (++breakcnt != 0) FI
#elif defined(SYSIII)
				IF execbrk
				THEN	IF breakcnt > 1 ORF execbrk > 0
					THEN	break;
					ELSE	execbrk = breakcnt = 0;
					FI
				FI
#else /* V7 */
				IF execbrk<0 THEN execbrk=0 FI
#endif
			   OD
#if defined(ULTRIX)
			   IF breakcnt > 0
			   THEN execbrk = (--breakcnt != 0) FI
			   loopcnt--;
#else /* V7 */
			   IF breakcnt THEN breakcnt-- FI
#if defined(SYSIII)
			   execbrk = (execbrk < 0 ? -breakcnt : breakcnt);
			   loopcnt--;
#else /* V7 */
			   execbrk=breakcnt; loopcnt--;
#endif
#endif
			   argfor=(DOLPTR) freeargs(argsav);
			END
			break;

		case TWH:
			/* while.. do */
		case TUN:
			/* do.. until */
			BEGIN
			   INT		i=0;
#if defined(RENO)
			   INT		saveflg;

			   saveflg = flags&errflg;
#endif
			   loopcnt++;
#if defined(RENO)
#if defined(SYSIII)
			   WHILE execbrk<=0
#else /* RENO */
			   WHILE execbrk==0
#endif
			   DO	flags &= ~errflg;
				i=execute(t->whnod.whtre,0,0,0);
				flags |= saveflg;
				IF (i==0)!=(type==TWH) THEN break FI
				i=execute(t->whnod.dotre,0,0,0);
#else /* V7 */
			   WHILE execbrk==0 ANDF (execute(t->whnod.whtre,0,0,0)==0)==(type==TWH)
			   DO i=execute(t->whnod.dotre,0,0,0); /* GCC */
#endif
#if defined(ULTRIX)
				IF breakcnt < 0
				THEN execbrk = (++breakcnt != 0) FI
#elif defined(SYSIII)
				IF execbrk
				THEN	IF breakcnt > 1 ORF execbrk > 0
					THEN	break;
					ELSE	execbrk = breakcnt = 0;
					FI
				FI
#else /* V7 */
			      IF execbrk<0 THEN execbrk=0 FI
#endif
			   OD
#if defined(ULTRIX)
			   IF breakcnt > 0
			   THEN execbrk = (--breakcnt != 0) FI
			   loopcnt--; exitval= i;
#else /* V7 */
			   IF breakcnt THEN breakcnt-- FI
#if defined(SYSIII)
			   execbrk = (execbrk < 0 ? -breakcnt : breakcnt);
			   loopcnt--; exitval= i;
#else /* V7 */
			   execbrk=breakcnt; loopcnt--; exitval=i;
#endif
#endif
			END
			break;

		case TIF:
#if RENO
			BEGIN
				INT	i, saveflg;

				saveflg = flags&errflg;
				flags &= ~errflg;
				i=execute(t->ifnod.iftre,0,0,0);
				flags |= saveflg;
				IF i==0
#else /* V7 */
				IF execute(t->ifnod.iftre,0,0,0)==0
#endif
				THEN	execute(t->ifnod.thtre,execflg,0,0);
#if defined(SYSIII)
				ELIF t->ifnod.eltre
				THEN	execute(t->ifnod.eltre,execflg,0,0);
				ELSE	exitval=0; /* force zero exit for if-then-fi */
#else /* V7 */
				ELSE	execute(t->ifnod.eltre,execflg,0,0);
#endif
				FI
#if RENO
			END
#endif
			break;

		case TSW:
			BEGIN
			   REG STRING	r = mactrim(t->swnod.swarg);
			   REG REGPTR	eg = t->swnod.swlst;
			   WHILE eg
			   DO	ARGPTR	rex=eg->regptr;
				WHILE rex
				DO	REG STRING	s;
					IF gmatch(r,s=macro(rex->argval)) ORF (trim(s), eq(r,s))
					THEN	execute(eg->regcom,0,0,0);
						t=0; break;
					ELSE	rex=rex->argnxt;
					FI
				OD
				IF eg THEN eg=eg->regnxt FI
			   OD
			END
			break;
		ENDSW
		exitset();
	FI

	sigchk();
	tdystak(sav);
	return(exitval);
}


VOID	execexp(s,f)
	STRING		s;
	UFD		f;
{
	FILEBLK		fb;
	push(&fb);
	IF s
	THEN	estabf(s); fb.feval=(STRING *)f;
	ELIF f>=0
	THEN	initf(f);
	FI
	execute(cmd(NL, NLFLG|MTFLG),0,0,0);
	pop();
}
