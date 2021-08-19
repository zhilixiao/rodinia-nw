//#include "nw_common.h"
#include <stdio.h>
#include <iostream>

int maximum(int a, int b, int c)
{
	#pragma HLS INLINE
	int k;
	if(a <= b)
		k = b;
	else
		k = a;

	if(k <= c)
		return(c);
	else
		return(k);
}

extern "C" {
	void nw_dynproc_kernel(int* 	reference, 
                    	int*  	input_itemsets,
                        int     dim,
                        int     penalty)

	{

		//bundle defines the name of the port, 
		//using the same bundle saves resources but limits performance
		#pragma HLS INTERFACE m_axi port=reference bundle=gmem
		//#pragma HLS INTERFACE m_axi port=input_itemsets bundle=gmem1
		#pragma HLS INTERFACE m_axi port=input_itemsets bundle=gmem
		
		
		#pragma HLS INTERFACE s_axilite port=dim bundle=control
		#pragma HLS INTERFACE s_axilite port=penalty bundle=control
		//#pragma HLS INTERFACE ap_ctrl_hs port=return bundle=control

		for (int j = 1; j < dim - 1; ++j)
		{
			for (int i = 1; i < dim - 1; ++i)
			{
				//std::cout << i << "," << j << std::endl;
				int index = j * dim + i;
				// int k;
				// int a = input_itemsets[index - 1 - dim] + reference[index];
				// int b = input_itemsets[index - 1] - penalty;
				// int c = input_itemsets[index - dim] - penalty;

				// k =  (a <= b) ? b : a;
				// input_itemsets[index] = (k <= c) ? c : k;

				input_itemsets[index] = maximum(input_itemsets[index - 1 - dim] + reference[index],
							input_itemsets[index - 1] - penalty,
							input_itemsets[index - dim] - penalty);

				//input_itemsets[index] = r;
			}
		}
	}
}


