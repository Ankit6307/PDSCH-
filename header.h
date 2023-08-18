#ifndef HEADER_H
#define HEADER_H

#include <bits/stdc++.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <ap_fixed.h>
#include <math.h>
#include <hls_math.h>

using namespace std;
using namespace hls;

struct data_axi
{
	ap_uint<128> data;
//	ap_uint<1> last;                               //last
};
struct data_axi_40
{
	ap_uint<40> data;
	ap_uint<1> last;
};

typedef hls::stream<data_axi> STREAM_T;
typedef hls::stream<data_axi_40> STREAM_U;

typedef ap_uint<128> datau128b;
typedef ap_uint<64> datau64b;
typedef ap_uint<40> datau40b;
typedef ap_uint<9> datau9b;
typedef ap_uint<5> datau5b;
typedef ap_uint<3> datau3b;
typedef ap_uint<1> datau1b;
typedef ap_uint<1> datau6b;
//
//const int a[8] = {2, 3, 5, 7, 9, 11, 13, 15};

void LDPC_Config(STREAM_T &config,  STREAM_U &ctrl);
#endif
