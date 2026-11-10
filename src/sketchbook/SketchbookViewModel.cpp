#include "SketchbookViewModel.h"

#include <QtCore/QLoggingCategory>
#include <QtCore/QUuid>

using namespace Qt::StringLiterals;

Q_STATIC_LOGGING_CATEGORY(lcSketchbook, "app.sketchbook")

namespace app::sketchbook {

SketchbookViewModel::SketchbookViewModel(SketchStorage &storage, canvas::CanvasStore &canvas,
    QObject *parent)
    : QObject(parent), m_storage(storage), m_canvas(canvas)
{
    m_model.setEntries(m_storage.list());
    connect(&m_canvas, &canvas::CanvasStore::edited, this, &SketchbookViewModel::saveOpenSketch);
}

SketchListModel *SketchbookViewModel::model()
{
    return &m_model;
}

QString SketchbookViewModel::openName() const
{
    const int row = m_model.rowOf(m_openId);
    return row < 0 ? QString() : m_model.entry(row).name;
}

void SketchbookViewModel::create()
{
    SketchEntry entry{
        .id = QUuid::createUuid().toString(QUuid::WithoutBraces),
        .name = tr("Sketch %1").arg(m_model.rowCount() + 1),
        .modified = QDateTime::currentDateTime(),
    };
    m_storage.save(entry.id, { .name = entry.name.toStdString(), .strokes = {}, .page = {} });
    qCDebug(lcSketchbook) << "Created sketch" << entry.id;
    m_model.prepend(std::move(entry));
    open(0);
}

void SketchbookViewModel::open(int row)
{
    if (row < 0 || row >= m_model.rowCount())
        return;
    const auto &entry = m_model.entry(row);
    auto document = m_storage.load(entry.id);
    if (!document) {
        qCWarning(lcSketchbook) << "Cannot open" << entry.name << document.error();
        return;
    }
    m_openId = entry.id;
    m_canvas.load({ .strokes = std::move(document->strokes), .page = document->page });
    qCDebug(lcSketchbook) << "Opened sketch" << m_openId;
    Q_EMIT openNameChanged();
}

void SketchbookViewModel::rename(int row, const QString &name)
{
    const auto trimmed = name.trimmed();
    if (row < 0 || row >= m_model.rowCount() || trimmed.isEmpty())
        return;
    auto entry = m_model.entry(row);
    auto document = m_storage.load(entry.id);
    if (!document) {
        qCWarning(lcSketchbook) << "Cannot rename" << entry.name << document.error();
        return;
    }
    document->name = trimmed.toStdString();
    m_storage.save(entry.id, std::move(*document));
    qCDebug(lcSketchbook) << "Renamed sketch" << entry.id;
    entry.name = trimmed;
    m_model.update(row, std::move(entry));
    if (m_model.entry(row).id == m_openId)
        Q_EMIT openNameChanged();
}

void SketchbookViewModel::renameOpen(const QString &name)
{
    rename(m_model.rowOf(m_openId), name);
}

void SketchbookViewModel::duplicate(int row)
{
    if (row < 0 || row >= m_model.rowCount())
        return;
    const auto &source = m_model.entry(row);
    auto document = m_storage.load(source.id);
    if (!document) {
        qCWarning(lcSketchbook) << "Cannot duplicate" << source.name << document.error();
        return;
    }
    SketchEntry copy{
        .id = QUuid::createUuid().toString(QUuid::WithoutBraces),
        .name = copyName(source.name),
        .modified = QDateTime::currentDateTime(),
        .strokeCount = static_cast<qsizetype>(document->strokes.size()),
    };
    document->name = copy.name.toStdString();
    m_storage.save(copy.id, std::move(*document));
    qCDebug(lcSketchbook) << "Duplicated sketch" << source.id << "as" << copy.id;
    m_model.prepend(std::move(copy));
}

void SketchbookViewModel::remove(int row)
{
    if (row < 0 || row >= m_model.rowCount())
        return;
    const auto id = m_model.entry(row).id;
    qCDebug(lcSketchbook) << "Removing sketch" << id;
    m_storage.remove(id);
    m_model.removeAt(row);
    if (id == m_openId) {
        m_openId.clear();
        m_canvas.load({});
        Q_EMIT openNameChanged();
    }
}

void SketchbookViewModel::saveOpenSketch()
{
    const int row = m_model.rowOf(m_openId);
    if (row < 0)
        return;
    auto entry = m_model.entry(row);
    m_storage.save(entry.id,
        {
            .name = entry.name.toStdString(),
            .strokes = m_canvas.sketch().strokes,
            .page = m_canvas.sketch().page,
        });
    entry.modified = QDateTime::currentDateTime();
    entry.strokeCount = static_cast<qsizetype>(m_canvas.sketch().strokes.size());
    m_model.update(row, std::move(entry));
}

QString SketchbookViewModel::copyName(const QString &name) const
{
    const auto stem = name + tr(" copy");
    QString candidate = stem;
    for (int number = 2;; ++number) {
        bool exists = false;
        for (int row = 0; row < m_model.rowCount(); ++row) {
            if (m_model.entry(row).name == candidate) {
                exists = true;
                break;
            }
        }
        if (!exists)
            break;
        candidate = stem + u" %1"_s.arg(number);
    }
    return candidate;
}

} // namespace app::sketchbook
