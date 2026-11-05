#include "StrokeItem.hpp"

#include <QtCore/QLineF>
#include <QtGui/QMouseEvent>
#include <QtQuick/QSGFlatColorMaterial>
#include <QtQuick/QSGGeometryNode>

#include <algorithm>
#include <cmath>
#include <ranges>
#include <span>

namespace app::canvas {

namespace {

    constexpr double StrokeWidth = 4.0;
    constexpr double MiterLimit = 2.0;
    constexpr double MinPointDistance = 1.5;

    QPointF toQt(shapes::Point point)
    {
        return { point.x, point.y };
    }

    QPointF unit(QPointF vector)
    {
        const double length = std::hypot(vector.x(), vector.y());
        return length > 0.0 ? vector / length : QPointF{};
    }

    QPointF normal(QPointF direction)
    {
        return { -direction.y(), direction.x() };
    }

    // Two vertices per point, offset along the miter of the adjoining segments, so corners stay sharp.
    QSGGeometryNode *createStrokeNode(std::span<const shapes::Point> points, const QColor &color)
    {
        const double half = StrokeWidth / 2.0;
        // A single point, from a click, becomes a dot.
        std::vector<shapes::Point> dot;
        if (points.size() == 1) {
            dot = {
                { .x = points[0].x - half, .y = points[0].y },
                { .x = points[0].x + half, .y = points[0].y }
            };
            points = dot;
        }

        const auto count = points.size();
        auto *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(),
            static_cast<int>(count * 2));
        geometry->setDrawingMode(QSGGeometry::DrawTriangleStrip);
        const std::span vertices(geometry->vertexDataAsPoint2D(), count * 2);
        const bool closed = count > 2 && points.front() == points.back();
        for (std::size_t i = 0; i < count; ++i) {
            // A closed stroke wraps around; an open one has no segment before its start.
            const auto previous = i > 0 ? points[i - 1] : closed ? points[count - 2] : points[i];
            const auto next = i + 1 < count ? points[i + 1] : closed ? points[1] : points[i];
            const auto point = toQt(points[i]);
            auto in = unit(point - toQt(previous));
            auto out = unit(toQt(next) - point);
            if (in.isNull())
                in = out;
            if (out.isNull())
                out = in;
            auto tangent = unit(in + out);
            if (tangent.isNull()) // the stroke turns back on itself
                tangent = in;
            const auto miter = normal(tangent);
            const double cosine = QPointF::dotProduct(miter, normal(in));
            const double length = std::min(half / std::max(cosine, 1e-6), half * MiterLimit);
            const auto left = point + (miter * length);
            const auto right = point - (miter * length);
            vertices[2 * i].set(static_cast<float>(left.x()), static_cast<float>(left.y()));
            vertices[(2 * i) + 1].set(static_cast<float>(right.x()), static_cast<float>(right.y()));
        }

        auto *node = new QSGGeometryNode;
        node->setGeometry(geometry);
        node->setFlag(QSGNode::OwnsGeometry);
        auto *material = new QSGFlatColorMaterial;
        material->setColor(color);
        node->setMaterial(material);
        node->setFlag(QSGNode::OwnsMaterial);
        return node;
    }

} // namespace

StrokeItem::StrokeItem(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents);
    setAcceptedMouseButtons(Qt::LeftButton);
}

CanvasStore *StrokeItem::store() const
{
    return m_store;
}

void StrokeItem::setStore(CanvasStore *store)
{
    if (store == m_store)
        return;
    if (m_store)
        disconnect(m_store, nullptr, this, nullptr);
    m_store = store;
    if (m_store)
        connect(m_store, &CanvasStore::changed, this, &QQuickItem::update);
    Q_EMIT storeChanged();
    update();
}

QColor StrokeItem::penColor() const
{
    return m_penColor;
}

void StrokeItem::setPenColor(const QColor &color)
{
    if (color == m_penColor)
        return;
    m_penColor = color;
    Q_EMIT penColorChanged();
}

void StrokeItem::mousePressEvent(QMouseEvent *event)
{
    const auto position = event->position();
    m_live = {
        { .x = position.x(), .y = position.y() }
    };
    // A drag draws: a surrounding Flickable must not take it over to scroll.
    setKeepMouseGrab(true);
    update();
}

void StrokeItem::mouseMoveEvent(QMouseEvent *event)
{
    const auto position = event->position();
    if (m_live.empty() || QLineF(toQt(m_live.back()), position).length() < MinPointDistance)
        return;
    m_live.push_back({ .x = position.x(), .y = position.y() });
    update();
}

void StrokeItem::mouseReleaseEvent(QMouseEvent *)
{
    QList<QPointF> points;
    points.reserve(static_cast<qsizetype>(m_live.size()));
    for (const auto point : m_live)
        points.append(toQt(point));
    m_live.clear();
    update();
    Q_EMIT strokeFinished(points);
}

// Another handler took over, for example a two-finger pinch: drop the unfinished stroke.
void StrokeItem::mouseUngrabEvent()
{
    m_live.clear();
    update();
}

QSGNode *StrokeItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    // Runs on the render thread while the GUI thread waits, so reading the store is safe here.
    auto *root = oldNode;
    if (!root) {
        root = new QSGNode;
        m_rendered.clear();
    }
    static const Sketch none;
    const Sketch &strokes = m_store ? m_store->sketch() : none;

    // A stroke never changes, so its node stays valid while the same stroke is in its place.
    const auto keep = static_cast<int>(std::ranges::mismatch(m_rendered, strokes).in1
        - m_rendered.begin());
    while (root->childCount() > keep) {
        auto *node = root->lastChild();
        root->removeChildNode(node);
        delete node;
    }
    for (const auto &stroke : strokes | std::views::drop(keep))
        root->appendChildNode(createStrokeNode(stroke->points, QColor::fromString(stroke->color)));
    m_rendered = strokes;

    // The stroke being drawn is rebuilt on every update and removed on the next one.
    if (!m_live.empty())
        root->appendChildNode(createStrokeNode(m_live, m_penColor));
    return root;
}

} // namespace app::canvas
