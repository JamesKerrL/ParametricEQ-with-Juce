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
	setInterceptsMouseClicks( false, true );
	// Make sure that before the constructor has finished, you've set the
	// editor's size to whatever you need it to be.
	mAnalysisView = std::make_unique<AnalysisComponent>( audioProcessor, audioProcessor.mFifo, audioProcessor.mSampleRate.load() );

	// Band selection
	mBandSelector = std::make_unique<BandSelectionComponent>( audioProcessor, Constants::NUMBER_OF_BANDS,
		[&]( int index )
	{
		SetVisibleIndex( index );
	} );

	auto selected_band_parameter = audioProcessor.GlobalStateTree.getRawParameterValue( "selectedBand" );
	mBandSelector->SetSelectedButton( selected_band_parameter->load() );
	addAndMakeVisible( *mBandSelector );


	for (int band = 0; band < Constants::NUMBER_OF_BANDS; band++)
	{
		auto local = std::make_unique<BandControlComponent>(
			audioProcessor.GlobalStateTree, std::bind( &ParametricEQAudioProcessor::updateFilter, &audioProcessor, band ), band );
		mBandControls.push_back( std::move(local) );
		addAndMakeVisible( *mBandControls.back() );
	}
	SetVisibleIndex( 0 );

	addAndMakeVisible( *mAnalysisView );

	setSize( 800, 350 );
}

ParametricEQAudioProcessorEditor::~ParametricEQAudioProcessorEditor()
{
}

//==============================================================================
void ParametricEQAudioProcessorEditor::paint( juce::Graphics& g )
{
	g.fillAll( juce::Colours::black );

	g.setColour( juce::Colours::white );
}

void ParametricEQAudioProcessorEditor::resized()
{
	mAnalysisView->setBounds( 10, 10, getWidth() - 10, 200 );
	mBandSelector->setBounds( 10, mAnalysisView->getBottom() + 5, getWidth() - 20, 40 );
	for (int band = 0; band < Constants::NUMBER_OF_BANDS; band++)
	{
		mBandControls[band]->setBounds( 10, mBandSelector->getBottom(), getWidth() - 20, getHeight() - (mBandSelector->getBottom()) - 7 );
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