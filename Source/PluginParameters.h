#pragma once


#include <JuceHeader.h>

namespace PluginParameters
{
	const std::string CUTOFF_PARAMETER_PREFIX = "cutoff";
	inline std::string GetCutOffParameterId( const int index )
	{
		return CUTOFF_PARAMETER_PREFIX + "_" + std::to_string( index );
	}

	const std::string RESONANCE_PARAMETER_PREFIX = "resonance";
	inline std::string GetResonanceParameterId( const int index )
	{
		return RESONANCE_PARAMETER_PREFIX + "_" + std::to_string( index );
	}

	const std::string GAIN_PARAMETER_PREFIX = "gain";
	inline std::string GetGainParameterId( const int index )
	{
		return GAIN_PARAMETER_PREFIX + "_" + std::to_string( index );
	}

	const std::string FILTER_TYPE_PARAMETER_PREFIX = "filterType";
	inline std::string GetFilterTypeParameterId( const int index )
	{
		return FILTER_TYPE_PARAMETER_PREFIX + "_" + std::to_string( index );
	}

	const std::string FILTER_SLOPE_PARAMETER_PREFIX = "filterSlope";
	inline std::string GetFilterSlopeParameterId( const int index )
	{
		return FILTER_SLOPE_PARAMETER_PREFIX + "_" + std::to_string( index );
	}
}