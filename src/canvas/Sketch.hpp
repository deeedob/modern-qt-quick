#pragma once

#include <shapes/Recognizer.h>

#include <QtCore/QObject>
#include <QtQmlIntegration/qqmlintegration.h>

#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace app::canvas {

Q_NAMESPACE
QML_NAMED_ELEMENT(Paper)

struct Stroke
{
    std::vector<shapes::Point> points;
    std::string color; // "#rrggbb", readable in the saved file
    double width = 4.0;
};

// Immutable strokes let undo and redo share point data across history snapshots.
using StrokePtr = std::shared_ptr<const Stroke>;

enum class PaperBackground { Plain, Dots, Ruled };
Q_ENUM_NS(PaperBackground)

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

// A value snapshot makes every action produce a history entry without inverse operations.
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
