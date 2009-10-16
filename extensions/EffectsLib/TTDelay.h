/*
 * TTBlue Delay Unit
 * Copyright © 2003, Timothy Place
 * 
 * License: This code is licensed under the terms of the GNU LGPL
 * http://www.gnu.org/licenses/lgpl.html 
 */

#ifndef __TT_DELAY_H__
#define __TT_DELAY_H__

#include "TTDSP.h"


/**	Delay a signal. */
TTAUDIOCLASS(TTDelay)

	TTFloat64			delay;
	TTUInt64			delayInSamples;
	TTFloat64			delayMax;
	TTUInt64			delayMaxInSamples;
	TTSymbol*			interpolation;
	
	// alloc'd for each channel
	TTSampleValue*		fractionalDelay;		///< used in interpolated dsp loops, if zero then the delay increment is precisely on a sample boundary
	TTSampleValue* 		fractionalDelaySamples;	///< fractionalDelay expressed in samples rather than ms
	TTSampleValue**		buffer;
	TTSampleValue**		inPtr;					///< "write" pointer for buffer
	TTSampleValue**		outPtr;					///< "read" pointer
	TTSampleValue**		endPtr;					///< points to last sample in buffer (for speed)	
	
	/**	This method gets called when the inherited maxNumChannels attribute is changed. */
	TTErr updateMaxNumChannels(const TTValue& oldMaxNumChannels);

	/** Receives notifications when there are changes to the inherited sr attribute. */
	TTErr updateSr();
	
	TTErr init(TTUInt64 newDelayMaxInSamples);
	void reset();	// internal

	// Process with a constant delay time
	TTErr processAudioNoInterpolation(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs);
	TTErr processAudioLinearInterpolation(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs);
	TTErr processAudioCubicInterpolation(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs);

	// Process with a delay time set by a signal
//	TTErr processAudioNoInterpolationWithDelaySignal(TTAudioSignal& in, TTAudioSignal& delayIn, TTAudioSignal& out, TTAudioSignal&);
//	TTErr processAudioLinearInterpolationWithDelaySignal(TTAudioSignal& in, TTAudioSignal& delayIn, TTAudioSignal& out, TTAudioSignal&);
//	TTErr processAudioCubicInterpolationWithDelaySignal(TTAudioSignal& in, TTAudioSignal& delayIn, TTAudioSignal& out, TTAudioSignal&);
	
	/** Zero out the delay's buffer. */
	TTErr clear();
	
	/** Attribute Accessor */
	TTErr setdelay(const TTValue& newValue);
	
	/** Attribute Accessor */
	TTErr setdelayInSamples(const TTValue& newValue);
	
	/** Attribute Accessor */
	TTErr setdelayMax(const TTValue& newValue);
	
	/** Attribute Accessor */
	TTErr setdelayMaxInSamples(const TTValue& newValue);
	
	/** Attribute Accessor */
	TTErr setinterpolation(const TTValue& newValue);

};


#endif // __TT_DCBLOCK_H__
