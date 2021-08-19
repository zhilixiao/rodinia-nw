#include "nw.hpp"



const int blosum62[24][24] = {
{ 4, -1, -2, -2,  0, -1, -1,  0, -2, -1, -1, -1, -1, -2, -1,  1,  0, -3, -2,  0, -2, -1,  0, -4},
{-1,  5,  0, -2, -3,  1,  0, -2,  0, -3, -2,  2, -1, -3, -2, -1, -1, -3, -2, -3, -1,  0, -1, -4},
{-2,  0,  6,  1, -3,  0,  0,  0,  1, -3, -3,  0, -2, -3, -2,  1,  0, -4, -2, -3,  3,  0, -1, -4},
{-2, -2,  1,  6, -3,  0,  2, -1, -1, -3, -4, -1, -3, -3, -1,  0, -1, -4, -3, -3,  4,  1, -1, -4},
{ 0, -3, -3, -3,  9, -3, -4, -3, -3, -1, -1, -3, -1, -2, -3, -1, -1, -2, -2, -1, -3, -3, -2, -4},
{-1,  1,  0,  0, -3,  5,  2, -2,  0, -3, -2,  1,  0, -3, -1,  0, -1, -2, -1, -2,  0,  3, -1, -4},
{-1,  0,  0,  2, -4,  2,  5, -2,  0, -3, -3,  1, -2, -3, -1,  0, -1, -3, -2, -2,  1,  4, -1, -4},
{ 0, -2,  0, -1, -3, -2, -2,  6, -2, -4, -4, -2, -3, -3, -2,  0, -2, -2, -3, -3, -1, -2, -1, -4},
{-2,  0,  1, -1, -3,  0,  0, -2,  8, -3, -3, -1, -2, -1, -2, -1, -2, -2,  2, -3,  0,  0, -1, -4},
{-1, -3, -3, -3, -1, -3, -3, -4, -3,  4,  2, -3,  1,  0, -3, -2, -1, -3, -1,  3, -3, -3, -1, -4},
{-1, -2, -3, -4, -1, -2, -3, -4, -3,  2,  4, -2,  2,  0, -3, -2, -1, -2, -1,  1, -4, -3, -1, -4},
{-1,  2,  0, -1, -3,  1,  1, -2, -1, -3, -2,  5, -1, -3, -1,  0, -1, -3, -2, -2,  0,  1, -1, -4},
{-1, -1, -2, -3, -1,  0, -2, -3, -2,  1,  2, -1,  5,  0, -2, -1, -1, -1, -1,  1, -3, -1, -1, -4},
{-2, -3, -3, -3, -2, -3, -3, -3, -1,  0,  0, -3,  0,  6, -4, -2, -2,  1,  3, -1, -3, -3, -1, -4},
{-1, -2, -2, -1, -3, -1, -1, -2, -2, -3, -3, -1, -2, -4,  7, -1, -1, -4, -3, -2, -2, -1, -2, -4},
{ 1, -1,  1,  0, -1,  0,  0,  0, -1, -2, -2,  0, -1, -2, -1,  4,  1, -3, -2, -2,  0,  0,  0, -4},
{ 0, -1,  0, -1, -1, -1, -1, -2, -2, -1, -1, -1, -1, -2, -1,  1,  5, -2, -2,  0, -1, -1,  0, -4},
{-3, -3, -4, -4, -2, -2, -3, -2, -2, -3, -2, -3, -1,  1, -4, -3, -2, 11,  2, -3, -4, -3, -2, -4},
{-2, -2, -2, -3, -2, -1, -2, -3,  2, -1, -1, -2, -1,  3, -3, -2, -2,  2,  7, -1, -3, -2, -1, -4},
{ 0, -3, -3, -3, -1, -2, -2, -3, -3,  3,  1, -2,  1, -1, -2, -2,  0, -3, -1,  4, -3, -2, -1, -4},
{-2, -1,  3,  4, -3,  0,  1, -1,  0, -3, -4,  0, -3, -3, -2,  0, -1, -4, -3, -3,  4,  1, -1, -4},
{-1,  0,  0,  1, -3,  3,  4, -2,  0, -3, -3,  1, -1, -3, -1,  0, -1, -3, -2, -2,  1,  4, -1, -4},
{ 0, -1, -1, -1, -2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -2,  0,  0, -2, -1, -1, -1, -1, -1, -4},
{-4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4,  1}
};

int maximum(int a, int b, int c)
{
	int k;
	if(a <= b)
		k = b;
	else 
		k = a;
	if(k <=c )
		return(c);
	else
		return(k);
}

NWVars::NWVars() :
	max_rows(-1),
	max_cols(-1),
	num_cols(-1),
	num_rows(-1),
	penalty(0),
	version(-1),
	data_size(-1),
	ref_size(-1),
	reference(nullptr),
	input_itemsets(nullptr),
	output_itemsets(nullptr),
	// for v5 and above?
	buffer_v(nullptr),
	buffer_h(nullptr),
	result_file(nullptr),
	ofile_name(""),
	//std::ofstream ofile;
	write_out(false)
{}

// standard constructor
// This holds the functionality of the old init part
NWVars::NWVars(int max_r, int max_c, int penalty, int version, const char *outfile_name, bool wo) 
: max_rows(max_r), max_cols(max_c), penalty(penalty), version(version), ofile_name(std::string(outfile_name)), write_out(wo)
{
/*------------Original Code to be taken used for NWVars initialization------------*/	
	//calculate parameters
	num_rows = max_rows;
	num_cols = (version == 5) ? max_cols - 1 : max_cols;

	data_size = max_cols * max_rows;
	ref_size = num_cols * num_rows;
	reference = (int *)AlignedMalloc(ref_size * sizeof(int));
	input_itemsets = (int *)AlignedMalloc(data_size * sizeof(int));
	output_itemsets = (int *)AlignedMalloc(ref_size * sizeof(int));
	// for v7 and above
	if (version >= 5)
	{
		buffer_h = (int *)AlignedMalloc(num_cols * sizeof(int));
		buffer_v = (int *)AlignedMalloc(num_rows * sizeof(int));
	}
	
	//initialization
	srand(7); //could be modified
	
	//first intialize all to zero
	for (int i = 0; i < max_cols; i++)
	{
		for (int j = 0; j < max_rows; j++)
		{
			input_itemsets[i * max_cols+j] = 0;
		}
	}
	//initialize the first column
	for(int i = 1; i < max_rows; i++)
	{    
		input_itemsets[i * max_cols] = rand() % 10 + 1;
	}
	
	for(int j = 1; j < max_cols; j++)
	{    //initialize the first row
		input_itemsets[j] = rand() % 10 + 1;
	}
	
	//initialize the reference[] using the random elements in input_itemsets and blosum62
	for (int i = 1; i < max_cols; i++)
	{
		for (int j = 1; j < max_rows; j++)
		{
			int ref_offset = (version == 5) ? i * num_cols + (j - 1) : i * num_cols + j;
			reference[ref_offset] = blosum62[input_itemsets[i * max_cols]][input_itemsets[j]];
		}
	}
	//buffer_v and input_itemsets
	for(int i = 1; i < max_rows; i++)
	{
		input_itemsets[i * max_cols] = -i * penalty;
		if (version == 5)
		{
			buffer_v[i] = -i * penalty;
		}
	}
	if (version== 5) buffer_v[0] = 0;

	//buffer_h and input_itemsets
	//buffer_h is for 
	for(int j = 1; j < max_cols; j++)
	{
		input_itemsets[j] = -j * penalty;
		if (version == 5)
		{
			buffer_h[j - 1] = -j * penalty;
		}
	}
	
	//Our code for output & deugging
	if (wo) 
	{
		result_file.open(ofile_name.c_str());
		result_file << "max_cols/rows: " << max_cols - 1 << ", penalty: " << penalty << std::endl;
		// this is just for debugging purposes
//#ifdef BENCH_PRINT
		//print input_itemsets[]
		result_file << "input_itemsets[]: " << std::endl;
		for (int i = 0; i < max_rows; i++)
		{
			for (int j = 0; j < max_cols; j++)
				result_file <<input_itemsets[i * max_cols + j] << " ";

			result_file << std::endl;
		}
		result_file << std::endl;
		
		//print reference[]
		result_file << "reference[]: " << std::endl;
		for (int i = 0; i < num_rows; i++)
		{
			for (int j = 0; j < num_cols; j++)
				result_file <<reference[i * num_cols + j] << " ";
 
			result_file << std::endl;
		}
		result_file << std::endl;	

//#endif
	}
}

// move constructor, currently not in use
//NWVars::NWVars(NWVars&& nw) :
//{
//std::cout << "calling move constructor" << std::endl;
//}


NWVars& NWVars::operator=(NWVars&& nw)
{
	max_rows = nw.max_rows;
	max_cols = nw.max_cols;
	num_cols = nw.num_cols,
	num_rows = nw.num_rows,
	penalty = nw.penalty;
	version = nw.version;
	data_size = nw.data_size;
	ref_size = nw.ref_size;

	if (reference)
		free(reference);
	reference = nw.reference;
	nw.reference = nullptr;

	if (input_itemsets)
		free(input_itemsets);
	input_itemsets = nw.input_itemsets;
	nw.input_itemsets = nullptr;

	if (output_itemsets)
		free(output_itemsets);
	output_itemsets = nw.output_itemsets;
	nw.output_itemsets = nullptr;

	if (buffer_v)
		free(buffer_v);
	buffer_v = nw.buffer_v;
	nw.buffer_v = nullptr;

	if (buffer_h)
		free(buffer_h);
	buffer_h = nw.buffer_h;
	nw.buffer_h = nullptr;

	if (result_file)
		result_file.close();
	nw.result_file.close();

	ofile_name = nw.ofile_name;
	result_file.open(ofile_name, std::ios::out | std::ios::app);
	nw.write_out = false;

	return *this;
}

// destructor
NWVars::~NWVars()
{
	
	if (reference)
	{
		std::cout << "reference = " << reference << std::endl;
		std::cout << "Freeing reference" << std::endl;
		free(reference);
		reference = nullptr;
		std::cout << "Freed reference" << std::endl;
	}
		
	if (input_itemsets)
	{
		std::cout << "input_itemsets = " << input_itemsets << std::endl;
		std::cout << "Freeing input_itemsets" << std::endl;
		free(input_itemsets);
		input_itemsets = nullptr;
		std::cout << "Freed input_itemsets" << std::endl;
	}

	if (output_itemsets)
	{
		std::cout << "output_itemsets = " << output_itemsets << std::endl;
		std::cout << "Freeing output_itemsets" << std::endl;
		free(output_itemsets);
		output_itemsets = nullptr;
		std::cout << "Freed output_itemsets" << std::endl;
	}
	
	//buffers
	if (buffer_v)
	{
		std::cout << "buffer_v = " << buffer_v << std::endl;
		std::cout << "Freeing buffer_v" << std::endl;
		free(buffer_v);
		buffer_v = nullptr;
		std::cout << "Freed buffer_v" << std::endl;
	}

	if (buffer_h)
	{
		std::cout << "buffer_h = " << buffer_h << std::endl;
		std::cout << "Freeing buffer_h" << std::endl;
		free(buffer_h);
		buffer_h = nullptr;
		std::cout << "Freed buffer_h" << std::endl;
	}


	if (result_file)
		result_file.close();
}

void* NWVars::AlignedMalloc(size_t size)
{
	void *temp_ptr;
	if ( posix_memalign (&temp_ptr, 4096, size) )
	{
		fprintf(stderr, "Aligned Malloc failed due to insufficient memory.\n");
		exit(-1);
	}
	return temp_ptr;
}

