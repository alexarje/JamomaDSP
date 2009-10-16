/* 
 * TTBlue Balance Signal Amplitude
 * Copyright � 2008, Trond Lossius
 * 
 * License: This code is licensed under the terms of the GNU LGPL
 * http://www.gnu.org/licenses/lgpl.html 
 */

#include "TTBalance.h"

#define thisTTClass			TTBalance
#define thisTTClassName		"balance"
#define thisTTClassTags		"audio, processor, dynamics"


TT_AUDIO_CONSTRUCTOR
{
	TTUInt16	initialMaxNumChannels = arguments;
	
	// register attributes
	registerAttributeWithSetter(frequency,	kTypeFloat64);

	// register for notifications from the parent class so we can allocate memory as required
	registerMessageWithArgument(updateMaxNumChannels);
	// register for notifications from the parent class so we can recalculate coefficients as required
	registerMessageSimple(updateSr);
	// make the clear method available to the outside world
	registerMessageSimple(clear);

	// Set Defaults...
	setAttributeValue(TT("maxNumChannels"),	initialMaxNumChannels);			// This attribute is inherited
	setAttributeValue(TT("frequency"),		10.0);						// Default frequency is 10 Hz
	setProcessMethod(processAudio);
}


TTBalance::~TTBalance()
{;}


TTErr TTBalance::updateMaxNumChannels(const TTValue& oldMaxNumChannels)
{
	xm1A.resize(maxNumChannels);
	xm2A.resize(maxNumChannels);
	ym1A.resize(maxNumChannels);
	ym2A.resize(maxNumChannels);
	xm1B.resize(maxNumChannels);
	xm2B.resize(maxNumChannels);
	ym1B.resize(maxNumChannels);
	ym2B.resize(maxNumChannels);	
	return clear();
}


TTErr TTBalance::updateSr()
{
	TTValue	v(frequency);
	return setfrequency(v);
}


TTErr TTBalance::clear()
{
	xm1A.assign(maxNumChannels, 0.0);
	xm2A.assign(maxNumChannels, 0.0);
	ym1A.assign(maxNumChannels, 0.0);
	ym2A.assign(maxNumChannels, 0.0);
	xm1B.assign(maxNumChannels, 0.0);
	xm2B.assign(maxNumChannels, 0.0);
	ym1B.assign(maxNumChannels, 0.0);
	ym2B.assign(maxNumChannels, 0.0);
	return kTTErrNone;
}


TTErr TTBalance::setfrequency(const TTValue& newValue)
{
	frequency = TTClip((double)newValue, 1., (sr*0.45));

	c = 1 / ( tan( kTTPi*(frequency/sr) ) );
	a0 = 1 / (1 + kTTSqrt2*c + c*c);
	a1 = 2*a0;
	a2 = a0;
	b1 = 2*a0*( 1 - c*c );
	b2 = a0 * (1 - kTTSqrt2*c + c*c);
	return kTTErrNone;
}


TTErr TTBalance::processAudio(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
{
	TTAudioSignal&	in = inputs->getSignal(0);
	TTAudioSignal&	out = outputs->getSignal(0);
	TTUInt16		vs;
	TTSampleValue	*inSampleA,
					*inSampleB,
					*outSample;
	TTFloat64		tempxA,
					absTempxA,
					tempxB,
					absTempxB,
					tempyA,
					tempyB;											
	TTUInt16		channel;
	TTUInt16		numChannels;

	// Twice as many input channels are expected as output channels
	numChannels = TTAudioSignal::getNumChannels(in) / 2;
	if ( TTAudioSignal::getNumChannels(out) < numChannels)
		numChannels = TTAudioSignal::getNumChannels(out);

	// This outside loop works through each channel one at a time
	for(channel=0; channel<numChannels; channel++){
		// We first expect all channels of inputSignalA, then all channels of inputSignalB
		inSampleA = in.sampleVectors[channel];
		inSampleB = in.sampleVectors[channel+numChannels];
		outSample = out.sampleVectors[channel];
		vs = in.getVectorSize();
		
		// This inner loop works through each sample within the channel one at a time
		while(vs--){
			tempxA = *inSampleA++;
			absTempxA = fabs(tempxA);
			tempxB = *inSampleB++;
			absTempxB = fabs(tempxB);
			// Lopass filter left and right signals
			tempyA = TTAntiDenormal(a0*absTempxA + a1*xm1A[channel] + a2*xm2A[channel] - b1*ym1A[channel] - b2*ym2A[channel]);
			tempyB = TTAntiDenormal(a0*absTempxB + a1*xm1B[channel] + a2*xm2B[channel] - b1*ym1B[channel] - b2*ym2B[channel]);		
			// Scale left input to produce output, avoid dividing by zero
			if (tempyA)
				*outSample++ = tempxA * (tempyB/tempyA);
			else
				*outSample++ = 0.;
			// Update filter values
			xm2A[channel] = xm1A[channel];
			xm1A[channel] = absTempxA;
			ym2A[channel] = ym1A[channel];
			ym1A[channel] = tempyA;
			xm2B[channel] = xm1B[channel];
			xm1B[channel] = absTempxB;
			ym2B[channel] = ym1B[channel];
			ym1B[channel] = tempyB;
		}
	}
	return kTTErrNone;
}
