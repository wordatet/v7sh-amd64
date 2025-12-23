#
/*
 *	v7sh-amd64 (UNIX shell)
 *
 *	S. R. Bourne
 *	Bell Telephone Laboratories
 *
 */

#include	"defs.h"
#include	<stdlib.h>

/*
 *	storage allocator
 *	(now using standard malloc with tracking)
 */

/* Magic number to identify our allocations */
#define ALLOC_MAGIC	0xA110CA7E

/* Extended header for tracking allocations */
struct alloc_header {
	unsigned int magic;
	size_t size;
};

#define HEADER_SIZE	(sizeof(struct alloc_header))

INT		brkincr=BRKINCR;

VOID	blok_init()
{
	/* No initialization needed for malloc */
}

ADDRESS	alloc(nbytes)
	POS		nbytes;
{
	REG POS		rbytes = round(nbytes + HEADER_SIZE, BYTESPERWORD);
	struct alloc_header *hdr = (struct alloc_header *) malloc(rbytes);
	
	if (hdr == NIL) {
		error(nospace);
	}
	
	hdr->magic = ALLOC_MAGIC;
	hdr->size = rbytes;
	
	return (ADDRESS)(hdr + 1);
}

VOID	shfree(ap)
	BLKPTR		ap;
{
	struct alloc_header *hdr;
	
	if (ap == NIL) {
		return;
	}
	
	/* Get the header */
	hdr = ((struct alloc_header *)ap) - 1;
	
	/* Only free if this was allocated by our alloc() */
	if (hdr->magic == ALLOC_MAGIC) {
		hdr->magic = 0;  /* Prevent double-free */
		free(hdr);
	}
	/* Otherwise silently ignore - it's a static/stack pointer */
}

VOID	addblok(reqd)
	POS		reqd;
{
	/* No-op in malloc mode */
}
