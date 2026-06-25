/*
 *  prom.c - Routines for talking to the Open Firmware PROM
 *
 *  Copyright (C) 2001, 2002 Ethan Benson
 *
 *  Copyright (C) 1999 Benjamin Herrenschmidt
 *
 *  Copyright (C) 1999 Marius Vollmer
 *
 *  Copyright (C) 1996 Paul Mackerras.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "prom.h"
#include "stdarg.h"
#include "stddef.h"
#include "stdlib.h"
#include "types.h"
#include "ctype.h"
#include "asm/processor.h"
#include "errors.h"
#include "debug.h"
#include "string.h"
#include <limits.h>

static int yaboot_debug;

prom_entry prom;

ihandle prom_stdin, prom_stdout;

static ihandle prom_mem, prom_mmu;
static ihandle prom_chosen, prom_options;

struct prom_args {
     const char *service; /* TODO */
     int nargs;
     int nret;
     prom_cell_t args[10];
};

#if defined(PPC32)
#define char_to_cell(p) (prom_cell_t)((uintptr_t) p)
#elif defined(PPC64)
#error TODO
static prom_cell_t char_to_cell(unsigned char *chr)
{
     uintptr_t ptr = (uintptr_t) chr;
     if(ptr < UINT32_MAX) return ptr;
     // copy ...
     return ptr;
}
#endif

prom_cell_t
call_prom (const char *service, size_t nargs, size_t nret, ...)
{
     va_list list;
     size_t i;
     struct prom_args prom_args;

     prom_args.service = service;
     prom_args.nargs = nargs;
     prom_args.nret = nret;
     va_start (list, nret);
     for (i = 0; i < nargs; ++i)
	  prom_args.args[i] = va_arg(list, prom_cell_t);
     va_end(list);
     for (i = 0; i < nret; ++i)
	  prom_args.args[i + nargs] = 0;
     prom (&prom_args);
     if (nret > 0)
	  return prom_args.args[nargs];
     else
	  return 0;
}

prom_cell_t
call_prom_return (const char *service, size_t nargs, size_t nret, ...)
{
     va_list list;
     size_t i;
     prom_cell_t result;
     struct prom_args prom_args;

     prom_args.service = service;
     prom_args.nargs = nargs;
     prom_args.nret = nret;
     va_start (list, nret);
     for (i = 0; i < nargs; ++i)
	  prom_args.args[i] = va_arg(list, prom_cell_t);
     for (i = 0; i < nret; ++i)
	  prom_args.args[i + nargs] = 0;
     if (prom (&prom_args) != 0)
	  return PROM_INVALID_HANDLE;
     if (nret > 0) {
	  result = prom_args.args[nargs];
	  for (i=1; i<nret; i++) {
	       prom_cell_t* rp = va_arg(list, prom_cell_t*);
	       *rp = prom_args.args[i+nargs];
	  }
     } else
	  result = 0;
     va_end(list);
     return result;
}

static prom_cell_t
call_method_1 (const char *method, prom_handle h, int nargs, ...)
{
     va_list list;
     int i;
     struct prom_args prom_args;

     prom_args.service = "call-method";
     prom_args.nargs = nargs+2;
     prom_args.nret = 2;
     prom_args.args[0] = char_to_cell(method);
     prom_args.args[1] = h;
     va_start (list, nargs);
     for (i = 0; i < nargs; ++i)
	  prom_args.args[2+i] = va_arg(list, prom_cell_t);
     va_end(list);
     prom_args.args[2+nargs] = 0;
     prom_args.args[2+nargs+1] = 0;

     prom (&prom_args);

     if (prom_args.args[2+nargs] != 0)
     {
	  prom_printf ("method '%s' failed %d\n", method, prom_args.args[2+nargs]);
	  return 0;
     }
     return prom_args.args[2+nargs+1];
}

prom_handle
prom_finddevice (const char *name)
{
     return call_prom ("finddevice", 1, 1, name);
}

prom_handle
prom_findpackage(const char *path)
{
     return call_prom ("find-package", 1, 1, path);
}

int32_t
prom_getprop (prom_handle pack, const char *name, void *mem, uint32_t len)
{
     return (int32_t)call_prom ("getprop", 4, 1, pack, name, mem, len);
}

int32_t
prom_getproplen(prom_handle pack, const char *name)
{
     return (int32_t)call_prom("getproplen", 2, 1, pack, name);
}

int32_t
prom_setprop (prom_handle pack, const char *name, void *mem, uint32_t len)
{
     return (int32_t)call_prom ("setprop", 4, 1, pack, name, mem, len);
}

int32_t
prom_get_chosen (const char *name, void *mem, uint32_t len)
{
     return prom_getprop (prom_chosen, name, mem, len);
}

int32_t
prom_get_options (const char *name, void *mem, uint32_t len)
{
     if (prom_options == PROM_INVALID_HANDLE)
	  return -1;
     return prom_getprop (prom_options, name, mem, len);
}

int32_t
prom_set_options (const char *name, void *mem, uint32_t len)
{
     if (prom_options == PROM_INVALID_HANDLE)
	  return -1;
     return prom_setprop (prom_options, name, mem, len);
}

int32_t
prom_get_devtype (const char *device)
{
     phandle    dev;
     int32_t    result;
     char       tmp[64];

     if (strstr(device, TOK_ISCSI))
	  return FILE_DEVICE_ISCSI;

     /* Find OF device phandle */
     dev = prom_finddevice(device);
     if (dev == PROM_INVALID_HANDLE) {
	  return FILE_ERR_BADDEV;
     }

     /* Check the kind of device */
     result = prom_getprop(dev, "device_type", tmp, 63);
     if (result == -1) {
	  prom_printf("can't get <device_type> for device: %s\n", device);
	  return FILE_ERR_BADDEV;
     }
     tmp[result] = 0;
     if (!strcmp(tmp, "block"))
	  return FILE_DEVICE_BLOCK;
     else if (!strcmp(tmp, "network"))
	  return FILE_DEVICE_NET;
     else {
	  prom_printf("Unknown device type <%s>\n", tmp);
	  return FILE_ERR_BADDEV;
     }
}

void
prom_init (prom_entry pp)
{
     prom = pp;

     prom_chosen = prom_finddevice ("/chosen");
     if (prom_chosen == PROM_INVALID_HANDLE)
	  prom_exit ();
     prom_options = prom_finddevice ("/options");
     if (prom_get_chosen ("stdout", &prom_stdout, sizeof(prom_stdout)) <= 0)
	  prom_exit();
     if (prom_get_chosen ("stdin", &prom_stdin, sizeof(prom_stdin)) <= 0)
	  prom_abort ("\nCan't open stdin");
     if (prom_get_chosen ("memory", &prom_mem, sizeof(prom_mem)) <= 0)
	  prom_abort ("\nCan't get mem handle");
     if (prom_get_chosen ("mmu", &prom_mmu, sizeof(prom_mmu)) <= 0)
	  prom_abort ("\nCan't get mmu handle");

     yaboot_debug = 0;
     prom_get_options("linux,yaboot-debug", &yaboot_debug, sizeof(yaboot_debug));

  // move cursor to fresh line
     prom_printf ("\n");

     /* Add a few OF methods (thanks Darwin) */
#if DEBUG
     prom_printf ("Adding OF methods...\n");
#endif

     prom_interpret (
	  /* All values in this forth code are in hex */
	  "hex "
	  /* Those are a few utilities ripped from Apple */
	  ": D2NIP decode-int nip nip ;\r"	// A useful function to save space
	  ": GPP$ get-package-property 0= ;\r"	// Another useful function to save space
	  ": ^on0 0= if -1 throw then ;\r"	// Bail if result zero
	  ": $CM $call-method ;\r"
	  );

     /* Some forth words used by the release method */
     prom_interpret (
	  " \" /chosen\" find-package if "
		 "dup \" memory\" rot GPP$ if "
			 "D2NIP swap "				 // ( MEMORY-ihandle "/chosen"-phandle )
			 "\" mmu\" rot GPP$ if "
				 "D2NIP "				 // ( MEMORY-ihandle MMU-ihandle )
			 "else "
				 "0 "					 // ( MEMORY-ihandle 0 )
			 "then "
		 "else "
			 "0 0 "						 // ( 0 0 )
		 "then "
	  "else "
		 "0 0 "							 // ( 0 0 )
	  "then\r"
	  "value mmu# "
	  "value mem# "
	  );

     prom_interpret (
	  ": ^mem mem# $CM ; "
	  ": ^mmu mmu# $CM ; "
	  );

     DEBUG_F("OF interface initialized.\n");
}

ihandle
prom_open (const char *spec)
{
     return call_prom ("open", 1, 1, spec, strlen(spec));
}

void
prom_close (ihandle file)
{
     call_prom ("close", 1, 0, file);
}

static ssize_t
prom_read32 (ihandle file, void *buf, uint32_t n)
{
     ssize_t result = 0;
     int retries = 10;

     if (n == 0)
	  return 0;
     while(--retries) {
	  result = (int)call_prom ("read", 3, 1, file, buf, n);
	  if (result != 0)
	       break;
	  call_prom("interpret", 1, 1, " 10 ms");
     }

     return result;
}

ssize_t
prom_write32 (ihandle file, void *buf, uint32_t n)
{
     return (ssize_t)call_prom ("write", 3, 1, file, buf, n);
}

typedef ssize_t (*prom_byte_processor)(ihandle file, void *buf, uint32_t n);

off_t
prom_process_bytes(prom_byte_processor bp, ihandle file, void *buf, off_t n)
{
     void *cur = buf;
     size_t todo  = n;
     size_t count = 0;

     do {
          int32_t chunk, procd;

          chunk = todo > INT_MAX ? INT_MAX : todo;
          procd = bp(file, cur, chunk);

          if(procd < 0) break;
          count += procd;
          todo  -= procd;
          if(procd < chunk) break;

     } while(todo > 0);

     return count;
}

ssize_t
prom_read (ihandle file, void *buf, off_t n)
{
     return (ssize_t)prom_process_bytes (prom_read32, file, buf, n);
}

ssize_t
prom_write (ihandle file, void *buf, off_t n)
{
     return (ssize_t)prom_process_bytes (prom_write32, file, buf, n);
}

off_t
prom_lseek (ihandle file, off_t pos)
{
     int32_t status = (int32_t)call_prom ("seek", 3, 1, file,
				  (uint32_t)(pos >> 32), (uint32_t)(pos & 0xffffffffUL));

     if(status == 0 || status == 1) return pos;
     return -1;
}

off_t prom_getfilesize(ihandle file)
{
     uint32_t hi, lo;

     int32_t status = (int32_t)call_prom_return("call-method", 2, 3, "size", file, &hi, &lo);
     
     if((status != 0) || (hi == ~0U && lo == ~0U)) return -1;
     return ((unsigned long long)hi << 32) | lo;
}

int32_t
prom_loadmethod (ihandle device, prom_addr_t addr)
{
     return (int32_t)call_method_1 ("load", device, 1, addr);
}

int32_t
prom_getblksize (ihandle file)
{
     return (int32_t)call_method_1 ("block-size", file, 0);
}

int32_t
prom_getchar ()
{
     char c;
     int32_t a;

     while ((a = (int32_t)call_prom ("read", 3, 1, prom_stdin, &c, 1)) == 0)
	  ;
     if (a == -1)
	  prom_abort ("EOF on console\n");
     return c;
}

int32_t
prom_nbgetchar()
{
     char ch;

     return (int32_t) call_prom("read", 3, 1, prom_stdin, &ch, 1) > 0? ch: -1;
}

void
prom_putchar (char c)
{
     if (c == '\n')
	  call_prom ("write", 3, 1, prom_stdout, "\r\n", 2);
     else
	  call_prom ("write", 3, 1, prom_stdout, &c, 1);
}

void
prom_puts (prom_handle file, const char *s)
{
     const char *p, *q;

     for (p = s; *p != 0; p = q)
     {
	  for (q = p; *q != 0 && *q != '\n'; ++q)
	       ;
	  if (q > p)
	       call_prom ("write", 3, 1, file, p, q - p);
	  if (*q != 0)
	  {
	       ++q;
	       call_prom ("write", 3, 1, file, "\r\n", 2);
	  }
     }
}

void
prom_vfprintf (prom_handle file, const char *fmt, va_list ap)
{
     static char printf_buf[2048];
     vsprintf (printf_buf, fmt, ap);
     prom_puts (file, printf_buf);
}

void
prom_vprintf (const char *fmt, va_list ap)
{
     static char printf_buf[2048];
     vsprintf (printf_buf, fmt, ap);
     prom_puts (prom_stdout, printf_buf);
}

void
prom_fprintf (prom_handle file, const char *fmt, ...)
{
     va_list ap;
     va_start (ap, fmt);
     prom_vfprintf (file, fmt, ap);
     va_end (ap);
}

void
prom_printf (const char *fmt, ...)
{
     va_list ap;
     va_start (ap, fmt);
     prom_vfprintf (prom_stdout, fmt, ap);
     va_end (ap);
}

void
prom_debug (const char *fmt, ...)
{
     va_list ap;

     if (!yaboot_debug)
          return;

     va_start (ap, fmt);
     prom_vfprintf (prom_stdout, fmt, ap);
     va_end (ap);
}

void
prom_perror (int error, const char *filename)
{
     if (error == FILE_ERR_EOF)
	  prom_printf("%s: Unexpected End Of File\n", filename);
     else if (error == FILE_ERR_NOTFOUND)
	  prom_printf("%s: No such file or directory\n", filename);
     else if (error == FILE_CANT_SEEK)
	  prom_printf("%s: Seek error\n", filename);
     else if (error == FILE_IOERR)
	  prom_printf("%s: Input/output error\n", filename);
     else if (error == FILE_BAD_PATH)
	  prom_printf("%s: Path too long\n", filename);
     else if (error == FILE_ERR_BAD_TYPE)
	  prom_printf("%s: Not a regular file\n", filename);
     else if (error == FILE_ERR_NOTDIR)
	  prom_printf("%s: Not a directory\n", filename);
     else if (error == FILE_ERR_BAD_FSYS)
	  prom_printf("%s: Unknown or corrupt filesystem\n", filename);
     else if (error == FILE_ERR_SYMLINK_LOOP)
	  prom_printf("%s: Too many levels of symbolic links\n", filename);
     else if (error == FILE_ERR_LENGTH)
	  prom_printf("%s: File too large\n", filename);
     else if (error == FILE_ERR_FSBUSY)
	  prom_printf("%s: Filesystem busy\n", filename);
     else if (error == FILE_ERR_BADDEV)
	  prom_printf("%s: Unable to open file, Invalid device\n", filename);
     else
	  prom_printf("%s: Unknown error\n", filename);
}

void
prom_readline (const char *prompt, char *buf, uint32_t len)
{
     uint32_t i = 0;
     uint32_t c;

     if (prompt)
	  prom_puts (prom_stdout, prompt);

     while (i < len-1 && (c = prom_getchar ()) != '\r')
     {
	  if (c == 8)
	  {
	       if (i > 0)
	       {
		    prom_puts (prom_stdout, "\b \b");
		    i--;
	       }
	       else
		    prom_putchar ('\a');
	  }
	  else if (isprint (c))
	  {
	       prom_putchar (c);
	       buf[i++] = c;
	  }
	  else
	       prom_putchar ('\a');
     }
     prom_putchar ('\n');
     buf[i] = 0;
}

#ifdef CONFIG_SET_COLORMAP
int32_t
prom_set_color(prom_handle device, uint32_t color, uint32_t r, uint32_t g, uint32_t b)
{
     return (int)call_prom( "call-method", 6, 1, "color!", device, color, b, g, r );
}
#endif /* CONFIG_SET_COLORMAP */

void
prom_exit ()
{
     call_prom ("exit", 0, 0);
}

void
prom_abort (const char *fmt, ...)
{
     va_list ap;
     va_start (ap, fmt);
     prom_vfprintf (prom_stdout, fmt, ap);
     va_end (ap);
     prom_exit ();
}

void
prom_sleep (unsigned int seconds)
{
     int end; /* FIXME */
     end = (prom_getms() + (seconds * 1000));
     while (prom_getms() <= end);
}

/* if address given is claimed look for other addresses to get the needed
 * space before giving up
 */
prom_addr_t
prom_claim_chunk(prom_addr_t virt, prom_size_t size, prom_size_t align)
{
     (void) align;

     prom_addr_t found, addr;
     for(addr=virt; addr <= PROM_CLAIM_MAX_ADDR;
         addr+=(0x100000/sizeof(addr))) {
          found = (prom_addr_t)  call_prom("claim", 3, 1, addr, size, 0);
          if (found != PROM_INVALID_ADDR) {
               prom_debug("claim of 0x%x at 0x%x returned 0x%x\n", size, (int)addr, (int)found);
               return(found);
          }
     }
     prom_printf("ERROR: claim of 0x%x in range 0x%x-0x%x failed\n", size, (int)virt, (int)PROM_CLAIM_MAX_ADDR);
     return PROM_INVALID_ADDR;
}

/* Start from top of memory and work down to get the needed space */
prom_addr_t
prom_claim_chunk_top(prom_size_t size, prom_size_t align)
{
     (void) align;

     prom_addr_t found, addr;
     for(addr=PROM_CLAIM_MAX_ADDR; (int) addr >= size;
         addr-=(0x100000/sizeof(addr))) {
          found = (prom_addr_t) call_prom("claim", 3, 1, addr, size, 0);
          if (found != PROM_INVALID_ADDR) {
               prom_debug("claim of 0x%x at 0x%x returned 0x%x\n", size, (int)addr, (int)found);
               return(found);
          }
     }
     prom_printf("ERROR: claim of 0x%x in range 0x0-0x%x failed\n", size, (int)PROM_CLAIM_MAX_ADDR);
     return PROM_INVALID_ADDR;
}

prom_addr_t
prom_claim (prom_addr_t virt, prom_size_t size, prom_size_t align)
{
     prom_addr_t ret;

     ret = (prom_addr_t) call_prom ("claim", 3, 1, virt, size, align);
     if (ret == PROM_INVALID_ADDR)
          prom_printf("ERROR: claim of 0x%x at 0x%x failed\n", size, (int)virt);
     else
          prom_debug("claim of 0x%x at 0x%x returned 0x%x\n", size, (int)virt, (int)ret);

     return ret;
}

void
prom_release(prom_addr_t virt, prom_size_t size)
{
     prom_cell_t ret;

     ret = call_prom ("release", 2, 0, virt, size);
     prom_debug("release of 0x%x at 0x%x returned 0x%x\n", size, (int)virt, (int)ret);
}

void
prom_map (prom_addr_t phys, prom_addr_t virt, prom_size_t size)
{
     unsigned long msr = mfmsr();

     /* Only create a mapping if we're running with relocation enabled. */
     if ( (msr & MSR_IR) && (msr & MSR_DR) )
	  call_method_1 ("map", prom_mmu, 4, -1, size, virt, phys);
}

void
prom_unmap (void *phys, void *virt, uint32_t size)
{
     unsigned long msr = mfmsr();

     /* Only unmap if we're running with relocation enabled. */
     if ( (msr & MSR_IR) && (msr & MSR_DR) )
	  call_method_1 ("map", prom_mmu, 4, -1, size, virt, phys);
}

char *
prom_getargs ()
{
     static char args[256];
     int l;

     l = prom_get_chosen ("bootargs", args, 255);
     args[l] = '\0';
     return args;
}

void
prom_setargs (const char *args)
{
     int l = strlen (args)+1;
     if ((int)call_prom ("setprop", 4, 1, prom_chosen, "bootargs", args, l) != l)
	  prom_printf ("can't set args\n");
}

int32_t 
prom_interpret (const char *forth)
{
     return (int32_t)call_prom("interpret", 1, 1, forth);
}

int32_t
prom_getms(void)
{
     return (int32_t) call_prom("milliseconds", 0, 1);
}

void
prom_pause(void)
{
     prom_print_available();
     call_prom("enter", 0, 0);
}

/*
 * prom_get_netinfo()
 * returns the packet with all needed info for netboot
 */
struct bootp_packet * prom_get_netinfo (void)
{
     void *bootp_response = NULL;
     char *propname;
     struct bootp_packet *packet;
     /* struct bootp_packet contains a VLA, so sizeof won't work.
        the VLA /must/ be the last field in the structure so use it's
        offset as a good estimate of the packet size */
     size_t packet_size = offsetof(struct bootp_packet, options);
     size_t size = 0, offset = 0;
     prom_handle chosen;
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

     chosen = prom_finddevice("/chosen");
     if ((int32_t) chosen < 0) {
          DEBUG_F("chosen=%lu\n", (unsigned long)chosen);
          return 0;
     }

     for (size_t i = 0; i < ARRAY_SIZE(bootp_response_properties); i++) {
         propname = bootp_response_properties[i].name;
         ssize_t _size = size = prom_getproplen(chosen, propname);
         if (_size <= 0)
             continue;
         size = _size;

         DEBUG_F("using /chosen/%s\n", propname);
         offset = bootp_response_properties[i].offset;
         break;
     }

     if (size <= 0)
         return NULL;

     if (packet_size > size - offset) {
         prom_printf("Malformed %s property?\n", propname);
         return NULL;
     }

     bootp_response = malloc(size);
     if (!bootp_response)
         return NULL;

     if (prom_getprop(chosen, propname, bootp_response, size) < 0)
         return NULL;

     packet = bootp_response + offset;
     return packet;
}

/*
 * prom_get_mac()
 * returns the mac addr of an net card
 */
char * prom_get_mac (struct bootp_packet * packet)
{
     char * conf_path;

     if (!packet)
        return NULL;

     /* 3 chars per byte in chaddr + \0 */
     conf_path = malloc(packet->hlen * 3 + 1);
     if (!conf_path)
         return NULL;
     sprintf(conf_path, "%02x", packet->chaddr[0]);

     for (size_t i = 1; i < packet->hlen; i++) {
      char tmp[4];
      sprintf(tmp, "-%02x", packet->chaddr[i]);
      strcat(conf_path, tmp);
     }

     return conf_path;
}

/*
 * prom_get_ip()
 * returns the ip addr of an net card
 */
char * prom_get_ip (struct bootp_packet * packet)
{
     char * conf_path;

     if (!packet)
        return NULL;

     /* 8 chars in yiaddr + \0 */
     conf_path = malloc(9);
     if (!conf_path)
         return NULL;
     sprintf(conf_path, "%08x", packet->yiaddr);

     return conf_path;
}

/* We call this too early to use malloc, 128 cells should be large enough */
#define NR_AVAILABLE 128

void prom_print_available(void)
{
     prom_handle root;
     unsigned int addr_cells, size_cells;
     ihandle mem;
     unsigned int available[NR_AVAILABLE];
     unsigned int len;
     unsigned int *p;

     if (!yaboot_debug)
          return;

     root = prom_finddevice("/");
     if (!root)
          return;

     addr_cells = 2;
     prom_getprop(root, "#address-cells", &addr_cells, sizeof(addr_cells));

     size_cells = 1;
     prom_getprop(root, "#size-cells", &size_cells, sizeof(size_cells));

     mem = prom_finddevice("/memory@0");
     if (mem == PROM_INVALID_HANDLE)
          return;

     int _len = prom_getprop(mem, "available", available, sizeof(available));
     if (_len == -1)
          return;
     len = _len / 4;

     prom_printf("\nAvailable memory ranges:\n");

     p = available;
     while (len > 0) {
          unsigned int addr, size;

          /*
           * Since we are in 32bit mode it should be safe to only print the
           * bottom 32bits of each range.
           */
          p += (addr_cells - 1);
          addr = *p;
          p++;

          p += (size_cells - 1);
          size = *p;
          p++;

          if (size)
               prom_printf("0x%08x-0x%08x (%3d MB)\n", addr, addr + size,
                           size/1024/1024);

          len -= (addr_cells + size_cells);
     }

     prom_printf("\n");
}

/*
 * Local variables:
 * c-file-style: "k&r"
 * c-basic-offset: 5
 * End:
 */
