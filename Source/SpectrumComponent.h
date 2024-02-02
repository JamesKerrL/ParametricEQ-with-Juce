#pragma once
#include <JuceHeader.h>

#include <cmath>
#include "FilterResponseCurveComponent.h"
#include "FifoBuffer.h"

class SpectrumComponent : public juce::Component, public juce::Timer
{
public:
	const int FFT_SIZE = 1024;
	SpectrumComponent( FifoBuffer& fifo_buffer, double sample_rate ) :
		forwardFFT( 10 ),
		window( FFT_SIZE, juce::dsp::WindowingFunction<float>::hann ),
		mFifoBuffer( fifo_buffer ),
		mSampleRate( sample_rate )
	{
		juce::FloatVectorOperations::fill( mInternalBuffer.data(), 0.0f, mInternalBuffer.size() );
		juce::FloatVectorOperations::fill( mRenderBuffer.data(), 0.0f, mRenderBuffer.size() );
		startTimerHz( 30 );
	}

	void paint( juce::Graphics& g ) override
	{
		g.fillAll( juce::Colours::black );


		g.setColour( juce::Colours::orange );
		if (mPath) {
			g.strokePath( *mPath, juce::PathStrokeType( 2.0f, juce::PathStrokeType::curved ) );
			juce::Path fill_path = *mPath;
			fill_path.lineTo( getWidth(), getHeight() );
			fill_path.lineTo( 0, getHeight() );
			fill_path.closeSubPath();
			juce::Colour color{ juce::Colours::orange };
			g.setColour( color.withAlpha( 0.3f ) );
			g.fillPath( fill_path );
		}
	}

	void resized() override
	{

	}

	void timerCallback() override 
	{
		if (mFifoBuffer.getNumReady() >= mInternalBuffer.size())
		{
			mFifoBuffer.ReadFrom( mInternalBuffer.data(), mInternalBuffer.size() );

			window.multiplyWithWindowingTable( mInternalBuffer.data(), FFT_SIZE );
			forwardFFT.performFrequencyOnlyForwardTransform( mInternalBuffer.data() );
			auto fsize = forwardFFT.getSize();

			juce::FloatVectorOperations::multiply( mRenderBuffer.data(), 0.7f, mRenderBuffer.size() );
			for (auto i = 0; i < mRenderBuffer.size(); ++i)
			{
				if (mInternalBuffer[i] > mRenderBuffer[i])
					mRenderBuffer[i] = mInternalBuffer[i];
			}

			juce::Path path;
			int index = 0;
			float frequency = 0;
			frequency = (index * mSampleRate) / FFT_SIZE;

			float y0 = juce::jmap<float>( juce::Decibels::gainToDecibels( mRenderBuffer[0] / FFT_SIZE ), -78.0, 0.0, getBottom(), getY() );
			path.startNewSubPath( 0, y0 );

			while (frequency < 20)
			{
				index++;
				frequency = (index * mSampleRate) / FFT_SIZE;
			}

			while (frequency <= 20000 && index < FFT_SIZE)
			{
				float y = juce::jmap<float>( juce::Decibels::gainToDecibels( mRenderBuffer[index] / FFT_SIZE ), -78.0, 0.0, getBottom(), getY() );
				float x = juce::mapFromLog10( frequency, 20.0f, 20000.0f ) * getWidth();

				path.lineTo( x, y );
				index++;
				frequency = (index * mSampleRate) / FFT_SIZE;
			}
			mPath = std::make_unique<juce::Path>( path.createPathWithRoundedCorners( 10 ) );
			repaint();
		}
	}

private:
	std::unique_ptr<juce::Path> mPath;
	juce::dsp::FFT forwardFFT;
	juce::dsp::WindowingFunction<float> window;
	FifoBuffer& mFifoBuffer;
	std::array<float, 2048> mInternalBuffer;
	std::array<float, 2048> mRenderBuffer;
	double mSampleRate;
};