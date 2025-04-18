/*
 * Copyright (C) 2025 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* Sierra release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

 #include "common.h"
 #include "syscall.h"
 #include "stdio.h"
 #include "libmem.h"

#include "queue.h"
#include "mm.h"
#include "stdlib.h"
#include "string.h"


// #ifdef MM_PAGING
// void release_process_memory(struct pcb_t *proc){
//     int pagenum;
//     uint32_t pte;

//     for (pagenum = 0; pagenum < PAGING_MAX_PGN; pagenum++) {
//         pte = proc->mm->pgd[pagenum];

//         if (PAGING_PAGE_PRESENT(pte)){
//             /*Page is in mem*/
//             int fpn =  PAGING_FPN(pte);
//             MEMPHY_get_freefp(proc->mram, fpn);
//         }
//         else {
//             int swpfpn = PAGING_PTE_SWP(pte);
//             MEMPHY_get_freefp(proc->active_mswp, swpfpn);
//         }
//     }

//     free(proc->mm->pgd);
// }

// #endif 





 int __sys_killall(struct pcb_t *caller, struct sc_regs* regs)
 {
    //  char proc_name[100];
    //  uint32_t data;
    //  int kill_count = 0; 
 
    //  //memory region ID
    //  uint32_t memrg = regs->a1;
     
    //  /* TODO: Get name of the target proc */
    //  //proc_name = libread..
    //  int i = 0;
    //  data = 0;
    //  while(data != -1){
    //      libread(caller, memrg, i, &data);
    //      proc_name[i]= data;
    //      if(data == -1) proc_name[i]='\0';
    //      i++;
    //  }
    //  printf("The procname retrieved from memregionid %d is \"%s\"\n", memrg, proc_name);
 
    //  /* TODO: Traverse proclist to terminate the proc
    //   *       stcmp to check the process match proc_name
    //   */


    //  if (caller->running_list != NULL){
    //     struct queue_t temp_queue; 
    //     temp_queue.size=0;


    //     while (!empty(caller->running_list)){
    //         struct pcb_t *proc = dequeue(caller->running_list); 
    //         if (strcmp(proc->path, proc_name) == 0){

    //             printf("Terminating running process PID %d with name %s\n", proc->pid, proc->path);

    //             free(proc->code);
    // #ifdef MM_PAGING 
    //             release_process_memory(proc);
    //             free(proc->mm);
    // #endif
    //             free(proc);
    //             kill_count++;
    //         }
    //         else{
    //             enqueue(&temp_queue, proc);
    //         }
    //     }

    //     while(!empty(&temp_queue)){
    //         enqueue(caller->running_list, dequeue(&temp_queue));
    //     }
    //  }





    //  //caller->running_list
    //  //caller->mlq_ready_queu
 
    //  /* TODO Maching and terminating 
    //   *       all processes with given
    //   *        name in var proc_name
    //   */
    //  printf("killall: terminated %d processes with name \"%s\"\n", kill_count, proc_name);

    //  return kill_count; 
    return 0;
 }
 