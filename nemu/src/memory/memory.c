#include "nemu.h"
#include "device/mmio.h"

#define PMEM_SIZE (256 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */
/*Pa2.3 读取内存*/
// uint32_t paddr_read(paddr_t addr, int len) {
//   return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
// }

uint32_t paddr_read(paddr_t addr, int len) {
  int mmio_id = is_mmio(addr);
  if (mmio_id != -1) {
    return mmio_read(addr,len,mmio_id);
  }
  return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
}

// void paddr_write(paddr_t addr, int len, uint32_t data) {
//   memcpy(guest_to_host(addr), &data, len);
// }

   void paddr_write(paddr_t addr, int len, uint32_t data) {
     // 保护低地址区域
     if (addr < 0x1000) {
       Log("Warning: Attempt to write to low memory address 0x%x, ignored", addr);
       return;  // 忽略对低地址的写入
     }
     
     int mmio_id = is_mmio(addr);
     if (mmio_id != -1) {
       mmio_write(addr,len,data,mmio_id);
       return;  // 已在mmio_write处理，不需要再写入主内存
     }
     memcpy(guest_to_host(addr), &data, len);
   }

/* 页表地址转换函数实现 */
paddr_t page_translate(vaddr_t addr) {
  /* 当CR0的PG位为0时，不启用分页机制 */
  if (!(cpu.cr0.paging)) {
    return addr;
  }

  /* 将32位虚拟地址分解为：
   * 高10位 - 页目录索引 (Directory)
   * 中10位 - 页表索引 (Page)
   * 低12位 - 页内偏移 (Offset)
   */
  uint32_t dir = (addr >> 22) & 0x3FF;  // 页目录索引（高10位）
  uint32_t page = (addr >> 12) & 0x3FF; // 页表索引（中10位）
  uint32_t offset = addr & 0xFFF;       // 页内偏移（低12位）

  /* 从CR3获取页目录基址 */
  uint32_t page_directory_base = cpu.cr3.page_directory_base << 12;
  
  /* 获取页目录项 */
  PDE pde;
  pde.val = paddr_read(page_directory_base + dir * 4, 4);
  
  /* 检查页目录项的present位 */
  Assert(pde.present, "Page Directory Entry not present! Virtual address = 0x%x", addr);
  
  /* 设置页目录项的accessed位 */
  if (!pde.accessed) {
    pde.accessed = 1;
    paddr_write(page_directory_base + dir * 4, 4, pde.val);
  }
  
  /* 获取页表基址 */
  uint32_t page_table_base = pde.page_frame << 12;
  
  /* 获取页表项 */
  PTE pte;
  pte.val = paddr_read(page_table_base + page * 4, 4);
  
  /* 检查页表项的present位 */
  Assert(pte.present, "Page Table Entry not present! Virtual address = 0x%x", addr);
  
  /* 设置页表项的accessed位 */
  if (!pte.accessed) {
    pte.accessed = 1;
    paddr_write(page_table_base + page * 4, 4, pte.val);
  }

  /* 返回物理地址 = 页框基址 + 偏移量 */
  return (pte.page_frame << 12) | offset;
}

/* 声明页表地址转换函数 */
// paddr_t page_translate(vaddr_t addr);

uint32_t vaddr_read(vaddr_t addr, int len) {
  if (cpu.cr0.paging) {
    /* 检查是否跨页访问 */
    vaddr_t page_start = addr & ~PAGE_MASK;
    vaddr_t page_end = (addr + len - 1) & ~PAGE_MASK;
    if (page_start != page_end) {
      /* 数据跨越页边界，需要分别读取两个页面 */
      int first_len = PAGE_SIZE - (addr & PAGE_MASK);
      int second_len = len - first_len;
      
      /* 分别读取两个页面的数据 */
      uint32_t first_data = vaddr_read(addr, first_len);
      uint32_t second_data = vaddr_read(addr + first_len, second_len);
      
      /* 拼接两个页面的数据 */
      return (second_data << (first_len * 8)) | first_data;
    }
    
    /* 进行页表地址转换 */
    paddr_t paddr = page_translate(addr);
    return paddr_read(paddr, len);
  } else {
    return paddr_read(addr, len);
  }
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  if (cpu.cr0.paging) {
    /* 检查是否跨页访问 */
    vaddr_t page_start = addr & ~PAGE_MASK;
    vaddr_t page_end = (addr + len - 1) & ~PAGE_MASK;
    if (page_start != page_end) {
      /* 数据跨越页边界，需要分别写入两个页面 */
      int first_len = PAGE_SIZE - (addr & PAGE_MASK);
      int second_len = len - first_len;
      
      /* 分别写入两个页面的数据 */
      vaddr_write(addr, first_len, data & ((1 << (first_len * 8)) - 1));
      vaddr_write(addr + first_len, second_len, data >> (first_len * 8));
      return;
    }
    
    /* 进行页表地址转换 */
    paddr_t paddr = page_translate(addr);
    
    /* 设置脏位（只在写操作时） */
    if (cpu.cr0.paging) {
      uint32_t dir = (addr >> 22) & 0x3FF;
      uint32_t page = (addr >> 12) & 0x3FF;
      
      uint32_t page_directory_base = cpu.cr3.page_directory_base << 12;
      PDE pde;
      pde.val = paddr_read(page_directory_base + dir * 4, 4);
      
      uint32_t page_table_base = pde.page_frame << 12;
      PTE pte;
      pte.val = paddr_read(page_table_base + page * 4, 4);
      
      if (!pte.dirty) {
        pte.dirty = 1;
        paddr_write(page_table_base + page * 4, 4, pte.val);
      }
    }
    
    paddr_write(paddr, len, data);
  } else {
    paddr_write(addr, len, data);
  }
}