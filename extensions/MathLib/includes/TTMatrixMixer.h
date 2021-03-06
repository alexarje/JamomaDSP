/* 
 * DSP Matrix Mixer Object
 * Copyright © 2009, Timothy Place
 * 
 * License: This code is licensed under the terms of the GNU LGPL
 * http://www.gnu.org/licenses/lgpl.html 
 */

#ifndef __TT_MATRIXMIXER_H__
#define __TT_MATRIXMIXER_H__

#include "TTDSP.h"


/**	TTGain is an exceptionally simple audio processor scales an input audio signal */
class TTMatrixMixer : TTAudioObject {
	TTCLASS_SETUP(TTMatrixMixer)

	TTUInt16		mNumInputs;
	TTUInt16		mNumOutputs;
	TTSampleMatrix	mGainMatrix;


	/**	A standard audio processing method as used by TTBlue objects.*/
	TTErr processAudio(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs);

	TTErr setNumInputs(const TTUInt16 newValue);
	TTErr setNumOutputs(const TTUInt16 newValue);
	
	TTErr Clear();

	
	/** setter for converting gain input from db to linear. 
		@param newValue The argument must consist of three values:
						- The x coordinate of the matrix point (beginning with zero)
						- The y coordinate of the matrix point (beggining with zero)
						- The gain level of the matrix point (in dB)
		@return			An error code.				*/
	TTErr SetGain(const TTValue& newValue);
	TTErr SetLinearGain(const TTValue& newValue);
	TTErr SetMidiGain(const TTValue& newValue);	
	
	void processOne(TTAudioSignal& in, TTAudioSignal& out, TTFloat64 gain);

};


#endif // __TT_MATRIXMIXER_H__
