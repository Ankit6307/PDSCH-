#include "header.h"

int v4;
void read_buffer(in_STREAM &inData, int times){
	v4 = max(v4,times);
	for(int i=0;i<110;i++){
#pragma HLS LOOP_TRIPCOUNT min=1 max=110
#pragma HLS PIPELINE
		if(i==times)break;
		inData.read();
	}
}

void filler_bit_removal(in_STREAM &inData, cn_STREAM &cnData, out_STREAM &outData)
{
    #pragma HLS INTERFACE axis port=inData
    #pragma HLS INTERFACE axis port=outData
	#pragma HLS INTERFACE axis port=cnData
	#pragma HLS INTERFACE ap_ctrl_none port=return

	datau128b cnTemp = cnData.read();
	datau19b TBS = cnTemp.range(18,0);
	datau6b C = cnTemp.range(27,22);
	datau14b K_ = cnTemp.range(85,72);
	datau1b Bg_no = cnTemp.range(19,19);
	datau9b Z_c = cnTemp.range(68,60);

	datau18b Nref;
	datau15b N;
	datau14b K;

	if(Bg_no == 0){
		N = 66*Z_c;
		K = 22*Z_c;
	}else{
		N = 50*Z_c;
		K = 10*Z_c;
	}

	Nref = (TBS*3)/(2*C);
	datau15b Ncb = N;
	if(Nref<datau18b(N)){
		Ncb = Nref;
	}

	datau7b a1,a2,t1,t2,index,burst,index1;
	datau14b kk = K-2*Z_c;
	datau14b kk_ = K_-2*Z_c;
	a1 = (kk_+128)/128;
	a2 = (kk+127)/128;
	t1 = (128*a1) - (kk_);
	t2 = (128*a2) - (kk);

	if(Ncb > kk_ && Ncb <= kk){
		Ncb = kk_;
	}
	burst = (Ncb+127)/128;
	index = (128*burst - Ncb);


	datau8b b,i;
	datau6b  k;
	b = (N+127)/128;
	index1= (128*b - N);
	datau128b temp,x,y;
	output_data_axi output;
	bool flag1 = Ncb<=kk_;
	bool flag2 = Ncb>kk;
	bool flag3 = Ncb>kk_;
	int v1,v2,v3;
//	cout<<"a1:"<<a1<<" a2:"<<a2<<" index1:"<<index1<<" index:"<<index<<" t1:"<<t1<<" t2:"<<t2<<" burst:"<<burst<<" b:"<<b<<" kk:"<<kk<<" kk_:"<<kk_<<" Ncb:"<<Ncb<<endl;
	for(k=1;k<=C;k++){
	#pragma HLS LOOP_TRIPCOUNT min=1 max=152
		v1 = min(a1,burst);
		v2 = a2-a1;
		v3 = burst-a2;
		for(i=1;i<=60;i++){
#pragma HLS LOOP_TRIPCOUNT min=1 max=60
#pragma HLS PIPELINE
			if(i==min(a1,burst))break;
			output.data = inData.read().data;
			output.tkeep = 127;
			output.tlast = 0;
			outData.write(output);
		}
		if(i==burst){
			output.data = inData.read().data.range(127,index);
			output.tkeep = 127-index;
			output.tlast = 1;
			outData.write(output);
			read_buffer(inData,b-burst);
			continue;
		}
		datau128b temp = inData.read().data;
		if(i==burst && flag1){
			output.data = temp.range(127,index);
			output.tkeep = 127-index;
			output.tlast = 1;
			outData.write(output);
			read_buffer(inData,b-burst);
			continue;
		}else if(i==burst && i==a2 && flag2){
			x = temp.range(127,t1);
			y = temp.range(t2-1,index);
			output.data = (x<<(t2-index))+y;
			output.tkeep = 127-t1+t2-index;
			output.tlast = 1;
			x = y = 0;
			outData.write(output);
			read_buffer(inData,b-burst);
			continue;
		}else if(a1==a2 && t2>0){
			x = temp.range(127,t1);
			y = temp.range(t2-1,0);
			output.data = (x<<(t2))+y;
			output.tkeep = 127-t1+t2;
			output.tlast = 0;
			x = y = 0;
		}else{
			output.data = temp.range(127,t1);
			output.tkeep = 127-t1;
			output.tlast = 0;
		}
		outData.write(output);
		for(int j=1;j<=10;j++){
#pragma HLS LOOP_TRIPCOUNT min=1 max=10
#pragma HLS PIPELINE
			if(j>(a2-a1-1))break;
			inData.read();
		}
		if(!(a1==a2)){
			temp = inData.read().data;
		}
		if(!(a1==a2) && t2==0){
			// do nothing
		}else if(!(a1==a2) && burst==a2 && flag3){
			output.data = temp.range(t2-1,index);
			output.tkeep = t2-index-1;
			output.tlast = 1;
			outData.write(output);
			read_buffer(inData,b-burst);
			continue;
		}else if(!(a1==a2)){
			output.data = temp.range(t2-1,0);
			output.tkeep = t2-1;
			output.tlast = 0;
			outData.write(output);
		}
		for(int j=1;j<=50;j++){
#pragma HLS LOOP_TRIPCOUNT min=1 max=50
#pragma HLS PIPELINE
			if(j>(burst-a2-1))break;
			temp = inData.read().data;
			output.data = temp;
			output.tkeep = 127;
			output.tlast = 0;
			outData.write(output);
		}
		temp = inData.read().data;
		output.data = temp.range(127,index);
		output.tkeep = 127-index;
		output.tlast = 1;
		outData.write(output);
		read_buffer(inData,b-burst);
	}
	//cout<<"v1:"<<v1<<"v2:"<<v2<<"v3:"<<v3<<"v4:"<<v4<<endl;
}


