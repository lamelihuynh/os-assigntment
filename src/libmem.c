// /*
//  * Copyright (C) 2025 pdnguyen of HCMC University of Technology VNU-HCM
//  */

// /* Sierra release
//  * Source Code License Grant: The authors hereby grant to Licensee
//  * personal permission to use and modify the Licensed Source Code
//  * for the sole purpose of studying while attending the course CO2018.
//  */

// // #ifdef MM_PAGING
// /*
//  * System Library
//  * Memory Module Library libmem.c 
//  */

// #include "string.h"
// #include "mm.h"
// #include "syscall.h"
// #include "libmem.h"
// #include <stdlib.h>
// #include <stdio.h>
// #include <pthread.h>

// static pthread_mutex_t mmvm_lock = PTHREAD_MUTEX_INITIALIZER;

// /*enlist_vm_freerg_list - add new rg to freerg_list
//  *@mm: memory region
//  *@rg_elmt: new region
//  *
//  */
// int enlist_vm_freerg_list(struct mm_struct *mm, struct vm_rg_struct *rg_elmt)
// {
//   struct vm_rg_struct *rg_node = mm->mmap->vm_freerg_list;

//   if (rg_elmt->rg_start >= rg_elmt->rg_end)
//     return -1;

//   if (rg_node != NULL)
//     rg_elmt->rg_next = rg_node;

//   /* Enlist the new region */
//   mm->mmap->vm_freerg_list = rg_elmt;

//   return 0;
// }

// /*get_symrg_byid - get mem region by region ID
//  *@mm: memory region
//  *@rgid: region ID act as symbol index of variable
//  *
//  */
// struct vm_rg_struct *get_symrg_byid(struct mm_struct *mm, int rgid)
// {
//   if (rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ)
//     return NULL;

//   return &mm->symrgtbl[rgid];
// }

// /*__alloc - allocate a region memory
//  *@caller: caller
//  *@vmaid: ID vm area to alloc memory region
//  *@rgid: memory region ID (used to identify variable in symbole table)
//  *@size: allocated size
//  *@alloc_addr: address of allocated memory region
//  *
//  */



// int __alloc(struct pcb_t *caller, int vmaid, int rgid, int size, int *alloc_addr)
// {
//   /* Validate input parameters */
//   if (!caller || !caller->mm || !alloc_addr || size <= 0 ||
//       rgid < 0 || rgid >= PAGING_MAX_SYMTBL_SZ)
//   {
//     *alloc_addr = 0;
//     return -1;
//   }

//   pthread_mutex_lock(&mmvm_lock);

//   struct vm_rg_struct rgnode;
//   struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

//   if (!cur_vma)
//   {
//     *alloc_addr = 0;
//     pthread_mutex_unlock(&mmvm_lock);
//     return -2;
//   }

//   /* Try to find free space in existing regions */
//   if (get_free_vmrg_area(caller, vmaid, size, &rgnode) == 0)
//   {
//     caller->mm->symrgtbl[rgid].rg_start = rgnode.rg_start;
//     caller->mm->symrgtbl[rgid].rg_end = rgnode.rg_end;
//     *alloc_addr = rgnode.rg_start;

//     pthread_mutex_unlock(&mmvm_lock);
//     return 256;
//   }

//   /* Need to increase VMA limit */
//   int inc_sz = PAGING_PAGE_ALIGNSZ(size);
//   int old_sbrk = cur_vma->sbrk;

//   /* Try to increase VMA limit */
//   if (inc_vma_limit(caller, vmaid, inc_sz) != 0)
//   {
//     *alloc_addr = 0;
//     pthread_mutex_unlock(&mmvm_lock);
//     return -3;
//   }

//   /* Store allocation in symrgtbl */
//   caller->mm->symrgtbl[rgid].rg_start = old_sbrk;
//   caller->mm->symrgtbl[rgid].rg_end = old_sbrk + size;
//   *alloc_addr = old_sbrk;

//   /* Update free region list if we allocated more than needed */
//   if (inc_sz > size)
//   {
//     struct vm_rg_struct *new_rgnode = malloc(sizeof(struct vm_rg_struct));
//     if (!new_rgnode)
//     {
//       *alloc_addr = 0;
//       pthread_mutex_unlock(&mmvm_lock);
//       return -4;
//     }
//     new_rgnode->rg_start = old_sbrk + size;
//     new_rgnode->rg_end = old_sbrk + inc_sz;
//     if (enlist_vm_freerg_list(caller->mm, new_rgnode) != 0)
//     {
//       free(new_rgnode);
//       *alloc_addr = 0;
//       pthread_mutex_unlock(&mmvm_lock);
//       return -5;
//     }
//   }

//   /* Update VMA */
//   cur_vma->sbrk = old_sbrk + inc_sz;

//   pthread_mutex_unlock(&mmvm_lock);
//   return 0;
// }



// /*__free - remove a region memory
//  *@caller: caller
//  *@vmaid: ID vm area to alloc memory region
//  *@rgid: memory region ID (used to identify variable in symbole table)
//  *@size: allocated size 
//  *
//  */
// int __free(struct pcb_t *caller, int vmaid, int rgid)
// {
//   /* Validate input parameters */
//   if (!caller || !caller->mm || rgid < 0 || rgid >= PAGING_MAX_SYMTBL_SZ)
//   {
//     return -1;
//   }

//   pthread_mutex_lock(&mmvm_lock);

//   struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
//   if (!cur_vma)
//   {
//     pthread_mutex_unlock(&mmvm_lock);
//     return -2; // Invalid VMA
//   }

//   struct vm_rg_struct *freed_region = &caller->mm->symrgtbl[rgid];
//   if (!freed_region || freed_region->rg_start >= freed_region->rg_end)
//   {
//     pthread_mutex_unlock(&mmvm_lock);
//     return -3; // Invalid region
//   }

//   /* Create a new free region node */
//   struct vm_rg_struct *new_region = malloc(sizeof(struct vm_rg_struct));
//   if (!new_region)
//   {
//     pthread_mutex_unlock(&mmvm_lock);
//     return -4; // Out of memory
//   }

//   new_region->rg_start = freed_region->rg_start;
//   new_region->rg_end = freed_region->rg_end;

//   if (enlist_vm_freerg_list(caller->mm, new_region) != 0)
//   {
//     free(new_region);
//     pthread_mutex_unlock(&mmvm_lock);
//     return -5; // Failed to enlist region
//   }

//   /* Reset the freed region in symrgtbl */
//   freed_region->rg_start = 0;
//   freed_region->rg_end = 0;
//   freed_region->rg_next = NULL;

//   pthread_mutex_unlock(&mmvm_lock);
//   return 0;
// }


// /*liballoc - PAGING-based allocate a region memory
//  *@proc:  Process executing the instruction
//  *@size: allocated size
//  *@reg_index: memory region ID (used to identify variable in symbole table)
//  */
// int liballoc(struct pcb_t *proc, uint32_t size, uint32_t reg_index)
// {
//   /* TODO Implement allocation on vm area 0 */
//   int addr;

//   /* By default using vmaid = 0 */
//    __alloc(proc, 0, reg_index, size, &addr);
//    return addr;
// }

// /*libfree - PAGING-based free a region memory
//  *@proc: Process executing the instruction
//  *@size: allocated size
//  *@reg_index: memory region ID (used to identify variable in symbole table)
//  */

// int libfree(struct pcb_t *proc, uint32_t reg_index)
// {
//   /* TODO Implement free region */

//   /* By default using vmaid = 0 */
//   return __free(proc, 0, reg_index);
// }

// /*pg_getpage - get the page in ram
//  *@mm: memory region
//  *@pagenum: PGN
//  *@framenum: return FPN
//  *@caller: caller
//  *
//  */
// int pg_getpage(struct mm_struct *mm, int pgn, int *fpn, struct pcb_t *caller)
// {
//   uint32_t pte = mm->pgd[pgn];

//   if (!PAGING_PAGE_PRESENT(pte))
//   { /* Page is not online, make it actively living */
//     int vicpgn, swpfpn; 
//     int vicfpn;
//     uint32_t vicpte;

//     int tgtfpn = PAGING_PTE_SWP(pte);//the target frame storing our variable

//     /* TODO: Play with your paging theory here */
//     /* Find victim page */
//     find_victim_page(caller->mm, &vicpgn);

//     /* Get free frame in MEMSWP */
//     MEMPHY_get_freefp(caller->active_mswp, &swpfpn);

//     /* TODO: Implement swap frame from MEMRAM to MEMSWP and vice versa*/
//     vicfpn = PAGING_FPN(mm->pgd[vicpgn]);
//     /* TODO copy victim frame to swap 
//      * SWP(vicfpn <--> swpfpn)
//      * SYSCALL 17 sys_memmap 
//      * with operation SYSMEM_SWP_OP
//      */
//     struct sc_regs regs;
//     regs.a1 = SYSMEM_SWP_OP;
//     regs.a2 = vicfpn;
//     regs.a3 = swpfpn;
//     syscall(caller, 17, &regs);

//     /* SYSCALL 17 sys_memmap */

//     /* TODO copy target frame form swap to mem 
//      * SWP(tgtfpn <--> vicfpn)
//      * SYSCALL 17 sys_memmap
//      * with operation SYSMEM_SWP_OP
//      */
//     /* TODO copy target frame form swap to mem 
//     //regs.a1 =...
//     //regs.a2 =...
//     //regs.a3 =..
//     */
   
//    regs.a2 = tgtfpn;
//    regs.a3 = vicfpn;
//    syscall(caller, 17, &regs);

//     /* SYSCALL 17 sys_memmap */

//     /* Update page table */
//     //pte_set_swap() 
//     //mm->pgd;

//     /* Update its online status of the target page */
//     //pte_set_fpn() &
//     //mm->pgd[pgn];
//     //pte_set_fpn();
//     pte_set_swap(&mm->pgd[vicpgn], 0, swpfpn);
//     pte_set_fpn(&mm->pgd[pgn], vicfpn);

//     enlist_pgn_node(&caller->mm->fifo_pgn,pgn);
//   }

//   *fpn = PAGING_FPN(mm->pgd[pgn]);

//   return 0;
// }

// /*pg_getval - read value at given offset
//  *@mm: memory region
//  *@addr: virtual address to acess
//  *@value: value
//  *
//  */
// int pg_getval(struct mm_struct *mm, int addr, BYTE *data, struct pcb_t *caller)
// { 
  
//   int pgn = PAGING_PGN(addr);
//   int off = PAGING_OFFST(addr);
//   int fpn;

//   /* Get the page to MEMRAM, swap from MEMSWAP if needed */
//   if (pg_getpage(mm, pgn, &fpn, caller) != 0)
//     return -1; /* invalid page access */

//   /* TODO 
//    *  MEMPHY_read(caller->mram, phyaddr, data);
//    *  MEMPHY READ 
//    *  SYSCALL 17 sys_memmap with SYSMEM_IO_READ
//    */
//    int phyaddr = (fpn << PAGING_ADDR_PGN_LOBIT) + off;

//    struct sc_regs regs;
//    regs.a1 = SYSMEM_IO_READ; // Thao tác đọc IO
//    regs.a2 = phyaddr;        // Địa chỉ vật lý cần đọc
//    regs.a3 = 0;             // Đối số 2: Giá trị ghi (không sử dụng trong đọc) 
//    syscall(caller, 17, &regs);  // call syscall #17 (memmap)
//     *data = (BYTE)regs.a2;
//   return 0;
// }

// /*pg_setval - write value to given offset
//  *@mm: memory region
//  *@addr: virtual address to acess
//  *@value: value
//  *
//  */
// int pg_setval(struct mm_struct *mm, int addr, BYTE value, struct pcb_t *caller)
// {
//   int pgn = PAGING_PGN(addr);
//   int off = PAGING_OFFST(addr);
//   int fpn;
//   int syscall_ret;

//   /* Get the page to MEMRAM, swap from MEMSWAP if needed */
//   if (pg_getpage(mm, pgn, &fpn, caller) != 0)
//     return -1; /* invalid page access */

//   /* TODO
//    *  MEMPHY_write(caller->mram, phyaddr, value);
//    *  MEMPHY WRITE
//    *  SYSCALL 17 sys_memmap with SYSMEM_IO_WRITE
//    */
//   int phyaddr = (fpn << PAGING_ADDR_FPN_LOBIT) + off;
//   struct sc_regs regs;
//   regs.a1 = SYSMEM_IO_WRITE; // Mã thao tác: Ghi IO
//   regs.a2 = phyaddr;         // Đối số 1: Địa chỉ vật lý
//   regs.a3 = value;           // Đối số 2: Giá trị ghi

//   /* SYSCALL 17 sys_memmap */
//   syscall_ret = syscall(caller, 17, &regs);
//   if (syscall_ret < 0) {
//     return -1; // Error in syscall
//   }
//   // Update data
//   // data = (BYTE) 
//   return 0;
// }

// /*__read - read value in region memory
//  *@caller: caller
//  *@vmaid: ID vm area to alloc memory region
//  *@offset: offset to acess in memory region
//  *@rgid: memory region ID (used to identify variable in symbole table)
//  *@size: allocated size
//  *
//  */
// int __read(struct pcb_t *caller, int vmaid, int rgid, int offset, BYTE *data)
// {
//   struct vm_rg_struct *currg = get_symrg_byid(caller->mm, rgid);
//   struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

//   if (currg == NULL || cur_vma == NULL) /* Invalid memory identify */
//     return -1;

//   pg_getval(caller->mm, currg->rg_start + offset, data, caller);

//   return 0;
// }

// /*libread - PAGING-based read a region memory */
// int libread(
//     struct pcb_t *proc, // Process executing the instruction
//     uint32_t source,    // Index of source register
//     uint32_t offset,    // Source address = [source] + [offset]
//     uint32_t* destination)
// {
//   BYTE data;
//   int val = __read(proc, 0, source, offset, &data);

//   /* TODO update result of reading action*/
//   //destination 
//   if( val == 0 && destination != NULL)
//   {
//     *destination = data;
//   }
// // #ifdef IODUMP
// //   printf("read region=%d offset=%d value=%d\n", source, offset, data);
// // #ifdef PAGETBL_DUMP
// //   print_pgtbl(proc, 0, -1); //print max TBL
// // #endif
// //   MEMPHY_dump(proc->mram);
// // #endif
// //   printf("finish libread\n");
//   return val;
// }

// /*__write - write a region memory
//  *@caller: caller
//  *@vmaid: ID vm area to alloc memory region
//  *@offset: offset to acess in memory region
//  *@rgid: memory region ID (used to identify variable in symbole table)
//  *@size: allocated size
//  *
//  */
// int __write(struct pcb_t *caller, int vmaid, int rgid, int offset, BYTE value)
// {
//   struct vm_rg_struct *currg = get_symrg_byid(caller->mm, rgid);
//   struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

//   if (currg == NULL || cur_vma == NULL) /* Invalid memory identify */
//     return -1;

//   pg_setval(caller->mm, currg->rg_start + offset, value, caller);

//   return 0;
// }

// /*libwrite - PAGING-based write a region memory */
// int libwrite(
//     struct pcb_t *proc,   // Process executing the instruction
//     BYTE data,            // Data to be wrttien into memory
//     uint32_t destination, // Index of destination register
//     uint32_t offset)
// {
// #ifdef IODUMP
//   printf("write region=%d offset=%d value=%d\n", destination, offset, data);
// #ifdef PAGETBL_DUMP
//   print_pgtbl(proc, 0, -1); //print max TBL
// #endif
//   MEMPHY_dump(proc->mram);
// #endif

//   return __write(proc, 0, destination, offset, data);
// }

// /*free_pcb_memphy - collect all memphy of pcb
//  *@caller: caller
//  *@vmaid: ID vm area to alloc memory region
//  *@incpgnum: number of page
//  */
// int free_pcb_memph(struct pcb_t *caller)
// {
//   int pagenum, fpn;
//   uint32_t pte;


//   for(pagenum = 0; pagenum < PAGING_MAX_PGN; pagenum++)
//   {
//     pte= caller->mm->pgd[pagenum];

//     if (!PAGING_PAGE_PRESENT(pte))
//     {
//       fpn = PAGING_PTE_FPN(pte);
//       MEMPHY_put_freefp(caller->mram, fpn);
//     } else {
//       fpn = PAGING_PTE_SWP(pte);
//       MEMPHY_put_freefp(caller->active_mswp, fpn);    
//     }
//   }

//   return 0;
// }


// /*find_victim_page - find victim page
//  *@caller: caller
//  *@pgn: return page number
//  *
//  */
// int find_victim_page(struct mm_struct *mm, int *retpgn)
// {
//   struct pgn_t *pg = mm->fifo_pgn;
//   struct pgn_t *prev = NULL;

//   /* TODO: Implement the theorical mechanism to find the victim page */
//   while(pg->pg_next != NULL){
//     prev = pg;
//     pg = pg->pg_next;
//   }
//   *retpgn = pg->pgn;
//   if(prev == NULL)
//     mm->fifo_pgn = NULL;
//   else
//     prev->pg_next = NULL;
//   free(pg);

//   return 0;
// }

// /*get_free_vmrg_area - get a free vm region
//  *@caller: caller
//  *@vmaid: ID vm area to alloc memory region
//  *@size: allocated size
//  *
//  */
// int get_free_vmrg_area(struct pcb_t *caller, int vmaid, int size, struct vm_rg_struct *newrg)
// {
//   struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

//   struct vm_rg_struct *rgit = cur_vma->vm_freerg_list;
//   struct vm_rg_struct *prev = NULL;


//   if (rgit == NULL)
//     return -1;

//   /* Probe unintialized newrg */
//   newrg->rg_start = newrg->rg_end = -1;

//   /* TODO Traverse on list of free vm region to find a fit space */
//   //while (...)
//   // ..
//   while (rgit != NULL)
//     {
//         int region_size = rgit->rg_end - rgit->rg_start;
//         if (region_size >= size)
//         {
//             newrg->rg_start = rgit->rg_start;
//             newrg->rg_end = rgit->rg_start + size;
//             if (region_size == size) {
//                 if (prev == NULL) cur_vma->vm_freerg_list = rgit->rg_next;
//                 else prev->rg_next = rgit->rg_next;
//                 free(rgit);
//             } else {
//                 rgit->rg_start += size;
//             }
//             // **Giải thích**: Duyệt danh sách vùng trống. Nếu tìm thấy vùng đủ lớn (>= size), cấp phát từ đầu vùng đó (cập nhật newrg). Nếu vùng trống bị dùng hết, xóa node khỏi danh sách và free. Nếu còn dư, cập nhật lại rg_start của node đó. Trả về 0.
//             return 0;
//         }
//         prev = rgit;
//         rgit = rgit->rg_next;
//     }
//   return -1;
// }

// //#endif






































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
   pthread_mutex_lock(&mmvm_lock);
 
   struct vm_rg_struct rgnode;
 
   if(get_free_vmrg_area(caller, vmaid, size, &rgnode) == 0)
   {
     caller->mm->symrgtbl[rgid].rg_start = rgnode.rg_start;
     caller->mm->symrgtbl[rgid].rg_end = rgnode.rg_end;
 
     *alloc_addr = rgnode.rg_start;
 
     pthread_mutex_unlock(&mmvm_lock);
     return 0;
   }
   /*Xử lí khi get_free_vmrg_area thất bại*/
   struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
   if (cur_vma == NULL){
     pthread_mutex_unlock(&mmvm_lock);
     return -1; // Không  tìm thấy VMA
   }
   /*Tăng giới hạn vùng nhớ*/
   int inc_sz = PAGING_PAGE_ALIGNSZ(size);
   int old_sbrk = cur_vma->sbrk;
   struct sc_regs regs;
   regs.a1 = SYSMEM_INC_OP; //Mã lệnh tăng giới hạn vùng nhớ
   regs.a2 = vmaid;  
   regs.a3 = inc_sz;
 
   /* SYSCALL 17 sys_memmap */
   int syscall_result = syscall(caller, 17, &regs);
   if (syscall_result < 0) {
     pthread_mutex_unlock(&mmvm_lock);
     return -1; // Lỗi trong syscall
   }
   /* Cập nhật địa chỉ đã cấp phát */
   if (get_free_vmrg_area(caller, vmaid, size, &rgnode) == 0) {
     caller->mm->symrgtbl[rgid].rg_start = rgnode.rg_start;
     caller->mm->symrgtbl[rgid].rg_end = rgnode.rg_end;
     *alloc_addr = rgnode.rg_start;
   }
   else {
       pthread_mutex_unlock(&mmvm_lock);
       return -1;
   }
   caller->mm->symrgtbl[rgid].rg_start = old_sbrk;
   caller->mm->symrgtbl[rgid].rg_end = old_sbrk + size;
   if( old_sbrk + size < cur_vma->vm_end)
   {
     struct vm_rg_struct * remain_rg = init_vm_rg(old_sbrk + size, cur_vma->vm_end);
     enlist_vm_freerg_list(caller->mm, remain_rg);
   }
   *alloc_addr = old_sbrk;
   cur_vma->sbrk = old_sbrk + size;
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
//  int __free(struct pcb_t *caller, int vmaid, int rgid)
//  {
//    pthread_mutex_lock(&mmvm_lock);
 
//    if(rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ){
//      pthread_mutex_unlock(&mmvm_lock);
//      return -1;
//    }
//    // Check if the region actually exists (has valid values)
//    if (caller->mm->symrgtbl[rgid].rg_start == 0 && 
//      caller->mm->symrgtbl[rgid].rg_end == 0) {
//    // Region doesn't exist or already freed
//    pthread_mutex_unlock(&mmvm_lock);
//    return -1;
//  }
 
//    struct vm_rg_struct *rgnode = malloc(sizeof(struct vm_rg_struct));
//    if (!rgnode) {
//      pthread_mutex_unlock(&mmvm_lock);
//      return -1;
//    }
 
//    rgnode->rg_start = caller->mm->symrgtbl[rgid].rg_start;
//    rgnode->rg_end = caller->mm->symrgtbl[rgid].rg_end;
//    rgnode->rg_next = NULL;
 
//    caller->mm->symrgtbl[rgid].rg_start = 0;
//    caller->mm->symrgtbl[rgid].rg_end = 0;
 
//    enlist_vm_freerg_list(caller->mm, rgnode);
//    pthread_mutex_unlock(&mmvm_lock);
 
//    return 0;
//  }

int __free(struct pcb_t *caller, int vmaid, int rgid)
{
  /* Validate input parameters */
  if (!caller || !caller->mm || rgid < 0 || rgid >= PAGING_MAX_SYMTBL_SZ)
  {
    return -1;
  }

  pthread_mutex_lock(&mmvm_lock);

  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
  if (!cur_vma)
  {
    pthread_mutex_unlock(&mmvm_lock);
    return -2; // Invalid VMA
  }

  struct vm_rg_struct *freed_region = &caller->mm->symrgtbl[rgid];
  if (!freed_region || freed_region->rg_start >= freed_region->rg_end)
  {
    pthread_mutex_unlock(&mmvm_lock);
    return -3; // Invalid region
  }

  /* Create a new free region node */
  struct vm_rg_struct *new_region = malloc(sizeof(struct vm_rg_struct));
  if (!new_region)
  {
    pthread_mutex_unlock(&mmvm_lock);
    return -4; // Out of memory
  }

  new_region->rg_start = freed_region->rg_start;
  new_region->rg_end = freed_region->rg_end;

  if (enlist_vm_freerg_list(caller->mm, new_region) != 0)
  {
    free(new_region);
    pthread_mutex_unlock(&mmvm_lock);
    return -5; // Failed to enlist region
  }

  /* Reset the freed region in symrgtbl */
  freed_region->rg_start = 0;
  freed_region->rg_end = 0;
  freed_region->rg_next = NULL;

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
 
     /* Get free frame in MEMSWP */
     MEMPHY_get_freefp(caller->active_mswp, &swpfpn);
 
     /* TODO: Implement swap frame from MEMRAM to MEMSWP and vice versa*/
     vicfpn = PAGING_FPN(mm->pgd[vicpgn]);
     /* TODO copy victim frame to swap 
      * SWP(vicfpn <--> swpfpn)
      * SYSCALL 17 sys_memmap 
      * with operation SYSMEM_SWP_OP
      */
     struct sc_regs regs;
     regs.a1 = SYSMEM_SWP_OP;
     regs.a2 = vicfpn;
     regs.a3 = swpfpn;
     syscall(caller, 17, &regs);
 
     /* SYSCALL 17 sys_memmap */
 
     /* TODO copy target frame form swap to mem 
      * SWP(tgtfpn <--> vicfpn)
      * SYSCALL 17 sys_memmap
      * with operation SYSMEM_SWP_OP
      */
     /* TODO copy target frame form swap to mem 
     //regs.a1 =...
     //regs.a2 =...
     //regs.a3 =..
     */
    
    regs.a2 = tgtfpn;
    regs.a3 = vicfpn;
    syscall(caller, 17, &regs);
 
     /* SYSCALL 17 sys_memmap */
 
     /* Update page table */
     //pte_set_swap() 
     //mm->pgd;
 
     /* Update its online status of the target page */
     //pte_set_fpn() &
     //mm->pgd[pgn];
     //pte_set_fpn();
     pte_set_swap(&mm->pgd[vicpgn], 0, swpfpn);
     pte_set_fpn(&mm->pgd[pgn], vicfpn);
 
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
   int off = PAGING_OFFST(addr);
   int fpn;
 
   /* Get the page to MEMRAM, swap from MEMSWAP if needed */
   if (pg_getpage(mm, pgn, &fpn, caller) != 0)
     return -1; /* invalid page access */
 
   /* TODO 
    *  MEMPHY_read(caller->mram, phyaddr, data);
    *  MEMPHY READ 
    *  SYSCALL 17 sys_memmap with SYSMEM_IO_READ
    */
    int phyaddr = (fpn << PAGING_ADDR_PGN_LOBIT) + off;
 
    struct sc_regs regs;
    regs.a1 = SYSMEM_IO_READ; // Thao tác đọc IO
    regs.a2 = phyaddr;        // Địa chỉ vật lý cần đọc
    regs.a3 = 0;             // Đối số 2: Giá trị ghi (không sử dụng trong đọc) 
    int ret = syscall(caller, 17, &regs);
    if (ret != 0) 
        return -1;               // call syscall #17 (memmap)
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
   int syscall_ret;
 
   /* Get the page to MEMRAM, swap from MEMSWAP if needed */
   if (pg_getpage(mm, pgn, &fpn, caller) != 0)
     return -1; /* invalid page access */
 
   /* TODO
    *  MEMPHY_write(caller->mram, phyaddr, value);
    *  MEMPHY WRITE
    *  SYSCALL 17 sys_memmap with SYSMEM_IO_WRITE
    */
   int phyaddr = (fpn << PAGING_ADDR_FPN_LOBIT) + off;
   struct sc_regs regs;
   regs.a1 = SYSMEM_IO_WRITE; // Mã thao tác: Ghi IO
   regs.a2 = phyaddr;         // Đối số 1: Địa chỉ vật lý
   regs.a3 = value;           // Đối số 2: Giá trị ghi
 
   /* SYSCALL 17 sys_memmap */
   syscall_ret = syscall(caller, 17, &regs);
   if (syscall_ret < 0) {
     return -1; // Error in syscall
   }
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
//  int __read(struct pcb_t *caller, int vmaid, int rgid, int offset, BYTE *data)
//  {
//    struct vm_rg_struct *currg = get_symrg_byid(caller->mm, rgid);
//    struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
 
//    if (currg == NULL || cur_vma == NULL) /* Invalid memory identify */
//      return -1;
 
//    pg_getval(caller->mm, currg->rg_start + offset, data, caller);
 
//    return 0;
//  }

int __read(struct pcb_t *caller, int vmaid, int rgid, int offset, BYTE *data)
{
    struct vm_rg_struct *currg = get_symrg_byid(caller->mm, rgid);
    struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
    if (!currg || !cur_vma) 
        return -1;

    // Kiểm tra offset trong region
    unsigned long region_len = currg->rg_end - currg->rg_start;
    if ((unsigned long)offset >= region_len ||
        currg->rg_start + offset >= cur_vma->vm_end)
        return -1;

    // Gọi pg_getval và kiểm tra lỗi
    int ret = pg_getval(caller->mm, currg->rg_start + offset, data, caller);
    if (ret != 0)
        return -1;

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
   //destination 
   if( val == 0 && destination )
   {
     *destination = (uint32_t)data;
   }
 // #ifdef IODUMP
 //   printf("read region=%d offset=%d value=%d\n", source, offset, data);
 // #ifdef PAGETBL_DUMP
 //   print_pgtbl(proc, 0, -1); //print max TBL
 // #endif
 //   MEMPHY_dump(proc->mram);
 // #endif
 //   printf("finish libread\n");
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
   struct pgn_t *prev = NULL;
 
   /* TODO: Implement the theorical mechanism to find the victim page */
   while(pg->pg_next != NULL){
     prev = pg;
     pg = pg->pg_next;
   }
   *retpgn = pg->pgn;
   if(prev == NULL)
     mm->fifo_pgn = NULL;
   else
     prev->pg_next = NULL;
   free(pg);
 
   return 0;
 }
 
 /*get_free_vmrg_area - get a free vm region
  *@caller: caller
  *@vmaid: ID vm area to alloc memory region
  *@size: allocated size
  *
  */
 int get_free_vmrg_area(struct pcb_t *caller, int vmaid, int size, struct vm_rg_struct *newrg)
 {
   struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
 
   struct vm_rg_struct *rgit = cur_vma->vm_freerg_list;
   struct vm_rg_struct *prev = NULL;
   if (rgit == NULL)
     return -1;
 
   /* Probe unintialized newrg */
   newrg->rg_start = newrg->rg_end = -1;
 
   /* TODO Traverse on list of free vm region to find a fit space */
   //while (...)
   // ..
   while (rgit != NULL)
     {
         int region_size = rgit->rg_end - rgit->rg_start;
         if (region_size >= size)
         {
             newrg->rg_start = rgit->rg_start;
             newrg->rg_end = rgit->rg_start + size;
             if (region_size == size) {
                 if (prev == NULL) cur_vma->vm_freerg_list = rgit->rg_next;
                 else prev->rg_next = rgit->rg_next;
                 free(rgit);
             } else {
                 rgit->rg_start += size;
             }
             // **Giải thích**: Duyệt danh sách vùng trống. Nếu tìm thấy vùng đủ lớn (>= size), cấp phát từ đầu vùng đó (cập nhật newrg). Nếu vùng trống bị dùng hết, xóa node khỏi danh sách và free. Nếu còn dư, cập nhật lại rg_start của node đó. Trả về 0.
             return 0;
         }
         prev = rgit;
         rgit = rgit->rg_next;
     }
   return -1;
 }
 
 //#endif
 
 
 