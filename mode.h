#
/*
 *	v7sh-amd64 (UNIX shell)
 *
 *	S. R. Bourne
 *	Bell Telephone Laboratories
 *
 */


#define BYTESPERWORD	(sizeof(char *))

TYPE char	CHAR;
TYPE char	BOOL;
TYPE int	UFD;
TYPE int	INT;
TYPE unsigned	UINT;
TYPE float	REAL;
TYPE char	*ADDRESS;
TYPE long	LONG;
TYPE void	VOID;
TYPE size_t	POS;
TYPE char	*STRING;
TYPE CONST char	*CSTRING;
TYPE char	MSG[];
TYPE CONST char	CMSG[];
TYPE int	PIPE[];
TYPE char	*STKPTR;
TYPE const char	*CSTKPTR;
TYPE char	*BYTPTR;

TYPE clock_t	CLOCK;
TYPE mode_t	MODE;
TYPE off_t	OFFSET;
TYPE sig_t	SIGPTR;
TYPE size_t	SIZE;
TYPE time_t	TIME;
TYPE uid_t	UID;

STRUCT dirent	*DIRPTR;	/* defined in dirent.h */
STRUCT stat	STATBUF;	/* defined in sys/stat.h */
STRUCT tms	TIMEBUF;	/* defined in sys/time.h */

STRUCT blk	*BLKPTR;
STRUCT fileblk	FILEBLK;
STRUCT filehdr	FILEHDR;
STRUCT fileblk	*FILEPTR;
UNION trenod	*TREPTR;
STRUCT forknod	*FORKPTR;
STRUCT comnod	*COMPTR;
STRUCT swnod	*SWPTR;
STRUCT regnod	*REGPTR;
STRUCT parnod	*PARPTR;
STRUCT ifnod	*IFPTR;
STRUCT whnod	*WHPTR;
STRUCT fornod	*FORPTR;
STRUCT lstnod	*LSTPTR;
STRUCT argnod	*ARGPTR;
STRUCT dolnod	*DOLPTR;
STRUCT ionod	*IOPTR;
STRUCT namnod	NAMNOD;
STRUCT namnod	*NAMPTR;
STRUCT sysnod	SYSNOD;
STRUCT sysnod	*SYSPTR;
#define NIL	((void*)0)


/* the following nonsense is required
 * because casts turn an Lvalue
 * into an Rvalue so two cheats
 * are necessary, one for each context.
 */
#include <stdint.h>
#if defined(RENO)
#define Lcheat(a)	(*(intptr_t *)&(a))
#else /* V7 */
union _cheat { intptr_t _cheat;};
#define Lcheat(a)	((*(union _cheat *)&(a))._cheat)
#endif
#define Rcheat(a)	((intptr_t)(a))


/* address puns for storage allocation */
UNION {
	FORKPTR	_forkptr;
	COMPTR	_comptr;
	PARPTR	_parptr;
	IFPTR	_ifptr;
	WHPTR	_whptr;
	FORPTR	_forptr;
	LSTPTR	_lstptr;
	BLKPTR	_blkptr;
	NAMPTR	_namptr;
	BYTPTR	_bytptr;
}	address;


#if defined(V7)
/* for functions that do not return values */
struct void {INT vvvvvvvv;};
#endif


/* heap storage */
struct blk {
	BLKPTR	word;
};

#define	BUFSIZ	64
struct fileblk {
	UFD	fdes;
	POS	flin;
	BOOL	feof;
	CHAR	fsiz;
	STRING	fnxt;
	STRING	fend;
	STRING	*feval;
	FILEPTR	fstak;
	CHAR	fbuf[BUFSIZ];
};

/* for files not used with file descriptors */
struct filehdr {
	UFD	fdes;
	POS	flin;
	BOOL	feof;
	CHAR	fsiz;
	STRING	fnxt;
	STRING	fend;
	STRING	*feval;
	FILEPTR	fstak;
	CHAR	_fbuf[1];
};

struct sysnod {
	CSTRING	sysnam;
	INT	sysval;
};

STRUCT sysnod	SYSTAB[];

/* dummy for access only */
struct argnod {
	ARGPTR	argnxt;
	CHAR	argval[1];
};

struct dolnod {
	DOLPTR	dolnxt;
	INT	doluse;
	CHAR	dolarg[1];
};

struct forknod {
	INT	forktyp;
	IOPTR	forkio;
	TREPTR	forktre;
};

struct comnod {
	INT	comtyp;
	IOPTR	comio;
	ARGPTR	comarg;
	ARGPTR	comset;
};

struct ifnod {
	INT	iftyp;
	TREPTR	iftre;
	TREPTR	thtre;
	TREPTR	eltre;
};

struct whnod {
	INT	whtyp;
	TREPTR	whtre;
	TREPTR	dotre;
};

struct fornod {
	INT	fortyp;
	TREPTR	fortre;
	STRING	fornam;
	COMPTR	forlst;
};

struct swnod {
	INT	swtyp;
	STRING	swarg;
	REGPTR	swlst;
};

struct regnod {
	ARGPTR	regptr;
	TREPTR	regcom;
	REGPTR	regnxt;
};

struct parnod {
	INT	partyp;
	TREPTR	partre;
};

struct lstnod {
	INT	lsttyp;
	TREPTR	lstlef;
	TREPTR	lstrit;
};

struct ionod {
	INT	iofile;
	STRING	ioname;
	IOPTR	ionxt;
	IOPTR	iolst;
};

#define	FORKTYPE	(sizeof(struct forknod))
#define	COMTYPE		(sizeof(struct comnod))
#define	IFTYPE		(sizeof(struct ifnod))
#define	WHTYPE		(sizeof(struct whnod))
#define	FORTYPE		(sizeof(struct fornod))
#define	SWTYPE		(sizeof(struct swnod))
#define	REGTYPE		(sizeof(struct regnod))
#define	PARTYPE		(sizeof(struct parnod))
#define	LSTTYPE		(sizeof(struct lstnod))
#define	IOTYPE		(sizeof(struct ionod))

/* this node is a proforma for those that precede */
union trenod {
	struct { INT	tretyp; IOPTR	treio; } treio;
	struct forknod	forknod;
	struct comnod	comnod;
	struct ifnod	ifnod;
	struct whnod	whnod;
	struct fornod	fornod;
	struct swnod	swnod;
	struct parnod	parnod;
	struct lstnod	lstnod;
};
