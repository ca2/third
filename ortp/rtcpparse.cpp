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


#include "framework.h"
#include "ortp_utils.h"
#include <stdio.h>


/*in case of coumpound packet, set read pter of m to the beginning of the next RTCP
packet */
bool_t rtcp_next_packet(mblk_t *m){
   const rtcp_common_header_t *ch=rtcp_get_common_header(m);
   if (ch){
      i32 nextlen=sizeof(rtcp_common_header_t)+
         (rtcp_common_header_get_length(ch)*4);
      if (m->b_rptr+nextlen<m->b_wptr){
         m->b_rptr+=nextlen;
         return TRUE;
      }
   }
   return FALSE;
}

void rtcp_rewind(mblk_t *m){
   m->b_rptr=m->b_datap->db_base;
}

/* get common header; this function will also check the sanity of the packet*/
const rtcp_common_header_t * rtcp_get_common_header(const mblk_t *m){
   i32 size=msgdsize(m);
   rtcp_common_header_t *ch;
   if (m->b_cont!=nullptr){
      ortp_fatal("RTCP parser does not work on fragmented mblk_t. Use msgpullup() before to re-assemble the packet.");
      return nullptr;
   }
   if (size<sizeof(rtcp_common_header_t)){
      ortp_warning("Bad RTCP packet, too i16.");
      return nullptr;
   }
   ch=(rtcp_common_header_t*)m->b_rptr;
   return ch;
}

bool_t rtcp_is_SR(const mblk_t *m){
   const rtcp_common_header_t *ch=rtcp_get_common_header(m);
   if (ch!=nullptr && rtcp_common_header_get_packet_type(ch)==RTCP_SR){
      if (msgdsize(m)<sizeof(rtcp_sr_t)){
         ortp_warning("Too i16 RTCP SR packet.");
         return FALSE;
      }
      return TRUE;
   }
   return FALSE;
}

/*Sender Report accessors */
u32 rtcp_SR_get_ssrc(const mblk_t *m){
   rtcp_sr_t *sr=(rtcp_sr_t*)m->b_rptr;
   return ntohl(sr->ssrc);
}

const sender_info_t * rtcp_SR_get_sender_info(const mblk_t *m){
   rtcp_sr_t *sr=(rtcp_sr_t*)m->b_rptr;
   return &sr->si;
}

const report_block_t * rtcp_SR_get_report_block(const mblk_t *m, i32 idx){
   rtcp_sr_t *sr=(rtcp_sr_t*)m->b_rptr;
   report_block_t *rb=&sr->rb[idx];
   i32 size=sizeof(rtcp_common_header_t)+(4*rtcp_common_header_get_length(&sr->ch));
   if ( ( (u8*)rb)+sizeof(report_block_t) <= m->b_rptr + size ) {
      return rb;
   }else{
      if (idx<rtcp_common_header_get_rc(&sr->ch)){
         ortp_warning("RTCP packet should include a report_block_t at pos %i but has no space for it.",idx);
      }
   }
   return nullptr;
}

/*Receiver report accessors*/
bool_t rtcp_is_RR(const mblk_t *m){
   const rtcp_common_header_t *ch=rtcp_get_common_header(m);
   if (ch!=nullptr && rtcp_common_header_get_packet_type(ch)==RTCP_RR){
      if (msgdsize(m)<sizeof(rtcp_rr_t)){
         ortp_warning("Too i16 RTCP RR packet.");
         return FALSE;
      }
      return TRUE;
   }
   return FALSE;
}

u32 rtcp_RR_get_ssrc(const mblk_t *m){
   rtcp_rr_t *rr=(rtcp_rr_t*)m->b_rptr;
   return ntohl(rr->ssrc);
}

const report_block_t * rtcp_RR_get_report_block(const mblk_t *m,i32 idx){
   rtcp_rr_t *rr=(rtcp_rr_t*)m->b_rptr;
   report_block_t *rb=&rr->rb[idx];
   i32 size=sizeof(rtcp_common_header_t)+(4*rtcp_common_header_get_length(&rr->ch));
   if ( ( (u8*)rb)+sizeof(report_block_t) <= (m->b_rptr + size ) ){
      return rb;
   }else{
      if (idx<rtcp_common_header_get_rc(&rr->ch)){
         ortp_warning("RTCP packet should include a report_block_t at pos %i but has no space for it.",idx);
      }
   }
   return nullptr;
}

/*SDES accessors */
bool_t rtcp_is_SDES(const mblk_t *m)
{
   const rtcp_common_header_t *ch = rtcp_get_common_header(m);

   if (ch && rtcp_common_header_get_packet_type(ch)==RTCP_SDES)
   {
      if(natural(msgdsize(m)) < sizeof(rtcp_common_header_t)+(4*rtcp_common_header_get_length(ch)))
      {
         ortp_warning("Too i16 RTCP SDES packet.");
         return FALSE;
      }
      return TRUE;
   }
   return FALSE;
}

void rtcp_sdes_parse(const mblk_t *m, SdesItemFoundCallback cb, void *user_data){
   u8 *rptr=(u8*)m->b_rptr+sizeof(rtcp_common_header_t);
   const rtcp_common_header_t *ch=(rtcp_common_header_t*)m->b_rptr;
   u8 *end=rptr+sizeof(rtcp_common_header_t)+
      (4*rtcp_common_header_get_length(ch));
   u32 ssrc=0;
   i32 nchunk=0;
   bool_t chunk_start=TRUE;

   if (end>(u8*)m->b_wptr) end=(u8*)m->b_wptr;

   while(rptr<end){
      if (chunk_start){
         if (rptr+4<=end){
            ssrc=ntohl(*(u32*)rptr);
            rptr+=4;
         }else{
            ortp_warning("incorrect chunk start in RTCP SDES");
            break;
         }
         chunk_start=FALSE;
      }else{
         if (rptr+2<=end){
            u8 type=rptr[0];
            u8 len=rptr[1];

            if (type==RTCP_SDES_END){
               /* pad to next 32bit boundary*/
               rptr=(u8*)(((uptr)rptr+4) & ~0x3);
               nchunk++;
               if (nchunk<rtcp_common_header_get_rc(ch)){
                  chunk_start=TRUE;
                  continue;
               }else break;
            }
            rptr+=2;
            if (rptr+len<=end){
               cb((void *)user_data,ssrc,(rtcp_sdes_type_t)type,(char*)rptr,len);
               rptr+=len;
            }else{
               ortp_warning("bad item length in RTCP SDES");
               break;
            }
         }else{
            /*end of packet */
            break;
         }
      }
   }
}

/*BYE accessors */
bool_t rtcp_is_BYE(const mblk_t *m)
{
   const rtcp_common_header_t *ch=rtcp_get_common_header(m);
   if (ch && rtcp_common_header_get_packet_type(ch)==RTCP_BYE)
   {
      if(natural(msgdsize(m)) <sizeof(rtcp_common_header_t)+rtcp_common_header_get_length(ch))
      {
         ortp_warning("Too i16 RTCP BYE packet.");
         return FALSE;
      }
      return TRUE;
   }
   return FALSE;
}

bool_t rtcp_BYE_get_ssrc(const mblk_t *m, i32 idx, u32 *ssrc){
   rtcp_bye_t *bye=(rtcp_bye_t*)m->b_rptr;
   i32 rc=rtcp_common_header_get_rc(&bye->ch);
   i32 len=rtcp_common_header_get_length(&bye->ch);
   if (idx<rc){
      if ((u8*)&bye->ssrc[idx]<=(m->b_rptr
         +sizeof(rtcp_common_header_t)+len-4)) {
            *ssrc=ntohl(bye->ssrc[idx]);
            return TRUE;
      }else{
         ortp_warning("RTCP BYE should contain %i ssrc, but there is not enough room for it.");
      }
   }
   return FALSE;
}

bool_t rtcp_BYE_get_reason(const mblk_t *m, const char **reason, i32 *reason_len){
   rtcp_bye_t *bye=(rtcp_bye_t*)m->b_rptr;
   i32 rc=rtcp_common_header_get_rc(&bye->ch);
   i32 len=rtcp_common_header_get_length(&bye->ch);
   u8 *rptr=(u8*)m->b_rptr+sizeof(rtcp_common_header_t)+rc*4;
   u8 *end=(u8*)(m->b_rptr+sizeof(rtcp_common_header_t)+len);
   if (rptr<end){
      u8 content_len=rptr[0];
      if (rptr+1+content_len<=end){
         *reason=(char*)rptr+1;
         *reason_len=content_len;
         return TRUE;
      }else{
         ortp_warning("RTCP BYE has not enough space for reason phrase.");
         return FALSE;
      }
   }
   return FALSE;
}

/*APP accessors */
bool_t rtcp_is_APP(const mblk_t *m)
{
   const rtcp_common_header_t *ch=rtcp_get_common_header(m);
   if (ch!=nullptr && rtcp_common_header_get_packet_type(ch)==RTCP_APP)
   {
      if(natural(msgdsize(m)) < sizeof(rtcp_common_header_t) + rtcp_common_header_get_length(ch))
      {
         ortp_warning("Too i16 RTCP APP packet.");
         return FALSE;
      }
      if(sizeof(rtcp_common_header_t) + rtcp_common_header_get_length(ch) < sizeof(rtcp_app_t))
      {
         ortp_warning("Bad RTCP APP packet.");
         return FALSE;
      }
      return TRUE;
   }
   return FALSE;
}

i32 rtcp_APP_get_subtype(const mblk_t *m){
   rtcp_app_t *app=(rtcp_app_t*)m->b_rptr;
   return rtcp_common_header_get_rc(&app->ch);
}

u32 rtcp_APP_get_ssrc(const mblk_t *m){
   rtcp_app_t *app=(rtcp_app_t*)m->b_rptr;
   return ntohl(app->ssrc);
}
/* name argument is supposed to be at least 4 characters (note: no '\0' written)*/
void rtcp_APP_get_name(const mblk_t *m, char *name){
   rtcp_app_t *app=(rtcp_app_t*)m->b_rptr;
   ::memcpy_dup(name,app->name,4);
}
/* retrieve the data. when returning, data pts directly into the mblk_t */
void rtcp_APP_get_data(const mblk_t *m, u8 **data, i32 *len){
   rtcp_app_t *app=(rtcp_app_t*)m->b_rptr;
   i32 datalen=sizeof(rtcp_common_header_t)+rtcp_common_header_get_length(&app->ch)-8;
   if (datalen>0){
      *data=(u8*)m->b_rptr+sizeof(rtcp_app_t);
      *len=datalen;
   }else{
      *len=0;
      *data=nullptr;
   }
}

/*old functions: deprecated, but some useful code parts can be reused */
/* Start from now this source code file was written by Nicola Baldo as an extension of 
the oRTP library. Copyright (C) 2005 Nicola Baldo nicola@baldo.biz*/

void report_block_parse(RtpSession *session, report_block_t *rb, struct timeval rcv_time_tv)
{         
   rb->ssrc = ntohl(rb->ssrc);

   if ( rb->ssrc != session->snd.ssrc )

   {
      ortp_debug("Received rtcp report block related to unknown ssrc (not from us)... discarded");
      return;
   }       

   else

   {
      u32 rcv_time_msw;      
      u32 rcv_time_lsw;
      u32 rcv_time;
      double rtt;

      rcv_time_msw = (u32) rcv_time_tv.tv_sec;
#if defined(_WIN32_WCE)
      rcv_time_lsw = (u32) ((double)rcv_time_tv.tv_usec*(double)(((u64)1)<<32)*1.0e-6);
#else
      rcv_time_lsw = (u32) ((double)rcv_time_tv.tv_usec*(double)(1LL<<32)*1.0e-6);
#endif
      rcv_time = (rcv_time_msw<<16) | (rcv_time_lsw >> 16);

      /*
      rb->cum_num_packet_lost = ntoh24(rb->cum_num_packet_lost);
      rb->ext_high_seq_num_rec = ntohl(rb->ext_high_seq_num_rec);
      rb->interarrival_jitter = ntohl(rb->interarrival_jitter);
      rb->lsr = ntohl(rb->lsr);
      rb->delay_snc_last_sr = ntohl(rb->delay_snc_last_sr);
      */

      /* calculating Round Trip Time*/ 
      if (rb->lsr != 0)
      {
         rtt = (double) (rcv_time - rb->delay_snc_last_sr - rb->lsr);
         rtt = rtt/65536;     
         //debug_print("RTT = %f s\n",rtt);
      }

   }

}

void rtp_session_rtcp_parse(RtpSession *session, mblk_t *mp)
{
   rtcp_common_header_t *rtcp;
   i32 msgsize;
   i32 rtcp_pk_size;
   RtpStream *rtpstream=&session->rtp;
   struct timeval rcv_time_tv;


   gettimeofday(&rcv_time_tv,nullptr);

   return_if_fail(mp!=nullptr);

   msgsize=(i32) (mp->b_wptr-mp->b_rptr);

   if (msgsize < RTCP_COMMON_HEADER_SIZE)
   {
      ortp_debug("Receiving too i16 rtcp packet... discarded");
      return;
   }

   rtcp=(rtcp_common_header_t *)mp->b_rptr;

   /* compound rtcp packet can be composed by more than one rtcp message */
   while (msgsize >= RTCP_COMMON_HEADER_SIZE)
   {

      if (rtcp->version!=2)
      {
         ortp_debug("Receiving rtcp packet with version number !=2...discarded");
         return;
      }

      /* convert header data from network order to host order */
      rtcp->length = ntohs(rtcp->length);

      /* compute length */
      rtcp_pk_size = (rtcp->length + 1) * 4;
      /* Sanity check of simple RTCP packet length. */
      if (rtcp_pk_size > msgsize)
      {
         ortp_debug("Receiving rtcp packet shorter than the specified length.. discared");
         return;
      }

      switch (rtcp->packet_type)   

      {

      case RTCP_SR:

         {
            rtcp_sr_t *sr = (rtcp_sr_t *) rtcp;
            report_block_t *rb;
            i32 i;

            if ( ntohl(sr->ssrc) != session->rcv.ssrc )
            {
               ortp_debug("Receiving rtcp sr packet from unknown ssrc.. discarded");      
               return;
            }       

            if (msgsize < RTCP_COMMON_HEADER_SIZE + RTCP_SSRC_FIELD_SIZE + RTCP_SENDER_INFO_SIZE + (RTCP_REPORT_BLOCK_SIZE*sr->ch.rc))
            {
               ortp_debug("Receiving too i16 rtcp sr packet... discarded");
               return;
            }       

            /* parsing RTCP Sender Info */ 
            sr->si.ntp_timestamp_msw = ntohl(sr->si.ntp_timestamp_msw);
            sr->si.ntp_timestamp_lsw = ntohl(sr->si.ntp_timestamp_lsw);
            sr->si.rtp_timestamp = ntohl(sr->si.rtp_timestamp);
            sr->si.senders_packet_count = ntohl(sr->si.senders_packet_count);
            sr->si.senders_octet_count = ntohl(sr->si.senders_octet_count);       

            /* saving data to fill LSR and DLSR field in next RTCP report to be transmitted  */
            rtpstream->last_rcv_SR_ts = (sr->si.ntp_timestamp_msw << 16) | (sr->si.ntp_timestamp_lsw >> 16);  
            rtpstream->last_rcv_SR_time.tv_usec = rcv_time_tv.tv_usec;
            rtpstream->last_rcv_SR_time.tv_sec = rcv_time_tv.tv_sec;              


            /* parsing all RTCP report blocks */         
            for (i=0; i<sr->ch.rc; i++)
            { 
               rb = &(sr->rb[i]);      
               report_block_parse(session, rb, rcv_time_tv);      
            }

         }
         break;



      case RTCP_RR:

         {
            rtcp_rr_t *rr = (rtcp_rr_t *) rtcp;
            report_block_t *rb;
            i32 i;

            if (session->rcv.ssrc == 0)
            {
               /* rcv.ssrc is not set, so we adopt the incoming one */
               session->rcv.ssrc = ntohl(rr->ssrc);
            }
            else if ( ntohl(rr->ssrc) != session->rcv.ssrc )
            {
               ortp_debug("Receiving rtcp rr packet from unknown ssrc.. discarded");
               return;
            }

            if (msgsize < RTCP_COMMON_HEADER_SIZE + RTCP_SSRC_FIELD_SIZE + (RTCP_REPORT_BLOCK_SIZE*rr->ch.rc))
            {
               ortp_debug("Receiving too i16 rtcp sr packet... discarded");
               return;
            }       

            /* parsing all RTCP report blocks */         
            for (i=0; i<rr->ch.rc; i++)
            { 
               rb = &(rr->rb[i]);      
               report_block_parse(session, rb, rcv_time_tv);      
            }

         }
         break;


      case RTCP_SDES: 
         /* to be implemented */
         break;


      case RTCP_BYE:
         {
            rtcp_bye_t *bye = (rtcp_bye_t *) rtcp;
            u32 sclen = bye->ch.rc * 4;
            i32 reason_space_len = rtcp_pk_size
               - sizeof (rtcp_common_header_t)
               - sclen;
            i32 i;
            char *reason = nullptr;
            bool_t rcv_ssrc_match = FALSE;

            if (reason_space_len < 0) {
               ortp_debug("Receiving too i16 RTCP BYE packet... discarded");
               return;
            }
            for (i = 0; i < bye->ch.rc; i++) {
               if (ntohl(bye->ssrc[i]) == session->rcv.ssrc) {
                  rcv_ssrc_match = TRUE;
                  break;
               }
            }
            if (rcv_ssrc_match) {
               if (session->on_rtcp_bye.count > 0) {
                  /* Get reason. */
                  if (reason_space_len > 1) {
                     u8 *reasonbuf = (u8 *) rtcp
                        + sizeof (rtcp_common_header_t)
                        + sclen;
                     if (reasonbuf[0] <= reason_space_len-1)
                        reason = ortp_strndup((char *)(reasonbuf+1), reasonbuf[0]);
                     else
                        ortp_debug("Incorrect RTCP BYE reason length");
                  }
                  rtp_signal_table_emit2(&session->on_rtcp_bye,
                     (uptr)reason);
                  if (reason)
                     ortp_free(reason);
               } else {
                  ortp_debug("Got RTCP BYE without RTCP BYE handler");
               }
            } else {
               ortp_debug("No SSRC in the BYE packet matched our rcv.ssrc.");
            }
            break;
         }

      case RTCP_APP:
         /* to be implemented */
         break;


      default:

         ortp_debug("Receiving unknown rtcp packet type... discarded");
         return;

      }


      msgsize -= rtcp_pk_size;              /* size of unparsed portion of UDP packet, in octets */
      rtcp = (rtcp_common_header_t *) (rtcp_pk_size + (char *) rtcp);  /* pter to next RTCP packet in current UDP packet */

   }

   /* The function did not failed sanity checks, write down the RTPC/RTCP
   reception time. */
   session->last_recv_time = rcv_time_tv;
}
