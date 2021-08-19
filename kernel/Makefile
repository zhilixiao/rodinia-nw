#HWC stands for hardware compiler
HWC ?= v++
HW_TARGET ?= hw
KERNEL_NAME ?= nw_dynproc_kernel
KERNEL_SRC ?= nw_kernel_v5.cl
# Required flags
DESIGN_CFG ?= design.cfg
HWCFLAGS := --config $(DESIGN_CFG)
HWCFLAGS += -t ${HW_TARGET}
# Parameters (depends on your kernel)
#BSIZE = 256
#PAR = 4
#HWCFLAGS += -DBSIZE=${BSIZE}
#HWCFLAGS += -DPAR=${PAR}
$(info HWCFLAGS=$(HWCFLAGS))
#HW stuff
XO_FILE ?= nw_kernel_v5.xo
XCLBIN_FILE ?= nw_kernel_v5.xclbin

# COMPILE the kernel
$(XO_FILE): $(KERNEL_SRC)
	if [-z "${XILINX_XRT}" ];then echo "XILINX_XRT must be set";false;fi
	$(HWC) -c $(HWCFLAGS) -k $(KERNEL_NAME) $(KERNEL_SRC) -o $@

$(XCLBIN_FILE): $(KERNEL_SRC)
	if [-z "${XILINX_XRT}" ];then echo "XILINX_XRT must be set";false;fi
	$(HWC) -l $(HWCFLAGS) $(XO_FILE) -o $@

.PHONY: clean all
all: $(XO_FILE) $(XCLBIN_FILE)
clean:
	rm -rf $(XO_FILE) $(XCLBIN_FILE) _x *.log *.xclbin* *.xo* .Xil/