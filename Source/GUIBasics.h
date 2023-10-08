#pragma once
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_extra/juce_gui_extra.h>

namespace ui
{
    template<typename Float>
    inline juce::Rectangle<Float> maxQuadIn(const juce::Rectangle<Float>& b) noexcept
    {
        const auto minDimen = std::min(b.getWidth(), b.getHeight());
        const auto x = b.getX() + static_cast<Float>(.5) * (b.getWidth() - minDimen);
        const auto y = b.getY() + static_cast<Float>(.5) * (b.getHeight() - minDimen);
        return { x, y, minDimen, minDimen };
    }

    class Layout
    {
    public:
        Layout(const std::vector<float>, const std::vector<float>);
        // SET
        /*
        * Resizes the bounds of the layout according to the total size of the given rectangle
        */
        void setBounds(const juce::Rectangle<float>&) noexcept;
        // PROCESS
        void place(juce::Component& comp, int posX, int posY, int width = 1, int height = 1, float padding = 0.f,
            bool isQuad = false);
        void place(juce::Component* comp, int posX, int posY, int width = 1, int height = 1, float padding = 0.f, 
            bool isQuad = false)
        {
            if (comp == nullptr) return;
            place(*comp, posX, posY, width, height, padding, isQuad);
        }

        juce::Rectangle<float> operator()() const noexcept;
        juce::Rectangle<float> operator()(int x, int y, int width = 1, int height = 1, float padding = 0.f,
            bool isQuad = false) const noexcept;

        juce::Rectangle<float> bottomBar() const noexcept;
        juce::Rectangle<float> topBar() const noexcept;
        juce::Rectangle<float> exceptBottomBar() const noexcept;

    private:
        std::vector<float> rXRaw, rYRaw, rX, rY;
    };
}