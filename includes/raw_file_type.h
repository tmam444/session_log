/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   raw_file_type.h                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chulee <chulee@nstek.com>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/10 14:02:15 by chulee            #+#    #+#             */
/*   Updated: 2023/04/27 10:55:24 by chulee           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RAW_FILE_TYPE_H
# define RAW_FILE_TYPE_H

struct Ufp_t
{
	unsigned short	file;
	unsigned int	offset;
} __attribute__ ((packed));

struct TrafficByte_t
{
	unsigned long long	byte:40;
} __attribute__ ((packed));

struct RawDataVer2_t
{
 unsigned int int_ipv4;
 unsigned int ext_ipv4;
 unsigned char int_ipv6[16];
 unsigned char ext_ipv6[16];
 unsigned char int_mac[6];
 unsigned char ext_mac[6];

 unsigned char protocol;
 unsigned char seg_num;
 unsigned short int_port;
 unsigned short ext_port;
 unsigned short aid;
 unsigned short int_cid; //version 2
 unsigned short ext_cid; //version 2
 unsigned short pid;
 struct TrafficByte_t int_byte;
 struct TrafficByte_t ext_byte;
 //unsigned char int_byte[5];
 //unsigned char ext_byte[5];
 unsigned int int_packet;
 unsigned int ext_packet;
 unsigned int int_flow; //version 2
 unsigned int ext_flow; //version 2
 unsigned char start_time;
 unsigned char end_time;
 struct Ufp_t url_fp;
 struct TrafficByte_t total_int_byte;
 struct TrafficByte_t total_ext_byte;
 //unsigned char total_int_byte[5];
 //unsigned char total_ext_byte[5];
 unsigned int total_int_packet;
 unsigned int total_ext_packet;
 unsigned int total_int_flow;
 unsigned int total_ext_flow;
// >>>>>>> 230323 hjkim - QoE 필드 추가
 unsigned int RTT;
 unsigned int delay[2];
 unsigned int retrans[2];
 unsigned int dropByte[2];
// <<<<<<< 230323 hjkim - QoE 필드 추가
}__attribute__ ((packed));

struct RawFileHeader2_t
{
	unsigned long long		time;
	unsigned char			version;
	struct RawDataVer2_t	raw[];
} __attribute__ ((packed));

#endif
