/* 
 * Rectangular Window Function Unit for Jamoma DSP
 * Copyright © 2009 by Trond Lossius
 * 
 * License: This code is licensed under the terms of the GNU LGPL
 * http://www.gnu.org/licenses/lgpl.html 
 */

#include "RectangularWindow.h"

#define thisTTClass RectangularWindow
#define thisTTClassName		"rectangular"
#define thisTTClassTags		"audio, processor, function, window"


TT_AUDIO_CONSTRUCTOR
{
	setProcessMethod(processAudio);
	setCalculateMethod(calculateValue);
}


RectangularWindow::~RectangularWindow()
{
	;
}


TTErr RectangularWindow::calculateValue(const TTFloat64& x, TTFloat64& y, TTPtrSizedInt data)
{
	y = 1.0;
	return kTTErrNone;
}


TTErr RectangularWindow::processAudio(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
{
	TT_WRAP_CALCULATE_METHOD(calculateValue);
}

