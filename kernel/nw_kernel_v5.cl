//using Anthony's v5
#include "nw_common.h"


/*
 * kernel parameters:
 *
 * reference: the reference matrix that holds the cost of moving to a
 * particular sqaure. The dimensions are max_rows * (max_cols - 1)
 * data: the scoring matrix. The dimensionx 
 *
 */

//__attribute__((max_global_work_dim(0))), Zhili: I think there is an Xilinx equivalent
__attribute__((reqd_work_group_size(1, 1, 1)))
__kernel void nw_dynproc_kernel(__global int* restrict reference, 
                         __global int* restrict data,
                         __global int* restrict input_v,// vertical input (first column)
                                  int           dim,
                                  int           penalty,
                                  int           loop_exit,
                                  int           block_offset)
{
	// output shift register; 2 registers per parallel comp_col_offset is 
	// required, one for writing, and two for passing data to the following 
	// diagonal lines to handle the dependency
								
	int out_SR[PAR - 1][3] __attribute__((xcl_array_partition(complete, 0)));	

	// shift register for last comp_col_offset in parallel chunk to pass data 
	// to next chunk, (PAR - 1) cells are always out of bound, one extra cell 
	// is added for writing and one more for top-left
	// It's for the vertical buffer for the next chunk of columns, where chunk
	// is of size BLKSIZE * PAR. It serves as the left and top left dependencies
	// for when elements that fall on the right of a chunk boundary.
	
	//int last_chunk_col_SR[BSIZE - (PAR - 1) + 2] __attribute__((xcl_array_partition(complete, 0)));					
	
	int last_chunk_col_SR[BSIZE - (PAR - 1) + 2];

	// shift registers to align reads from the reference buffer
	int ref_SR[PAR][PAR] __attribute__((xcl_array_partition(complete, 0)));										

	// shift registers to align reading the first comp_row of data buffer
	// shift register for when block_row == 0.  
	int data_h_SR[PAR][PAR] __attribute__((xcl_array_partition(complete, 0)));										

	// shift registers to align writes to external memory
	// similar idea to fef_SR
	int write_SR[PAR][PAR] __attribute__((xcl_array_partition(complete, 0)));						 				

	// this is for holding onto elements to satisfy dependencies when 
	// comp_col_offset == 0, aka when we're at the very left of a block.
	//one for left, one for top-left. 
	int input_v_SR[2] __attribute__((xcl_array_partition(complete, 0)));											

	// initialize shift registers
	__attribute__((opencl_unroll_hint(PAR - 1)))
	for (int i = 0; i < PAR - 1; i++)
	{
		__attribute__((opencl_unroll_hint(3)))
		for (int j = 0; j < 3; j++)
		{
			out_SR[i][j] = 0;
		}
	}
	__attribute__((opencl_unroll_hint(BSIZE - PAR + 3)))
	for (int i = 0; i < BSIZE - PAR + 3; i++)
	{
		last_chunk_col_SR[i] = 0;
	}

	__attribute__((opencl_unroll_hint(PAR)))
	for (int i = 0; i < PAR; i++)
	{
		__attribute__((opencl_unroll_hint(PAR)))
		for (int j = 0; j < PAR; j++)
		{
			write_SR[i][j] = 0;
		}
	}
	__attribute__((opencl_unroll_hint(PAR)))
	for (int i = 0; i < PAR; i++)
	{
		__attribute__((opencl_unroll_hint(PAR)))
		for (int j = 0; j < PAR; j++)
		{
			ref_SR[i][j] = 0;
		}
	}

	__attribute__((opencl_unroll_hint(PAR)))
	for (int i = 0; i < PAR; i++)
	{
		__attribute__((opencl_unroll_hint(PAR)))
		for (int j = 0; j < PAR; j++)
		{
			data_h_SR[i][j] = 0;
		}
	}
	__attribute__((opencl_unroll_hint(2)))
	for (int i = 0; i < 2; i++)
	{
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
		__attribute__((xcl_dependence(variable="data", type="inter", direction="RAW", dependent="false")));
		loop_index++;

		// shift the shift registers
		__attribute__((opencl_unroll_hint(PAR-1)))
		for (int i = 0; i < PAR - 1; i++)
		{
			__attribute__((opencl_unroll_hint(2)))
			for (int j = 0; j < 2; j++)
			{
				out_SR[i][j] = out_SR[i][j + 1];
			}
		}
		__attribute__((opencl_unroll_hint(BSIZE - PAR + 2)))
		for (int i = 0; i < BSIZE - PAR + 2; i++)
		{
			last_chunk_col_SR[i] = last_chunk_col_SR[i + 1];
		}
		
		__attribute__((opencl_unroll_hint(PAR)))
		for (int i = 0; i < PAR; i++)
		{
			__attribute__((opencl_unroll_hint(PAR - 1)))
			for (int j = 0; j < PAR - 1; j++)
			{
				write_SR[i][j] = write_SR[i][j + 1];
			}
		}
		__attribute__((opencl_unroll_hint(PAR)))
		for (int i = 0; i < PAR; i++)
		{
			__attribute__((opencl_unroll_hint(PAR - 1)))
			for (int j = 0; j < PAR - 1; j++)
			{
				ref_SR[i][j] = ref_SR[i][j + 1];
			}
		}
		__attribute__((opencl_unroll_hint(PAR)))
		for (int i = 0; i < PAR; i++)
		{
			__attribute__((opencl_unroll_hint(PAR - 1)))
			for (int j = 0; j < PAR - 1; j++)
			{
				data_h_SR[i][j] = data_h_SR[i][j + 1];
			}
		}
		__attribute__((opencl_unroll_hint(1)))
		for (int i = 0; i < 1; i++)
		{
			input_v_SR[i] = input_v_SR[i + 1];
		}

		int read_block_row = block_row;
		int read_row = block_offset + read_block_row;

		//if (comp_col_offset == 0 && read_row < dim - 1)
		if (comp_col_offset == 0 && read_row < dim + 1)
		//if (comp_col_offset == 0 && read_row < dim)
		{
			input_v_SR[1] = input_v[read_row];
		}

		if (block_row == 0)
		{		
			__attribute__((opencl_unroll_hint(PAR)))
			for (int i = 0; i < PAR; i++)
			{
				int read_col = comp_col_offset + i;
				int read_index = read_row * dim + read_col;

				//if (read_col < dim - 1 && read_row < dim + 1)
				if (read_col < dim && read_row < dim + 1)
				//if (read_col < dim && read_row < dim)
				{
					data_h_SR[i][i] = data[read_index] ;
				}
			}
		}
		// DONE shifting the shift registers

		__attribute__((opencl_unroll_hint(PAR)))
		for (int i = PAR - 1; i >= 0; i--)
		{
			//anything comp_* is trying to figure out which elt in the scoring
			//matrix that we're trying to executre.
			int comp_block_row = (BSIZE + block_row - i) & (BSIZE - 1); // read_col > 0 is skipped since it has area overhead and removing it is harmless

			//anything read_* is trying to figure out where in reference matrix
			//to read correct value
			int read_col = comp_col_offset + i;
			int read_index = read_row * dim + read_col;

			if (read_row > 0 && read_col < dim - 1 && read_row < dim - 1) // read_col > 0 is skipped since it has area overhead and removing it is harmless
			//if (read_row > 0 && read_col < dim && read_row < dim + 1) // read_col > 0 is skipped since it has area overhead and removing it is harmless
			//if (read_row > 0 && read_col < dim && read_row < dim) // read_col > 0 is skipped since it has area overhead and removing it is harmless
			{
				ref_SR[i][i] = reference[read_index];
			}

			int top      = (i == PAR - 1) ? last_chunk_col_SR[BSIZE - PAR + 1] : out_SR[  i  ][1];
			int top_left = (comp_col_offset == 0 && i == 0) ? input_v_SR[0] : ((i == 0) ? last_chunk_col_SR[0] : out_SR[i - 1][0]);
			int left     = (comp_col_offset == 0 && i == 0) ? input_v_SR[1] : ((i == 0) ? last_chunk_col_SR[1] : out_SR[i - 1][1]);

			int out1 = top_left + ref_SR[i][0];
			int out2 = left - penalty;
			int out3 = top - penalty;
			int max_temp = (out1 > out2) ? out1 : out2;
			int max = (out3 > max_temp) ? out3 : max_temp;

			// directly pass input to output if on the first row in the block which is overlapped with the previous block
			int out = (comp_block_row == 0) ? data_h_SR[i][0] : max;

			if (i == PAR - 1)									// if on last column in chunk
			{
				last_chunk_col_SR[BSIZE - PAR + 2] = out;
			}
			else
			{
				out_SR[i][2] = out;
			}

			write_SR[i][PAR - i - 1] = out;

			int write_col = write_col_offset + i;
			int write_block_row = (BSIZE + block_row - (PAR - 1)) & (BSIZE - 1); // write to memory is always PAR - 1 rows behind compute
			int write_row = block_offset + write_block_row;
			int write_index = write_row * dim + write_col;

			//if (write_block_row > 0 && write_col < dim - 1 && write_row < dim - 1)
			if (write_block_row > 0 && write_col < dim && write_row < dim + 1)
			//if (write_block_row > 0 && write_col < dim && write_row < dim+1)
			{
				data[write_index] = write_SR[i][0] ; //__attribute__((xcl_dependence(inter false)))
			}
		}

		block_row = (block_row + 1) & (BSIZE - 1);
		//printf("block_row = %d, \n", block_row);
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
