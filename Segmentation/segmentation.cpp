#include "header.h"
#include <bitset>

void segmentation(STREAM_T &inData, cn_STREAM &cnData,STREAM_T &outData)
{
    #pragma HLS INTERFACE axis port=inData
    #pragma HLS INTERFACE axis port=outData
	#pragma HLS INTERFACE axis port=cnData
	#pragma HLS INTERFACE ap_ctrl_none port=return

	datau128b cnTemp = cnData.read();
    datau19b TBS = cnTemp.range(18,0);
    datau6b C = cnTemp.range(27,22);
    datau14b K_ = cnTemp.range(85,72);
    const int L = 24;
    int cbz = K_;
    if(C>1){
    	cbz-=L;
    }
    int L_ = 16;
    if(TBS>3824){
    	L_ = 24;
    }
    int TBS_ = TBS + L_;
    data_axi output, tb_temp, out_temp;
    datau128b x,y;
    int burst = (cbz+127)/128;
    int cursor = 0;

    int read = 1;
    int totalReads = (TBS_+127)/128;
    int N = totalReads*128 - TBS_;
    tb_temp = inData.read();
    if(read == totalReads){
    	tb_temp.data = tb_temp.data << (N);
    }

    for(int i=1;i<=C;i++){
#pragma HLS PIPELINE
    	datau128b writeout;
    	for(int j=1;j<=burst-1;j++){
#pragma HLS PIPELINE
    		tb_temp.data = tb_temp.data.range(127-cursor,0);
    		writeout = (tb_temp.data << cursor);
    		read++;
			tb_temp = inData.read();
			if(read == totalReads){
				tb_temp.data = tb_temp.data << (N);
			}
			if(cursor>0){
				 writeout += tb_temp.data.range(127,127-cursor+1);
			}
			output.data = writeout;
			output.last = 0;
			outData.write(output);
    	}
    	cbz = cbz%128;
    	if(128-cursor > cbz){
    		writeout = tb_temp.data.range(127-cursor,127-cursor-cbz+1) << (128-cbz);
    		cursor += cbz;
    		cursor %= 128;
    	}else{
    		int l1 = 128-cursor;
    		int l2 = cbz-l1;
    		writeout = tb_temp.data.range(127-cursor,0) << cursor;
    		read++;
			tb_temp = inData.read();
			if(read == totalReads){
				tb_temp.data = tb_temp.data << (N);
			}
			if(l2>0){
				writeout += (tb_temp.data.range(127,127-l2+1) << (128-cbz));
			}
			cursor = l2;
    	}
    	output.data = writeout;
    	output.last = 0;
    	if(i==C){
    		output.last = 1;
    	}
    	outData.write(output);
    }
}
