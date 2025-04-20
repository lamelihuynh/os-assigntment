// // #ifdef MM_PAGING
// /*
//  * PAGING based Memory Management
//  * Virtual memory module mm/mm-vm.c
//  */

// #include "string.h"
// #include "mm.h"
// #include <stdlib.h>
// #include <stdio.h>
// #include <pthread.h>

// /*get_vma_by_num - get vm area by numID
//  *@mm: memory region
//  *@vmaid: ID vm area to alloc memory region
//  *
//  */
// struct vm_area_struct *get_vma_by_num(struct mm_struct *mm, int vmaid)
// {
//   struct vm_area_struct *pvma = mm->mmap;
//   while (pvma != NULL)
//   {
//     if (pvma->vm_id == vmaid)
//     {
//       return pvma;
//     }
//     pvma = pvma->vm_next;
//   }
//   return NULL;
// }

// int __mm_swap_page(struct pcb_t *caller, int vicfpn , int swpfpn)
// {
//     __swap_cp_page(caller->mram, vicfpn, caller->active_mswp, swpfpn);
//     return 0;
// }

// /*get_vm_area_node - get vm area for a number of pages
//  *@caller: caller
//  *@vmaid: ID vm area to alloc memory region
//  *@incpgnum: number of page
//  *@vmastart: vma end
//  *@vmaend: vma end
//  *
//  */
// struct vm_rg_struct *get_vm_area_node_at_brk(struct pcb_t *caller, int vmaid, int size, int alignedsz)
// {
//   // struct vm_rg_struct * newrg;
//   // /* TODO retrive current vma to obtain newrg, current comment out due to compiler redundant warning*/
//   // struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
//   // if( cur_vma == NULL){
//   //   return NULL;
//   // }
//   // if (alignedsz < size) alignedsz = PAGING_PAGE_ALIGNSZ(size);
//   // newrg = malloc(sizeof(struct vm_rg_struct));
//   // if (newrg == NULL)
//   // {
//   //   return NULL;
//   // } 

//   // // TODO: update the newrg boundary
//   // cur_vma->sbrk += alignedsz;
//   //  newrg->rg_start = cur_vma->vm_end;
//   //  newrg->rg_end = newrg->rg_start + alignedsz;
//   // return newrg;
//   struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
//   if (!cur_vma)
//     return NULL;
//   if (alignedsz < size)
//     alignedsz = PAGING_PAGE_ALIGNSZ(size);
//   struct vm_rg_struct *newrg = malloc(sizeof(struct vm_rg_struct));
//   if (!newrg)
//     return NULL;

//   cur_vma->sbrk += alignedsz;
//   newrg->rg_start = cur_vma->sbrk;
//   newrg->rg_end = newrg->rg_start + alignedsz;

//   return newrg;
// }

// /*validate_overlap_vm_area
//  *@caller: caller
// //  *@vmaid: ID vm area to alloc memory region
//  *@vmastart: vma end
//  *@vmaend: vma end
//  *
//  */
// int validate_overlap_vm_area(struct pcb_t *caller, int vmaid, int vmastart, int vmaend)
// {
//   struct vm_area_struct *vma = caller->mm->mmap;
//   while (vma != NULL)
//   {
//     if (((vmastart < vma->vm_end && vmaend > vma->vm_start) ||
//          (vma->vm_start < vmaend && vma->vm_end > vmastart)))
//     {
//       return -1;
//     }
//     vma = vma->vm_next;
//   }
//   return 0;
// }

// /*inc_vma_limit - increase vm area limits to reserve space for new variable
//  *@caller: caller
//  *@vmaid: ID vm area to alloc memory region
//  *@inc_sz: increment size
//  *
//  */

//  int inc_vma_limit(struct pcb_t *caller, int vmaid, int inc_sz)
//  {
//    if (!caller || !caller->mm)
//      return -1;
 
//    struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
//    if (!cur_vma)
//      return -1;
 
//    if (inc_sz == 0)
//      return PAGING_PAGE_ALIGNSZ(1);
 
//    if (inc_sz < 0)
//      return -1;
 
//    int inc_amt = PAGING_PAGE_ALIGNSZ(inc_sz);
//    int old_end = cur_vma->vm_end;
//    int new_end = old_end + inc_amt;
 
//    // Cộng dồn sbrk
 
//    cur_vma->vm_end = new_end;
//    cur_vma->sbrk = old_end + inc_sz;
 
//    if (validate_overlap_vm_area(caller, vmaid, old_end, new_end) < 0)
//      return -1;
 
//    struct vm_rg_struct *free_rg = NULL;
//    struct vm_rg_struct *last_rg = NULL;
 
//    if (cur_vma->vm_freerg_list)
//    {
//      last_rg = cur_vma->vm_freerg_list;
//      while (last_rg->rg_next)
//      {
//        last_rg = last_rg->rg_next;
//      }
//    }
 
//    if (!last_rg || last_rg->rg_end != old_end)
//    {
//      free_rg = malloc(sizeof(struct vm_rg_struct));
//      if (!free_rg)
//        return -1;
 
//      free_rg->rg_start = old_end;
//      free_rg->rg_end = new_end;
 
//      if (!cur_vma->vm_freerg_list)
//        cur_vma->vm_freerg_list = free_rg;
//      else
//        last_rg->rg_next = free_rg;
//    }
//    else
//    {
//      last_rg->rg_start = old_end;
//      last_rg->rg_end = new_end;
//    }
 
//    return inc_amt;
//  }

// // #endif



// #ifdef MM_PAGING
/*
 * PAGING based Memory Management
 * Virtual memory module mm/mm-vm.c
 */

 #include "string.h"
 #include "mm.h"
 #include <stdlib.h>
 #include <stdio.h>
 #include <pthread.h>
 
 /*get_vma_by_num - get vm area by numID
  *@mm: memory region
  *@vmaid: ID vm area to alloc memory region
  *
  */
 struct vm_area_struct *get_vma_by_num(struct mm_struct *mm, int vmaid)
 {
   struct vm_area_struct *pvma = mm->mmap;
 
   if (mm->mmap == NULL)
     return NULL;
 
   int vmait = pvma->vm_id;
 
   while (vmait < vmaid)
   {
     if (pvma == NULL) return NULL;
 
     pvma = pvma->vm_next;
     vmait = pvma->vm_id;
   }
 
   return pvma;
 }
 
 int __mm_swap_page(struct pcb_t *caller, int vicfpn , int swpfpn)
 {
     __swap_cp_page(caller->mram, vicfpn, caller->active_mswp, swpfpn);
     return 0;
 }
 
 /*get_vm_area_node - get vm area for a number of pages
  *@caller: caller
  *@vmaid: ID vm area to alloc memory region
  *@incpgnum: number of page
  *@vmastart: vma end
  *@vmaend: vma end
  *
  */
 struct vm_rg_struct *get_vm_area_node_at_brk(struct pcb_t *caller, int vmaid, int size, int alignedsz)
 {
   struct vm_rg_struct * newrg;
   /* TODO retrive current vma to obtain newrg, current comment out due to compiler redundant warning*/
   struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
   if( cur_vma == NULL){
     return NULL;
   }
   if (alignedsz < size) alignedsz = PAGING_PAGE_ALIGNSZ(size);
   newrg = malloc(sizeof(struct vm_rg_struct));
   if (newrg == NULL)
   {
     return NULL;
   } 
 
   // TODO: update the newrg boundary
   cur_vma->sbrk += alignedsz;
    newrg->rg_start = cur_vma->vm_end;
    newrg->rg_end = newrg->rg_start + alignedsz;
   return newrg;
 }
 
 /*validate_overlap_vm_area
  *@caller: caller
 //  *@vmaid: ID vm area to alloc memory region
  *@vmastart: vma end
  *@vmaend: vma end
  *
  */
 int validate_overlap_vm_area(struct pcb_t *caller, int vmaid, int vmastart, int vmaend)
 {
   struct vm_area_struct *vma = caller->mm->mmap;
   while (vma != NULL)
   {
     if (((vmastart < vma->vm_end && vmaend > vma->vm_start) ||
          (vma->vm_start < vmaend && vma->vm_end > vmastart)))
     {
       return -1;
     }
     vma = vma->vm_next;
   }
   return 0;
 }
 
 /*inc_vma_limit - increase vm area limits to reserve space for new variable
  *@caller: caller
  *@vmaid: ID vm area to alloc memory region
  *@inc_sz: increment size
  *
  */
 int inc_vma_limit(struct pcb_t *caller, int vmaid, int inc_sz)
 {
   struct vm_rg_struct * newrg = malloc(sizeof(struct vm_rg_struct));
   if (newrg == NULL) return -1;
 
   int inc_amt = PAGING_PAGE_ALIGNSZ(inc_sz);
 
   int incnumpage =  inc_amt / PAGING_PAGESZ;
 
   struct vm_rg_struct *area = get_vm_area_node_at_brk(caller, vmaid, inc_sz, inc_amt);
   if (area == NULL) {
     free(newrg);
     return -1;
   }
 
 
   struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
   if (cur_vma== NULL){
     free(newrg);
     free (area);
     return -1;
   }
 
   int old_end = cur_vma->vm_end;
   cur_vma->sbrk = old_end + inc_sz;
 
   /*Validate overlap of obtained region */
   if (validate_overlap_vm_area(caller, vmaid, area->rg_start, area->rg_end) < 0){
     free (area);
     free(newrg);
     return -1; /*Overlap and failed allocation */
   }
 
   /* TODO: Obtain the new vm area based on vmaid */
   if (area->rg_end > cur_vma->vm_end){
     cur_vma->vm_end = area->rg_end;
   }
   // inc_limit_ret...
 
   if (vm_map_ram(caller, area->rg_start, area->rg_end, old_end, incnumpage , newrg) < 0){
     cur_vma->vm_end = old_end;
     free (area);
     free (newrg);
     return -1;
   }
 
   // if (cur_vma->sbrk < cur_vma->vm_end){
   //   struct vm_rg_struct* free_rg = malloc (sizeof(struct vm_rg_struct));
   //   free_rg->rg_start = cur_vma->sbrk;
   //   free_rg->rg_end = cur_vma->vm_end;
   //   free_rg->rg_next = NULL;
 
 
   
     enlist_vm_rg_node(&caller->mm->mmap->vm_freerg_list, area);
     
   // }
 
 
 
   // free(area);
   return inc_amt;
 }
 
 // #endif
 
 
 