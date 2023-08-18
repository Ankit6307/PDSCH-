#include "header.h"
#include <bitset>

void LDPC_Config(STREAM_T &config, STREAM_U &ctrl)
{
#pragma HLS interface ap_ctrl_none port=return
#pragma HLS INTERFACE axis port = config
#pragma HLS INTERFACE axis port = ctrl


        static datau128b cnTemp;// define it as static and axi stream
        cnTemp = config.read().data;
        datau40b bg, j;
        datau40b ils;
        datau40b mb;
        datau40b zj;
       //datau40b kb=cnTemp.range(127, 124);
	static data_axi_40 ctrlTemp;//axi stream, define it as static
	datau9b Zc = cnTemp.range(68, 60);
        bg=cnTemp.range(19,19);
         switch(bg)
        {
          case 0:mb=46;
          	  	  break;
          case 1:mb=42;
          	  	  break;
        }


        // switch(kb)
       // {
          //case 0:ctrlTemp.data.range(8,6)=000; //base graph 1
          	  	//  break;
         // case 10:ctrlTemp.data.range(8,6)=001; //base graph 2,kb=10
          	 	//  break;
         // case 9: ctrlTemp.data.range(8,6)=010; //base graph 2,kb=9
          	 	//  break;
        // case 8:ctrlTemp.data.range(8,6)=011; //base graph 2,kb=8
          	 	//  break;
        // case 6:ctrlTemp.data.range(8,6)=100; //base graph 2,kb=6
          	  	// break;
       // }
switch(Zc)
{
  case 2:ils=0;
         zj=0;
          break;
  case 4:ils=0;
         zj=1;
          break;
  case 8:ils=0;
         zj=2;
          break;
  case 16:ils=0;
         zj=3;
          break;
  case 32:ils=0;
         zj=4;
          break;
  case 64:ils=0;
         zj=5;
          break;
  case 128:ils=0;
         zj=6;
          break;
  case 256:ils=0;
         zj=7;
          break;

  case 3:ils=1;
         zj=0;
          break;
  case 6:ils=1;
         zj=1;
          break;
  case 12:ils=1;
         zj=2;
          break;
  case 24:ils=1;
         zj=3;
          break;
  case 48:ils=1;
         zj=4;
          break;
  case 96:ils=1;
         zj=5;
          break;
  case 192:ils=1;
          zj=6;
          break;
  case 384:ils=1;
           zj=7;
           break;
  case 5: ils=2;
          zj=0;
          break;
  case 10: ils=2;
          zj=1;
          break;
  case 20:ils=2;
          zj=2;
          break;
  case 40:ils=2;
          zj=3;
          break;
  case 80:ils=2;
          zj=4;
          break;
  case 160:ils=2;
          zj=5;
          break;
  case 320:ils=2;
          zj=6;
          break;
  case 7:ils=3;
         zj=0;
         break;
   case 14:ils=3;
         zj=1;
         break;
  case 28:ils=3;
         zj=2;
         break;
   case 56:ils=3;
         zj=3;
         break;
   case 112:ils=3;
         zj=4;
         break;
  case 224:ils=3;
         zj=5;
         break;
  case 9:ils=4;
         zj=0;
         break;
    case 18:ils=4;
         zj=1;
         break;
    case 36:ils=4;
         zj=2;
         break;
    case 72:ils=4;
         zj=3;
         break;
    case 144:ils=4;
         zj=4;
         break;
     case 288:ils=4;
         zj=5;
         break;
   case 11:ils=5;
         zj=0;
         break;
case 22:ils=5;
         zj=1;
         break;
case 44:ils=5;
         zj=2;
         break;
case 88:ils=5;
         zj=3;
         break;
case 176:ils=5;
         zj=4;
         break;
case 352:ils=5;
         zj=5;
         break;
case 13:ils=6;
         zj=0;
         break;
case 26:ils=6;
         zj=1;
         break;
case 52:ils=6;
         zj=2;
         break;
case 104:ils=6;
         zj=3;
         break;
case 208:ils=6;
         zj=4;
         break;
case 15:ils=7;
         zj=0;
         break;
case 30:ils=7;
         zj=1;
         break;
case 60:ils=7;
         zj=2;
         break;
case 120:ils=7;
         zj=3;
         break;
case 240:ils=7;
         zj=4;
         break;
}

ctrlTemp.data.range(2, 0) = zj;
ctrlTemp.data.range(5, 3) = ils;
ctrlTemp.data.range(8, 6) = bg;
ctrlTemp.data.range(31, 24) = 32;
ctrlTemp.data.range(37, 32) = mb;
ctrlTemp.data.range(39, 38) = 0;
ctrlTemp.last=1;
ctrl.write(ctrlTemp);

}
