# rodinia-nw
The paper can be found at HPEC 21

# Usage
  ## To compile the nw host program
    g++ -std=c++11 \
    nw_main_xilinx.cpp xlx_rodinia.cpp nw.cpp nw_opencl.cpp \
    -I${XILINX_XRT}/include/ \
    -L${XILINX_XRT}/lib/ -lOpenCL -lpthread -lrt -lstdc++ \

  ## To build the .xclbin file
  ### For opencl kernels
  Go to "kernel" folder, modify the KERNEL_SRC in Makefile
  
    make all
  
  ### For C++ kernels
  Go to "ckernel" folder, modify the KERNEL_SRC in Makefile
  
    make all
    
  ## To run the program
    ./a.out dim diff result.txt version kernel/ckernel
    
