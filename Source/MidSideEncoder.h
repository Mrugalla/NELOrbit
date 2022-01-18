#pragma once

namespace midSide
{
	inline void encode(float** samples, int numSamples) noexcept
	{
		for (auto s = 0; s < numSamples; ++s)
		{
			const auto mid = (samples[0][s] + samples[1][s]) * .5f;
			const auto side = (samples[0][s] - samples[1][s]) * .5f;
			samples[0][s] = mid;
			samples[1][s] = side;
		}
	}
	inline void decode(float** samples, int numSamples) noexcept
	{
		for (auto s = 0; s < numSamples; ++s)
		{
			const auto left = samples[0][s] + samples[1][s];
			const auto right = samples[0][s] - samples[1][s];
			samples[0][s] = left;
			samples[1][s] = right;
		}
	}
}
