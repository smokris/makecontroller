/*********************************************************************************

 Copyright 2006-2008 MakingThings

 Licensed under the Apache License, 
 Version 2.0 (the "License"); you may not use this file except in compliance 
 with the License. You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0 
 
 Unless required by applicable law or agreed to in writing, software distributed
 under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied. See the License for
 the specific language governing permissions and limitations under the License.

*********************************************************************************/


#ifndef DIPSWITCH_H
#define DIPSWITCH_H

#include "spi.h"

/**
  The DIP Switch subsystem reads values in from the 8 position DIP Switch (0 - 255) on the Application Board.
  Mask off the appropriate bits in the value returned from the DIP switch to determine whether a particular channel is on or off.
  
  See the <a href="http://www.makingthings.com/documentation/tutorial/application-board-overview/user-interface">
  Application Board overview</a> for details.
  \ingroup Libraries
*/
class DipSwitch
{
  public:
    DipSwitch();
    ~DipSwitch();

    int value( );
    bool value( int channel );

  protected:
    static Spi* spi;
    static int refcount;
    
};

int DipSwitch_SetActive( int state );
int DipSwitch_GetActive( void );

int DipSwitch_GetValue( void );
bool DipSwitch_GetValueChannel( int channel );

bool DipSwitch_GetAutoSend( bool init );
void DipSwitch_SetAutoSend( int onoff );

/* DipSwitchOsc Interface */

const char* DipSwitchOsc_GetName( void );
int DipSwitchOsc_ReceiveMessage( int channel, char* message, int length );
int DipSwitchOsc_Async( int channel );

#endif
