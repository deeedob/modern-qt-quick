#pragma once

#include <shapes/Recognizer.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

// The drawing as plain C++: the state, the actions that change it, and the reducer.
namespace app::canvas {

struct Stroke
{
    std::vector<shapes::Point> points;
    std::string color; // "#rrggbb", readable in the saved file
    double width = 4.0;
};

// A stroke never changes once drawn, so every snapshot of the sketch shares it.
using StrokePtr = std::shared_ptr<const Stroke>;

enum class PaperBackground : std::uint8_t { Plain, Dots, Ruled };

[[nodiscard]] constexpr std::string_view paperBackgroundName(PaperBackground background)
{
    switch (background) {
    case PaperBackground::Plain:
        return "plain";
    case PaperBackground::Dots:
        return "dots";
    case PaperBackground::Ruled:
        return "ruled";
    }
    return "plain";
}

[[nodiscard]] constexpr std::optional<PaperBackground> paperBackgroundFromName(
    std::string_view name)
{
    if (name == "plain")
        return PaperBackground::Plain;
    if (name == "dots")
        return PaperBackground::Dots;
    if (name == "ruled")
        return PaperBackground::Ruled;
    return std::nullopt;
}

struct Page
{
    int width = 1600;
    int height = 1200;
    PaperBackground background = PaperBackground::Plain;
    int spacing = 32;
    int thickness = 1;

    bool operator==(const Page &) const = default;
};

struct Sketch
{
    std::vector<StrokePtr> strokes;
    Page page;

    [[nodiscard]] bool empty() const { return strokes.empty(); }
    bool operator==(const Sketch &) const = default;
};

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
    struct SetPage
    {
        Page page;
    };
} // namespace action

using Action = std::variant<action::AddStroke, action::ReplaceLastStroke, action::Clear,
    action::SetPage>;

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
            [&](const action::AddStroke &add) { sketch.strokes.push_back(add.stroke); },
            [&](const action::ReplaceLastStroke &replace) {
                if (!sketch.strokes.empty())
                    sketch.strokes.back() = replace.stroke;
            },
            [&](action::Clear) { sketch.strokes.clear(); },
            [&](const action::SetPage &set) { sketch.page = set.page; },
        },
        action);
    return sketch;
}

} // namespace app::canvas
