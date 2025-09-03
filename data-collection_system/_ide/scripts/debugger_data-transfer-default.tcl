# Usage with Vitis IDE:
# In Vitis IDE create a Single Application Debug launch configuration,
# change the debug type to 'Attach to running target' and provide this 
# tcl script in 'Execute Script' option.
# Path of this script: /home/neutrino/Desktop/temp/ad4134_fmcz/data-collection_system/_ide/scripts/debugger_data-transfer-default.tcl
# 
# 
# Usage with xsct:
# To debug using xsct, launch xsct and run below command
# source /home/neutrino/Desktop/temp/ad4134_fmcz/data-collection_system/_ide/scripts/debugger_data-transfer-default.tcl
# 
connect -url tcp:127.0.0.1:3121
targets -set -nocase -filter {name =~"APU*"}
rst -system
after 3000
targets -set -filter {jtag_cable_name =~ "Digilent Zed 210248BD1168" && level==0 && jtag_device_ctx=="jsn-Zed-210248BD1168-23727093-0"}
fpga -file /home/neutrino/Desktop/temp/ad4134_fmcz/data-transfer/_ide/bitstream/system_top.bit
targets -set -nocase -filter {name =~"APU*"}
loadhw -hw /home/neutrino/Desktop/temp/ad4134_fmcz/zedboard/export/zedboard/hw/system_top.xsa -mem-ranges [list {0x40000000 0xbfffffff}] -regs
configparams force-mem-access 1
targets -set -nocase -filter {name =~"APU*"}
source /home/neutrino/Desktop/temp/ad4134_fmcz/data-transfer/_ide/psinit/ps7_init.tcl
ps7_init
ps7_post_config
targets -set -nocase -filter {name =~ "*A9*#0"}
dow /home/neutrino/Desktop/temp/ad4134_fmcz/data-transfer/Debug/data-transfer.elf
configparams force-mem-access 0
targets -set -nocase -filter {name =~ "*A9*#0"}
con
