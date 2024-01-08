/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "FilterResponseCurveComponent.h"
#include "AnalysisComponent.h"
#include "BandControlComponent.h"

//==============================================================================
/**
*/
class ParametricEQAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
	ParametricEQAudioProcessorEditor( ParametricEQAudioProcessor& );
	~ParametricEQAudioProcessorEditor() override;

	//==============================================================================
	void paint( juce::Graphics& ) override;
	void resized() override;

private:
	// This reference is provided as a quick way for your editor to
	// access the processor object that created it.
	ParametricEQAudioProcessor& audioProcessor;

	void SetVisibleIndex( int index );
	AnalysisComponent mAnalysisView;
	juce::ComboBox mFilterTypeComboBox;
	juce::ComboBox mSlopeComboBox;
	juce::ComboBox mSelectedBandComboBox;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> mComboBoxAtt;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> mSlopeComboBoxAtt;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> mSelectedBandAtt;

	std::vector<std::unique_ptr<BandControlComponent>> mBandControls = {};


	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( ParametricEQAudioProcessorEditor )
};
