#ifndef _HEADER_H_
#define _HEADER_H_
#include <hls_stream.h>
#include <ap_int.h>
#include <stdio.h>
#include <ap_fixed.h>
#include <math.h>
#include <iostream>
#include <algorithm>
#include <hls_math.h>
#include <ap_axi_sdata.h>
#include <fstream>
#include <string>
#include <bits/stdc++.h>

using namespace std;
using namespace hls;

typedef ap_uint<128> datau128b;
typedef ap_uint<96> datau96b;
typedef ap_uint<64> datau64b;
typedef ap_uint<32> datau32b;
typedef ap_uint<22> datau22b;
typedef ap_uint<19> datau19b;
typedef ap_uint<18> datau18b;
typedef ap_uint<16> datau16b;
typedef ap_uint<15> datau15b;
typedef ap_uint<14> datau14b;
typedef ap_uint<10> datau10b;
typedef ap_uint<9> datau9b;
typedef ap_uint<8> datau8b;
typedef ap_uint<7> datau7b;
typedef ap_uint<6> datau6b;
typedef ap_uint<3> datau3b;
typedef ap_uint<2> datau2b;
typedef ap_uint<1> datau1b;

struct input_data_axi
{
    ap_uint<128> data;
    ap_uint<7> tkeep;
    ap_uint<1> tlast;
};

struct output_data_axi
{
	ap_uint<96> data;
	ap_uint<1> tlast;
};

typedef hls::stream<input_data_axi> in_STREAM;
typedef hls::stream<output_data_axi> out_STREAM;
typedef hls::stream<datau128b> cn_STREAM;

void Interleaver(in_STREAM &inData, cn_STREAM &cnData, out_STREAM &outData);

#endif

