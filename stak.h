#
/*
 *	v7sh-amd64 (UNIX shell)
 *
 *	S. R. Bourne
 *	Bell Telephone Laboratories
 *
 */

/* To use stack as temporary workspace across
 * possible storage allocation (eg name lookup)
 * a) get ptr from `relstak'
 * b) can now use `pushstak'
 * c) then reset with `setstak'
 * d) `absstak' gives real address if needed
 */
#define		relstak()	(staktop-stakbot)
#define		absstak(x)	(stakbot+Rcheat(x))
#define		setstak(x)	(staktop=absstak(x))
#define		pushstak(c)	(*staktop++=(c))
#define		zerostak()	(*staktop=0)

/* Used to address an item left on the top of
 * the stack (very temporary)
 */
#define		curstak()	(staktop)

/* `usestak' before `pushstak' then `fixstak'
 * These routines are safe against heap
 * being allocated.
 */
#define		usestak()	{locstak();}

/* for local use only since it hands
 * out a real address for the stack top
 */
PROC STKPTR	locstak(VOID);

/* Will allocate the item being used and return its
 * address (safe now).
 */
#define		fixstak()	endstak(staktop)

/* For use after `locstak' to hand back
 * new stack top and then allocate item
 */
PROC STKPTR	endstak(STRING);

/* Copy a string onto the stack and
 * allocate the space.
 */
PROC STKPTR	cpystak(CSTKPTR);

/* Allocate given ammount of stack space */
PROC STKPTR	getstak(POS);

/* A chain of ptrs of stack blocks that
 * have become covered by heap allocation.
 * `tdystak' will return them to the heap.
 */
EXTERN BLKPTR		stakbsy;

/* Base of the entire stack */
EXTERN STKPTR		stakbas;

/* Top of entire stack */
EXTERN STKPTR		brkend;

/* Base of current item */
EXTERN STKPTR		stakbot;

/* Top of current item */
EXTERN STKPTR		staktop;

/* Used with tdystak */
PROC STKPTR	savstak(VOID);

PROC VOID	tdystak(STKPTR);
PROC VOID	stakchk(VOID);
