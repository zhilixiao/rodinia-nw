#include <map>
#include <vector>
#include <string>

#include "xlx_rodinia.hpp"
//#include "./kernels/nw_common.h"
#include "./ckernel/nw_common.h"
#include <CL/cl2.hpp> // c++ bindings in /usr/include/CL
#include <iostream>
// not sure if these headers are meaningful
/*#include <cstring>
#include <CL/cl.h>
#include <CL/cl_gl.h>
#include <CL/cl_gl_ext.h>
#include <CL/cl_ext.h>*/

// Modifying the OpenCL class from OpenCL.hpp 
// - variable/fxn name case
// - struct instead of a class
// - everything is public. why should it be private?
typedef struct NW_OpenCL
{
/***********Original local varibles and varibles in initliaze() in nw.c**************/
	//from initialize() & local varibles
	int kernel_v;
	bool verbose_flag;
	size_t size;
	//local_work[3] = { (size_t)((workgroupsize>0)?workgroupsize:1), 1, 1 };	
	size_t lw_size;
	//global_work[3] = { (size_t)nworkitems, 1, 1 }; //nworkitems = no. of GPU threads = BSIZE
	size_t gw_size;
	cl_int num_devices;
	cl_int result;
    //does not need to be a member variable
	cl_int err;

	//C++ class for C's data struct
	cl::Context ctx;
	cl::CommandQueue cmd_q;
	cl::CommandQueue cmd_q2;	//need 2 cmd queues?
	std::vector<cl::Device> devices;
    std::map<std::string, cl::Kernel> str_kernel_map; 
    cl::Program program;

	//constructor
	NW_OpenCL(){}
	NW_OpenCL(
		int kernel_v,
		bool print_debug,			
		int lw_size,
		int gw_size
	);
	//destructor
	~NW_OpenCL();
	void GetDevices();
	void BuildKernel(int version, std::string binary_name, 
		cl::Program::Binaries& bins);
	void init(int version, const std::string build_tgt,
		const std::string kernel_name);
	void createKernel(std::string kernelName);
	cl::Kernel kernel(std::string kernelName);
	void launch(std::string to_launch, int version);
    
	// move assignment, why overload?
	// NW_OpenCL& operator=(NW_OpenCL&& other);
	

	// I think the next few are getters and I don't think we need them?
	//void gwSize(size_t theSize);
	//void lwSize(size_t theSize);
	//cl::Context ctxt();
	//cl::command_queue q();

} NW_OpenCL;
