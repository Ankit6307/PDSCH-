#include "header.h"
#include <bitset>

int crc_poly;

datau25b get_crc(datau128b tb_temp, datau25b crc_temp, int L){
    for(int i=0;i<128;i++){
		crc_temp *= 2;
		crc_temp[0] = tb_temp[127-i];
		if(crc_temp[L] == 1){
			crc_temp = crc_temp ^ crc_poly;
		}
    }
    return crc_temp;
}


void CB_CRC(STREAM_T &inData, cn_STREAM &cnData,STREAM_T &outData)
{
    #pragma HLS INTERFACE axis port=inData
    #pragma HLS INTERFACE axis port=outData

	cout<<"entering function"<<endl;

	datau128b cnTemp = cnData.read();
	datau19b TBS = cnTemp.range(18,0);
	datau6b C = cnTemp.range(27,22);
	datau14b K_ = cnTemp.range(85,72);
	datau1b Bg_no = cnTemp.range(19,19);
	datau9b Z_c = cnTemp.range(68,60);

	int temp1 = 0;
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
    int filler = K-K_;
    int filler_bursts = filler/128;
    bool premature_output_last = filler_bursts == 0;
    if(C>1){
    for(int block=0;block<C;block++){
    	cout<<"starting process"<<endl;
    	datau25b crc_temp = temp1;
    	cout<<"burst";
    	for(int j=0;j<burst-1;j++){
    		cout<<"reading";
			data_axi input = inData.read();
			cout<<"data read\n";
			datau128b tb_temp = input.data;
			output.last = 0;
			output.data = tb_temp;
			outData.write(output);
			crc_temp = get_crc(tb_temp,crc_temp,L);
		}

    	 // final burst logic
    	cout<<"coming to final burst";
		int n = 128 * burst - cbz;
		data_axi tb_temp = inData.read();
		data_axi out_temp = tb_temp;
		for(int j=127; j>=n;j--){
			crc_temp = 2 * crc_temp;
			crc_temp[0] = tb_temp.data[j];
			if(crc_temp[L] == 1){
				crc_temp = crc_temp ^ crc_poly;
			}
		}
		for(int j=0;j<L;j++){
			crc_temp = 2 * crc_temp;
			if(crc_temp[L] == 1){
				crc_temp = crc_temp ^ crc_poly;
			}
		}
		datau25b crc = crc_temp;
		crc[L] = 0;
		cout<<"evaluation done \n";
		if(n == 0){
			output.last = 0;
			output.data = out_temp.data;
			outData.write(output);
			output.last = premature_output_last;
			datau128b crc_temp = crc.read().range(L-1,0);
			output.data = crc_temp << (128-L);
			outData.write(output);
		}else{
			if(n>=L){
				datau128b new_crc_temp = out_temp.data;
				output.last = premature_output_last;
				datau128b crc_temp = crc;
				output.data = new_crc_temp + (crc_temp << (n-L));
				outData.write(output);
			}else{
				datau128b new_crc_temp = crc.read().range(n-1,0);
				output.last = 0;
				output.data = new_crc_temp + out_temp.data;
				outData.write(output);
				output.last = premature_output_last;
				datau128b crc_temp = crc.read().range(L-1,n);
				output.data = crc_temp << (128-L+n);
				outData.write(output);
			}
		}
		cout<<"writing filler bits\n";

		for(int i=1;i<=filler_bursts;i++){
			output.data = 0;
			output.last = i == filler_bursts;
			outData.write(output);
		}
    }
    }else{
    	for(int j=0;j<burst-1;j++){
    		input = inData.read();
    		datau128b temp = input.data;
    		output.last = 0;
    		output.data = temp;
    		outData.write(output);
    	}
    	input = inData.read();
    	output.data = (input.data);
    	output.last = premature_output_last;
    	outData.write(output);
    	for(int i=1;i<=filler_bursts;i++){
    		output.data = 0;
    		output.last = i == filler_bursts;
    		outData.write(output);
    	}
    }

}
