#include "nw_opencl.hpp"

//defualt constructor
//NW_OpenCL():
//{}



//standard constructor
NW_OpenCL::NW_OpenCL(
	int kernel_v,
	bool print_debug,				
	int lw_size,
	int gw_size
	//
	//std::string kernel_name
) : kernel_v(kernel_v), verbose_flag(print_debug), lw_size(lw_size), 
	gw_size(gw_size)
{
	//OpenCL cl(1);  // 1 means to display output (debugging mode).
	//	taken care of init. list
	
	// start here
	//cl.init(version);
	//	this fxn wraps:
	//			GetDevices
	//			BuildKernel
}

void NW_OpenCL::GetDevices()
{
	std::vector<cl::Platform> platforms;
	OCL_CHECK(err, err = cl::Platform::get(&platforms));
	cl::Platform platform;
	for (int i  = 0 ; i < platforms.size(); i++){
		platform = platforms[i];
		OCL_CHECK(err, std::string platformName = platform.getInfo<CL_PLATFORM_NAME>(&err));
		if (platformName == "Xilinx"){
			std::cout << "Found Platform" << std::endl;
			std::cout << "Platform Name: " << platformName.c_str() << std::endl;
			break;
		}
	}

	OCL_CHECK(err, platform.getDevices(CL_DEVICE_TYPE_ALL, &devices));
}

void NW_OpenCL::BuildKernel(int version, std::string binary_name,
	cl::Program::Binaries& bins)
{
	unsigned int file_buf_size;	
	char *file_buf = xlx_rodinia::read_binary_file(binary_name, file_buf_size);
	bins = {{file_buf, file_buf_size}};

	// TODO: figure out a way to free file_buf

}

//1. Define Plaform Device Context and Queues
//2. Create & build the program
//3. Create and setup kernel, creation of memory objects like buffer are left out in main()
void NW_OpenCL::init(int version, const std::string build_tgt, 
	const std::string kernel_name)
{
	//1. Step 1
    GetDevices();

	devices.resize(1); //Isolating the first Xilinx FPGA enumerated
	OCL_CHECK(err, cl::Device device(devices[0]));
	
	//OCL_CHECK(err, ctx = cl::Context(device, NULL, NULL, NULL, &err));
	ctx = cl::Context(device, NULL, NULL, NULL, &err);
	OCL_CHECK_NO_CALL(err, "creating ctx");
	std::cout << ctx.getInfo<CL_CONTEXT_NUM_DEVICES>() << std::endl;
	//OCL_CHECK(err, q = cl::CommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &err));
	std::cout << "hi starting cmd q" << std::endl;
	cmd_q = cl::CommandQueue(ctx, device, CL_QUEUE_PROFILING_ENABLE, &err);
	OCL_CHECK_NO_CALL(err, "creating cmd_q");
	//std::cout << err << std::endl;

    //The second command queue doesn't seem to be used in original code
    // cmd_q2 = cl::CommandQueue(ctx, device, CL_QUEUE_PROFILING_ENABLE, &err);
	// OCL_CHECK_NO_CALL(err, "creating cmd_q2");
	// std::cout << err << std::endl;
	// std::cout << "hi done with cmd q" << std::endl;

	//3 & 4.Step4 
	//getVersionName
	//build_tgt means directory path?
	std::string my_kernel_name(
		std::string("./") + build_tgt + std::string("/nw_kernel_v")
		+ std::to_string(version)
	);
	//binaries is the OpenCl xlinix bin file
	std::string binary_name(
		my_kernel_name
		+ std::string(".xclbin")
	);
	std::cout << "my_kernel_name = " << my_kernel_name << std::endl;
	std::cout << "binary_name = " << binary_name << std::endl;

	cl::Program::Binaries bins;

	BuildKernel(version, binary_name, bins);

	OCL_CHECK(err, cl::Program program(ctx, devices, bins, NULL, &err)); 

	OCL_CHECK(err, cl::Kernel krnl_dynproc(program, kernel_name.c_str(), &err));

	str_kernel_map[kernel_name.c_str()] = krnl_dynproc;

/*
  TODO:
	// Memory cleanup for the variable used to hold the kernel source.
*/
    //free binaries file buffer
	for (int i = 0; i < bins.size(); ++i)
		delete[] const_cast<char *>(static_cast<const char*>(bins[i].first));
}


NW_OpenCL::~NW_OpenCL()
{
	
}




