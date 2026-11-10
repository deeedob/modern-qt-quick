#include <shapes/Recognizer.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>
#include <ranges>
#include <stdexcept>

// The numbered steps follow the pseudocode in the appendix of the $1 paper.
namespace shapes {

namespace {

    constexpr std::size_t SampleCount = 64;
    constexpr double SquareSize = 250.0;
    constexpr double HalfDiagonal = 0.5 * SquareSize * std::numbers::sqrt2;
    constexpr double AngleRange = std::numbers::pi / 4.0;
    constexpr double AnglePrecision = std::numbers::pi / 90.0;
    constexpr double Phi = std::numbers::phi - 1.0;

    double distance(Point a, Point b)
    {
        return std::hypot(b.x - a.x, b.y - a.y);
    }

    double pathLength(std::span<const Point> points)
    {
        double length = 0.0;
        for (std::size_t i = 1; i < points.size(); ++i)
            length += distance(points[i - 1], points[i]);
        return length;
    }

    Point centroid(std::span<const Point> points)
    {
        Point sum;
        for (const auto point : points) {
            sum.x += point.x;
            sum.y += point.y;
        }
        const auto count = static_cast<double>(points.size());
        return { .x = sum.x / count, .y = sum.y / count };
    }

    // Step 1: SampleCount points, evenly spaced along the path, however fast it was drawn.
    std::vector<Point> resample(std::span<const Point> points)
    {
        const double interval = pathLength(points) / static_cast<double>(SampleCount - 1);
        std::vector<Point> result{ points.front() };
        result.reserve(SampleCount);
        double carried = 0.0;
        Point previous = points.front();
        for (const auto current : points.subspan(1)) {
            double step = distance(previous, current);
            while (carried + step >= interval && result.size() < SampleCount) {
                const double t = (interval - carried) / step;
                previous = { .x = previous.x + (t * (current.x - previous.x)),
                    .y = previous.y + (t * (current.y - previous.y)) };
                result.push_back(previous);
                step = distance(previous, current);
                carried = 0.0;
            }
            carried += step;
            previous = current;
        }
        // Rounding can leave the last sample short.
        result.resize(SampleCount, points.back());
        return result;
    }

    std::vector<Point> rotateBy(std::span<const Point> points, double angle)
    {
        const auto center = centroid(points);
        const double cos = std::cos(angle);
        const double sin = std::sin(angle);
        return points | std::views::transform([&](Point p) {
            const double dx = p.x - center.x;
            const double dy = p.y - center.y;
            return Point{ .x = (dx * cos) - (dy * sin) + center.x,
                .y = (dx * sin) + (dy * cos) + center.y };
        }) | std::ranges::to<std::vector>();
    }

    // Unlike the paper, scale before rotating: an ellipse then matches a circle, a tall rectangle a square.
    std::vector<Point> normalize(std::span<const Point> points)
    {
        if (points.size() < 2)
            return {};
        const auto [minX, maxX] = std::ranges::minmax(points | std::views::transform(&Point::x));
        const auto [minY, maxY] = std::ranges::minmax(points | std::views::transform(&Point::y));
        const double width = maxX - minX;
        const double height = maxY - minY;
        if (width <= 0.0 || height <= 0.0)
            return {};
        auto scaled = points | std::views::transform([&](Point p) {
            return Point{ .x = p.x * SquareSize / width, .y = p.y * SquareSize / height };
        }) | std::ranges::to<std::vector>();

        // Step 1 after scaling, so the samples are spread evenly over the square shape.
        auto result = resample(scaled);

        // Step 2: rotate so the line from the centroid to the first point is horizontal.
        const auto center = centroid(result);
        result = rotateBy(result,
            -std::atan2(center.y - result.front().y, center.x - result.front().x));

        // Step 3: translate to the origin (the paper's scaling happened above).
        const auto rotatedCenter = centroid(result);
        for (auto &point : result) {
            point.x -= rotatedCenter.x;
            point.y -= rotatedCenter.y;
        }
        return result;
    }

    double pathDistance(std::span<const Point> a, std::span<const Point> b)
    {
        double sum = 0.0;
        for (std::size_t i = 0; i < a.size(); ++i)
            sum += distance(a[i], b[i]);
        return sum / static_cast<double>(a.size());
    }

    // Step 4: the indicative angle is only an estimate, so search ±45° around it for the
    // rotation that fits best. A golden-section search needs few evaluations.
    double distanceAtBestAngle(std::span<const Point> points, std::span<const Point> templ)
    {
        const auto distanceAt = [&](double angle) {
            return pathDistance(rotateBy(points, angle), templ);
        };
        double from = -AngleRange;
        double to = AngleRange;
        double x1 = (Phi * from) + ((1.0 - Phi) * to);
        double x2 = ((1.0 - Phi) * from) + (Phi * to);
        double f1 = distanceAt(x1);
        double f2 = distanceAt(x2);
        while (to - from > AnglePrecision) {
            if (f1 < f2) {
                to = x2;
                x2 = x1;
                f2 = f1;
                x1 = (Phi * from) + ((1.0 - Phi) * to);
                f1 = distanceAt(x1);
            } else {
                from = x1;
                x1 = x2;
                f1 = f2;
                x2 = ((1.0 - Phi) * from) + (Phi * to);
                f2 = distanceAt(x2);
            }
        }
        return std::min(f1, f2);
    }

} // namespace

void Recognizer::addTemplate(std::string name, std::span<const Point> points)
{
    auto normalized = normalize(points);
    if (normalized.empty())
        throw std::invalid_argument("shape template without extent: " + name);
    m_templates.push_back({ .name = std::move(name), .points = std::move(normalized) });
}

std::optional<Match> Recognizer::recognize(std::span<const Point> points) const
{
    const auto candidate = normalize(points);
    if (candidate.empty() || m_templates.empty())
        return std::nullopt;

    const Template *best = nullptr;
    double bestDistance = std::numeric_limits<double>::infinity();
    for (const auto &templ : m_templates) {
        if (const double d = distanceAtBestAngle(candidate, templ.points); d < bestDistance) {
            bestDistance = d;
            best = &templ;
        }
    }
    return Match{ .name = best->name, .score = 1.0 - (bestDistance / HalfDiagonal) };
}

} // namespace shapes
