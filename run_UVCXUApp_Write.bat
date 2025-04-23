@echo off

UVCXUApp --deviceName="USB Camera"^
 --xferBytes=4^
 --wdataValue=0x55AA7788^
 --guid_g1=0x01234567^
 --guid_g2=0x89AB^
 --guid_g3=0xCDEF^
 --guid_g4=0x1122^
 --guid_g5=0x334455667788
 
pause