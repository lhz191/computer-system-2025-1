#include <x86.h>

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
  // // map kernel space
  // for (int i = 0; i < NR_PDE; i ++) {
  //   updir[i] = kpdirs[i];
  // }

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
  
  // 检查页目录项是否存在，不存在则创建一个新的页表
  if (!(pgdir[pde_idx] & PTE_P)) {
    // 申请一个新页用作页表
    PTE *ptab = (PTE*)palloc_f();
    
    // 初始化页表，将所有页表项设为无效
    for (int i = 0; i < NR_PTE; i++) {
      ptab[i] = 0;
    }
    
    // 更新页目录项，指向新的页表，并设置存在位和读写位
    pgdir[pde_idx] = (uint32_t)ptab | PTE_P | PTE_W | PTE_U;
  }
  
  // 获取页表地址
  PTE *ptab = (PTE*)(pgdir[pde_idx] & ~0xFFF);  // 清除低12位标志位得到页表基址
  
  // 更新页表项，建立va到pa的映射
  ptab[pte_idx] = (uint32_t)pa | PTE_P | PTE_W | PTE_U;
}

void _unmap(_Protect *p, void *va) {
}

_RegSet *_umake(_Protect *p, _Area ustack, _Area kstack, void *entry, char *const argv[], char *const envp[]) {
  return NULL;
}
