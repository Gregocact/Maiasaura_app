#pragma once

#include <QMainWindow>
#include <QString>
#include <QVector>

#include "Track.h"

class QListView;
class QPushButton;
class PlaylistModel;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    void applyOrderToFolder(const QString& folder, const QVector<Track>& ordered);

    QPushButton* m_chooseFolderBtn = nullptr;
    QPushButton* m_saveOrderBtn = nullptr;
    QListView* m_listView = nullptr;

    QString m_currentDir;            // folder currently loaded
    PlaylistModel* m_model = nullptr;
};
