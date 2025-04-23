# UVCXUApp

A UVC Extension Unit Example

```
Usage: UVCXUApp
Options:
 --deviceName STR             specify device name (e.g., USB Camera)
 --g1 HEX                XU GUID g1 32bit Hax value
 --g2 HEX                XU GUID g2 16bit Hax value
 --g3 HEX                XU GUID g3 16bit Hax value
 --g4 HEX                XU GUID g4 16bit Hax value
 --g5 HEX                XU GUID g5 48bit Hax value
 --wdataValue HEX             Write 64bit data to wdataByesArray
 --xferBytes Decimal          xfer Bytes (1 ~ 64)
 --controlSelectors Decimal   control Selectors (1 ~ 32)
 --help                       help message

Example USB Camera XU GUID {01234567-89AB-CDEF-1122-334455667788}:
 Write 4 Bytes
  UVCXUApp --deviceName="USB Camera"
   --controlSelectors=1
   --xferBytes=4
   --wdataValue=0x55AA7788
   --g1=0x01234567
   --g2=0x89AB
   --g3=0xCDEF
   --g4=0x1122
   --g5=0x334455667788
 Read 4 Bytes
  UVCXUApp --deviceName="USB Camera"
   --controlSelectors=1
   --xferBytes=4
   --g1=0x01234567
   --g2=0x89AB
   --g3=0xCDEF
   --g4=0x1122
   --g5=0x334455667788
```
