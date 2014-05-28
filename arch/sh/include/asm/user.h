#ifndef __ASM_SH_USER_H
#define __ASM_SH_USER_H

#include <asm/ptrace.h>
#include <asm/page.h>

/*
 * Core file format: The core file is written in such a way that gdb
 * can understand it and provide useful information to the user (under
 * linux we use the `trad-core' bfd).  The file contents are as follows:
 *
 *  upage: 1 page consisting of a user struct that tells gdb
 *	what is present in the file.  Directly after this is a
 *	copy of the task_struct, which is currently not used by gdb,
 *	but it may come in handy at some point.  All of the registers
 *	are stored as part of the upage.  The upage should always be
 *	only one page long.
 *  data: The data segment follows next.  We use current->end_text to
 *	current->brk to pick up all of the user variables, plus any memory
 *	that may have been sbrk'ed.  No attempt is made to determine if a
 *	page is demand-zero or if a page is totally unused, we just cover
 *	the entire range.  All of the addresses are rounded in such a way
 *	that an integral number of pages is written.
 *  stack: We need the stack information in order to get a meaningful
 *	backtrace.  We need to write the data from usp to
 *	current->start_stack, so we round each of these in order to be able
 *	to write an integer number of pages.
 */

#if defined(__SH5__) || defined(CONFIG_CPU_SH5)
struct user_fpu_struct {
	unsigned long fp_regs[32];
	unsigned int fpscr;
};
#else
struct user_fpu_struct {
	unsigned long fp_regs[16];
	unsigned long xfp_regs[16];
	unsigned long fpscr;
	unsigned long fpul;
};
#endif

struct user {
	struct pt_regs	regs;			
	struct user_fpu_struct fpu;	
	int u_fpvalid;		
	size_t		u_tsize;		
	size_t		u_dsize;		
	size_t		u_ssize;		
	unsigned long	start_code;		
	unsigned long	start_data;		
	unsigned long	start_stack;		
	long int	signal;			
	unsigned long	u_ar0;			
	struct user_fpu_struct* u_fpstate;	
	unsigned long	magic;			
	char		u_comm[32];		
};

#define NBPG			PAGE_SIZE
#define UPAGES			1
#define HOST_TEXT_START_ADDR	(u.start_code)
#define HOST_DATA_START_ADDR	(u.start_data)
#define HOST_STACK_END_ADDR	(u.start_stack + u.u_ssize * NBPG)

#endif 
