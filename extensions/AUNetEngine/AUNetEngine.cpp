/* 
  	AUNetEngine
  	Copyright © 2009, Timothy Place
  
  	License: This code is licensed under the terms of the GNU LGPL
  	http://www.gnu.org/licenses/lgpl.html 
 */

#include "AUNetEngine.h"

#define thisTTClass		AUNetEngine
#define thisTTClassName	"AUNetEngine"
#define thisTTClassTags	"audio, engine"

/**	An AudioUnit render callback.  
	The AudioUnit will get its audio input by calling this function.  */
OSStatus AUNetEngineGetInputSamples(void*						inRefCon,
									AudioUnitRenderActionFlags*	ioActionFlags,
									const AudioTimeStamp*		inTimeStamp,
									UInt32						inBusNumber,
									UInt32						inNumberFrames,
									AudioBufferList*			ioData)
{
	AUNetEngine* engine = (AUNetEngine*)inRefCon;

	for(TTUInt16 channel=0; channel < ioData->mNumberBuffers; channel++)
		memcpy(ioData->mBuffers[channel].mData, engine->inputBufferList->mBuffers[channel].mData, sizeof(TTFloat32) * inNumberFrames);
	return noErr;
}


TT_OBJECT_CONSTRUCTOR_EXPORT,
	mAUNetSend(NULL),
	mAUNetReceive(NULL),
	inputBufferList(NULL), 
	outputBufferList(NULL),	
	mNumInputChannels(2),
	mNumOutputChannels(2),
	mVectorSize(64),
	mSampleRate(44100),
	mInputDevice(NULL),
	mOutputDevice(NULL),
	isRunning(false),
	inputBuffer(NULL),
	outputBuffer(NULL)
{
	timeStamp.mSampleTime = 0;
	timeStamp.mFlags = kAudioTimeStampSampleTimeValid;	
	callbackObservers = new TTList;
	callbackObservers->setThreadProtection(NO);	// ... because we make calls into this at every audio vector calculation ...

	TTObjectInstantiate(kTTSym_audiosignal, &inputBuffer, 1);
	TTObjectInstantiate(kTTSym_audiosignal, &outputBuffer, 1);
	
	// numChannels should be readonly -- how do we do that?
	addAttributeWithSetter(NumInputChannels,	kTypeUInt16);
	addAttributeWithSetter(NumOutputChannels,	kTypeUInt16);	
//	addAttributeWithSetter(VectorSize,			kTypeUInt16);
//	addAttributeWithSetter(SampleRate,			kTypeUInt32);
//	addAttributeWithSetter(InputDevice,			kTypeSymbol);
//	addAttributeWithSetter(OutputDevice,		kTypeSymbol);
	
//	addMessage(start);
//	addMessage(stop);
	//registerMessageWithArgument(getCpuLoad);
//	addMessageWithArgument(addCallbackObserver);
//	addMessageWithArgument(removeCallbackObserver);

	// Set defaults
	setAttributeValue(TT("inputDevice"), TT("default"));
	setAttributeValue(TT("outputDevice"), TT("default"));
}


AUNetEngine::~AUNetEngine()
{
	if (mAUNetSend) {
		AudioUnitUninitialize(mAUNetSend);
		CloseComponent(mAUNetSend);
		mAUNetSend = NULL;
	}		
	if (mAUNetReceive) {
		AudioUnitUninitialize(mAUNetReceive);
		CloseComponent(mAUNetReceive);
		mAUNetReceive = NULL;
	}		
	free(inputBufferList);
	free(outputBufferList);
}


void AUNetEngine::loadPlugins()
{
	ComponentDescription searchDesc;
	TTErr err;
	
	searchDesc.componentType			= kAudioUnitType_Generator;
	searchDesc.componentSubType			= kAudioUnitSubType_NetReceive;
	searchDesc.componentManufacturer	= kAudioUnitManufacturer_Apple;
	searchDesc.componentFlags			= 0;
	searchDesc.componentFlagsMask		= 0;
	err = loadPlugin(searchDesc, mAUNetReceive);

	if (!err) {
		searchDesc.componentType			= kAudioUnitType_Effect;
		searchDesc.componentSubType			= kAudioUnitSubType_NetSend;
		searchDesc.componentManufacturer	= kAudioUnitManufacturer_Apple;
		searchDesc.componentFlags			= 0;
		searchDesc.componentFlagsMask		= 0;
		err = loadPlugin(searchDesc, mAUNetSend);
	}
}


TTErr AUNetEngine::loadPlugin(ComponentDescription& searchDesc, AudioUnit& plug)
{
	Component				comp = NULL;
	AURenderCallbackStruct	callbackStruct;

	comp = FindNextComponent(comp, &searchDesc);
	if (comp) {
		plug = OpenComponent(comp);
		callbackStruct.inputProc = &AUNetEngineGetInputSamples;
		callbackStruct.inputProcRefCon = this;
		AudioUnitSetProperty(plug, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &callbackStruct, sizeof(AURenderCallbackStruct));
		AudioUnitSetProperty(plug, kAudioUnitProperty_SampleRate, kAudioUnitScope_Global, 0, &mSampleRate, sizeof(mSampleRate));
		AudioUnitInitialize(plug);		
		return kTTErrNone;		
	}
	return kTTErrGeneric;
}


TTErr AUNetEngine::setNumInputChannels(const TTValue& v)
{
	if (inputBufferList)
		free(inputBufferList);
	
	mNumInputChannels = v;
	inputBufferList = (AudioBufferList*)malloc(offsetof(AudioBufferList, mBuffers) + (mNumInputChannels * sizeof(AudioBuffer)));
	
	for (TTUInt16 channel=0; channel<mNumInputChannels; channel++) {
		inputBufferList->mBuffers[channel].mNumberChannels = 1; 
		inputBufferList->mBuffers[channel].mData = NULL;			// We will set this pointer in the process method
	} 
	return kTTErrNone;
}


TTErr AUNetEngine::setNumOutputChannels(const TTValue& v)
{
	if (outputBufferList)
		free(outputBufferList);
	
	mNumOutputChannels = v;
	outputBufferList = (AudioBufferList*)malloc(offsetof(AudioBufferList, mBuffers) + (mNumOutputChannels * sizeof(AudioBuffer)));

	for(TTUInt16 channel=0; channel<mNumOutputChannels; channel++){
		outputBufferList->mBuffers[channel].mNumberChannels = 1; 
		outputBufferList->mBuffers[channel].mData = NULL;			// Tell the AU to deal with the memory
	} 
	return kTTErrNone;
}


/*
TTErr AUNetEngine::setParameter(const TTValue& nameAndValue)
{
	TTSymbolPtr	parameterName;
	TTFloat32	parameterValue;
	TTValue		v;
	TTErr		err;

	if(nameAndValue.getSize() != 2){
		logError("Bad arguments for setParameter()");
		return kTTErrGeneric;
	}
	
	nameAndValue.get(0, &parameterName);
	nameAndValue.get(1, parameterValue);
	err = parameterNames->lookup(parameterName, v);
	if(!err)
		AudioUnitSetParameter(audioUnit, v, kAudioUnitScope_Global, 0, parameterValue, 0);
	return err;
}


TTErr AUNetEngine::getParameter(TTValue& nameInAndValueOut)
{
	TTSymbolPtr	parameterName = nameInAndValueOut;
	TTValue		v;
	TTErr		err;
	long		parameterID = -1;
	Float32		parameterValue = 0.0;
	
	err = parameterNames->lookup(parameterName, v);
	if(!err){
		parameterID = v;
		AudioUnitGetParameter(audioUnit, parameterID, kAudioUnitScope_Global, 0, &parameterValue);
		nameInAndValueOut = parameterValue;
	}
	return err;
}
*/


/** Audio Processing Method */
/*
TTErr AUNetEngine::processAudio(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
{
	TTAudioSignal&				in = inputs->getSignal(0);
	TTAudioSignal&				out = outputs->getSignal(0);
	TTUInt16					vs = in.getVectorSize();
	TTUInt16					numInputChannels = in.getNumChannels();
	TTUInt16					numOutputChannels = out.getNumChannels();		
	TTFloat32*					auInput[numInputChannels];
	TTFloat32*					auOutput[numOutputChannels];
	AudioUnitRenderActionFlags	ioActionFlags = 0;
	
	// prepare the input
	for(TTUInt16 channel=0; channel<numInputChannels; channel++){
		in.getVector(channel, vs, auInput[channel]);
		inputBufferList->mBuffers[channel].mData = auInput[channel];
		inputBufferList->mBuffers[channel].mDataByteSize = sizeof(TTFloat32) * vs;
		outputBufferList->mBuffers[channel].mDataByteSize = sizeof(TTFloat32) * vs;
	}
	inputBufferList->mNumberBuffers = numInputChannels;
	outputBufferList->mNumberBuffers = numOutputChannels;
	
	// render the output using the plugin
	AudioUnitRender(audioUnit, &ioActionFlags, &timeStamp, 0, vs, outputBufferList);

	// handle the output
	numOutputChannels = outputBufferList->mNumberBuffers;
	for(TTUInt16 channel=0; channel<numOutputChannels; channel++){
		auOutput[channel] = (TTFloat32*)outputBufferList->mBuffers[channel].mData;
		out.setVector(channel, vs, auOutput[channel]);
	}
	
	timeStamp.mSampleTime += vs;
	return kTTErrNone;
}
 */
