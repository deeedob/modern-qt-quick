// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0

#include "ProgressRing.hpp"

#include <QtQuick/QSGFlatColorMaterial>
#include <QtQuick/QSGGeometryNode>

#include <algorithm>
#include <cmath>
#include <numbers>
#include <span>

namespace app::ui {

namespace {

    constexpr double SegmentsPerTurn = 128;

    // One arc as a triangle strip, clockwise from 12 o'clock. `node` owns geometry and material.
    QSGGeometryNode *updateArc(QSGGeometryNode *node, const QRectF &bounds, double thickness,
        double fraction, const QColor &color)
    {
        if (!node) {
            node = new QSGGeometryNode;
            auto *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 0);
            geometry->setDrawingMode(QSGGeometry::DrawTriangleStrip);
            node->setGeometry(geometry);
            node->setFlag(QSGNode::OwnsGeometry);
            node->setMaterial(new QSGFlatColorMaterial);
            node->setFlag(QSGNode::OwnsMaterial);
        }

        const auto segments = static_cast<std::size_t>(std::max(1.0,
            std::ceil(SegmentsPerTurn * fraction)));
        auto *geometry = node->geometry();
        geometry->allocate(fraction > 0.0 ? static_cast<int>((segments + 1) * 2) : 0);

        const auto center = bounds.center();
        const auto outer = std::min(bounds.width(), bounds.height()) / 2.0;
        const auto inner = std::max(0.0, outer - thickness);
        const std::span vertices(geometry->vertexDataAsPoint2D(),
            static_cast<std::size_t>(geometry->vertexCount()));
        for (std::size_t i = 0; 2 * i + 1 < vertices.size(); ++i) {
            const double angle = (2.0 * std::numbers::pi * fraction * static_cast<double>(i)
                                     / static_cast<double>(segments))
                - (std::numbers::pi / 2.0);
            const auto point = [&](double radius) {
                return QPointF(center.x() + (radius * std::cos(angle)),
                    center.y() + (radius * std::sin(angle)));
            };
            const auto outerPoint = point(outer);
            const auto innerPoint = point(inner);
            vertices[2 * i].set(static_cast<float>(outerPoint.x()),
                static_cast<float>(outerPoint.y()));
            vertices[(2 * i) + 1].set(static_cast<float>(innerPoint.x()),
                static_cast<float>(innerPoint.y()));
        }
        node->markDirty(QSGNode::DirtyGeometry);

        auto *material = static_cast<QSGFlatColorMaterial *>(node->material());
        if (material->color() != color) {
            material->setColor(color);
            node->markDirty(QSGNode::DirtyMaterial);
        }
        return node;
    }

} // namespace

ProgressRing::ProgressRing(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents);
    setImplicitSize(160, 160);
}

double ProgressRing::progress() const
{
    return m_progress;
}

void ProgressRing::setProgress(double progress)
{
    progress = std::clamp(progress, 0.0, 1.0);
    if (qFuzzyCompare(progress, m_progress))
        return;
    m_progress = progress;
    emit progressChanged();
    update();
}

double ProgressRing::thickness() const
{
    return m_thickness;
}

void ProgressRing::setThickness(double thickness)
{
    thickness = std::max(thickness, 1.0);
    if (qFuzzyCompare(thickness, m_thickness))
        return;
    m_thickness = thickness;
    emit thicknessChanged();
    update();
}

QColor ProgressRing::color() const
{
    return m_color;
}

void ProgressRing::setColor(const QColor &color)
{
    if (color == m_color)
        return;
    m_color = color;
    emit colorChanged();
    update();
}

QColor ProgressRing::trackColor() const
{
    return m_trackColor;
}

void ProgressRing::setTrackColor(const QColor &color)
{
    if (color == m_trackColor)
        return;
    m_trackColor = color;
    emit trackColorChanged();
    update();
}

QSGNode *ProgressRing::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    // Runs on the render thread while the GUI thread is blocked: reading members is safe here.
    auto *root = oldNode ? oldNode : new QSGNode;
    auto *track = static_cast<QSGGeometryNode *>(root->firstChild());
    auto *arc = track ? static_cast<QSGGeometryNode *>(track->nextSibling()) : nullptr;

    track = updateArc(track, boundingRect(), m_thickness, 1.0, m_trackColor);
    arc = updateArc(arc, boundingRect(), m_thickness, m_progress, m_color);
    if (!oldNode) {
        root->appendChildNode(track);
        root->appendChildNode(arc);
    }
    return root;
}

void ProgressRing::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);
    update();
}

} // namespace app::ui
