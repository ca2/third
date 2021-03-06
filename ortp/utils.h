/***************************************************************************
 *            utils.h
 *
 *  Wed Feb 23 14:15:36 2005
 *  Copyright  2005  Simon Morlat
 *  Email simon.morlat@linphone.org
 ****************************************************************************/
/*
  The oRTP library is an RTP (Realtime Transport Protocol - rfc3550) stack.
  Copyright (C) 2001  Simon MORLAT simon.morlat@linphone.org

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef UTILS_H
#define UTILS_H

#include "event.h"

struct _OList {
   struct _OList *next;
   struct _OList *prev;
   void *data;
};

typedef struct _OList OList;


#define o_list_next(elem) ((elem)->next)

OList * o_list_append(OList *elem, void * data);
OList * o_list_remove(OList *list, void *data);
OList * o_list_free(OList *elem);

#ifndef min
#define min(a,b) (((a)>(b)) ? (b) : (a))
#endif
#ifndef max
#define max(a,b) (((a)>(b)) ? (a) : (b))
#endif

#define return_if_fail(expr) if (!(expr)) {debug_print("%s:%i- assertion"#expr "failed\n",__FILE__,__LINE__); return;}
#define return_val_if_fail(expr,ret) if (!(expr)) {debug_print("%s:%i- assertion" #expr "failed\n",__FILE__,__LINE__); return (ret);}


#define INT_TO_POINTER(truc)   ((long)(long)(truc))
#define POINTER_TO_INT(truc)   ((i32)(long)(truc))

typedef struct _dwsplit_t{
#ifdef ORTP_BIGENDIAN
   u16 hi;
   u16 lo;
#else
   u16 lo;
   u16 hi;
#endif
} dwsplit_t;

typedef union{
   dwsplit_t split;
   u32 one;
} poly32_t;

#ifdef ORTP_BIGENDIAN
#define hton24(x) (x)
#else
#define hton24(x) ((( (x) & 0x00ff0000) >>16) | (( (x) & 0x000000ff) <<16) | ( (x) & 0x0000ff00) )
#endif
#define ntoh24(x) hton24(x)

#if defined(WIN32) || defined(_WIN32_WCE)
#define is_would_block_error(errnum)   (errnum==WSAEWOULDBLOCK)
#else
#define is_would_block_error(errnum)   (errnum==EWOULDBLOCK || errnum==EAGAIN)
#endif

void ortp_ev_queue_put(OrtpEvQueue *q, OrtpEvent *ev);

#endif
