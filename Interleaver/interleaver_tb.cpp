#include "header.h"
int main()
{
	in_STREAM inDataFIFO;
	cn_STREAM cnDataFIFO;
	out_STREAM outDataFIFO;
	datau128b PHY_MAC_input;
	input_data_axi inData;
	output_data_axi outData;

	const int TEST_CASES = 20;

	// Enter the configuration details as per the test case:

	int config_params[11][13] = {
		{0, 0, 0, 2, 0, 1, 2, 0, 1, 0, 0, 3, 2},
		{2, 1, 1, 1, 1, 2, 2, 5, 12, 1, 1, 1, 1},
		{8968, 24, 848, 7936, 32, 11272, 9992, 40976, 94248, 160, 152, 224, 160},
		{0, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1},
		{208, 7, 88, 384, 8, 288, 240, 384, 384, 30, 28, 30, 30},
		{1, 0, 0, 1, 0, 1, 1, 2, 3, 0, 0, 0, 0},
		{0, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
		{4520, 40, 864, 7960, 48, 5672, 5032, 8224, 7880, 176, 168, 240, 176},
		{16800, 40, 7168, 13120, 1360, 19680, 16400, 54000, 120000, 672, 1040, 1856, 1392},
		{2, 1, 1, 1, 1, 2, 2, 5, 12, 1, 1, 1, 1},
		{53, 1, 10, 90, 1, 62, 58, 96, 89, 1, 2, 3, 1},
	};

	for(int tc = 1; tc <= 13; tc++){
		if(tc == 9){
			continue;
		}
		int RV = config_params[0][tc-1];
		int C = config_params[1][tc-1];
		int TBS = config_params[2][tc-1];
		int Bg_no = config_params[3][tc-1];
		int Z_c = config_params[4][tc-1];
		int Q_m = config_params[5][tc-1];
		int V = config_params[6][tc-1];
		int K_ = config_params[7][tc-1];
		int G = config_params[8][tc-1];
		int Cr = config_params[9][tc-1];

		string s1 = "test_case_"+to_string(tc)+"_data_filler_out.txt";
		string s2 = "test_case_"+to_string(tc)+"_Tkeep_out.txt";
		string s3 = "test_case_"+to_string(tc)+"_Tlast_out.txt";

		ifstream data_input_file(s1);
		ifstream T_keep_file(s2);
		ifstream T_last_file(s3);

		string cmd = "diff -w output.txt test_case_"+to_string(tc)+"_data_Interleaver_out.txt";

		int L;
			if (TBS > 3824)
			{
				L = 24;
			}
			else
			{
				L = 16;
			}

			int K = 10 * Z_c;
			int N = 50 * Z_c;

			if (Bg_no == 0)
			{
				K = 22 * Z_c;
				N = 66 * Z_c;
			}

			int Nref = (TBS * 3) / (2 * C);
			int Ncb = N;
			if (Nref < N)
			{
				Ncb = Nref;
			}
			int Qm = 2 * (Q_m + 1);
			int E = ((V + 1) * Qm) * (G / ((V + 1) * Qm * C));
			int F = K - K_;

			PHY_MAC_input.range(18, 0) = TBS;
			PHY_MAC_input.range(29, 28) = RV;
			PHY_MAC_input.range(21, 20) = Q_m;
			PHY_MAC_input.range(32, 30) = V;
			PHY_MAC_input.range(27, 22) = C;
			PHY_MAC_input.range(107, 86) = G;
			PHY_MAC_input.range(85, 72) = K_;
			PHY_MAC_input.range(19, 19) = Bg_no;
			PHY_MAC_input.range(68, 60) = Z_c;
			PHY_MAC_input.range(123, 118) = Cr;
			// cout << PHY_MAC_input << "\n";
			// cnDataFIFO.write(PHY_MAC_input);

			ap_uint<128> fileTemp;
			ap_uint<7> t_keep;
			ap_uint<1> t_last;
			int count = TEST_CASES;
			while (count > 0)
			{
				int c = C;
				while (c > 0)
				{
					int left = config_params[10][tc-1];
					//cout << left << "\n";
					while (left > 0)
					{
						char buffer[32];
						// Input read
						data_input_file.read(buffer, 32);
						string s = "0x";
						for (int j = 0; j < 32; j++)
						{
							s += buffer[j];
						}
						//cout <<"left"<<left<<" Data: " << s << "\n";

						istringstream iss(s);
						iss >> hex >> fileTemp;

						char s2[7];
						T_keep_file.read(s2, 7);
						t_keep = 0;
						for (int j = 0; j < 7; j++)
						{
							if (s2[j] == '1')
							{
								t_keep += (1 << (6 - j));
							}
						}

						char s3[1];
						T_last_file.read(s3, 1);
						if (s3[0] == '1')
						{
							t_last = 1;
						}
						else
						{
							t_last = 0;
						}
						// cout << "Data: " << fileTemp << " " << t_keep << " " << t_last << "\n";

						inData.data = fileTemp;
						inData.tkeep = t_keep;
						inData.tlast = t_last;
						inDataFIFO.write(inData);
						left--;
					}
					char trailing[1];
					data_input_file.read(trailing, 1);
					T_keep_file.read(trailing, 1);
					T_last_file.read(trailing, 1);
					c--;
				}
				count--;
			}
			data_input_file.close();
			T_keep_file.close();
			T_last_file.close();

			while (!inDataFIFO.empty())
			{
				cnDataFIFO.write(PHY_MAC_input);
				Interleaver(inDataFIFO, cnDataFIFO, outDataFIFO);
			}

			// cout << outDataFIFO.empty() << "\n";

			ofstream output_file("output.txt");
			count = TEST_CASES;

			while (!outDataFIFO.empty())
			{
				while (count > 0)
				{
					int c = C;
					while (c > 0)
					{
						int left = E;
						// cout << E << "\n";
						while (left > 0)
						{
							outDataFIFO >> outData;
							datau96b outTemp = outData.data.read();
							datau1b tl = outData.tlast.read();
							string s = "";
							int mn = left;
							if (mn >= 96)
							{
								mn = 96;
								int streams = (mn + 31) / 32;
								int trailing = mn - 32 * (streams - 1);

								for (int j = 0; j < streams; j++)
								{
									datau32b readTemp;
									stringstream ss;
									if (j == streams - 1)
									{
										readTemp = outTemp.read().range(95 - 32 * j, 95 - 32 * j - trailing + 1);
										unsigned int u = readTemp.read();
										ss << std::setfill('0') << std::setw(trailing / 4) << uppercase << hex << u;
									}
									else
									{
										readTemp = outTemp.read().range(95 - 32 * j, 64 - 32 * j);
										unsigned int u = readTemp.read();
										ss << std::setfill('0') << std::setw(8) << uppercase << hex << u;
									}
									// cout << j  <<": "<< ss.str() << "\n";
									s += ss.str();
								}
							}
							else{
								datau32b readTemp;
								stringstream ss;
								unsigned int u;
								if(mn <= 32){
									readTemp = outTemp.read().range(mn-1, 0);
									u = readTemp.read();
									ss.str("");
									ss << std::setfill('0') << std::setw(mn / 4) << uppercase << hex << u;
									s = ss.str() + s;
								}
								else{
									readTemp = outTemp.read().range(31, 0);
									u = readTemp.read();
									ss.str("");
									ss << std::setfill('0') << std::setw(8) << uppercase << hex << u;
									s = ss.str() + s;
									if(mn <= 64){
										readTemp = outTemp.read().range(mn-1, 32);
										unsigned int u = readTemp.read();
										ss.str("");
										ss << std::setfill('0') << std::setw((mn-32)/4) << uppercase << hex << u;
										s = ss.str() + s;
									}
									else{
										readTemp = outTemp.read().range(63, 32);
										u = readTemp.read();
										ss.str("");
										ss << std::setfill('0') << std::setw(8) << uppercase << hex << u;
										s = ss.str() + s;

										readTemp = outTemp.read().range(mn-1, 64);
										u = readTemp.read();
										ss.str("");
										ss << std::setfill('0') << std::setw((mn-64)/4) << uppercase << hex << u;
										s = ss.str() + s;
									}
								}
							}
							//cout << s;
							output_file << s;
							left -= 96;
						}
						//cout << "\n";
						output_file << endl;
						c--;
					}
					count--;
				}
			}
			output_file.close();
			cout<<"\nComparing with MATLAB results....\n";
			if (system(cmd.c_str()))
			{
				printf("Test failed !!!\n");
				return 1;
			}
			else
			{
				cout << "Test Case: " << tc << " Passed !!!\n";
			}
	}
	return 0;
}
