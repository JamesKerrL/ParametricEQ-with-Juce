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
		mFrequency = frequency;
		mResonance = resonance;
		mGainInDb = gain_in_db;
		mFilterType = type;
		// Even though the coefficients will be the same across filters they could be different in a later commit
		for (auto& filter : mFilters)
		{
			filter.RecalculateCoefficients( true, mFrequency, mResonance, mGainInDb, mFilterType );
		}
		mRecalculateCoefficients.store( true );
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
		if (mRecalculateCoefficients.load())
		{
			for (auto& filter : mFilters)
			{
				filter.RecalculateCoefficients( false, mFrequency, mResonance, mGainInDb, mFilterType );
			}
			mRecalculateCoefficients.store( false );
		}
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

	std::atomic<bool> mRecalculateCoefficients{ false };
	float mFrequency = 0.0f;
	float mResonance = 0.0f;
	float mGainInDb = 0.0f;
	BiquadFilter::FilterType mFilterType = BiquadFilter::FilterType::LPF;
};