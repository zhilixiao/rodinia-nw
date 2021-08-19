#ifndef XLX_RODINIA_HPP
#define XLX_RODINIA_HPP

// these pound defines necessary to feed to cl2.hpp
#define CL_HPP_CL_2_2_DEFAULT_BUILD
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY 1
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <iostream>
#include <cstdio>
#include <vector>
#include <string>
#include <fstream>
#include <unistd.h> // for R_OK, access
#include <CL/cl2.hpp>

// from Xilinx example code
//OCL_CHECK doesn't work if call has templatized function call
#define OCL_CHECK(error,call)                                       \
    call;                                                           \
    if (error != CL_SUCCESS) {                                      \
      printf("%s:%d Error calling " #call ", error code is: %d\n",  \
              __FILE__,__LINE__, error);                            \
      exit(EXIT_FAILURE);                                           \
    }

// Something I had to implement that is a variation of the above macro
#define OCL_CHECK_NO_CALL(error,call)                               \
    if (error != CL_SUCCESS) {                                      \
      printf("%s:%d Error calling " #call ", error code is: %d\n",  \
              __FILE__,__LINE__, error);                            \
      exit(EXIT_FAILURE);                                           \
    }

namespace xlx_rodinia
{
	// this grabs the build target from argv
	// this will decrease argc by 1
	// build target must always be the last argument
	std::string get_build_tgt(int *argc, char **argv);	

	// this takes the place of init_fpga
	// this will also decrease argc
	int get_krnl_version(int *argc, char **argv);

	// TODO: say sum'n bout dis fxn
	void display_device_info(std::vector<cl::Platform> &platforms);

	// ripping this straight out of the Xilinx v-add example
	// replaces read_kernel
	char* read_binary_file(const std::string &xclbin_file_name, unsigned &nb); 

	
}


#endif // XLX_RODINIA_HPP
