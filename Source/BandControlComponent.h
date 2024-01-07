#pragma once
#define _USE_MATH_DEFINES


#include <JuceHeader.h>

#include <cmath>
#include "FilterResponseCurveComponent.h"

class BandControlComponent : public juce::Component
{
public:
	BandControlComponent( juce::AudioProcessorValueTreeState& state_tree, std::function<void()> update_filter ) : mStateTree( state_tree ), mFilterUpdateCallback( update_filter )
	{
		mComboBoxAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>( mStateTree, "filterType", mFilterTypeComboBox );
		auto* parameter = mStateTree.getParameter( "filterType" );
		mFilterTypeComboBox.addItemList( parameter->getAllValueStrings(), 1 );

		mFilterTypeComboBox.onChange = [&]()
		{
			mFilterUpdateCallback();
		};

		mSlopeComboBoxAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>( mStateTree, "filterSlope", mSlopeComboBox );
		auto* parameter_slope = mStateTree.getParameter( "filterSlope" );
		mSlopeComboBox.addItemList( parameter_slope->getAllValueStrings(), 1 );

		mSlopeComboBox.onChange = [&]()
		{
			mFilterUpdateCallback();
		};

		addAndMakeVisible( mFilterTypeComboBox );
		addAndMakeVisible( mSlopeComboBox );
	}


	void paint( juce::Graphics& g ) override
	{
		g.fillAll( juce::Colours::black );
	}

	void resized() override
	{
		mFilterTypeComboBox.setBounds( 10, 20, 80, 40 );
		mSlopeComboBox.setBounds( 110, 20, 80, 40 );
	}
private:
	juce::ComboBox mFilterTypeComboBox;
	juce::ComboBox mSlopeComboBox;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> mSlopeComboBoxAtt;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> mComboBoxAtt;

	std::function<void()> mFilterUpdateCallback;
	juce::AudioProcessorValueTreeState& mStateTree;
};