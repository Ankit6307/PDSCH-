#include "header.h"
#include <bitset>

int crc_poly;

//datau25b get_crc(datau128b tb_temp, datau25b crc_temp, int L){
//    for(int i=0;i<128;i++){
//#pragma HLS LOOP_TRIPCOUNT min=128 max=128
//		crc_temp *= 2;
//		crc_temp[0] = tb_temp[127-i];
//		if(crc_temp[L] == 1){
//			crc_temp = crc_temp ^ crc_poly;
//		}
//    }
//    return crc_temp;
//}


void cb_crc(STREAM_T &inData, cn_STREAM &cnData,STREAM_T &outData)
{
    #pragma HLS INTERFACE axis port=inData
    #pragma HLS INTERFACE axis port=outData
	#pragma HLS INTERFACE axis port=cnData
#pragma HLS INTERFACE ap_ctrl_none port=return

	int w = 1;

	datau128b cnTemp = cnData.read();
	datau19b TBS = cnTemp.range(18,0);
	datau6b C = cnTemp.range(27,22);
	datau14b K_ = cnTemp.range(85,72);
	datau1b Bg_no = cnTemp.range(19,19);
	datau9b Z_c = cnTemp.range(68,60);

	const int L = 24;
	int cbz = K_;
	if(C>1){
		cbz = K_ - L;
	}
	int L_ = 16;
	if(TBS>3824){
		L_ = 24;
	}
	int TBS_ = TBS + L_;
	int K = 10*Z_c;
	if(Bg_no == 0){
		K = 22*Z_c;
	}
    crc_poly = 25165923;

    data_axi input,output;

    int burst = (cbz+127)/128;
    int n = 128 * burst - cbz;
    int filler = K-K_;
    if(C>1){
    for(int block=0;block<152;block++){
#pragma HLS LOOP_TRIPCOUNT min=0 max=152
#pragma HLS PIPELINE
    	if(block==C)break;
    	filler = K-K_;
    	bool tlast = block == C-1;
    	datau25b crc_temp = 0;
    	for(int j=0;j<66;j++){
#pragma HLS LOOP_TRIPCOUNT min=0 max=66
#pragma HLS PIPELINE
    		if(j==burst-1)break;
			data_axi input = inData.read();
			datau128b tb_temp = input.data;
			output.last = 0;
			output.data = tb_temp;
			outData.write(output);
			datau25b crc = 0;
			for(int a=0;a<4;a++){
			#pragma HLS LOOP_TRIPCOUNT min=4 max=4
			#pragma HLS PIPELINE
				for(int b=0;b<4;b++){
				#pragma HLS LOOP_TRIPCOUNT min=4 max=4
				#pragma HLS PIPELINE
					int ind = a*32+b*8;
					int k = tb_temp.range(ind+7,ind);
					crc ^= lookup(a,b*8,k);
				}
			}
			datau32b crc_25_back = crc_temp;
			crc_temp = crc;
			for(int b=0;b<4;b++){
			#pragma HLS LOOP_TRIPCOUNT min=4 max=4
			#pragma HLS PIPELINE
				int ind = b*8;
				datau8b k = crc_25_back.range(ind+7,ind);
				crc_temp ^= lookup(4,b*8,k);
			}
		}
    	 // final burst logic
		data_axi tb_temp = inData.read();
		datau152b tb = tb_temp.data.range(127,n);
		tb = tb<<24;
		data_axi out_temp = tb_temp;

		datau8b last_bits = 128-n+24;
		datau3b last_chunk = (last_bits+31)/32;

		datau32b crc_25_back = crc_temp;
		crc_temp = 0;
		int extra = last_bits-((last_chunk-1)*32);
		for(int b=0;b<5;b++){
			#pragma HLS LOOP_TRIPCOUNT min=1 max=5
			#pragma HLS PIPELINE
			if(b==last_chunk)break;
			int ind;
			if(b==0){
				int bytes = (extra+7)/8;
				int extra_extra = extra - (bytes-1)*8;
				for(int a=0;a<4;a++){
			#pragma HLS LOOP_TRIPCOUNT min=1 max=4
			#pragma HLS PIPELINE
					if(a==bytes)break;
					datau8b k;
					if(a==0){
						k = tb.range(extra_extra-1,0);
						crc_temp ^= lookup(0,0,k);
					}else{
						k = tb.range(extra_extra+8*(a-1)+7,8*(a-1)+extra_extra);
						crc_temp ^= lookup(0,extra_extra+(a-1)*8,k);
					}
				}
			}else{
				for(int a=0;a<4;a++){
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min=4 max=4
					int zeroes = extra+(b-1)*32+a*8;
					int bytegroup = zeroes/32;
					int rem_byte = zeroes-(bytegroup*32);
					datau8b k = tb.range(8*a+(b-1)*32+extra+7,8*a+(b-1)*32+extra);
					crc_temp ^= lookup(bytegroup,rem_byte,k);
				}
			}

		}
		for(int b=0;b<4;b++){
		#pragma HLS LOOP_TRIPCOUNT min=4 max=4
		#pragma HLS PIPELINE
			int ind = b*8;
			datau8b k = crc_25_back.range(ind+7,ind);
			int zeroes = 8*b+last_bits;
			int bytegroup = zeroes/32;
			int rem_z = zeroes-(bytegroup*32);
			crc_temp ^= lookup(bytegroup,rem_z,k);
		}
		datau25b crc = crc_temp;
		crc[L] = 0;
		if(n == 0){
			output.last = 0;
			output.data = out_temp.data;
			outData.write(output);
			output.last = (filler+L<=128) && tlast;
			filler -= (128-L);
			datau128b crc_temp = crc.read().range(L-1,0);
			output.data = crc_temp << (128-L);
			outData.write(output);
		}else{
			if(n>=L){
				datau128b new_crc_temp = out_temp.data;
				output.last = (filler+L<=n) && tlast;
				filler -= (n-L);
				datau128b crc_temp = crc;
				output.data = new_crc_temp + (crc_temp << (n-L));
				outData.write(output);
			}else{
				datau128b new_crc_temp = crc.read().range(L-1,L-n);
				output.last = 0;
				output.data = new_crc_temp + out_temp.data;
				outData.write(output);
				output.last = (filler+L-n<=128) && tlast;
				datau128b crc_temp = crc.read().range(L-n-1,0);
				output.data = crc_temp << (128-L+n);
				filler -= (128-(L-n));
				outData.write(output);
			}
		}

		int filler_bursts = (filler+127)/128;
		for(int i=1;i<=25;i++){
#pragma HLS LOOP_TRIPCOUNT min=0 max=25
#pragma HLS PIPELINE
			if(i==filler_bursts+1)break;
			output.data = 0;
			output.last = (i == filler_bursts) && tlast;
			outData.write(output);
		}
    }
    }else{
    	for(int j=0;j<66;j++){
#pragma HLS LOOP_TRIPCOUNT min=0 max=66
#pragma HLS PIPELINE
    		if(j==burst-1)break;
    		input = inData.read();
    		datau128b temp = input.data;
    		output.last = 0;
    		output.data = temp;
    		outData.write(output);
    	}
    	input = inData.read();
    	int n = 128*burst-cbz;
    	output.data = (input.data);
    	output.last = (filler<=n);
    	filler -= (128-n);
    	outData.write(output);
    	int filler_bursts = (filler+127)/128;
    	for(int i=1;i<=25;i++){
#pragma HLS LOOP_TRIPCOUNT min=0 max=25
#pragma HLS PIPELINE
    		if(i==filler_bursts+1)break;
    		output.data = 0;
    		output.last = i == filler_bursts;
    		outData.write(output);
    	}
    }

}
