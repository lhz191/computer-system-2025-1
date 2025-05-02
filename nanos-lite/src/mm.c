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

/* The brk() system call handler. */
int mm_brk(uint32_t new_brk) {
  if (current->cur_brk == 0) {
    Log("Initializing memory break at 0x%08x", new_brk);
    current->cur_brk = current->max_brk = new_brk;
  }
  else {
    if (new_brk > current->max_brk) {
      // 计算需要映射的页面范围
      uint32_t first = PGROUNDUP(current->max_brk);
      uint32_t end = PGROUNDDOWN(new_brk);
      
      // 特殊情况：如果new_brk恰好在页边界上，不需要映射该页
      if ((new_brk & 0xfff) == 0) {
        end -= PGSIZE;
      }
      
      Log("Extending memory break from 0x%08x to 0x%08x", current->max_brk, new_brk);
      
      // 遍历需要映射的每一页，分配物理内存并映射
      for (uint32_t va = first; va <= end; va += PGSIZE) {
        void* pa = new_page();
        if (pa == NULL) {
          Log("Failed to allocate new page at 0x%08x", va);
          return -1; // 页面分配失败
        }
        
        _map(&current->as, (void*)va, pa);
        Log("Mapped page at 0x%08x to physical address %p", va, pa);
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
