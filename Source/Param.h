#pragma once
#include <juce_core/juce_core.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace param
{
	static constexpr float tau = 6.28318530718f;
	static constexpr float pi = 3.14159265359f;
	static constexpr float piHalf = 1.57079632679f;
	static constexpr float piQuart = .785398163397f;
	static constexpr float piInv = 1.f / pi;

	inline juce::String toID(const juce::String& name) { return name.toLowerCase().removeCharacters(" "); }

	enum class PID
	{
		Depth, Mix, Gain, StereoConfig,
		NumPlanets,
		Gravity,
		SpaceMud,
		Attraction,
		NumParams
	};

	static constexpr int NumParams = static_cast<int>(PID::NumParams);
	inline juce::String toString(PID pID)
	{
		switch (pID)
		{
		case PID::Depth: return "Depth";
		case PID::Mix: return "Mix";
		case PID::Gain: return "Gain";
		case PID::StereoConfig: return "Stereo Config";

		case PID::NumPlanets: return "Num Planets";
		case PID::Gravity: return "Gravity";
		case PID::SpaceMud: return "Space Mud";
		case PID::Attraction: return "Attraction";
		
		default: return "";
		}
	}
	inline int withOffset(PID p, int o) noexcept { return static_cast<int>(p) + o; }

	enum class Unit { Percent, Hz, Beats, Degree, Octaves, Semi, Fine, Ms, Decibel, NumUnits };
	inline juce::String toString(Unit pID)
	{
		switch (pID)
		{
		case Unit::Percent: return "%";
		case Unit::Hz: return "hz";
		case Unit::Beats: return "x";
		case Unit::Degree: return juce::CharPointer_UTF8("\xc2\xb0");
		case Unit::Octaves: return "oct";
		case Unit::Semi: return "semi";
		case Unit::Fine: return "fine";
		case Unit::Ms: return "ms";
		case Unit::Decibel: return "db";
		default: return "";
		}
	}

	using ValToStrFunc = std::function<juce::String(float)>;
	using StrToValFunc = std::function<float(const juce::String&)>;

	namespace makeRange
	{
		inline juce::NormalisableRange<float> biasXL(float start, float end, float bias) noexcept
		{
			// https://www.desmos.com/calculator/ps8q8gftcr
			const auto a = bias * .5f + .5f;
			const auto a2 = 2.f * a;
			const auto aM = 1.f - a;
			const auto r = end - start;
			const auto aR = r * a;
			if (bias != 0.f)
				return
			{
					start, end,
					[a2, aM, aR](float min, float, float x)
					{
						return min + aR * x / (aM - x + a2 * x);
					},
					[a2, aM, aR](float min, float, float x)
					{
						return aM * (x - min) / (a2 * min + aR - a2 * x - min + x);
					},
					nullptr
			};
			else return { start, end };
		}

		inline juce::NormalisableRange<float> toggle()
		{
			return { 0.f, 1.f, 1.f };
		}

		inline juce::NormalisableRange<float> stepped(float start, float end, float steps = 1.f)
		{
			return
			{
					start, end,
					[range = end - start](float min, float, float normalized)
					{
						return min + normalized * range;
					},
					[rangeInv = 1.f / (end - start)](float min, float, float denormalized)
					{
						return (denormalized - min) * rangeInv;
					},
					[steps, stepsInv = 1.f / steps](float, float, float val)
					{
						return std::rint(val * stepsInv) * steps;
					}
			};
		}
	}

	struct StateIDs
	{
		juce::Identifier state{ "state" };
		juce::Identifier params{ "params" };
		juce::Identifier param{ "param" };
		juce::Identifier name{ "name" };
		juce::Identifier value{ "value" };
	};

	struct Param :
		public juce::AudioProcessorParameter
	{
		Param(const PID pID, const juce::NormalisableRange<float>& _range, const float _valDenormDefault,
			const ValToStrFunc& _valToStr, const StrToValFunc& _strToVal,
			const Unit _unit = Unit::NumUnits) :

			juce::AudioProcessorParameter(),
			id(pID),
			range(_range),
			valDenormDefault(_valDenormDefault),
			valNorm(range.convertTo0to1(_valDenormDefault)),
			valToStr(_valToStr),
			strToVal(_strToVal),
			unit(_unit)
		{
		}

		//called by host, normalized, thread-safe
		float getValue() const override { return valNorm.load(); }
		float getValDenorm() const noexcept { return range.convertFrom0to1(getValue()); }

		// called by host, normalized, avoid locks, not used by editor
		// use setValueNotifyingHost() from the editor
		void setValue(float normalized) override
		{
			valNorm.store(normalized);
		}
		void setValueWithGesture(float norm)
		{
			beginChangeGesture();
			setValueNotifyingHost(norm);
			endChangeGesture();
		}
		void beginGesture() { beginChangeGesture(); }
		void endGesture() { endChangeGesture(); }

		float getDefaultValue() const override { return range.convertTo0to1(valDenormDefault); }

		juce::String getName(int) const override { return toString(id); }

		// units of param (hz, % etc.)
		juce::String getLabel() const override { return toString(unit); }

		// string of norm val
		juce::String getText(float norm, int) const override
		{
			return valToStr(range.snapToLegalValue(range.convertFrom0to1(norm)));
		}

		// string to norm val
		float getValueForText(const juce::String& text) const override
		{
			const auto val = juce::jlimit(range.start, range.end, strToVal(text));
			return range.convertTo0to1(val);
		}
		// string to denorm val
		float getValForTextDenorm(const juce::String& text) const { return strToVal(text); }

		juce::String _toString()
		{
			auto v = getValue();
			return getName(10) + ": " + juce::String(v) + "; " + getText(v, 10);
		}
		void dbg()
		{
			DBG(_toString());
		}

		const PID id;
		const juce::NormalisableRange<float> range;
	protected:
		const float valDenormDefault;
		std::atomic<float> valNorm;
		ValToStrFunc valToStr;
		StrToValFunc strToVal;
		Unit unit;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Param)
	};

	struct Params
	{
		Params(juce::AudioProcessor& audioProcessor) :
			params()
		{
			const auto strToValDivision = [](const juce::String& txt, const float altVal)
			{
				if (txt.contains(":") || txt.contains("/"))
				{
					for (auto i = 0; i < txt.length(); ++i)
					{
						if (txt[i] == ':' || txt[i] == '/')
						{
							const auto a = txt.substring(0, i).getFloatValue();
							const auto b = txt.substring(i + 1).getFloatValue();
							if (b != 0.f)
								return a / b;
						}
					}
				}
				return altVal;
			};
			const auto valToStrPercent = [](float v) { return juce::String(std::floor(v * 100.f)) + " " + toString(Unit::Percent); };
			const auto valToStrHz = [](float v) { return juce::String(v).substring(0, 4) + " " + toString(Unit::Hz); };
			const auto valToStrPhase = [](float v) { return juce::String(std::floor(v * 180.f)) + " " + toString(Unit::Degree); };
			const auto valToStrPhase360 = [](float v) { return juce::String(std::floor(v * 360.f)) + " " + toString(Unit::Degree); };
			const auto valToStrOct = [](float v) { return juce::String(std::floor(v)) + " " + toString(Unit::Octaves); };
			const auto valToStrOct2 = [](float v) { return juce::String(std::floor(v / 12.f)) + " " + toString(Unit::Octaves); };
			const auto valToStrSemi = [](float v) { return juce::String(std::floor(v)) + " " + toString(Unit::Semi); };
			const auto valToStrFine = [](float v) { return juce::String(std::floor(v * 100.f)) + " " + toString(Unit::Fine); };
			const auto valToStrRatio = [](float v)
			{
				const auto y = static_cast<int>(std::floor(v * 100.f));
				return juce::String(100 - y) + " : " + juce::String(y);
			};
			const auto valToStrLRMS = [](float v) { return v > .5f ? juce::String("m/s") : juce::String("l/r"); };
			const auto valToStrFreeSync = [](float v) { return v > .5f ? juce::String("sync") : juce::String("free"); };
			const auto valToStrPolarity = [](float v) { return v > .5f ? juce::String("on") : juce::String("off"); };
			const auto valToStrMs = [](float v) { return juce::String(std::floor(v * 10.f) * .1f) + " " + toString(Unit::Ms); };
			const auto valToStrDb = [](float v) { return juce::String(std::floor(v * 100.f) * .01f) + " " + toString(Unit::Decibel); };
			const auto valToStrPlanets = [](float v) { return juce::String(v).substring(0, 2); };
			const auto valToStrGravity = [](float v) { return juce::String(v).substring(0, 5) + " G"; };
			const auto valToStrEmpty = [](float) { return juce::String(""); };

			const auto strToValPercent = [strToValDivision](const juce::String& txt)
			{
				const auto val = strToValDivision(txt, 0.f);
				if (val != 0.f)
					return val;
				return txt.trimCharactersAtEnd(toString(Unit::Percent)).getFloatValue() * .01f;
			};
			const auto strToValHz = [](const juce::String& txt) { return txt.trimCharactersAtEnd(toString(Unit::Hz)).getFloatValue(); };
			const auto strToValPhase = [](const juce::String& txt) { return txt.trimCharactersAtEnd(toString(Unit::Degree)).getFloatValue(); };
			const auto strToValOct = [](const juce::String& txt) { return txt.trimCharactersAtEnd(toString(Unit::Octaves)).getIntValue(); };
			const auto strToValOct2 = [](const juce::String& txt) { return txt.trimCharactersAtEnd(toString(Unit::Octaves)).getFloatValue() / 12.f; };
			const auto strToValSemi = [](const juce::String& txt) { return txt.trimCharactersAtEnd(toString(Unit::Semi)).getIntValue(); };
			const auto strToValFine = [](const juce::String& txt) { return txt.trimCharactersAtEnd(toString(Unit::Fine)).getFloatValue() * .01f; };
			const auto strToValRatio = [strToValDivision](const juce::String& txt)
			{
				const auto val = strToValDivision(txt, -1.f);
				if (val != -1.f)
					return val;
				return juce::jlimit(0.f, 1.f, txt.getFloatValue() * .01f);
			};
			const auto strToValLRMS = [](const juce::String& txt) { return txt[0] == 'l' ? 0.f : 1.f; };
			const auto strToValFreeSync = [](const juce::String& txt) { return txt[0] == 'f' ? 0.f : 1.f; };
			const auto strToValPolarity = [](const juce::String& txt) { return txt[0] == '0' ? 0.f : 1.f; };
			const auto strToValMs = [](const juce::String& txt) { return txt.trimCharactersAtEnd(toString(Unit::Ms)).getFloatValue(); };
			const auto strToValDb = [](const juce::String& txt) { return txt.trimCharactersAtEnd(toString(Unit::Decibel)).getFloatValue(); };
			const auto strToValPlanets = [](const juce::String& txt) { return std::floor(txt.getFloatValue()); };
			const auto strToValGravity = [](const juce::String& txt) { return std::floor(txt.getFloatValue()); };

			params.push_back(new Param(PID::Depth, makeRange::biasXL(.02f, 1.f, 0.f), 1.f, valToStrPercent, strToValPercent));
			params.push_back(new Param(PID::Mix, makeRange::biasXL(0.f, 1.f, 0.f), 1.f, valToStrRatio, strToValRatio));
			params.push_back(new Param(PID::Gain, makeRange::biasXL(-120.f, 3.f, .9f), -0.f, valToStrDb, strToValDb));
			params.push_back(new Param(PID::StereoConfig, makeRange::toggle(), 1.f, valToStrLRMS, strToValLRMS));

			params.push_back(new Param(PID::NumPlanets, makeRange::stepped(2, 32, 1), 13, valToStrPlanets, strToValPlanets));
			params.push_back(new Param(PID::Gravity, makeRange::biasXL(.001f, 1.f, -.95f), .001f, valToStrGravity, strToValGravity));
			params.push_back(new Param(PID::SpaceMud, makeRange::biasXL(0.f, .3f, -.9f), 0.f, valToStrPercent, strToValPercent));
			params.push_back(new Param(PID::Attraction, makeRange::biasXL(-1.f, 1.f, 0.f), 1.f, valToStrPercent, strToValPercent));

			for (auto param : params)
				audioProcessor.addParameter(param);
		}

		void loadPatch(juce::ValueTree& state)
		{
			const StateIDs ids;

			auto childParams = state.getChildWithName(ids.params);
			if (!childParams.isValid())
				return;

			for (auto c = 0; c < childParams.getNumChildren(); ++c)
			{
				const auto childParam = childParams.getChild(c);
				if (childParam.hasType(ids.param))
				{
					const auto pName = childParam.getProperty(ids.name).toString();
					const auto pIdx = getParamIdx(pName);
					if (pIdx != -1)
					{
						const auto pVal = static_cast<float>(childParam.getProperty(ids.value));
						params[pIdx]->setValueNotifyingHost(params[pIdx]->range.convertTo0to1(pVal));
					}
				}
			}
		}
		void savePatch(juce::ValueTree& state)
		{
			const StateIDs ids;

			auto childParams = state.getChildWithName(ids.params);
			if (!childParams.isValid())
			{
				childParams = juce::ValueTree(ids.params);
				state.appendChild(childParams, nullptr);
			}

			for (auto param : params)
			{
				const auto paramID = toID(toString(param->id));
				auto childParam = childParams.getChildWithProperty(ids.name, paramID);
				if (!childParam.isValid())
				{
					childParam = juce::ValueTree(ids.param);
					childParam.setProperty(ids.name, paramID, nullptr);
					childParams.appendChild(childParam, nullptr);
				}
				childParam.setProperty(ids.value, param->getValDenorm(), nullptr);
			}
		}

		int getParamIdx(const juce::String& name) const noexcept
		{
			for (auto p = 0; p < params.size(); ++p)
			{
				const auto str = toString(params[p]->id);
				if (name == str || name == toID(str))
					return p;
			}
			return -1;
		}

		size_t numParams() const noexcept { return params.size(); }

		Param& operator[](int i) noexcept { return *params[i]; }
		const Param& operator[](int i) const noexcept { return *params[i]; }
		Param& operator[](PID pID) noexcept { return operator[](static_cast<int>(pID)); }
		const Param& operator[](PID pID) const noexcept { return operator[](static_cast<int>(pID)); }
	protected:
		std::vector<Param*> params;
	};
}