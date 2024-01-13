#pragma once
#define _USE_MATH_DEFINES


#include <JuceHeader.h>

#include <cmath>
#include "FilterResponseCurveComponent.h"
#include "SpectrumComponent.h"

class AnalysisComponent : public juce::Component
{
public:
	AnalysisComponent( FifoBuffer& fifo_buffer )
	{
		mSpectrum = std::make_unique<SpectrumComponent>( fifo_buffer );
		addAndMakeVisible( *mSpectrum );
		addAndMakeVisible( mResponseCurveView );
	}

	void SetMagnitudes(std::vector<float>& magnitudes)
	{
		mResponseCurveView.SetMagnitudes( magnitudes );
	}

	juce::Rectangle<int> getAnalysisAreaBounds()
	{
		return mResponseCurveView.getBounds();
	}

	void paint( juce::Graphics& g ) override
	{
		DrawFrequencyMarkers( g );
		DrawDecibelMarkers( g );
	}

	void DrawFrequencyMarkers( juce::Graphics& g )
	{
		g.setColour( juce::Colours::white );
		g.setFont( 10 );
		int step = getWidth() / 10;
		for (int freq_label = 0; freq_label < 10; freq_label++)
		{
			int freq = juce::mapToLog10( static_cast<double>(freq_label) / 10.0, 20.0, 20000.0 );
			juce::String str;
			str << freq;
			g.drawFittedText( str, { step * freq_label,getHeight() - 10 ,20,10 }, juce::Justification::centred, 1 );
		}
	}

	void DrawDecibelMarkers( juce::Graphics& g )
	{
		g.setColour( juce::Colours::white );
		g.setFont( 10 );
		int step_number = 6;
		int step = getHeight() / step_number;
		for (int db_marker = step_number-1; db_marker >= 0; db_marker--)
		{
			double db = juce::jmap( static_cast<double>(db_marker) / step_number, MAX_GAIN_IN_DB, MIN_GAIN_IN_DB );
			juce::String str;
			str << db << " db";
			g.drawFittedText( str, { getWidth() - 30,  step * db_marker , 20,10 }, juce::Justification::centred, 1 );
		}
	}

	void resized() override
	{
		auto local_bounds = getLocalBounds();
		auto view_bounds = juce::Rectangle<int>{ local_bounds.getX(), local_bounds.getY(), local_bounds.getWidth() - 30, local_bounds.getHeight() - 10 };
		mSpectrum->setBounds( view_bounds );
		mResponseCurveView.setBounds( view_bounds );
	}
private:
	std::vector<float> mMagnitudes;
	FilterResponseCurveComponent mResponseCurveView;
	std::unique_ptr<SpectrumComponent> mSpectrum;
	const double MAX_GAIN_IN_DB = 24.0;
	const double MIN_GAIN_IN_DB = -24.0;
};