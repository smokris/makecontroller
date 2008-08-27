/**
 * @file qextserialenumerator.cpp
 * @author Michał Policht
 * @see QextSerialEnumerator
 */
 
#include "qextserialenumerator.h"


#ifdef _TTY_WIN_
#include <QRegExp>
#include <objbase.h>
#include <initguid.h>
	//this is serial port GUID
	#ifndef GUID_CLASS_COMPORT
		//DEFINE_GUID(GUID_CLASS_COMPORT, 0x86e0d1e0L, 0x8089, 0x11d0, 0x9c, 0xe4, 0x08, 0x00, 0x3e, 0x30, 0x1f, 0x73);
        // use more Make Controller specific guid
		DEFINE_GUID(GUID_CLASS_COMPORT, 0x4D36E978, 0xE325, 0x11CE, 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 );
	#endif

	/* Gordon Schumacher's macros for TCHAR -> QString conversions and vice versa */	
	#ifdef UNICODE
		#define QStringToTCHAR(x)     (wchar_t*) x.utf16()
		#define PQStringToTCHAR(x)    (wchar_t*) x->utf16()
		#define TCHARToQString(x)     QString::fromUtf16((ushort*)(x))
		#define TCHARToQStringN(x,y)  QString::fromUtf16((ushort*)(x),(y))
	#else
		#define QStringToTCHAR(x)     x.local8Bit().constData()
		#define PQStringToTCHAR(x)    x->local8Bit().constData()
		#define TCHARToQString(x)     QString::fromLocal8Bit((x))
		#define TCHARToQStringN(x,y)  QString::fromLocal8Bit((x),(y))
	#endif /*UNICODE*/


	//static
	QString QextSerialEnumerator::getRegKeyValue(HKEY key, LPCTSTR property)
	{
		DWORD size = 0;
		RegQueryValueEx(key, property, NULL, NULL, NULL, & size);
		BYTE * buff = new BYTE[size];
		if (RegQueryValueEx(key, property, NULL, NULL, buff, & size) == ERROR_SUCCESS) {
			return TCHARToQStringN(buff, size);
			delete [] buff;
		} else {
			qWarning("QextSerialEnumerator::getRegKeyValue: can not obtain value from registry");
			delete [] buff;
			return QString();
		}
	}
	
	//static
	QString QextSerialEnumerator::getDeviceProperty(HDEVINFO devInfo, PSP_DEVINFO_DATA devData, DWORD property)
	{
		DWORD buffSize = 0;
		SetupDiGetDeviceRegistryProperty(devInfo, devData, property, NULL, NULL, 0, & buffSize);
		BYTE * buff = new BYTE[buffSize];
		if (!SetupDiGetDeviceRegistryProperty(devInfo, devData, property, NULL, buff, buffSize, NULL))
			qCritical("Can not obtain property: %ld from registry", property); 
		QString result = TCHARToQString(buff);
		delete [] buff;
		return result;
	}

	//static
	void QextSerialEnumerator::setupAPIScan(QList<QextPortInfo> & infoList)
	{
		HDEVINFO devInfo = INVALID_HANDLE_VALUE;
		GUID * guidDev = (GUID *) & GUID_CLASS_COMPORT;

		devInfo = SetupDiGetClassDevs(guidDev, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
		if(devInfo == INVALID_HANDLE_VALUE) {
			qCritical("SetupDiGetClassDevs failed. Error code: %ld", GetLastError());
			return;
		}

		//enumerate the devices
		bool ok = true;
		SP_DEVICE_INTERFACE_DATA ifcData;
		ifcData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		SP_DEVICE_INTERFACE_DETAIL_DATA * detData = NULL;
		DWORD detDataSize = 0;
		DWORD oldDetDataSize = 0;
		
		for (DWORD i = 0; ok; i++) {
			ok = SetupDiEnumDeviceInterfaces(devInfo, NULL, guidDev, i, &ifcData);
			if (ok) {
				SP_DEVINFO_DATA devData = {sizeof(SP_DEVINFO_DATA)};
				//check for required detData size
				SetupDiGetDeviceInterfaceDetail(devInfo, & ifcData, NULL, 0, & detDataSize, & devData);
				//if larger than old detData size then reallocate the buffer
				if (detDataSize > oldDetDataSize) {
					delete [] detData;
					detData = (SP_DEVICE_INTERFACE_DETAIL_DATA *) new char[detDataSize];
					detData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
					oldDetDataSize = detDataSize;
				}
				//check the details
				if (SetupDiGetDeviceInterfaceDetail(devInfo, & ifcData, detData, detDataSize, 
													NULL, & devData)) {
					// Got a device. Get the details.
					QextPortInfo info;
					info.friendName = getDeviceProperty(devInfo, & devData, SPDRP_FRIENDLYNAME);
					info.physName = getDeviceProperty(devInfo, & devData, SPDRP_PHYSICAL_DEVICE_OBJECT_NAME);
					info.enumName = getDeviceProperty(devInfo, & devData, SPDRP_ENUMERATOR_NAME);
					//anyway, to get the port name we must still open registry directly :( ??? 
					//Eh...			
					HKEY devKey = SetupDiOpenDevRegKey(devInfo, & devData, DICS_FLAG_GLOBAL, 0,
														DIREG_DEV, KEY_READ);
					info.portName = getRegKeyValue(devKey, TEXT("PortName"));
					
					// MakingThings
					QRegExp rx("COM(\\d+)");
					if(info.portName.contains(rx))
                    {
                      int portnum = rx.cap(1).toInt();
                      if(portnum > 9)
                        info.portName.prepend("\\\\.\\"); // COM ports greater than 9 need \\.\ prepended
                    }
                    // end MakingThings
					
					RegCloseKey(devKey);
					infoList.append(info);
				} else {
					qCritical("SetupDiGetDeviceInterfaceDetail failed. Error code: %ld", GetLastError());
					delete [] detData;
					return;
				}
			} else {
				if (GetLastError() != ERROR_NO_MORE_ITEMS) {
					delete [] detData;
					qCritical("SetupDiEnumDeviceInterfaces failed. Error code: %ld", GetLastError());
					return;
				}
			}
		}
		delete [] detData;
	}

#endif /*_TTY_WIN_*/

#ifdef _TTY_POSIX_

#ifdef Q_WS_MAC
// OS X version
#include <IOKit/IOKitLib.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <CoreFoundation/CFNumber.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>
#include <sys/param.h>
#include <IOKit/IOBSD.h>

// static
void QextSerialEnumerator::scanPortsOSX(QList<QextPortInfo> & infoList)
{
  io_iterator_t serialPortIterator = 0;
  io_object_t modemService;
  kern_return_t kernResult = KERN_FAILURE;
  CFMutableDictionaryRef matchingDictionary;
  
  matchingDictionary = IOServiceMatching(kIOSerialBSDServiceValue);
  if (matchingDictionary == NULL)
    return qWarning("IOServiceMatching returned a NULL dictionary.");
  CFDictionaryAddValue(matchingDictionary, CFSTR(kIOSerialBSDTypeKey), CFSTR(kIOSerialBSDAllTypes));
  
  // then create the iterator with all the matching devices
  if( IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDictionary, &serialPortIterator) != KERN_SUCCESS )
    return qCritical("IOServiceGetMatchingServices failed, returned %d", kernResult);
  
  // Iterate through all modems found.
  while((modemService = IOIteratorNext(serialPortIterator)))
  {
    CFTypeRef bsdPathAsCFString = NULL;
    CFTypeRef productNameAsCFString = NULL;
    CFTypeRef vendorIdAsCFNumber = NULL;
    CFTypeRef productIdAsCFNumber = NULL;
    // check the name of the modem's callout device
    bsdPathAsCFString = IORegistryEntryCreateCFProperty(modemService, CFSTR(kIOCalloutDeviceKey), kCFAllocatorDefault, 0);

    // wander up the hierarchy until we find the level that can give us the vendor/product IDs and the product name, if available
    io_registry_entry_t parent;
    kernResult = IORegistryEntryGetParentEntry(modemService, kIOServicePlane, &parent);
    while( !kernResult && !vendorIdAsCFNumber && !productIdAsCFNumber )
    {
      if(!productNameAsCFString)
        productNameAsCFString = IORegistryEntrySearchCFProperty(parent, kIOServicePlane, CFSTR("Product Name"), kCFAllocatorDefault, 0);
      vendorIdAsCFNumber = IORegistryEntrySearchCFProperty(parent, kIOServicePlane, CFSTR(kUSBVendorID), kCFAllocatorDefault, 0);
      productIdAsCFNumber = IORegistryEntrySearchCFProperty(parent, kIOServicePlane, CFSTR(kUSBProductID), kCFAllocatorDefault, 0);
      io_registry_entry_t oldparent = parent;
      kernResult = IORegistryEntryGetParentEntry(parent, kIOServicePlane, &parent);
      IOObjectRelease(oldparent);
    }
    
    QextPortInfo info;
    info.vendorID = 0;
    info.productID = 0;
    
    io_string_t ioPathName;
    IORegistryEntryGetPath( modemService, kIOServicePlane, ioPathName );
    info.physName = ioPathName;
    
    if( bsdPathAsCFString )
    {   
      char path[MAXPATHLEN];
      if( CFStringGetCString((CFStringRef)bsdPathAsCFString, path, PATH_MAX, kCFStringEncodingUTF8) )
        info.portName = path;
      CFRelease(bsdPathAsCFString);
    }
    
    if(productNameAsCFString)
    {
      char productName[MAXPATHLEN];
      if( CFStringGetCString((CFStringRef)productNameAsCFString, productName, PATH_MAX, kCFStringEncodingUTF8) )
        info.friendName = productName;
      CFRelease(productNameAsCFString);
    }
    
    if(vendorIdAsCFNumber)
    {
      SInt32 vID;
      if(CFNumberGetValue((CFNumberRef)vendorIdAsCFNumber, kCFNumberSInt32Type, &vID))
        info.vendorID = vID;
      CFRelease(vendorIdAsCFNumber);
    }
    
    if(productIdAsCFNumber)
    {
      SInt32 pID;
      if(CFNumberGetValue((CFNumberRef)productIdAsCFNumber, kCFNumberSInt32Type, &pID))
        info.productID = pID;
      CFRelease(productIdAsCFNumber);
    }
    
    infoList.append(info);
    IOObjectRelease(modemService);
  }
  IOObjectRelease(serialPortIterator);
  
  getSamBaBoards( infoList );
}

void QextSerialEnumerator::getSamBaBoards( QList<QextPortInfo> & infoList )
{
  kern_return_t err;
  SInt32 idVendor = 0x03EB;
  SInt32 idProduct = 0x6124;
  CFNumberRef numberRef;
  mach_port_t masterPort = 0;
  CFMutableDictionaryRef matchingDictionary = 0;
  io_iterator_t iterator = 0;
  io_service_t usbDeviceRef;
  
  if( (err = IOMasterPort( MACH_PORT_NULL, &masterPort )) )
    return qWarning( "could not create master port, err = %08x\n", err );

  if( !(matchingDictionary = IOServiceMatching(kIOUSBDeviceClassName)) )
    return qWarning( "could not create matching dictionary\n" );
  
  if( !(matchingDictionary = IOServiceMatching(kIOUSBDeviceClassName)) )
    return qWarning( "could not create matching dictionary\n" );
  
  if( !(numberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &idVendor)) )
    return qWarning( "could not create CFNumberRef for vendor\n" );

  CFDictionaryAddValue( matchingDictionary, CFSTR(kUSBVendorID), numberRef);
  CFRelease( numberRef );
  numberRef = 0;

  if( !(numberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &idProduct)) )
    return qWarning( "could not create CFNumberRef for product\n" );

  CFDictionaryAddValue( matchingDictionary, CFSTR(kUSBProductID), numberRef);
  CFRelease( numberRef );
  numberRef = 0;
  
  err = IOServiceGetMatchingServices( masterPort, matchingDictionary, &iterator );
  
  while( (usbDeviceRef = IOIteratorNext( iterator )) )
  {
    QextPortInfo info;
    info.vendorID = idVendor;
    info.productID = idProduct;
    infoList.append(info);
  }
  IOObjectRelease(iterator);
}

#else /* Q_WS_MAC */

#include <unistd.h>
#include <hal/libhal.h>
// thanks to EBo for unix support
void QextSerialEnumerator::scanPortsNix(QList<QextPortInfo> & infoList)
{
	int i;
	int num_udis;
	char **udis;
	DBusError error;
	LibHalContext *hal_ctx;

	dbus_error_init (&error);	
	if ((hal_ctx = libhal_ctx_new ()) == NULL) {
		qWarning("error: libhal_ctx_new");
		LIBHAL_FREE_DBUS_ERROR (&error);
		return;
	}
	if (!libhal_ctx_set_dbus_connection (hal_ctx, 
           dbus_bus_get (DBUS_BUS_SYSTEM, &error))) {
		qWarning("error: libhal_ctx_set_dbus_connection: %s: %s", error.name, error.message);
		LIBHAL_FREE_DBUS_ERROR (&error);
		return;
	}
	if (!libhal_ctx_init (hal_ctx, &error)) {
		if (dbus_error_is_set(&error)) {
			qWarning("error: libhal_ctx_init: %s: %s", error.name, error.message);
			LIBHAL_FREE_DBUS_ERROR (&error);
		}
		qWarning("Could not initialise connection to hald.\n  
                 Normally this means the HAL daemon (hald) is not running or not ready.");
		return;
	}


	udis = libhal_find_device_by_capability (hal_ctx, "serial",
             &num_udis, &error);

	if (dbus_error_is_set (&error)) {
		qWarning("libahl error: %s: %s", error.name, error.message);
		LIBHAL_FREE_DBUS_ERROR (&error);
		return;
	}

  // spin through and find the device names...
	for (i = 0; i < num_udis; i++)
  {
    //printf ("udis = %s\n", udis[i]);

    QextPortInfo info;
    char * iparent;

    // get the device file name and the product name...
    info.portName = libhal_device_get_property_string (hal_ctx,
                      udis[i],"linux.device_file",&error);

    info.friendName = libhal_device_get_property_string (hal_ctx, 
                       udis[i],"info.product",&error);

    // the vendor and product ID's cannot be obtained by the same
    // device entry as the serial device, but must be gotten for
    // the parent.
    iparent = libhal_device_get_property_string (hal_ctx,
    udis[i], "info.parent", &error);

    // get the vendor and product ID's from the parent
    info.vendorID  = libhal_device_get_property_int (hal_ctx,
                       iparent , "usb.vendor_id", &error);

    info.productID = libhal_device_get_property_int (hal_ctx, 
                       iparent,"usb.product_id",&error);

    // check for errors.
    if (dbus_error_is_set (&error))
    {
      qWarning("libhal error: %s: %s", error.name, error.message);
      LIBHAL_FREE_DBUS_ERROR (&error);
      return;
    }

    infoList.append(info);
	}

	libhal_free_string_array (udis);

	return;
}

#endif /* Q_WS_MAC */
#endif /* _TTY_POSIX_ */


//static
QList<QextPortInfo> QextSerialEnumerator::getPorts()
{
	QList<QextPortInfo> ports;

	#ifdef _TTY_WIN_
		OSVERSIONINFO vi;
		vi.dwOSVersionInfoSize = sizeof(vi);
		if (!::GetVersionEx(&vi)) {
			qCritical("Could not get OS version.");
			return ports;
		}
		// Handle windows 9x and NT4 specially
		if (vi.dwMajorVersion < 5) {
			qCritical("Enumeration for this version of Windows is not implemented yet");
/*			if (vi.dwPlatformId == VER_PLATFORM_WIN32_NT)
				EnumPortsWNt4(ports);
			else
				EnumPortsW9x(ports);*/
		} else	//w2k or later
			setupAPIScan(ports);
	#endif /*_TTY_WIN_*/
	#ifdef _TTY_POSIX_
  #ifdef Q_WS_MAC
    scanPortsOSX(ports); 
  #else /* Q_WS_MAC */
    scanPortsNix(ports);
  #endif /* Q_WS_MAC */
	#endif /*_TTY_POSIX_*/
	
	return ports;
}