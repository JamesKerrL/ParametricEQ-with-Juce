#pragma once


#include <JuceHeader.h>


namespace
{
}

namespace Constants
{
	constexpr int NUMBER_OF_BANDS = 5;

	static const std::vector<juce::Colour> BAND_COLORS{ 
		juce::Colour( 0xffFF6C22 ),
		juce::Colour( 0xffC16991 ), 
		juce::Colour( 0xff9585B6 ), 
		juce::Colour( 0xff22B5FF ), 
		juce::Colour( 0xff69C199 ),
		// Just a duplicate of first
		juce::Colour( 0xffFF6C22 ),
		juce::Colour( 0xffC16991 ),
		juce::Colour( 0xff9585B6 ),
		juce::Colour( 0xff22B5FF ),
		juce::Colour( 0xff69C199 )
	};

	constexpr int FFT_SIZE = 2048 * 2;
	constexpr int FFT_ORDER = 12;
	//static juce::Colour get_band_color_for_index( const int index )
	//{
	//	int wrapped_index = index % BAND_COLORS.size();
	//	return BAND_COLORS[wrapped_index];
	//}
}