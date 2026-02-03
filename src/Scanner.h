#pragma once

#include <QVector>
#include <QString>

#include "Track.h"

class Scanner {
public:
    static QVector<Track> scanFolder(const QString& folderPath);
};
