/*
 *  Copyright (c) 2011, 2014 Freescale Semiconductor, Inc.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 *
 */


#ifndef _MODULE_TUNNEL_H_
#define _MODULE_TUNNEL_H_

#include "channels.h"
#include "modules.h"
#include "fpart.h"
#include "fe.h"
#include "module_ethernet.h"
#include "layer2.h"
#include "module_ipv6.h"
#include "module_ipv4.h"


#define TNL_MAX_HEADER		(40 + 14 + 2)
#define TNL_MAX_TUNNEL		2


#define TNL_STATE_FREE 				0x00
#define TNL_STATE_CREATED 			0x01
#define TNL_STATE_ENABLED			0x02
#define TNL_STATE_SA_COMPLETE			0x04
#define TNL_STATE_SAREPLY_COMPLETE		0x08
#define TNL_STATE_REMOTE_ANY			0x10


#define TNL_NOSET_PRIV				0
#define TNL_SET_PRIV				1

enum TNL_MODE {
	TNL_MODE_ETHERIPV6,
	TNL_MODE_6O4,
	TNL_MODE_4O6,
	TNL_MODE_ETHERIPV4,
};

enum SAM_ID_CONV_TYPE {
	SAM_ID_CONV_NONE =0,
	SAM_ID_CONV_DUPSPORT =1, 
	SAM_ID_CONV_PSID =2, 
};

#define TNL_ETHERIP_VERSION 0x3000
#define TNL_ETHERIP_HDR_LEN 2

//#define TNL_DBG
#undef TNL_DBG

#ifdef TNL_DBG
/* debug entries */
enum TNL_DBG_SLOT {
	TNL_DBG_START = 0,
	TNL_DBG_NUM_ENTRIES,
	TNL_DBG_COMPLETE,
	TNL_DBG_SA_IN_NOMATCH,
	TNL_DBG_SANUM_IN_NOMATCH,
	TNL_DBG_TNL_IN_NOMATCH,
	TNL_DBG_IN_RXBRIDGE,
	TNL_DBG_IN_EXCEPT,
	TNL_DBG_OUT_EXCEPT_1,
	TNL_DBG_OUT_EXCEPT_2,
	TNL_DBG_OUT_EXCEPT_3,
	TNL_DBG_L2BRIDGE_NO_MATCH,
	TNL_DBG_END,
};

/* debug flags */
#define TNL_DBG_SA_COMPLETE	0x2
#define TNL_DBG_SAREPLY_COMPLETE	0x4
#endif


/***********************************
* Tunnel API Command and Entry strutures
*
************************************/

typedef struct _tTNLCommand_create {
	U8	name[16];
	U32 	local[4];
	U32	remote[4];
	U8	output_device[16];
	U8	mode;
	/* options */
	U8 	secure;
	U8	elim;
	U8	hlim;
	U32	fl;
	U16	frag_off;
	U16	enabled;
	U32	route_id;
}TNLCommand_create , *PTNLCommand_create;


typedef struct _tTNLCommand_delete {
	U8	name[16];
}TNLCommand_delete, *PTNLCommand_delete;


typedef struct _tTNLCommand_ipsec {
	U8	name[16];
	U16 	SA_nr;
	U16	SAReply_nr;
	U16	SA_handle[4];
	U16	SAReply_handle[4];
} TNLCommand_ipsec, *PTNLCommand_ipsec;

typedef struct _tTNLCommand_query{
        U16     result;
        U16     unused;
        U8      name[16];
        U32     local[4];
        U32     remote[4];
        U8      mode;
        U8      secure;
	U8	elim;
	U8	hlim;
        U32     fl;
        U16     frag_off;
        U16     enabled;
	U32	route_id;
}TNLCommand_query , *PTNLCommand_query;

typedef struct {
        int  port_set_id;          /* Port Set ID               */
        int  port_set_id_length;   /* Port Set ID length        */
        int  psid_offset;          /* PSID offset               */
}sam_port_info_t;

typedef struct _tTNLCommand_IdConvDP {
        U16      IdConvStatus;
	U16	 Pad;
}TNLCommand_IdConvDP, *pTNLCommand_IdConvDP;

typedef struct _tTNLCommand_IdConvPsid {
        U8       name[16];
	sam_port_info_t sam_port_info;
        U32      IdConvStatus:1,
		 unused:31;
}TNLCommand_IdConvPsid, *pTNLCommand_IdConvPsid;



#if !defined(COMCERTO_2000)

typedef struct _tTnlEntry{
	struct itf itf;
	
	union {
	  U8	header[TNL_MAX_HEADER];
	  ipv4_hdr_t header_v4;
	};
	U8	header_size;
	U8	mode;
	U8	proto;
	U8	secure;
	U8	state;
	U32 	local[4];
	U32	remote[4];
	U8	hlim;
	U8	elim;
	U8	output_proto;
	U32 fl;
	U16 frag_off;
	PRouteEntry pRtEntry;
	U16 SAReply_nr;
	U16 SA_nr;
	U16 hSAEntry_in[SA_MAX_OP];
	U16 hSAEntry_out[SA_MAX_OP];
	U32 route_id;
	int tunnel_index;
	U16 sam_abit; 
	U16 sam_abit_max;
	U16 sam_kbit; 
	U16 sam_mbit; 
	U16 sam_mbit_max; 
	U8 sam_mbit_len; 
	U8 sam_abit_len; 
	U8 sam_kbit_len; 
	U8 sam_id_conv_enable;
}TnlEntry, *PTnlEntry;

#elif defined(COMCERTO_2000_CONTROL)

typedef struct _tTnlEntry{
	struct itf itf;
	struct hw_route route;
	
	union {
	  U8	header[TNL_MAX_HEADER];
	  ipv4_hdr_t header_v4;
	};
	U8	header_size;
	U8	mode;
	U8	proto;
	U8	secure;
	U8	state;
	U8	hlim;
	U8	elim;
	U8	output_proto;
	U32 	local[4];
	U32	remote[4];
	U32 fl;
	U16 frag_off;
	U16 SAReply_nr;
	U16 SA_nr;
	U16 hSAEntry_in[SA_MAX_OP];
	U16 hSAEntry_out[SA_MAX_OP];
	U16 sam_abit; 
	U16 sam_abit_max;
	U16 sam_kbit; 
	U16 sam_mbit; 
	U16 sam_mbit_max; 
	U8 sam_mbit_len; 
	U8 sam_abit_len; 
	U8 sam_kbit_len; 
	U8 sam_id_conv_enable;

	// following entries not used in data path
	U32 route_id;
	PRouteEntry pRtEntry;
	int tunnel_index;
}TnlEntry, *PTnlEntry;


typedef struct tTNL_context {
	TnlEntry	tunnel_table[TNL_MAX_TUNNEL];
}TNL_context;

#else	// defined(COMCERTO_2000)

typedef struct _tTnlEntry{
	struct itf itf;
	RouteEntry route;
	
	union {
	  U8	header[TNL_MAX_HEADER];
	  ipv4_hdr_t header_v4;
	};
	U8	header_size;
	U8	mode;
	U8	proto;
	U8	secure;
	U8	state;
	U8	hlim;
	U8	elim;
	U8	output_proto;
	U32 	local[4];
	U32	remote[4];
	U32 fl;
	U16 frag_off;
	U16 SAReply_nr;
	U16 SA_nr;
	U16 hSAEntry_in[SA_MAX_OP];
	U16 hSAEntry_out[SA_MAX_OP];
	U16 sam_abit; 
	U16 sam_abit_max;
	U16 sam_kbit; 
	U16 sam_mbit; 
	U16 sam_mbit_max; 
	U8 sam_mbit_len; 
	U8 sam_abit_len; 
	U8 sam_kbit_len; 
	U8 sam_id_conv_enable;
	// following entries not used in data path 
	U32 unused_route_id;
	U32 unused_pRtEntry;
	int unused_tunnel_index;
}TnlEntry, *PTnlEntry;


typedef struct tTNL_context {
	TnlEntry	tunnel_table[TNL_MAX_TUNNEL];
}TNL_context;

#endif	/* !defined(COMCERTO_2000) */

extern struct tTNL_context gTNLCtx;

void M_TNL_OUT_process_packet(PMetadata mtd);
void M_TNL_IN_process_packet(PMetadata mtd);

#if !defined(COMCERTO_2000)
BOOL M_tnl_in_init(PModuleDesc pModule);
BOOL M_tnl_out_init(PModuleDesc pModule);
void M_tnl_entry_in(void);
void M_tnl_entry_out(void);
#else
int tunnel_init(void);
void tunnel_exit(void);
#endif

int M_tnl_update_header(PTnlEntry pTunnelEntry, PMetadata mtd);
U16 Tnl_Get_Next_Hash_Entry(PTNLCommand_query pTnlCmd, int reset_action);
#ifdef TNL_DBG
void M_tnl_debug(void);
#endif

PTnlEntry M_tnl6_match_ingress(PMetadata mtd, ipv6_hdr_t *ipv6_hdr, U8 proto, U8 set_tnl);
PTnlEntry M_tnl4_match_ingress(PMetadata mtd, ipv4_hdr_t *ipv4_hdr, U8 proto);


static __inline void * M_tnl_add_header(PMetadata mtd,  PTnlEntry pTnlEntry)
{
	mtd->offset -= pTnlEntry->header_size;
	mtd->length += pTnlEntry->header_size;
	SFL_memcpy(mtd->data + mtd->offset, pTnlEntry->header, pTnlEntry->header_size);
	M_tnl_update_header(pTnlEntry, mtd);
	return (mtd->data + mtd->offset);
}


/* Inherited from linux-2.6.21.1/include/net/inet_ecn.h for tos management in 6o4 tunnels */
enum {
	INET_ECN_NOT_ECT = 0,
	INET_ECN_ECT_1 = 1,
	INET_ECN_ECT_0 = 2,
	INET_ECN_CE = 3,
	INET_ECN_MASK = 3,
};

static inline int INET_ECN_is_ce(U8 dsfield)
{
	return (dsfield & INET_ECN_MASK) == INET_ECN_CE;
}

static inline U8 INET_ECN_encapsulate(U8 outer, U8 inner)
{
	outer &= ~INET_ECN_MASK;
	outer |= !INET_ECN_is_ce(inner) ? (inner & INET_ECN_MASK) : INET_ECN_ECT_0;
	return outer;
}

static inline void ipv6_change_dsfield(ipv6_hdr_t *ipv6h,U8 mask, U8 value)
{
        U16 tmp;

        tmp = ntohs(*(U16*)ipv6h);
        tmp = (tmp & ((mask << 4) | 0xf00f)) | (value << 4);
        *(U16*) ipv6h = htons(tmp);
}


#endif /* _MODULE_TUNNEL_H_ */
