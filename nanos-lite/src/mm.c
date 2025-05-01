#include "proc.h"
#include "memory.h"

static void *pf = NULL;

void* new_page(void) {
  assert(pf < (void *)_heap.end);
  void *p = pf;
  pf += PGSIZE;
  return p;
}

void free_page(void *p) {
  panic("not implement yet");
}

int mm_brk(uint32_t new_brk) {
  if (current->cur_brk == 0) {
    current->cur_brk = current->max_brk = new_brk;
  }
  else {
    if (new_brk > current->max_brk) {
      // 计算需要映射的起始和结束地址
      uint32_t first = PGROUNDUP(current->max_brk);
      uint32_t end = PGROUNDUP(new_brk); // 向上对齐确保包含最后一个不完整页
      
      // 打印调试信息
      Log("Mapping memory region: [0x%x, 0x%x)", first, end);
      
      // 确保first < end才进行映射
      if (first < end) {
        for (uint32_t va = first; va < end; va += PGSIZE) {
          void* pa = new_page();
          _map(&(current->as), (void*)va, pa);
          Log("Mapped: va=0x%x -> pa=%p", va, pa);
        }
      }
      
      current->max_brk = new_brk;
    }
    current->cur_brk = new_brk;
  }
  return 0;
}

void init_mm() {
  pf = (void *)PGROUNDUP((uintptr_t)_heap.start);
  Log("free physical pages starting from %p", pf);

  _pte_init(new_page, free_page);
}
