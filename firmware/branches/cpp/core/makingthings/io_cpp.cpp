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


/** \file io.c	
	Subsystem for manipulating the general purpose IO lines.
*/

/* Hardware specific headers */
#include "Board.h"
#include "AT91SAM7X256.h" 

#include "io_cpp.h"
#include "config.h"

#define IO_PIN_COUNT_ 64

Io::Io( int index, bool direction, IoPeripheral peripheral )
{
  io_pin = INVALID_PIN;
  if ( index < 0 || index > IO_PIN_COUNT_ )
    return;
  io_pin = index;
  setDirection( direction );
  setPeripheral( peripheral );
}

bool Io::setPin( int pin )
{
  if ( pin < 0 || pin > IO_PIN_COUNT_ )
    return false;
  io_pin = pin;
  return true;
}

int Io::getPin( )
{
  return ( io_pin == INVALID_PIN ) ? -1 : io_pin;
}

bool Io::getValue( )
{
  if ( io_pin == INVALID_PIN )
    return 0;
  
  int mask = 1 << ( io_pin & 0x1F );
  if ( io_pin < 32 )
    return ( ( AT91C_BASE_PIOA->PIO_PDSR & mask ) != 0 ) ? 1 : 0;
  else
    return ( ( AT91C_BASE_PIOB->PIO_PDSR & mask ) != 0 ) ? 1 : 0;
}

bool Io::setValue( bool onoff )
{
  if ( io_pin == INVALID_PIN )
    return 0;

  int mask = 1 << ( io_pin & 0x1F );
  if ( io_pin < 32 ) // port A
  {
    if ( !onoff )
      AT91C_BASE_PIOA->PIO_SODR = mask; // set it
    else
      AT91C_BASE_PIOA->PIO_CODR = mask; // clear
  }
  else // port B
  {
    if ( !onoff )
      AT91C_BASE_PIOB->PIO_SODR = mask; // set it
    else
      AT91C_BASE_PIOB->PIO_CODR = mask; // clear it
  }
  return true;
}

bool Io::getDirection( )
{
  if ( io_pin == INVALID_PIN )
    return 0;

  if ( io_pin < 32 ) // port A
    return ( AT91C_BASE_PIOA->PIO_OSR & ( 1 << io_pin ) ) != 0;
  else // port B
    return ( AT91C_BASE_PIOB->PIO_OSR & ( 1 << ( io_pin & 0x1F ) ) ) != 0;
}

bool Io::setDirection( bool output )
{
  if ( io_pin == INVALID_PIN )
    return 0;
  
  if( output ) // set as output
  {
    if ( io_pin < 32 ) // port A
      AT91C_BASE_PIOA->PIO_OER = 1 << io_pin;
    else // port B
      AT91C_BASE_PIOB->PIO_OER = 1 << ( io_pin & 0x1F );
  }
  else // set as input
  {
    if ( io_pin < 32 ) // port A
      AT91C_BASE_PIOA->PIO_ODR = 1 << io_pin;
    else // port B
      AT91C_BASE_PIOB->PIO_ODR = 1 << ( io_pin & 0x1F );
  }
  return true;
}

bool Io::getPullup( )
{
  if ( io_pin == INVALID_PIN )
    return 0;

  // The PullUp status register is inverted.
  if ( io_pin < 32 ) 
    return ( AT91C_BASE_PIOA->PIO_PPUSR & ( 1 << io_pin ) ) == 0;
  else
    return ( AT91C_BASE_PIOB->PIO_PPUSR & ( 1 << ( io_pin & 0x1F ) ) ) == 0;
}

bool Io::setPullup( bool enabled )
{
  if ( io_pin == INVALID_PIN )
    return 0;
    
  int mask = 1 << ( io_pin & 0x1F );
  if( enabled ) // turn it on
  {
    if ( io_pin < 32 )
        AT91C_BASE_PIOA->PIO_PPUER = mask;
    else
        AT91C_BASE_PIOB->PIO_PPUER = mask;
  }
  else // turn it off
  {
    if ( io_pin < 32 )
        AT91C_BASE_PIOA->PIO_PPUDR = mask;
    else
        AT91C_BASE_PIOB->PIO_PPUDR = mask;
  }
  return true;
}

bool Io::getFilter( )
{
  if ( io_pin == INVALID_PIN )
    return 0;
    
  if ( io_pin < 32 ) 
    return AT91C_BASE_PIOA->PIO_IFSR & ( 1 << io_pin );
  else
    return AT91C_BASE_PIOB->PIO_IFSR & ( 1 << ( io_pin & 0x1F ) );
}

bool Io::setFilter( bool enabled )
{
  if ( io_pin == INVALID_PIN )
    return 0;
  
  int mask = 1 << ( io_pin & 0x1F );
  if( enabled ) // turn it on
  {
    if ( io_pin < 32 )
        AT91C_BASE_PIOA->PIO_IFER = mask;
    else
        AT91C_BASE_PIOB->PIO_IFER = mask;
  }
  else // turn it off
  {
    if ( io_pin < 32 )
        AT91C_BASE_PIOA->PIO_IFDR = mask;
    else
        AT91C_BASE_PIOB->PIO_IFDR = mask;
  }
  return true;
}

int Io::getPeripheral( )
{
  if ( io_pin == INVALID_PIN )
    return 0;
    
  if ( io_pin < 32 ) 
    return ( AT91C_BASE_PIOA->PIO_PSR & ( 1 << io_pin ) ) != 0;
  else
    return ( AT91C_BASE_PIOB->PIO_PSR & ( 1 << ( io_pin & 0x1F ) ) ) != 0;
}

bool Io::setPeripheral( IoPeripheral periph )
{
  if ( io_pin == INVALID_PIN )
    return 0;
  
  int mask = 1 << ( io_pin & 0x1F );
  switch( periph )
  {
    case A:
      if ( io_pin < 32 )
          AT91C_BASE_PIOA->PIO_ASR = mask;
      else
          AT91C_BASE_PIOB->PIO_ASR = mask;
      break;
    case B:
      if ( io_pin < 32 )
          AT91C_BASE_PIOA->PIO_BSR = mask;
      else
          AT91C_BASE_PIOB->PIO_BSR = mask;
      break;
    case GPIO:
      if ( io_pin < 32 )
          AT91C_BASE_PIOA->PIO_PER = mask;
      else
          AT91C_BASE_PIOB->PIO_PER = mask;
      break;
  }
  return true;
}

bool Io::releasePeripherals( )
{
  if ( io_pin == INVALID_PIN )
    return 0;
  
  // check each possible peripheral, and make sure it's cleared
  return false;
}



