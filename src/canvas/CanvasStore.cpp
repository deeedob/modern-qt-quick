#include "CanvasStore.h"

#include <QtCore/QLoggingCategory>
#include <QtCore/QRectF>

#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>
#include <ranges>

Q_STATIC_LOGGING_CATEGORY(lcCanvas, "app.canvas")

namespace app::canvas {

namespace {

    constexpr std::size_t MaxHistory = 100;
    // Real shapes score above 0.9, scribbles and open curves below 0.8.
    constexpr double SnapScore = 0.85;

    struct NamedShape
    {
        std::string_view name;
        CanvasStore::Shape shape;
    };
    constexpr std::array Shapes{
        NamedShape{ .name = "circle",    .shape = CanvasStore::Shape::Circle    },
        NamedShape{ .name = "rectangle", .shape = CanvasStore::Shape::Rectangle },
        NamedShape{ .name = "triangle",  .shape = CanvasStore::Shape::Triangle  },
    };

    QRectF bounds(std::span<const shapes::Point> points)
    {
        const auto [minX,
            maxX] = std::ranges::minmax(points | std::views::transform(&shapes::Point::x));
        const auto [minY,
            maxY] = std::ranges::minmax(points | std::views::transform(&shapes::Point::y));
        return { QPointF(minX, minY), QPointF(maxX, maxY) };
    }

    // The clean outline of a recognized shape. Drawn into a unit square, it is also the template.
    std::vector<shapes::Point> idealShape(std::string_view name, const QRectF &box)
    {
        const auto at = [&](double x, double y) {
            return shapes::Point{ .x = box.left() + (x * box.width()),
                .y = box.top() + (y * box.height()) };
        };
        if (name == "rectangle")
            return { at(0, 0), at(1, 0), at(1, 1), at(0, 1), at(0, 0) };
        if (name == "triangle")
            return { at(0, 1), at(0.5, 0), at(1, 1), at(0, 1) };
        std::vector<shapes::Point> circle;
        for (int i = 0; i <= 64; ++i) {
            const double angle = 2.0 * std::numbers::pi * i / 64.0;
            circle.push_back(at(0.5 + (0.5 * std::cos(angle)), 0.5 + (0.5 * std::sin(angle))));
        }
        return circle;
    }

    // The templates are polygons with only a few points; resampling spreads them out.
    shapes::Recognizer basicShapes()
    {
        shapes::Recognizer recognizer;
        for (const auto &[name, shape] : Shapes) {
            auto outline = idealShape(name, QRectF(0, 0, 1, 1));
            recognizer.addTemplate(std::string(name), outline);
            // People draw both clockwise and counterclockwise.
            std::ranges::reverse(outline);
            recognizer.addTemplate(std::string(name), outline);
        }
        return recognizer;
    }

} // namespace

CanvasStore::CanvasStore(QObject *parent)
    : QObject(parent), m_recognizer(basicShapes())
{
}

const Sketch &CanvasStore::sketch() const
{
    return m_history.at(m_current);
}

void CanvasStore::load(Sketch sketch)
{
    m_history = { std::move(sketch) };
    m_current = 0;
    Q_EMIT changed();
}

bool CanvasStore::canUndo() const
{
    return m_current > 0;
}

bool CanvasStore::canRedo() const
{
    return m_current + 1 < m_history.size();
}

bool CanvasStore::isEmpty() const
{
    return sketch().empty();
}

int CanvasStore::strokeCount() const
{
    return static_cast<int>(sketch().strokes.size());
}

int CanvasStore::pageWidth() const
{
    return sketch().page.width;
}

int CanvasStore::pageHeight() const
{
    return sketch().page.height;
}

int CanvasStore::paperBackgroundValue() const
{
    return static_cast<int>(sketch().page.background);
}

bool CanvasStore::snapShapes() const
{
    return m_snapShapes;
}

void CanvasStore::setSnapShapes(bool snap)
{
    if (snap == m_snapShapes)
        return;
    m_snapShapes = snap;
    Q_EMIT snapShapesChanged();
}

void CanvasStore::addStroke(const QList<QPointF> &points, const QColor &color, double width)
{
    if (points.isEmpty())
        return;
    const auto toPoint = [](QPointF p) {
        return shapes::Point{ .x = p.x(), .y = p.y() };
    };
    auto stroke = std::make_shared<const Stroke>(Stroke{
        .points = points | std::views::transform(toPoint) | std::ranges::to<std::vector>(),
        .color = color.name().toStdString(),
        .width = std::clamp(width, 1.0, 12.0),
    });
    dispatch(action::AddStroke{ stroke });

    if (!m_snapShapes)
        return;
    const auto match = m_recognizer.recognize(stroke->points);
    if (!match || match->score < SnapScore)
        return;
    qCDebug(lcCanvas) << "Snapping to" << match->name << "with score" << match->score;
    dispatch(action::ReplaceLastStroke{ std::make_shared<const Stroke>(Stroke{
        .points = idealShape(match->name, bounds(stroke->points)),
        .color = stroke->color,
        .width = stroke->width,
    }) });
    Q_EMIT shapeSnapped(std::ranges::find(Shapes, match->name, &NamedShape::name)->shape);
}

void CanvasStore::clear()
{
    dispatch(action::Clear{});
}

void CanvasStore::setPageSize(int width, int height)
{
    if (width <= 0 || height <= 0)
        return;
    auto page = sketch().page;
    page.width = width;
    page.height = height;
    dispatch(action::SetPage{ page });
}

void CanvasStore::setPaperBackground(int background)
{
    if (background < static_cast<int>(PaperBackground::Plain)
        || background > static_cast<int>(PaperBackground::Ruled))
        return;
    setPaperBackground(static_cast<PaperBackground>(background));
}

void CanvasStore::setPaperBackground(PaperBackground background)
{
    auto page = sketch().page;
    page.background = background;
    dispatch(action::SetPage{ page });
}

void CanvasStore::undo()
{
    if (canUndo()) {
        moveTo(m_current - 1);
        Q_EMIT historyMoved(false);
    }
}

void CanvasStore::redo()
{
    if (canRedo()) {
        moveTo(m_current + 1);
        Q_EMIT historyMoved(true);
    }
}

void CanvasStore::dispatch(const Action &action)
{
    auto next = reduce(sketch(), action);
    if (next == sketch())
        return;
    // A new edit makes the undone steps unreachable.
    m_history.resize(m_current + 1);
    m_history.push_back(std::move(next));
    if (m_history.size() > MaxHistory)
        m_history.erase(m_history.begin());
    moveTo(m_history.size() - 1);
}

void CanvasStore::moveTo(std::size_t index)
{
    m_current = index;
    Q_EMIT changed();
    Q_EMIT edited();
}

} // namespace app::canvas
