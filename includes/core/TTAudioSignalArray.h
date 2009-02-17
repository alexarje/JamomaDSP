/* 
 * TTBlue Audio Signal Array Class
 * Copyright © 2008, Timothy Place
 * 
 * License: This code is licensed under the terms of the GNU LGPL
 * http://www.gnu.org/licenses/lgpl.html 
 */

#ifndef __TT_AUDIO_SIGNAL_ARRAY_H__
#define __TT_AUDIO_SIGNAL_ARRAY_H__

#include "TTObject.h"
#include "TTSymbol.h"
#include "TTValue.h"
#include "TTAudioSignal.h"


/****************************************************************************************************/
// Class Specification

/**	A simple container for an array of TTAudioSignal pointers.
	This class does not automatically manage the signals themselves, instantiate, or free them.	*/
class TTEXPORT TTAudioSignalArray : public TTObject {
protected:
	TTAudioSignalPtr*	audioSignals;			///< The actual array of audio signal pointers.
	TTUInt16			maxNumAudioSignals;		///< The maximum number of audio signals that can be passed in this array.
public:
	TTUInt16			numAudioSignals;		///< The number of audio signal pointers which are actually valid.

	
	TTAudioSignalArray(const TTUInt16 initialMaxNumAudioSignals);
	~TTAudioSignalArray();
	
	
	void		init();
	void		releaseAll();
	void		clearAll();
	void		allocAllWithVectorSize(TTUInt16 vs);
	TTUInt16	getVectorSize();
	void		setAllMaxNumChannels(TTUInt16 newMaxNumChannels);
	void		setAllNumChannels(TTUInt16 newNumChannels);
	
	/**	Note: calling this function will invalidate all audioSignal pointers contained within the array. */
	void setMaxNumAudioSignals(TTUInt16 newMaxNumAudioSignals)
	{
		maxNumAudioSignals = newMaxNumAudioSignals;
		init();
	}
	
	TTUInt16 getMaxNumAudioSignals()
	{
		return maxNumAudioSignals;
	}

	
	inline TTAudioSignal& getSignal(TTUInt16 index)
	{
		return *audioSignals[index];
	}
	
	inline TTErr setSignal(TTUInt16 index, const TTAudioSignalPtr aSignal)
	{
		audioSignals[index] = aSignal;
		return kTTErrNone;
	}
	
	
};


typedef TTAudioSignalArray* TTAudioSignalArrayPtr;

#endif // __TT_AUDIO_SIGNAL_ARRAY_H__
