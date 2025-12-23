#
/*
 *	v7sh-amd64 (UNIX shell)
 *
 *	S. R. Bourne
 *	Bell Telephone Laboratories
 *
 */


#include	"defs.h"
#include	"sym.h"
#include	"pathnames.h"

CMSG		version = "\nVERSION sys137	DATE 1978 Nov 6 14:29:22\n";

/* error messages */
CMSG	badopt		= "bad option(s)";
CMSG	mailmsg		= "you have mail\n";
CMSG	nospace		= "no space";
CMSG	synmsg		= "syntax error";

CMSG	badnum		= "bad number";
#if defined(SYSIII)
CMSG	badparam	= "parameter null or not set";
CMSG	unset		= "parameter not set";
#else /* V7 */
CMSG	badparam	= "parameter not set";
#endif
CMSG	badsub		= "bad substitution";
CMSG	badcreate	= "cannot create";
CMSG	illegal		= "illegal io";
CMSG	restricted	= "restricted";
#if defined(SYSIII)
CMSG	nofork		= "cannot fork: too many processes";
CMSG	noswap		= "cannot fork: no swap space";
#endif
CMSG	piperr		= "cannot make pipe";
CMSG	badopen		= "cannot open";
CMSG	coredump	= " - core dumped";
CMSG	arglist		= "arg list too long";
#if defined(SYSIII)
CMSG	argcount	= "argument count";
#endif
CMSG	txtbsy		= "text busy";
CMSG	toobig		= "too big";
CMSG	badexec		= "cannot execute";
CMSG	notfound	= "not found";
CMSG	badfile		= "bad file number";
CMSG	badshift	= "cannot shift";
CMSG	baddir		= "bad directory";
CMSG	badtrap		= "bad trap";
CMSG	memfault	= "cannot trap 11";
CMSG	wtfailed	= "is read only";
CMSG	notid		= "is not an identifier";
CMSG	badulimit	= "bad ulimit";

#if defined(SYSIII)
/*	messages for 'builtin' functions	*/
CMSG	btest		= "test";
CMSG	badop		= "unknown operator";
#endif

/* built in names */
CMSG	pathname	= "PATH";
CMSG	homename	= "HOME";
CMSG	mailname	= "MAIL";
CMSG	fngname		= "FILEMATCH";
CMSG	ifsname		= "IFS";
CMSG	ps1name		= "PS1";
CMSG	ps2name		= "PS2";

/* string constants */
CMSG	nullstr		= "";
CMSG	sptbnl		= " \t\n";
CMSG	defpath		= _PATH_DEFPATH;
CMSG	colon		= ": ";
CMSG	minus		= "-";
CMSG	endoffile	= "end of file";
CMSG	endofline	= "newline or ;";
CMSG	unexpected 	= " unexpected";
CMSG	atline		= " at line ";
CMSG	devnull		= _PATH_DEVNULL;
CMSG	execpmsg	= "+ ";
CMSG	readmsg		= "> ";
CMSG	stdprompt	= "$ ";
CMSG	supprompt	= "# ";
CMSG	profile		= _PATH_DOTPROFILE;
#if defined(SYSIII)
CMSG	sysprofile	= _PATH_ETCPROFILE;
#endif

/* tables */
SYSTAB reserved={
		{"in",		INSYM},
		{"esac",	ESSYM},
		{"case",	CASYM},
		{"for",		FORSYM},
		{"done",	ODSYM},
		{"if",		IFSYM},
		{"while",	WHSYM},
		{"do",		DOSYM},
		{"then",	THSYM},
		{"else",	ELSYM},
		{"elif",	EFSYM},
		{"fi",		FISYM},
		{"until",	UNSYM},
		{ "{",		BRSYM},
		{ "}",		KTSYM},
		{0,	0}
};

CSTRING	sysmsg[]={
		0,
		"Hangup",
		0,	/* Interrupt */
		"Quit",
		"Illegal instruction",
		"Trace/BPT trap",
		"Abort trap",	/* IOT trap */
		"EMT trap",
		"Floating point exception",
		"Killed",
		"Bus error",
		"Memory fault",
		"Bad system call",
		0,	/* Broken pipe */
		"Alarm call",
		"Terminated",
		"Urgent I/O condition",
		"Stopped",
		"Stopped from terminal",
		"Continued",
		"Child exited",
		"Stopped on terminal input",
		"Stopped on terminal output",
		"Asynchronous I/O",
		"Cputime limit exceeded",
		"Filesize limit exceeded",
		"Virtual timer expired",
		"Profiling timer expired",
		"Window size changed",
		"Information request",
		"User defined signal 1",
		"User defined signal 2",
		"Thread interrupt"
};
#if defined(RENO)
INT		num_sysmsg = (sizeof sysmsg / sizeof sysmsg[0]);
#endif

CMSG		export = "export";
CMSG		readonly = "readonly";

SYSTAB	commands={
		{"cd",		SYSCD},
		{"read",	SYSREAD},
		{"set",		SYSSET},
		{":",		SYSNULL},
		{"trap",	SYSTRAP},
#if defined(ULTRIX)
		{"ulimit",	SYSULIMIT},
#endif
		{"login",	SYSLOGIN},
		{"wait",	SYSWAIT},
		{"eval",	SYSEVAL},
		{".",		SYSDOT},
#if defined(SYSIII)
		{"newgrp",	SYSNEWGRP},
#else /* V7 */
		{"newgrp",	SYSLOGIN},
#endif
		{readonly,	SYSRDONLY},
		{export,	SYSXPORT},
		{"chdir",	SYSCD},
		{"break",	SYSBREAK},
		{"continue",	SYSCONT},
		{"shift",	SYSSHFT},
		{"exit",	SYSEXIT},
		{"exec",	SYSEXEC},
		{"times",	SYSTIMES},
		{"umask",	SYSUMASK},
		{0,	0}
};

#if defined(SYSIII)
SYSTAB	builtins={
		{btest,		TEST},
		{"[",		TEST},
		{0,	0}
};
#endif
