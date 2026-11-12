#include <QtCore/QBuffer>
#include <QtCore/QCoreApplication>
#include <QtCore/QDataStream>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QSaveFile>
#include <QtCore/QTextStream>
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtSvg/QSvgRenderer>

#include <array>

using namespace Qt::StringLiterals;

namespace {
QByteArray render(QSvgRenderer &svg, int size)
{
    QImage image(size, size, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    svg.render(&painter);
    painter.end();
    QByteArray png;
    QBuffer buffer(&png);
    if (!buffer.open(QIODevice::WriteOnly) || !image.save(&buffer, "PNG"))
        return {};
    return png;
}

bool writeIco(QSvgRenderer &svg, const QString &path)
{
    constexpr std::array sizes{ 16, 32, 48, 64, 128, 256 };
    std::array<QByteArray, sizes.size()> images;
    for (size_t i = 0; i < sizes.size(); ++i) {
        images[i] = render(svg, sizes[i]);
        if (images[i].isEmpty())
            return false;
    }

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    QDataStream out(&file);
    out.setByteOrder(QDataStream::LittleEndian);
    out << quint16(0) << quint16(1) << quint16(sizes.size());
    auto offset = quint32(6 + 16 * sizes.size());
    for (size_t i = 0; i < sizes.size(); ++i) {
        const auto dimension = quint8(sizes[i] == 256 ? 0 : sizes[i]);
        const auto length = quint32(images[i].size());
        out << dimension << dimension << quint8(0) << quint8(0) << quint16(1) << quint16(32)
            << length << offset;
        offset += length;
    }
    for (const QByteArray &image : images)
        out.writeRawData(image.constData(), image.size());
    return out.status() == QDataStream::Ok && file.commit();
}

bool writeIconset(QSvgRenderer &svg, const QString &path)
{
    const QDir directory(path);
    if (!directory.mkpath(u"."_s))
        return false;
    for (const int size : { 16, 32, 128, 256, 512 }) {
        for (const int scale : { 1, 2 }) {
            const QByteArray png = render(svg, size * scale);
            const QString name = u"icon_%1x%1%2.png"_s.arg(size).arg(scale == 2 ? u"@2x"_s : u""_s);
            QSaveFile file(directory.filePath(name));
            if (png.isEmpty() || !file.open(QIODevice::WriteOnly) || file.write(png) != png.size()
                || !file.commit())
                return false;
        }
    }
    return true;
}
} // namespace

int main(int argc, char *argv[])
{
    const QCoreApplication app(argc, argv);
    const QStringList arguments = QCoreApplication::arguments();
    if (arguments.size() != 3) {
        QTextStream(stderr) << "Usage: app_icon_generator input.svg output.ico|output.iconset\n";
        return 1;
    }

    QSvgRenderer svg(arguments[1]);
    const QString &output = arguments[2];
    const QString extension = QFileInfo(output).suffix();
    const bool ok = svg.isValid()
        && (extension == u"ico"_s ? writeIco(svg, output)
                                  : extension == u"iconset"_s && writeIconset(svg, output));
    if (!ok)
        QTextStream(stderr)
            << "Failed to generate icon from " << arguments[1] << " to " << output << '\n';
    return ok ? 0 : 1;
}
