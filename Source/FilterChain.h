#pragma once


#include <JuceHeader.h>
#include "BiquadFilter.h"

#include <cmath>

class FilterChain
{
public:
	FilterChain() : mFilters{ BiquadFilter{} }
	{

	}

	void setFilterOrder( int order )
	{
		if (order > mFilters.size())
		{
			for (int i = mFilters.size(); i < order; i++)
			{
				mFilters.push_back( BiquadFilter{} );
				mFilters.back().SetSampleRate( mSampleRate );
			}
		}
		else if (order < mFilters.size())
		{
			mFilters.erase( std::next( mFilters.begin(), order - 1 ), std::next( mFilters.begin(), mFilters.size() - 1 ) );
		}
	}

	void setParameters( float frequency, float resonance, float gain_in_db, BiquadFilter::FilterType type )
	{
		for (auto& filter : mFilters)
		{
			filter.setParameters( frequency, resonance, gain_in_db, type );
		}
	}

	void SetSampleRate( double sampleRate ) 
	{
		for (auto& filter : mFilters)
		{
			filter.SetSampleRate( sampleRate );
		}
		mSampleRate = sampleRate;
	}
	
	void process( juce::AudioBuffer<float>& buffer )
	{
		for (auto& filter : mFilters)
		{
			filter.process( buffer );
		}
	}

	// pass frequency into transfer function
	float getMagnitudeAtFrequency( double frequency )
	{
		float total = 1.0;
		for (auto& filter : mFilters)
		{
			total *= filter.getMagnitudeAtFrequency( frequency );
		}
		return total;
	}

private:
	std::vector<BiquadFilter> mFilters;
	double mSampleRate = 0.0;
};