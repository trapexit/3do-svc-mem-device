# svc_mem device

This is a 3DO Portfolio OS device driver which mimic the internal
device driver called `raw` which abstracts access to memory with
custom read and write functions for accessing each type of memory
(DRAM, VRAM, NVRAM, ROM, and memory mapped devices.)

The primary difference is that this library allows unfettered access
to the hardware without any authorization checks. Meaning you can read
the ROMS, read or write to NVRAM directly rather than through the
filesystem, and interact with any memory mapped hardware like MADAM,
CLIO, SPORT, etc.

While useful for interacting with the system the primary purpose it to
demonstrate building a device driver with supervisor privileges. For
general interaction with hardware it would probably be best to
leverage a [folio](https://github.com/trapexit/3do-example-folio). The
dispatch overhead will be lower than using the IO subsystem as device
drivers do.

## API

See the [svc_mem.h header
file](https://github.com/trapexit/3do-svc-mem-device/blob/master/src/svc_mem.h)
and the `read_rom` and `overwrite_folio_func` examples in the [3do-devkit](https://github.com/trapexit/3do-devkit)

## Building

1. Get [3do-devkit](https://github.com/trapexit/3do-devkit)
2. `source 3do-devkit/activate-env`
3. `make`
4. `make install` will install the header and library into 3do-devkit
