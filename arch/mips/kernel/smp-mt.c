/*
 *  This program is free software; you can distribute it and/or modify it
 *  under the terms of the GNU General Public License (Version 2) as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 * Copyright (C) 2004, 05, 06 MIPS Technologies, Inc.
 *    Elizabeth Clarke (beth@mips.com)
 *    Ralf Baechle (ralf@linux-mips.org)
 * Copyright (C) 2006 Ralf Baechle (ralf@linux-mips.org)
 */
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/cpumask.h>
#include <linux/interrupt.h>
#include <linux/compiler.h>
#include <linux/smp.h>

#include <linux/atomic.h>
#include <asm/cacheflush.h>
#include <asm/cpu.h>
#include <asm/processor.h>
#include <asm/hardirq.h>
#include <asm/mmu_context.h>
#include <asm/time.h>
#include <asm/mipsregs.h>
#include <asm/mipsmtregs.h>
#include <asm/mips_mt.h>

static void __init smvp_copy_vpe_config(void)
{
	write_vpe_c0_status(
		(read_c0_status() & ~(ST0_IM | ST0_IE | ST0_KSU)) | ST0_CU0);

	
	write_vpe_c0_config( read_c0_config());

	
	write_vpe_c0_cause(0);

	
	write_vpe_c0_config7(read_c0_config7());

	write_vpe_c0_count(read_c0_count());
}

static unsigned int __init smvp_vpe_init(unsigned int tc, unsigned int mvpconf0,
	unsigned int ncpu)
{
	if (tc > ((mvpconf0 & MVPCONF0_PVPE) >> MVPCONF0_PVPE_SHIFT))
		return ncpu;

	
	if (tc != 0) {
		unsigned long tmp = read_vpe_c0_vpeconf0();

		tmp &= ~VPECONF0_VPA;

		
		tmp |= VPECONF0_MVP;
		write_vpe_c0_vpeconf0(tmp);

		
		set_cpu_possible(tc, true);
		__cpu_number_map[tc]	= ++ncpu;
		__cpu_logical_map[ncpu]	= tc;
	}

	
	write_vpe_c0_vpecontrol(read_vpe_c0_vpecontrol() & ~VPECONTROL_TE);

	if (tc != 0)
		smvp_copy_vpe_config();

	return ncpu;
}

static void __init smvp_tc_init(unsigned int tc, unsigned int mvpconf0)
{
	unsigned long tmp;

	if (!tc)
		return;

	if (tc >= (((mvpconf0 & MVPCONF0_PVPE) >> MVPCONF0_PVPE_SHIFT)+1))
		write_tc_c0_tcbind(read_tc_c0_tcbind() | ((mvpconf0 & MVPCONF0_PVPE) >> MVPCONF0_PVPE_SHIFT));
	else {
		write_tc_c0_tcbind(read_tc_c0_tcbind() | tc);

		
		write_vpe_c0_vpeconf0(read_vpe_c0_vpeconf0() | (tc << VPECONF0_XTC_SHIFT));
	}

	tmp = read_tc_c0_tcstatus();

	
	tmp &= ~(TCSTATUS_A | TCSTATUS_DA);
	tmp |= TCSTATUS_IXMT;		
	write_tc_c0_tcstatus(tmp);

	write_tc_c0_tchalt(TCHALT_H);
}

static void vsmp_send_ipi_single(int cpu, unsigned int action)
{
	int i;
	unsigned long flags;
	int vpflags;

	local_irq_save(flags);

	vpflags = dvpe();	

	switch (action) {
	case SMP_CALL_FUNCTION:
		i = C_SW1;
		break;

	case SMP_RESCHEDULE_YOURSELF:
	default:
		i = C_SW0;
		break;
	}

	
	settc(cpu);
	write_vpe_c0_cause(read_vpe_c0_cause() | i);
	evpe(vpflags);

	local_irq_restore(flags);
}

static void vsmp_send_ipi_mask(const struct cpumask *mask, unsigned int action)
{
	unsigned int i;

	for_each_cpu(i, mask)
		vsmp_send_ipi_single(i, action);
}

static void vsmp_init_secondary(void)
{
	extern int gic_present;

	
	if (gic_present)
		change_c0_status(ST0_IM, STATUSF_IP3 | STATUSF_IP4 |
					 STATUSF_IP6 | STATUSF_IP7);
	else
		change_c0_status(ST0_IM, STATUSF_IP0 | STATUSF_IP1 |
					 STATUSF_IP6 | STATUSF_IP7);
}

static void vsmp_smp_finish(void)
{
	
	write_c0_compare(read_c0_count() + (8* mips_hpt_frequency/HZ));

#ifdef CONFIG_MIPS_MT_FPAFF
	
	if (cpu_has_fpu)
		cpu_set(smp_processor_id(), mt_fpu_cpumask);
#endif 

	local_irq_enable();
}

static void vsmp_cpus_done(void)
{
}

static void vsmp_boot_secondary(int cpu, struct task_struct *idle)
{
	struct thread_info *gp = task_thread_info(idle);
	dvpe();
	set_c0_mvpcontrol(MVPCONTROL_VPC);

	settc(cpu);

	
	write_tc_c0_tcrestart((unsigned long)&smp_bootstrap);

	
	write_tc_c0_tcstatus((read_tc_c0_tcstatus() & ~TCSTATUS_IXMT) | TCSTATUS_A);

	write_tc_c0_tchalt(0);

	
	write_vpe_c0_vpeconf0(read_vpe_c0_vpeconf0() | VPECONF0_VPA);

	
	write_tc_gpr_sp( __KSTK_TOS(idle));

	
	write_tc_gpr_gp((unsigned long)gp);

	flush_icache_range((unsigned long)gp,
	                   (unsigned long)(gp + sizeof(struct thread_info)));

	
	clear_c0_mvpcontrol(MVPCONTROL_VPC);

	evpe(EVPE_ENABLE);
}

static void __init vsmp_smp_setup(void)
{
	unsigned int mvpconf0, ntc, tc, ncpu = 0;
	unsigned int nvpe;

#ifdef CONFIG_MIPS_MT_FPAFF
	
	if (cpu_has_fpu)
		cpu_set(0, mt_fpu_cpumask);
#endif 
	if (!cpu_has_mipsmt)
		return;

	
	dvpe();
	dmt();

	
	set_c0_mvpcontrol(MVPCONTROL_VPC);

	mvpconf0 = read_c0_mvpconf0();
	ntc = (mvpconf0 & MVPCONF0_PTC) >> MVPCONF0_PTC_SHIFT;

	nvpe = ((mvpconf0 & MVPCONF0_PVPE) >> MVPCONF0_PVPE_SHIFT) + 1;
	smp_num_siblings = nvpe;

	for (tc = 0; tc <= ntc; tc++) {
		settc(tc);

		smvp_tc_init(tc, mvpconf0);
		ncpu = smvp_vpe_init(tc, mvpconf0, ncpu);
	}

	
	clear_c0_mvpcontrol(MVPCONTROL_VPC);

	

	printk(KERN_INFO "Detected %i available secondary CPU(s)\n", ncpu);
}

static void __init vsmp_prepare_cpus(unsigned int max_cpus)
{
	mips_mt_set_cpuoptions();
}

struct plat_smp_ops vsmp_smp_ops = {
	.send_ipi_single	= vsmp_send_ipi_single,
	.send_ipi_mask		= vsmp_send_ipi_mask,
	.init_secondary		= vsmp_init_secondary,
	.smp_finish		= vsmp_smp_finish,
	.cpus_done		= vsmp_cpus_done,
	.boot_secondary		= vsmp_boot_secondary,
	.smp_setup		= vsmp_smp_setup,
	.prepare_cpus		= vsmp_prepare_cpus,
};
