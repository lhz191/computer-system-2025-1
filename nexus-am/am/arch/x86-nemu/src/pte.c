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
    PDE *pgdirs=p->ptr;
    PDE *pde=&pgdirs[PDX(va)];
    PTE *pgtabs;
    if(*pde&PTE_P)  pgtabs=(PTE*)PTE_ADDR(*pde);
    else
    {
        pgtabs=(PTE *)palloc_f();
		    *pde=PTE_ADDR(pgtabs)|PTE_P;
    }
    pgtabs[PTX(va)]=PTE_ADDR(pa)|PTE_P;
}

void _unmap(_Protect *p, void *va) {
}

_RegSet *_umake(_Protect *p, _Area ustack, _Area kstack, void *entry, char *const argv[], char *const envp[]) {
  return NULL;
}
