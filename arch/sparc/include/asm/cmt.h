#ifndef _SPARC64_CMT_H
#define _SPARC64_CMT_H

/* cmt.h: Chip Multi-Threading register definitions
 *
 * Copyright (C) 2004 David S. Miller (davem@redhat.com)
 */

#define LP_ID		0x0000000000000010UL
#define  LP_ID_MAX	0x00000000003f0000UL
#define  LP_ID_ID	0x000000000000003fUL

#define LP_INTR_ID	0x0000000000000000UL
#define  LP_INTR_ID_ID	0x00000000000003ffUL

#define CESR_ID		0x0000000000000040UL
#define  CESR_ID_ID	0x00000000000000ffUL

#define LP_AVAIL	0x0000000000000000UL
#define  LP_AVAIL_1	0x0000000000000002UL
#define  LP_AVAIL_0	0x0000000000000001UL

#define LP_ENAB_STAT	0x0000000000000010UL
#define  LP_ENAB_STAT_1	0x0000000000000002UL
#define  LP_ENAB_STAT_0	0x0000000000000001UL

#define LP_ENAB		0x0000000000000020UL
#define  LP_ENAB_1	0x0000000000000002UL
#define  LP_ENAB_0	0x0000000000000001UL

#define LP_RUNNING_RW	0x0000000000000050UL
#define LP_RUNNING_W1S	0x0000000000000060UL
#define LP_RUNNING_W1C	0x0000000000000068UL
#define  LP_RUNNING_1	0x0000000000000002UL
#define  LP_RUNNING_0	0x0000000000000001UL

#define LP_RUN_STAT	0x0000000000000058UL
#define  LP_RUN_STAT_1	0x0000000000000002UL
#define  LP_RUN_STAT_0	0x0000000000000001UL

#define LP_XIR_STEER	0x0000000000000030UL
#define  LP_XIR_STEER_1	0x0000000000000002UL
#define  LP_XIR_STEER_0	0x0000000000000001UL

#define CMT_ER_STEER	0x0000000000000040UL
#define  CMT_ER_STEER_1	0x0000000000000002UL
#define  CMT_ER_STEER_0	0x0000000000000001UL

#endif 
