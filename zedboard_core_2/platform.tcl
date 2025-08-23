# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct /home/neutrino/work/ethernet-data-transfer/zedboard_core_2/platform.tcl
# 
# OR launch xsct and run below command.
# source /home/neutrino/work/ethernet-data-transfer/zedboard_core_2/platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform create -name {zedboard_core_2}\
-hw {/home/neutrino/Desktop/system_top.xsa}\
-proc {ps7_cortexa9_1} -os {standalone} -out {/home/neutrino/work/ethernet-data-transfer}

platform write
platform generate -domains 
platform active {zedboard_core_2}
platform generate
