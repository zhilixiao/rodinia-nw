#ifndef NW_COMMON_H_
#define NW_COMMON_H_

#ifndef BSIZE
#ifdef RD_WG_SIZE_0_0
	#define BSIZE RD_WG_SIZE_0_0
#elif defined(RD_WG_SIZE_0)
	#define BSIZE RD_WG_SIZE_0
#elif defined(RD_WG_SIZE)
	#define BSIZE RD_WG_SIZE
#else
	#define BSIZE 512
	//#define BSIZE 16
#endif 
#endif // BSIZE

#ifndef PAR
	#define PAR 32
#endif

#endif 

