/*
 * Copyright 2024 Thomas Kuehne - https://github.com/ThomasKuehne
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following
 * conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <libusb-1.0/libusb.h>

const char * decode_descriptor_type(uint8_t type){
	switch(type){
		case LIBUSB_DT_DEVICE: return "DT_DEVICE";
		case LIBUSB_DT_CONFIG: return "DT_CONFIG";
		case LIBUSB_DT_STRING: return "DT_STRING";
		case LIBUSB_DT_INTERFACE: return "DT_INTERFACE";
		case LIBUSB_DT_ENDPOINT: return "DT_ENDPOINT";
		case LIBUSB_DT_BOS: return "DT_BOS";
		case LIBUSB_DT_DEVICE_CAPABILITY: return "DT_DEVICE_CAPABILITY";
		case LIBUSB_DT_HID: return "DT_HID";
		case LIBUSB_DT_REPORT: return "DT_REPORT";
		case LIBUSB_DT_PHYSICAL: return "DT_PHYSICAL";
		case LIBUSB_DT_HUB: return "DT_HUB";
		case LIBUSB_DT_SUPERSPEED_HUB: return "DT_SUPERSPEED_HUB";
		case LIBUSB_DT_SS_ENDPOINT_COMPANION: return "DT_SS_ENDPOINT_COMPANION";
		default: return "unknown descriptor";
	}
}

const char * decode_class(uint8_t class){
	switch(class){
		case LIBUSB_CLASS_AUDIO: return "CLASS_AUDIO";
		case LIBUSB_CLASS_COMM: return "CLASS_COMM";
		case LIBUSB_CLASS_HID: return "CLASS_HID";
		case LIBUSB_CLASS_PHYSICAL: return "CLASS_PHYSICAL";
		case LIBUSB_CLASS_IMAGE: return "CLASS_IMAGE";
		case LIBUSB_CLASS_PRINTER: return "CLAS_PRINTER";
		case LIBUSB_CLASS_MASS_STORAGE: return "CLASS_MASS_STORAGE";
		case LIBUSB_CLASS_HUB: return "CLASS_HUB";
		case LIBUSB_CLASS_DATA: return "CLAS_DATA";
		case LIBUSB_CLASS_SMART_CARD: return "CLASS_SMART_CARD";
		case LIBUSB_CLASS_CONTENT_SECURITY: return "CLASS_CONTENT_SECURITY";
		case LIBUSB_CLASS_VIDEO: return "CLASS_VIDEO";
		case LIBUSB_CLASS_PERSONAL_HEALTHCARE: return "CLASS_PERSONAL_HEALTHCARE";
		case LIBUSB_CLASS_DIAGNOSTIC_DEVICE: return "CLASS_DIAGNOSTIC_DEVICE";
		case LIBUSB_CLASS_WIRELESS: return "CLASS_WIRELESS";
		case LIBUSB_CLASS_MISCELLANEOUS: return "CLASS_MISCELLANEOUS";
		case LIBUSB_CLASS_APPLICATION: return "CLASS_APPLICATION";
		case LIBUSB_CLASS_VENDOR_SPEC: return "CLASS_VENDOR_SPEC";
		default: return "unknown class";
	}
}

int analyze_usb_descriptor(const uint8_t * data, int max, uint8_t requested, uint8_t target_interface){
	int hid_wDescriptorLength = -1;
	bool our_interface = (data[1] == LIBUSB_DT_HID);
	int16_t actual_interface;

	printf("\n      === requested 0x%02x %s got %i bytes ===\n", requested, decode_descriptor_type(requested), max);
	
	if(max < 1) return hid_wDescriptorLength;

	for(int i = 0; i < max; i += data[i]){
		int len = data[i];
		if(len == 0) return hid_wDescriptorLength;

		uint8_t type = data[i+1];
		printf("      [%i] >> type 0x%02x %s with length %i\n", i, type, decode_descriptor_type(type), len);

		if (type == LIBUSB_DT_INTERFACE){
			actual_interface = data[i+2];
			our_interface = (target_interface == actual_interface);
			printf("%s       bInterfaceNumber = %i\n", our_interface ? ">" : " ", actual_interface);
		}

		if(type != LIBUSB_DT_HID) continue;

		uint8_t bNumDescriptors = data[i+5];
		if(bNumDescriptors != 1){
			printf("        bNumDescriptors = %i - BAD must be 1\n", bNumDescriptors);
		}
		
		for(int desc = 0; desc < bNumDescriptors; desc++){
			int offset = i + 6 + 3 * desc;
			type = data[offset];
			if(type != LIBUSB_DT_REPORT){
				printf("        bDescriptorType = 0x%02x %s - BAD must be 0x%02x\n", type, decode_descriptor_type(type), LIBUSB_DT_REPORT);
				continue;
			}

			uint16_t len = data[offset + 1] | (data[offset + 2] << 8);
			printf("%s       wDescriptorLength = %i\n",our_interface ? ">" : " ", len);
			if(our_interface){
				hid_wDescriptorLength = len;
			}
		}
	}
	return hid_wDescriptorLength;
}

int main(int argc, char** argv){
	uint8_t buffer[3072];
	struct libusb_context * ctx = NULL;
	struct libusb_device ** list;
	int ret;
	struct libusb_device_descriptor desc;

	ret = libusb_init(&ctx);
	if( ret < LIBUSB_SUCCESS){
		printf("libusb_init %i %s\n", ret, libusb_strerror((enum libusb_error) ret));
		return -1;
	}

	ret = libusb_get_device_list(ctx, &list); 
	if( ret < LIBUSB_SUCCESS){
		printf("libusb_get_device_list %i %s\n", ret, libusb_strerror((enum libusb_error) ret));
		libusb_exit(ctx);
		return -1;
	}

	ssize_t devices = ret;
	for(ssize_t devI = 0; devI < devices; devI++){
		struct libusb_config_descriptor * config;
		
		libusb_device * dev = list[devI];
		libusb_get_device_descriptor(dev, &desc);
		libusb_get_active_config_descriptor(dev, &config);
		printf("device %04x:%04x with %i interfaces\n", desc.idVendor, desc.idProduct, config->bNumInterfaces);
		
		for(uint8_t ifI = 0; ifI < config->bNumInterfaces; ifI++){
			for(int altI = 0; altI < config->interface[ifI].num_altsetting; altI++){
				const struct libusb_interface_descriptor idf = config->interface[ifI].altsetting[altI];
				printf("  interface %i alternate %i is class %s\n", idf.bInterfaceNumber, altI, decode_class(idf.bInterfaceClass));
				
				if(idf.bInterfaceClass != LIBUSB_CLASS_HID){
					continue;
				}

				// connct to USB device
				libusb_device_handle * dev_handle;
				ret = libusb_open(dev, &dev_handle);
				if(ret != LIBUSB_SUCCESS){
				       printf("      libusb_open %i %s\n", ret, libusb_strerror(ret));
				       continue;
				} 
				
				libusb_set_auto_detach_kernel_driver(dev_handle, 1);

		                ret = libusb_kernel_driver_active(dev_handle, idf.bInterfaceNumber);
		                if (ret < LIBUSB_SUCCESS){
					printf("   libusb_kernel_driver_active %i %s\n", ret, libusb_strerror((enum libusb_error) ret));
					continue;
				}
				
				ret = libusb_claim_interface(dev_handle, idf.bInterfaceNumber);
				if (ret != LIBUSB_SUCCESS) {
					printf("    libusb_claim_interface %i %s\n", ret, libusb_strerror((enum libusb_error) ret));
					continue;
				}

				ssize_t from_CONFIG = -2;
				ssize_t from_HID = -2;
				ssize_t from_actual = -2;
				

				// ask for USB configuration descriptor: USBD_HID_CfgDesc
				memset(buffer, 0, sizeof(buffer));
				ret = libusb_control_transfer(dev_handle, 0x80, 0x06, 0x0200, 0, buffer, sizeof(buffer), 1000);
				from_CONFIG = analyze_usb_descriptor(buffer, ret, LIBUSB_DT_CONFIG, idf.bInterfaceNumber);
					
				// ask for HID descriptor: USBD_HID_Desc 
				memset(buffer, 0, sizeof(buffer));
				ret = libusb_control_transfer(dev_handle, 0x81, 0x06, 0x2100, idf.bInterfaceNumber, buffer, sizeof(buffer), 1000);
				from_HID = analyze_usb_descriptor(buffer, ret, LIBUSB_DT_HID, idf.bInterfaceNumber);
				
				// ask for actual HID report descriptor: usbReportDesc
				memset(buffer, 0, sizeof(buffer));
				ret = libusb_control_transfer(dev_handle, 0x81, 0x06, 0x2200, idf.bInterfaceNumber, buffer, sizeof(buffer), 1000);
				from_actual = ret;
				printf("\n      === requested 0x%02x %s got %i bytes ===\n     ", LIBUSB_DT_REPORT, decode_descriptor_type(LIBUSB_DT_REPORT), ret);
				for(int i = 0; i < ret; i++){
					printf(" %02x", buffer[i]);
				}
				printf("\n");

				// sanity checks
				if(0 < from_CONFIG || 0 < from_HID || 0 < from_actual){
					printf("\n      === result for device %04x:%04x interface %i ===\n", desc.idVendor, desc.idProduct, idf.bInterfaceNumber);
					printf(">     %s - %zi vs %zi (configuration descriptor vs actual HID report)\n", from_CONFIG == from_actual ? "OK " : "BAD", from_CONFIG, from_actual);
					printf(">     %s - %zi vs %zi (configuration descriptor vs HID descriptor)\n", from_CONFIG == from_HID ? "OK " : "BAD", from_CONFIG, from_HID);
					printf("\n");
				}

				libusb_release_interface(dev_handle, idf.bInterfaceNumber);
				libusb_close(dev_handle);
			}
		}
		printf("\n");
	}
	
	libusb_free_device_list(list, 1);

	libusb_exit(ctx);

	return 0;
}
