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

#include "svc_mem_drv.h"
#include "svc_mem_dev.h"

#include "debug.h"
#include "operror.h"
#include "task.h"

#define NAME "svc-mem"
static const char VERSION[] = "1.0.0 " __DATE__ " " __TIME__;

int
main()
{
  Item drv;
  Item dev;
  i32 signal;
  i32 rxsignal;

  kprintf(NAME ": starting - version='%s'\n",VERSION);

  drv = svc_mem_drv_create();
  if(drv <= 0)
    {
      kprintf(NAME ": create driver failed - ");
      PrintfSysErr(drv);
      return 0;
    }
  else
    {
      kprintf(NAME ": drv_item=%x;\n",drv);
    }

  dev = svc_mem_dev_create(drv);
  if(dev <= 0)
    {
      kprintf(NAME ": create device failed - ");
      PrintfSysErr(dev);
      return 0;
    }
  else
    {
      kprintf(NAME ": dev_item=%x;\n",dev);
    }

  signal = AllocSignal(0);
  if(signal <= 0)
    {
      kprintf(NAME ": unable to alloc signal - ");
      PrintfSysErr(signal);
      return 0;
    }

  kprintf(NAME ": entering wait signal loop - drv_item=%x; dev_item=%x\n",
          drv,
          dev);
  for(;;)
    {
      rxsignal = WaitSignal(signal);
      if(rxsignal & SIGF_ABORT)
        {
          kprintf(NAME ": SIGF_ABORT received\n");
          return 0;
        }
      else
        {
          kprintf(NAME ": received signal %x - ignoring\n",rxsignal);
        }
    }

  kprintf(NAME ": exited main loop - drv_item=%x; dev_item=%x\n",
          drv,
          dev);

  return 0;
}
