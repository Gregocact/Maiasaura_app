#pragma once

#include <QAbstractListModel>
#include <QVector>
#include <QMimeData>
#include <QStringList>

#include "Track.h"

class PlaylistModel final : public QAbstractListModel {
    Q_OBJECT
public:
    explicit PlaylistModel(QObject* parent = nullptr);

    // QAbstractItemModel
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // Drag & drop (MIME-based, reliable reorder)
    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QModelIndexList& indexes) const override;
    bool dropMimeData(const QMimeData* data, Qt::DropAction action,
                      int row, int column, const QModelIndex& parent) override;
    Qt::DropActions supportedDropActions() const override;
    Qt::DropActions supportedDragActions() const override;

    // Our API
    void setTracks(QVector<Track> tracks);
    const QVector<Track>& tracks() const;

private:
    QVector<Track> m_tracks;
};
