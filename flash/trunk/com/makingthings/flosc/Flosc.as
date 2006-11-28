﻿/********************************************************************************* Copyright 2006 MakingThings Licensed under the Apache License,  Version 2.0 (the "License"); you may not use this file except in compliance  with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0  Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License. *********************************************************************************/import com.makingthings.flosc.OscMessage;import com.makingthings.flosc.OscBundle;import mx.utils.Delegate;/** 	The interface to the FLOSC server.	FLOSC (Flash-OSC) is a separate application that must be run simultaneously with your Flash application.	Because Flash can't connect to external devices by itself, FLOSC is a necessary intermediate step - 	it can connect to the Make Controller over the network, and then it formats communication with the board	into XML that can be fed in and out of Flash.  		FLOSC needs to know about a few values concerning the network configuration:	- <b>localAddress</b> is the IP address of the machine that FLOSC and your Flash application are running on.	- <b>localPort</b> is the port that your computer is listening on - the default is <b>10000</b>.	- <b>remoteAddress</b> is the address of the Make Controller that you're communicating with - the default is <b>192.168.0.200</b>	- <b>remotePort</b> is the port that the Make Controller is listening on for incoming messages - the default is <b>10000</b>.		It usually makes sense to initialize all these values in your onLoad() function.  However, you can always use the connect() function to make	a connection to FLOSC at any time.*/class com.makingthings.flosc.Flosc{	private var localAddress; // whatever the IP address of your machine is most likely to be	private var localPort;  //Make Controller default	private var remoteAddress;  //Make Controller default	private var remotePort;  //Make Controller default		private var mySocket:XMLSocket;	private var connected:Boolean; 	private var oscMessageHandler:Function; // the function to call back when an OSC message arrives		/**  * Constructor - initialize a bit of state. 	*/  function Flosc( )  {		connected = false;		localPort = 10000;		remoteAddress = "192.168.0.200";		remotePort = 10000;		oscMessageHandler = onOscMessage;  }		/**	* Query the local address that has been set.	* \return A string specifying the current address. 	*/	public function getLocalAddress( ):String	{		return localAddress;	}	/**	* Set the address of the computer that is connecting to FLOSC.	* \param addr A string specifying the address.	<h3>Example</h3>	\code	flosc.setLocalAddress( "192.168.0.215" );	\endcode 	*/	public function setLocalAddress( addr:String )	{		localAddress = addr;	}	/**	* Query the port that has been set.	* \return A number specifying the current port. 	*/	public function getLocalPort( ):Number	{		return localPort;	}	/**	* Set the port that the local computer should listen on.	* By default, this should be \b 10000 for use with the Make Controller.	* \param port A number specifying the port. 	*/	public function setLocalPort( port:Number )	{		localPort = port;	}		public function getRemoteAddress( ):String	{		return remoteAddress;	}	/**	* Set the address of the device you're sending to.	* The default address of the Make Controller is 192.168.0.200	* \param addr A string specifying the address.	<h3>Example</h3>	\code	flosc.setRemoteAddress( "192.168.0.200" );	\endcode 	*/	public function setRemoteAddress( addr:String )	{		remoteAddress = addr;	}	/**	* Query the port that you're sending messages on.	* \return A number specifying the current port. 	*/	public function getRemotePort( ):Number	{		return remotePort;	}	/**	* Set the port to send messages on.	* The default port that the Make Controller is listening on is 10000	* \param port A number specifying the port. 	*/	public function setRemotePort( port:Number )	{		remotePort = port;	}			// ********************************************************************	// OSC stuff	// ********************************************************************		// ** parse messages from an XML-encoded OSC packet	private function parseMessages( node )  //this is called when we get OSC packets back from FLOSC.	{		if (node.nodeName == "MESSAGE")		{			for (var child = node.firstChild; child != null; child=child.nextSibling)			{				if (child.nodeName == "ARGUMENT")				{					oscMessageHandler( node.attributes.NAME, child.attributes.VALUE );					//trace( "Address: " + node.attributes.NAME + ", Arg: " + child.attributes.VALUE );				}			}		}		else { // look recursively for a message node			for (var child = node.firstChild; child != null; child=child.nextSibling) {				parseMessages(child);			}		}	}		/**	* Get called back on this function with any incoming OSC messages.	You'll want to set this up with a call to \code setMessageHandler( onOscMessage ) \endcode	Then, override this method to deal with incoming messages however you like.  Usually, you'll	want to test the address of the incoming message to see if it's something you're interested in.	So if you want to listen for the trimpot (analogin 7), you might implement it like...		<h3>Example</h3>	\code 	setMessageHandler( onOscMessage );		onOscMessage = function( address, arg )	{		if( address == "/analogin/7/value" )			var trimpot = arg;	}	\endcode	Now the <b>trimpot</b> variable holds the value of the trimpot and you can do whatever you like with it. 	* \param address The address of the incoming OSC message as a string.	* \param argument The value included in the incoming OSC message. 	*/	public function onOscMessage( address:String, argument )	{			}		/**	* Set the function that will be called when an OSC message arrives from FLOSC.	* \param handler The name of the function that will be called back. 	*/	public function setMessageHandler( handler )	{		oscMessageHandler = handler;	}		/**	* Make a connection to the FLOSC server.	* This will connect using the current values of <b>localAddress</b> and <b>localPort</b>.   	*/	public function connect( )	{		mySocket = new XMLSocket();		mySocket.onConnect = Delegate.create( this, handleConnect );		//mySocket.onClose = handleClose;		mySocket.onXML = Delegate.create( this, handleIncoming );			if (!mySocket.connect(localAddress, localPort))			trace( "Can't create XML connection to FLOSC." );		//else			//trace( "FLOSC opened ok." );	}		/**	Disconnect from the FLOSC server.	This closes the XML connection to FLOSC. 	*/	public function disconnect( ) 	{		if( connected )		{			mySocket.close();			connected = false;		}		else			return;	}		// *** event handler for incoming XML-encoded OSC packets	private function handleIncoming (xmlIn)	{		// parse out the packet information		var e = xmlIn.firstChild;		if (e != null && e.nodeName == "OSCPACKET") {			//var packet = new OSCPacket(e.attributes.address, e.attributes.port, e.attributes.time, xmlIn);			//displayPacketHeaders(packet);			parseMessages( xmlIn );		}	}		// *** event handler to respond to successful connection attempt	private function handleConnect (succeeded)	{		if(succeeded)			this.connected = true;		else		{			trace( "Connection to FLOSC did not succeed." );			trace( "** Make sure it's running, and that you've set your localAddress properly." );		}	}		// display text information about an OSCPacket object	//function displayPacketHeaders(packet) {		//trace("** OSC Packet from " +packet.address+ ", port " +packet.port+ " for time " +packet.time);	//}		/**	* Send a message to the board.	This will send a message to a board at the current <b>remoteAddress</b> and <b>remotePort</b>.	If you need to specify the address and port for each message, use sendToAddress( ).	\param address The OSC address to send to, as type \b String.	\param arg The value to be sent. 	*/	public function send( address:String, arg )	{		sendToAddress( address, arg, remoteAddress, remotePort );	}		/**	* Send a message to the board, specifying the IP address and port. 	* \param name The OSC address to send to.	* \param arg The value to be sent.	* \param destAddr The IP address of the board you're sending to.	* \param destPort	 The port to send your message on. 	*/	public function sendToAddress( address:String, arg, destAddr:String, destPort:Number )	{		var xmlOut:XML = new XML();		var packetOut = createPacketOut( xmlOut, 0, destAddr, destPort );		var xmlMessage = createMessage( xmlOut, packetOut, address );		parseArgument( xmlOut, xmlMessage, arg );				// NOTE : to send more than one argument, just create		// more elements and appendChild them to the message.		// the same goes for multiple messages in a packet.		//packetOut.appendChild(xmlMessage);		xmlOut.appendChild(packetOut);			if( mySocket && this.connected )			mySocket.send(xmlOut);		//else			//trace( "Couldn't send message - not connected to FLOSC server." );			//trace( "connected? " + this.connected );	}		// used internally to prep an XML object to be sent out.	private function createPacketOut( xmlOut:XML, time:Number, destAddr:String, destPort:Number ):XMLNode	{		var packetOut = xmlOut.createElement("OSCPACKET");		packetOut.attributes.TIME = 0;		packetOut.attributes.PORT = destPort;		packetOut.attributes.ADDRESS = destAddr;				return packetOut;	}		// used internally to create a message element within the xmlOut object	private function createMessage( xmlOut:XML, packetOut:XMLNode, address:String ):XMLNode	{		var xmlMessage = xmlOut.createElement("MESSAGE");		xmlMessage.attributes.NAME = address;		packetOut.appendChild(xmlMessage);		return xmlMessage;	}		// used internally to determine the type of an argument, and append it to its corresponding message in the outgoing XML object.	private function parseArgument( xmlOut:XML, xmlMessage:XMLNode, arg )	{		var xmlArg = xmlOut.createElement("ARGUMENT");		var argInt = parseInt(arg);		if( isNaN( argInt ) )		{			// NOTE : the server expects all strings to be encoded with the escape function.			xmlArg.attributes.VALUE = escape(arg);			xmlArg.attributes.TYPE = "s";		} 		// how to check for float?		else 		{			xmlArg.attributes.VALUE = parseInt(arg);			xmlArg.attributes.TYPE="i";		}		xmlMessage.appendChild(xmlArg);		// how to check for blob?	}		/**	* Send an OscMessage to the board. 	This will send a message, of type OscMessage, to a board at the current <b>remoteAddress</b> and <b>remotePort</b>.	If you need to specify the address and port for each message, use sendMessageToAddress( ).	\param oscM The message, of type OscMessage, to be sent 	*/	public function sendMessage( oscM:OscMessage )	{		sendMessageToAddress( oscM, remoteAddress, remotePort );	}		/**	* Send an OscMessage to the board, specifying the destination IP address and port. 	* \param oscM The message, of type OscMessage, to be sent	* \param destAddr The IP address of the board you're sending to.	* \param destPort	 The port to send your message on. 	*/	public function sendMessageToAddress( oscM:OscMessage, destAddr:String, destPort:Number )	{		var xmlOut:XML = new XML();		var packetOut = createPacketOut( xmlOut, 0, destAddr, destPort );		var xmlMessage = createMessage( xmlOut, packetOut, oscM.address );		for( var i = 0; i < oscM.args.length; i++ )			parseArgument( xmlOut, xmlMessage, oscM.args[i] );		xmlOut.appendChild(packetOut);			if( mySocket && this.connected )			mySocket.send(xmlOut);		//else			//trace( "Couldn't send message - not connected to FLOSC server." );			//trace( "connected? " + this.connected );	}		/**	Send an OscBundle to the board. 	This will send an OscBundle to a board at the current <b>remoteAddress</b> and <b>remotePort</b>.	It's a good idea to send bundles when possible, in order to reduce the traffic between the board and Flash.	If you need to specify the address and port for each message, use sendBundleToAddress( ).	* \param oscB The OscBundle to be sent 	*/	public function sendBundle( oscB:Array )	{		sendBundleToAddress( oscB, remoteAddress, remotePort );	}		/**	* Send an OscBundle to the board, specifying the destination IP address and port. 	* \param oscB The OscBundle to be sent	* \param destAddr The IP address of the board you're sending to.	* \param destPort	 The port to send on. 	*/	public function sendBundleToAddress( oscB:Array, destAddr:String, destPort:Number )	{		var xmlOut:XML = new XML();		var packetOut = createPacketOut( xmlOut, 0, destAddr, destPort );		for( var i = 0; i < oscB.length; i++ )		{			if( oscB[i] instanceof OscMessage == false )			{				trace( "One of the members in the OscBundle was not an OscMessage...the entire bundle was not sent." );				return;			}			var oscM:OscMessage = oscB[i];			//trace( "Message " + i + ", address " + oscM.address + ", arg: " + oscM.args[0] );			var xmlMessage = createMessage( xmlOut, packetOut, oscM.address );			for( var j = 0; j < oscM.args.length; j++ )				parseArgument( xmlOut, xmlMessage, oscM.args[j] );		}		xmlOut.appendChild(packetOut);			if( mySocket && this.connected )			mySocket.send(xmlOut);		//else			//trace( "Couldn't send message - not connected to FLOSC server." );			//trace( "connected? " + this.connected );	}		/**	\todo Add setAddressHandler functionality - get called back on a given function when an OSC message with a particular address is received*/}