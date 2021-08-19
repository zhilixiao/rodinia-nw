//#include "./kernels/nw_common.h"
#include "./ckernel/nw_common.h"
#include "xlx_rodinia.hpp"
#include "nw.hpp"
#include "nw_opencl.hpp"


//std lib includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <assert.h>
#include <fstream>
#include <chrono>
#include <ratio>
#include <iomanip>
#include <ctime>

// Host application structure:
// 1) Setting up the hardware 
// 	- Retrieving the list of available *Xilinx platforms* (not other vendors)
// 	- Retrieving the list of devices supported by each Xilinx platform
// 	- Creating a context 
// 	- Creating a program object from the pre-compiled FPGA binary (xclbin)
// 	- Creating a kernel object
// 2) Executing the kernels
// 3) Releasing the hw resources after the kernel returns
//

using std::cout;
using std::endl;

inline static int is_ndrange_kernel(int version) {
  return (version % 2) == 0;
}

//modified the usage() in nw.c for arguments
void usage(int argc, char **argv)
{
	fprintf(stderr, "Usage: %s <max_rows/max_cols> <penalty> <output_file> <kernel_version> <build_target>\n", argv[0]);
	fprintf(stderr, "\t<dimension>  - x and y dimensions\n");
	fprintf(stderr, "\t<penalty> - penalty(positive integer)\n");
	fprintf(stderr, "\t<kernel_version> - version of kernel or bianry file to load\n");
	exit(1);
}

//Current main() omits the power related code 
int main(int argc, char** argv)
{
/*----------Original Code for Variables------------*/

	//from original code nw.c main(), parameters
	int max_rows, max_cols, penalty;
		
/*------------Our Code for Varibales------------*/	

	//Varibales exclusive for us
	NWVars nw;	
	// OpenCL variables
	NW_OpenCL nw_cl;
	cl_int err, result;
	size_t sourcesize;	//for program source file

	// specific to xilinx
	std::string build_tgt;
	std::string kernel_name("nw_dynproc_kernel");
	int version; // i.e., the kernel version, same as version_number
	
	//cl::Event for iterating kernels
	std::vector<cl::Event> iteration_events;
	cl::Event iteration_done;

/*------------Our Code for parsing arguments------------*/
	if (argc < 3 || argc > 7){
		usage(argc, argv);
	}
	
	build_tgt = xlx_rodinia::get_build_tgt(&argc, argv);	
	//the same as init_fpga2() which just retrieves the version string and version number,
	// note that this decrements argc by 1
	version = xlx_rodinia::get_krnl_version(&argc, argv);
	
/*------------Original Code for parsing remaining arguments------------*/		
	if (argc >= 3)
	{
		//max_rows = max_cols for nw
		max_rows = atoi(argv[1]);
		max_cols = atoi(argv[1]);
		penalty = atoi(argv[2]);
	}
	
	// the lengths of the two sequences should be divisible by 16.
	// And at current stage, max_rows needs to equal max_cols
	if (is_ndrange_kernel(version))
	{
		if(atoi(argv[1])%16!=0)
		{
			cout << "The dimension values must be a multiple of 16" << endl;
			exit(1);
		}
	}

/*------------Our Code for initilization------------*/		

	//create varibale and initialize
	max_rows = max_rows + 1;
	max_cols = max_cols + 1;

	if(argc == 5){
		cout << "debug mode" << endl;
	}

	nw = NWVars(max_rows, max_cols, atoi(argv[2]), version, argv[3], (argc == 5));

	cout << "penalty = " << nw.penalty << endl; 
	cout << "grid_size = " << nw.max_cols << endl;
	cout << "nw's num_rows = " << nw.num_rows << endl;
	cout << "nw's num_cols = " << nw.num_cols << endl;
	cout << "block_size = " << BSIZE << endl;
	cout << "ofname = " << nw.ofile_name << endl;
	
	
/*------------Original Code for reading kernels------------*/
		
	// set global and local workitems for multi-workitem kernel
	int nworkitems, workgroupsize = 0;
	nworkitems = BSIZE;

	if(nworkitems < 1 || workgroupsize < 0)
	{
		printf("ERROR: invalid or missing <num_work_items>[/<work_group_size>]\n"); 
		return -1;
	}
	int local_work = (workgroupsize>0)?workgroupsize:1;
	int global_work = nworkitems; //nworkitems = no. of GPU threads
	

	// OPENCL HOST CODE AREA START	
// ------------------------------------------------------------------------------------
// Step 1: Get All PLATFORMS, then search for Target_Platform_Vendor (CL_PLATFORM_VENDOR)
// Search for Platform: Xilinx 
// Check if the current platform matches Target_Platform_Vendor
// The get_devices function wraps the above steps 
// ------------------------------------------------------------------------------------	
// ------------------------------------------------------------------------------------
// Step 1: Create Context; OCL_CHECK wraps some error checking
// ------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------
// Step 1: Create Command Queue
// ------------------------------------------------------------------------------------
// ------------------------------------------------------------------
// Step 1: Load Binary File from disk
// ------------------------------------------------------------------		
// -------------------------------------------------------------
// Step 1: Create the program object from the binary and program the FPGA device with it
// -------------------------------------------------------------	
// -------------------------------------------------------------
// Step 1: Create Kernels
// -------------------------------------------------------------

// all of this is handled by PV_OpenCL::init
	nw_cl.init(version, build_tgt, kernel_name);
	
	
// ================================================================
// Step 2: Setup Buffers and run Kernels
// ================================================================
//   o) Allocate Memory to store the results 
//   o) Create Buffers in Global Memory to store data
// ================================================================

// ------------------------------------------------------------------
// Step 2: Create Buffers in Global Memory to store data
// ------------------------------------------------------------------	

// .......................................................
// Allocate Global Memory for source_in1
// .......................................................	
	//currently only v1 and v5 are ported
	cl::Buffer reference_d;
	cl::Buffer input_itemsets_d;
	
	reference_d = cl::Buffer(nw_cl.ctx,
			CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
			nw.ref_size * sizeof(int), 
			nw.reference, //NULL
			&err
		);
	OCL_CHECK_NO_CALL(err, "creating reference_d cl::Buffer");

	int device_buff_size =  (version >= 7) ? nw.num_cols * (nw.num_rows + 1) : ((version == 5) ? nw.ref_size : nw.data_size);

	//create buffer for input_itemsets
	input_itemsets_d = cl::Buffer(nw_cl.ctx,
		CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
		device_buff_size * sizeof(int), 
		nw.input_itemsets,//nw.output_itemsets, // //NULL
		&err
	);
	OCL_CHECK_NO_CALL(err, "creating input_itemsets_d cl::Buffer");

	//extra buffer for v5 and above   
	cl::Buffer buffer_v_d;
	if (version >= 5){
		buffer_v_d = cl::Buffer(nw_cl.ctx,
			CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
			nw.num_rows * sizeof(int), 
			nw.buffer_v, //NULL
			&err
		);
		OCL_CHECK_NO_CALL(err, "creating extra buffer_v_d cl::Buffer");
	}	





// ----------------------------------------
// Step 2: Submit Kernels for Execution
// ----------------------------------------
	int worksize = nw.max_cols - 1;
	cout << "WG size of kernel " << BSIZE << endl; 
	cout << "worksize " << worksize << endl; 

	//these two parameters are for extension use
	int offset_r = 0, offset_c = 0;
	int block_width = worksize/BSIZE;

	
/*------------Code for setting kernel arguments, line 410 in original------------*/	
	//constant kernel arguments
	if(is_ndrange_kernel(version)){
	}
	else if(version < 5)
	{
		OCL_CHECK(err, err = 
			nw_cl.str_kernel_map[kernel_name.c_str()].setArg(0, reference_d)
		);
		 
		OCL_CHECK(err, err = 
			nw_cl.str_kernel_map[kernel_name.c_str()].setArg(1, input_itemsets_d)
		);
	
		OCL_CHECK(err, err = 
			nw_cl.str_kernel_map[kernel_name.c_str()].setArg(2, nw.num_rows)
		);
	
		OCL_CHECK(err, err = 
			nw_cl.str_kernel_map[kernel_name.c_str()].setArg(3, penalty) 
		);
	}
	else
	{
		int cols = nw.num_cols - 1 + PAR; // -1 since last column is invalid, +PAR to make sure all cells in the last chunk are processed
		int exit_col = (cols % PAR == 0) ? cols : cols + PAR - (cols % PAR);
		int loop_exit = exit_col * (BSIZE / PAR);
		cout << "exit_col = " <<  exit_col << endl;
		cout << "loop_exit = " << loop_exit << endl;

		OCL_CHECK(err, err = 
			nw_cl.str_kernel_map[kernel_name.c_str()].setArg(0, reference_d)
		);
		 
		OCL_CHECK(err, err = 
			nw_cl.str_kernel_map[kernel_name.c_str()].setArg(1, input_itemsets_d)
		);

		OCL_CHECK(err, err = 
			nw_cl.str_kernel_map[kernel_name.c_str()].setArg(2, buffer_v_d)
		);
	
		OCL_CHECK(err, err = 
			nw_cl.str_kernel_map[kernel_name.c_str()].setArg(3, nw.num_cols)
		);
	
		OCL_CHECK(err, err = 
			nw_cl.str_kernel_map[kernel_name.c_str()].setArg(4, penalty) 
		);

		OCL_CHECK(err, err = 
			nw_cl.str_kernel_map[kernel_name.c_str()].setArg(5, loop_exit) 
		);
	}

	cout << "kernel arguments are set" << endl;

// ------------------------------------------------------
// Step 2: Copy Input data from Host to Global Memory on the device
// ------------------------------------------------------
	
	//enqueue write buffer for input_itemsets
	if(version == 5){
		//buffer_h
		OCL_CHECK(err, err = nw_cl.cmd_q.enqueueWriteBuffer(
				input_itemsets_d, 1, 0, 
				nw.num_cols * sizeof(int), 
				nw.buffer_h, //NULL
				NULL, NULL)
			);
		nw_cl.cmd_q.finish();

	}else{
		OCL_CHECK(err, err = nw_cl.cmd_q.enqueueWriteBuffer(
				input_itemsets_d, 1, 0,
				device_buff_size * sizeof(int),
				nw.input_itemsets,
				NULL, NULL)
			);
		nw_cl.cmd_q.finish();
		cout << "finsih write buffer for input_itemsets" << endl;
	}
	
	OCL_CHECK_NO_CALL(err, "enqueue write buffer input_itemsets_d");		


	//enqueueMigrateMemObjects, enqueue a command to associate memory object with devices
	if (version == 5){
		OCL_CHECK(err, nw_cl.cmd_q.enqueueMigrateMemObjects(
			{reference_d, buffer_v_d},
			0)
		);
		nw_cl.cmd_q.finish();
	}
	else{
		OCL_CHECK(err, nw_cl.cmd_q.enqueueMigrateMemObjects(
			{reference_d},
			0)
		);
		
		nw_cl.cmd_q.finish();
		cout << "finish migrate memobjects for reference" << endl;

	}




	std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();	
/*------------Code for lauching kernels, line 468 in original------------*/	
	//events
	if (is_ndrange_kernel(version))
	{
	}
	else if (version < 5)
	{
		cout << "executing v1 or v3" << endl;
		OCL_CHECK(err, 
				err = nw_cl.cmd_q.enqueueTask(
						nw_cl.str_kernel_map[kernel_name.c_str()],
						NULL,
						NULL)
		);
		nw_cl.cmd_q.finish();
	}
	else
	{
		cout << "executing v5" << endl;
		int num_diags  = nw.max_rows - 1; // -1 since the first row is unchanged and the last row is invalid
		int comp_bsize = BSIZE - 1;
		int last_diag  = (num_diags % comp_bsize == 0) ? num_diags : num_diags + comp_bsize - (num_diags % comp_bsize);
		//int last_diag  = (num_diags % BSIZE == 0) ? num_diags : num_diags + comp_bsize - (num_diags % comp_bsize);
		int num_blocks = last_diag / comp_bsize;

		cout << "num_blocks = " << num_blocks << endl;
		cout << "BSIZE = " << BSIZE << endl;
		cout << "PAR = " << PAR << endl; 

		for (int bx = 0; bx < num_blocks ; bx++)
		{
			int block_offset = bx * comp_bsize;
			//cout << "bx:" << bx << endl;

			OCL_CHECK(err, err = 
						nw_cl.str_kernel_map[kernel_name.c_str()].setArg(6, block_offset) 
			);

			// if(bx == 0){
			// 	OCL_CHECK(err, 
			// 	err = nw_cl.cmd_q.enqueueTask(
			// 			nw_cl.str_kernel_map[kernel_name.c_str()],
			// 			NULL,
			// 			&iteration_done)
			// 	);
			// }
			// else{
			// 	OCL_CHECK(err, 
			// 	err = nw_cl.cmd_q.enqueueTask(
			// 			nw_cl.str_kernel_map[kernel_name.c_str()],
			// 			&iteration_events,
			// 			&iteration_done)
			// 	);
			// }
			
			// iteration_events.push_back(iteration_done);
			OCL_CHECK(err, 
			err = nw_cl.cmd_q.enqueueTask(
			 			nw_cl.str_kernel_map[kernel_name.c_str()],
			 			NULL,
			 			NULL)
			);
			nw_cl.cmd_q.finish();
		}
	}
	
	cout << "finish kernel execution" << endl;

	std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();

	std::chrono::duration<double> time_span = 
		std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);

	std::cout << "Computation done in "; 
	std::cout << std::setprecision(5)
		<< std::fixed 
		<< time_span.count()
		<< " s."
		<< std::endl;

// --------------------------------------------------
// Step 2: Copy Results from Device Global Memory to Host
// --------------------------------------------------
/*------------Code for copy results, line 523 in original------------*/
	OCL_CHECK(err, err = nw_cl.cmd_q.enqueueReadBuffer(input_itemsets_d, 1, 0, 
		nw.ref_size * sizeof(int), nw.output_itemsets, NULL,NULL)
	);

	nw_cl.cmd_q.finish();

	cout << "copy result from device to host" << endl;

	
	// OPENCL HOST CODE AREA END

// ============================================================================
// Step 3: Release Allocated Resources
// ============================================================================
/*------------Code for output file, line 545 in original------------*/
	//OpenCl objects in C++ API do not need release
	//int start_j = (version >= 5) ? 0 : 1;
	if (argc == 5)
	{
		std::ofstream solution_file;
		solution_file.open("solution.txt");
		solution_file << "Solution:" << endl;

		cout << "v1's dim = " << nw.num_rows << endl;
		
		if (version == 5) {
			solution_file << "v5's dim = " << nw.num_cols << endl;
			solution_file << "max_cols = " << nw.max_cols << endl; 
			solution_file << "max_rows = " << nw.max_rows << endl; 
			solution_file << "num_cols = " << nw.num_cols << endl; 
			solution_file << "num_rows = " << nw.num_rows << endl; 

			solution_file << "device_buff_size = " << device_buff_size << endl;
			
			solution_file << "input_v: " << endl; 
			for (int i = 0; i < max_rows; i++){
				solution_file << nw.buffer_v[i] << " ";
			}
			solution_file << endl;
		}

		solution_file << "solution: " << endl; 
		for (int i = 0; i < nw.num_rows; i++)
		{
			for (int j = 0; j < nw.num_cols; j++)
				solution_file << nw.output_itemsets[i * nw.num_cols + j] << " ";

			solution_file << std::endl;
		}
		
		cout << "returned output itemsets saved in solution.txt" << endl;
	
/*------------Original Code for tracing back, line 560------------*/
		cout << "start tracing back" << endl;			
	 	solution_file << "Traceback Result:" << endl;
		int i, j;
	 	for(i = nw.max_cols - 2, j = nw.max_rows - 2; (i>=0 && j>=0);)
		{
			solution_file << "[" << i << " , " << j << "] ";
	    	int nw_r = 0, n = 0, w = 0, traceback;
	    	if (i == 0 && j == 0) 
			{
				solution_file << "(output: " << nw.output_itemsets[0] << ")" << endl;
	      		break;
	    	}
	     	if (i > 0 && j >= 0)
		 	{
	       		nw_r = nw.output_itemsets[(i - 1) * nw.num_cols + j - 1];
	       		w  = nw.output_itemsets[ i * nw.num_cols + j - 1 ];
	       		n  = nw.output_itemsets[(i - 1) * nw.num_cols + j];
		 		solution_file << "(nw: " << nw_r << ", w: " << w << ", n: " << n << ", ref: " << nw.reference[i * nw.num_cols+j] << ")";
	   	 	}
	    	else if (i == 0)
			{
	      		nw_r = n = LIMIT;
	      		w  = nw.output_itemsets[ i * nw.num_cols + j - 1 ];
	    	}
	    	else if (j == 0){
	      		nw_r = w = LIMIT;
	      		n = nw.output_itemsets[(i - 1) * nw.num_cols + j];
	    	}
	    	else{
	    	}

	    	int new_nw, new_w, new_n;
	    	new_nw = nw_r + nw.reference[i * nw.num_cols + j];
	    	new_w = w - nw.penalty;
	    	new_n = n - nw.penalty;

	    	traceback = maximum(new_nw, new_w, new_n);
	    	if (traceback != nw.output_itemsets[i * nw.num_cols +j]) 
			{
				cout << "Mismatch at (" << i << ", " << j <<")." << endl;
				cout << "traceback: " << traceback << endl;
				cout << "output_itemsets: " << nw.output_itemsets[i * nw.num_cols+j] << endl;
	      		
	    	}
	    	solution_file <<  "(output: " << traceback << ")" ;

	    	if(traceback == new_nw) 
			{
	      		traceback = nw_r;
	      		solution_file <<  "(->nw_r) ";
	    	} 
			else if(traceback == new_w) 
			{
	      		traceback = w;
	      		solution_file << "(->w) ";
	    	} 
			else if(traceback == new_n) 
			{
	      		traceback = n;
	      		solution_file << "(->n) ";
	    	} 
			else 
			{
	      		cout << "Error: inconsistent traceback at (" << i << ", " << j << ")" << endl;
	      		abort();
	    	}

	    	solution_file << endl;

	    	if(traceback == nw_r)
	    	{
				i--; 
				j--; 
				continue;
			}

	    	else if(traceback == w)
	    	{
				j--; 
				continue;
			}

	    	else if(traceback == n)
	    	{
				i--; 
				continue;
			}

	    	else;
	  	}
	  	cout << "Traceback saved in result.txt" << endl;
	}
	return 0;
}
