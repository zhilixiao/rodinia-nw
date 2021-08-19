//using Anthony's v5
#include "nw_common.h"
#include "ap_shift_reg.h"

//#include <iostream>

/*
 * kernel parameters:
 *
 * reference: the reference matrix that holds the cost of moving to a
 * particular sqaure. The dimensions are max_rows * (max_cols - 1)
 * data: the scoring matrix. The dimensionx 
 *
 */

//__attribute__((reqd_work_group_size(1, 1, 1)))
extern "C"{
	void nw_dynproc_kernel(int* reference, 
                         int* 		 data,
                         int* 		 input_v,// vertical input (first column)
                         int         dim,
                         int         penalty,
                         int         loop_exit,
                         int         block_offset)
	{
		#pragma HLS INTERFACE m_axi port=reference  bundle=gmem  max_read_burst_length=64 //latency=100//num_read_outstanding=128
		// from hls we know that data needs to have more ports for access

		#pragma HLS INTERFACE m_axi port=data bundle=gmem1  max_read_burst_length=64  max_write_burst_length=64 //latency=100//num_read_outstanding=128 num_write_outstanding=128
		//#pragma HLS INTERFACE ap_memory port=data
		
		#pragma HLS INTERFACE m_axi port=input_v  bundle=gmem2  max_read_burst_length=64 //num_read_outstanding=128

		#pragma HLS INTERFACE s_axilite port=dim  //bundle=control

		#pragma HLS INTERFACE s_axilite port=penalty //bundle=control

		#pragma HLS INTERFACE s_axilite port=loop_exit //bundle=control

		#pragma HLS INTERFACE s_axilite port=block_offset //bundle=control
		
		// output shift register; 2 registers per parallel comp_col_offset is 
		// required, one for writing, and two for passing data to the following 
		// diagonal lines to handle the dependency

								
		int out_SR[PAR - 1][3];	
		#pragma HLS ARRAY_PARTITION variable=out_SR complete

		// shift register for last comp_col_offset in parallel chunk to pass data 
		// to next chunk, (PAR - 1) cells are always out of bound, one extra cell 
		// is added for writing and one more for top-left
		// It's for the vertical buffer for the next chunk of columns, where chunk
		// is of size BLKSIZE * PAR. It serves as the left and top left dependencies
		// for when elements that fall on the right of a chunk boundary.
		
		//int last_chunk_col_SR[BSIZE - (PAR - 1) + 2] __attribute__((xcl_array_partition(complete, 0)));					
        static ap_shift_reg<int, BSIZE - (PAR - 1) + 2> last_chunk_col_SR;
		//#pragma HLS ARRAY_PARTITION variable=last_chunk_col_SR //cyclic factor=32
		//#pragma HLS bind_storage variable=last_chunk_col_SR type=RAM_1P latency=1
		//#pragma HLS ARRAY_PARTITION variable=last_chunk_col_SR complete

		// shift registers to align reads from the reference buffer
		int ref_SR[PAR][PAR];							
		#pragma HLS ARRAY_PARTITION variable=ref_SR complete		

		// shift registers to align reading the first comp_row of data buffer
		// shift register for when block_row == 0.  
		int data_h_SR[PAR][PAR] ;			
		#pragma HLS ARRAY_PARTITION variable=data_h_SR complete								

		// shift registers to align writes to external memory
		// similar idea to fef_SR
		int write_SR[PAR][PAR] ;		
		#pragma HLS ARRAY_PARTITION variable=write_SR complete					 				

		// this is for holding onto elements to satisfy dependencies when 
		// comp_col_offset == 0, aka when we're at the very left of a block.
		//one for left, one for top-left. 
		int input_v_SR[2] ;											
		#pragma HLS ARRAY_PARTITION variable=input_v_SR complete

		// initialize shift registers
		
		for (int i = 0; i < PAR - 1; i++)
		{
			#pragma HLS unroll	
			
			// for (int j = 0; j < 3; j++)
			// {
			// #pragma HLS UNROLL
			// 	out_SR[i][j] = 0;
			// }
			out_SR[i][0] = 0;
			out_SR[i][1] = 0;
			out_SR[i][2] = 0;
		}
		
		// for (int i = 0; i < BSIZE - PAR + 3; i++)
		// {
		// 	#pragma HLS UNROLL 
		// 	last_chunk_col_SR[i] = 0;
		// }
        //initialize shift reg
        for (int i = 0; i < BSIZE - PAR + 3; i++)
		{
		 	last_chunk_col_SR.shift(0,0);
		}

		
		for (int i = 0; i < PAR; i++)
		{
			#pragma HLS UNROLL
			for (int j = 0; j < PAR; j++)
			{
				#pragma HLS UNROLL
				write_SR[i][j] = 0;
			}
		}
		
		for (int i = 0; i < PAR; i++)
		{
			#pragma HLS unroll
			for (int j = 0; j < PAR; j++)
			{
				#pragma HLS unroll
				ref_SR[i][j] = 0;
			}
		}

		
		for (int i = 0; i < PAR; i++)
		{
			#pragma HLS unroll
			for (int j = 0; j < PAR; j++)
			{
				#pragma HLS unroll
				data_h_SR[i][j] = 0;
			}
		}

		
		for (int i = 0; i < 2; i++)
		{
			#pragma HLS unroll
			input_v_SR[i] = 0;
		}

		// starting points
		// where we're at in scoring matrix.
		int comp_col_offset = 0;
		// column to write to in scoring matrix
		int write_col_offset = -PAR;
		// what row we're in in the LOCAL block
		int block_row = 0;
		// index that keeps track of when we quit.
		int loop_index = 0;

		//#pragma ivdep array(data)
		while (loop_index != loop_exit)
		{
		
		#pragma HLS PIPELINE
		#pragma HLS dependence variable=data inter false  
		//#pragma HLS dependence variable=data inter RAW false

			loop_index++;
			
			// shift the shift registers
			
			for (int i = 0; i < PAR - 1; i++)
			{
				#pragma HLS unroll
				// #pragma HLS UNROLL
				// for (int j = 0; j < 2; j++)
				// {
				// 	out_SR[i][j] = out_SR[i][j + 1];
				// }
				out_SR[i][0] = out_SR[i][1];
				out_SR[i][1] = out_SR[i][2];
			}

			
			// for (int i = 0; i < BSIZE - PAR + 2; i++)
			// {
			// 	#pragma HLS unroll
			// 	last_chunk_col_SR[i] = last_chunk_col_SR[i + 1];
			// }
			

			
			for (int i = 0; i < PAR; i++)
			{
				#pragma HLS unroll
				for (int j = 0; j < PAR - 1; j++)
				{
					#pragma HLS unroll
					write_SR[i][j] = write_SR[i][j + 1];
				}
			}

			
			for (int i = 0; i < PAR; i++)
			{
				#pragma HLS unroll
				
				for (int j = 0; j < PAR - 1; j++)
				{
					#pragma HLS unroll
					ref_SR[i][j] = ref_SR[i][j + 1];
				}
			}

			
			for (int i = 0; i < PAR; i++)
			{
				#pragma HLS UNROLL
				for (int j = 0; j < PAR - 1; j++)
				{
					#pragma HLS unroll
					data_h_SR[i][j] = data_h_SR[i][j + 1];
				}
			}

			// #pragma HLS UNROLL
			// for (int i = 0; i < 1; i++)
			// {
			// 	input_v_SR[i] = input_v_SR[i + 1];
			// }
			input_v_SR[0] = input_v_SR[1];

			
			int read_block_row = block_row;
			//critical path issue?
			int read_row = block_offset + read_block_row;

			//if (comp_col_offset == 0 && read_row < dim - 1)
			if (comp_col_offset == 0 && read_row < dim + 1)
			//if (comp_col_offset == 0 && read_row < dim)
			{
				//critical path issue?
				input_v_SR[1] = input_v[read_row];
			}
			

			int read_index_offset = read_row * dim + comp_col_offset;
		
			if (block_row == 0 && read_row < (dim + 1))
			{		
				if (comp_col_offset + PAR - 1 >= dim){
					//int read_index_offset = read_row * dim + comp_col_offset;
					for (int i = 0; i < PAR; i++)
					{
						#pragma HLS pipeline II=1
						//exploring dependence to reduce II: add the following dependence
						//#pragma HLS dependence variable=data inter false
						//#pragma HLS dependence variable=data_h_SR inter false
						//#pragma HLS dependence variable=data intra false

						int read_col = comp_col_offset + i;
						//int read_index = read_row * dim + read_col;

						//if (read_col < dim - 1 && read_row < dim + 1)
						//if (read_col < dim && read_row < dim + 1)
						//if (read_col < dim && read_row < dim)
						if (read_col < dim)
						{
							data_h_SR[i][i] = data[read_index_offset + i];
						}
					}
				}
				else{
					for (int i = 0; i < PAR; i++)
					{
						//CHANGE UNROLL TO PIPELINE
						#pragma HLS pipeline II=1
						int read_col = comp_col_offset + i;

						data_h_SR[i][i] = data[read_index_offset + i];
					}
				}
				
			}
			
		
			

			//change to ++
			//read reference
			if (read_row > 0 && read_row < dim + 1){
				if (comp_col_offset + PAR - 1 >= dim){
					for (int i = 0; i < PAR ; i++){
						#pragma HLS pipeline 
						int read_col = comp_col_offset + i;
						if (read_col < dim){
							ref_SR[i][i] = reference[read_index_offset + i];
						}
					}
				}
				else{
					for (int i = 0; i < PAR ; i++){
						#pragma HLS pipeline 
						ref_SR[i][i] = reference[read_index_offset + i];			
					}
				}
			}
			// DONE shifting the shift registers
		
			//for (int i = 0; i < PAR ; i++)
			//{
				//#pragma HLS dependence variable=data inter false
				//#pragma HLS dependence variable=input_v_SR inter false
				//#pragma HLS dependence variable=last_chunk_col_SR inter false
				//#pragma HLS dependence variable=out_SR inter false
				//#pragma HLS dependence variable=data_h_SR inter false

				//anything comp_* is trying to figure out which elt in the scoring
				//matrix that we're trying to executre.
				//int comp_block_row = (BSIZE + block_row - i) & (BSIZE - 1); // read_col > 0 is skipped since it has area overhead and removing it is harmless

				//anything read_* is trying to figure out where in reference matrix
				//to read correct value
				//int read_col = comp_col_offset + i;
				//int read_index = read_row * dim + read_col;

				

				//if (read_row > 0 && read_col < dim - 1 && read_row < dim - 1) // read_col > 0 is skipped since it has area overhead and removing it is harmless
				//if (read_row > 0 && read_col < dim && read_row < dim + 1) // read_col > 0 is skipped since it has area overhead and removing it is harmless
				//if (read_row > 0 && read_col < dim && read_row < dim) // read_col > 0 is skipped since it has area overhead and removing it is harmless
				//{
					//ref_SR[i][i] = reference[read_index];
				//	ref_SR[i][i] = reference[read_index_offset + i];
				//}
			//}


            int last_chunk_col_SR_0 = last_chunk_col_SR.read(BSIZE - PAR + 2);
            int last_chunk_col_SR_1 = last_chunk_col_SR.read(BSIZE - PAR + 1);


			for (int i = 0; i < PAR ; i++)
			{
				#pragma HLS unroll
				
				int comp_block_row = (BSIZE + block_row - i) & (BSIZE - 1);

				int top      = (i == PAR - 1) ? last_chunk_col_SR.read(1) : out_SR[  i  ][1];
				//II violation: carried dependency
				int top_left = (comp_col_offset == 0 && i == 0) ? input_v_SR[0] : ((i == 0) ? last_chunk_col_SR_0 : out_SR[i - 1][0]);
				int left     = (comp_col_offset == 0 && i == 0) ? input_v_SR[1] : ((i == 0) ? last_chunk_col_SR_1 : out_SR[i - 1][1]);

				int out1 = top_left + ref_SR[i][0];
				int out2 = left - penalty;
				int out3 = top - penalty;
				int max_temp = (out1 > out2) ? out1 : out2;
				int max = (out3 > max_temp) ? out3 : max_temp;
                
				int out = (comp_block_row == 0) ? data_h_SR[i][0] : max;

				if (i == PAR - 1)									// if on last column in chunk
				{
					//#pragma HLS occurrence cycle=PAR
					last_chunk_col_SR.shift(out, 0);
                    //last_chunk_col_SR[BSIZE - PAR + 2] = out;
				}
				else
				{
					out_SR[i][2] = out;
				}

				write_SR[i][PAR - i - 1] = out;
			}


			//int write_col = write_col_offset + i;
			int write_block_row = (BSIZE + block_row - (PAR - 1)) & (BSIZE - 1);
			int write_row = block_offset + write_block_row;
			int write_index_offset = write_row * dim + write_col_offset;
			//change to ++

			// if ( write_block_row > 0 && write_row < dim + 1 ){
			// 	if (write_col_offset + PAR - 1 >= dim || write_col_offset < 0){
			// 		for (int i = 0; i < PAR; i++){
			// 			#pragma HLS pipeline
			// 			int write_col = write_col_offset + i;
						
							
			// 			if (write_col < dim && write_col > 0){
			// 				data[write_index_offset + i] = write_SR[i][0];	
			// 			}	
			// 		}
			// 	}
			// 	else{
			// 		for (int i = 0; i < PAR; i++){
			// 			#pragma HLS pipeline
			// 			data[write_index_offset + i] = write_SR[i][0];
			// 		}	
			// 	}

			// }

			// correction:
			if ( write_block_row > 0 && write_row < dim + 1 && write_col_offset >= 0){
				if (write_col_offset + PAR - 1 >= dim ){
					for (int i = 0; i < PAR; i++){
						#pragma HLS pipeline
						int write_col = write_col_offset + i;
						
							
						if (write_col < dim ){
							data[write_index_offset + i] = write_SR[i][0];	
						}	
					}
				}
				else{
					for (int i = 0; i < PAR; i++){
						#pragma HLS pipeline
						data[write_index_offset + i] = write_SR[i][0];
					}	
				}

			}

			//critical path issue?
			block_row = (block_row + 1) & (BSIZE - 1);
			if (block_row == PAR - 1)
			{
				write_col_offset += PAR;
			}
			// if block_row = 0, that means we're done processing that chunk of
			// columns and can move on to the next chunk. In Figure 4-1 of the 
			// thesis, that is equivalent to starting to process the 5th column
			if (block_row == 0)
			{
				comp_col_offset += PAR;
			}
		
		}
	}
}
