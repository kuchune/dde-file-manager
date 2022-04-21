/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#include "filepropertydialogmanager.h"
#include "views/multifilepropertydialog.h"

#include "services/common/propertydialog/propertydialogservice.h"

#include <QApplication>
#include <QScreen>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DSC_USE_NAMESPACE
DPPROPERTYDIALOG_USE_NAMESPACE
const int kMaxPropertyDialogNumber = 16;
FilePropertyDialogManager::FilePropertyDialogManager(QObject *parent)
    : QObject(parent)
{
    closeIndicatorTimer = new QTimer(this);
    closeIndicatorTimer->setInterval(1000);
    closeAllDialog = new CloseAllDialog;
    closeAllDialog->setWindowIcon(QIcon::fromTheme("dde-file-manager"));
    connect(closeAllDialog, &CloseAllDialog::allClosed, this, &FilePropertyDialogManager::closeAllFilePropertyDialog);
    connect(closeIndicatorTimer, &QTimer::timeout, this, &FilePropertyDialogManager::updateCloseIndicator);
}

FilePropertyDialogManager::~FilePropertyDialogManager()
{
    filePropertyDialogs.clear();

    if (closeAllDialog) {
        closeAllDialog->deleteLater();
    }

    devicePropertyDialogs.clear();
}

void FilePropertyDialogManager::showPropertyDialog(const QList<QUrl> &urls, int widgetFilter)
{
    for (const QUrl &url : urls) {
        QWidget *widget = createCustomizeView(url);
        if (widget) {
            widget->show();
            widget->activateWindow();
        } else {
            showFilePropertyDialog(urls, widgetFilter);
            break;
        }
    }
}

void FilePropertyDialogManager::showFilePropertyDialog(const QList<QUrl> &urls, int widgetFilter)
{
    int count = urls.count();
    if (count < kMaxPropertyDialogNumber) {
        for (const QUrl &url : urls) {
            int index = urls.indexOf(url);
            if (!filePropertyDialogs.contains(url)) {
                FilePropertyDialog *dialog = new FilePropertyDialog();
                dialog->selectFileUrl(url, widgetFilter);
                filePropertyDialogs.insert(url, dialog);
                createControlView(url);
                connect(dialog, &FilePropertyDialog::closed, this, &FilePropertyDialogManager::closeFilePropertyDialog);
                if (1 == count) {
                    QPoint pos = getPropertyPos(dialog->size().width(), dialog->height());
                    dialog->move(pos);
                } else {
                    QPoint pos = getPerportyPos(dialog->size().width(), dialog->size().height(), count, index);
                    dialog->move(pos);
                }
                dialog->show();
            } else {
                filePropertyDialogs.value(url)->show();
                filePropertyDialogs.value(url)->activateWindow();
            }
            filePropertyDialogs.value(url)->show();
        }

        if (urls.count() >= 2) {
            closeAllDialog->show();
            closeIndicatorTimer->start();
        }
    } else {
        MultiFilePropertyDialog *multiFilePropertyDialog = new MultiFilePropertyDialog(urls);
        multiFilePropertyDialog->show();
        multiFilePropertyDialog->moveToCenter();
        multiFilePropertyDialog->raise();
    }
}

/*!
 * \brief           Normal view control extension
 * \param index     Subscript to be inserted
 * \param widget    The view to be inserted
 */
void FilePropertyDialogManager::insertExtendedControlFileProperty(const QUrl &url, int index, QWidget *widget)
{
    if (widget) {
        FilePropertyDialog *dialog = nullptr;
        if (filePropertyDialogs.contains(url)) {
            dialog = filePropertyDialogs.value(url);
        } else {
            dialog = new FilePropertyDialog();
        }
        dialog->insertExtendedControl(index, widget);
    }
}

/*!
 * \brief           Normal view control extension
 * \param widget    The view to be inserted
 */
void FilePropertyDialogManager::addExtendedControlFileProperty(const QUrl &url, QWidget *widget)
{
    if (widget) {
        FilePropertyDialog *dialog = nullptr;
        if (filePropertyDialogs.contains(url)) {
            dialog = filePropertyDialogs.value(url);
        } else {
            dialog = new FilePropertyDialog();
        }
        dialog->addExtendedControl(widget);
    }
}

void FilePropertyDialogManager::closeFilePropertyDialog(const QUrl url)
{
    if (filePropertyDialogs.contains(url)) {
        filePropertyDialogs.remove(url);
    }

    if (filePropertyDialogs.isEmpty())
        closeAllDialog->close();
}

void FilePropertyDialogManager::closeAllFilePropertyDialog()
{
    QList<FilePropertyDialog *> dialogs = filePropertyDialogs.values();
    for (FilePropertyDialog *dialog : dialogs) {
        dialog->close();
    }
    closeIndicatorTimer->stop();
    closeAllDialog->close();
}

void FilePropertyDialogManager::createControlView(const QUrl &url)
{
    QMap<int, QWidget *> controlView = createView(url);
    int count = controlView.keys().count();
    for (int i = 0; i < count; ++i) {
        QWidget *view = controlView.value(controlView.keys()[i]);
        if (controlView.keys()[i] == -1) {
            addExtendedControlFileProperty(url, view);
        } else {
            insertExtendedControlFileProperty(url, controlView.keys()[i], view);
        }
    }
}

void FilePropertyDialogManager::showDevicePropertyDialog(const Property::DeviceInfo &info)
{
    if (devicePropertyDialogs.contains(info.deviceUrl)) {
        devicePropertyDialogs.value(info.deviceUrl)->show();
        devicePropertyDialogs.value(info.deviceUrl)->activateWindow();
    } else {
        DevicePropertyDialog *devicePropertyDialog = new DevicePropertyDialog;
        devicePropertyDialog->show();
        devicePropertyDialog->setSelectDeviceInfo(info);
        devicePropertyDialogs.insert(info.deviceUrl, devicePropertyDialog);
        connect(devicePropertyDialog, &DevicePropertyDialog::closed, this, &FilePropertyDialogManager::closeDevicePropertyDialog);
    }
}

void FilePropertyDialogManager::insertExtendedControlDeviceProperty(const QUrl &url, int index, QWidget *widget)
{
    if (devicePropertyDialogs.contains(url) && widget) {
        devicePropertyDialogs.value(url)->insertExtendedControl(index, widget);
    } else if (widget) {
        DevicePropertyDialog *dialog = new DevicePropertyDialog();
        devicePropertyDialogs.insert(url, dialog);
        dialog->insertExtendedControl(index, widget);
        dialog->show();
        connect(dialog, &DevicePropertyDialog::closed, this, &FilePropertyDialogManager::closeDevicePropertyDialog);
    }
}

void FilePropertyDialogManager::addExtendedControlDeviceProperty(const QUrl &url, QWidget *widget)
{
    if (devicePropertyDialogs.contains(url) && widget) {
        devicePropertyDialogs.value(url)->addExtendedControl(widget);
    } else if (widget) {
        DevicePropertyDialog *dialog = new DevicePropertyDialog();
        devicePropertyDialogs.insert(url, dialog);
        dialog->addExtendedControl(widget);
        dialog->show();
        connect(dialog, &DevicePropertyDialog::closed, this, &FilePropertyDialogManager::closeDevicePropertyDialog);
    }
}

void FilePropertyDialogManager::closeDevicePropertyDialog(const QUrl &url)
{
    if (devicePropertyDialogs.contains(url)) {
        devicePropertyDialogs.remove(url);
    }
}

void FilePropertyDialogManager::updateCloseIndicator()
{
    qint64 size { 0 };
    int fileCount { 0 };

    for (FilePropertyDialog *d : filePropertyDialogs.values()) {
        size += d->getFileSize();
        fileCount += d->getFileCount();
    }

    closeAllDialog->setTotalMessage(size, fileCount);
}

FilePropertyDialogManager *FilePropertyDialogManager::instance()
{
    static FilePropertyDialogManager propertyManager;
    return &propertyManager;
}

QMap<int, QWidget *> FilePropertyDialogManager::createView(const QUrl &url)
{
    return propertyServIns->createControlView(url);
}

QWidget *FilePropertyDialogManager::createCustomizeView(const QUrl &url)
{
    return propertyServIns->createWidget(url);
}

QPoint FilePropertyDialogManager::getPropertyPos(int dialogWidth, int dialogHeight)
{
    const QScreen *cursor_screen = Q_NULLPTR;
    const QPoint &cursor_pos = QCursor::pos();

    auto screens = qApp->screens();
    auto iter = std::find_if(screens.begin(), screens.end(), [cursor_pos](const QScreen *screen) {
        return screen->geometry().contains(cursor_pos);
    });

    if (iter != screens.end()) {
        cursor_screen = *iter;
    }

    if (!cursor_screen) {
        cursor_screen = qApp->primaryScreen();
    }

    int desktopWidth = cursor_screen->size().width();
    int desktopHeight = cursor_screen->size().height();
    int x = (desktopWidth - dialogWidth) / 2;

    int y = (desktopHeight - dialogHeight) / 2;

    return QPoint(x, y) + cursor_screen->geometry().topLeft();
}

QPoint FilePropertyDialogManager::getPerportyPos(int dialogWidth, int dialogHeight, int count, int index)
{
    Q_UNUSED(dialogHeight)
    const QScreen *cursor_screen = Q_NULLPTR;
    const QPoint &cursor_pos = QCursor::pos();

    auto screens = qApp->screens();
    auto iter = std::find_if(screens.begin(), screens.end(), [cursor_pos](const QScreen *screen) {
        return screen->geometry().contains(cursor_pos);
    });

    if (iter != screens.end()) {
        cursor_screen = *iter;
    }

    if (!cursor_screen) {
        cursor_screen = qApp->primaryScreen();
    }

    int desktopWidth = cursor_screen->size().width();
    //    int desktopHeight = cursor_screen->size().height();//后面未用，注释掉
    int SpaceWidth = 20;
    int SpaceHeight = 70;
    int row, x, y;
    int numberPerRow = desktopWidth / (dialogWidth + SpaceWidth);
    Q_ASSERT(numberPerRow != 0);
    if (count % numberPerRow == 0) {
        row = count / numberPerRow;
    } else {
        row = count / numberPerRow + 1;
    }
    Q_UNUSED(row)
    int dialogsWidth;
    if (count / numberPerRow > 0) {
        dialogsWidth = dialogWidth * numberPerRow + SpaceWidth * (numberPerRow - 1);
    } else {
        dialogsWidth = dialogWidth * (count % numberPerRow) + SpaceWidth * (count % numberPerRow - 1);
    }

    //    int dialogsHeight = dialogHeight + SpaceHeight * (row - 1);//未用注释掉

    x = (desktopWidth - dialogsWidth) / 2 + (dialogWidth + SpaceWidth) * (index % numberPerRow);

    y = 5 + (index / numberPerRow) * SpaceHeight;
    return QPoint(x, y) + cursor_screen->geometry().topLeft();
}
