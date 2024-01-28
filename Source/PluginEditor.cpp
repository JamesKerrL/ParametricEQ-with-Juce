/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Constants.h"

//==============================================================================
ParametricEQAudioProcessorEditor::ParametricEQAudioProcessorEditor( ParametricEQAudioProcessor& p )
	: AudioProcessorEditor( &p ), audioProcessor( p )
{
	// Make sure that before the constructor has finished, you've set the
	// editor's size to whatever you need it to be.
	mAnalysisView = std::make_unique<AnalysisComponent>( audioProcessor.mFifo, audioProcessor.mSampleRate );

	mSelectedBandAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>( audioProcessor.GlobalStateTree, "selectedBand", mSelectedBandComboBox );
	auto* parameter = audioProcessor.GlobalStateTree.getParameter( "selectedBand" );
	mSelectedBandComboBox.addItemList( parameter->getAllValueStrings(), 1 );

	mSelectedBandComboBox.onChange = [&]()
	{
		auto selected_band_parameter = audioProcessor.GlobalStateTree.getRawParameterValue( "selectedBand" );
		int selected_band = selected_band_parameter->load();
		SetVisibleIndex( selected_band );
	};

	for (int band = 0; band < Constants::NUMBER_OF_BANDS; band++)
	{
		auto local = std::make_unique<BandControlComponent>(
			audioProcessor.GlobalStateTree, std::bind( &ParametricEQAudioProcessor::updateFilter, &audioProcessor, band ), band );
		mBandControls.push_back( std::move(local) );
		addAndMakeVisible( *mBandControls.back() );
	}

	SetVisibleIndex( 0 );
	addAndMakeVisible( mSelectedBandComboBox );
	addAndMakeVisible( *mAnalysisView );

	setSize( 800, 300 );

	std::function<void()> func = [&]()
	{
		if (mAnalysisView)
		{
			std::vector<float> magnitudes;
			int analysis_area_width = mAnalysisView->getAnalysisAreaBounds().getWidth();
			for (int i = 0; i < analysis_area_width; i++)
			{
				float frq = juce::mapToLog10( static_cast<double>(i) / static_cast<double>(analysis_area_width), 20.0, 20000.0 );
				float value = 1.0f;
				for (auto& filter : audioProcessor.mFilterBands)
				{
					value *= filter.getMagnitudeAtFrequency( frq );
				}
				magnitudes.push_back( juce::Decibels::gainToDecibels( value ) );
			}
			mAnalysisView->SetMagnitudes( magnitudes );
		}
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
}

void ParametricEQAudioProcessorEditor::resized()
{
	mAnalysisView->setBounds( 10, 10, 500, 200 );
	mSelectedBandComboBox.setBounds( 600, 10, 50, 50 );
	for (int band = 0; band < Constants::NUMBER_OF_BANDS; band++)
	{
		mBandControls[band]->setBounds( 10, mAnalysisView->getBottom(), getWidth() - 20, getHeight() - mAnalysisView->getBottom() );
	}
}

void ParametricEQAudioProcessorEditor::SetVisibleIndex( int index_to_show )
{
	for (int index = 0; index < mBandControls.size(); index++)
	{
		if (index == index_to_show)
		{
			mBandControls[index]->setVisible( true );
		}
		else
		{
			mBandControls[index]->setVisible( false );
		}
	}
}