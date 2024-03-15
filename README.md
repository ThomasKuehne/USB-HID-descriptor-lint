# USB-HID-descriptor-lint

This tool uses [libusb](https://github.com/libusb/libusb) to check all USB devices with interface class HID:

1. that the USB configuration descriptor can be retrieved
2. that the USB HID descriptor(s) can be retrieved
3. that the HID report descriptor(s) can be retrieved
4. that the specified length and actual lenth of the HID report descriptor(s) match

# compiling
```clang `pkg-config libusb-1.0 --libs --cflags` -o USB-HID-descriptor-lint USB-HID-descriptor-lint.c```

# executing
```sudo ./USB-HID-descriptor-lint```

# well behaved device
```
device 093a:2530 with 1 interfaces
  interface 0 alternate 0 is class CLASS_HID

      === requested 0x02 DT_CONFIG got 34 bytes ===
      [0] >> type 0x02 DT_CONFIG with length 9
      [9] >> type 0x04 DT_INTERFACE with length 9
>       bInterfaceNumber = 0
      [18] >> type 0x21 DT_HID with length 9
>       wDescriptorLength = 79
      [27] >> type 0x05 DT_ENDPOINT with length 7

      === requested 0x21 DT_HID got 9 bytes ===
      [0] >> type 0x21 DT_HID with length 9
>       wDescriptorLength = 79

      === requested 0x22 DT_REPORT got 79 bytes ===
      05 01 09 02 a1 01 09 01 a1 00 05 09 19 01 29 05 15 00 25 01 75 01 95 05 81 02 75 03 95 01 81 03 06 00 ff 09 40 95 02 75 08 15 81 25 7f 81 02 05 01 09 38 15 81 25 7f 75 08 95 01 81 06 09 30 09 31 16 01 80 26 ff 7f 75 10 95 02 81 06 c0 c0

      === result for device 093a:2530 interface 0 ===
>     OK  - 79 vs 79 (USBD_HID_CfgDesc vs actual)
>     OK  - 79 vs 79 (USBD_HID_CfgDesc vs USBD_HID_Desc)
```

# buggy device
```
device 1209:4f54 with 1 interfaces
  interface 0 alternate 0 is class CLASS_HID

      === requested 0x02 DT_CONFIG got 34 bytes ===
      [0] >> type 0x02 DT_CONFIG with length 9
      [9] >> type 0x04 DT_INTERFACE with length 9
>       bInterfaceNumber = 0
      [18] >> type 0x21 DT_HID with length 9
>       wDescriptorLength = 56
      [27] >> type 0x05 DT_ENDPOINT with length 7

      === requested 0x21 DT_HID got 9 bytes ===
      [0] >> type 0x21 DT_HID with length 9
>       wDescriptorLength = 74

      === requested 0x22 DT_REPORT got 56 bytes ===
      05 01 09 05 a1 01 a1 00 05 09 19 01 29 18 15 00 25 01 95 18 75 01 81 02 05 01 09 30 09 31 09 32 09 33 09 34 09 35 09 36 09 37 16 00 00 26 ff 07 75 10 95 08 81 02 c0 c0

      === result for device 1209:4f54 interface 0 ===
>     OK  - 56 vs 56 (USBD_HID_CfgDesc vs actual)
>     BAD - 56 vs 74 (USBD_HID_CfgDesc vs USBD_HID_Desc)
```
