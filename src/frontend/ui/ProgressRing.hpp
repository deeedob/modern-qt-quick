// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0

#pragma once

#include <QtGui/QColor>
#include <QtQuick/QQuickItem>

namespace app::ui {

// Scene graph drawing: properties live on the GUI thread, updatePaintNode() runs on the render thread.
class ProgressRing : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(double progress READ progress WRITE setProgress NOTIFY progressChanged FINAL)
    Q_PROPERTY(double thickness READ thickness WRITE setThickness NOTIFY thicknessChanged FINAL)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged FINAL)
    Q_PROPERTY(QColor trackColor READ trackColor WRITE setTrackColor NOTIFY trackColorChanged FINAL)

public:
    explicit ProgressRing(QQuickItem *parent = nullptr);

    [[nodiscard]] double progress() const;
    void setProgress(double progress);
    [[nodiscard]] double thickness() const;
    void setThickness(double thickness);
    [[nodiscard]] QColor color() const;
    void setColor(const QColor &color);
    [[nodiscard]] QColor trackColor() const;
    void setTrackColor(const QColor &color);

signals:
    void progressChanged();
    void thicknessChanged();
    void colorChanged();
    void trackColorChanged();

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data) override;
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;

private:
    double m_progress = 0.0;
    double m_thickness = 12.0;
    QColor m_color = Qt::black;
    QColor m_trackColor = Qt::lightGray;
};

} // namespace app::ui
