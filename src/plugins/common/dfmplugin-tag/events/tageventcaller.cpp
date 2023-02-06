/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "tageventcaller.h"

#include "dfm-base/dfm_event_defines.h"

#include <dfm-framework/dpf.h>

#include <QUrl>
#include <QRectF>
#include <QWidget>
#include <QAbstractItemView>

Q_DECLARE_METATYPE(QRectF *)
Q_DECLARE_METATYPE(QPoint *)

using namespace dfmplugin_tag;
DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE

void TagEventCaller::sendOpenWindow(const QUrl &url)
{
    dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, url);
}

void TagEventCaller::sendOpenTab(quint64 windowId, const QUrl &url)
{
    dpfSignalDispatcher->publish(GlobalEventType::kOpenNewTab, windowId, url);
}

void TagEventCaller::sendOpenFiles(const quint64 windowID, const QList<QUrl> &urls)
{
    dpfSignalDispatcher->publish(GlobalEventType::kOpenFiles, windowID, urls);
}

void TagEventCaller::sendFileUpdate(const QString &path)
{
    QUrl fileUrl = QUrl::fromLocalFile(path);
    dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_FileUpdate", fileUrl);
    dpfSlotChannel->push("ddplugin_canvas", "slot_FileInfoModel_UpdateFile", fileUrl);
}

bool TagEventCaller::sendCheckTabAddable(quint64 windowId)
{
    return dpfSlotChannel->push("dfmplugin_workspace", "slot_Tab_Addable", windowId).toBool();
}

QRectF TagEventCaller::getVisibleGeometry(const quint64 windowID)
{
    const QVariant &ret = dpfSlotChannel->push("dfmplugin_workspace", "slot_View_GetVisualGeometry", windowID);
    return ret.toRectF();
}

QRectF TagEventCaller::getItemRect(const quint64 windowID, const QUrl &url, const ItemRoles role)
{
    const QVariant &ret = dpfSlotChannel->push("dfmplugin_workspace", "slot_View_GetViewItemRect", windowID, url, role);
    return ret.toRectF();
}

QAbstractItemView *TagEventCaller::getDesktopView(int viewIdx)
{
    const QVariant &ret = dpfSlotChannel->push("ddplugin_canvas", "slot_CanvasManager_View", viewIdx);
    return ret.value<QAbstractItemView *>();
}

QAbstractItemView *TagEventCaller::getCollectionView(const QString &id)
{
    const QVariant &ret = dpfSlotChannel->push("ddplugin_organizer", "slot_CollectionView_View", id);
    return ret.value<QAbstractItemView *>();
}

int TagEventCaller::getDesktopViewIndex(const QString &url, QPoint *pos)
{
    return dpfSlotChannel->push("ddplugin_canvas", "slot_CanvasGrid_Point", url, pos).toInt();
}

QString TagEventCaller::getCollectionViewId(const QUrl &url, QPoint *pos)
{
    return dpfSlotChannel->push("ddplugin_organizer", "slot_CollectionView_GridPoint", url, pos).toString();
}

QRect TagEventCaller::getVisualRect(int viewIndex, const QUrl &url)
{
    return dpfSlotChannel->push("ddplugin_canvas", "slot_CanvasView_VisualRect", viewIndex, url).toRect();
}

QRect TagEventCaller::getCollectionVisualRect(const QString &id, const QUrl &url)
{
    return dpfSlotChannel->push("ddplugin_organizer", "slot_CollectionView_VisualRect", id, url).toRect();
}

QRect TagEventCaller::getIconRect(int viewIndex, QRect visualRect)
{
    return dpfSlotChannel->push("ddplugin_canvas", "slot_CanvasItemDelegate_IconRect", viewIndex, visualRect).toRect();
}

QRect TagEventCaller::getCollectionIconRect(const QString &id, QRect visualRect)
{
    return dpfSlotChannel->push("ddplugin_organizer", "slot_CollectionItemDelegate_IconRect", id, visualRect).toRect();
}
