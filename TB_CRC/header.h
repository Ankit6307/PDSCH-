#ifndef _HEADER_H_
#define _HEADER_H_
#include <hls_stream.h>
#include <ap_int.h>
#include <stdio.h>
#include <ap_fixed.h>
#include <math.h>
#include<iostream>
#include<algorithm>
#include <hls_math.h>
#include<ap_axi_sdata.h>
#include<fstream>
#include<iomanip>

using namespace std;

typedef ap_uint<128> datau128b;
typedef ap_uint<108> datau108b;
typedef ap_uint<64> datau64b;
typedef ap_uint<32> datau32b;
typedef ap_uint<8> datau8b;
typedef ap_uint<16> datau16b;
typedef ap_uint<17> datau17b;
typedef ap_uint<19> datau19b;
typedef ap_uint<240> datau240b;
typedef ap_uint<5>datau5b;
typedef ap_uint<6>datau6b;
typedef ap_uint<20> datau20b;
typedef ap_uint<25> datau25b;
datau25b lookup(int i, int j, int k, int L);
struct data_axi
{
	ap_uint<128> data;
	ap_uint<1> last;
};

typedef hls::stream<data_axi> STREAM_T;
typedef hls::stream<datau128b> cn_STREAM;

void tb_crc(STREAM_T &inData, cn_STREAM &cnData, STREAM_T &outData);
datau25b lookup(int i, int j, int k);
#endif
