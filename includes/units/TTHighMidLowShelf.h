/* 
 * TTBlue High-Mid-Low shelf filter 
 * portd by Nils Peters 2009, from the PD external hml_shelf~ by Thomas Musil
 * 
 */

//#ifndef __TT_HIGHPASS_BUTTERWORTH_2_H__
//#define __TT_HIGHPASS_BUTTERWORTH_2_H__

#ifndef _HIMIDLOW_H_
#define _HIMIDLOW_H_

#include "TTAudioObject.h"

class TTEXPORT TTHighMidLowShelf : public TTAudioObject {
protected:
	TTFloat64		frequencyLm, frequencyMh, gainL, gainM, gainH;///< filter parameter
	TTFloat64		a0, a1, a2, b1, b2;		///< filter coefficients
	TTFloat64		*xm1;
	TTFloat64		*xm2;
	TTFloat64		*xm0;
	//TTFloat64		*ym2;						// previous input and output samples

	/**	Receives notifications when there are changes to the inherited 
		maxNumChannels attribute.  This allocates memory for xm1, xm2, ym1, and ym2 
		so that each channel's previous values are remembered.		*/
	TTErr updateMaxNumChannels(const TTValue& oldMaxNumChannels);

	/** Receives notifications when there are changes to the inherited 
		sr attribute.						*/
	TTErr updateSr();

	/**	Standard audio processing method as used by TTBlue objects. */
	TTErr processAudio(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs);

	TTFloat64 lf_, mf_, hf_, lg_, mg_, hg_;
	void init();
	
public:

	/**	Constructor. */
	TTHighMidLowShelf(TTUInt16 newMaxNumChannels);

	/**	Destructor. */
	virtual ~TTHighMidLowShelf();

	/**	Setter for the filter attribute. */
	TTErr setgainL(const TTValue& value);
	TTErr setgainM(const TTValue& value);
	TTErr setgainH(const TTValue& value);
	TTErr setfrequencyLm(const TTValue& value);
	TTErr setfrequencyMh(const TTValue& value);
	
	TTErr clear();
};


#endif //  _HIMIDLOW_H_