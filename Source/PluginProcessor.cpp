/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ParametricEQAudioProcessor::ParametricEQAudioProcessor()
	: AudioProcessor( BusesProperties()
		.withInput( "Input", juce::AudioChannelSet::stereo(), true )
		.withOutput( "Output", juce::AudioChannelSet::stereo(), true )
	), GlobalStateTree( *this, &mUndoManager, "PARAMETERS", ParametricEQAudioProcessor::CreateParameterLayout() )
{
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
	mFilter.SetSampleRate( sampleRate );
	updateFilter();
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

	mFilter.process( buffer );
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
	auto cutoff = std::make_unique<juce::AudioParameterFloat>( "cutoff",
		"Cutoff",
		FrequencyRange( 20.0f, 20000.0f, 0.25f ),
		1000.0f );

	auto resonance = std::make_unique<juce::AudioParameterFloat>( "resonance",
		"Resonance",
		0.2,
		5,
		0.707f );

	auto gain = std::make_unique<juce::AudioParameterFloat>( "gain",
		"Gain",
		-24.f,
		24.f,
		0.f );

	auto filterType = std::make_unique<juce::AudioParameterChoice>( "filterType",
		"FilterType",
		juce::StringArray{ "LPF", "HPF", "NOTCH", "PEAK" },
		0 );

	auto filterSlope = std::make_unique<juce::AudioParameterChoice>( "filterSlope",
		"FilterSlope",
		juce::StringArray{ "12db/OCT", "24db/OCT", "36db/OCT", "48db/OCT" },
		0 );

	layout.add( std::move( cutoff ) );
	layout.add( std::move( resonance ) );
	layout.add( std::move( gain ) );
	layout.add( std::move( filterType ) );
	layout.add( std::move( filterSlope ) );

	return layout;
}

juce::NormalisableRange<float>
ParametricEQAudioProcessor::FrequencyRange( float min, float max, float interval )
{
	return { min, max, interval, 1.f / std::log2( 1.f + std::sqrt( max / min ) ) };
}

void
ParametricEQAudioProcessor::updateFilter()
{
	auto cutoff_parameter = GlobalStateTree.getRawParameterValue( "cutoff" );
	float cutoff = cutoff_parameter->load();
	auto resonance_parameter = GlobalStateTree.getRawParameterValue( "resonance" );
	float resonance = resonance_parameter->load();
	auto gain_parameter = GlobalStateTree.getRawParameterValue( "gain" );
	float gain = gain_parameter->load();
	auto filter_type_parameter = GlobalStateTree.getRawParameterValue( "filterType" );
	float filter_type = filter_type_parameter->load();

	auto order = GlobalStateTree.getRawParameterValue( "filterSlope" );
	int filter_order = order->load();
	mFilter.setFilterOrder( filter_order + 1 );
	mFilter.setParameters( cutoff, resonance, gain, ToEnum( filter_type ) );
	mFilterViewCallback();
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