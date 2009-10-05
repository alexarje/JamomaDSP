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
//OSStatus AUNetEngineGetInputSamples(void*						inRefCon,
//									AudioUnitRenderActionFlags*	ioActionFlags,
//									const AudioTimeStamp*		inTimeStamp,
//									UInt32						inBusNumber,
//									UInt32						inNumberFrames,
//									AudioBufferList*			ioData)
//{
//	AUNetEngine* AUNetEngine = (AUNetEngine*)inRefCon;
//
//	for(TTUInt16 channel=0; channel < ioData->mNumberBuffers; channel++)
//		memcpy(ioData->mBuffers[channel].mData, AUNetEngine->inputBufferList->mBuffers[channel].mData, sizeof(TTFloat32) * inNumberFrames);
//	return noErr;
//}

	
/**	Constructor. */
TT_OBJECT_CONSTRUCTOR_EXPORT
AUNetEngine::AUNetEngine(TTValue& arguments),
	mAUNetSend(NULL),
	mAUNetReceive(NULL)
//	inputBufferList(NULL), 
//	outputBufferList(NULL)
{
//	registerAttributeWithSetter(plugin,	kTypeSymbol);	
//	registerMessageWithArgument(setParameter);
//	registerMessageWithArgument(getParameter);

	timeStamp.mSampleTime = 0;
	timeStamp.mFlags = kAudioTimeStampSampleTimeValid;
	
}


/**	Destructor. */
AUNetEngine::~AUNetEngine()
{
	if(mAUNetSend){
		AudioUnitUninitialize(mAUNetSend);
		CloseComponent(mAUNetSend);
		mAUNetSend = NULL;
	}		
	if(mAUNetReceive){
		AudioUnitUninitialize(mAUNetReceive);
		CloseComponent(mAUNetReceive);
		mAUNetReceive = NULL;
	}		
//	free(inputBufferList);
//	free(outputBufferList);
}


void AUNetEngine::loadPlugins()
{
	ComponentDescription searchDesc;
	
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
	
	return err;
}


TTErr AUNetEngine::loadPlugin(const ComponentDescription& searchDesc, AudioUnit& plug)
{
	Component				comp = NULL;
	ComponentDescription	compDesc;
	Handle					compName;
	char*					compNameStr;
	int						compNameLen;
	TTSymbolPtr				pluginName = newPluginName;

	while (comp = FindNextComponent(comp, &searchDesc)) {
		compName = NewHandle(0);
		GetComponentInfo(comp, &compDesc, compName, NULL, NULL);
		HLock(compName);
		compNameStr = *compName;
		compNameLen = *compNameStr++;
		compNameStr[compNameLen] = 0;
		compNameStr = strchr(compNameStr, ':');
		compNameStr++;
		compNameStr++;
		
		if (!strcmp(compNameStr, pluginName->getCString())) {	
			AURenderCallbackStruct callbackStruct;

			audioUnit = OpenComponent(comp);
			plugin = pluginName;
			
			stuffParameterNamesIntoHash();
			
			HUnlock(compName);
			DisposeHandle(compName);
			
			// plugin is loaded, now activate it
			callbackStruct.inputProc = &AUNetEngineGetInputSamples;
			callbackStruct.inputProcRefCon = this;
			AudioUnitSetProperty(audioUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &callbackStruct, sizeof(AURenderCallbackStruct));
			AudioUnitSetProperty(audioUnit, kAudioUnitProperty_SampleRate, kAudioUnitScope_Global, 0, &sr, sizeof(sr));
			AudioUnitInitialize(audioUnit);
			
			return kTTErrNone;
		}
		
		HUnlock(compName);
		DisposeHandle(compName);
	}
}


TTErr AUNetEngine::updateMaxNumChannels(const TTValue& oldMaxNumChannels)
{
	if(inputBufferList)
		free(inputBufferList);
	if(outputBufferList)
		free(outputBufferList);
	
	// inputBufferList = (AudioBufferList*)malloc(offsetof(AudioBufferList, mBuffers[maxNumChannels]));
	// outputBufferList = (AudioBufferList*)malloc(offsetof(AudioBufferList, mBuffers[maxNumChannels]));
	inputBufferList = (AudioBufferList*)malloc(offsetof(AudioBufferList, mBuffers) + (maxNumChannels * sizeof(AudioBuffer)));
	outputBufferList = (AudioBufferList*)malloc(offsetof(AudioBufferList, mBuffers) + (maxNumChannels * sizeof(AudioBuffer)));

	for(TTUInt16 channel=0; channel<maxNumChannels; channel++){
		inputBufferList->mBuffers[channel].mNumberChannels = 1; 
		inputBufferList->mBuffers[channel].mData = NULL;			// We will set this pointer in the process method
		outputBufferList->mBuffers[channel].mNumberChannels = 1; 
		outputBufferList->mBuffers[channel].mData = NULL;			// Tell the AU to deal with the memory
	} 
	return kTTErrNone;
}


TTErr AUNetEngine::setplugin(TTValue& newPluginName)
{
	ComponentDescription	searchDesc;
	Component				comp = NULL;
	ComponentDescription	compDesc;
	Handle					compName;
	char*					compNameStr;
	int						compNameLen;
	TTSymbolPtr				pluginName = newPluginName;
	
	if(audioUnit){
		AudioUnitUninitialize(audioUnit);
		CloseComponent(audioUnit);
		audioUnit = NULL;
	}
	
	searchDesc.componentType = kAudioUnitType_Effect;	// TODO: support other types
	searchDesc.componentSubType = 0;					// kAudioUnitSubType_DefaultOutput;
	searchDesc.componentManufacturer = 0;				//kAudioUnitManufacturer_Apple;
	searchDesc.componentFlags = 0;
	searchDesc.componentFlagsMask = 0;
	
	while(comp = FindNextComponent(comp, &searchDesc)){
		compName = NewHandle(0);
		GetComponentInfo(comp, &compDesc, compName, NULL, NULL);
		HLock(compName);
		compNameStr = *compName;
		compNameLen = *compNameStr++;
		compNameStr[compNameLen] = 0;
		compNameStr = strchr(compNameStr, ':');
		compNameStr++;
		compNameStr++;
		
		if(!strcmp(compNameStr, pluginName->getCString())){	
			AURenderCallbackStruct callbackStruct;

			audioUnit = OpenComponent(comp);
			plugin = pluginName;
			
			stuffParameterNamesIntoHash();
			
			HUnlock(compName);
			DisposeHandle(compName);
			
			// plugin is loaded, now activate it
			callbackStruct.inputProc = &AUNetEngineGetInputSamples;
			callbackStruct.inputProcRefCon = this;
			AudioUnitSetProperty(audioUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &callbackStruct, sizeof(AURenderCallbackStruct));
			AudioUnitSetProperty(audioUnit, kAudioUnitProperty_SampleRate, kAudioUnitScope_Global, 0, &sr, sizeof(sr));
			AudioUnitInitialize(audioUnit);
			
			return kTTErrNone;
		}
		
		HUnlock(compName);
		DisposeHandle(compName);
	}
	return kTTErrGeneric;
}


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


/** Audio Processing Method */
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
