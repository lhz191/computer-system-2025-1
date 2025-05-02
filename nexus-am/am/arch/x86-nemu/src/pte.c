#include <x86.h>
#include <string.h>  // 添加string.h以使用memset函数

#define PG_ALIGN __attribute((aligned(PGSIZE)))//页

static PDE kpdirs[NR_PDE] PG_ALIGN;//kpdirs页目录
static PTE kptabs[PMEM_SIZE / PGSIZE] PG_ALIGN;//kptabs页表数组，1个页目录表对应1个页表数组
//PMEM_SIZE是1个页目录项对应的页表数组的大小，PMEM_SIZE / PGSIZE是页的数量
static void* (*palloc_f)();//分配页
static void (*pfree_f)(void*);//释放页

_Area segments[] = {      // Kernel memory mappings
  {.start = (void*)0,          .end = (void*)PMEM_SIZE}
};

#define NR_KSEG_MAP (sizeof(segments) / sizeof(segments[0]))

void _pte_init(void* (*palloc)(), void (*pfree)(void*)) {
  palloc_f = palloc;
  pfree_f = pfree;

  int i;

  // make all PDEs invalid
  for (i = 0; i < NR_PDE; i ++) {
    kpdirs[i] = 0;
  }

  PTE *ptab = kptabs;
  for (i = 0; i < NR_KSEG_MAP; i ++) {
    uint32_t pdir_idx = (uintptr_t)segments[i].start / (PGSIZE * NR_PTE);
    uint32_t pdir_idx_end = (uintptr_t)segments[i].end / (PGSIZE * NR_PTE);
    for (; pdir_idx < pdir_idx_end; pdir_idx ++) {
      // fill PDE页目录项
      kpdirs[pdir_idx] = (uintptr_t)ptab | PTE_P;

      // fill PTE页表项
      PTE pte = PGADDR(pdir_idx, 0, 0) | PTE_P;
      PTE pte_end = PGADDR(pdir_idx + 1, 0, 0) | PTE_P;
      for (; pte < pte_end; pte += PGSIZE) {
        *ptab = pte;
        ptab ++;
      }
    }
  }

  set_cr3(kpdirs);
  set_cr0(get_cr0() | CR0_PG);
}

void _protect(_Protect *p) {
  PDE *updir = (PDE*)(palloc_f());
  p->ptr = updir;
  // map kernel space
  for (int i = 0; i < NR_PDE; i ++) {
    updir[i] = kpdirs[i];
  }

  p->area.start = (void*)0x8000000;
  p->area.end = (void*)0xc0000000;
}

void _release(_Protect *p) {
}

void _switch(_Protect *p) {
  set_cr3(p->ptr);
}

void _map(_Protect *p, void *va, void *pa) {
  // Pa4.1: 实现虚拟地址到物理地址的映射
  PDE *pgdir = (PDE*)p->ptr;  // 获取页目录基址
  
  // 计算页目录索引和页表索引
  uint32_t pde_idx = PDX(va);  // 取虚拟地址的高10位作为页目录索引
  uint32_t pte_idx = PTX(va);  // 取虚拟地址的中10位作为页表索引
  
  PTE *ptab;
  // 检查页目录项是否存在，不存在则创建一个新的页表
  if (!(pgdir[pde_idx] & PTE_P)) {
    // 申请一个新页用作页表
    ptab = (PTE*)palloc_f();
    
    // 初始化页表，将所有页表项设为无效
    for (int i = 0; i < NR_PTE; i++) {
      ptab[i] = 0;
    }
    
    // 更新页目录项，指向新的页表，并设置存在位和读写位
    pgdir[pde_idx] = (uint32_t)ptab | PTE_P | PTE_W | PTE_U;
  } else {
    // 页目录项已存在，获取页表地址
    ptab = (PTE*)PTE_ADDR(pgdir[pde_idx]);
  }
  
  // 更新页表项，建立va到pa的映射
  ptab[pte_idx] = (uint32_t)pa | PTE_P | PTE_W | PTE_U;
}

void _unmap(_Protect *p, void *va) {
}

_RegSet *_umake(_Protect *p, _Area ustack, _Area kstack, void *entry, char *const argv[], char *const envp[]) {
  // 计算陷阱帧在用户栈底部的位置
  uintptr_t tf_addr = (uintptr_t)ustack.end - sizeof(_RegSet);
  _RegSet *tf = (_RegSet *)tf_addr;
  
  // 将陷阱帧初始化为0
  memset(tf, 0, sizeof(_RegSet));
  
  // 将CS设置为8，这是为了保证differential testing的正确运行
  tf->cs = 8;
  
  // 设置EIP指向用户程序的入口点（Navy-apps的入口点）
  tf->eip = (uintptr_t)entry;
  
  // 设置EFLAGS，启用中断（IF标志位）
  // 0x202 = 0x2 (保留位) | 0x200 (IF中断标志位)
  tf->eflags = 0x202;
  
  // 在Navy-apps中，_start()函数需要argc、argv和envp参数
  // 在陷阱帧上方设置_start()函数的栈帧
  uintptr_t *_start_frame = (uintptr_t*)(tf_addr - 4 * sizeof(uintptr_t));
  
  // _start()函数栈帧结构：[argc][argv][envp][返回地址]
  // 但由于_start()函数永远不会返回，我们不需要设置返回地址
  _start_frame[0] = 0;          // argc = 0
  _start_frame[1] = 0;          // argv = NULL
  _start_frame[2] = 0;          // envp = NULL
  
  // 设置ESP寄存器指向_start()函数的栈帧
  tf->esp = (uintptr_t)_start_frame;
  
  // 返回陷阱帧指针，由Nanos-lite将此指针记录到用户进程PCB的tf字段中
  return tf;
}
