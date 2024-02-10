#pragma once
#define _USE_MATH_DEFINES


#include <JuceHeader.h>

#include <cmath>
#include "FilterResponseCurveComponent.h"
#include "FilterTypeComboBoxLook.h"
#include "FilterSlopeComboBoxLook.h"
#include "PluginParameters.h"
#include "Constants.h"

class BandControlComponent : public juce::Component
{
public:
	BandControlComponent( juce::AudioProcessorValueTreeState& state_tree, std::function<void()> update_filter, int index) : mStateTree( state_tree ),
		mFilterUpdateCallback( update_filter ), mIndex( index )
	{
		mFilterTypeComboBox.setLookAndFeel( &mFilterTypeLook );
		mFilterTypeComboBox.setJustificationType( juce::Justification::left );

		juce::PopupMenu * menu = mFilterTypeComboBox.getRootMenu();
		mComboBoxAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>( mStateTree, PluginParameters::GetFilterTypeParameterId( mIndex ), mFilterTypeComboBox );
		auto* parameter = mStateTree.getParameter( PluginParameters::GetFilterTypeParameterId( mIndex ) );
		int id = 1;
		for (auto value: parameter->getAllValueStrings())
		{
			juce::PopupMenu::Item item;
			item.text = value;
			item.setID( id );
			auto drawable = juce::Drawable::createFromImageData( BinaryData::lowpassicon_svg, BinaryData::lowpassicon_svgSize );
			item.setImage( std::move( drawable ) );
			menu->addItem( item );
			id++;
		}

		mFilterTypeComboBox.setSelectedItemIndex( parameter->convertFrom0to1( parameter->getDefaultValue() ) );

		mSlopeComboBoxAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>( mStateTree, PluginParameters::GetFilterSlopeParameterId( mIndex ), mSlopeComboBox );
		auto* parameter_slope = mStateTree.getParameter( (PluginParameters::GetFilterSlopeParameterId( mIndex )) );
		mSlopeComboBox.addItemList( parameter_slope->getAllValueStrings(), 1 );

		mSlopeComboBox.onChange = [&]()
		{
			mFilterUpdateCallback();
		};

		mSlopeComboBox.setLookAndFeel( &mFilterSlopeLook );
		mSlopeComboBox.setSelectedItemIndex( parameter->convertFrom0to1( parameter_slope->getDefaultValue() ) );

		mFreqSlider.setTextValueSuffix( " Hz" );
		mFreqSlider.setSliderStyle( juce::Slider::SliderStyle::RotaryVerticalDrag );
		mFreqSlider.setTextBoxStyle( juce::Slider::TextBoxBelow, true, 70, 20 );
		mFreqAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>( mStateTree, PluginParameters::GetCutOffParameterId( mIndex ), mFreqSlider );

		mFreqSlider.setColour( juce::Slider::ColourIds::rotarySliderFillColourId, Constants::BAND_COLORS[mIndex] );
		mFreqSlider.setColour( juce::Slider::ColourIds::thumbColourId, juce::Colours::transparentWhite );
		mFreqSlider.setColour( juce::Slider::ColourIds::textBoxOutlineColourId, juce::Colours::transparentWhite );
		//mFreqSlider.setNumDecimalPlacesToDisplay( 0 );

		mResonanceSlider.setSliderStyle( juce::Slider::SliderStyle::RotaryVerticalDrag );
		mResonanceSlider.setTextBoxStyle( juce::Slider::TextBoxBelow, true, 50, 20 );
		mResonanceAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>( mStateTree, PluginParameters::GetResonanceParameterId( mIndex ), mResonanceSlider );
		mResonanceSlider.setColour( juce::Slider::ColourIds::rotarySliderFillColourId, Constants::BAND_COLORS[mIndex] );
		mResonanceSlider.setColour( juce::Slider::ColourIds::thumbColourId, juce::Colours::transparentWhite );
		mResonanceSlider.setColour( juce::Slider::ColourIds::textBoxOutlineColourId, juce::Colours::transparentWhite );

		mGainSlider.setSliderStyle( juce::Slider::SliderStyle::RotaryVerticalDrag );
		mGainSlider.setTextBoxStyle( juce::Slider::TextBoxBelow, true, 70, 20 );
		mGainAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>( mStateTree, PluginParameters::GetGainParameterId( mIndex ), mGainSlider );
		mGainSlider.setColour( juce::Slider::ColourIds::rotarySliderFillColourId, Constants::BAND_COLORS[mIndex] );
		mGainSlider.setColour( juce::Slider::ColourIds::thumbColourId, juce::Colours::transparentWhite );
		mGainSlider.setColour( juce::Slider::ColourIds::textBoxOutlineColourId, juce::Colours::transparentWhite );
		mGainSlider.setTextValueSuffix( " dB" );

		addAndMakeVisible( mFreqSlider );
		addAndMakeVisible( mResonanceSlider );
		addAndMakeVisible( mGainSlider );
		addAndMakeVisible( mFilterTypeComboBox );
		addAndMakeVisible( mSlopeComboBox );
	}

	~BandControlComponent()
	{
		mFilterTypeComboBox.setLookAndFeel( nullptr );
		mSlopeComboBox.setLookAndFeel( nullptr );
	}

	BandControlComponent( BandControlComponent&& ) = default;


	void paint( juce::Graphics& g ) override
	{
		g.fillAll( juce::Colours::black );
		juce::Path path;
		path.addRectangle( getLocalBounds() );
		g.setColour( Constants::BAND_COLORS[mIndex].withAlpha( 0.2f ) );
		g.strokePath( path, juce::PathStrokeType( 2 ) );
	}

	void resized() override
	{
		float start = 250;
		float interval = ((getWidth() - start - 200) / 3);
		mFilterTypeComboBox.setBounds( 10, 20, 80, 40 );
		mSlopeComboBox.setBounds( 150, 20, 80, 40 );
		mFreqSlider.setBounds( start, 5, 80, 80 );
		mResonanceSlider.setBounds( start + (interval) , 5, 80, 80 );
		mGainSlider.setBounds( start +(interval* 2), 5, 80, 80 );
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

	FilterTypeComboBoxLook mFilterTypeLook;
	FilterSlopeComboBoxLook mFilterSlopeLook;

	std::function<void()> mFilterUpdateCallback;
	juce::AudioProcessorValueTreeState& mStateTree;
	int mIndex;
};