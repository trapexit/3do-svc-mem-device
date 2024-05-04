#include "svc_mem_drv_opts.h"
#include "svc_mem_dev.h"

#include "svc_funcs.h"

#include "portfolio.h"
#include "strings.h"

#define SVC_MEM_DEV_NAME "svc-mem-dev"

enum device_tags_e
  {
    CREATEDEVICE_TAG_DRVR = TAG_ITEM_LAST+1, // 0x0A
    CREATEDEVICE_TAG_CRIO,                   // 0x0B - create IO routine
    CREATEDEVICE_TAG_DLIO,                   // 0x0C - delete IO routine
    CREATEDEVICE_TAG_OPEN,                   // 0x0D - open routine
    CREATEDEVICE_TAG_CLOSE,                  // 0x0E - close routine
    CREATEDEVICE_TAG_IOREQSZ,                // 0x0F - optional, request size
    CREATEDEVICE_TAG_INIT                    // 0x10 - init routine
  };

static
i32
dev_init(struct Device *dev_)
{
  svc_kprintf(SVC_MEM_DEV_NAME ": dev_init\n");

  dev_->dev_OpenCnt = 0;
  dev_->dev_MaxUnitNum = SVC_MEM_UNIT_MAX;

  return dev_->dev.n_Item;
}

static
i32
dev_open(struct Device *dev_)
{
  svc_kprintf(SVC_MEM_DEV_NAME ": dev_open - "
              "opencnt=%d; "
              "driver=%p;"
              "\n",
              dev_->dev_OpenCnt,
              dev_->dev_Driver);

  return dev_->dev.n_Item;
}

static
void
dev_close(struct Device *dev_)
{
  svc_kprintf(SVC_MEM_DEV_NAME ": dev_close - "
              "opencnt=%d; "
              "driver=%p;"
              "\n",
              dev_->dev_OpenCnt,
              dev_->dev_Driver);
}

Item
svc_mem_dev_create(Item driver_)
{
  static TagArg dev_tags[] =
    {
      {TAG_ITEM_PRI,           (void*)150},              // 0
      {CREATEDEVICE_TAG_DRVR,  (void*)0},                // 1
      {TAG_ITEM_NAME,          (void*)SVC_MEM_DEV_NAME}, // 2
      {CREATEDEVICE_TAG_INIT,  (void*)dev_init},         // 3
      {CREATEDEVICE_TAG_OPEN,  (void*)dev_open},         // 4
      {CREATEDEVICE_TAG_CLOSE, (void*)dev_close},        // 5
      {TAG_END,                (void*)0}
    };

  dev_tags[1].ta_Arg = (void*)driver_;

  return CreateItem(MKNODEID(KERNELNODE,DEVICENODE),dev_tags);
}
