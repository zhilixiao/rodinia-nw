#ifndef NW_HPP
#define NW_HPP

// //from work_group_size.h 
// #ifndef BSIZE
// #ifdef RD_WG_SIZE_0_0
// 	#define BSIZE RD_WG_SIZE_0_0
// #elif defined(RD_WG_SIZE_0)
// 	#define BSIZE RD_WG_SIZE_0
// #elif defined(RD_WG_SIZE)
// 	#define BSIZE RD_WG_SIZE
// #else
// 	#define BSIZE 16
// #endif 
// #endif // BSIZE

// #ifndef PAR
// 	#define PAR 4
// #endif

//#include "./kernels/nw_common.h"
#include "./ckernel/nw_common.h"
#include <iostream>
#include <fstream>

//from main nw.c
#define LIMIT -999

//blosum62 scoring matrix, stands for block substitution matrix for comparing divergent sequences,
//BLOSUM62 matrix was developed by analyzing the frequencies of amino acid substitutions in clusters of related proteins
//Within each cluster, or block, the amino acid sequences were at least 62% identical when two proteins were aligned.
extern const int blosum62[24][24];

//from main nw.c
extern int maximum(int a, int b, int c);

typedef struct NWVars
{
public:
	//Program variables. Originally from nw.c's main()
	int max_rows;
	int max_cols;
	int num_cols;
	int num_rows;
	int penalty;
	int version; //needed for initialization
	//Vars parameters from nw.c's initilization section in main()
	//Calculated from max_rows, max_cols
	int data_size;
	int ref_size;
	int* reference;
	int* input_itemsets;
	int* output_itemsets;
	// for v5 and above?
	int *buffer_v;
	int *buffer_h;
	
	//for output and debugging?
	std::ofstream result_file;
	std::string ofile_name;
	//std::ofstream ofile;
	bool write_out;

	//constructors
	NWVars();
	NWVars(int max_r, int max_c, int penalty, int version, const char *outfile_name, bool wo); 
	// move constructor
	//NWVars(NWVars&& nw);
	// move assignment 
	NWVars& operator=(NWVars&& nw);
	// destructor
	~NWVars();
	//To replace the initilization of itemsets in nw.c's main()
	void* AlignedMalloc(size_t size);

}NWVars;
#endif //NW_HPP
