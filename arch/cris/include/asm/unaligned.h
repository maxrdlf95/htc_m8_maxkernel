#ifndef _ASM_CRIS_UNALIGNED_H
#define _ASM_CRIS_UNALIGNED_H

#include <linux/unaligned/access_ok.h>
#include <linux/unaligned/generic.h>

#define get_unaligned	__get_unaligned_le
#define put_unaligned	__put_unaligned_le

#endif 
