@echo off

UVCXUApp --deviceName="USB Camera"^
 --controlSelectors=2^
 --xferBytes=4^
 --g1=0x01234567^
 --g2=0x89AB^
 --g3=0xCDEF^
 --g4=0x1122^
 --g5=0x334455667788

pause