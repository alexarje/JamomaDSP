/* 
 * TTBlue Pulse-based Envelope Substitution
 * Copyright © 2004, Timothy Place
 * 
 * License: This code is licensed under the terms of the GNU LGPL
 * http://www.gnu.org/licenses/lgpl.html 
 */

#include "TTPulseSub.h"

#define thisTTClass			TTPulseSub
#define thisTTClassName		"pulsesub"
#define thisTTClassTags		"audio, processor, dynamics, envelope"


TT_AUDIO_CONSTRUCTOR, 
	attrMode(TT("linear")), 
	env_gen(NULL),
	phasor(NULL), 
	offset(NULL), 
	scaler(NULL), 
	sig1(NULL), 
	sig2(NULL)
{
	TTUInt16	initialMaxNumChannels = arguments;
	
	registerAttribute(TT("Attack"),		kTypeFloat64,	&attrAttack,	(TTSetterMethod)&TTPulseSub::setAttack);
	registerAttribute(TT("Decay"),		kTypeFloat64,	&attrDecay,		(TTSetterMethod)&TTPulseSub::setDecay);
	registerAttribute(TT("Release"),	kTypeFloat64,	&attrRelease,	(TTSetterMethod)&TTPulseSub::setRelease);
	registerAttribute(TT("Sustain"),	kTypeFloat64,	&attrSustain,	(TTSetterMethod)&TTPulseSub::setSustain);
	registerAttribute(TT("Trigger"),	kTypeBoolean,	&attrTrigger,	(TTSetterMethod)&TTPulseSub::setTrigger);
	registerAttribute(TT("Mode"),		kTypeSymbol,	&attrMode,		(TTSetterMethod)&TTPulseSub::setMode);
	registerAttribute(TT("Frequency"),	kTypeFloat64,	&attrFrequency,	(TTSetterMethod)&TTPulseSub::setFrequency);
	registerAttribute(TT("Length"),		kTypeFloat64,	&attrLength,	(TTSetterMethod)&TTPulseSub::setLength);

	// register for notifications
	registerMessageWithArgument(updateMaxNumChannels);
	registerMessageSimple(updateSr);

	TTObjectInstantiate(kTTSym_adsr, &env_gen, initialMaxNumChannels);
	TTObjectInstantiate(kTTSym_phasor, &phasor, initialMaxNumChannels);
	TTObjectInstantiate(kTTSym_operator, &offset, initialMaxNumChannels);
	TTObjectInstantiate(kTTSym_operator, &scaler, initialMaxNumChannels);	
	TTObjectInstantiate(kTTSym_audiosignal, &sig1, 1);	
	TTObjectInstantiate(kTTSym_audiosignal, &sig2, 1);	
	offset->setAttributeValue(TT("Operator"), TT("+"));
	scaler->setAttributeValue(TT("Operator"), TT("*"));

	setAttributeValue(TT("Attack"), 50.);
	setAttributeValue(TT("Decay"), 100.);
	setAttributeValue(TT("Sustain"), -6.);
	setAttributeValue(TT("Release"), 500.);
	setAttributeValue(TT("Mode"), TT("linear"));	// <-- sets the process method
	setAttributeValue(TT("Length"), 0.25);
	
	setProcessMethod(processAudio);
}

TTPulseSub::~TTPulseSub()
{
	TTObjectRelease(&offset);
	TTObjectRelease(&phasor);
	TTObjectRelease(&env_gen);
	TTObjectRelease(&scaler);
	TTObjectRelease(&sig1);
	TTObjectRelease(&sig2);
}


TTErr TTPulseSub::updateSr()
{
	phasor->setAttributeValue(TT("SampleRate"), sr);
	offset->setAttributeValue(TT("SampleRate"), sr);
	env_gen->setAttributeValue(TT("SampleRate"), sr);
	scaler->setAttributeValue(TT("SampleRate"), sr);
	return kTTErrNone;
}


TTErr TTPulseSub::updateMaxNumChannels(const TTValue& oldMaxNumChannels)
{
	phasor->setAttributeValue(TT("MaxNumChannels"), maxNumChannels);
	offset->setAttributeValue(TT("MaxNumChannels"), maxNumChannels);
	env_gen->setAttributeValue(TT("MaxNumChannels"), maxNumChannels);
	scaler->setAttributeValue(TT("MaxNumChannels"), maxNumChannels);
	return kTTErrNone;
}


TTErr TTPulseSub::setAttack(const TTValue& newValue)
{
	attrAttack = newValue;
	return env_gen->setAttributeValue(TT("Attack"), const_cast<TTValue&>(newValue));
}

TTErr TTPulseSub::setDecay(const TTValue& newValue)
{
	attrDecay = newValue;
	return env_gen->setAttributeValue(TT("Decay"), attrDecay);
}


TTErr TTPulseSub::setSustain(const TTValue& newValue)
{
	attrSustain = newValue;
	return env_gen->setAttributeValue(TT("Sustain"), const_cast<TTValue&>(newValue));
}

TTErr TTPulseSub::setRelease(const TTValue& newValue)
{
	attrRelease = newValue;
	return env_gen->setAttributeValue(TT("Release"), const_cast<TTValue&>(newValue));
}


TTErr TTPulseSub::setMode(const TTValue& newValue)
{
	attrMode = newValue;
	return env_gen->setAttributeValue(TT("Mode"), const_cast<TTValue&>(newValue));
}

TTErr TTPulseSub::setTrigger(const TTValue& newValue)
{
	attrTrigger = newValue;
	return env_gen->setAttributeValue(TT("Trigger"), const_cast<TTValue&>(newValue));
}


TTErr TTPulseSub::setFrequency(const TTValue& newValue)
{
	attrFrequency = newValue;
	return phasor->setAttributeValue(TT("Frequency"), attrFrequency);
}

TTErr TTPulseSub::setLength(const TTValue& newValue)
{
	attrLength = newValue;
	return offset->setAttributeValue(TT("Operand"), attrLength - 0.5);
}


TTErr TTPulseSub::processAudio(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
{
	TTAudioSignal&	in = inputs->getSignal(0);
	TTAudioSignal&	out = outputs->getSignal(0);
	TTSampleValue*	inSample;
	TTSampleValue*	outSample;
	TTUInt16		vs = in.getVectorSizeAsInt();

	inSample = in.mSampleVectors[0];
	outSample = out.mSampleVectors[0];
	
	sig1->allocWithVectorSize(vs);
	sig2->allocWithVectorSize(vs);
	
	phasor->process(*sig1);					// ramp wave, stored in a temporary vector
	offset->process(*sig1, *sig2);			// offset the ramp wave, effectively altering the duty cycle
	env_gen->process(*sig2, *sig1);			// generate the envelope, reusing the temp[0] vector
	scaler->process(in, *sig1, out);		// apply the envelope to the input vector

	return kTTErrNone;
}
