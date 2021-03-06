/*
 * srtp_priv.h
 *
 * private internal data structures and functions for libSRTP
 *
 * David A. McGrew
 * Cisco Systems, Inc.
 */
/*
 *   
 * Copyright (ca) 2001-2006 Cisco Systems, Inc.
 * 
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 *   Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * 
 *   Redistributions in binary form must reproduce the above
 *   copyright notice, this list of conditions and the following
 *   disclaimer in the documentation and/or other materials provided
 *   with the distribution.
 * 
 *   Neither the name of the Cisco Systems, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef SRTP_PRIV_H
#define SRTP_PRIV_H

//#include "srtp.h"
//
///*
// * an srtp_hdr_t represents the srtp header
// *
// * in this implementation, an srtp_hdr_t is assumed to be 32-bit aligned
// * 
// * (note that this definition follows that of RFC 1889 Appendix A, but
// * is not identical)
// */
// 
//#ifndef WORDS_BIGENDIAN
//
///*
// * srtp_hdr_t represents an RTP or SRTP header.  The bit-fields in
// * this structure should be declared "u32" instead of 
// * "uchar", but doing so causes the MS compiler to not
// * fully pack the bit fields.
// */
//
//typedef struct {
//  uchar cc:4;   /* CSRC count             */
//  uchar x:1;   /* header extension flag  */
//  uchar p:1;   /* padding flag           */
//  uchar version:2; /* protocol version    */
//  uchar pt:7;   /* ::payload type           */
//  uchar m:1;   /* marker bit             */
//  u16 seq;      /* sequence number        */
//  u32 ts;      /* timestamp              */
//  u32 ssrc;   /* synchronization source */
//} srtp_hdr_t;
//
//#else /*  BIG_ENDIAN */
//
//typedef struct {
//  uchar version:2; /* protocol version    */
//  uchar p:1;   /* padding flag           */
//  uchar x:1;   /* header extension flag  */
//  uchar cc:4;   /* CSRC count             */
//  uchar m:1;   /* marker bit             */
//  u32 pt:7;   /* ::payload type           */
//  u16 seq;      /* sequence number        */
//  u32 ts;      /* timestamp              */
//  u32 ssrc;   /* synchronization source */
//} srtp_hdr_t;
//
//#endif
//
//typedef struct {
//  u16 profile_specific;    /* profile-specific info               */
//  u16 length;              /* number of 32-bit words in extension */
//} srtp_hdr_xtnd_t;
//
//
///*
// * srtcp_hdr_t represents a secure rtcp header 
// *
// * in this implementation, an srtcp header is assumed to be 32-bit
// * alinged
// */
//
//#ifndef WORDS_BIGENDIAN
//
//typedef struct {
//  uchar rc:5;      /* reception report count */
//  uchar p:1;      /* padding flag           */
//  uchar version:2;   /* protocol version       */
//  uchar pt:8;      /* ::payload type           */
//  u16 len;         /* length                 */
//  u32 ssrc;             /* synchronization source */
//} srtcp_hdr_t;
//
//typedef struct {
//  u32 index:31;    /* srtcp packet index in network order! */
//  u32 e:1;         /* encrypted? 1=yes */
//  /* optional mikey/etc go here */
//  /* and then the ::payload-length auth tag */
//} srtcp_trailer_t;
//
//
//#else /*  BIG_ENDIAN */
//
//typedef struct {
//  uchar version:2;   /* protocol version       */
//  uchar p:1;      /* padding flag           */
//  uchar rc:5;      /* reception report count */
//  uchar pt:8;      /* ::payload type           */
//  u16 len;         /* length                 */
//  u32 ssrc;             /* synchronization source */
//} srtcp_hdr_t;
//
//typedef struct {
//  u32 version:2;  /* protocol version                     */
//  u32 p:1;        /* padding flag                         */
//  u32 count:5;    /* varies by packet type                */
//  u32 pt:8;       /* ::payload type                         */
//  u16 length;         /* len of ::u32s of packet less header */
//} rtcp_common_t;
//
//typedef struct {
//  u32 e:1;         /* encrypted? 1=yes */
//  u32 index:31;    /* srtcp packet index */
//  /* optional mikey/etc go here */
//  /* and then the ::payload-length auth tag */
//} srtcp_trailer_t;
//
//#endif
//
//
/////*
//// * the following declarations are libSRTP internal functions 
//// */
////
/////*
//// * srtp_get_stream(ssrc) returns a pointer to the stream corresponding
//// * to ssrc, or nullptr if no stream exists for that ssrc
//// */
////
////srtp_stream_t 
////srtp_get_stream(srtp_t srtp, u32 ssrc);
////
////
/////*
//// * srtp_stream_init_keys(s, k) (re)initializes the srtp_stream_t s by
//// * deriving all of the needed keys using the KDF and the key k.
//// */
////
////
////err_status_t
////srtp_stream_init_keys(srtp_stream_t srtp, const void *key);
////
/////*
//// * libsrtp internal datatypes 
//// */
////
////typedef enum direction_t { 
////  dir_unknown       = 0,
////  dir_srtp_sender   = 1, 
////  dir_srtp_receiver = 2
////} direction_t;
////
/////* 
//// * an srtp_stream_t has its own SSRC, encryption key, authentication
//// * key, sequence number, and replay database
//// * 
//// * note that the keys might not actually be unique, in which case the
//// * cipher_t and auth_t pointers will point to the same structures
//// */
////
////typedef struct srtp_stream_ctx_t {
////  u32   ssrc;
////  cipher_t  *rtp_cipher;
////  auth_t    *rtp_auth;
////  rdbx_t     rtp_rdbx;
////  sec_serv_t rtp_services;
////  cipher_t  *rtcp_cipher;
////  auth_t    *rtcp_auth;
////  rdb_t      rtcp_rdb;
////  sec_serv_t rtcp_services;
////  key_limit_ctx_t *limit;
////  direction_t direction;
////  struct srtp_stream_ctx_t *next;   /* linked list of streams */
////} srtp_stream_ctx_t;
////
////
/////*
//// * an srtp_ctx_t holds a stream list and a service description
//// */
////
////typedef struct srtp_ctx_t {
////  srtp_stream_ctx_t *stream_list;     /* linked list of streams            */
////  srtp_stream_ctx_t *stream_template; /* act as template for other streams */
////} srtp_ctx_t;
////
////
////
/////*
//// * srtp_handle_event(srtp, srtm, evnt) calls the event handling
//// * function, if there is one.
//// *
//// * This macro is not included in the documentation as it is 
//// * an internal-only function.
//// */
////
////#define srtp_handle_event(srtp, strm, evnt)         \
////   if(srtp_event_handler) {                         \
////      srtp_event_data_t data;                       \
////      data.session = srtp;                          \
////      data.stream  = strm;                          \
////      data.event   = evnt;                          \
////      srtp_event_handler(&data);                    \
////}   
////
////
#endif /* SRTP_PRIV_H */
