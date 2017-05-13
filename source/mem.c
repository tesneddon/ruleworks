#define QUICK_ALLOC 1
/* #define CHECK_ALLOC 1 */
/* #define SAVE_ALLOCS 1 */
/* #define LOG_ALLOCS  1 */
/****************************************************************************
**                                                                         **
**                         O P S _ M E M . C                               **
**                    
RuleWorks - Rules based application development tool.

Copyright (C) 1999  Compaq Computer Corporation

This program is free software; you can redistribute it and/or modify it 
under the terms of the GNU General Public License as published by the 
Free Software Foundation; either version 2 of the License, or any later 
version. 

This program is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranty of 
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General 
Public License for more details. 

You should have received a copy of the GNU General Public License along 
with this program; if not, write to the Free Software Foundation, 
Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Email: info@ruleworks.co.uk
****************************************************************************/


/*
 * FACILITY:
 *	RULEWORKS run time system
 *
 * ABSTRACT:
 *	This module provides run time functions for allocating and tracking
 *	dynamic memory.  If the CHECK_ALLOC macro is defined, it does
 *	additional boundary checking to find array overflow bugs.
 *
 * MODIFIED BY:
 *	DEC	Digital Equipment Corporation
**	CPQ	Compaq Computer Corporation
 *
 * MODIFICATION HISTORY:
 *
 *	21-Apr-1993	DEC	Initial version
 *	25-Jul-1994	DEC	Don't fail for any request > 262128 bytes
 *	01-Dec-1999	CPQ	Release with GPL
 *
 *
 *	R O U T I N E S :
 *
 *		rul__mem_malloc (size)
 *		rul__mem_calloc (num_objs, size)
 *		rul__mem_realloc (ptr, new_size)
 *		rul__mem_free (ptr)
 *
 */


#include <common.h>
#include <msg.h>
#include <msg_mess.h>

#ifdef __VMS
#include <ssdef>
#define		MEM_ERROR	SS$_INSFMEM
#else
#define		MEM_ERROR	EXIT_FAILURE
#endif

#ifdef CHECK_ALLOC
#include <ios.h>
#define 	MEM_EXTRA	(2 * sizeof (long *))
#else
#define 	MEM_EXTRA	0
#endif

#ifdef QUICK_ALLOC
#define		QCK_EXTRA	(sizeof (long *))
#ifdef CHECK_ALLOC
#define         QCK_DELTA       (2 * sizeof (long *))
#else
#define         QCK_DELTA       (sizeof (long *))
#endif
#if defined(__VMS) || defined(__UNIX)
#define QMC1 1000
#define QMC2  200
#define QMC3   50
#define QMK1 5000
#define QMK2 2000
#define QMK3  500

#else
#ifdef __WATCOMC__
#define QMC1  500
#define QMC2  100
#define QMC3   20
#define QMK1 5000
#define QMK2 5000
#define QMK3 2000
#else
	/*  PC's other than WATCOM  */
#define QMC1  500
#define QMC2  100
#define QMC3   20
#define QMK1 2500
#define QMK2 1000
#define QMK3  100
#endif
#endif
#define QCK_DIFF 16
static void *qck_mem[16];
static long  qck_mem_cnt[16];
static long  qck_mem_chunk[16] = {0,1,QMC1,QMC2,QMC3,1,1,
				    1,1,1,1,1,1,1,1,1};
static long  qck_mem_keep[16] =  {0,2,QMK1,QMK2,QMK3,1,1,
				    0,0,0,0,0,0,0,0,0};
#define WHICH_IDX        if (size <= 64-QCK_DIFF)     { idx = 2;  } \
		    else if (size <= 128-QCK_DIFF)    { idx = 3;  } \
		    else if (size <= 256-QCK_DIFF)    { idx = 4;  } \
		    else if (size <= 512-QCK_DIFF)    { idx = 5;  } \
		    else if (size <= 1024-QCK_DIFF)   { idx = 6;  } \
		    else if (size <= 2048-QCK_DIFF)   { idx = 7;  } \
		    else if (size <= 4096-QCK_DIFF)   { idx = 8;  } \
		    else if (size <= 8192-QCK_DIFF)   { idx = 9;  } \
		    else if (size <= 16384-QCK_DIFF)  { idx = 10; } \
		    else if (size <= 32768-QCK_DIFF)  { idx = 11; } \
		    else if (size <= 65536-QCK_DIFF)  { idx = 12; } \
		    else if (size <= 131072-QCK_DIFF) { idx = 13; } \
		    else if (size <= 262144-QCK_DIFF) { idx = 14; }
#else
#define 	QCK_EXTRA	0
#endif



#if (defined(LOG_ALLOCS) || defined(SAVE_ALLOCS) || defined(CHECK_ALLOC))
#define MEM_CHECKS_ON
#endif

#ifdef MEM_CHECKS_ON

unsigned long get_pc (void);
unsigned long get_pc1 (void);
unsigned long get_depth (void);
static void close_mem_log (void);
static FILE *mem_log_file (void);
static void rul__mem_log (char *str);
static void *mem_hash_get_entry (void *key);
static void *mem_hash_add_entry (void *key, void *object);
static void mem_hash_for_each_entry (void (*routine)(void *));
static void save_mem_add (void *addr, size_t size,
			  unsigned long pc, unsigned long depth);
static void save_mem_rem (void *addr);
static void save_mem_report (void *data);
static void save_mem_dump (void);
static void mem_set_checksum (void *address, size_t size);

static FILE *SA_mem_log_file_ptr = NULL;

/********************
**                 **
**  CLOSE_MEM_LOG  **
**                 **
********************/

static void close_mem_log (void)
{
  if (SA_mem_log_file_ptr) {
#ifdef SAVE_ALLOCS
    save_mem_dump();
#endif
    fflush (SA_mem_log_file_ptr);
    fclose (SA_mem_log_file_ptr);
  }
}

/*******************
**                **
**  MEM_LOG_FILE  **
**                **
*******************/

static FILE *mem_log_file (void)
{
  if (SA_mem_log_file_ptr == NULL) {
    SA_mem_log_file_ptr = fopen ("rulmem.log", "w");
    atexit (close_mem_log);
  }
  return (SA_mem_log_file_ptr);
}

/* logging routine... */

static void rul__mem_log (char *str)
{
  fprintf (mem_log_file(), " Pc: %12d Depth: %3d   Rtn: %s\n",
	   get_pc1(), get_depth()-1, str);
}

#endif

#ifdef SAVE_ALLOCS

unsigned long get_pc (void);
unsigned long get_pc1 (void);
unsigned long get_depth (void);
static void save_mem_realloc_end (void);

#define SAVE_HT_SIZE    1001

struct mem_hash_chain {
  void                  *symbol;/* The key being hashed */
  void	   	        *object;/* The structure being stored */
  struct mem_hash_chain *next;	/* Next entry in this bucket */
};

typedef struct mem_hash_chain *Mem_Bucket;
typedef Mem_Bucket            *Mem_Hash_Table;

typedef struct memdata { 
  void            *addr;
  size_t           size;
  unsigned long    pc;
  unsigned long    depth;
} *Mem ;


static  Mem_Hash_Table         mem_ht;
static  FILE                  *mem_dump_file;

#define mem_hash(key) ((long) key % SAVE_HT_SIZE)

static void *mem_hash_get_entry (void *key)
{
  Mem_Bucket hc;

  for (hc = mem_ht[mem_hash (key)]; hc != NULL; hc = hc->next) {
    if (hc->symbol == key)
      return hc->object;
  }
  return NULL;
}

static void * mem_hash_add_entry (void *key, void *object)
{
  Mem_Bucket hc, new_chain_link;
  unsigned long hash_obj_id;
  
  hash_obj_id = mem_hash (key);
  for (hc = mem_ht[hash_obj_id]; hc != NULL; hc = hc->next) {
    if (hc->symbol == key)
      return NULL;	/* Oops, it's already there; return NULL */
  }
  new_chain_link = malloc (sizeof (struct mem_hash_chain));
  new_chain_link->symbol = key;
  new_chain_link->object = object;
  new_chain_link->next = mem_ht[hash_obj_id];
  mem_ht[hash_obj_id] = new_chain_link;
  return object;
}

static void mem_hash_for_each_entry (void (*routine)(void *))
{
  int i;
  Mem_Bucket hc, nhc;

  for (i = 0; i < SAVE_HT_SIZE; i++) {
    hc = mem_ht[i]; 
    while (hc != NULL) {
      nhc = hc->next;
      (*routine)(hc->object);
      hc = nhc;
    }
  }
}

static void save_mem_add (void *addr, size_t size,
			  unsigned long pc, unsigned long depth)
{
  Mem m;

#ifndef LOG_ALLOCS
  if (SA_mem_log_file_ptr == NULL)
    mem_dump_file = mem_log_file ();
#endif

  if (! mem_ht)
    mem_ht = calloc (1, (SAVE_HT_SIZE * sizeof (long *)));

  m = mem_hash_get_entry (addr);
  if (m == NULL) {
    m = malloc (sizeof (struct memdata));
  }
  else {
    if (m->pc != 0) {
#ifdef LOG_ALLOCS
      fprintf (mem_log_file(),
	       " Unknown free for pc: %12d, for address: malloc %u (%u)\n",
	       m->pc, m->addr, m->size);
#endif
      fprintf (stderr,
	       " Unknown free for pc: %12d, for address: malloc %u (%u)\n",
	       m->pc, m->addr, m->size);
    }
  }

  m->addr  = addr;
  m->size  = size;
  m->pc    = pc;
  m->depth = depth;
  mem_hash_add_entry (addr, m);
}

static void save_mem_rem (void *addr)
{
  Mem m;

  if (! mem_ht)
    mem_ht = calloc (1, (SAVE_HT_SIZE * sizeof (long *)));

  m = mem_hash_get_entry (addr);
  if (m == NULL) {
#ifdef LOG_ALLOCS
    fprintf (mem_log_file(), " Invalid free of address: %u\n", addr);
#endif
    fprintf (stderr, " Invalid free of address: %u\n", addr);
  }
  else
    m->pc = 0;
}

static void save_mem_report (void *data)
{
  Mem m = data;

  if (m->pc)
    fprintf (mem_dump_file,
	     "Rtn: %12d, Addr: %12u, size: %6u, depth: %3u\n",
	     m->pc, m->addr, m->size, m->depth);
}


static void save_mem_dump (void)
{

  if (mem_dump_file == NULL)
    mem_dump_file = stderr;

  fprintf (mem_dump_file, "\n\n Unfreed memory...\n");
  mem_hash_for_each_entry (save_mem_report);
}

#endif



#ifdef CHECK_ALLOC

static void mem_set_checksum (void *address, size_t size)
{
  long 	checksum;

  /* put our guard bits on the ends if we're in check mode.
     The first check word contains the inverse of the size, which
     we later recover and use to verify the tail gaurd bits */
  
  *(long *)address = size;
  checksum = -1 * size;
  *((long *)(((char *)address) + size + sizeof(long))) = checksum;
}

#endif



/**********************
**                   **
**  RUL__MEM_MALLOC  **
**                   **
**********************/

void *rul__mem_malloc (size_t size)
{
  void	*address = NULL;
#ifdef QUICK_ALLOC
  long   idx = 0, i, c;
  void  *tmpadr;
#endif

#ifdef QUICK_ALLOC

  WHICH_IDX;
  c = qck_mem_chunk[idx];

  if (idx) {
    if (qck_mem[idx] == NULL) {
      size = (16 << idx) - QCK_DIFF;
      qck_mem_cnt[idx] += c;
      qck_mem_cnt[0] += c;
      qck_mem[idx] = malloc (size + MEM_EXTRA + QCK_EXTRA);
      address = qck_mem[idx];
      for (i = 0; i < (c - 1); i++) {
	tmpadr = malloc (size + MEM_EXTRA + QCK_EXTRA);
	*(void **)address = tmpadr;
	address = tmpadr;
      }
      *(void **)address = NULL;
    }
    qck_mem_cnt[idx] -= 1;
    address = qck_mem[idx];
    qck_mem[idx] = *(void **)address;
  }
  else				/* idx == 0, too big for lookaside list */
    address = malloc (size + MEM_EXTRA + QCK_EXTRA);
#endif

  if (address == NULL) {		/* complain if none is available  */
    assert (address != NULL);
    exit (MEM_ERROR);
  }
  
#ifdef QUICK_ALLOC
  *(long *)address = idx;
  address = ((void *)((char *)address + sizeof (long *)));
#endif
  
#ifdef LOG_ALLOCS
  fprintf (mem_log_file(), " Pc: %12d Depth: %3d  malloc %12u size: %6u\n",
	   get_pc1(), get_depth(), address, size);
#endif
#ifdef SAVE_ALLOCS
  if (get_pc1() > (unsigned long)rul__mem_realloc &&
      get_pc1() < (unsigned long)save_mem_realloc_end)
    save_mem_add (address, size, get_pc(), get_depth());
  else
    save_mem_add (address, size, get_pc1(), get_depth());
#endif

#ifdef CHECK_ALLOC
  mem_set_checksum (address, size);
  return ((void *)((char *)address + sizeof (long)));
#else
  return (address);
#endif
}




/********************
**                 **
**  RUL__MEM_FREE  **
**                 **
********************/

void rul__mem_free (void *address)
{
#ifdef QUICK_ALLOC
  void *oldmem;
  long idx, c;
#endif

  if (address == NULL)
    return;

#ifdef QUICK_ALLOC
  oldmem = (void *)((long) address - QCK_DELTA);
  idx = *(long *)oldmem;

  if (idx) {
    if (qck_mem_cnt[idx] < qck_mem_keep[idx]) {
      *(void **)oldmem = qck_mem[idx];
      qck_mem[idx] = oldmem;
      qck_mem_cnt[idx] += 1;
    }
    else {
      idx = 0;
      qck_mem_cnt[0] -= 1;
    }
  }
#endif

#ifdef LOG_ALLOCS
#ifndef CHECK_ALLOC  
  fprintf (mem_log_file(), " Pc: %12d Depth: %3d Address: %12u\n",
	   get_pc1(), get_depth(), address);
#ifdef SAVE_ALLOCS
  save_mem_rem (address);
#endif
#endif
#else
#ifndef CHECK_ALLOC  
#ifdef SAVE_ALLOCS
  save_mem_rem (address);
#endif
#endif
#endif
  
#ifdef CHECK_ALLOC
  {
    char *sa; 
    long size;
    long checksum;
    
    sa = (char *)((long) address - sizeof (long));
    size = * (long *)sa;		/* this is the size. */
    checksum = *((long *)(((char *)address) + size));
    
#ifdef LOG_ALLOCS
  fprintf (mem_log_file(), " Pc: %12d Depth: %3d Address: %12u size: %6u\n",
	   get_pc1(), get_depth(), sa, size);
#endif
#ifdef SAVE_ALLOCS
  save_mem_rem (sa);
#endif

    /* test for overflowing size*/
    if (checksum != -1 * size) {
      fprintf (stderr, "\nMemory free error --  ");
      fprintf (stderr, "Address: %16d  Size: %6d  Check: %10d\n",
	       sa, size, checksum);
#ifdef LOG_ALLOCS
      fprintf (mem_log_file(), "\n -- Memory free error -- \n");
      fprintf (mem_log_file(), " Pc: %12d Depth: %3d  Address: %12u Size: %6u Check: %u\n",
	       get_pc1(), get_depth(), sa, size, checksum);
#endif
    }
    
    /* clobber gaurd bits so they can't be used again.  */
    *(long *)sa = 0;			
    *(long *)((long) address + size) = 0;
    
    /* fix up address so that C's free can deal with it... */
    address = sa;
  }
#endif

  assert ((unsigned long) address > 10000);

#ifdef QUICK_ALLOC
  if (!idx)
    free (oldmem);
#else
  free (address);
#endif
}




/**********************
**                   **
**  RUL__MEM_CALLOC  **
**                   **
**********************/

void *rul__mem_calloc (size_t num_objs, size_t osize)
{
  void   *address = NULL;
  size_t  size = (num_objs * osize);
#ifdef QUICK_ALLOC
  long   idx = 0, c;
#endif

#ifdef QUICK_ALLOC

  WHICH_IDX;

  if (idx) {
    qck_mem_cnt[0] += 1;
    size = (16 << idx) - QCK_DIFF;
    if (qck_mem[idx]) {
      address = qck_mem[idx];
      qck_mem_cnt[idx] -= 1;
      qck_mem[idx] = *(void **)address;
      memset (address, 0, ((osize * num_objs) + MEM_EXTRA + QCK_EXTRA));
    }
    else {
      address = (void *) calloc (1, (size + MEM_EXTRA + QCK_EXTRA));
    }
  }
  else				/* idx == 0, too big for lookaside list */
    address = calloc (1, size + MEM_EXTRA + QCK_EXTRA);
#endif

  if (address == NULL) {		/* complain if none is available  */
    assert (address != NULL);
    exit (MEM_ERROR);
  }
  
#ifdef QUICK_ALLOC
  *(long *)address = idx;
  address = ((void *)((char *)address + sizeof (long *)));
#endif
  
#ifdef LOG_ALLOCS
  fprintf (mem_log_file(), " Pc: %12d Depth: %3d  calloc Address: %12u size: %6u\n",
	   get_pc1(), get_depth(), address, size);
#endif
#ifdef SAVE_ALLOCS
  save_mem_add (address, size, get_pc1(), get_depth());
#endif
  
#ifdef CHECK_ALLOC
  mem_set_checksum (address, size);
  return ((void *)((char *)address + sizeof (long)));
#else
  return (address);
#endif
}




/***********************
**                    **
**  RUL__MEM_REALLOC  **
**                    **
***********************/

void *rul__mem_realloc (void *address, size_t size)
{
  void   *new_address = NULL;
#ifdef QUICK_ALLOC
  void   *oldmem;
  void   *newmem;
  long    idx = 0, c, i;
#endif

#ifdef QUICK_ALLOC
  if (address != NULL) {
    
    WHICH_IDX;

    oldmem = (void *)((long) address - QCK_DELTA);

    if (*(long *)oldmem) {
      if (*(long *)oldmem == idx)
	new_address = address;
      else {
	new_address = rul__mem_malloc (size);
	memcpy (new_address, address, (16 << (*(long *)oldmem)) - QCK_DIFF);
	rul__mem_free (address);
      }
    }
  }
#endif

#ifdef CHECK_ALLOC
  {
    char *sa; 
    long size;
    long checksum;
    
    if (address != NULL) {
      sa = (char *)((long)address - sizeof(long));
      size = *(long *)sa;		/* this is the size. */
      checksum = *((long *)(((char *)address) + size));
    
      /* test for overflowing size*/
      if (checksum != -1 * size) {
	fprintf (stderr, "\nMemory realloc error --  ");
	fprintf (stderr, "Address: %12d  Size: %6d  Check: %10d\n",
		 sa, size, checksum);
#ifdef LOG_ALLOCS
	fprintf (mem_log_file(), "\n -- Memory realloc error -- \n");
	fprintf (mem_log_file(), " Pc: %12d Depth: %3d Address: %12u Size: %6u Check: %u\n",
		 get_pc1(), get_depth(), sa, size, checksum);
#endif
      }
      /* clobber gaurd bits so they can't be used again.  */
      *(long *)sa = 0;			
      *(long *)((long) address + size) = 0;
      
      /* fix up address so that C's realloc
	 can deal with it... */
      address = sa;
    }
  }
#endif

  if (new_address == NULL) {
    if (address == NULL) {
      new_address = rul__mem_malloc (size);
    }
    else {
#ifdef QUICK_ALLOC
      oldmem = (void *)((long) address - QCK_DELTA);
      new_address = realloc (oldmem, size + MEM_EXTRA + QCK_EXTRA);
      *(long *)new_address = 0;
      new_address = ((void *)((char *)new_address + sizeof (long *)));
#else
      new_address = realloc (address, size + MEM_EXTRA);
#endif
    }
#ifdef SAVE_ALLOCS
      save_mem_rem (address);
      save_mem_add (new_address, size, get_pc1(), get_depth());
#endif
  }
  
#ifdef LOG_ALLOCS
  fprintf (mem_log_file(), " Pc: %12d Depth: %3d realloc Address: %12u -> %12u size: %6u\n",
	   get_pc1(), get_depth(), address, new_address, size);
#endif

  if (new_address == NULL) {	/* complain if none is available  */
    assert (new_address != NULL);
    exit (MEM_ERROR);
  }
  
#ifdef CHECK_ALLOC
  mem_set_checksum (new_address, size);
  return ((void *)((char *)new_address + sizeof(long)));
#else
  return (new_address);
#endif
}

#ifdef SAVE_ALLOCS
static void save_mem_realloc_end (void) {}
#endif

