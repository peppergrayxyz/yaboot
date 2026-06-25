/*
 *  prom.h - Routines for talking to the Open Firmware PROM
 *
 *  Copyright (C) 2001 Ethan Benson
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

#ifndef PROM_H
#define PROM_H

#include "types.h"
#include "stdarg.h"

typedef int32_t     prom_cell_t;
typedef void*       prom_addr_t;
typedef prom_cell_t prom_size_t;
typedef prom_cell_t prom_handle;
typedef prom_handle ihandle;
typedef prom_handle phandle;

#define PROM_INVALID_HANDLE	  ((prom_handle)-1UL)
#define PROM_INVALID_ADDR       ((prom_addr_t) PROM_INVALID_HANDLE)
#define BOOTDEVSZ               (2048) /* iscsi args can be in excess of 1040 bytes */
#define TOK_ISCSI               "iscsi"
#define TOK_IPV6                "ipv6"
#define PROM_CLAIM_MAX_ADDR     ((prom_addr_t) 0x10000000)
#define BOOTLASTSZ              1024
#define FW_NBR_REBOOTSZ         4

struct prom_args;
typedef int (*prom_entry)(struct prom_args *);

extern void prom_init (prom_entry pe);

extern prom_entry prom;

/* I/O */

extern ihandle prom_stdin;
extern ihandle prom_stdout;

ihandle prom_open (const char *spec);
ssize_t prom_read (ihandle file, void *buf, off_t len);
ssize_t prom_write (ihandle file, void *buf, off_t len);
off_t prom_lseek (ihandle file, off_t pos);
off_t prom_getfilesize(ihandle file);

void prom_close (ihandle file);
int32_t prom_getblksize (ihandle file);
int32_t prom_loadmethod (ihandle device, prom_addr_t addr);

#define K_UP    0x141
#define K_DOWN  0x142
#define K_LEFT  0x144
#define K_RIGHT 0x143

int32_t prom_getchar ();
void prom_putchar (char);
int32_t prom_nbgetchar();

#ifdef __GNUC__
void prom_vprintf (const char *prom_set_color, va_list ap) __attribute__ ((format (printf, 1, 0)));
void prom_vfprintf (ihandle file, const char *fmt, va_list ap)  __attribute__ ((format (printf, 2, 0)));
void prom_fprintf (ihandle dev, const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
void prom_printf (const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void prom_debug (const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
#else
void prom_vprintf (const char *fmt, va_list ap);
void prom_fprintf (ihandle dev, const char *fmt, ...);
void prom_printf (const char *fmt, ...);
void prom_debug (const char *fmt, ...);
#endif

void prom_perror (int error, const char *filename);
void prom_readline (const char *prompt, char *line, uint32_t len);
int32_t prom_set_color(ihandle device, uint32_t color, uint32_t r, uint32_t g, uint32_t b);

/* memory */

prom_addr_t prom_claim_chunk(prom_addr_t virt, prom_size_t size, prom_size_t align);
prom_addr_t prom_claim_chunk_top(prom_size_t size, prom_size_t align);
prom_addr_t prom_claim (prom_addr_t virt, prom_size_t size, prom_size_t align);
void prom_release(prom_addr_t virt, prom_size_t size);
void prom_map (prom_addr_t phys, prom_addr_t virt, prom_size_t size);
void prom_print_available(void);

/* packages and device nodes */

phandle prom_finddevice (const char *name);
phandle prom_findpackage (const char *path);
int32_t prom_getprop (phandle dev, const char *name, void *buf, uint32_t len);
int32_t prom_setprop (phandle dev, const char *name, void *buf, uint32_t len);
int32_t prom_getproplen(phandle, const char *);
int32_t prom_get_devtype (const char *device);

/* misc */

char *prom_getargs ();
void prom_setargs (const char *args);

void prom_exit ();
void prom_abort (const char *fmt, ...);
void prom_sleep (unsigned int seconds);

int32_t prom_interpret (const char *forth);

int32_t prom_get_chosen (const char *name, void *mem, uint32_t len);
int32_t prom_get_options (const char *name, void *mem, uint32_t len);
int32_t prom_set_options (const char *name, void *mem, uint32_t len);

extern int32_t prom_getms(void);
extern void prom_pause(void);

extern prom_cell_t call_prom (const char *service, size_t nargs, size_t nret, ...);
extern prom_cell_t call_prom_return (const char *service, size_t nargs, size_t nret, ...);

/* Netboot stuffs */

/*
 * "bootp-response" is the property name which is specified in
 * the recommended practice doc for obp-tftp. However, pmac
 * provides a "dhcp-response" property and chrp provides a
 * "bootpreply-packet" property.  The latter appears to begin the
 * bootp packet at offset 0x2a in the property for some reason.
 */

struct bootp_property_offset {
     char *name; /* property name */
     int offset; /* offset into property where bootp packet occurs */
};

static const struct bootp_property_offset bootp_response_properties[] = {
     { .name = "bootp-response", .offset = 0 },
     { .name = "dhcp-response", .offset = 0 },
     { .name = "bootpreply-packet", .offset = 0x2a },
};

struct bootp_packet {
     __u8 op, htype, hlen, hops;
     __u32 xid;
     __u16 secs, flags;
     __u32 ciaddr, yiaddr, siaddr, giaddr;
     unsigned char chaddr[16];
     unsigned char sname[64];
     unsigned char file[128];
     unsigned char options[]; /* vendor options */
};

struct bootp_packet * prom_get_netinfo (void);
char * prom_get_mac (struct bootp_packet * packet);
char * prom_get_ip (struct bootp_packet * packet);

#endif
