#pragma once
#define _USE_MATH_DEFINES


#include <JuceHeader.h>

#include <cmath>

class BiquadFilter
{
public:
	enum class FilterType
	{
		LPF,
		HPF,
		NOTCH,
		PEAK
	};

	void setParameters( float frequency, float resonance, float gain_in_db,  FilterType type )
	{
		FilterCoefficients new_coefficients;
		float A = std::sqrt( juce::Decibels::decibelsToGain( gain_in_db ) );
		auto Q = resonance;
		float wo = 2 * PI * frequency / mSampleRate;
		float sinW = sin( wo );
		float cosW = cos( wo );
		float alpha = sinW / (2 * Q);

		switch (type)
		{
		case FilterType::LPF:
		{
			new_coefficients.a0 = 1 + alpha;
			new_coefficients.a1 = -2 * cosW;
			new_coefficients.a2 = 1 - alpha;
			new_coefficients.b0 = (1 - cosW) / 2;
			new_coefficients.a1 /= new_coefficients.a0;
			new_coefficients.a2 /= new_coefficients.a0;
			new_coefficients.b0 /= new_coefficients.a0;
			new_coefficients.b1 = 2 * new_coefficients.b0;
			new_coefficients.b2 = new_coefficients.b0;
			break;
		}
		case FilterType::HPF:
		{
			new_coefficients.a0 = 1 + alpha;
			new_coefficients.a1 = -2 * cosW;
			new_coefficients.a2 = 1 - alpha;
			new_coefficients.b0 = (1 + cosW) / 2;
			new_coefficients.a1 /= new_coefficients.a0;
			new_coefficients.a2 /= new_coefficients.a0;
			new_coefficients.b0 /= new_coefficients.a0;
			new_coefficients.b1 = -2 * new_coefficients.b0;
			new_coefficients.b2 = new_coefficients.b0;
			break;
		}
		case FilterType::NOTCH:
		{
			new_coefficients.a0 = 1 + alpha;
			new_coefficients.a1 = -2 * cosW;
			new_coefficients.a2 = 1 - alpha;
			new_coefficients.b0 = 1;
			new_coefficients.a1 /= new_coefficients.a0;
			new_coefficients.a2 /= new_coefficients.a0;
			new_coefficients.b0 /= new_coefficients.a0;
			new_coefficients.b1 = -2 * cosW / new_coefficients.a0;
			new_coefficients.b2 = new_coefficients.b0;
			break;
		}
		case FilterType::PEAK:
		{
			new_coefficients.a0 = 1 + (alpha/A);
			new_coefficients.a1 = -2 * cosW;
			new_coefficients.a2 = 1 - (alpha / A);
			new_coefficients.b0 = 1 + (alpha * A);
			new_coefficients.a1 /= new_coefficients.a0;
			new_coefficients.a2 /= new_coefficients.a0;
			new_coefficients.b0 /= new_coefficients.a0;
			new_coefficients.b1 = -2 * cosW / new_coefficients.a0;
			new_coefficients.b2 = (1 - (alpha * A))/ new_coefficients.a0;
			break;
		}

		}


		mFilterCoefficients = new_coefficients;
	}

	void SetSampleRate( double sampleRate ) 
	{
		mSampleRate = sampleRate;
	}
	
	void process( juce::AudioBuffer<float>& buffer )
	{
		for ( int channel = 0; channel < buffer.getNumChannels(); ++channel )
		{
			auto* channel_ptr = buffer.getWritePointer( channel );
			ChannelState& state = states[channel];
			for (int sample = 0; sample < buffer.getNumSamples(); sample++)
			{
				float yn = (channel_ptr[sample] * mFilterCoefficients.b0) +
					(state.xn_1 * mFilterCoefficients.b1) + (state.xn_2 * mFilterCoefficients.b2) -
					(state.yn_1 *mFilterCoefficients.a1) - (state.yn_2 *mFilterCoefficients.a2);

				float xn = channel_ptr[sample];
				channel_ptr[sample] = yn;
				state.xn_2 = state.xn_1;
				state.yn_2 = state.yn_1;
				state.xn_1 = xn;
				state.yn_1 = yn;
			}
		}
	}

	// pass frequency into transfer function
	float getMagnitudeAtFrequency( double frequency )
	{
		auto w = 2.0f * PI * frequency / mSampleRate;
		std::complex<float> z( cos( w ), sin( w ) ); // e^jw
		auto den = 1.0f + mFilterCoefficients.a1 * z + mFilterCoefficients.a2  * z * z;
		auto num = mFilterCoefficients.b0 + mFilterCoefficients.b1 * z + mFilterCoefficients.b2 * z * z;
		return std::abs( (num / den) );
	}

private:
	struct FilterCoefficients{
		FilterCoefficients():
			a0(0.0f),
			a1( 0.0f ),
			a2( 0.0f ),
			b0( 0.0f ),
			b1(0.0f),
			b2( 0.0f )
		{
		}
		float a0;
		float a1;
		float a2;
		float b0;
		float b1;
		float b2;
	} mFilterCoefficients;

	 double PI = std::atan( 1 ) * 4;
	double mSampleRate = 0.0;

	struct ChannelState {
		ChannelState() :
			xn_1(0.0f),
			xn_2( 0.0f ),
			yn_1( 0.0f ),
			yn_2( 0.0f )
		{

		}
		float xn_1;
		float xn_2;
		float yn_1;
		float yn_2;
	};

	ChannelState states[2];
};