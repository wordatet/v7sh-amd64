/*
 * v7sh-amd64 (UNIX shell)
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 * This port (v7sh-amd64) is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as published 
 * by the Free Software Foundation; either version 2 of the License.
 *
 *      test expression
 *      [ expression ]
 */

#if defined(SYSIII)
#include	"defs.h"

#undef eq
#define eq(a,b)	((at=a)==0?0:cf(at,b)==0)

LOCAL STRING	nxtarg(INT);
#define exp	exp_
LOCAL INT	exp(VOID);
LOCAL INT	e1(VOID);
LOCAL INT	e2(VOID);
LOCAL INT	e3(VOID);
LOCAL INT	tio(STRING, INT);
LOCAL INT	ftype(STRING, INT);
LOCAL INT	fsizep(STRING);

LOCAL CMSG	bramis	= "] missing";
LOCAL CMSG	argexp	= "argument expected";
LOCAL CMSG	braexp	= ") expected";

LOCAL INT	ap, ac;
LOCAL STRING	*av;
LOCAL STRING	at;


INT	test(argn, com)
	INT		argn;
	STRING		com[];
{
        ac = argn; av = com; ap = 1;
        IF eq(com[0],"[")
	THEN	IF !eq(com[--ac], "]")
		THEN	failed(btest, bramis);
			/*NOTREACHED*/
		FI
	FI
        com[ac] = 0;
	IF ac <= 1 THEN return(1) FI
        return(exp()?0:1);
}

LOCAL STRING	nxtarg(mt)
	INT		mt;
{
	IF ap >= ac
	THEN	IF mt
		THEN	ap++;
                        return(0);
		FI
		failed(btest, argexp);
		/*NOTREACHED*/
	FI
        return(av[ap++]);
}

LOCAL INT	exp()
{
	INT		p1;

        p1 = e1();
	IF eq(nxtarg(1), "-o") THEN return(p1 | exp()) FI
        ap--;
        return(p1);
}

LOCAL INT	e1()
{
	INT		p1;

        p1 = e2();
	IF eq(nxtarg(1), "-a") THEN return(p1 & e1()) FI
        ap--;
        return(p1);
}

LOCAL INT	e2()
{
        IF eq(nxtarg(0), "!")
	THEN	return(!e3())
	FI
        ap--;
        return(e3());
}

LOCAL INT	e3()
{
	INT		p1;
	REG STRING	a;
	STRING		p2;
	LONG		int1, int2;

        a=nxtarg(0);
        IF eq(a, "(")
	THEN	p1 = exp();
		IF !eq(nxtarg(0), ")")
		THEN	failed(btest, braexp)
			/*NOTREACHED*/
		FI
                return(p1);
	FI

        p2 = nxtarg(1);
        ap--;
        IF !eq(p2,"=")&&!eq(p2,"!=")
	THEN	IF eq(a, "-r") THEN return(tio(nxtarg(0), R_OK)) FI
		IF eq(a, "-w") THEN return(tio(nxtarg(0), W_OK)) FI
		IF eq(a, "-x") THEN return(tio(nxtarg(0), X_OK)) FI
		IF eq(a, "-d") THEN return(ftype(nxtarg(0), S_IFDIR)) FI
		IF eq(a, "-c") THEN return(ftype(nxtarg(0),S_IFCHR)) FI
		IF eq(a, "-b") THEN return(ftype(nxtarg(0), S_IFBLK)) FI
		IF eq(a, "-f") THEN return(ftype(nxtarg(0), S_IFREG)) FI
		IF eq(a, "-u") THEN return(ftype(nxtarg(0), S_ISUID)) FI
		IF eq(a, "-g") THEN return(ftype(nxtarg(0), S_ISGID)) FI
		IF eq(a, "-k") THEN return(ftype(nxtarg(0), S_ISVTX)) FI
		IF eq(a, "-s") THEN return(fsizep(nxtarg(0))) FI
		IF eq(a, "-t")
		THEN	IF ap >= ac	/* no args */
			THEN return(isatty(1));
			ELIF eq((a=nxtarg(0)), "-a")
				ORF eq(a, "-o")
			     THEN	ap--;
					return(isatty(1));
			ELSE return(isatty(atoi(a)));
			FI
		FI
		IF eq(a, "-n") THEN return(!eq(nxtarg(0), "")) FI
		IF eq(a, "-z") THEN return(eq(nxtarg(0), "")) FI
	FI

        p2 = nxtarg(1);
	IF p2==0 THEN return(!eq(a, "")) FI
	IF eq(p2, "-a") ORF eq(p2, "-o") 
	THEN	ap--;
		return(!eq(a, ""))
	FI
	IF eq(p2, "=") THEN return(eq(nxtarg(0), a)) FI
	IF eq(p2, "!=") THEN return(!eq(nxtarg(0), a)) FI
	IF eq(a, "-l")
	THEN	int1 = (LONG) length(p2) - 1;
		p2 = nxtarg(0);
	ELSE
        int1 = atol(a);
	FI
        int2 = atol(nxtarg(0));
	IF eq(p2, "-eq") THEN return(int1==int2) FI
	IF eq(p2, "-ne") THEN return(int1!=int2) FI
	IF eq(p2, "-gt") THEN return(int1>int2) FI
	IF eq(p2, "-lt") THEN return(int1<int2) FI
	IF eq(p2, "-ge") THEN return(int1>=int2) FI
	IF eq(p2, "-le") THEN return(int1<=int2) FI

	bfailed(btest, badop, p2);
	/*NOTREACHED*/
	return(0);				/* GCC */
}

LOCAL INT	tio(a, f)
	STRING		a;
	INT		f;
{
	IF access(a, f)==0 
	THEN	return(1);
	ELSE	return(0)
	FI
}

LOCAL INT	ftype(f,field)
	STRING		f;
	INT		field;
{
	STATBUF		statb;

	IF stat(f,&statb)<0 THEN return(0) FI
	IF (statb.st_mode&field)==field
	THEN	return(1);
	ELSE	return(0)
	FI
}

LOCAL INT	fsizep(f)
	STRING		f;
{
	STATBUF		statb;

	IF stat(f, &statb) <0 THEN return(0) FI
        return(statb.st_size>0);
}
#endif
