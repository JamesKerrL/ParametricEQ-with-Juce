/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ParametricEQAudioProcessorEditor::ParametricEQAudioProcessorEditor( ParametricEQAudioProcessor& p )
	: AudioProcessorEditor( &p ), audioProcessor( p )
{
	// Make sure that before the constructor has finished, you've set the
	// editor's size to whatever you need it to be.
	mFreqSlider.setTextValueSuffix( " Hz" );
	mFreqSlider.setSliderStyle( juce::Slider::SliderStyle::LinearVertical );
	mFreqSlider.setTextBoxStyle( juce::Slider::TextBoxBelow, true, 50, 20 );
	mFreqAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>( audioProcessor.GlobalStateTree, "cutoff", mFreqSlider );
	mFreqSlider.onValueChange = [&]()
	{
		audioProcessor.updateFilter();
	};

	mResonanceSlider.setSliderStyle( juce::Slider::SliderStyle::LinearVertical );
	mResonanceSlider.setTextBoxStyle( juce::Slider::TextBoxBelow, true, 50, 20 );
	mResonanceAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>( audioProcessor.GlobalStateTree, "resonance", mResonanceSlider );
	mResonanceSlider.onValueChange = [&]()
	{
		audioProcessor.updateFilter();
	};

	mGainSlider.setSliderStyle( juce::Slider::SliderStyle::LinearVertical );
	mGainSlider.setTextBoxStyle( juce::Slider::TextBoxBelow, true, 50, 20 );
	mGainAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>( audioProcessor.GlobalStateTree, "gain", mGainSlider );
	mGainSlider.onValueChange = [&]()
	{
		audioProcessor.updateFilter();
	};

	//mComboBoxAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>( audioProcessor.GlobalStateTree, "filterType", mFilterTypeComboBox );
	//auto* parameter = audioProcessor.GlobalStateTree.getParameter( "filterType" );
	//mFilterTypeComboBox.addItemList( parameter->getAllValueStrings(), 1 );

	//mFilterTypeComboBox.onChange = [&]()
	//{
	//	audioProcessor.updateFilter();
	//};

	//mSlopeComboBoxAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>( audioProcessor.GlobalStateTree, "filterSlope", mSlopeComboBox );
	//auto* parameter_slope = audioProcessor.GlobalStateTree.getParameter( "filterSlope" );
	//mSlopeComboBox.addItemList( parameter_slope->getAllValueStrings(), 1 );

	//mSlopeComboBox.onChange = [&]()
	//{
	//	audioProcessor.updateFilter();
	//};

	mBandControls = std::make_unique<BandControlComponent>( audioProcessor.GlobalStateTree, std::bind( &ParametricEQAudioProcessor::updateFilter, &audioProcessor ) );

	addAndMakeVisible( mAnalysisView );
	addAndMakeVisible( mFreqSlider );
	addAndMakeVisible( mResonanceSlider );
	addAndMakeVisible( mGainSlider );
	addAndMakeVisible( *mBandControls );
	//addAndMakeVisible( mFilterTypeComboBox );
	//addAndMakeVisible( mSlopeComboBox );

	setSize( 800, 300 );
	std::function<void()> func = [&]()
	{
		std::vector<float> magnitudes;
		int analysis_area_width = mAnalysisView.getAnalysisAreaBounds().getWidth();
		for (int i = 0; i < analysis_area_width; i++)
		{
			float frq = juce::mapToLog10( static_cast<double>(i) / static_cast<double>(analysis_area_width), 20.0, 20000.0 );
			magnitudes.push_back( juce::Decibels::gainToDecibels( audioProcessor.mFilter.getMagnitudeAtFrequency( frq ) ) );
		}
		mAnalysisView.SetMagnitudes( magnitudes );
	};
	audioProcessor.setFilterCallback( func );
	func();
}

ParametricEQAudioProcessorEditor::~ParametricEQAudioProcessorEditor()
{
}

//==============================================================================
void ParametricEQAudioProcessorEditor::paint( juce::Graphics& g )
{
	// (Our component is opaque, so we must completely fill the background with a solid colour)
	g.fillAll( getLookAndFeel().findColour( juce::ResizableWindow::backgroundColourId ) );

	g.setColour( juce::Colours::white );
	g.setFont( 15.0f );
	g.drawFittedText( "Hello dwwWorld!", getLocalBounds(), juce::Justification::centred, 1 );
}

void ParametricEQAudioProcessorEditor::resized()
{
	mAnalysisView.setBounds( 10, 10, 500, 200 );
	mFreqSlider.setBounds( 550, 10, 50, 200 );
	mResonanceSlider.setBounds( 600, 10, 50, 200 );
	mGainSlider.setBounds( 650, 10, 50, 200 );
	mBandControls->setBounds( 10, mAnalysisView.getBottom(), getWidth() - 20, getHeight() - mAnalysisView.getBottom() );

	//mFilterTypeComboBox.setBounds( 10, 240, 80, 40 );
	//mSlopeComboBox.setBounds( 110, 240, 80, 40 );
}
