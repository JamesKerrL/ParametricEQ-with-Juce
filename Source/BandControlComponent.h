#pragma once
#define _USE_MATH_DEFINES


#include <JuceHeader.h>

#include <cmath>
#include "FilterResponseCurveComponent.h"

class BandControlComponent : public juce::Component
{
public:
	BandControlComponent( juce::AudioProcessorValueTreeState& state_tree, std::function<void()> update_filter, int index) : mStateTree( state_tree ),
		mFilterUpdateCallback( update_filter ), mIndex( index )
	{
		mComboBoxAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>( mStateTree, FILTER_TYPE_PARAMETER_PREFIX + "_" + std::to_string( mIndex ), mFilterTypeComboBox );
		auto* parameter = mStateTree.getParameter( FILTER_TYPE_PARAMETER_PREFIX + "_" + std::to_string( mIndex ) );
		mFilterTypeComboBox.addItemList( parameter->getAllValueStrings(), 1 );

		mFilterTypeComboBox.onChange = [&]()
		{
			mFilterUpdateCallback();
		};

		mSlopeComboBoxAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>( mStateTree, FILTER_SLOPE_PARAMETER_PREFIX + "_" + std::to_string( mIndex ), mSlopeComboBox );
		auto* parameter_slope = mStateTree.getParameter( (FILTER_SLOPE_PARAMETER_PREFIX + "_" + std::to_string( mIndex )) );
		mSlopeComboBox.addItemList( parameter_slope->getAllValueStrings(), 1 );

		mSlopeComboBox.onChange = [&]()
		{
			mFilterUpdateCallback();
		};

		mFreqSlider.setTextValueSuffix( " Hz" );
		mFreqSlider.setNumDecimalPlacesToDisplay( 0 );
		mFreqSlider.setSliderStyle( juce::Slider::SliderStyle::RotaryVerticalDrag );
		mFreqSlider.setTextBoxStyle( juce::Slider::TextBoxBelow, true, 50, 20 );
		mFreqAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>( mStateTree, CUTOFF_PARAMETER_PREFIX + "_" + std::to_string( mIndex ), mFreqSlider );
		mFreqSlider.onValueChange = [&]()
		{
			mFilterUpdateCallback();
		};

		mResonanceSlider.setSliderStyle( juce::Slider::SliderStyle::RotaryVerticalDrag );
		mResonanceSlider.setTextBoxStyle( juce::Slider::TextBoxBelow, true, 50, 20 );
		mResonanceAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>( mStateTree, RESONANCE_PARAMETER_PREFIX + "_" + std::to_string( mIndex ), mResonanceSlider );
		mResonanceSlider.onValueChange = [&]()
		{
			mFilterUpdateCallback();
		};

		mGainSlider.setSliderStyle( juce::Slider::SliderStyle::RotaryVerticalDrag );
		mGainSlider.setTextBoxStyle( juce::Slider::TextBoxBelow, true, 50, 20 );
		mGainAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>( mStateTree, GAIN_PARAMETER_PREFIX + "_" + std::to_string( mIndex ), mGainSlider );
		mGainSlider.onValueChange = [&]()
		{
			mFilterUpdateCallback();
		};

		addAndMakeVisible( mFreqSlider );
		addAndMakeVisible( mResonanceSlider );
		addAndMakeVisible( mGainSlider );
		addAndMakeVisible( mFilterTypeComboBox );
		addAndMakeVisible( mSlopeComboBox );
	}

	BandControlComponent( BandControlComponent&& ) = default;


	void paint( juce::Graphics& g ) override
	{
		g.fillAll( juce::Colours::black );
	}

	void resized() override
	{
		mFilterTypeComboBox.setBounds( 10, 20, 80, 40 );
		mSlopeComboBox.setBounds( 110, 20, 80, 40 );
		mFreqSlider.setBounds( 230, 10, 80, 80 );
		mResonanceSlider.setBounds( 300, 10, 80, 80 );
		mGainSlider.setBounds( 390, 10, 80, 80 );
	}
private:
	juce::Slider mFreqSlider;
	juce::Slider mResonanceSlider;
	juce::Slider mGainSlider;
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mFreqAtt;
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mResonanceAtt;
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mGainAtt;
	juce::ComboBox mFilterTypeComboBox;
	juce::ComboBox mSlopeComboBox;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> mSlopeComboBoxAtt;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> mComboBoxAtt;

	std::function<void()> mFilterUpdateCallback;
	juce::AudioProcessorValueTreeState& mStateTree;
	int mIndex;

	const std::string CUTOFF_PARAMETER_PREFIX = "cutoff";
	const std::string RESONANCE_PARAMETER_PREFIX = "resonance";
	const std::string GAIN_PARAMETER_PREFIX = "gain";
	const std::string FILTER_TYPE_PARAMETER_PREFIX = "filterType";
	const std::string FILTER_SLOPE_PARAMETER_PREFIX = "filterSlope";
};