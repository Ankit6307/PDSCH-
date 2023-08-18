#include "header.h"
#include <bitset>

datau25b crc_poly;

datau17b get_crc_17(datau128b tb_temp, datau17b crc_temp){
    for(datau8b i=0;i<128;i++){
		#pragma HLS LOOP_TRIPCOUNT min=127 max=127
#pragma HLS PIPELINE
		crc_temp *= 2;
		crc_temp[0] = tb_temp[127-i];
		if(crc_temp[16] == 1){
			crc_temp = crc_temp ^ crc_poly;
		}
    }
    return crc_temp;
}
//
//datau25b get_crc_25(datau128b tb_temp, datau25b crc_temp, int L){
//	for(int i=0;i<128;i++){
//		#pragma HLS LOOP_TRIPCOUNT min=0 max=127
//		crc_temp *= 2;
//		crc_temp[0] = tb_temp[127-i];
//		if(crc_temp[L] == 1){
//			crc_temp = crc_temp ^ crc_poly;
//		}
//	}
//	return crc_temp;
//}

void tb_crc(STREAM_T &inData, cn_STREAM &cnData,STREAM_T &outData)
{
    #pragma HLS INTERFACE axis port=inData
    #pragma HLS INTERFACE axis port=outData
	#pragma HLS INTERFACE axis port=cnData
	#pragma HLS INTERFACE ap_ctrl_none port=return

//  int temp1 = 65535;  // use this value for AUG-CCITT Algorithm
    int temp1 = 0;	// using this value for CCITT Algorithm

    datau19b TBS = cnData.read().range(18,0);
    int burst = (TBS+127)/128; // ceiling function
    datau5b L;
    if(TBS>3824){
    	L = 24;
    }else{
    	L=16;
    }

    if(L==16){
    	crc_poly = 69665;
    }else if(L==24){
    	crc_poly = 25578747;
    }

    data_axi output;
    datau17b crc_temp_17 = temp1;
    datau25b crc_temp_25 = temp1;

    if(L==16){
        for(datau6b j=0;j<32;j++){
    		#pragma HLS LOOP_TRIPCOUNT min=0 max=32
#pragma HLS PIPELINE
        	if(j==burst-1)break;
            data_axi input = inData.read();
            datau128b tb_temp = input.data;
            output.last = 0;
            output.data = tb_temp;
            outData.write(output);
            crc_temp_17 = get_crc_17(tb_temp,crc_temp_17);
        }
    }else{
        for(int j=0;j<10000;j++){
    		#pragma HLS LOOP_TRIPCOUNT min=0 max=10000
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
					crc ^= lookup(a,b,k);
				}
			}
			datau32b crc_25_back = crc_temp_25;
			crc_temp_25 = crc;
			for(int b=0;b<4;b++){
#pragma HLS LOOP_TRIPCOUNT min=4 max=4
#pragma HLS PIPELINE
				int ind = b*8;
				int k = crc_25_back.range(ind+7,ind);
				crc_temp_25 ^= lookup(4,b,k);
			}
		}
    }

    datau25b crc_temp;
    if(L==16){
    	crc_temp = crc_temp_17;
    }else if(L==24){
    	crc_temp = crc_temp_25;
    }
    // final burst logic
    int n = 128 * burst - TBS;
    data_axi tb_temp = inData.read();
    data_axi out_temp = tb_temp;
    for(int j=127-n; j>=0;j--){
		#pragma HLS LOOP_TRIPCOUNT min=0 max=127
    	crc_temp = 2 * crc_temp;
        crc_temp[0] = tb_temp.data[j];
        if(crc_temp[L] == 1){
            crc_temp = (crc_temp ^ crc_poly) & ((1<<L)-1);
        }
    }
    for(datau5b j=0;j<L;j++){
		#pragma HLS LOOP_TRIPCOUNT min=0 max=23
        crc_temp = 2 * crc_temp;
        if(crc_temp[L] == 1){
            crc_temp = (crc_temp ^ crc_poly) & ((1<<L)-1);
        }
    }
    datau25b crc = crc_temp;
    crc[L] = 0;
    if(n == 0){
        output.last = 0;
        output.data = out_temp.data;
        outData.write(output);
        output.last = 1;
        output.data = crc.read().range(L-1,0);
        outData.write(output);
    }else{
        if(n>=L){
            datau128b new_crc_temp = (out_temp.data<<L);
            output.last = 1;
            output.data = new_crc_temp + crc.read().range(L-1,0);
            outData.write(output);
        }else{
            datau128b new_crc_temp = crc.read().range(L-1,L-n);
            output.last = 0;
            output.data = new_crc_temp + (out_temp.data<<n);
            outData.write(output);
            output.last = 1;
            output.data = crc.read().range(L-n-1,0);
            outData.write(output);
        }
    }
}
