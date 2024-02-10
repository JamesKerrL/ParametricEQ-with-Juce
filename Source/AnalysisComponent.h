#pragma once
#define _USE_MATH_DEFINES


#include <JuceHeader.h>

#include <cmath>
#include "FilterResponseCurveComponent.h"
#include "SpectrumComponent.h"
#include "FilterControllersComponent.h"
#include "Constants.h"
#include "PluginParameters.h"

class AnalysisComponent : public juce::Component, public juce::AudioProcessorValueTreeState::Listener, public juce::Timer
{
public:
	AnalysisComponent( ParametricEQAudioProcessor& processor, FifoBuffer& fifo_buffer, double sample_rate ) : mProcessor( processor )
	{
		mSpectrum = std::make_unique<SpectrumComponent>( fifo_buffer, sample_rate );
		mFilterControlsView = std::make_unique<FilterControllersComponent>( mProcessor.GlobalStateTree, Constants::NUMBER_OF_BANDS );
		addAndMakeVisible( *mSpectrum );
		addAndMakeVisible( mResponseCurveView );
		addAndMakeVisible( *mFilterControlsView );

		addMouseListener( mFilterControlsView.get(), false );


		for (int i = 0; i < Constants::NUMBER_OF_BANDS; i++)
		{
			mProcessor.GlobalStateTree.addParameterListener( PluginParameters::GetCutOffParameterId( i ), this );
			mProcessor.GlobalStateTree.addParameterListener( PluginParameters::GetResonanceParameterId( i ), this );
			mProcessor.GlobalStateTree.addParameterListener( PluginParameters::GetGainParameterId( i ), this );
			mProcessor.GlobalStateTree.addParameterListener( PluginParameters::GetFilterTypeParameterId( i ), this );
			mProcessor.GlobalStateTree.addParameterListener( PluginParameters::GetFilterSlopeParameterId( i ), this );
		}

		startTimerHz( 60 );
	}

	~AnalysisComponent( )
	{
		removeMouseListener( mFilterControlsView.get() );

		for (int i = 0; i < Constants::NUMBER_OF_BANDS; i++)
		{
			mProcessor.GlobalStateTree.removeParameterListener( PluginParameters::GetCutOffParameterId( i ), this );
			mProcessor.GlobalStateTree.removeParameterListener( PluginParameters::GetResonanceParameterId( i ), this );
			mProcessor.GlobalStateTree.removeParameterListener( PluginParameters::GetGainParameterId( i ), this );
			mProcessor.GlobalStateTree.removeParameterListener( PluginParameters::GetFilterTypeParameterId( i ), this );
			mProcessor.GlobalStateTree.removeParameterListener( PluginParameters::GetFilterSlopeParameterId( i ), this );
		}
	}

	juce::Rectangle<int> getAnalysisAreaBounds()
	{
		return mResponseCurveView.getBounds();
	}

	void paint( juce::Graphics& g ) override
	{
		auto local_bounds = getLocalBounds();
		auto view_bounds = juce::Rectangle<int>{ local_bounds.getX(), local_bounds.getY(), local_bounds.getWidth() - 30, local_bounds.getHeight() - 10 };
		juce::Path path;
		path.addRectangle( view_bounds );
		juce::Colour color{ juce::Colours::white };
		g.setColour( color.withAlpha( 0.7f ) );
		g.strokePath( path, juce::PathStrokeType( 1.0f ) );
		DrawFrequencyMarkers( g );
		DrawDecibelMarkers( g );

		//Draw lines
		juce::Path path_background;
	}

	void DrawFrequencyMarkers( juce::Graphics& g )
	{
		std::vector<float> frequencies_of_interest = { 20, 30, 40, 60, 80, 100, 200, 300, 400, 600, 800, 1000, 2000, 3000, 4000, 6000, 8000, 10000, 20000 };
		g.setColour( juce::Colours::white );
		g.setFont( 10 );
		int idx = 0;
		for (auto freq : frequencies_of_interest)
		{
			juce::String str;
			if (freq > 1000)
			{
				str << (freq / 1000) << " kHz";
			}
			else
			{
				str << freq;
			}
			int x = std::round(juce::mapFromLog10( freq, 20.0f, 20000.0f ) * mSpectrum->getWidth());
			g.drawFittedText( str, { x, getHeight() - 8 ,20,10 }, juce::Justification::centred, 1 );
			idx++;
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
			g.drawFittedText( str, { getWidth() - 28,  step * db_marker , 20,10 }, juce::Justification::centred, 1 );
		}
	}

	void resized() override
	{
		auto local_bounds = getLocalBounds();
		auto view_bounds = juce::Rectangle<int>{ local_bounds.getX(), local_bounds.getY(), local_bounds.getWidth() - 30, local_bounds.getHeight() - 10 };
		mSpectrum->setBounds( view_bounds );
		mResponseCurveView.setBounds( view_bounds );
		mFilterControlsView->setBounds( view_bounds );

		UpdateMagnitudes();
	}

	void parameterChanged( const juce::String& id, float newValue )
	{
		mUpdateMagnitudes.store( true );
	}

	void UpdateMagnitudes()
	{
		std::vector<float> magnitudes;
		int analysis_area_width = mResponseCurveView.getWidth();
		for (int i = 0; i < analysis_area_width; i++)
		{
			float frq = juce::mapToLog10( static_cast<double>(i) / static_cast<double>(analysis_area_width), 20.0, 20000.0 );
			float value = 1.0f;
			for (auto& filter : mProcessor.mFilterBands)
			{
				value *= filter->getMagnitudeAtFrequency( frq );
			}
			magnitudes.push_back( juce::Decibels::gainToDecibels( value ) );
		}
		mResponseCurveView.SetMagnitudes( magnitudes );
	}

	void timerCallback() override
	{
		if (mUpdateMagnitudes.load())
		{
			UpdateMagnitudes();
			mUpdateMagnitudes.store( false );
		}
		if (mSpectrum) {
			mSpectrum->setSampleRate( mProcessor.mSampleRate.load() );
		}
	}

private:
	ParametricEQAudioProcessor& mProcessor;
	FilterResponseCurveComponent mResponseCurveView;
	std::unique_ptr<SpectrumComponent> mSpectrum;
	std::unique_ptr<FilterControllersComponent> mFilterControlsView;
	const double MAX_GAIN_IN_DB = 24.0;
	const double MIN_GAIN_IN_DB = -24.0;
	std::atomic<bool> mUpdateMagnitudes{ false };
};