
#pragma  once

//#pragma pack(push, 1)

typedef struct _iphdr {
	UCHAR		header_len:4;
	UCHAR		version:4;   
	UCHAR		tos;            // type of service
	USHORT		total_len;      // length of the packet
	USHORT		ident;          // unique identifier
	USHORT		flags;          
	UCHAR		ttl;            
	UCHAR		proto;          // protocol ( IP , TCP, UDP etc)
	USHORT		checksum;       
	ULONG		saddr;
	ULONG		daddr;
	
}iphdr;

#define ICMP_ECHOREPLY		0	/* Echo Reply			*/
#define ICMP_DEST_UNREACH	3	/* Destination Unreachable	*/
#define ICMP_SOURCE_QUENCH	4	/* Source Quench		*/
#define ICMP_REDIRECT		5	/* Redirect (change route)	*/
#define ICMP_ECHO			8	/* Echo Request			*/
#define ICMP_TIME_EXCEEDED	11	/* Time Exceeded		*/
#define ICMP_PARAMETERPROB	12	/* Parameter Problem		*/
#define ICMP_TIMESTAMP		13	/* Timestamp Request		*/
#define ICMP_TIMESTAMPREPLY	14	/* Timestamp Reply		*/
#define ICMP_INFO_REQUEST	15	/* Information Request		*/
#define ICMP_INFO_REPLY		16	/* Information Reply		*/
#define ICMP_ADDRESS		17	/* Address Mask Request		*/
#define ICMP_ADDRESSREPLY	18	/* Address Mask Reply		*/


/* Codes for UNREACH. */
typedef struct _icmphdr {
	UCHAR		type;
	UCHAR		code;
	USHORT		checksum;
	union {
		struct {
			USHORT	id;
			USHORT	sequence;
		} echo;
		ULONG	gateway;
	} un;
}icmphdr;

#define LITTLE_ENDIAN	0
#define BIG_ENDIAN		1
#define BYTE_ORDER		BIG_ENDIAN

typedef struct _tcphdr {
	USHORT	source;
	USHORT	dest;
	ULONG	seq;
	ULONG	ack_seq;
	
#if BYTE_ORDER == LITTLE_ENDIAN
	USHORT	doff:4,
			res1:4,
#endif
#if BYTE_ORDER == BIG_ENDIAN
	USHORT  res1:4,
			doff:4,
#endif	
			res2:2,
			urg:1,
			ack:1,
			psh:1,
			rst:1,
			syn:1,
			fin:1;
	USHORT	window;
	USHORT	check;
	USHORT	urg_ptr;
}tcphdr;

typedef struct _udphdr {
	USHORT		source;
	USHORT		dest;
	USHORT		len;
	USHORT		check;
}udphdr;

//#pragma pack(pop)
