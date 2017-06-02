# mini6410_none_os
# cpu pin 状态
OM[0] = 0  选择时钟源为外部晶振
OM[1] = 1
OM[2] = 1
OM[4:3] = 11 选择从 NAND boot 
OM[4：3] = 00 选择从 SDCARD boot
XSELNAND = 1   

GPN13-->BOOT_EINT13 = 0
GPN14-->BOOT_EINT14 = 0
GPN15-->BOOT_EINT15 = 0

DBGSEL = 0

# 板子上未贴的电阻
R14
R15

R29
R31
R32
R33
R34
R36
