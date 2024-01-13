/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "BiquadFilter.h"
#include "FilterChain.h"
#include "FifoBuffer.h"

//==============================================================================
/**
*/

const std::string CUTOFF_PARAMETER_PREFIX = "cutoff";
const std::string RESONANCE_PARAMETER_PREFIX = "resonance";
const std::string GAIN_PARAMETER_PREFIX = "gain";
const std::string FILTER_TYPE_PARAMETER_PREFIX = "filterType";
const std::string FILTER_SLOPE_PARAMETER_PREFIX = "filterSlope";

class ParametricEQAudioProcessor : public juce::AudioProcessor
{
public:
	//==============================================================================
	ParametricEQAudioProcessor();
	~ParametricEQAudioProcessor() override;

	//==============================================================================
	void prepareToPlay( double sampleRate, int samplesPerBlock ) override;
	void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
	bool isBusesLayoutSupported( const BusesLayout& layouts ) const override;
#endif

	void processBlock( juce::AudioBuffer<float>&, juce::MidiBuffer& ) override;

	//==============================================================================
	juce::AudioProcessorEditor* createEditor() override;
	bool hasEditor() const override;

	//==============================================================================
	const juce::String getName() const override;

	bool acceptsMidi() const override;
	bool producesMidi() const override;
	bool isMidiEffect() const override;
	double getTailLengthSeconds() const override;

	//==============================================================================
	int getNumPrograms() override;
	int getCurrentProgram() override;
	void setCurrentProgram( int index ) override;
	const juce::String getProgramName( int index ) override;
	void changeProgramName( int index, const juce::String& newName ) override;

	//==============================================================================
	void getStateInformation( juce::MemoryBlock& destData ) override;
	void setStateInformation( const void* data, int sizeInBytes ) override;

	juce::AudioProcessorValueTreeState GlobalStateTree;
	std::vector<FilterChain> mFilterBands = {};

	void setFilterCallback( std::function<void()> filter_view_callback )
	{
		mFilterViewCallback = filter_view_callback;
	}

	void updateFilter( int index );

	FifoBuffer mFifo;

private:
	juce::AudioProcessorValueTreeState::ParameterLayout CreateParameterLayout();

	juce::UndoManager mUndoManager;
	std::function<void()> mFilterViewCallback = []() {};
	BiquadFilter::FilterType ToEnum( float filter_type );

	juce::AudioBuffer<float> mMonoBuffer;

	juce::NormalisableRange<float> FrequencyRange( float min, float max, float interval );
	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( ParametricEQAudioProcessor )
};
