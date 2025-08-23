# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct /home/neutrino/work/ethernet-data-transfer/zedboard/platform.tcl
# 
# OR launch xsct and run below command.
# source /home/neutrino/work/ethernet-data-transfer/zedboard/platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform create -name {zedboard}\
-hw {/home/neutrino/Desktop/system_top.xsa}\
-proc {ps7_cortexa9_0} -os {freertos10_xilinx} -out {/home/neutrino/work/ethernet-data-transfer}

platform write
platform generate -domains 
platform generate
domain active {zynq_fsbl}
bsp reload
domain active {freertos10_xilinx_domain}
bsp reload
bsp setlib -name lwip213 -ver 1.0
bsp write
bsp reload
catch {bsp regenerate}
bsp config api_mode "SOCKET_API"
bsp write
bsp reload
catch {bsp regenerate}
bsp config use_axieth_on_zynq "0"
bsp config phy_link_speed "CONFIG_LINKSPEED1000"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains freertos10_xilinx_domain 
platform active {zedboard}
platform generate -domains 
platform clean
platform generate
domain active {zynq_fsbl}
bsp reload
bsp reload
domain active {freertos10_xilinx_domain}
bsp reload
bsp reload
platform active {zedboard}
platform generate -domains 
platform clean
platform clean
platform generate
platform generate -domains freertos10_xilinx_domain 
platform generate
platform active {zedboard}
domain active {zynq_fsbl}
bsp reload
bsp reload
domain active {freertos10_xilinx_domain}
bsp reload
bsp reload
platform generate -domains 
platform active {zedboard}
platform config -updatehw {/home/neutrino/Desktop/system_top.xsa}
platform generate
platform clean
platform generate
platform active {zedboard}
domain active {zynq_fsbl}
bsp reload
domain active {freertos10_xilinx_domain}
bsp reload
platform active {zedboard}
platform active {zedboard}
platform active {zedboard}
platform generate
domain active {zynq_fsbl}
bsp reload
domain active {freertos10_xilinx_domain}
bsp reload
domain active {zynq_fsbl}
bsp reload
domain active {freertos10_xilinx_domain}
bsp reload
platform active {zedboard}
domain create -name {daq_trigger} -os {standalone} -proc {ps7_cortexa9_1} -arch {32-bit} -display-name {daq_trigger} -desc {} -runtime {cpp}
platform generate -domains 
platform write
domain -report -json
bsp reload
bsp reload
platform generate -domains daq_trigger 
bsp reload
bsp config extra_compiler_flags "-mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -nostartfiles -g -Wall -Wextra -fno-tree-loop-distribute-patterns-DUSE_AMP=1"
bsp config extra_compiler_flags "-mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -nostartfiles -g -Wall -Wextra -fno-tree-loop-distribute-patterns -DUSE_AMP=1"
bsp write
bsp reload
catch {bsp regenerate}
domain active {freertos10_xilinx_domain}
bsp reload
bsp reload
domain active {daq_trigger}
bsp reload
platform generate -domains daq_trigger 
platform clean
platform generate
platform clean
platform generate
