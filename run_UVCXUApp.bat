@echo off

:: guidExtensionCode : {01234567-89AB-CDEF-1122-334455667788}
:: bmControls        : 0x01, 0x00, 0x00
::  D0               : 0  yes -  Vendor-Specific (Optional)
::  D1               : 1   no -  Vendor-Specific (Optional)

set deviceName="USB Camera"
set g1=0x01234567
set g2=0x89AB
set g3=0xCDEF
set g4=0x1122
set g5=0x334455667788
set controlSelectors=2

REM Write
UVCXUApp --deviceName=%deviceName%^
 --controlSelectors=%controlSelectors%^
 --xferBytes=4^
 --wdataValue=0x55AA7788^
 --g1=%g1% --g2=%g2% --g3=%g3% --g4=%g4% --g5=%g5%

REM Read
UVCXUApp --deviceName=%deviceName%^
 --controlSelectors=%controlSelectors%^
 --xferBytes=4^
 --g1=%g1% --g2=%g2% --g3=%g3% --g4=%g4% --g5=%g5%

pause