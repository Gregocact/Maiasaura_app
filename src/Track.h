#pragma once

#include <QString>
#include <QDateTime>

struct Track {
    QString absPath;
    QString fileName;
    qint64 sizeBytes = 0;
    QDateTime modified;
};
