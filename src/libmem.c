/*
 * Copyright (C) 2025 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* Sierra release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

// #ifdef MM_PAGING
/*
 * System Library
 * Memory Module Library libmem.c 
 */

#include "string.h"
#include "mm.h"
#include "syscall.h"
#include "libmem.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>


// #define syscall os_syscall 

static pthread_mutex_t mmvm_lock = PTHREAD_MUTEX_INITIALIZER;

/*enlist_vm_freerg_list - add new rg to freerg_list
 *@mm: memory region
 *@rg_elmt: new region
 *
 */
int enlist_vm_freerg_list(struct mm_struct *mm, struct vm_rg_struct *rg_elmt)
{
  struct vm_rg_struct *rg_node = mm->mmap->vm_freerg_list;

  if (rg_elmt->rg_start >= rg_elmt->rg_end)
    return -1;

  if (rg_node != NULL)
    rg_elmt->rg_next = rg_node;

  /* Enlist the new region */
  mm->mmap->vm_freerg_list = rg_elmt;

  return 0;
}

/*get_symrg_byid - get mem region by region ID
 *@mm: memory region
 *@rgid: region ID act as symbol index of variable
 *
 */
struct vm_rg_struct *get_symrg_byid(struct mm_struct *mm, int rgid)
{
  if (rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ)
    return NULL;
  
  return &mm->symrgtbl[rgid];
}

/*__alloc - allocate a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *@alloc_addr: address of allocated memory region
 *
 */
int __alloc(struct pcb_t *caller, int vmaid, int rgid, int size, int *alloc_addr)
{
  /* Lock the memory */
  pthread_mutex_lock(&mmvm_lock);
  struct vm_rg_struct rgnode;

  /* First try to allocate from existing free regions */
  if (get_free_vmrg_area(caller, vmaid, size, &rgnode) == 0)
  {
    /* Track allocation in symbol table */
    if (caller->mm->symrgtbl[rgid].rg_start == 0) {
      /* First allocation for this symbol */
      caller->mm->symrgtbl[rgid].rg_start = rgnode.rg_start;
      caller->mm->symrgtbl[rgid].rg_end = rgnode.rg_start + size;
      caller->mm->symrgtbl[rgid].rg_next = NULL;
    } else {
      /* Symbol already has memory, create linked region node */
      struct vm_rg_struct *new_rg = malloc(sizeof(struct vm_rg_struct));
      if (new_rg == NULL) {
        pthread_mutex_unlock(&mmvm_lock);
        return -1;
      }
      
      /* Set up new region */
      new_rg->rg_start = rgnode.rg_start;
      new_rg->rg_end = rgnode.rg_end;
      
      /* Link to existing regions */
      new_rg->rg_next = caller->mm->symrgtbl[rgid].rg_next;
      caller->mm->symrgtbl[rgid].rg_next = new_rg;
    }
    
    if (rgnode.rg_end >= caller->mm->mmap->sbrk ) {
      *alloc_addr = caller->mm->mmap->sbrk ; 
      caller->mm->mmap->sbrk = caller->mm->mmap->sbrk + size; // Update sbrk
    }
    else{
      *alloc_addr = rgnode.rg_start;
    }
    pthread_mutex_unlock(&mmvm_lock);
    return 0;
  }

  /* No suitable free region found, increase memory limit */
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
  if (cur_vma == NULL) {
    pthread_mutex_unlock(&mmvm_lock);
    return -1;
  }
  
  /* Calculate required size and get old break */
  int inc_sz = PAGING_PAGE_ALIGNSZ(size);
  int old_sbrk = cur_vma->sbrk;

  /* Invoke syscall to increase memory limit */
  struct sc_regs regs;
  regs.a1 = SYSMEM_INC_OP;
  regs.a2 = vmaid;
  regs.a3 = inc_sz;
  // regs.a3 = size;
  int syscall_result = os_syscall(caller, 17, &regs);

  if (syscall_result < 0) {
    pthread_mutex_unlock(&mmvm_lock);
    return -1;
  }

  /* After increasing limit, try again to get free region */
  if (get_free_vmrg_area(caller, vmaid, size, &rgnode) == 0) {
    /* Track allocation in symbol table */
    if (caller->mm->symrgtbl[rgid].rg_start == 0) {
      /* First allocation for this symbol */
      caller->mm->symrgtbl[rgid].rg_start = rgnode.rg_start;
      caller->mm->symrgtbl[rgid].rg_end = rgnode.rg_start + size ;
      caller->mm->symrgtbl[rgid].rg_next = NULL;
    } else {
      /* Symbol already has memory, create linked region node */
      struct vm_rg_struct *new_rg = malloc(sizeof(struct vm_rg_struct));
      if (new_rg == NULL) {
        pthread_mutex_unlock(&mmvm_lock);
        return -1;
      }
      
      /* Set up new region */
      new_rg->rg_start = rgnode.rg_start;
      new_rg->rg_end = rgnode.rg_start + size;
      
      /* Link to existing regions */
      new_rg->rg_next = caller->mm->symrgtbl[rgid].rg_next;
      caller->mm->symrgtbl[rgid].rg_next = new_rg;
    }
    if (rgnode.rg_end > caller->mm->mmap->sbrk){
      caller->mm->mmap->sbrk = old_sbrk + size; // Update sbrk
      *alloc_addr = old_sbrk;
    }
    else {
      *alloc_addr = rgnode.rg_start;
    }
  } else {
    /* Allocation failed even after expanding memory */
    pthread_mutex_unlock(&mmvm_lock);
    return -1;
  }

  pthread_mutex_unlock(&mmvm_lock);
  return 0;
}

/*__free - remove a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *
 */
int __free(struct pcb_t *caller, int vmaid, int rgid)
{
  //struct vm_rg_struct rgnode;

  pthread_mutex_lock(&mmvm_lock);
  // Dummy initialization for avoding compiler dummay warning
  // in incompleted TODO code rgnode will overwrite through implementing
  // the manipulation of rgid later

  if(rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ){
    pthread_mutex_unlock(&mmvm_lock);
    return -1;
  }
  /* TODO: Manage the collect freed region to freerg_list */
  struct vm_rg_struct * rgnode = malloc(sizeof(struct vm_rg_struct));
  if (rgnode == NULL ){
    pthread_mutex_unlock(&mmvm_lock);
    return -1;
  }

  /* Get region info from symbol table */
  rgnode->rg_start = caller->mm->symrgtbl[rgid].rg_start;
  rgnode->rg_end = caller->mm->symrgtbl[rgid].rg_end;
  rgnode->rg_next = NULL;

  caller->mm->symrgtbl[rgid].rg_start = 0;
  caller->mm->symrgtbl[rgid].rg_end = 0;
  /*enlist the obsoleted memory region */

  enlist_vm_freerg_list(caller->mm, rgnode);

  struct vm_rg_struct *next_rg = caller->mm->symrgtbl[rgid].rg_next;
  while (next_rg != NULL) {
    struct vm_rg_struct *temp = next_rg;
    next_rg = next_rg->rg_next;
    
    /* Create new free region node */
    struct vm_rg_struct *free_rg = malloc(sizeof(struct vm_rg_struct));
    if (free_rg != NULL) {
      free_rg->rg_start = temp->rg_start;
      free_rg->rg_end = temp->rg_end;
      free_rg->rg_next = NULL;
      
      /* Add to free list */
      enlist_vm_freerg_list(caller->mm, free_rg);
    }
    
    /* Free the linked region node */
    free(temp);
  }

  caller->mm->symrgtbl[rgid].rg_start = 0;
  caller->mm->symrgtbl[rgid].rg_end = 0;
  caller->mm->symrgtbl[rgid].rg_next = NULL;

  //enlist_vm_freerg_list();
  pthread_mutex_unlock(&mmvm_lock);

  return 0;
}

/*liballoc - PAGING-based allocate a region memory
 *@proc:  Process executing the instruction
 *@size: allocated size
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int liballoc(struct pcb_t *proc, uint32_t size, uint32_t reg_index)
{

  /* TODO Implement allocation on vm area 0 */
  int addr;
    
  /* By default using vmaid = 0 */
  __alloc(proc, 0, reg_index, size, &addr);

  printf("===== PHYSICAL MEMORY AFTER ALLOCATION =====\n");
  
  printf("PID=%d - Region=%d - Address=%08X - Size=%d byte\n",proc->pid, reg_index, addr, size);  
  print_pgtbl(proc, 0, -1); //print max TBL
  return addr;
  
}

/*libfree - PAGING-based free a region memory
 *@proc: Process executing the instruction
 *@size: allocated size
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */

int libfree(struct pcb_t *proc, uint32_t reg_index)
{
  /* TODO Implement free region */

  /* By default using vmaid = 0 */
  
    __free(proc, 0, reg_index);
    printf("===== PHYSICAL MEMORY AFTER DEALLOCATION =====\n");
    printf("PID=%d - Region=%d \n",proc->pid, reg_index);  
    print_pgtbl(proc, 0, -1); //print max TBL
    return 0;
}

/*pg_getpage - get the page in ram
 *@mm: memory region
 *@pagenum: PGN
 *@framenum: return FPN
 *@caller: caller
 *
 */
int pg_getpage(struct mm_struct *mm, int pgn, int *fpn, struct pcb_t *caller)
{
  uint32_t pte = mm->pgd[pgn];

  if (!PAGING_PAGE_PRESENT(pte))
  { /* Page is not online, make it actively living */
    int vicpgn, swpfpn; 
    int vicfpn;
    uint32_t vicpte;

    int tgtfpn = PAGING_PTE_SWP(pte);//the target frame storing our variable

    /* TODO: Play with your paging theory here */
    /* Find victim page */
    find_victim_page(caller->mm, &vicpgn);


    vicpte = caller->mm->pgd[vicpgn];
    vicfpn = PAGING_FPN(vicpte);

    /* Get free frame in MEMSWP */
    MEMPHY_get_freefp(caller->active_mswp, &swpfpn);

    /* TODO: Implement swap frame from MEMRAM to MEMSWP and vice versa*/

    /* Swap out: Copy victim frame from RAM to swap*/
    struct sc_regs regs;
    regs.a1 = SYSMEM_SWP_OP;
    regs.a2 = vicfpn;
    regs.a3 = swpfpn;
    syscall(caller, 17, &regs);

    /* Swap in : Copy target frame from swap to RAM */
    regs.a1 = SYSMEM_SWP_OP;
    regs.a2 = tgtfpn;
    regs.a3 = vicfpn;

    syscall (caller, 17, &regs);
    
    /*Update page table: mark victim as swapped*/
    pte_set_swap(&caller->mm->pgd[vicpgn], 0, swpfpn);
    /*Update page table: mark target as present*/
    pte_set_fpn(&caller->mm->pgd[pgn], vicfpn);

    PAGING_PTE_SET_PRESENT(caller->mm->pgd[pgn]);

    enlist_pgn_node(&caller->mm->fifo_pgn,pgn);
  }


  *fpn = PAGING_FPN(mm->pgd[pgn]);

  return 0;
}

/*pg_getval - read value at given offset
 *@mm: memory region
 *@addr: virtual address to acess
 *@value: value
 *
 */
int pg_getval(struct mm_struct *mm, int addr, BYTE *data, struct pcb_t *caller)
{
  int pgn = PAGING_PGN(addr);
  //int off = PAGING_OFFST(addr);
  int fpn;
  int off = PAGING_OFFST(addr);

  /* Get the page to MEMRAM, swap from MEMSWAP if needed */
  if (pg_getpage(mm, pgn, &fpn, caller) != 0)
    return -1; /* invalid page access */

  int phyaddr = (fpn << PAGING_ADDR_PGN_LOBIT) + off;

  /* TODO 
   *  MEMPHY_read(caller->mram, phyaddr, data);
   *  MEMPHY READ 
   *  SYSCALL 17 sys_memmap with SYSMEM_IO_READ
   */
  // int phyaddr
  struct sc_regs regs;
  regs.a1 = SYSMEM_IO_READ;
  regs.a2 = phyaddr;
  regs.a3 = 0;

  syscall(caller, 17, &regs);

  /* SYSCALL 17 sys_memmap */

  // Update data
  *data = (BYTE)regs.a3;

  return 0;
}

/*pg_setval - write value to given offset
 *@mm: memory region
 *@addr: virtual address to acess
 *@value: value
 *
 */
int pg_setval(struct mm_struct *mm, int addr, BYTE value, struct pcb_t *caller)
{
  int pgn = PAGING_PGN(addr);
  int off = PAGING_OFFST(addr);
  int fpn;

  /* Get the page to MEMRAM, swap from MEMSWAP if needed */
  if (pg_getpage(mm, pgn, &fpn, caller) != 0)
    return -1; /* invalid page access */

  int phyaddr = (fpn << PAGING_ADDR_PGN_LOBIT) + off; 
  /* TODO
   *  MEMPHY_write(caller->mram, phyaddr, value);
   *  MEMPHY WRITE
   *  SYSCALL 17 sys_memmap with SYSMEM_IO_WRITE
   */
  // int phyaddr
  struct sc_regs regs;
  regs.a1 = SYSMEM_IO_WRITE;
  regs.a2 = phyaddr;
  regs.a3 = value;

  syscall(caller, 17, &regs);
  /* SYSCALL 17 sys_memmap */

  // Update data
  // data = (BYTE) 

  return 0;
}

/*__read - read value in region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@offset: offset to acess in memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *
 */
int __read(struct pcb_t *caller, int vmaid, int rgid, int offset, BYTE *data)
{
  struct vm_rg_struct *currg = get_symrg_byid(caller->mm, rgid);
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  if (currg == NULL || cur_vma == NULL) /* Invalid memory identify */
    return -1;

  pg_getval(caller->mm, currg->rg_start + offset, data, caller);

  return 0;
}

/*libread - PAGING-based read a region memory */
int libread(
    struct pcb_t *proc, // Process executing the instruction
    uint32_t source,    // Index of source register
    uint32_t offset,    // Source address = [source] + [offset]
    uint32_t* destination)
{
  BYTE data;
  int val = __read(proc, 0, source, offset, &data);

  /* TODO update result of reading action*/
    *destination = data;
  //destination 
#ifdef IODUMP
  printf("read region=%d offset=%d value=%d\n", source, offset, data);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif

  return val;
}

/*__write - write a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@offset: offset to acess in memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *
 */
int __write(struct pcb_t *caller, int vmaid, int rgid, int offset, BYTE value)
{
  struct vm_rg_struct *currg = get_symrg_byid(caller->mm, rgid);
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  if (currg == NULL || cur_vma == NULL) /* Invalid memory identify */
    return -1;

  pg_setval(caller->mm, currg->rg_start + offset, value, caller);

  return 0;
}

/*libwrite - PAGING-based write a region memory */
int libwrite(
    struct pcb_t *proc,   // Process executing the instruction
    BYTE data,            // Data to be wrttien into memory
    uint32_t destination, // Index of destination register
    uint32_t offset)
{
  __write(proc, 0, destination, offset, data);
  printf("===== PHYSICAL MEMORY AFTER WRITING =====\n");
#ifdef IODUMP
  printf("write region=%d offset=%d value=%d\n", destination, offset, data);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif

  return  0;
}

/*free_pcb_memphy - collect all memphy of pcb
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@incpgnum: number of page
 */
int free_pcb_memph(struct pcb_t *caller)
{
  int pagenum, fpn;
  uint32_t pte;


  for(pagenum = 0; pagenum < PAGING_MAX_PGN; pagenum++)
  {
    pte= caller->mm->pgd[pagenum];

    if (!PAGING_PAGE_PRESENT(pte))
    {
      fpn = PAGING_PTE_FPN(pte);
      MEMPHY_put_freefp(caller->mram, fpn);
    } else {
      fpn = PAGING_PTE_SWP(pte);
      MEMPHY_put_freefp(caller->active_mswp, fpn);    
    }
  }

  return 0;
}


/*find_victim_page - find victim page
 *@caller: caller
 *@pgn: return page number
 *
 */
int find_victim_page(struct mm_struct *mm, int *retpgn)
{
  struct pgn_t *pg = mm->fifo_pgn;

  if (pg == NULL){
    *retpgn = 0;
    return -1;
  }
  while (pg->pg_next != NULL){
    pg = pg->pg_next;
  }

  /* TODO: Implement the theorical mechanism to find the victim page */

  /*Get the first page in FIFO queue*/
  *retpgn = pg->pgn;

  /*Remove from FIFO queue*/
  // mm->fifo_pgn = pg->pg_next;

  free(pg);

  return 0;
}

// int find_victim_page(struct mm_struct *mm, int *retpgn)
//  {
//    struct pgn_t *pg = mm->fifo_pgn;
//    struct pgn_t *prev = NULL;
 
//    /* TODO: Implement the theorical mechanism to find the victim page */
//    while(pg->pg_next != NULL){
//      prev = pg;
//      pg = pg->pg_next;
//    }
//    *retpgn = pg->pgn;
//    if(prev == NULL)
//      mm->fifo_pgn = NULL;
//    else
//      prev->pg_next = NULL;
//    free(pg);
 
//    return 0;
//  }









/*get_free_vmrg_area - get a free vm regxion
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@size: allocated size
 *
 */
int get_free_vmrg_area(struct pcb_t *caller, int vmaid, int size, struct vm_rg_struct *newrg)
{
  int aligned_size = PAGING_PAGE_ALIGNSZ(size);

  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  if (cur_vma == NULL){
    return -1;
  }

  struct vm_rg_struct *rgit = cur_vma->vm_freerg_list;

  if (rgit == NULL){
    return -1;
  }

  /* Probe unintialized newrg */
  newrg->rg_start = newrg->rg_end = -1;

  /* TODO Traverse on list of free vm region to find a fit space */
  while (rgit != NULL){
    if (rgit -> rg_end - rgit-> rg_start >= size ){
      /*Found a region large enough*/
      newrg->rg_start = rgit->rg_start;
      newrg->rg_end = rgit->rg_start + size;
      
      /*Update the free region*/

      if (rgit->rg_end - rgit->rg_start == size){
        cur_vma->vm_freerg_list = rgit->rg_next;
        free(rgit);
      }
      else {
        rgit->rg_start = rgit->rg_start + size;
      }
      return 0;
    }

    rgit=rgit->rg_next;
  }

  return -1;
}

//#endif
