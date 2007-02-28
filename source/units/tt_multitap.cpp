#include "tt_multitap.h"


// OBJECT LIFE ************************************	
TT_INLINE tt_multitap::tt_multitap(tt_attribute_value_discrete value)		// Instance Constructor: INT ARGUMENT
{
	reset();
	set_attr(k_buffersize_samples, value);
}

TT_INLINE tt_multitap::tt_multitap(tt_attribute_value value)				// Instance Constructor: FLOAT ARGUMENT
{
	reset();
	set_attr(k_buffersize_ms, value);
}

TT_INLINE tt_multitap::~tt_multitap()										// Instance Destructor
{	
	mem_free(buffer);	
}


// OVER-RIDEN SR METHOD ***************************
TT_INLINE void tt_multitap::set_sr(int value){
	if(value != sr){		// Do this only if the SR has changed
		short i;
		
		sr = value;
		r_sr = 1.0 / sr;
		m_sr = 0.001 * sr;
	
		if(buffersize_type)	// if the size was spec'd in ms then resize (don't do anything if spec'd in samples)
			set_attr(k_buffersize_ms, buffersize_ms);
		// THE ABOVE MAY CAUSE PROBLEMS(!!!) BECAUSE IT WILL CAUSE TROUBLE FOR THE REALTIME PERFORM ROUTINES
		for(i=0; i<num_taps; i++)
			set_attr(k_delay_ms, i, delay_ms[i]);
	}
}

// OVER-RIDEN VS METHOD ***************************
TT_INLINE void tt_multitap::set_vectorsize(int value)
{
	if(value != vectorsize){
		short i;
		
		//log_post("setting vs: %i (was: %i)", value, vectorsize);			
		vectorsize = value;
		if(buffersize_type)	// if the size was spec'd in ms then resize (don't do anything if spec'd in samples)
			set_attr(k_buffersize_ms, buffersize_ms);
		// THE ABOVE MAY CAUSE PROBLEMS(!!!) BECAUSE IT WILL CAUSE TROUBLE FOR THE REALTIME PERFORM ROUTINES
		for(i=0; i<num_taps; i++)
			set_attr(k_delay_ms, i, delay_ms[i]);			
	}
}




// ATTRIBUTES *************************************

TT_INLINE void tt_multitap::set_attr(tt_selector sel, tt_attribute_value val)				// "GLOBAL" ATTRIBUTES FOR THIS OBJECT...
{
	switch(sel){
		case k_buffersize_ms:
			buffersize_type = k_buffersize_type_ms;
			buffersize_ms = val;
			buffersize_samples = (tt_attribute_value_discrete)(buffersize_ms * m_sr);
			init_buffer();
			break;
		case k_buffersize_samples:
			buffersize_type = k_buffersize_type_samples;
			buffersize_samples = (tt_attribute_value_discrete)val;
			buffersize_ms = buffersize_samples / sr * 1000.0;
			init_buffer();
			break;
		case k_master_gain:
			master_gain = decibels_to_amplitude(val);
			break;
		case k_num_taps:
			num_taps = clip((int)val, 1, k_max_num_taps - 1);
			break;
	}
}

TT_INLINE tt_attribute_value tt_multitap::get_attr(tt_selector sel)
{
	switch(sel){
		case k_buffersize_ms:
			return buffersize_ms;
		case k_buffersize_samples:
			return (tt_attribute_value)buffersize_samples;
		case k_master_gain:
			return amplitude_to_decibels(master_gain);
		case k_num_taps:
			return (tt_attribute_value)num_taps;
		default:
			return -1;
	}
}


TT_INLINE void tt_multitap::set_attr(tt_selector sel, int tap, tt_attribute_value val)	// "TAP-SPECIFIC" ATTRIBUTES FOR THIS OBJECT...
{
	tap = clip(tap, 0, k_max_num_taps -1);		// range-limit the tap number
	switch(sel){
		case k_delay_ms:
			delay_ms[tap] = clip(val, 0.0F, buffersize_ms);
			delay_samples[tap] = (tt_attribute_value_discrete)(delay_ms[tap] * m_sr);
			position_playheads();
			break;
		case k_delay_samples:
			delay_samples[tap] = clip((tt_attribute_value_discrete)val, 0L, buffersize_samples);
			delay_ms[tap] = (tt_attribute_value)delay_samples[tap] / sr * 1000.0;
			position_playheads();
			break;
		case k_gain:
			gain[tap] = decibels_to_amplitude(val);
			break;
			// !!!!NEED TO CALL A FUNCTION HERE TO REPOSITION THE PLAY HEADS!!!!
	}
}

TT_INLINE tt_attribute_value tt_multitap::get_attr(tt_selector sel, int tap)
{
	tap = clip(tap, 0, k_max_num_taps -1);		// range-limit the tap number
	switch(sel){
		case k_delay_ms:
			return delay_ms[tap];
		case k_delay_samples:
			return (tt_attribute_value)delay_samples[tap];
		case k_gain:
			return gain[tap];
		default:
			return -1;
	}
}


// DSP ROUTINE(S) *********************************

TT_INLINE void tt_multitap::dsp_vector_calc(tt_audio_signal *in, tt_audio_signal *out)
{
	tt_sample_value temp;
	short i;
	temp_vs = in->vectorsize;
	
	if(buffer == NULL)								// there was a problem allocating memory
		return;										//	(can happen during fast sr/vs switching in auval)
	
	while(temp_vs--){
		*buffer_in++ = *in->vector++;				// record input with the write-pointer
		temp = 0.0;
		for(i=0; i < num_taps; i++)					// For each tap in the delay line:
			temp += (*buffer_out[i]++ * gain[i]);		//	get the output, scale it, and move to the next sample
		*out->vector++ = temp * master_gain;		// OUTPUT
		
//		if(buffer_in > buffer_end-10)					// Wrap the record head if necessary
//		if(buffer_in > buffer_end-1)					// Wrap the record head if necessary
		if(buffer_in > buffer_end)						// Wrap the record head if necessary

			buffer_in = buffer;		
		for(i=0; i<num_taps; i++){					// Wrap the play heads if neccessary
			if(buffer_out[i] > buffer_end)
				buffer_out[i] = buffer;
		}
	}
	in->reset(); out->reset();
}


// ADDITIONAL METHODS *****************************

TT_INLINE void tt_multitap::clear(void)
{
	long i;
	for(i=0; i<buffersize_samples; i++){
		buffer[i] = 0.0;
	}
}


TT_INLINE void tt_multitap::init_buffer()
{
	mem_free(buffer);	// release previously used memory (if any)
	buffer = (tt_sample_value *)mem_alloc((buffersize_samples + 1) * sizeof(tt_sample_value));
	if(buffer){
		buffer_in = buffer;									// point the record-head to the first location in the buffer
		buffer_end = buffer + buffersize_samples;			// point the end-pointer to the last location in the buffer
		position_playheads();
		clear();
	}
	else{
		log_error("tt_multitap could not allocate memory for the delay buffer!");
		buffer = NULL;
	}
}

TT_INLINE void tt_multitap::reset(void)
{
	short i;
	
	for(i=0; i<k_max_num_taps; i++){
		gain[i] = delay_ms[i] = delay_samples[i] = 0;
		buffer_out[i] = 0;
	}
	buffersize_ms = buffersize_samples = 0;
	buffersize_type = 0;
	buffer = buffer_in = buffer_end = 0;
	master_gain = num_taps = 0;

	set_sr(global_sr);
}

TT_INLINE void tt_multitap::position_playheads()				// point play heads to the correct location
{
	short i;
	for(i=0; i<k_max_num_taps; i++){		
		buffer_out[i] = buffer_in - delay_samples[i];
		if(buffer_out[i] < buffer)
			buffer_out[i] = buffer_end + (buffer_out[i] - buffer) + 1;
	}
}
