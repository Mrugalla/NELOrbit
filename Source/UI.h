#pragma once
#include "Param.h"
#include "GUIBasics.h"
#include <functional>
#include <array>

namespace orbit
{
	namespace gui
	{
        using Notify = std::function<void(const int, const void*)>;
        enum NotificationType
        {
            ParameterHovered,
            ParameterDragged,
            KillEnterValue,
            EnterValue,
            NumTypes
        };

        struct Events
        {
            struct Evt
            {
                Evt(Events& _events) :
                    events(_events),
                    notifier(nullptr)
                {
                }
                Evt(Events& _events, const Notify& _notifier) :
                    events(_events),
                    notifier(_notifier)
                {
                    events.add(this);
                }
                Evt(Events& _events, Notify&& _notifier) :
                    events(_events),
                    notifier(_notifier)
                {
                    events.add(this);
                }
                ~Evt()
                {
                    events.remove(this);
                }
                void operator()(const int type, const void* stuff = nullptr)
                {
                    events.notify(type, stuff);
                }
                Notify notifier;
                Events& getEvents() noexcept { return events; }
            protected:
                Events& events;
            };

            Events() :
                evts()
            {
            }
            void add(Evt* e) { evts.push_back(e); }
            void remove(const Evt* e)
            {
                for (auto i = 0; i < evts.size(); ++i)
                    if (e == evts[i])
                    {
                        evts.erase(evts.begin() + i);
                        return;
                    }
            }
            void notify(const int notificationType, const void* stuff = nullptr)
            {
                for (auto e : evts)
                    e->notifier(notificationType, stuff);
            }
        protected:
            std::vector<Evt*> evts;
        };

        struct Utils
        {
            using PID = param::PID;
            using Param = param::Param;
            using Params = param::Params;

            Utils(juce::Component& _pluginTop, Params& _params) :
                eventSystem(),
                font(getCustomFont()),
                pluginTop(_pluginTop),
                params(_params)
            {}

            Events eventSystem;
            const juce::Font font;

            const Param& getParam(int idx) const noexcept
            {
                return params[idx];
            }
            Param& getParam(int idx) noexcept
            {
                return params[idx];
            }
            const Param& getParam(PID pID) const noexcept
            {
                return params[static_cast<int>(pID)];
            }
            Param& getParam(PID pID) noexcept
            {
                return params[static_cast<int>(pID)];
            }

            float getDragSpeed() noexcept
            {
                static constexpr float DragSpeed = .5f;
                const auto w = static_cast<float>(pluginTop.getWidth());
                const auto speed = DragSpeed * w;
                return speed;
            }
        protected:
            juce::Component& pluginTop;
            Params& params;
        private:
            juce::Font getCustomFont() noexcept
            {
                auto f = juce::Font(juce::Typeface::createSystemTypefaceFor(BinaryData::nel19_ttf, BinaryData::nel19_ttfSize));
                return f.withExtraKerningFactor(.2f);
            }
        };

		class Comp :
			public juce::Component
		{
        public:
			Comp(Utils& _utils, Notify&& _notify = [](int, const void*){}) :
                utils(_utils),
                notify(utils.eventSystem, _notify)
			{}
            Utils utils;
            Events::Evt notify;
        protected:
            void paint(juce::Graphics& g) override
            {
                g.setColour(juce::Colours::red);
                g.drawRect(getLocalBounds().toFloat());
            }
		};

        struct ASR
        {
            enum class State { A, S, R };
            ASR(float _atk, float _rls, float fps) :
                env(0.f),
                atk(1.f / (_atk * fps)),
                rls(1.f / (_rls * fps)),
                state(State::R)
            {
            }
            float operator()(bool e) noexcept
            {
                switch (state)
                {
                case State::A:
                    if (e)
                        if (env >= 1.f)
                        {
                            env = 1.f;
                            state = State::S;
                        }
                        else
                            env += atk;
                    else
                        state = State::R;
                    break;
                case State::S:
                    if (!e)
                        state = State::R;
                    break;
                case State::R:
                    if (e)
                        state = State::A;
                    else
                        if (env <= 0.f)
                            env = 0.f;
                        else
                            env -= rls;
                    break;
                }

                return env;
            }

            float env;
        protected:
            const float atk, rls;
            State state;
        };

        inline int getNumRows(const juce::String& txt) noexcept
        {
            auto rows = 1;
            for (auto i = 0; i < txt.length(); ++i)
                rows = txt[i] == '\n' ? rows + 1 : rows;
            return rows;
        }

        struct Label :
            public Comp
        {
            Label(Utils& u, juce::String&& _txt = "", Notify&& _notify = [](int, const void*){}) :
                Comp(u, std::move(_notify)),
                font(),
                txt(_txt),
                bgC(juce::Colours::transparentBlack),
                outlineC(juce::Colours::white),
                txtC(juce::Colours::white),
                just(juce::Justification::centred)
            {
                setInterceptsMouseClicks(false, false);
                setBufferedToImage(true);
            }

            void setText(const juce::String& t)
            {
                txt = t;
                updateBounds();
                repaint();
            }
            void setText(juce::String&& t)
            {
                setText(t);
            }
            const juce::String& getText() const noexcept { return txt; }
            juce::String& getText() noexcept { return txt; }

            juce::Font font;
            juce::Colour bgC, outlineC, txtC;
            juce::Justification just;
        protected:
            juce::String txt;

            void resized() override
            {
                updateBounds();
            }

            void paint(juce::Graphics& g) override
            {
                const auto bounds = getLocalBounds().toFloat();
                g.setFont(font);
                g.setColour(bgC);
                g.fillRoundedRectangle(bounds, 2.f);
                g.setColour(outlineC);
                g.drawRoundedRectangle(bounds, 12.f, 2.f);
                g.setColour(txtC);
                g.setImageResamplingQuality(juce::Graphics::lowResamplingQuality);
                g.drawFittedText(
                    txt, getLocalBounds(), just, 1
                );
            }

            void updateBounds()
            {
                const auto height = static_cast<float>(getHeight());
                const auto minHeight = font.getHeight() * static_cast<float>(getNumRows(txt));
                const auto dif = static_cast<int>(height - minHeight);
                if (dif < 0)
                {
                    const auto dif2 = dif / 2;
                    const auto x = getX();
                    const auto y = getY() + dif2;
                    const auto w = getWidth();
                    const auto h = getHeight() - dif;
                    setBounds(x, y, w, h);
                }
            }
        };

        static constexpr float SensitiveDrag = .15f;
        static constexpr float WheelDefaultSpeed = .02f;
        static constexpr float WheelInertia = .9f;

        class Paramtr :
            public Comp,
            public juce::Timer
        {
            using PID = param::PID;
            using Param = param::Param;

            static constexpr float Pi = 3.14159265359f;
            static constexpr float PiQuart = Pi * .25f;
            static constexpr float AngleWidth = PiQuart * 3.f;
            static constexpr float AngleRange = AngleWidth * 2.f;

        public:
            Paramtr(Utils& u, const juce::String& _name, juce::String&& _tooltip, PID _pID) :
                juce::Timer(),
                Comp(u),
                param(u.getParam(_pID)),
                label(u, _name + ": " + getValueAsText()),
                valNorm(param.getValue()), dragY(0.f)
            {
                setName(_name);
                addAndMakeVisible(label);
                startTimerHz(24);
            }

            PID getPID() const noexcept { return param.id; }
            juce::String getValueAsText() const noexcept { return param.getText(param.getValue(), 10); }
        protected:
            Param& param;
            Label label;
            
            float valNorm, dragY;

            void timerCallback() override
            {
                const auto vn = param.getValue();
                if (valNorm != vn)
                {
                    valNorm = vn;
                    label.setText(
                        getName() + ": " + getValueAsText()
                    );
                    repaint();
                }
            }
            void paint(juce::Graphics& g) override
            {
                // cool look and feel
                g.setColour(juce::Colour(0x44ffffff));
                g.fillRoundedRectangle(
                    0.f,
                    0.f,
                    static_cast<float>(getWidth()) * valNorm,
                    getHeight() * 1.f,
                    12.f
                );
            }

            void resized() override
            {
                const auto thicc = 2.f;
                const auto bounds = getLocalBounds().toFloat().reduced(thicc);
                label.setBounds(getLocalBounds());
            }
            
            void mouseEnter(const juce::MouseEvent& evt) override
            {
                Comp::mouseEnter(evt);
                this->notify(NotificationType::ParameterHovered, this);
            }
            void mouseDown(const juce::MouseEvent& evt) override
            {
                if (evt.mods.isLeftButtonDown())
                {
                    notify(NotificationType::KillEnterValue);
                    param.beginGesture();
                    dragY = evt.position.x / this->utils.getDragSpeed();
                }
            }
            void mouseDrag(const juce::MouseEvent& evt) override
            {
                if (evt.mods.isLeftButtonDown())
                {
                    const auto dragYNew = evt.position.x / this->utils.getDragSpeed();
                    auto dragOffset = dragYNew - dragY;
                    if (evt.mods.isShiftDown())
                        dragOffset *= SensitiveDrag;
                    const auto newValue = juce::jlimit(0.f, 1.f, param.getValue() + dragOffset);
                    param.setValueNotifyingHost(newValue);
                    dragY = dragYNew;
                    notify(NotificationType::ParameterDragged, this);
                }
            }
            void mouseUp(const juce::MouseEvent& evt) override
            {
                if (evt.mods.isLeftButtonDown())
                {
                    if (!evt.mouseWasDraggedSinceMouseDown())
                    {
                        if (evt.mods.isCtrlDown())
                            param.setValueNotifyingHost(param.getDefaultValue());
                        else
                        {
                            const auto v = evt.position.x / static_cast<float>(getWidth());
                            param.setValue(v);
                        }
                    }
                    param.endGesture();
                    notify(NotificationType::ParameterDragged, this);
                }
                else if (evt.mods.isRightButtonDown())
                    if (!evt.mouseWasDraggedSinceMouseDown())
                        if (evt.mods.isCtrlDown())
                            param.setValueWithGesture(param.getDefaultValue());
                        else
                            notify(NotificationType::EnterValue, this);
            }
            void mouseWheelMove(const juce::MouseEvent& evt, const juce::MouseWheelDetails& wheel) override
            {
                if (evt.mods.isAnyMouseButtonDown()) return;
                const bool reversed = wheel.isReversed ? -1.f : 1.f;
                const bool isTrackPad = wheel.deltaY * wheel.deltaY < .0549316f;
                if (isTrackPad)
                    dragY = reversed * wheel.deltaY;
                else
                {
                    const auto deltaYPos = wheel.deltaY > 0.f ? 1.f : -1.f;
                    dragY = reversed * WheelDefaultSpeed * deltaYPos;
                }
                if (evt.mods.isShiftDown())
                    dragY *= SensitiveDrag;
                const auto v = param.getValue() + dragY;
                const auto newValue = juce::jlimit(0.f, 1.f, v);
                param.setValueWithGesture(newValue);
                notify(NotificationType::ParameterDragged, this);
            }
        };

		struct UI :
			public Comp,
            public juce::Timer
		{
            using PID = param::PID;

            enum class PIdx { Depth, Mix, Gain, StereoConfig, NumPlanets, Gravity, SpaceMud, Attraction, NumParams };
            static constexpr int NumParams = static_cast<int>(PIdx::NumParams);

#define FPS 30.f
            UI(Utils& u) :
                layout(
                    { 20, 70, 20 },
                    { 5, 30, 2, 20, 20, 20, 20, 30, 30, 30, 30, 2, 5 }
                ),
				Comp(u),
                Timer(),
                asrEnv(.1f, .4f, FPS),
                title(u, "NEL\nORBIT"),

                params
                {
                    Paramtr(u, "Depth", "tooltip", PID::Depth),
                    Paramtr(u, "Mix", "tooltip", PID::Mix),
                    Paramtr(u, "Gain", "tooltip", PID::Gain),
                    Paramtr(u, "StereoConfig", "tooltip", PID::StereoConfig),
                    Paramtr(u, "Num Planets", "tooltip", PID::NumPlanets),
                    Paramtr(u, "Gravity", "tooltip", PID::Gravity),
                    Paramtr(u, "Space Mud", "tooltip", PID::SpaceMud),
                    Paramtr(u, "Attraction", "tooltip", PID::Attraction)
                }
			{
                title.font = u.font;
                addAndMakeVisible(title);
                startTimerHz(static_cast<int>(FPS));
                for (auto& p : params)
                    addAndMakeVisible(p);
            }
#undef FPS
		protected:
            ui::Layout layout;
            ASR asrEnv;
            Label title;

            std::array<Paramtr, NumParams> params;

            void resized() override
            {
                const auto margin = 2.f;
                layout.setBounds(getLocalBounds().toFloat());
                layout.place(title, 1, 1, 1, 1, 0.f, false);
                for (auto p = 0; p < NumParams; ++p)
                    layout.place(params[p], 0, 3 + p, 3, 1, margin, false);
            }
            void paint(juce::Graphics& g) override
            {
                g.fillAll(juce::Colour(0xff000000));
            }
            void timerCallback() override
            {
                bool isHovering = isMouseOverOrDragging(true);
                const auto env = asrEnv(isHovering);
                setAlpha(env * .6f);
            }
		};
	}
}