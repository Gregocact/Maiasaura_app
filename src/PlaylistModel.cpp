#include "PlaylistModel.h"

#include <QDataStream>
#include <QMimeData>
#include <QIODevice>

static constexpr const char* kMimeType = "application/x-maiasaura-track-row";

PlaylistModel::PlaylistModel(QObject* parent)
    : QAbstractListModel(parent) {}

int PlaylistModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return m_tracks.size();
}

QVariant PlaylistModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return {};
    const int row = index.row();
    if (row < 0 || row >= m_tracks.size()) return {};

    const auto& t = m_tracks[row];

    if (role == Qt::DisplayRole) {
        return t.fileName;
    }
    return {};
}

Qt::ItemFlags PlaylistModel::flags(const QModelIndex& index) const {
    auto f = QAbstractListModel::flags(index);
    // Enable dragging for valid items; enable dropping on the view
    if (index.isValid()) {
        f |= Qt::ItemIsDragEnabled;
    }
    f |= Qt::ItemIsDropEnabled;
    return f;
}

Qt::DropActions PlaylistModel::supportedDropActions() const {
    return Qt::MoveAction;
}
Qt::DropActions PlaylistModel::supportedDragActions() const {
    return Qt::MoveAction;
}

QStringList PlaylistModel::mimeTypes() const {
    return { kMimeType };
}

QMimeData* PlaylistModel::mimeData(const QModelIndexList& indexes) const {
    if (indexes.isEmpty()) return nullptr;

    // v1: only support one selected row
    const int row = indexes.first().row();

    auto* mime = new QMimeData();
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out << row;
    mime->setData(kMimeType, payload);
    return mime;
}

bool PlaylistModel::dropMimeData(const QMimeData* data, Qt::DropAction action,
                                 int row, int /*column*/, const QModelIndex& parent) {
    if (action != Qt::MoveAction) return false;
    if (!data || !data->hasFormat(kMimeType)) return false;
    if (parent.isValid()) return false; // list model root only

    QByteArray payload = data->data(kMimeType);
    QDataStream in(&payload, QIODevice::ReadOnly);

    int sourceRow = -1;
    in >> sourceRow;

    if (sourceRow < 0 || sourceRow >= m_tracks.size()) return false;

    // row == -1 means append
    int destRow = row;
    if (destRow < 0) destRow = m_tracks.size();

    // Removing shifts indices
    if (destRow > sourceRow) destRow -= 1;

    if (destRow == sourceRow) return false;

    // beginMoveRows wants destination index BEFORE removal semantics,
    // so use (row < 0 ? size : row) like before.
    beginMoveRows(QModelIndex(), sourceRow, sourceRow,
                  QModelIndex(), row < 0 ? m_tracks.size() : row);

    Track moved = m_tracks.takeAt(sourceRow);
    m_tracks.insert(destRow, moved);

    endMoveRows();
    return true;
}

void PlaylistModel::setTracks(QVector<Track> tracks) {
    beginResetModel();
    m_tracks = std::move(tracks);
    endResetModel();
}

const QVector<Track>& PlaylistModel::tracks() const {
    return m_tracks;
}
