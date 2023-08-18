#include "header.h"
#include <bitset>

void Interleaver(in_STREAM &inData, cn_STREAM &cnData, out_STREAM &outData)
{
#pragma HLS INTERFACE axis port = inData
#pragma HLS INTERFACE axis port = outData
#pragma HLS INTERFACE axis port = cnData
#pragma HLS INTERFACE ap_ctrl_none port=return

    datau128b cnTemp = cnData.read();
    datau2b RV = cnTemp.range(29, 28);
    datau19b TBS = cnTemp.range(18, 0);
    datau6b C = cnTemp.range(27, 22);
    datau14b K_ = cnTemp.range(85, 72);
    datau1b Bg_no = cnTemp.range(19, 19);
    datau9b Z_c = cnTemp.range(68, 60);
    datau2b Q_m = cnTemp.range(21, 20);
    datau3b V = cnTemp.range(32, 30);
    datau22b G = cnTemp.range(107, 86);
    datau6b Cr = cnTemp.range(123, 118);

    int Qm;
    datau18b Nref;
    datau15b N;
    datau14b K;
    datau22b E;
    datau10b F;

    if (Bg_no == 0)
    {
        N = 66 * Z_c;
        K = 22 * Z_c;
    }
    else
    {
        N = 50 * Z_c;
        K = 10 * Z_c;
    }

    Nref = (TBS * 3) / (2 * C);
    datau15b Ncb = N;
    if (Nref < N)
    {
        Ncb = Nref;
    }
    Qm = 2 * (Q_m + 1);
    E = ((V + 1) * Qm) * (G / ((V + 1) * Qm * C));
    F = K - K_;

    // datau1b arr[25000], arr2[25000];
    /* New algo */
    datau128b arr[200], arr1[200];
    for (int i = 0; i < 200; i++)
    {
#pragma HLS LOOP_TRIPCOUNT min = 200 max = 200
#pragma HLS PIPELINE
        arr[i] = 0;
        arr1[i] = 0;
    }

    int i, j, k;
    datau128b in_Temp;
    datau96b out_temp;
    datau7b t_keep;
    datau1b t_last;
    datau1b temp;
    int m, it;
    int index;
    int cb, row, column;
    output_data_axi output;

    int k0;
    if (Bg_no == 0)
    {
        switch (RV)
        {
        case 0:
            k0 = 0;
            break;
        case 1:
            k0 = (17 * Ncb / N) * Z_c;
            break;
        case 2:
            k0 = (33 * Ncb / N) * Z_c;
            break;
        default:
            k0 = (56 * Ncb / N) * Z_c;
            break;
        }
    }
    else
    {
        switch (RV)
        {
        case 0:
            k0 = 0;
            break;
        case 1:
            k0 = (13 * Ncb / N) * Z_c;
            break;
        case 2:
            k0 = (25 * Ncb / N) * Z_c;
            break;
        default:
            k0 = (43 * Ncb / N) * Z_c;
            break;
        }
    }

    if (k0 > (K - 2 * Z_c))
    {
        k0 = k0 - F;
    }
    else if (k0 > (K_ - 2 * Z_c) && k0 <= (K - 2 * Z_c) && Ncb > (K_ - 2 * Z_c) && Ncb <= (K - 2 * Z_c))
    {
        k0 = 0;
    }
    else if (k0 > (K_ - 2 * Z_c) && k0 <= (K - 2 * Z_c))
    {
        k0 = K_ - 2 * Z_c;
    }
    datau7b t_kd;
    input_data_axi data_read;
    int j2;
    int keep_total;
    for (cb = 0; cb < C; cb++)
    {
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min = 1 max = 152
        j = 0, j2 = 0;
        keep_total = 0;
        do
        {
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min = 1 max = 96
            data_read = inData.read();
            in_Temp = data_read.data;
            t_keep = data_read.tkeep;
            t_last = data_read.tlast;
            keep_total += (t_keep + 1);
            if (cb > Cr - 1)
            {
                E = E + (V + 1) * Qm;
            }
            datau128b temp = in_Temp.range(t_keep, 0);
            if (j2 == 128)
            {
                j++;
                j2 = 0;
            }
            if (j2 + t_keep > 127)
            {
                int jk = (j2 + t_keep) - 127;
                arr[j] += temp >> jk;
                arr1[j] = arr[j];
                j++;
                arr[j] += temp << (128 - jk);
                arr1[j] = arr[j];
                j2 = jk;
            }
            else
            {
                j2 += (t_keep + 1);
                arr[j] += temp << (128 - j2);
                arr1[j] = arr[j];
            }
        } while (t_last != 1);
        int temp = E / Qm;
        k = 0;
        int i1, j1, ind, ind1, ind2;
        output.tlast = 0;
        for (int i = 0; i < E; i += 4)
        {
#pragma HLS LOOP_TRIPCOUNT min = 1 max = 1250
#pragma HLS PIPELINE
            if (k >= 96)
            {
                k = 0;
                output.data = out_temp;
                outData.write(output);
                out_temp = 0;
            }
            i1 = i % Qm;
            j1 = i / Qm;
            ind = j1 + i1 * temp;
            ind = (ind + k0) % keep_total;
            ind1 = ind / 128;
            ind2 = 127 - (ind % 128);

            out_temp[95 - k] = arr[ind1][ind2];
            i1 = (i + 1) % Qm;
            j1 = (i + 1) / Qm;
            ind = j1 + i1 * temp;
            ind = (ind + k0) % keep_total;
            ind1 = ind / 128;
            ind2 = 127 - (ind % 128);

            out_temp[95 - k - 1] = arr1[ind1][ind2];
            i1 = (i + 2) % Qm;
            j1 = (i + 2) / Qm;
            ind = j1 + i1 * temp;
            ind = (ind + k0) % keep_total;
            ind1 = ind / 128;
            ind2 = 127 - (ind % 128);

            out_temp[95 - k - 2] = arr[ind1][ind2];
            i1 = (i + 3) % Qm;
            j1 = (i + 3) / Qm;
            ind = j1 + i1 * temp;
            ind = (ind + k0) % keep_total;
            ind1 = ind / 128;
            ind2 = 127 - (ind % 128);

            out_temp[95 - k - 3] = arr1[ind1][ind2];
            k += 4;
        }
        if(cb == C-1){
        	output.tlast = 1;
        }

        out_temp = out_temp >> ((96 - (E%96))%96);
        output.data = out_temp;
        outData.write(output);
        for (int i = 0; i < 200; i++)
        {
        #pragma HLS LOOP_TRIPCOUNT min = 200 max = 200
        #pragma HLS PIPELINE
        	arr[i] = 0;
        	arr1[i] = 0;
        }
    }
}
