#pragma once
#include <array>
#include <vector>
#include "Orbit.h"

namespace drywet
{
	struct Processor
	{
		using Smooth = orbit::Smooth<float>;
		using DryBuf = std::array<std::vector<float>, 2>;
		using SqrtBuf = std::array<std::vector<float>, 2>;
		using ParamBuf = std::vector<float>;

		Processor() :
			mixSmooth(), gainSmooth(),
			mixBuf(), gainBuf(),
			dryBuf(),
			sqrtBuf(),
			gain(0.f), gainVal(1.f)
		{
		}
		void prepare(float sampleRate, int blockSize)
		{
			orbit::prepareParam(mixSmooth, mixBuf, 20.f, sampleRate, blockSize);
			orbit::prepareParam(gainSmooth, gainBuf, 20.f, sampleRate, blockSize);
			for (auto& d : dryBuf)
				d.resize(blockSize, 0.f);
			for (auto& s : sqrtBuf)
				s.resize(blockSize);
		}
		void saveDry(const float** samples, float mixVal, int numChannelsIn, int numChannelsOut, int numSamples) noexcept
		{
			{ // SMOOTHEN PARAMETER VALUE pVAL
				for (auto s = 0; s < numSamples; ++s)
					mixBuf[s] = mixSmooth(mixVal);
			}
			{ // MAKING EQUAL LOUDNESS CURVES
				for (auto s = 0; s < numSamples; ++s)
					sqrtBuf[0][s] = std::sqrt(1.f - mixBuf[s]);
				for (auto s = 0; s < numSamples; ++s)
					sqrtBuf[1][s] = std::sqrt(mixBuf[s]);
			}
			{ // SAVE DRY BUFFER
				for (auto ch = 0; ch < numChannelsIn; ++ch)
				{
					auto dry = dryBuf[ch].data();
					const auto smpls = samples[ch];

					juce::FloatVectorOperations::copy(dry, smpls, numSamples);
				}
			}
		}
		void processWet(float** samples, float _gain, int numChannelsIn, int numChannelsOut, int numSamples) noexcept
		{
			if (gain != _gain)
			{
				gain = _gain;
				gainVal = juce::Decibels::decibelsToGain(gain);
			}
			{
				for (auto s = 0; s < numSamples; ++s)
					gainBuf[s] = gainSmooth(gainVal);
			}
			{
				auto smpls = samples[0];
				const auto dry = dryBuf[0].data();

				for (auto s = 0; s < numSamples; ++s)
					smpls[s] = (dry[s] * sqrtBuf[0][s] + smpls[s] * sqrtBuf[1][s]) * gainBuf[s];
			}
			if (numChannelsOut == 2)
			{
				auto smpls = samples[1];
				const auto dry = dryBuf[1 % numChannelsIn].data();

				for (auto s = 0; s < numSamples; ++s)
					smpls[s] = (dry[s] * sqrtBuf[0][s] + smpls[s] * sqrtBuf[1][s]) * gainBuf[s];
			}
		}
	protected:
		Smooth mixSmooth, gainSmooth;
		ParamBuf mixBuf, gainBuf;
		DryBuf dryBuf;
		SqrtBuf sqrtBuf;
		float gain, gainVal;
	};
}