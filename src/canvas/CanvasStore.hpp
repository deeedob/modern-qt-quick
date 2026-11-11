#pragma once

#include "Sketch.hpp"

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QPointF>
#include <QtCore/QString>
#include <QtGui/QColor>
#include <QtQmlIntegration/qqmlintegration.h>

namespace app::canvas {

// Flux store: reduce() applies every action, and keeping each result makes undo and redo trivial.
class CanvasStore : public QObject
{
    Q_OBJECT
    // Not QML_ELEMENT: Qt 6.12's qmltyperegistrar then checks the namespace-qualified name and
    // wrongly warns that a type starting with a lowercase letter cannot export its enums.
    QML_NAMED_ELEMENT(CanvasStore)
    QML_UNCREATABLE("Created in main() and passed to Main.qml as a required property")
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY changed FINAL)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY changed FINAL)
    Q_PROPERTY(bool empty READ isEmpty NOTIFY changed FINAL)
    Q_PROPERTY(int strokeCount READ strokeCount NOTIFY changed FINAL)
    Q_PROPERTY(int pointCount READ pointCount NOTIFY changed FINAL)
    Q_PROPERTY(int pageWidth READ pageWidth NOTIFY changed FINAL)
    Q_PROPERTY(int pageHeight READ pageHeight NOTIFY changed FINAL)
    Q_PROPERTY(
        app::canvas::PaperBackground paperBackground READ paperBackground NOTIFY changed FINAL)
    Q_PROPERTY(int paperSpacing READ paperSpacing NOTIFY changed FINAL)
    Q_PROPERTY(int paperThickness READ paperThickness NOTIFY changed FINAL)
    Q_PROPERTY(bool snapShapes READ snapShapes WRITE setSnapShapes NOTIFY snapShapesChanged FINAL)

public:
    enum class Shape : quint8 { Circle, Rectangle, Triangle };
    Q_ENUM(Shape)

    explicit CanvasStore(QObject *parent = nullptr);

    [[nodiscard]] const Sketch &sketch() const;
    // Starts a new history: opening a sketch is not an edit and cannot be undone.
    void load(Sketch sketch);

    [[nodiscard]] bool canUndo() const;
    [[nodiscard]] bool canRedo() const;
    [[nodiscard]] bool isEmpty() const;
    [[nodiscard]] int strokeCount() const;
    [[nodiscard]] int pointCount() const;
    [[nodiscard]] int pageWidth() const;
    [[nodiscard]] int pageHeight() const;
    [[nodiscard]] PaperBackground paperBackground() const;
    [[nodiscard]] int paperSpacing() const;
    [[nodiscard]] int paperThickness() const;
    [[nodiscard]] bool snapShapes() const;
    void setSnapShapes(bool snap);

    // A recognized shape is replaced by its clean version, as a second undoable step.
    Q_INVOKABLE void addStroke(const QList<QPointF> &points, const QColor &color,
        double width = 4.0);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void setPageSize(int width, int height);
    Q_INVOKABLE void setPaperBackground(PaperBackground background);
    Q_INVOKABLE void setPaperPattern(int spacing, int thickness);
    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();

Q_SIGNALS:
    void changed();
    // User edits only, including undo and redo: what needs saving.
    void edited();
    void snapShapesChanged();
    void shapeSnapped(app::canvas::CanvasStore::Shape shape);
    void historyMoved(bool redo);

private:
    void dispatch(const Action &action);
    void moveTo(std::size_t index);

    std::vector<Sketch> m_history{ Sketch{} };
    std::size_t m_current = 0;
    shapes::Recognizer m_recognizer;
    bool m_snapShapes = true;
};

} // namespace app::canvas
