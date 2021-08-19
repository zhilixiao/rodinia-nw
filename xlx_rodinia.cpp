#include "xlx_rodinia.hpp"

std::string xlx_rodinia::get_build_tgt(int *argc, char **argv)
{
	char build_tgt[8]; 
	int arg_idx = *argc - 1;

	std::cerr << "Grab build target from argv" << std::endl;
	std::cerr << "arg_idx = " << arg_idx << std::endl;
	std::cerr << "argv[" << arg_idx << "] = " << argv[arg_idx] << std::endl;
	if (arg_idx > 0)
		int ret = std::sscanf(argv[arg_idx], "%8s", build_tgt);

	std::string ret_build_tgt(build_tgt);
	std::cerr << "Build tgt was found to be " << ret_build_tgt << std::endl;

	*argc -= 1;
	std::cerr << "argc now = " << *argc << std::endl;
	return std::string(build_tgt);
}

int xlx_rodinia::get_krnl_version(int *argc, char **argv)
{
	int v_num = 0;
	int shift = 0;
	int arg_idx = *argc - 1;

	std::cerr << "Grab kernel version number" << std::endl;
	std::cerr << "arg_idx = " << arg_idx << std::endl;
	std::cerr << "argv[" << arg_idx << "] = " << argv[arg_idx] << std::endl;
	if (arg_idx > 0)
	{
		int ret = std::sscanf(argv[arg_idx], "v%d", &v_num);
		if (ret == 1)
			++shift;
	}
	std::cerr << "Using version " << v_num << std::endl;

	*argc -= shift;
	std::cerr << "argc now = " << *argc << std::endl;
	return v_num;
}

// returns number of platforms found
void xlx_rodinia::display_device_info(std::vector<cl::Platform> &platforms)
{
	cl_int err;
	cl_uint device_count;	
	
	// Get all platforms
	OCL_CHECK(err, cl::Platform::get(&platforms));
	std::cout << "num platforms = " << platforms.size() << std::endl;;

	fprintf(stderr, "\nQuerying devices for info:\n");
	for (std::vector<cl::Platform>::iterator it1 = platforms.begin(); 
		it1 < platforms.end(); ++it1)
	{
		cl::Platform platform(*it1);
		std::cout 
			<< "Platform Name: " 
			<< platform.getInfo<CL_PLATFORM_NAME>() 
			<< std::endl;
		std::cout 
			<< "Platform Vendor: " 
			<< platform.getInfo<CL_PLATFORM_VENDOR>() 
			<< std::endl;

		std::vector<cl::Device> devices;
		platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);

		for(std::vector<cl::Device>::iterator it2 = devices.begin();
			it2 < devices.end(); ++it2)
		{
			cl::Device device(*it2);
			std::cout 
				<< "\t\tDevice Vendor: " 
				<< device.getInfo<CL_DEVICE_VENDOR>() 
				<< std::endl;
			std::cout 
				<< "\t\tDevice Name: " 
				<< device.getInfo<CL_DEVICE_NAME>() 
				<< std::endl;  
			std::cout 
				<< "\t\tDevice Version: " 
				<< device.getInfo<CL_DEVICE_VERSION>()
				<< std::endl;  
			std::cout 
				<< "\t\tDevice Global Memory: " 
				<< device.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>() 
				<< std::endl;
			std::cout 
				<< "\t\tDevice Local Memory: " 
				<< device.getInfo<CL_DEVICE_LOCAL_MEM_SIZE>() 
				<< std::endl;
			std::cout
				<< "\t\tDevice Max Allocateable Memory: " 
				<< device.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>() 
				<< std::endl;
			std::cout 
				<< "\t\tDevice Local Memory: " 
				<< device.getInfo<CL_DEVICE_LOCAL_MEM_SIZE>() 
				<< std::endl;
			std::cout 
				<< "\t\tDevice Type: " 
				<< device.getInfo<CL_DEVICE_TYPE>()
				<< std::endl;  
		}
		
	}
}

char* xlx_rodinia::read_binary_file(
	const std::string &xclbin_file_name, unsigned &nb) 
{
    std::cout << "INFO: Reading " << xclbin_file_name << std::endl;

	if(access(xclbin_file_name.c_str(), R_OK) != 0) {
		printf("ERROR: %s xclbin not available please build\n", xclbin_file_name.c_str());
		exit(EXIT_FAILURE);
	}
    //Loading XCL Bin into char buffer 
    std::cout << "Loading: '" << xclbin_file_name.c_str() << "'\n";
    std::ifstream bin_file(xclbin_file_name.c_str(), std::ifstream::binary);
    bin_file.seekg (0, bin_file.end);
    nb = bin_file.tellg();
    bin_file.seekg (0, bin_file.beg);
    char *buf = new char [nb];
    bin_file.read(buf, nb);
    return buf;
}
