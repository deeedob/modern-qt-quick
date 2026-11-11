#pragma once

#include <Sketch.hpp>

#include <QtCore/QDateTime>
#include <QtCore/QString>
#include <QtCore/QThreadPool>

#include <expected>
#include <string>
#include <vector>

namespace app::sketchbook {

// What a sketch file holds. glaze reads and writes it by reflection, with no mapping code.
struct SketchDocument
{
    std::string name;
    std::vector<canvas::StrokePtr> strokes;
    canvas::Page page;
};

struct SketchEntry
{
    QString id;
    QString name;
    QDateTime modified;
    qsizetype strokeCount = 0;
};

// One JSON file per sketch. Saves run in order on a worker thread; every other call waits for them.
class SketchStorage
{
public:
    explicit SketchStorage(QString directory);

    // Newest first. Files that cannot be read are logged and left out.
    [[nodiscard]] std::vector<SketchEntry> list();
    [[nodiscard]] std::expected<SketchDocument, QString> load(const QString &id);
    void save(const QString &id, SketchDocument document);
    void remove(const QString &id);
    void waitForSaves();

private:
    [[nodiscard]] QString filePath(const QString &id) const;

    QString m_directory;
    QThreadPool m_writer;
};

} // namespace app::sketchbook
