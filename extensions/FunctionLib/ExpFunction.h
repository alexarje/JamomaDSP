/* 
 * ExpFunction Unit for TTBlue
 * Originally written for the Jamoma FunctionLib
 * Copyright © 2009 by Trond Lossius
 * Formula based on exscale (for ISPW and later for Max/MSP) by Norbert Schnell
 * 
 * License: This code is licensed under the terms of the GNU LGPL
 * http://www.gnu.org/licenses/lgpl.html 
 */

#ifndef __EXPFUNCTION_H__
#define __EXPFUNCTION_H__

#include "TTDSP.h"


/**	A function unit providing a curve based on
	y = exp(log(base)*x)
	but with a variety of additions to the basic trig function.
 */
class ExpFunction : TTAudioObject {
	TTCLASS_SETUP(ExpFunction)

	TTFloat64	mBase;			///< Base for the exponential function
	double		k;				///< Internal coefficient

	
	/** Calculate coefficients when this attr is set */
	TTErr setBase(const TTValue& value);
		
	/** y = f(x) for a single value */
	inline TTErr calculateValue(const TTFloat64& x, TTFloat64& y, TTPtrSizedInt data);
	
	/**	A standard audio processing method as used by TTBlue objects.*/
	TTErr processAudio(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs);

};


#endif // __EXPFUNCTION_H__
