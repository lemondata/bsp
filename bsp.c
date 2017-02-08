/* bsp.c -- compress files to the bsp format
   Copyright (C) 2017, Lemon-Data.net Inc.
   Copyright (C) 2017, Rely Blanc for algorithm claim * 
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.
   */
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <malloc.h>
#include <memory.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dir.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>

static char  *license_msg[] = {
"Copyright (c) 2017, Lemon-Data.net Inc.",
"Copyright (C) 2017, Remi BLANC for algorithm claim * & written by Philippe ETTER",
"This program comes with ABSOLUTELY NO WARRANTY.",
"You may redistribute copies of this program",
"under the terms of the GNU General Public License.",
"For more information about these matters, see the file named COPYING.",
0};

/* Return codes from bsp */
#define OK      0
#define ERROR   1
#define WARNING 2

/* Define as the return type of signal handlers (`int' or `void'). */
#define RETSIGTYPE void

#ifdef __STDC__
   typedef void *voidp;
#else
   typedef char *voidp;
#endif

/* Version number of package */
#define VERSION "0.0.1"
#define REVDATE "2017-02-07"

/* Common defaults */
#ifndef OS_CODE
#  define OS_CODE  0x03  /* assume Unix */
#endif

#ifndef PATH_SEP
#  define PATH_SEP '/'
#endif

#ifdef DYN_ALLOC
#  define EXTERN(type, array)  extern type * near array
#  define ALLOC(type, array, size)
#  define FREE(array)
#endif

#ifndef	O_BINARY
#  define  O_BINARY  0  /* creation mode for open() */
#endif
#ifndef	RW_USER
#  define RW_USER (S_IRUSR | S_IWUSR)  /* creation mode for open() */
#endif

#define tolow(c)  (isupper (c) ? tolower (c) : (c))  /* force to lower case */
#ifndef MACROS
#  define PATH_SEP '/'
#  define MAX_PATH_LEN  128
#  define chmod(file, mode) (0)
#  define OPEN(name, flags, mode) open(name, flags)
#  define casemap(c) tolow(c) /* Force file names to lower case */
#endif

#ifndef OPEN
#  define OPEN(name, flags, mode) open(name, flags, mode)
#endif

#ifndef	OUTBUFSIZ
#  define OUTBUFSIZ   8192  /* output buffer size */
#endif

extern unsigned insize; /* valid bytes in inbuf */
extern unsigned inptr;  /* index of next byte to be processed in inbuf */
extern unsigned outcnt; /* bytes in output buffer */
extern int rsync;  /* deflate into rsyncable chunks */

extern off_t bytes_in;   /* number of input bytes */
extern off_t bytes_out;  /* number of output bytes */
extern off_t header_bytes;/* number of bytes in bsp header */

extern int  ifd;        /* input file descriptor */
extern int  ofd;        /* output file descriptor */
extern char ifname[];   /* input file name or "stdin" */
extern char ofname[];   /* output file name or "stdout" */
extern char *progname;  /* program name */

extern int decrypt;        /* flag to turn on decryption */
extern int exit_code;      /* program exit code */
extern int verbose;        /* be verbose (-v) */
extern int quiet;          /* be quiet (-q) */

RETSIGTYPE abort_bsp(void);
extern char *strlwr(char *s);
char * base_name(char *);
void flush_outbuf(void);
void progerror();
void read_error();
void write_error();

/* put_byte is used for the compressed output, put_ubyte for the
 * uncompressed output. However unlzw() uses window for its
 * suffix table instead of its output buffer, so it does not use put_ubyte
 * (to be cleaned up).
 */
#define put_byte(c) {outbuf[outcnt++]=(unsigned char)(c); if (outcnt==OUTBUFSIZ)\
   flush_outbuf();}
   
   
  /* global buffers */ 
  
// EXTERN(unsigned char, inbuf);          /* input buffer */
// EXTERN(unsigned char, outbuf);         /* output buffer */
unsigned char *inbuf;
unsigned char *outbuf;

	/* local variables */
  
int exit_code = OK;   /* program exit code */
unsigned outcnt;           /* bytes in output buffer */
int ascii = 0;        /* convert end-of-lines to local OS conventions */
int extract = 0;   /* decompress (-x) */
char *progname;       /* program name */

off_t bytes_in;             /* number of input bytes */
off_t bytes_out;            /* number of output bytes */
int  ifd;                  /* input file descriptor */
int  ofd;                  /* output file descriptor */
char ifname[MAX_PATH_LEN]; /* input file name */
char ofname[MAX_PATH_LEN]; /* output file name */
unsigned insize;           /* valid bytes in inbuf */
unsigned inptr;            /* index of next byte to be processed in inbuf */
unsigned outcnt;           /* bytes in output buffer */
off_t header_bytes;   /* number of bytes in gzip header */

// #include bsp.h

int bsp(int in, int out)  /* input and output file descriptors */
{
  ifd = in;
  ofd = out;
  outcnt = 0;

  // copy - paste file,
  char *p = base_name(ifname);
  do {
  
  } while (*p++);
  header_bytes = (off_t)outcnt;
  
  flush_outbuf();
  return OK;
}

  
int bsp_compress_blk(unsigned char *bits, int base)
{
}


/* ========================================================================
 * Put string s in lower case, return s.
 */
char *strlwr(s)
    char *s;
{
    char *t;
    for (t = s; *t; t++) 
      *t = tolow ((unsigned char) *t);
    return s;

}

void progerror (char *string)
{
    int e = errno;
    fprintf(stderr, "%s: ", progname);
    errno = e;
    perror(string);
    exit_code = ERROR;
}

void read_error()
{
    int e = errno;
    fprintf(stderr, "\n%s: ", progname);
    if (e != 0) {
	errno = e;
	perror(ifname);
    } else {
	fprintf(stderr, "%s: unexpected end of file\n", ifname);
    }
    abort_bsp();
}

void write_error()
{
    int e = errno;
    fprintf(stderr, "\n%s: ", progname);
    errno = e;
    perror(ofname);
    abort_bsp();
}

/* ========================================================================
 * Error handler.
 */
RETSIGTYPE abort_bsp()
{
	// do_remove();
	_exit(ERROR);
}

/* ========================================================================
 * Return the base name of a file (remove any directory prefix and
 * any version suffix). For systems with file names that are not
 * case sensitive, force the base name to lower case.
 */
char *base_name(fname)
    char *fname;
{
    char *p;

    if ((p = strrchr(fname, PATH_SEP))  != NULL) fname = p+1;
#ifdef PATH_SEP2
    if ((p = strrchr(fname, PATH_SEP2)) != NULL) fname = p+1;
#endif
#ifdef PATH_SEP3
    if ((p = strrchr(fname, PATH_SEP3)) != NULL) fname = p+1;
#endif
#ifdef SUFFIX_SEP
    if ((p = strrchr(fname, SUFFIX_SEP)) != NULL) *p = '\0';
#endif
    if (casemap('A') == 'a') strlwr(fname);
    return fname;
}


/* ===========================================================================
 * Clear input and output buffers
 */
void clear_bufs()
{
    outcnt = 0;
    insize = inptr = 0;
    bytes_in = bytes_out = 0L;
}

/* ===========================================================================
 * Does the same as write(), but also handles partial pipe writes and checks
 * for error return.
 */
void write_buf(int fd, voidp buf, unsigned cnt)
{
    unsigned  n;

    while ((n = write(fd, buf, cnt)) != cnt) {
	    if (n == (unsigned)(-1)) {
	    write_error();
	    }
	  cnt -= n;
	  buf = (voidp)((char*)buf+n);
    }
}

/* ===========================================================================
 * Write the output buffer outbuf[0..outcnt-1] and update bytes_out.
 * (used for the compressed data only)
 */
void flush_outbuf()
{
    if (outcnt == 0) return;

    write_buf(ofd, (char *)outbuf, outcnt);
    bytes_out += (off_t)outcnt;
    outcnt = 0;
}


/* ========================================================================
* Compress or extract the given file
*/
void treat_file(char *iname)
{
    /* Open the input file and determine compression method. The mode
     * parameter is ignored but required by some systems (VMS) and forbidden
     * on other systems (MacOS).
     */
     ifd = OPEN(ifname, ascii && !extract ? O_RDONLY : O_RDONLY | O_BINARY, RW_USER);
     if (ifd == -1) {
	      progerror(ifname);
	      return;
    }
}

/* ===========================================================================
 * Read a new buffer from the current input file, perform end-of-line
 * translation, and update the crc and input file size.
 * IN assertion: size >= 2 (for end-of-line translation)
 */
int file_read(char *buf, unsigned size)
{
  unsigned len;
  // Assert(insize == 0, "inbuf not empty");

  len = read(ifd, buf, size);
  if (len == 0) return (int)len;
  if (len == (unsigned)-1) {
     read_error();
     return EOF;
  }
  
  // crc = updcrc((uunsigned char*)buf, len);
  bytes_in += (off_t)len;
  return (int)len;
}

void license()
{
    char **p = license_msg;

    printf ("%s %s\n(%s)\n", progname, VERSION, REVDATE);
    while (*p) printf ("%s\n", *p++);
}

void version()
{
  license();
  printf("\n");
  printf("Witten by Etter-Philipp. (@lemon-data.net)\n");
}

int main(int argc, char *argv[])
{
  progname = base_name (argv[0]);
}

/* end bsp.c */
