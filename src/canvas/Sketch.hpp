#pragma once

#include <shapes/Recognizer.hpp>

#include <memory>
#include <string>
#include <variant>
#include <vector>

// The drawing as plain C++: the state, the actions that change it, and the reducer.
namespace app::canvas {

struct Stroke
{
    std::vector<shapes::Point> points;
    std::string color; // "#rrggbb", readable in the saved file
};

// A stroke never changes once drawn, so every snapshot of the sketch shares it.
using StrokePtr = std::shared_ptr<const Stroke>;
using Sketch = std::vector<StrokePtr>;

namespace action {
    struct AddStroke
    {
        StrokePtr stroke;
    };
    struct ReplaceLastStroke
    {
        StrokePtr stroke;
    };
    struct Clear
    {
    };
} // namespace action

using Action = std::variant<action::AddStroke, action::ReplaceLastStroke, action::Clear>;

template <typename... Fs>
struct Overloaded : Fs...
{
    using Fs::operator()...;
};

// A pure function: the same sketch and action always give the same new sketch.
[[nodiscard]] inline Sketch reduce(Sketch sketch, const Action &action)
{
    std::visit(
        Overloaded{
            [&](const action::AddStroke &add) { sketch.push_back(add.stroke); },
            [&](const action::ReplaceLastStroke &replace) {
                if (!sketch.empty())
                    sketch.back() = replace.stroke;
            },
            [&](action::Clear) { sketch.clear(); },
        },
        action);
    return sketch;
}

} // namespace app::canvas
