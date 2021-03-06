/* 
 * Welch Window Function Unit for Jamoma DSP
 * Copyright © 2010 by Timothy Place
 * 
 * License: This code is licensed under the terms of the GNU LGPL
 * http://www.gnu.org/licenses/lgpl.html 
 */

#include "WelchWindow.h"

#define thisTTClass			WelchWindow
#define thisTTClassName		"welch"
#define thisTTClassTags		"audio, processor, function, window"


TT_AUDIO_CONSTRUCTOR
{
	setProcessMethod(processAudio);
	setCalculateMethod(calculateValue);
}


WelchWindow::~WelchWindow()
{
	;
}


// welch(i) = 1.0 - ((i-n/2)/(n/2)) * ((i-n/2)/(n/2))
TTErr WelchWindow::calculateValue(const TTFloat64& x, TTFloat64& y, TTPtrSizedInt data)
{
	TTFloat64	w;
	
	//w = ((x-1.0/2.0)/(1.0/2.0)); 
	w = (x-0.5)/0.5; 
	y = 1.0 - (w * w);
	return kTTErrNone;
}


TTErr WelchWindow::processAudio(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
{
	TT_WRAP_CALCULATE_METHOD(calculateValue);
}

