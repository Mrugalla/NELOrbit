#pragma once
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_extra/juce_gui_extra.h>

namespace gui
{
    template<typename Float>
    inline juce::Rectangle<Float> maxQuadIn(const juce::Rectangle<Float>& b) noexcept
    {
        const auto minDimen = std::min(b.getWidth(), b.getHeight());
        const auto x = b.getX() + static_cast<Float>(.5) * (b.getWidth() - minDimen);
        const auto y = b.getY() + static_cast<Float>(.5) * (b.getHeight() - minDimen);
        return { x, y, minDimen, minDimen };
    }
}