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

public:
	AudioUnit			mAUNetSend;			///< the actual plugin
	AudioUnit			mAUNetReceive;		///< the actual plugin
	AudioBufferList*	inputBufferList;
	AudioBufferList*	outputBufferList;
	AudioTimeStamp		timeStamp;

	TTUInt16			mNumInputChannels;
	TTUInt16			mNumOutputChannels;
	TTUInt16			mVectorSize;			///< framesPerBuffer
	TTUInt32			mSampleRate;
	TTListPtr			callbackObservers;
	TTSymbolPtr			mInputDevice;
	TTSymbolPtr			mOutputDevice;
	TTInt16				inputDeviceIndex;
	TTInt16				outputDeviceIndex;
	TTBoolean			isRunning;
	TTAudioSignalPtr	inputBuffer;
	TTAudioSignalPtr	outputBuffer;

private:		
	void loadPlugins();
	TTErr loadPlugin(ComponentDescription& searchDesc, AudioUnit& plug);

	TTErr setNumInputChannels(const TTValue& v);
	TTErr setNumOutputChannels(const TTValue& v);
	
protected:
	TTErr initStream();
//	TTErr start();
//	TTErr stop();
//	TTErr getCpuLoad(TTValue& returnedValue);
	TTErr getAvailableInputDevices(TTValue& returnedDeviceNames);
	TTErr getAvailableOutputDevices(TTValue& returnedDeviceNames);

	// Attribute Accessors
//	TTErr setInputDevice(TTValue& newDeviceName);
//	TTErr setOutputDevice(TTValue& newDeviceName);
//	TTErr setVectorSize(TTValue& newVectorSize);
//	TTErr setSampleRate(TTValue& newSampleRate);
	
//	TTErr addCallbackObserver(const TTValue& objectToReceiveNotifications);
//	TTErr removeCallbackObserver(const TTValue& objectCurrentlyReceivingNotifications);
	
	/**	This is called repeatedly by PortAudio every time a new vector of audio is needed.	*/
//	TTInt32 callback(const TTFloat32*					input, 
//					 TTFloat32*							output, 
//					 TTUInt32							frameCount, 
//					 const PaStreamCallbackTimeInfo*	timeInfo, 
//					 PaStreamCallbackFlags				statusFlags);
	
};

typedef AUNetEngine* AUNetEnginePtr;


#endif // __TT_NETENGINE_H__
