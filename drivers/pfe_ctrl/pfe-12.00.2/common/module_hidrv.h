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


#ifndef _HIDRV_H
#define _HIDRV_H

#include "channels.h"
#include "modules.h"
#include "fpart.h"

U32 FCODE_TO_EVENT(U32 fcode);

#if !defined(COMCERTO_2000_CONTROL)

typedef struct tHostCommand {
	U16	length;
	U16	code;
	U32	pad;		// keep data aligned on a DWORD boundary
	U16	data[128];
} HostCommand;

typedef struct tHostMessage {
	struct tHostMessage *next;
	U16	length;
	U16	code;
	U16	data[128];
} HostMessage;

void msg_free(HostMessage *msg);
HostMessage *msg_alloc(void);
void msg_send(HostMessage *msg);

#endif /* !defined(COMCERTO_2000_CONTROL) */

typedef U16 (*CmdProc)(U16, U16, U16 *);


extern CmdProc gCmdProcTable[];

#define set_cmd_handler(event, handler)		gCmdProcTable[event] = handler;

#if !defined (COMCERTO_2000)

extern HostCommand host_cmd;

BOOL M_hidrv_init(PModuleDesc pModule);
void M_hidrv_entry(void);

extern DataQueue hostmsg_queue;

#define 	CMD_MBOX_0_ADDR		0x0A000000
#define 	CMD_MBOX_1_ADDR		0x0A000004
#define 	EVENT_MBOX_0_ADDR	0x0A000010
#define 	EVENT_MBOX_1_ADDR	0x0A000014

#define 	CMD_DATA_ADDR      		0x0A000020
#define 	EVENT_DATA_ADDR      	0x0A000220


#define 	M0_CMD			0x00000001
#define 	M0_ACK			0x00000002
#define 	M0_EVENT		0x00000004
#define 	TX_MAGIC			0x4D535044 // -> MSPD

#endif /* !defined (COMCERTO_2000) */


// Function codes
// 0x0000 -> 0x00FF : RX module
#define 	FC_RX				0x0000
#define	CMD_RX_ENABLE		0x0001	
#define	CMD_RX_DISABLE	0x0002	
#define CMD_RX_CNG_ENABLE	0x0003
#define CMD_RX_CNG_DISABLE	0x0004
#define CMD_RX_CNG_SHOW		0x0005
#define CMD_RX_FILL_BUFFERPOOL  0x0006
#define CMD_RX_FILTER_PROMISCUOUS	0x0007
#define CMD_RX_L2BRIDGE_ENABLE	0x0008
#define L2BRIDGE_FIRST_COMMAND	CMD_RX_L2BRIDGE_ENABLE
#define CMD_RX_L2BRIDGE_ADD	0x0009
#define CMD_RX_L2BRIDGE_REMOVE	0x000a
#define CMD_RX_L2BRIDGE_QUERY_STATUS	0x000b
#define CMD_RX_L2BRIDGE_QUERY_ENTRY	0x000c
#define CMD_RX_L2BRIDGE_FLOW_ENTRY	0x000d
#define CMD_RX_L2BRIDGE_MODE		0x000e
#define CMD_RX_L2BRIDGE_FLOW_TIMEOUT	0x000f
#define CMD_RX_L2BRIDGE_FLOW_RESET	0x0010

#define L2BRIDGE_LAST_COMMAND	CMD_RX_L2BRIDGE_FLOW_RESET
#define CMD_RX_LRO				0x0011

// 0x0100 -> 0x01FF : Ethernet module
#define 	FC_ETH				0x0001

// 0x0200 -> 0x02FF : QM module
#define 	FC_QM				0x0002
#define CMD_QM_QOSENABLE			0x0201
#define CMD_QM_QOSALG				0x0202
#define CMD_QM_NHIGH				0x0203
#define CMD_QM_MAX_TXDEPTH			0x0204
#define CMD_QM_MAX_QDEPTH			0x0205
#define CMD_QM_MAX_WEIGHT			0x0206
#define CMD_QM_RATE_LIMIT			0x0207
#define CMD_QM_EXPT_RATE			0x020c
#define CMD_QM_QUERY				0x020d
#define CMD_QM_QUERY_EXPT_RATE  	0x020e

#define CMD_QM_RESET			0x0210
#define CMD_QM_SHAPER_CONFIG		0x0211
#define CMD_QM_SCHEDULER_CONFIG		0x0212
#define CMD_QM_DSCP_QM			0x0213

#define CMD_QM_QUEUE_QOSENABLE	0x0214


// 0x0300 -> 0x03FF : IPv4 module
#define FC_IPV4						0x0003
#define CMD_IP_ROUTE				0x0313
#define CMD_IPV4_CONNTRACK			0x0314
#define CMD_IPV4_CONNTRACK_CHANGE	0x0315
#define CMD_IPV4_RESET				0x0316
#define CMD_IPV4_SET_TIMEOUT 		0x0319
#define CMD_IPV4_GET_TIMEOUT 		0x0320
#define CMD_IPV4_FF_CONTROL		0x0321

#define CMD_IPV4_SOCK_OPEN		0x0330
#define CMD_IPV4_SOCK_CLOSE		0x0331
#define CMD_IPV4_SOCK_UPDATE		0x0332
#define CMD_IPV4_FRAGTIMEOUT		0x0333
#define CMD_IPV4_SAM_FRAGTIMEOUT	0x0334


// 0x0400 -> 0x04FF : IPv6 module
#define FC_IPV6						0x0004
#define CMD_IPV6_CONNTRACK			0x0414
#define CMD_IPV6_CONNTRACK_CHANGE	0x0415
#define CMD_IPV6_RESET				0x0416
#define CMD_IPV6_GET_TIMEOUT 		0x0420
#define CMD_IPV6_SOCK_OPEN		0x0430
#define CMD_IPV6_SOCK_CLOSE		0x0431
#define CMD_IPV6_SOCK_UPDATE		0x0432
#define CMD_IPV6_FRAGTIMEOUT		0x0433


// 0x0500 -> 0x05FF : Tx module
#define 	FC_TX				0x0005
#define	CMD_TX_ENABLE		0x0501	
#define	CMD_TX_DISABLE	0x0502	
#define CMD_LINK_UP		0x0504
#define CMD_PORT_UPDATE		0x0505

// 0x0600 -> 0x06FF : PPPoE module
#define FC_PPPOE                    0x0006
#define CMD_PPPOE_ENTRY             0x0601
#define CMD_PPPOE_ENTRY_CHANGE    	0x0602
#define CMD_PPPOE_GET_IDLE			0x0603
#define CMD_PPPOE_RELAY_ENTRY   	0x0610


// 0x0700 -> 0x07FF : MC4 and MC6 modules
#define	FC_MC						0x0007
#define	CMD_MC4_MULTICAST		0x0701
#define	CMD_MC4_RESET				0x0702
#define	CMD_MC6_MULTICAST		0x0703
#define	CMD_MC6_RESET				0x0704
#define CMD_MC6_MODE				0x0705

// 0x0800 -> 0x08FF : RTP relay module
#define	FC_RTP						0x0008
#define	CMD_RTP_OPEN				0x0801
#define	CMD_RTP_UPDATE			0x0802
#define	CMD_RTP_TAKEOVER			0x0803
#define	CMD_RTP_CONTROL			0x0804
#define	CMD_RTP_SPECTX_PLD		0x0805
#define	CMD_RTP_SPECTX_CTRL		0x0806
#define	CMD_RTCP_QUERY			0x0807
#define	CMD_RTP_CLOSE				0x0808

#define CMD_RTP_STATS_ENABLE		0x0810
#define CMD_RTP_STATS_DISABLE		0x0811
#define CMD_RTP_STATS_QUERY			0x0812
#define CMD_RTP_STATS_DTMF_PT		0x0813

#define CMD_VOICE_BUFFER_LOAD		0x0820
#define CMD_VOICE_BUFFER_UNLOAD		0x0821
#define CMD_VOICE_BUFFER_START		0x0822
#define CMD_VOICE_BUFFER_STOP		0x0823
#define CMD_VOICE_BUFFER_RESET		0x0824

// 0x0900 -> 0x09FF : VLAN module
#define FC_VLAN                     0x0009
#define CMD_VLAN_ENTRY				0x0901
#define CMD_VLAN_ENTRY_RESET		0x0902

// 0x0a00 -> 0x0aff : IPSec module
#define   FC_IPSEC		                     0x000A
#define 	CMD_IPSEC_SA_CREATE		0x0A01	
#define 	CMD_IPSEC_SA_DELETE 		0x0A02
#define 	CMD_IPSEC_SA_FLUSH 		0x0A03
#define 	CMD_IPSEC_SA_SET_KEYS 	0x0A04
#define 	CMD_IPSEC_SA_SET_TUNNEL 	0x0A05
#define 	CMD_IPSEC_SA_SET_NATT 	0x0A06
#define 	CMD_IPSEC_SA_SET_STATE 	0x0A07
#define 	CMD_IPSEC_SA_SET_LIFETIME 	0x0A08
#define   	CMD_IPSEC_SA_NOTIFY		0x0A09
#define   	CMD_IPSEC_SA_ACTION_QUERY	0x0A0A
#define 	CMD_IPSEC_SA_ACTION_QUERY_CONT	0x0A0B
#define   	CMD_IPSEC_FRAG_CFG		0x0A14

// 0x0b00 -> 0x0bff : Tunnel module
#define   FC_TNL		                     0x000B
#define 	CMD_TNL_CREATE		0x0B01	
#define 	CMD_TNL_DELETE 		0x0B02
#define 	CMD_TNL_UPDATE		0x0B03
#define	CMD_TNL_IPSEC			0x0B04
#define	CMD_TNL_QUERY			0x0B05
#define	CMD_TNL_QUERY_CONT		0x0B06
#define CMD_TNL_4o6_ID_CONVERSION_dupsport 0x0B07
#define CMD_TNL_4o6_ID_CONVERSION_psid  0x0B08

// 0x0c00 -> 0x0cFF : QM module
#define 	FC_EXPT					0x000c
#define  CMD_EXPT_QUEUE_DSCP		0x0c01
#define  CMD_EXPT_QUEUE_CTRL		0x0c02
#define  CMD_EXPT_QUEUE_RESET		0x0c03

// 0x0d00-> 0x0dFF : Packet capture module

#define FC_PKTCAP			0x000d
#define CMD_PKTCAP_ENABLE		0x0d01
#define CMD_PKTCAP_IFSTATUS		0x0d02
#define CMD_PKTCAP_SLICE		0x0d04
#define CMD_PKTCAP_FLF			0x0d03
#define CMD_PKTCAP_QUERY		0x0d05

/* 0x0e00 -> 0x0eff : Stat module */
#define FC_STAT                          	0x000E
#define CMD_STAT_ENABLE                 	0x0E01 
#define CMD_STAT_QUEUE                 		0x0E02  
#define CMD_STAT_INTERFACE_PKT              	0x0E03
#define CMD_STAT_CONN           		0x0E04
#define CMD_STAT_PPPOE_STATUS			0x0E05
#define CMD_STAT_PPPOE_ENTRY			0x0E06
#define CMD_STAT_BRIDGE_STATUS			0x0E07
#define CMD_STAT_BRIDGE_ENTRY			0x0E08
#define CMD_STAT_IPSEC_STATUS			0x0E09
#define CMD_STAT_IPSEC_ENTRY			0x0E0A
#define CMD_STAT_VLAN_STATUS			0x0E0B
#define CMD_STAT_VLAN_ENTRY			0x0E0C

// 0x0f00 ->0x0fff : Trace/Profiling/Debugging
#define         FC_TRC                          	0x000f
#define CMD_TRC_ON                              0x0f01
#define CMD_TRC_OFF                             0x0f02
#define CMD_TRC_SWITCH                          0x0f03
#define CMD_TRC_DMEM                            0x0f04
#define CMD_TRC_SETMASK                         0x0f05
#define CMD_TRC_SHOW                            0x0f06
#define CMD_TRC_BSYCPU                          0x0f07
#define CMD_TRC_STATUS                          0x0f08
/* Command return codes */
#define CMD_TRC_UNIMPLEMENTED	0x0f7f
#define CMD_TRC_PARTIAL         0xf00
#define CMD_TRC_ERR             0xffe

// 0x1000: Alternate Configuration
#define FC_ALTCONF				0x0010
#define CMD_ALTCONF_SET			0x1001
#define CMD_ALTCONF_RESET		0x1002

#ifdef CFG_WIFI_OFFLOAD
//0x2000: WiFi Rx module
#define FC_WIFI_RX			0x0020 
#define CMD_WIFI_VAP_ENTRY              0x2001
#define CMD_CFG_WIFI_OFFLOAD                 0x2002
#define CMD_WIFI_DISABLE                0x2003
#define CMD_WIFI_VAP_QUERY              0x2004
#define CMD_WIFI_VAP_RESET              0x2005
#endif

#if !defined(COMCERTO_100)
// 0x1400 -> 0x14ff : MACVLAN module
#define   	FC_MACVLAN		        0x0014
#define 	CMD_MACVLAN_ENTRY		0x1401	
#define 	CMD_MACVLAN_ENTRY_RESET 	0x1402
#endif
// 0x1100: NAT-PT Module
#define FC_NATPT				0x0011
#define CMD_NATPT_OPEN				0x1101
#define CMD_NATPT_CLOSE				0x1102
#define CMD_NATPT_QUERY				0x1103


// 0x1200: Fppdiag configuration
#define FC_FPPDIAG					0x0012
#define CMD_FPPDIAG_ENABLE		0x1201
#define CMD_FPPDIAG_DISABLE		0x1202
#define CMD_FPPDIAG_UPDATE		0x1203
#define CMD_FPPDIAG_DUMP_CTRS 	0x1204

// 0x1500: ICC
#define FC_ICC						0x0015
#define CMD_ICC_RESET			0x1500
#define CMD_ICC_THRESHOLD		0x1501
#define CMD_ICC_ADD_DELETE		0x1502
#define CMD_ICC_QUERY			0x1503

// 0x1600: L2TP
#define FC_L2TP					0x0016
#define CMD_L2TP_ITF_ADD		0x1600
#define CMD_L2TP_ITF_DEL		0x1601

// return code
#define CMD_OK	0x0000
#define CMD_ERR 0xFFFE
#endif
