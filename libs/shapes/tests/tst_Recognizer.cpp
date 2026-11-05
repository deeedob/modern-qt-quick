#include <shapes/Recognizer.hpp>

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>
#include <utility>
#include <vector>

using shapes::Point;
using shapes::Recognizer;

namespace {

struct Box
{
    double x = 0.0;
    double y = 0.0;
    double width = 1.0;
    double height = 1.0;
};

std::vector<Point> circle(Box box, double startAngle = 0.0)
{
    std::vector<Point> points;
    for (int i = 0; i <= 48; ++i) {
        const double angle = startAngle + (2.0 * std::numbers::pi * i / 48.0);
        points.push_back({ box.x + (box.width / 2.0 * (1.0 + std::cos(angle))),
            box.y + (box.height / 2.0 * (1.0 + std::sin(angle))) });
    }
    return points;
}

// Corners joined by straight lines, with points every few units like a real pointer stroke.
std::vector<Point> polyline(std::span<const Point> corners)
{
    std::vector<Point> points;
    for (std::size_t i = 0; i + 1 < corners.size(); ++i) {
        for (int step = 0; step < 12; ++step) {
            const double t = step / 12.0;
            points.push_back({ corners[i].x + (t * (corners[i + 1].x - corners[i].x)),
                corners[i].y + (t * (corners[i + 1].y - corners[i].y)) });
        }
    }
    points.push_back(corners.back());
    return points;
}

std::vector<Point> rectangle(Box b)
{
    const std::vector<Point> corners{
        { b.x,           b.y            },
        { b.x + b.width, b.y            },
        { b.x + b.width, b.y + b.height },
        { b.x,           b.y + b.height },
        { b.x,           b.y            }
    };
    return polyline(corners);
}

std::vector<Point> triangle(Box b)
{
    const std::vector<Point> corners{
        { b.x,                   b.y + b.height },
        { b.x + (b.width / 2.0), b.y            },
        { b.x + b.width,         b.y + b.height },
        { b.x,                   b.y + b.height }
    };
    return polyline(corners);
}

std::vector<Point> reversed(std::vector<Point> points)
{
    std::ranges::reverse(points);
    return points;
}

// Hand-drawn wobble of up to `amount`, as a fixed pattern so the tests stay deterministic.
std::vector<Point> jittered(std::vector<Point> points, double amount)
{
    for (std::size_t i = 0; i < points.size(); ++i) {
        points[i].x += amount * std::sin(static_cast<double>(i) * 12.9898);
        points[i].y += amount * std::cos(static_cast<double>(i) * 78.233);
    }
    return points;
}

Recognizer basicShapes()
{
    Recognizer recognizer;
    for (const auto &[name, points] : {
             std::pair{ "circle",    circle({})    },
             std::pair{ "rectangle", rectangle({}) },
             std::pair{ "triangle",  triangle({})  }
    }) {
        recognizer.addTemplate(name, points);
        recognizer.addTemplate(name, reversed(points));
    }
    return recognizer;
}

} // namespace

TEST_CASE("Recognizes each shape however it is placed, sized and drawn")
{
    const auto recognizer = basicShapes();
    const Box box = GENERATE(Box{ 0, 0, 100, 100 }, Box{ 300, 40, 220, 120 },
        Box{ -50, 10, 30, 60 });
    const bool clockwise = GENERATE(true, false);
    const auto drawn = [&](const std::vector<Point> &points) {
        return jittered(clockwise ? points : reversed(points), box.width * 0.03);
    };

    CHECK(recognizer.recognize(drawn(circle(box, 1.0)))->name == "circle");
    CHECK(recognizer.recognize(drawn(rectangle(box)))->name == "rectangle");
    CHECK(recognizer.recognize(drawn(triangle(box)))->name == "triangle");
}

TEST_CASE("A template matches itself perfectly")
{
    const auto recognizer = basicShapes();
    const auto match = recognizer.recognize(circle({ 10, 10, 50, 50 }));
    REQUIRE(match);
    CHECK(match->score > 0.99);
}

TEST_CASE("A scribble scores lower than a real shape")
{
    const auto recognizer = basicShapes();
    const std::vector<Point> zigzag{
        { 0,   0  },
        { 20,  80 },
        { 40,  0  },
        { 60,  80 },
        { 80,  0  },
        { 100, 80 }
    };
    const auto scribble = recognizer.recognize(polyline(zigzag));
    const auto shape = recognizer.recognize(jittered(circle({ 0, 0, 100, 100 }), 3.0));
    REQUIRE(scribble);
    REQUIRE(shape);
    CHECK(scribble->score < shape->score - 0.1);
}

TEST_CASE("Strokes without extent are not recognized")
{
    const auto recognizer = basicShapes();
    CHECK_FALSE(recognizer.recognize({}));
    CHECK_FALSE(recognizer.recognize(std::vector<Point>{
        { 5, 5 }
    }));
    CHECK_FALSE(recognizer.recognize(std::vector<Point>{
        { 5, 5 },
        { 5, 5 }
    }));
}

TEST_CASE("Nothing is recognized without templates")
{
    CHECK_FALSE(Recognizer{}.recognize(circle({})));
}

TEST_CASE("Templates need an extent")
{
    Recognizer recognizer;
    CHECK_THROWS_AS(
        recognizer.addTemplate("dot",
            std::vector<Point>{
                { 1, 1 }
    }),
        std::invalid_argument);
}

// Hidden from normal test runs; run with: tst_shapes "[benchmark]"
TEST_CASE("Recognition speed", "[.][benchmark]")
{
    const auto recognizer = basicShapes();
    const auto stroke = jittered(circle({ 0, 0, 400, 300 }), 8.0);
    BENCHMARK("recognize a 49-point stroke against 6 templates")
    {
        return recognizer.recognize(stroke);
    };
}
