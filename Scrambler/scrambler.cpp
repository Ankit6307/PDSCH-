#include "header.h"

void scrambler(STREAM_T &inData, cn_STREAM &cnData, STREAM_T &goldenData, STREAM_T &outData)
{
#pragma HLS interface ap_ctrl_none port=return
#pragma HLS INTERFACE axis port=inData
#pragma HLS INTERFACE axis port=cnData
#pragma HLS INTERFACE axis port=goldenData
#pragma HLS INTERFACE axis port=outData

	datau64b cnTemp = cnData.read();
	datau11b E = cnTemp.range(29,19);

    datau5b burst = E/108;
    data_axi output;
    for(datau5b j=0;j<burst;j++){
		#pragma HLS PIPELINE
		#pragma HLS LOOP_TRIPCOUNT min=1 max=16
        data_axi input = inData.read();
        datau108b a = input.data;
        input = goldenData.read();
        datau108b b = input.data;
        output.last = 0;
        output.data = a^b;
        if(j == burst-1){
        	output.last = 1;
        }
        outData.write(output);
    }
}
