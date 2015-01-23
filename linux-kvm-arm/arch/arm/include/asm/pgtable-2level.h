/*
 *  arch/arm/include/asm/pgtable-2level.h
 *
 *  Copyright (C) 1995-2002 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _ASM_PGTABLE_2LEVEL_H
#define _ASM_PGTABLE_2LEVEL_H

/*
 * Hardware-wise, we have a two level page table structure, where the first
 * level has 4096 entries, and the second level has 256 entries.  Each entry
 * is one 32-bit word.  Most of the bits in the second level entry are used
 * by hardware, and there aren't any "accessed" and "dirty" bits.
 *
 * Linux on the other hand has a three level page table structure, which can
 * be wrapped to fit a two level page table structure easily - using the PGD
 * and PTE only.  However, Linux also expects one "PTE" table per page, and
 * at least a "dirty" bit.
 *
 * Therefore, we tweak the implementation slightly - we tell Linux that we
 * have 2048 entries in the first level, each of which is 8 bytes (iow, two
 * hardware pointers to the second level.)  The second level contains two
 * hardware PTE tables arranged contiguously, preceded by Linux versions
 * which contain the state information Linux needs.  We, therefore, end up
 * with 512 entries in the "PTE" level.
 *
 * This leads to the page tables having the following layout:
 *
 *    pgd             pte
 * |        |
 * +--------+
 * |        |       +------------+ +0
 * +- - - - +       | Linux pt 0 |
 * |        |       +------------+ +1024
 * +--------+ +0    | Linux pt 1 |
 * |        |-----> +------------+ +2048
 * +- - - - + +4    |  h/w pt 0  |
 * |        |-----> +------------+ +3072
 * +--------+ +8    |  h/w pt 1  |
 * |        |       +------------+ +4096
 *
 * See L_PTE_xxx below for definitions of bits in the "Linux pt", and
 * PTE_xxx for definitions of bits appearing in the "h/w pt".
 *
 * PMD_xxx definitions refer to bits in the first level page table.
 *
 * The "dirty" bit is emulated by only granting hardware write permission
 * iff the page is marked "writable" and "dirty" in the Linux PTE.  This
 * means that a write to a clean page will cause a permission fault, and
 * the Linux MM layer will mark the page dirty via handle_pte_fault().
 * For the hardware to notice the permission change, the TLB entry must
 * be flushed, and ptep_set_access_flags() does that for us.
 *
 * The "accessed" or "young" bit is emulated by a similar method; we only
 * allow accesses to the page if the "young" bit is set.  Accesses to the
 * page will cause a fault, and handle_pte_fault() will set the young bit
 * for us as long as the page is marked present in the corresponding Linux
 * PTE entry.  Again, ptep_set_access_flags() will ensure that the TLB is
 * up to date.
 *
 * However, when the "young" bit is cleared, we deny access to the page
 * by clearing the hardware PTE.  Currently Linux does not flush the TLB
 * for us in this case, which means the TLB will retain the transation
 * until either the TLB entry is evicted under pressure, or a context
 * switch which changes the user space mapping occurs.
 */
#define PTRS_PER_PTE		512
#define PTRS_PER_PMD		1
#define PTRS_PER_PGD		2048

#define PTE_HWTABLE_PTRS	(PTRS_PER_PTE)
#define PTE_HWTABLE_OFF		(PTE_HWTABLE_PTRS * sizeof(pte_t))
#define PTE_HWTABLE_SIZE	(PTRS_PER_PTE * sizeof(u32))

/*
 * PMD_SHIFT determines the size of the area a second-level page table can map
 * PGDIR_SHIFT determines what a third-level page table entry can map
 */
#define PMD_SHIFT		21
#define PGDIR_SHIFT		21

#define PMD_SIZE		(1UL << PMD_SHIFT)
#define PMD_MASK		(~(PMD_SIZE-1))
#define PGDIR_SIZE		(1UL << PGDIR_SHIFT)
#define PGDIR_MASK		(~(PGDIR_SIZE-1))

/*
 * section address mask and size definitions.
 */
#define SECTION_SHIFT		20
#define SECTION_SIZE		(1UL << SECTION_SHIFT)
#define SECTION_MASK		(~(SECTION_SIZE-1))

/*
 * ARMv6 supersection address mask and size definitions.
 */
#define SUPERSECTION_SHIFT	24
#define SUPERSECTION_SIZE	(1UL << SUPERSECTION_SHIFT)
#define SUPERSECTION_MASK	(~(SUPERSECTION_SIZE-1))

#define USER_PTRS_PER_PGD	(TASK_SIZE / PGDIR_SIZE)

/*
 * "Linux" PTE definitions.
 *
 * We keep two sets of PTEs - the hardware and the linux version.
 * This allows greater flexibility in the way we map the Linux bits
 * onto the hardware tables, and allows us to have YOUNG and DIRTY
 * bits.
 *
 * The PTE table pointer refers to the hardware entries; the "Linux"
 * entries are stored 1024 bytes below.
 */
#define L_PTE_VALID		(_AT(pteval_t, 1) << 0)		/* Valid */
#define L_PTE_PRESENT		(_AT(pteval_t, 1) << 0)
#define L_PTE_YOUNG		(_AT(pteval_t, 1) << 1)
#define L_PTE_FILE		(_AT(pteval_t, 1) << 2)	/* only when !PRESENT */
#define L_PTE_DIRTY		(_AT(pteval_t, 1) << 6)
#define L_PTE_RDONLY		(_AT(pteval_t, 1) << 7)
#define L_PTE_USER		(_AT(pteval_t, 1) << 8)
#define L_PTE_XN		(_AT(pteval_t, 1) << 9)
#define L_PTE_SHARED		(_AT(pteval_t, 1) << 10)	/* shared(v6), coherent(xsc3) */
#define L_PTE_NONE		(_AT(pteval_t, 1) << 11)

/*
 * These are the memory types, defined to be compatible with
 * pre-ARMv6 CPUs cacheable and bufferable bits:   XXCB
 */
#define L_PTE_MT_UNCACHED	(_AT(pteval_t, 0x00) << 2)	/* 0000 */
#define L_PTE_MT_BUFFERABLE	(_AT(pteval_t, 0x01) << 2)	/* 0001 */
#define L_PTE_MT_WRITETHROUGH	(_AT(pteval_t, 0x02) << 2)	/* 0010 */
#define L_PTE_MT_WRITEBACK	(_AT(pteval_t, 0x03) << 2)	/* 0011 */
#define L_PTE_MT_MINICACHE	(_AT(pteval_t, 0x06) << 2)	/* 0110 (sa1100, xscale) */
#define L_PTE_MT_WRITEALLOC	(_AT(pteval_t, 0x07) << 2)	/* 0111 */
#define L_PTE_MT_DEV_SHARED	(_AT(pteval_t, 0x04) << 2)	/* 0100 */
#define L_PTE_MT_DEV_NONSHARED	(_AT(pteval_t, 0x0c) << 2)	/* 1100 */
#define L_PTE_MT_DEV_WC		(_AT(pteval_t, 0x09) << 2)	/* 1001 */
#define L_PTE_MT_DEV_CACHED	(_AT(pteval_t, 0x0b) << 2)	/* 1011 */
#define L_PTE_MT_MASK		(_AT(pteval_t, 0x0f) << 2)

#ifndef __ASSEMBLY__

/*
 * The "pud_xxx()" functions here are trivial when the pmd is folded into
 * the pud: the pud entry is never bad, always exists, and can't be set or
 * cleared.
 */
#define pud_none(pud)		(0)
#define pud_bad(pud)		(0)
#define pud_present(pud)	(1)
#define pud_clear(pudp)		do { } while (0)
#define set_pud(pud,pudp)	do { } while (0)

static inline pmd_t *pmd_offset(pud_t *pud, unsigned long addr)
{
	return (pmd_t *)pud;
}

#define pmd_bad(pmd)		(pmd_val(pmd) & 2)

#define copy_pmd(pmdpd,pmdps)		\
	do {				\
		pmdpd[0] = pmdps[0];	\
		pmdpd[1] = pmdps[1];	\
		flush_pmd_entry(pmdpd);	\
	} while (0)

#define pmd_clear(pmdp)			\
	do {				\
		pmdp[0] = __pmd(0);	\
		pmdp[1] = __pmd(0);	\
		clean_pmd_entry(pmdp);	\
	} while (0)

/* we don't need complex calculations here as the pmd is folded into the pgd */
#define pmd_addr_end(addr,end) (end)

#define pmd_present(pmd)        ((pmd_val(pmd) & PMD_TYPE_MASK) != PMD_TYPE_FAULT)

#define set_pte_ext(ptep,pte,ext) cpu_set_pte_ext(ptep,pte,ext)


#ifdef CONFIG_SYS_SUPPORTS_HUGETLBFS

/*
 * now follows some of the definitions to allow huge page support, we can't put
 * these in the hugetlb source files as they are also required for transparent
 * hugepage support.
 */

#define HPAGE_SHIFT             PMD_SHIFT
#define HPAGE_SIZE              (_AC(1, UL) << HPAGE_SHIFT)
#define HPAGE_MASK              (~(HPAGE_SIZE - 1))
#define HUGETLB_PAGE_ORDER      (HPAGE_SHIFT - PAGE_SHIFT)

#define HUGE_LINUX_PTE_COUNT       (PAGE_OFFSET >> HPAGE_SHIFT)
#define HUGE_LINUX_PTE_SIZE        (HUGE_LINUX_PTE_COUNT * sizeof(pte_t *))
#define HUGE_LINUX_PTE_INDEX(addr) (addr >> HPAGE_SHIFT)

/*
 *  We re-purpose the following domain bits in the section descriptor
 */
#define PMD_DOMAIN_MASK		(_AT(pmdval_t, 0xF) << 5)
#define PMD_DSECT_DIRTY		(_AT(pmdval_t, 1) << 5)
#define PMD_DSECT_AF		(_AT(pmdval_t, 1) << 6)
#define PMD_DSECT_SPLITTING	(_AT(pmdval_t, 1) << 7)

#define PMD_BIT_FUNC(fn,op) \
static inline pmd_t pmd_##fn(pmd_t pmd) { pmd_val(pmd) op; return pmd; }

static inline void set_pmd_at(struct mm_struct *mm, unsigned long addr,
				pmd_t *pmdp, pmd_t pmd)
{
	/*
	 * we can sometimes be passed a pmd pointing to a level 2 descriptor
	 * from collapse_huge_page.
	 */
	if ((pmd_val(pmd) & PMD_TYPE_MASK) == PMD_TYPE_TABLE) {
		pmdp[0] = __pmd(pmd_val(pmd));
		pmdp[1] = __pmd(pmd_val(pmd) + 256 * sizeof(pte_t));
	} else {
		pmdp[0] = __pmd(pmd_val(pmd));			/* first 1M section  */
		pmdp[1] = __pmd(pmd_val(pmd) + SECTION_SIZE);	/* second 1M section */
	}

	flush_pmd_entry(pmdp);
}

extern pmdval_t arm_hugepmdprotval;
extern pteval_t arm_hugepteprotval;

#define pmd_mkhuge(pmd)		(__pmd((pmd_val(pmd) & ~PMD_TYPE_MASK) | PMD_TYPE_SECT))

PMD_BIT_FUNC(mkold, &= ~PMD_DSECT_AF);
PMD_BIT_FUNC(mkdirty, |= PMD_DSECT_DIRTY);
PMD_BIT_FUNC(mkclean, &= ~PMD_DSECT_DIRTY);
PMD_BIT_FUNC(mkyoung, |= PMD_DSECT_AF);
PMD_BIT_FUNC(mkwrite, |= PMD_SECT_AP_WRITE);
PMD_BIT_FUNC(wrprotect,	&= ~PMD_SECT_AP_WRITE);
PMD_BIT_FUNC(mknotpresent, &= ~PMD_TYPE_MASK);
PMD_BIT_FUNC(mkexec,	&= ~PMD_SECT_XN);
PMD_BIT_FUNC(mknexec,	|= PMD_SECT_XN);

#define pmd_young(pmd)			(pmd_val(pmd) & PMD_DSECT_AF)
#define pmd_write(pmd)			(pmd_val(pmd) & PMD_SECT_AP_WRITE)
#define pmd_exec(pmd)			(!(pmd_val(pmd) & PMD_SECT_XN))
#define pmd_dirty(pmd)			(pmd_val(pmd) & PMD_DSECT_DIRTY)

#define __HAVE_ARCH_PMD_WRITE

#define pmd_modify(pmd, prot)							\
({										\
	pmd_t pmdret = __pmd((pmd_val(pmd) & (PMD_MASK | PMD_DOMAIN_MASK)) 	\
		| arm_hugepmdprotval);						\
	pgprot_t inprot = prot;							\
	pte_t newprot = __pte(pgprot_val(inprot));				\
										\
	if (pte_dirty(newprot))							\
		pmdret = pmd_mkdirty(pmdret);					\
	else									\
		pmdret = pmd_mkclean(pmdret);					\
										\
	if (pte_exec(newprot))							\
		pmdret = pmd_mkexec(pmdret);					\
	else									\
		pmdret = pmd_mknexec(pmdret);					\
										\
	if (pte_write(newprot))							\
		pmdret = pmd_mkwrite(pmdret);					\
	else									\
		pmdret = pmd_wrprotect(pmdret);					\
										\
	if (pte_young(newprot))							\
		pmdret = pmd_mkyoung(pmdret);					\
	else									\
		pmdret = pmd_mkold(pmdret);					\
										\
	pmdret;									\
})

#else
#define HPAGE_SIZE 0
#endif /* CONFIG_SYS_SUPPORTS_HUGETLBFS */

#ifdef CONFIG_TRANSPARENT_HUGEPAGE
#define pmd_mkhuge(pmd)		(__pmd((pmd_val(pmd) & ~PMD_TYPE_MASK) | PMD_TYPE_SECT))

PMD_BIT_FUNC(mksplitting, |= PMD_DSECT_SPLITTING);
#define pmd_trans_splitting(pmd)	(pmd_val(pmd) & PMD_DSECT_SPLITTING)
#define pmd_trans_huge(pmd)		((pmd_val(pmd) & PMD_TYPE_MASK) == PMD_TYPE_SECT)

static inline unsigned long pmd_pfn(pmd_t pmd)
{
	/*
	 * for a section, we need to mask off more of the pmd
	 * before looking up the pfn
	 */
	if ((pmd_val(pmd) & PMD_TYPE_MASK) == PMD_TYPE_SECT)
		return __phys_to_pfn(pmd_val(pmd) & HPAGE_MASK);
	else
		return __phys_to_pfn(pmd_val(pmd) & PHYS_MASK);
}

#define pfn_pmd(pfn,prot) pmd_modify(__pmd(__pfn_to_phys(pfn)),prot);
#define mk_pmd(page,prot) pfn_pmd(page_to_pfn(page),prot);

static inline int has_transparent_hugepage(void)
{
	return 1;
}

#define _PMD_HUGE(pmd) ((pmd_val(pmd) & PMD_TYPE_MASK) == PMD_TYPE_SECT)
#define _PMD_HPAGE(pmd) (phys_to_page(pmd_val(pmd) & HPAGE_MASK))
#else
#define _PMD_HUGE(pmd) (0)
#define _PMD_HPAGE(pmd) (0)
#endif /* CONFIG_TRANSPARENT_HUGEPAGE */

static inline struct page *pmd_page(pmd_t pmd)
{
	/*
	 * for a section, we need to mask off more of the pmd
	 * before looking up the page as it is a section descriptor.
	 */
	if (_PMD_HUGE(pmd))
		return _PMD_HPAGE(pmd);

	return phys_to_page(pmd_val(pmd) & PHYS_MASK);
}

#endif /* __ASSEMBLY__ */

#endif /* _ASM_PGTABLE_2LEVEL_H */
