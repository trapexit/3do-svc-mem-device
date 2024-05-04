#include "svc_mem_drv.h"

#include "svc_funcs.h"

#include "kernel.h"
#include "portfolio.h"
#include "setjmp.h"
#include "strings.h"

#define SVC_MEM_DRV_NAME "svc-mem-drv"

#define ABT_ROMF 0x00000001

#define ONEMEG (1024 * 1024)

#define DRAM_START_ADDR  0x00000000
#define VRAM_START_ADDR  0x00200000
#define NVRAM_START_ADDR 0x03140000
#define ROM1_START_ADDR  0x03000000
#define ROM2_START_ADDR  0x03000000
#define MADAM_START_ADDR 0x03300000
#define CLIO_START_ADDR  0x03400000
#define SPORT_START_ADDR 0x03200000

#define DRAM_SIZE  (2 * ONEMEG)
#define VRAM_SIZE  (1 * ONEMEG)
#define ROM1_SIZE  (1 * ONEMEG)
#define ROM2_SIZE  (1 * ONEMEG)
#define NVRAM_SIZE (32 * 1024)
#define MADAM_SIZE ( 2 * 1024)
#define CLIO_SIZE  ( 1 * 1024)
#define SPORT_SIZE (1 * ONEMEG)

#define SYSINFO_TAG_SETROMBANK 0x11006
#define SYSINFO_TAG_CURROMBANK 0x10006
#define SYSINFO_ROMBANK1       0
#define SYSINFO_ROMBANK2       1
#define SYSINFO_TAG_ROM2BASE   0x10007
#define SYSINFO_ROM2FOUND      0
#define SYSINFO_ROM2NOTFOUND   1


enum driver_tags_e
  {
    CREATEDRIVER_TAG_ABORTIO = TAG_ITEM_LAST+1, // 0x0A
    CREATEDRIVER_TAG_MAXCMDS,	                // 0x0B
    CREATEDRIVER_TAG_CMDTABLE,	                // 0x0C
    CREATEDRIVER_TAG_MSGPORT,	                // 0x0D
    CREATEDRIVER_TAG_INIT,	                // 0x0E
    CREATEDRIVER_TAG_DISPATCH	                // 0x0F
  };

#define DRV_CMDTABLE_LEN 3

static
void*
get_rom2_base(void)
{
  Err err;
  void *rv;

  err = svc_QuerySysInfo(SYSINFO_TAG_ROM2BASE,&rv,sizeof(rv));

  return rv;
}

static
Err
set_rom_bank1(void)
{
  Err err;

  err = svc_SetSysInfo(SYSINFO_TAG_SETROMBANK,(void*)SYSINFO_ROMBANK1,0);

  return err;
}

static
Err
set_rom_bank2(void)
{
  Err err;

  err = svc_SetSysInfo(SYSINFO_TAG_SETROMBANK,(void*)SYSINFO_ROMBANK2,0);

  return err;
}

static
void
drv_abortio(struct IOReq *ior_)
{
  (void)ior_;

  svc_kprintf(SVC_MEM_DRV_NAME ": drv_abortio\n");
}

static
Item
drv_init(struct Driver *drv_)
{
  svc_kprintf(SVC_MEM_DRV_NAME ": drv_init - opencnt=%d;\n",
              drv_->drv_OpenCnt);

  return drv_->drv.n_Item;
}

static
i32
drv_cmdwrite_u32(const void *src_,
                 const i32   len_,
                 void       *dst_,
                 const i32   offset_)
{
  i32 i;
  const u32 *src = (const u32*)src_;
  u32       *dst = (u32*)dst_;

  dst += offset_;
  for(i = 0; i < len_; i++)
    dst[i] = src[i];

  return 1;
}

static
i32
drv_cmdwrite_u8(const void *src_,
                const i32   len_,
                void       *dst_,
                const i32   offset_)
{
  i32 i;
  const u8 *src = (const u8*)src_;
  u8       *dst = (u8*)dst_;

  dst += offset_;
  for(i = 0; i < len_; i++)
    dst[i] = src[i];

  return 1;
}

static
i32
aligned(const void *p0_,
        const void *p1_)
{
  return ((((u32)p0_ | (u32)p1_) & 0x3) == 0);
}

static
i32
drv_cmdwrite(struct IOReq *ior_)
{
  const void *src;
  void       *dst;
  i32         offset;
  i32         offset_in_bytes;
  i32         len;
  i32         in_words;

  src             = ior_->io_Info.ioi_Send.iob_Buffer;
  len             = ior_->io_Info.ioi_Send.iob_Len;
  in_words        = !!(ior_->io_Info.ioi_CmdOptions & SVC_MEM_CMD_FLAG_WORDS);
  offset          = ior_->io_Info.ioi_Offset;
  offset_in_bytes = (offset * (in_words ? sizeof(u32) : sizeof(u8)));

  ior_->io_Actual = len;

  switch(ior_->io_Info.ioi_Unit)
    {
    case SVC_MEM_UNIT_NONE:
      dst = ior_->io_Info.ioi_Recv.iob_Buffer;

      if(in_words && !aligned(src,dst))
        ior_->io_Error = BADPTR;
      if(ior_->io_Error)
        return 1;

      if(in_words)
        return drv_cmdwrite_u32(src,len,dst,offset);
      return drv_cmdwrite_u8(src,len,dst,offset);

    case SVC_MEM_UNIT_DRAM:
      dst = (void*)DRAM_START_ADDR;

      if((offset_in_bytes + len) > DRAM_SIZE)
        ior_->io_Error = BADPTR;
      if(in_words && !aligned(src,dst))
        ior_->io_Error = BADPTR;
      if(ior_->io_Error)
        return 1;

      if(in_words)
        return drv_cmdwrite_u32(src,len,dst,offset);
      return drv_cmdwrite_u8(src,len,dst,offset);

    case SVC_MEM_UNIT_VRAM:
      dst = (void*)VRAM_START_ADDR;

      if((offset_in_bytes + len) > VRAM_SIZE)
        ior_->io_Error = BADPTR;
      if(in_words && !aligned(src,dst))
        ior_->io_Error = BADPTR;
      if(ior_->io_Error)
        return 1;

      if(in_words)
        return drv_cmdwrite_u32(src,len,dst,offset);
      return drv_cmdwrite_u8(src,len,dst,offset);

    case SVC_MEM_UNIT_ROM1:
    case SVC_MEM_UNIT_ROM2:
      ior_->io_Error = NOSUPPORT;
      return 1;

    case SVC_MEM_UNIT_NVRAM:
      // TODO create dedicated write to nvram function
      ior_->io_Error = NOSUPPORT;
      return 1;
      dst = (void*)NVRAM_START_ADDR;

      if(in_words)
        ior_->io_Error = BADSIZE;
      if(in_words && !aligned(src,dst))
        ior_->io_Error = BADPTR;
      if((offset_in_bytes + len) > NVRAM_SIZE)
        ior_->io_Error = BADPTR;
      if(ior_->io_Error)
        return 1;
      break;

    case SVC_MEM_UNIT_MADAM:
      dst = (void*)MADAM_START_ADDR;

      if(!in_words)
        ior_->io_Error = BADSIZE;
      if(!aligned(src,dst))
        ior_->io_Error = BADPTR;
      if((offset_in_bytes + len) > MADAM_SIZE)
        ior_->io_Error = BADPTR;
      if(ior_->io_Error)
        return 1;
      return drv_cmdwrite_u32(src,len,dst,offset);

    case SVC_MEM_UNIT_CLIO:
      dst = (void*)CLIO_START_ADDR;

      if(!in_words)
        ior_->io_Error = BADSIZE;
      if(!aligned(src,dst))
        ior_->io_Error = BADPTR;
      if((offset_in_bytes + len) > CLIO_SIZE)
        ior_->io_Error = BADPTR;
      if(ior_->io_Error)
        return 1;
      return drv_cmdwrite_u32(src,len,dst,offset);

    case SVC_MEM_UNIT_SPORT:
      dst = (void*)SPORT_START_ADDR;

      if(!in_words)
        ior_->io_Error = BADSIZE;
      if(!aligned(src,dst))
        ior_->io_Error = BADPTR;
      if((offset_in_bytes + len) > SPORT_SIZE)
        ior_->io_Error = BADPTR;
      if(ior_->io_Error)
        return 1;
      return drv_cmdwrite_u32(src,len,dst,offset);
    }

  ior_->io_Error = BADUNIT;
  return 1;
}

static
i32
drv_cmdread_u32(const void *src_,
                const i32   offset_,
                void       *dst_,
                const i32   len_)
{
  i32 i;
  const u32 *src = (const u32*)src_;
  u32       *dst = (u32*)dst_;

  src += offset_;
  for(i = 0; i < len_; i++)
    dst[i]  = src[i];

  return 1;
}

static
i32
drv_cmdread_u8(const void *src_,
               const i32   offset_,
               void       *dst_,
               const i32   len_)
{
  i32 i;
  const u8 *src = (const u8*)src_;
  u8       *dst = (u8*)dst_;

  src += offset_;
  for(i = 0; i < len_; i++)
    dst[i] = src[i];

  return 1;
}

static
i32
drv_cmdread_u8_per_u32_aborts(const void *src_,
                              const i32   offset_,
                              void       *dst_,
                              const i32   len_)
{
  const u32 *src;
  u8 *dst;
  jmp_buf jmpbuf;
  jmp_buf *old_catchdataaborts;
  u32 old_quietaborts;
  volatile i32 i;

  old_catchdataaborts = KernelBase->kb_CatchDataAborts;
  old_quietaborts     = KernelBase->kb_QuietAborts;

  i   = 0;
  src = (const u32*)src_;
  src = &src[offset_];
  dst = (u8*)dst_;

 catch_abort:

  KernelBase->kb_CatchDataAborts = &jmpbuf;
  KernelBase->kb_QuietAborts     =  ABT_ROMF;

  if(setjmp(jmpbuf))
    goto catch_abort;

  for(; i < len_; i++)
    dst[i] = (u8)(src[i] & 0xFF);

  KernelBase->kb_CatchDataAborts = old_catchdataaborts;
  KernelBase->kb_QuietAborts     = old_quietaborts;

  return 1;
}

static
i32
drv_cmdread_u8_aborts(const void *src_,
                      const i32   offset_,
                      void       *dst_,
                      const i32   len_)
{
  const u8 *src;
  u8 *dst;
  jmp_buf jmpbuf;
  jmp_buf *old_catchdataaborts;
  u32   old_quietaborts;
  volatile i32 i;

  old_catchdataaborts = KernelBase->kb_CatchDataAborts;
  old_quietaborts     = KernelBase->kb_QuietAborts;

  i   = 0;
  src = (const u8*)src_;
  src = &src[offset_];
  dst = (u8*)dst_;

 catch_abort:

  KernelBase->kb_CatchDataAborts = &jmpbuf;
  KernelBase->kb_QuietAborts     =  ABT_ROMF;

  if(setjmp(jmpbuf))
    goto catch_abort;

  for(; i < len_; i++)
    dst[i] = src[i];

  KernelBase->kb_CatchDataAborts = old_catchdataaborts;
  KernelBase->kb_QuietAborts     = old_quietaborts;

  return 1;
}

static
i32
drv_cmdread_u32_aborts(const void *src_,
                       const i32   offset_,
                       void       *dst_,
                       const i32   len_)
{
  volatile const u32 *src;
  u32 *dst;
  jmp_buf jmpbuf;
  jmp_buf *old_catchdataaborts;
  u32 old_quietaborts;
  volatile i32 i;

  old_catchdataaborts = KernelBase->kb_CatchDataAborts;
  old_quietaborts     = KernelBase->kb_QuietAborts;

  i   = 0;
  src = (const u32*)src_;
  src = &src[offset_];
  dst = (u32*)dst_;

 catch_abort:

  KernelBase->kb_CatchDataAborts = &jmpbuf;
  KernelBase->kb_QuietAborts     =  ABT_ROMF;

  if(setjmp(jmpbuf))
    goto catch_abort;

  for(; i < len_; i++)
    dst[i] = src[i];

  KernelBase->kb_CatchDataAborts = old_catchdataaborts;
  KernelBase->kb_QuietAborts     = old_quietaborts;

  return 1;
}

static
i32
drv_cmdread(struct IOReq *ior_)
{
  Err         err;
  const void *src;
  void       *dst;
  i32         offset;
  i32         offset_in_bytes;
  i32         len;
  i32         in_words;

  dst             = ior_->io_Info.ioi_Recv.iob_Buffer;
  len             = ior_->io_Info.ioi_Recv.iob_Len;
  in_words        = !!(ior_->io_Info.ioi_CmdOptions & SVC_MEM_CMD_FLAG_WORDS);
  offset          = ior_->io_Info.ioi_Offset;
  offset_in_bytes = (offset * (in_words ? sizeof(u32) : sizeof(u8)));

  ior_->io_Actual = len;

  switch(ior_->io_Info.ioi_Unit)
    {
    case SVC_MEM_UNIT_NONE:
      src = ior_->io_Info.ioi_Send.iob_Buffer;

      if(in_words && !aligned(src,dst))
        ior_->io_Error = BADPTR;

      if(in_words)
        return drv_cmdread_u32(src,offset,dst,len);
      return drv_cmdread_u8(src,offset,dst,len);

    case SVC_MEM_UNIT_DRAM:
      src = (void*)DRAM_START_ADDR;

      if((offset_in_bytes + len) > DRAM_SIZE)
        ior_->io_Error = BADPTR;
      if(in_words && !aligned(src,dst))
        ior_->io_Error = BADPTR;
      if(ior_->io_Error)
        return 1;

      if(in_words)
        return drv_cmdread_u32(src,offset,dst,len);
      return drv_cmdread_u8(src,offset,dst,len);

    case SVC_MEM_UNIT_VRAM:
      src = (void*)VRAM_START_ADDR;

      if((offset_in_bytes + len) > VRAM_SIZE)
        ior_->io_Error = BADPTR;
      if(in_words && !aligned(src,dst))
        ior_->io_Error = BADPTR;
      if(ior_->io_Error)
        return 1;

      if(in_words)
        return drv_cmdread_u32(src,offset,dst,len);
      return drv_cmdread_u8(src,offset,dst,len);

    case SVC_MEM_UNIT_ROM1:
      src = (void*)ROM1_START_ADDR;

      err = set_rom_bank1();
      if(err)
        ior_->io_Error = err;
      if((offset_in_bytes + len) > ROM1_SIZE)
        ior_->io_Error = BADPTR;
      if(in_words && !aligned(src,dst))
        ior_->io_Error = BADPTR;
      if(ior_->io_Error)
        return 1;

      if(in_words)
        drv_cmdread_u32_aborts(src,offset,dst,len);
      return drv_cmdread_u8_aborts(src,offset,dst,len);

    case SVC_MEM_UNIT_ROM2:
      src = get_rom2_base();

      err = set_rom_bank2();
      if(err)
        ior_->io_Error = err;
      if((offset_in_bytes + len) > ROM2_SIZE)
        ior_->io_Error = BADPTR;
      if(in_words && !aligned(src,dst))
        ior_->io_Error = BADPTR;
      if(ior_->io_Error)
        return 1;

      if(in_words)
        drv_cmdread_u32_aborts(src,offset,dst,len);
      return drv_cmdread_u8_aborts(src,offset,dst,len);


    case SVC_MEM_UNIT_NVRAM:
      src = (void*)NVRAM_START_ADDR;

      if(in_words)
        ior_->io_Error = BADSIZE;
      if((offset_in_bytes + len) > NVRAM_SIZE)
        ior_->io_Error = BADPTR;
      if(ior_->io_Error)
        return 1;

      return drv_cmdread_u8_per_u32_aborts(src,offset,dst,len);

    case SVC_MEM_UNIT_MADAM:
      src = (void*)MADAM_START_ADDR;

      if(!in_words)
        ior_->io_Error = BADSIZE;
      if(!aligned(src,dst))
        ior_->io_Error = BADPTR;
      if((offset_in_bytes + len) > MADAM_SIZE)
        ior_->io_Error = BADPTR;
      if(ior_->io_Error)
        return 1;

      return drv_cmdread_u32(src,offset,dst,len);

    case SVC_MEM_UNIT_CLIO:
      src = (void*)CLIO_START_ADDR;

      if(!in_words)
        ior_->io_Error = BADSIZE;
      if(!aligned(src,dst))
        ior_->io_Error = BADPTR;
      if((offset_in_bytes + len) > CLIO_SIZE)
        ior_->io_Error = BADPTR;
      if(ior_->io_Error)
        return 1;

      return drv_cmdread_u32(src,offset,dst,len);

    case SVC_MEM_UNIT_SPORT:
      src = (void*)SPORT_START_ADDR;

      if(!in_words)
        ior_->io_Error = BADSIZE;
      if(!aligned(src,dst))
        ior_->io_Error = BADPTR;
      if((offset_in_bytes + len) > SPORT_SIZE)
        ior_->io_Error = BADPTR;
      if(ior_->io_Error)
        return 1;

      return drv_cmdread_u32(src,offset,dst,len);
    }

  ior_->io_Error = BADUNIT;
  return 1;
}

static
i32
drv_cmdstatus(struct IOReq *ior_)
{
  (void)ior_;

  return 0;
}

Item
svc_mem_drv_create(void)
{
  static void *drv_cmdtable[DRV_CMDTABLE_LEN] =
    {
      (void*)drv_cmdwrite,
      (void*)drv_cmdread,
      (void*)drv_cmdstatus
    };

  static TagArg drv_tags[] =
    {
      {TAG_ITEM_PRI,              (void*)1},	            // 0
      {TAG_ITEM_NAME,             (void*)SVC_MEM_DRV_NAME}, // 1
      {CREATEDRIVER_TAG_ABORTIO,  (void*)drv_abortio},      // 2
      {CREATEDRIVER_TAG_MAXCMDS,  (void*)DRV_CMDTABLE_LEN}, // 3
      {CREATEDRIVER_TAG_CMDTABLE, (void*)NULL},             // 4
      {CREATEDRIVER_TAG_INIT,     (void*)drv_init},         // 5
      {TAG_END,                   (void*)0}
    };

  drv_tags[4].ta_Arg = (void*)drv_cmdtable;

  return CreateItem(MKNODEID(KERNELNODE,DRIVERNODE),drv_tags);
}
