#include "Scanner.h"

#include <QDirIterator>
#include <QFileInfo>

QVector<Track> Scanner::scanFolder(const QString& folderPath) {
    QVector<Track> result;

    const QStringList filters = {
        "*.mp3", "*.wav", "*.m4a", "*.aac", "*.ogg", "*.flac"
    };

    QDirIterator it(
        folderPath,
        filters,
        QDir::Files | QDir::Readable,
        QDirIterator::NoIteratorFlags
    );

    while (it.hasNext()) {
        it.next();
        QFileInfo info = it.fileInfo();

        Track t;
        t.absPath = info.absoluteFilePath();
        t.fileName = info.fileName();
        t.sizeBytes = info.size();
        t.modified = info.lastModified();

        result.push_back(t);
    }

    return result;
}
