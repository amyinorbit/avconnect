/*===--------------------------------------------------------------------------------------------===
 * enum_macos.c
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2025 Amy Parent. All rights reserved
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#include <serial/serial.h>
#include <stdio.h>
#include <string.h>

#include <CoreFoundation/CFNumber.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/usb/IOUSBLib.h>



CFTypeRef get_property(io_service_t device, CFStringRef key_name) {
    return IORegistryEntrySearchCFProperty(
        device, kIOServicePlane, key_name, NULL,
        kIORegistryIterateRecursively | kIORegistryIterateParents
    );
}

bool get_str_prop(io_service_t device, CFStringRef key_name, const char **out) {
    CFStringRef val = (CFStringRef)get_property(device, key_name);
    if(val) {
        CFStringEncoding encoding = CFStringGetSystemEncoding();
        const char* str = CFStringGetCStringPtr(val, encoding);
        
        if(str != NULL) {
            *out = str;
            return true;
        }
    }
    return false;
}

bool get_int_prop(io_service_t device, CFStringRef key_name, int *out) {
    CFNumberRef val = (CFNumberRef)get_property(device, key_name);
    if(val)
        return CFNumberGetValue(val, kCFNumberIntType, out);
    return false;
}

int serial_list_devices(serial_info_t *dev_info, int cap) {
    CFMutableDictionaryRef matching = IOServiceMatching(kIOSerialBSDServiceValue);
    if(!matching)
        return 0;
    
    io_iterator_t iter;
    if(IOServiceGetMatchingServices(kIOMasterPortDefault, matching, &iter) != KERN_SUCCESS)
        return 0;
    
    int count = 0;
    io_service_t device;
    while((device = IOIteratorNext(iter))) {
        int product_id = 0, vendor_id = 0;
        const char *product = NULL, *address = NULL;
        
        if(!get_str_prop(device, CFSTR(kIODialinDeviceKey), &address))
            continue;
        if(!get_int_prop(device, CFSTR(kUSBVendorID), &vendor_id))
            continue;
        if(!get_int_prop(device, CFSTR(kUSBProductID), &product_id))
            continue;
        
        if(!get_str_prop(device, CFSTR(kUSBProductString), &product))
            product = NULL;
        
        if(cap > 0 && count >= cap)
            break;
        
        if(cap > 0) {

            if(product == NULL) {
                const char *last_slash = strrchr(address, '/');
                product = last_slash ? last_slash + 1 : address;
            }
            
            serial_info_t *dev = &dev_info[count];
            dev->address = strdup(address);
            dev->name = strdup(product);
        }
        count += 1;
    }
    return count;
}
