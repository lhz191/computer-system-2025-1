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

uint32_t vaddr_read(vaddr_t addr, int len) {
  return paddr_read(addr, len);
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  paddr_write(addr, len, data);
}
