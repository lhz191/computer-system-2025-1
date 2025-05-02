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
  // 首次调用时初始化
  if (current->cur_brk == 0) {
    current->cur_brk = current->max_brk = new_brk;
    return 0;
  }
  
  // 申请新内存
  if (new_brk > current->max_brk) {
    // 向上取整计算起始地址
    uint32_t start = PGROUNDUP(current->max_brk);
    // 对于结束地址，也向上取整确保覆盖全部请求区域
    uint32_t end = PGROUNDUP(new_brk);
    
    // 映射页面
    for (uint32_t addr = start; addr < end; addr += PGSIZE) {
      void *pa = new_page();
      _map(&(current->as), (void*)addr, pa);
    }
    
    current->max_brk = new_brk;
  }
  
  current->cur_brk = new_brk;
  return 0;
}
void init_mm() {
  pf = (void *)PGROUNDUP((uintptr_t)_heap.start);
  Log("free physical pages starting from %p", pf);

  _pte_init(new_page, free_page);
}
