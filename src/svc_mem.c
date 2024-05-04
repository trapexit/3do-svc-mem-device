/*
  ISC License

  Copyright (c) 2024, Antonio SJ Musumeci <trapexit@spawn.link>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include "svc_mem.h"
#include "svc_mem_drv_opts.h"

#include "types.h"
#include "io.h"
#include "filefunctions.h"
#include "device.h"

static volatile Item g_SVC_MEM_DRIVER = 0;

Err
svc_mem_init(void)
{
  if(g_SVC_MEM_DRIVER == 0)
    g_SVC_MEM_DRIVER = LoadProgram("System/Drivers/svc_mem_drv");
  if(g_SVC_MEM_DRIVER < 0)
    return g_SVC_MEM_DRIVER;

  return 0;
}

Err
svc_mem_destroy(void)
{
  return DeleteItem(g_SVC_MEM_DRIVER);
}

Item
svc_mem_open_device(void)
{
  return OpenNamedDevice("svc-mem-dev",0);
}

Err
svc_mem_close_device(Item device_)
{
  return CloseNamedDevice(device_);
}

Item
svc_mem_create_ioreq(Item device_)
{
  return CreateIOReq(NULL,0,device_,0);
}

Err
svc_mem_r_u8_unit(Item  device_,
                  u8    unit_,
                  i32   offset_,
                  u8   *dst_,
                  i32   len_)
{
  Err rv;
  Item ioreq;
  IOInfo ioi = {0};

  ioreq = svc_mem_create_ioreq(device_);
  if(ioreq < 0)
    return ioreq;

  ioi.ioi_Command         = CMD_READ;
  ioi.ioi_Unit            = unit_;
  ioi.ioi_Offset          = offset_;
  ioi.ioi_Recv.iob_Buffer = (void*)dst_;
  ioi.ioi_Recv.iob_Len    = len_;

  rv = DoIO(ioreq,&ioi);

  DeleteIOReq(ioreq);

  return rv;
}

Err
svc_mem_r_u32_unit(Item  device_,
                   u8    unit_,
                   i32   offset_,
                   u32   *dst_,
                   i32   len_)
{
  Err rv;
  Item ioreq;
  IOInfo ioi = {0};

  ioreq = svc_mem_create_ioreq(device_);
  if(ioreq < 0)
    return ioreq;

  ioi.ioi_Command         = CMD_READ;
  ioi.ioi_CmdOptions     |= SVC_MEM_CMD_FLAG_WORDS;
  ioi.ioi_Unit            = unit_;
  ioi.ioi_Offset          = offset_;
  ioi.ioi_Recv.iob_Buffer = (void*)dst_;
  ioi.ioi_Recv.iob_Len    = len_;

  rv = DoIO(ioreq,&ioi);

  DeleteIOReq(ioreq);

  return rv;
}

Err
svc_mem_r_u8(Item  device_,
             u8   *src_,
             i32   offset_,
             u8   *dst_,
             i32   len_)
{
  Err rv;
  Item ioreq;
  IOInfo ioi = {0};

  ioreq = svc_mem_create_ioreq(device_);
  if(ioreq < 0)
    return ioreq;

  ioi.ioi_Command         = CMD_READ;
  ioi.ioi_Unit            = SVC_MEM_UNIT_NONE;
  ioi.ioi_Offset          = offset_;
  ioi.ioi_Send.iob_Buffer = (void*)src_;
  ioi.ioi_Recv.iob_Buffer = (void*)dst_;
  ioi.ioi_Recv.iob_Len    = len_;

  rv = DoIO(ioreq,&ioi);

  DeleteIOReq(ioreq);

  return rv;
}

Err
svc_mem_r_u32(Item  device_,
              u32  *src_,
              i32   offset_,
              u32  *dst_,
              i32   len_)
{
  Err rv;
  Item ioreq;
  IOInfo ioi = {0};

  ioreq = svc_mem_create_ioreq(device_);
  if(ioreq < 0)
    return ioreq;

  ioi.ioi_Command         = CMD_READ;
  ioi.ioi_CmdOptions     |= SVC_MEM_CMD_FLAG_WORDS;
  ioi.ioi_Unit            = SVC_MEM_UNIT_NONE;
  ioi.ioi_Offset          = offset_;
  ioi.ioi_Send.iob_Buffer = (void*)src_;
  ioi.ioi_Recv.iob_Buffer = (void*)dst_;
  ioi.ioi_Recv.iob_Len    = len_;

  rv = DoIO(ioreq,&ioi);

  DeleteIOReq(ioreq);

  return rv;
}

Err
svc_mem_r_u8_dram(Item  device_,
                  i32   offset_,
                  u8   *dst_,
                  i32   len_)
{
  return svc_mem_r_u8_unit(device_,
                           SVC_MEM_UNIT_DRAM,
                           offset_,
                           dst_,
                           len_);
}

Err
svc_mem_r_u32_dram(Item  device_,
                   i32   offset_,
                   u32  *dst_,
                   i32   len_)
{
  return svc_mem_r_u32_unit(device_,
                            SVC_MEM_UNIT_DRAM,
                            offset_,
                            dst_,
                            len_);
}

Err
svc_mem_r_u8_vram(Item  device_,
                  i32   offset_,
                  u8   *dst_,
                  i32   len_)
{
  return svc_mem_r_u8_unit(device_,
                           SVC_MEM_UNIT_VRAM,
                           offset_,
                           dst_,
                           len_);
}

Err
svc_mem_r_u32_vram(Item  device_,
                   i32   offset_,
                   u32  *dst_,
                   i32   len_)
{
  return svc_mem_r_u32_unit(device_,
                            SVC_MEM_UNIT_VRAM,
                            offset_,
                            dst_,
                            len_);
}

Err
svc_mem_r_u8_rom1(Item  device_,
                  i32   offset_,
                  u8   *dst_,
                  i32   len_)
{
  return svc_mem_r_u8_unit(device_,
                           SVC_MEM_UNIT_ROM1,
                           offset_,
                           dst_,
                           len_);
}

Err
svc_mem_r_u32_rom1(Item  device_,
                   i32   offset_,
                   u32   *dst_,
                   i32   len_)
{
  return svc_mem_r_u32_unit(device_,
                            SVC_MEM_UNIT_ROM1,
                            offset_,
                            dst_,
                            len_);
}

Err
svc_mem_r_u8_rom2(Item  device_,
                  i32   offset_,
                  u8   *dst_,
                  i32   len_)
{
  return svc_mem_r_u8_unit(device_,
                           SVC_MEM_UNIT_ROM2,
                           offset_,
                           dst_,
                           len_);
}

Err
svc_mem_r_u32_rom2(Item  device_,
                   i32   offset_,
                   u32   *dst_,
                   i32   len_)
{
  return svc_mem_r_u32_unit(device_,
                            SVC_MEM_UNIT_ROM2,
                            offset_,
                            dst_,
                            len_);
}

Err
svc_mem_r_u8_nvram(Item  device_,
                i32   offset_,
                u8   *dst_,
                i32   len_)
{
  return svc_mem_r_u8_unit(device_,
                           SVC_MEM_UNIT_NVRAM,
                           offset_,
                           dst_,
                           len_);
}

Err
svc_mem_r_u32_madam(Item  device_,
                i32   offset_,
                u32   *dst_,
                i32   len_)
{
  return svc_mem_r_u32_unit(device_,
                            SVC_MEM_UNIT_MADAM,
                            offset_,
                            dst_,
                            len_);
}

Err
svc_mem_r_u32_clio(Item  device_,
               i32   offset_,
               u32   *dst_,
               i32   len_)
{
  return svc_mem_r_u32_unit(device_,
                            SVC_MEM_UNIT_CLIO,
                            offset_,
                            dst_,
                            len_);
}

Err
svc_mem_r_u32_sport(Item  device_,
                i32   offset_,
                u32   *dst_,
                i32   len_)
{
  return svc_mem_r_u32_unit(device_,
                            SVC_MEM_UNIT_SPORT,
                            offset_,
                            dst_,
                            len_);
}


Err
svc_mem_w_u8(Item  device_,
             u8   *src_,
             i32   len_,
             u8   *dst_,
             i32   offset_)
{
  Err rv;
  Item ioreq;
  IOInfo ioi = {0};

  ioreq = svc_mem_create_ioreq(device_);
  if(ioreq < 0)
    return ioreq;

  ioi.ioi_Command         = CMD_WRITE;
  ioi.ioi_Unit            = SVC_MEM_UNIT_NONE;
  ioi.ioi_Send.iob_Buffer = src_;
  ioi.ioi_Send.iob_Len    = len_;
  ioi.ioi_Recv.iob_Buffer = dst_;
  ioi.ioi_Offset          = offset_;

  rv = DoIO(ioreq,&ioi);

  DeleteIOReq(ioreq);

  return rv;
}

Err
svc_mem_w_u32(Item  device_,
              u32  *src_,
              i32   len_,
              u32  *dst_,
              i32   offset_)
{
  Err rv;
  Item ioreq;
  IOInfo ioi = {0};

  ioreq = svc_mem_create_ioreq(device_);
  if(ioreq < 0)
    return ioreq;

  ioi.ioi_Command         = CMD_WRITE;
  ioi.ioi_CmdOptions     |= SVC_MEM_CMD_FLAG_WORDS;
  ioi.ioi_Unit            = SVC_MEM_UNIT_NONE;
  ioi.ioi_Send.iob_Buffer = src_;
  ioi.ioi_Send.iob_Len    = len_;
  ioi.ioi_Recv.iob_Buffer = dst_;
  ioi.ioi_Offset          = offset_;

  rv = DoIO(ioreq,&ioi);

  DeleteIOReq(ioreq);

  return rv;
}

Err
svc_mem_w1_u32(Item  device_,
               u32   src_,
               u32  *dst_,
               i32   offset_)
{
  return svc_mem_w_u32(device_,&src_,1,dst_,offset_);
}

Err
svc_mem_w_u8_unit(Item  device_,
                  u8   *src_,
                  i32   len_,
                  u8    unit_,
                  i32   offset_)
{
  Err rv;
  Item ioreq;
  IOInfo ioi = {0};

  ioreq = svc_mem_create_ioreq(device_);
  if(ioreq < 0)
    return ioreq;

  ioi.ioi_Command         = CMD_WRITE;
  ioi.ioi_Send.iob_Buffer = src_;
  ioi.ioi_Send.iob_Len    = len_;
  ioi.ioi_Unit            = unit_;
  ioi.ioi_Offset          = offset_;

  rv = DoIO(ioreq,&ioi);

  DeleteIOReq(ioreq);

  return rv;
}

Err
svc_mem_w_u32_unit(Item  device_,
                   u32  *src_,
                   i32   len_,
                   u8    unit_,
                   i32   offset_)
{
  Err rv;
  Item ioreq;
  IOInfo ioi = {0};

  ioreq = svc_mem_create_ioreq(device_);
  if(ioreq < 0)
    return ioreq;

  ioi.ioi_Command         = CMD_WRITE;
  ioi.ioi_CmdOptions     |= SVC_MEM_CMD_FLAG_WORDS;
  ioi.ioi_Send.iob_Buffer = src_;
  ioi.ioi_Send.iob_Len    = len_;
  ioi.ioi_Unit            = unit_;
  ioi.ioi_Offset          = offset_;

  rv = DoIO(ioreq,&ioi);

  DeleteIOReq(ioreq);

  return rv;
}

Err
svc_mem_w_u8_dram(Item  device_,
                  u8   *src_,
                  i32   len_,
                  i32   offset_)
{
  return svc_mem_w_u8_unit(device_,
                           src_,
                           len_,
                           SVC_MEM_UNIT_DRAM,
                           offset_);
}

Err
svc_mem_w_u32_dram(Item  device_,
                   u32  *src_,
                   i32   len_,
                   i32   offset_)
{
  return svc_mem_w_u32_unit(device_,
                            src_,
                            len_,
                            SVC_MEM_UNIT_DRAM,
                            offset_);
}

Err
svc_mem_w_u8_vram(Item  device_,
                  u8   *src_,
                  i32   len_,
                  i32   offset_)
{
  return svc_mem_w_u8_unit(device_,
                           src_,
                           len_,
                           SVC_MEM_UNIT_VRAM,
                           offset_);
}

Err
svc_mem_w_u32_vram(Item  device_,
                   u32  *src_,
                   i32   len_,
                   i32   offset_)
{
  return svc_mem_w_u32_unit(device_,
                            src_,
                            len_,
                            SVC_MEM_UNIT_VRAM,
                            offset_);
}

Err
svc_mem_w_u8_nvram(Item  device_,
                   u8   *src_,
                   i32   len_,
                   i32   offset_)
{
  return svc_mem_w_u8_unit(device_,
                           src_,
                           len_,
                           SVC_MEM_UNIT_NVRAM,
                           offset_);
}

Err
svc_mem_w_u32_madam(Item  device_,
                    u32  *src_,
                    i32   len_,
                    i32   offset_)
{
  return svc_mem_w_u32_unit(device_,
                            src_,
                            len_,
                            SVC_MEM_UNIT_MADAM,
                            offset_);
}

Err
svc_mem_w_u32_clio(Item  device_,
                   u32  *src_,
                   i32   len_,
                   i32   offset_)
{
  return svc_mem_w_u32_unit(device_,
                            src_,
                            len_,
                            SVC_MEM_UNIT_CLIO,
                            offset_);
}

Err
svc_mem_w_u32_sport(Item  device_,
                    u32  *src_,
                    i32   len_,
                    i32   offset_)
{
  return svc_mem_w_u32_unit(device_,
                            src_,
                            len_,
                            SVC_MEM_UNIT_SPORT,
                            offset_);
}
