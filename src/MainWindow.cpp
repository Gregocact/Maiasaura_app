#include "MainWindow.h"

#include <QAbstractItemView>
#include <QFileDialog>
#include <QListView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>

#include "PlaylistModel.h"
#include "Scanner.h"

static QString makeStamp() {
    return QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);

    m_chooseFolderBtn = new QPushButton("Choose folder…", central);
    m_saveOrderBtn = new QPushButton("Save order to SD", central);
    m_saveOrderBtn->setEnabled(false); // disabled until folder loaded
    m_listView = new QListView(central);

    layout->addWidget(m_chooseFolderBtn);
    layout->addWidget(m_saveOrderBtn);
    layout->addWidget(m_listView);
    setCentralWidget(central);

    // Create the playlist model once and attach it to the view
    m_model = new PlaylistModel(this);
    m_listView->setModel(m_model);

    // List view config (one-time)
    m_listView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_listView->setDragEnabled(true);
    m_listView->setAcceptDrops(true);
    m_listView->setDropIndicatorShown(true);
    m_listView->setDragDropMode(QAbstractItemView::InternalMove);
    m_listView->setDefaultDropAction(Qt::MoveAction);
    m_listView->setDragDropOverwriteMode(false);

    connect(m_chooseFolderBtn, &QPushButton::clicked, this, [this]() {
        const QString dir = QFileDialog::getExistingDirectory(this, "Select SD folder");
        if (dir.isEmpty())
            return;

        auto tracks = Scanner::scanFolder(dir);
        m_model->setTracks(std::move(tracks));

        m_currentDir = dir;
        m_saveOrderBtn->setEnabled(m_model->rowCount() > 0);

        setWindowTitle(QString("Maiasaura Playlist Tool — %1 files")
                           .arg(m_model->rowCount()));
    });

    connect(m_saveOrderBtn, &QPushButton::clicked, this, [this]() {
        if (m_currentDir.isEmpty() || m_model->rowCount() == 0)
            return;

        // Prevent double-click / re-entry
        m_saveOrderBtn->setEnabled(false);

        applyOrderToFolder(m_currentDir, m_model->tracks());

        // Re-enable if folder still loaded
        m_saveOrderBtn->setEnabled(!m_currentDir.isEmpty() && m_model->rowCount() > 0);
    });

    setWindowTitle("Maiasaura Playlist Tool");
    resize(700, 500);
}

void MainWindow::applyOrderToFolder(const QString& folder, const QVector<Track>& ordered) {
    if (folder.isEmpty()) {
        QMessageBox::warning(this, "Save order", "No folder selected.");
        return;
    }
    if (ordered.isEmpty()) {
        QMessageBox::warning(this, "Save order", "No tracks to save.");
        return;
    }

    QDir srcDir(folder);
    if (!srcDir.exists()) {
        QMessageBox::critical(this, "Save order", "Folder does not exist:\n" + folder);
        return;
    }

    QFileInfo srcInfo(folder);
    QDir parentDir(srcInfo.absolutePath());
    const QString folderName = srcInfo.fileName();

    const QString stamp = makeStamp();
    const QString tmpName = ".maiasaura_tmp_" + stamp;
    const QString bakName = ".maiasaura_backup_" + stamp;

    const QString tmpPath = parentDir.absoluteFilePath(tmpName);
    const QString bakPath = parentDir.absoluteFilePath(bakName);
    const QString srcPath = parentDir.absoluteFilePath(folderName);

    if (QFileInfo::exists(tmpPath) || QFileInfo::exists(bakPath)) {
        QMessageBox::critical(this, "Save order",
                              "Temp/backup folder already exists.\nTry again.");
        return;
    }

    // 1) Create temp folder
    if (!parentDir.mkdir(tmpName)) {
        QMessageBox::critical(this, "Save order",
                              "Cannot create temp folder:\n" + tmpPath);
        return;
    }

    QDir tmpDir(tmpPath);

    // 2) Copy files in the chosen order into temp folder
    for (int i = 0; i < ordered.size(); ++i) {
        const QString srcFile = ordered[i].absPath;
        const QString fileName = ordered[i].fileName;
        const QString dstFile = tmpDir.absoluteFilePath(fileName);

        if (!QFileInfo::exists(srcFile)) {
            tmpDir.removeRecursively();
            QMessageBox::critical(this, "Save order",
                                  "Missing source file:\n" + srcFile);
            return;
        }

        // Prevent overwriting (duplicate names)
        if (QFileInfo::exists(dstFile)) {
            tmpDir.removeRecursively();
            QMessageBox::critical(this, "Save order",
                                  "Duplicate filename detected:\n" + fileName +
                                      "\n\nTwo files with the same name cannot coexist.");
            return;
        }

        if (!QFile::copy(srcFile, dstFile)) {
            tmpDir.removeRecursively();
            QMessageBox::critical(this, "Save order",
                                  "Failed to copy:\n" + srcFile + "\n\nto:\n" + dstFile);
            return;
        }
    }

    // 3) Rename original folder -> backup
    if (!parentDir.rename(srcPath, bakPath)) {
        tmpDir.removeRecursively();
        QMessageBox::critical(this, "Save order",
                              "Failed to rename original folder to backup.\n\n"
                              "Original:\n" + srcPath + "\n\nBackup:\n" + bakPath);
        return;
    }

    // 4) Rename temp -> original name
    if (!parentDir.rename(tmpPath, srcPath)) {
        // Rollback: restore original folder name
        parentDir.rename(bakPath, srcPath);
        tmpDir.removeRecursively();
        QMessageBox::critical(this, "Save order",
                              "Failed to activate the new folder.\n"
                              "Rolled back to the original.");
        return;
    }

    // 5) Update model paths to point to the new files
    QVector<Track> updated = ordered;
    for (auto& t : updated) {
        t.absPath = QDir(srcPath).absoluteFilePath(t.fileName);
    }
    m_model->setTracks(std::move(updated));
    m_currentDir = srcPath;

    QMessageBox::information(this, "Save order",
                             "Order saved successfully.\n\nBackup kept at:\n" + bakPath);
}
