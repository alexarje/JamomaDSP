/*
 * TTBlue Zero-crossing detector and counter.
 * Copyright © 2008, Timothy Place
 * 
 * License: This code is licensed under the terms of the GNU LGPL
 * http://www.gnu.org/licenses/lgpl.html 
 */

#include "TTZerocross.h"

#define thisTTClass			TTZerocross
#define thisTTClassName		"zerocross"
#define thisTTClassTags		"audio, analysis, frequency"


TT_AUDIO_CONSTRUCTOR
{
	TTUInt16	initialMaxNumChannels = arguments;
	
	// Attributes
	registerAttributeWithSetter(size, kTypeUInt32);
	
	// Messages
	registerMessageSimple(clear);
	registerMessageWithArgument(updateMaxNumChannels);
	
	// Set Defaults
	setAttributeValue(TT("maxNumChannels"),	initialMaxNumChannels);
	setAttributeValue(TT("size"), TTUInt32(2000));
	setProcessMethod(processAudio);
	clear();
}


TTZerocross::~TTZerocross()
{
	;
}


TTErr TTZerocross::updateMaxNumChannels(const TTValue& oldMaxNumChannels)
{
	return clear();
}


TTErr TTZerocross::clear()
{
	lastSampleWasOverZero = false;
	counter = 0;
	finalCount = 0;
	analysisLocation = 0;
	return kTTErrNone;
}


TTErr TTZerocross::setsize(const TTValue& value)
{
	size = value;
	rSize = 1.0 / size;
	return kTTErrNone;
}


// TODO: this unit requires 1 input and 2 outputs -- it does not yet configure itself for other arrangements!
TTErr TTZerocross::processAudio(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
{
	TTAudioSignal&	in = inputs->getSignal(0);
	TTAudioSignal&	out = outputs->getSignal(0);
	TTUInt16		vs = in.getVectorSize();
	TTSampleValue*	inSample;
	TTSampleValue*	out1Sample;
	TTSampleValue*	out2Sample;
	TTBoolean		thisSampleIsOverZero;
	TTBoolean		zeroxOccured;

	inSample = in.sampleVectors[0];
	out1Sample = out.sampleVectors[0];
	out2Sample = out.sampleVectors[1];
		
	while(vs--){
		thisSampleIsOverZero = (0 < (*inSample++));
		zeroxOccured = lastSampleWasOverZero != thisSampleIsOverZero;
		lastSampleWasOverZero = thisSampleIsOverZero;
		
		counter += zeroxOccured;
		analysisLocation++;
		
		if(analysisLocation >= size){
			finalCount = ((sr * counter) * rSize) * srInv;
			analysisLocation = 0;
			counter = 0;
		}
		
		*out1Sample++ = finalCount;
		*out2Sample++ = zeroxOccured;
	}
		
	return kTTErrNone;
}

