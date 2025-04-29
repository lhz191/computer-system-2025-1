#include "nemu.h"
#include "device/mmio.h"
#include "memory/memory.h"
#include "cpu/reg.h"
#include "x86.h"

#define PMEM_SIZE (128 * 1024 * 1024)

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
  int mmio_id = is_mmio(addr);
  if (mmio_id != -1) {
    mmio_write(addr,len,data,mmio_id);
  }
  memcpy(guest_to_host(addr), &data, len);
}




/* PA4.1: 实现页地址转换函数 */
paddr_t page_translate(vaddr_t addr) {
  /* 检查是否启用分页机制 */
  if (!(cpu.cr0 & CR0_PG)) {
    return addr;  // 分页未开启，直接返回地址
  }

  /* 获取虚拟地址的各部分 */
  uint32_t pdx = PDX(addr);  // 页目录索引
  uint32_t ptx = PTX(addr);  // 页表索引
  uint32_t off = OFF(addr);  // 页内偏移

  /* 页目录基址从CR3寄存器获取 */
  PDE *pgdir = (PDE *)(cpu.cr3 & ~0xfff);  // 去掉低12位，对齐页边界
  PDE pde = paddr_read((paddr_t)&pgdir[pdx], 4);

  /* 检查页目录项是否有效 */
  Assert(pde & PTE_P, "page directory entry not present");

  /* 获取页表项 */
  PTE *pgtab = (PTE *)(PTE_ADDR(pde));  // 从页目录项获取页表基址
  PTE pte = paddr_read((paddr_t)&pgtab[ptx], 4);

  /* 检查页表项是否有效 */
  Assert(pte & PTE_P, "page table entry not present");

  /* 设置accessed位 */
  if (!(pte & PTE_A)) {
    pte |= PTE_A;
    paddr_write((paddr_t)&pgtab[ptx], 4, pte);
  }

  /* 返回物理地址 */
  return (PTE_ADDR(pte) | off);
}

//Pa4.1 vaddr_read函数
uint32_t vaddr_read(vaddr_t addr, int len) {
  if ((addr & ~0xfff) != ((addr + len - 1) & ~0xfff)) {
    /* 数据跨页边界 */
    assert(0);
  }
  else {
    paddr_t paddr = page_translate(addr);
    return paddr_read(paddr, len);
  }
}

//Pa4.1 vaddr_write函数
void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  if ((addr & ~0xfff) != ((addr + len - 1) & ~0xfff)) {
    /* 数据跨页边界 */
    assert(0);
  }
  else {
    paddr_t paddr = page_translate(addr);
    
    /* 如果分页开启，设置dirty位 */
    if (cpu.cr0 & CR0_PG) {
      uint32_t pdx = PDX(addr);
      uint32_t ptx = PTX(addr);
      PDE *pgdir = (PDE *)(cpu.cr3 & ~0xfff);
      PDE pde = paddr_read((paddr_t)&pgdir[pdx], 4);
      PTE *pgtab = (PTE *)(PTE_ADDR(pde));
      PTE pte = paddr_read((paddr_t)&pgtab[ptx], 4);
      
      if (!(pte & PTE_D)) {
        pte |= PTE_D;
        paddr_write((paddr_t)&pgtab[ptx], 4, pte);
      }
    }
    
    paddr_write(paddr, len, data);
  }
}
