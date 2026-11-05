#pragma once

#include <optional>
#include <span>
#include <string>
#include <vector>

namespace shapes {

struct Point
{
    double x = 0.0;
    double y = 0.0;

    friend bool operator==(const Point &, const Point &) = default;
};

struct Match
{
    std::string name;
    double score = 0.0; // 1.0 is a perfect match
};

// The $1 Unistroke Recognizer (Wobbrock et al. 2007): matching independent of position, size, rotation.
class Recognizer
{
public:
    // Throws std::invalid_argument for a template without extent, such as a single point.
    void addTemplate(std::string name, std::span<const Point> points);

    // Empty when there are no templates or the stroke has no extent to compare.
    [[nodiscard]] std::optional<Match> recognize(std::span<const Point> points) const;

private:
    struct Template
    {
        std::string name;
        std::vector<Point> points;
    };

    std::vector<Template> m_templates;
};

} // namespace shapes
