/*
	config.h - Select which features & hardware you're using.
  MakingThings
*/

#ifndef CONFIG_H
#define CONFIG_H

#define FIRMWARE_NAME          "Heavy svn"
#define FIRMWARE_MAJOR_VERSION 2
#define FIRMWARE_MINOR_VERSION 0
#define FIRMWARE_BUILD_NUMBER  0

//----------------------------------------------------------------
//  Comment out the systems that you don't want to include in your build.
//----------------------------------------------------------------
#define MAKE_CTRL_USB     // enable the USB system
#define MAKE_CTRL_NETWORK // enable the Ethernet system
#define OSC               // enable the OSC system

//  The version of the MAKE Controller Board you're using.
#define CONTROLLER_VERSION  100    // valid options: 50, 90, 95, 100, 200

//  The version of the MAKE Application Board you're using.
#define APPBOARD_VERSION  100    // valid options: 50, 90, 95, 100, 200

#endif // CONFIG_H

