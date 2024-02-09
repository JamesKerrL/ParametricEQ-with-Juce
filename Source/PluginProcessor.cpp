/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Constants.h"

//==============================================================================
ParametricEQAudioProcessor::ParametricEQAudioProcessor()
	: AudioProcessor( BusesProperties()
		.withInput( "Input", juce::AudioChannelSet::stereo(), true )
		.withOutput( "Output", juce::AudioChannelSet::stereo(), true )
	), GlobalStateTree( *this, &mUndoManager, "PARAMETERS", ParametricEQAudioProcessor::CreateParameterLayout() )
	, mFifo( 2048 * 2 )
	, mListener( *this )
{
	for (int i = 0; i < Constants::NUMBER_OF_BANDS; i++)
	{
		auto filter_ptr = std::make_unique<FilterChain>();
		mFilterBands.push_back( std::move( filter_ptr ) );
	}
}

ParametricEQAudioProcessor::~ParametricEQAudioProcessor()
{
}

//==============================================================================
const juce::String ParametricEQAudioProcessor::getName() const
{
	return JucePlugin_Name;
}

bool ParametricEQAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
	return true;
#else
	return false;
#endif
}

bool ParametricEQAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
	return true;
#else
	return false;
#endif
}

bool ParametricEQAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
	return true;
#else
	return false;
#endif
}

double ParametricEQAudioProcessor::getTailLengthSeconds() const
{
	return 0.0;
}

int ParametricEQAudioProcessor::getNumPrograms()
{
	return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
				// so this should be at least 1, even if you're not really implementing programs.
}

int ParametricEQAudioProcessor::getCurrentProgram()
{
	return 0;
}

void ParametricEQAudioProcessor::setCurrentProgram( int index )
{
}

const juce::String ParametricEQAudioProcessor::getProgramName( int index )
{
	return {};
}

void ParametricEQAudioProcessor::changeProgramName( int index, const juce::String& newName )
{
}

//==============================================================================
void ParametricEQAudioProcessor::prepareToPlay( double sampleRate, int samplesPerBlock )
{
	mSampleRate = sampleRate;
	mMonoBuffer.setSize( 1, samplesPerBlock );
	int index = 0;
	for (auto& filter : mFilterBands)
	{
		filter->SetSampleRate( sampleRate );
		updateFilter( index );
		index++;
	}
}

void ParametricEQAudioProcessor::releaseResources()
{
	// When playback stops, you can use this as an opportunity to free up any
	// spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ParametricEQAudioProcessor::isBusesLayoutSupported( const BusesLayout& layouts ) const
{
#if JucePlugin_IsMidiEffect
	juce::ignoreUnused( layouts );
	return true;
#else
	// This is the place where you check if the layout is supported.
	// In this template code we only support mono or stereo.
	// Some plugin hosts, such as certain GarageBand versions, will only
	// load plugins that support stereo bus layouts.
	if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
		&& layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
		return false;

	// This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
	if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
		return false;
#endif

	return true;
#endif
}
#endif

void ParametricEQAudioProcessor::processBlock( juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages )
{
	juce::ScopedNoDenormals noDenormals;
	auto totalNumInputChannels = getTotalNumInputChannels();
	auto totalNumOutputChannels = getTotalNumOutputChannels();

	// In case we have more outputs than inputs, this code clears any output
	// channels that didn't contain input data, (because these aren't
	// guaranteed to be empty - they may contain garbage).
	// This is here to avoid people getting screaming feedback
	// when they first compile a plugin, but obviously you don't need to keep
	// this code if your algorithm always overwrites all the output channels.
	for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
		buffer.clear( i, 0, buffer.getNumSamples() );

	const float* left_ptr = buffer.getReadPointer( 0 );
	const float* right_ptr = buffer.getReadPointer( 1 );
	float* mono_ptr = mMonoBuffer.getWritePointer(0);
	for (int i = 0; i < buffer.getNumSamples(); i++)
	{
		mono_ptr[i] = (left_ptr[i] + right_ptr[i]) / 2;
	}

	mFifo.WriteTo( mono_ptr, buffer.getNumSamples() );

	for (auto& filter : mFilterBands)
	{
		filter->process( buffer );
	}
}

//==============================================================================
bool ParametricEQAudioProcessor::hasEditor() const
{
	return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ParametricEQAudioProcessor::createEditor()
{
	return new ParametricEQAudioProcessorEditor( *this );
}

//==============================================================================
void ParametricEQAudioProcessor::getStateInformation( juce::MemoryBlock& destData )
{
	// You should use this method to store your parameters in the memory block.
	// You could do that either as raw data, or use the XML or ValueTree classes
	// as intermediaries to make it easy to save and load complex data.
}

void ParametricEQAudioProcessor::setStateInformation( const void* data, int sizeInBytes )
{
	// You should use this method to restore your parameters from this memory block,
	// whose contents will have been created by the getStateInformation() call.
}

juce::AudioProcessorValueTreeState::ParameterLayout
ParametricEQAudioProcessor::CreateParameterLayout()
{
	juce::AudioProcessorValueTreeState::ParameterLayout layout;
	for (int index = 0; index < Constants::NUMBER_OF_BANDS; index++)
	{
		auto cutoff = std::make_unique<juce::AudioParameterFloat>( CUTOFF_PARAMETER_PREFIX + "_" + std::to_string( index ),
			"Cutoff Band " + std::to_string( index ),
			FrequencyRange( 20.0f, 20000.0f, 0.25f ),
			juce::mapToLog10( static_cast<float>(index + 1) / (Constants::NUMBER_OF_BANDS + 1), 20.0f, 20000.0f ) );

		auto resonance = std::make_unique<juce::AudioParameterFloat>( RESONANCE_PARAMETER_PREFIX + "_" + std::to_string( index ),
			"Resonance Band " + std::to_string( index ),
			0.2,
			5,
			0.707f );

		auto gain = std::make_unique<juce::AudioParameterFloat>( GAIN_PARAMETER_PREFIX + "_" + std::to_string( index ),
			"Gain Band " + std::to_string( index ),
			-24.f,
			24.f,
			0.f );

		auto filterType = std::make_unique<juce::AudioParameterChoice>( FILTER_TYPE_PARAMETER_PREFIX + "_" + std::to_string( index ),
			"FilterType Band " + std::to_string( index ),
			juce::StringArray{ "LPF", "HPF", "NOTCH", "PEAK" },
			3 );

		auto filterSlope = std::make_unique<juce::AudioParameterChoice>( FILTER_SLOPE_PARAMETER_PREFIX + "_" + std::to_string( index ),
			"FilterSlope Band " + std::to_string( index ),
			juce::StringArray{ "12db/OCT", "24db/OCT", "36db/OCT", "48db/OCT" },
			0 );

		layout.add( std::move( cutoff ) );
		layout.add( std::move( resonance ) );
		layout.add( std::move( gain ) );
		layout.add( std::move( filterType ) );
		layout.add( std::move( filterSlope ) );
	}

	juce::StringArray band_strings;
	for (int i = 0; i < Constants::NUMBER_OF_BANDS; i++)
	{
		band_strings.add(std::to_string( i ));
	}
	auto selectedBand = std::make_unique<juce::AudioParameterChoice>( "selectedBand",
		"SelectedBand",
		band_strings,
		0 );

	layout.add( std::move( selectedBand ) );

	return layout;
}

juce::NormalisableRange<float>
ParametricEQAudioProcessor::FrequencyRange( float min, float max, float interval )
{
	return { min, max, interval, 1.f / std::log2( 1.f + std::sqrt( max / min ) )};
}

void
ParametricEQAudioProcessor::updateFilter( int index )
{
	auto cutoff_parameter = GlobalStateTree.getRawParameterValue( CUTOFF_PARAMETER_PREFIX + "_" + std::to_string(index) );
	float cutoff = cutoff_parameter->load();
	auto resonance_parameter = GlobalStateTree.getRawParameterValue( RESONANCE_PARAMETER_PREFIX + "_" + std::to_string( index ) );
	float resonance = resonance_parameter->load();
	auto gain_parameter = GlobalStateTree.getRawParameterValue( GAIN_PARAMETER_PREFIX + "_" + std::to_string( index ) );
	float gain = gain_parameter->load();
	auto filter_type_parameter = GlobalStateTree.getRawParameterValue( FILTER_TYPE_PARAMETER_PREFIX + "_" + std::to_string( index ) );
	float filter_type = filter_type_parameter->load();

	auto order = GlobalStateTree.getRawParameterValue( FILTER_SLOPE_PARAMETER_PREFIX + "_" + std::to_string( index ) );
	int filter_order = order->load();

	mFilterBands[index]->setFilterOrder( filter_order + 1 );
	mFilterBands[index]->setParameters( cutoff, resonance, gain, ToEnum( filter_type ) );
}

BiquadFilter::FilterType
ParametricEQAudioProcessor::ToEnum( float filter_type )
{
	BiquadFilter::FilterType type;
	if (filter_type == 0.0f)
	{
		type = BiquadFilter::FilterType::LPF;
	}
	else if (filter_type == 1.0f)
	{
		type = BiquadFilter::FilterType::HPF;
	}
	else if (filter_type == 2.0f)
	{
		type = BiquadFilter::FilterType::NOTCH;
	}
	else if (filter_type == 3.0f)
	{
		type = BiquadFilter::FilterType::PEAK;
	}
	return type;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new ParametricEQAudioProcessor();
}


ParametricEQAudioProcessor::FilterUpdateListener::FilterUpdateListener( ParametricEQAudioProcessor& processor ) : mProcessor( processor )
{
	for (int i = 0; i < Constants::NUMBER_OF_BANDS; i++)
	{
		mProcessor.GlobalStateTree.addParameterListener( PluginParameters::GetCutOffParameterId( i ), this );
		mProcessor.GlobalStateTree.addParameterListener( PluginParameters::GetResonanceParameterId( i ), this );
		mProcessor.GlobalStateTree.addParameterListener( PluginParameters::GetGainParameterId( i ), this );
		mProcessor.GlobalStateTree.addParameterListener( PluginParameters::GetFilterTypeParameterId( i ), this );
		mProcessor.GlobalStateTree.addParameterListener( PluginParameters::GetFilterSlopeParameterId( i ), this );
	}
}

ParametricEQAudioProcessor::FilterUpdateListener::~FilterUpdateListener()
{
	for (int i = 0; i < Constants::NUMBER_OF_BANDS; i++)
	{
		mProcessor.GlobalStateTree.removeParameterListener( PluginParameters::GetCutOffParameterId( i ), this );
		mProcessor.GlobalStateTree.removeParameterListener( PluginParameters::GetResonanceParameterId( i ), this );
		mProcessor.GlobalStateTree.removeParameterListener( PluginParameters::GetGainParameterId( i ), this );
		mProcessor.GlobalStateTree.removeParameterListener( PluginParameters::GetFilterTypeParameterId( i ), this );
		mProcessor.GlobalStateTree.removeParameterListener( PluginParameters::GetFilterSlopeParameterId( i ), this );
	}
}

void 
ParametricEQAudioProcessor::FilterUpdateListener::parameterChanged( const juce::String& id, float newValue )
{
	for (int i = 0; i < Constants::NUMBER_OF_BANDS; i++)
	{
		mProcessor.updateFilter( i );
	}
}