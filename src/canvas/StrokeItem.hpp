#pragma once

#include "CanvasStore.hpp"

#include <QtCore/QPointer>
#include <QtGui/QColor>
#include <QtQuick/QQuickItem>

namespace app::canvas {

// Draws the store's strokes with the scene graph. A drawn stroke is a signal; QML makes it an action.
class StrokeItem : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT
    // Fully qualified: moc records the type as written, and QML tooling looks up the full name.
    Q_PROPERTY(app::canvas::CanvasStore *store READ store WRITE setStore NOTIFY storeChanged FINAL)
    Q_PROPERTY(QColor penColor READ penColor WRITE setPenColor NOTIFY penColorChanged FINAL)

public:
    explicit StrokeItem(QQuickItem *parent = nullptr);

    [[nodiscard]] CanvasStore *store() const;
    void setStore(CanvasStore *store);
    [[nodiscard]] QColor penColor() const;
    void setPenColor(const QColor &color);

Q_SIGNALS:
    void storeChanged();
    void penColorChanged();
    void strokeFinished(const QList<QPointF> &points);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data) override;

private:
    QPointer<CanvasStore> m_store;
    QColor m_penColor = Qt::black;
    std::vector<shapes::Point> m_live;
    // Kept alive so a freed stroke's address cannot be reused and fool updatePaintNode()'s check.
    Sketch m_rendered;
};

} // namespace app::canvas
