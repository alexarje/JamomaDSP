/* 
 * TTBlue Pulse-based Envelope Substitution
 * Copyright © 2004, Timothy Place
 * 
 * License: This code is licensed under the terms of the GNU LGPL
 * http://www.gnu.org/licenses/lgpl.html 
 */

#ifndef __TT_PULSESUB_H__
#define __TT_PULSESUB_H__

#include "TTAudioObject.h"
#include "TTPhasor.h"
#include "TTAdsr.h"
#include "TTOperator.h"


/**	TTPulseSub generates a repeating ADSR envelope which is then applied to the gain of an input signal.
 */ 
class TTEXPORT TTPulseSub : public TTAudioObject {
protected:
	TTFloat64			attrAttack;
	TTFloat64			attrDecay;
	TTFloat64			attrSustain;
	TTFloat64			attrRelease;
	TTBoolean			attrTrigger;
	TTSymbolPtr			attrMode;
	TTFloat64			attrFrequency;
	TTFloat64			attrLength;
	TTAudioObjectPtr	env_gen;		///< TTAdsr
	TTAudioObjectPtr	phasor;			///< TTPhasor
	TTAudioObjectPtr	offset;			///< TTOperator
	TTAudioObjectPtr	scaler;			///< TTOperator
	TTAudioSignalPtr	sig1;
	TTAudioSignalPtr	sig2;
	
	TTErr updateSr();
	TTErr updateMaxNumChannels(const TTValue& oldMaxNumChannels);
	
	TTErr processAudio(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs);
	
public:
	TTPulseSub(TTValue& arguments);
	virtual ~TTPulseSub();
	
	TTErr setTrigger(const TTValue& newValue);
	TTErr setAttack(const TTValue& newValue);
	TTErr setDecay(const TTValue& newValue);
	TTErr setSustain(const TTValue& newValue);
	TTErr setRelease(const TTValue& newValue);
	TTErr setMode(const TTValue& newValue);
	
	TTErr setFrequency(const TTValue& newValue);
	TTErr setLength(const TTValue& newValue);
};

#endif // __TT_PULSESUB_H__

