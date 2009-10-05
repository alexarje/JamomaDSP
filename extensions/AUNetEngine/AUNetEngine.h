/* 
  	AUNetEngine
  	Copyright © 2009, Timothy Place
  
  	License: This code is licensed under the terms of the GNU LGPL
  	http://www.gnu.org/licenses/lgpl.html 
 */

#ifndef __TT_NETENGINE_H__
#define __TT_NETENGINE_H__

#include "TTDSP.h"
#include "TTAudioSignal.h"
#include <CoreServices/CoreServices.h>
#include <AudioToolbox/AudioToolBox.h>
#include <AudioUnit/AudioUnit.h>


/**	AUNetEngine is a class that is used to drive realtime audio and scheduling operations in the Jamoma DSP environment.
	It is currently implemented as a wrapper around AUNetSend/AUNetReceive AudioUnit plug-ins.	
  	@see	TTAudioEngine																				 */

class AUNetEngine : public TTObject {
	TTCLASS_SETUP(AUNetEngine)
	
	AudioUnit			mAUNetSend;			///< the actual plugin
	AudioUnit			mAUNetReceive;		///< the actual plugin
//	AudioBufferList*	inputBufferList;
//	AudioBufferList*	outputBufferList;
	AudioTimeStamp		timeStamp;

	TTUInt16			numInputChannels;
	TTUInt16			numOutputChannels;
	TTUInt16			vectorSize;			///< framesPerBuffer
	TTUInt32			sampleRate;
//    PaStream*			stream;
	TTListPtr			callbackObservers;
	TTSymbolPtr			inputDevice;
	TTSymbolPtr			outputDevice;
//	const PaDeviceInfo*	inputDeviceInfo;
//	const PaDeviceInfo*	outputDeviceInfo;
	TTInt16				inputDeviceIndex;
	TTInt16				outputDeviceIndex;
	TTBoolean			isRunning;
	TTAudioSignalPtr	inputBuffer;
	TTAudioSignalPtr	outputBuffer;

private:		
	void loadPlugins();
	TTErr loadPlugin(const ComponentDescription& searchDesc, AudioUnit& plug);
	
protected:
	TTErr initStream();
	TTErr start();
	TTErr stop();
	TTErr getCpuLoad(TTValue& returnedValue);
	TTErr getAvailableInputDevices(TTValue& returnedDeviceNames);
	TTErr getAvailableOutputDevices(TTValue& returnedDeviceNames);

	// Attribute Accessors
	TTErr setinputDevice(TTValue& newDeviceName);
	TTErr setoutputDevice(TTValue& newDeviceName);
	TTErr setvectorSize(TTValue& newVectorSize);
	TTErr setsampleRate(TTValue& newSampleRate);
	
	TTErr addCallbackObserver(const TTValue& objectToReceiveNotifications);
	TTErr removeCallbackObserver(const TTValue& objectCurrentlyReceivingNotifications);
	
	/**	This is called repeatedly by PortAudio every time a new vector of audio is needed.	*/
//	TTInt32 callback(const TTFloat32*					input, 
//					 TTFloat32*							output, 
//					 TTUInt32							frameCount, 
//					 const PaStreamCallbackTimeInfo*	timeInfo, 
//					 PaStreamCallbackFlags				statusFlags);
	
};

typedef AUNetEngine* AUNetEnginePtr;


#endif // __TT_NETENGINE_H__

