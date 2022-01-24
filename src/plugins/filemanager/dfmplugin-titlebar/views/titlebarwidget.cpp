/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "titlebarwidget.h"

#include "events/titlebareventcaller.h"
#include "utils/crumbinterface.h"
#include "utils/crumbmanager.h"
#include "utils/devicemanager.h"
#include "services/common/dfm_common_service_global.h"
#include "services/common/dialog/dialogservice.h"
#include "dfm-framework/framework.h"

#include <QHBoxLayout>

DPTITLEBAR_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

static DSC_NAMESPACE::DialogService *dlgServ()
{
    auto &ctx = dpfInstance.serviceContext();
    auto dlgServ = ctx.service<DSC_NAMESPACE::DialogService>(DSC_NAMESPACE::DialogService::name());
    return dlgServ;
}

TitleBarWidget::TitleBarWidget(QFrame *parent)
    : AbstractFrame(parent)
{
    initializeUi();
    initConnect();
}

void TitleBarWidget::setCurrentUrl(const QUrl &url)
{
    titlebarUrl = url;
    emit currentUrlChanged(url);
}

QUrl TitleBarWidget::currentUrl() const
{
    return titlebarUrl;
}

NavWidget *TitleBarWidget::navWidget() const
{
    return curNavWidget;
}

void TitleBarWidget::handleHotkeyCtrlF()
{
    onSearchButtonClicked();
}

void TitleBarWidget::handleHotkeyCtrlL()
{
    showAddrsssBar(currentUrl());
}

void TitleBarWidget::initializeUi()
{
    setFocusPolicy(Qt::NoFocus);

    // layout
    titleBarLayout = new QHBoxLayout(this);
    titleBarLayout->setMargin(0);
    titleBarLayout->setSpacing(0);

    // nav
    curNavWidget = new NavWidget;
    titleBarLayout->addSpacing(14);   // 导航栏左侧间隔14
    titleBarLayout->addWidget(curNavWidget, 0, Qt::AlignLeft);

    // address
    addressBar = new AddressBar;
    addressBar->setFixedHeight(36);
    addressBar->installEventFilter(this);
    titleBarLayout->addSpacing(4);
    titleBarLayout->addWidget(addressBar);

    // crumb
    crumbBar = new CrumbBar;
    titleBarLayout->addWidget(crumbBar);

    // search button
    searchButton = new QToolButton;
    searchButton->setFixedSize({ 36, 36 });
    searchButton->setFocusPolicy(Qt::NoFocus);
    searchButton->setIcon(QIcon::fromTheme("search"));
    searchButton->setIconSize({ 16, 16 });
    titleBarLayout->addWidget(searchButton);

    // search filter button
    searchFilterButton = new QToolButton;
    searchFilterButton->setFixedSize({ 36, 36 });
    searchFilterButton->setFocusPolicy(Qt::NoFocus);
    searchFilterButton->setIcon(QIcon::fromTheme("dfview_filter"));
    searchFilterButton->setIconSize({ 16, 16 });
    titleBarLayout->addWidget(searchFilterButton);

    // option button
    optionButtonBox = new OptionButtonBox;
    titleBarLayout->addSpacing(4);
    titleBarLayout->addWidget(optionButtonBox, 0, Qt::AlignRight);

    setLayout(titleBarLayout);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    showCrumbBar();
}

void TitleBarWidget::initConnect()
{
    connect(searchButton, &QToolButton::clicked,
            this, &TitleBarWidget::onSearchButtonClicked);

    connect(this, &TitleBarWidget::currentUrlChanged, optionButtonBox, &OptionButtonBox::onUrlChanged);
    connect(this, &TitleBarWidget::currentUrlChanged, crumbBar, &CrumbBar::onUrlChanged);
    connect(this, &TitleBarWidget::currentUrlChanged, curNavWidget, &NavWidget::onUrlChanged);
    connect(crumbBar, &CrumbBar::hideAddressBar, addressBar, &AddressBar::hide);
    connect(crumbBar, &CrumbBar::selectedUrl, this, [this](const QUrl &url) {
        TitleBarEventCaller::sendCd(this, url);
    });
    connect(crumbBar, &CrumbBar::editUrl, this, &TitleBarWidget::showAddrsssBar);

    connect(addressBar, &AddressBar::escKeyPressed, this, [this]() {
        if (crumbBar->controller())
            crumbBar->controller()->processAction(CrumbInterface::kEscKeyPressed);
    });
    connect(addressBar, &AddressBar::lostFocus, this, [this]() {
        if (crumbBar->controller())
            crumbBar->controller()->processAction(CrumbInterface::kAddressBarLostFocus);
    });
    connect(addressBar, &AddressBar::clearButtonPressed, this, [this]() {
        if (crumbBar->controller())
            crumbBar->controller()->processAction(CrumbInterface::kClearButtonPressed);
    });
    connect(addressBar, &AddressBar::editingFinishedSearch, this, [this](const QString &keyword) {
        TitleBarEventCaller::sendSearch(this, keyword);
    });
    connect(addressBar, &AddressBar::editingFinishedUrl, this, &TitleBarWidget::onAddressBarJump);
    // TODO(zhangs): addressbar pause
}

void TitleBarWidget::showAddrsssBar(const QUrl &url)
{
    crumbBar->hide();
    addressBar->show();
    addressBar->setFocus();
    addressBar->setCurrentUrl(url);
}

void TitleBarWidget::showCrumbBar()
{
    showSearchButton();

    if (crumbBar)
        crumbBar->show();

    if (addressBar) {
        addressBar->clear();
        addressBar->hide();
    }
    setFocus();
}

void TitleBarWidget::showSearchButton()
{
    if (searchButton)
        searchButton->show();
    if (searchFilterButton)
        searchFilterButton->hide();
}

void TitleBarWidget::showSearchFilterButton()
{
    if (searchButton)
        searchButton->hide();
    if (searchFilterButton)
        searchFilterButton->show();
}

bool TitleBarWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this && event->type() == QEvent::Show) {
        activateWindow();
        return false;
    }

    if (watched == addressBar && event->type() == QEvent::Hide) {
        showCrumbBar();
        return true;
    }

    return false;
}

bool TitleBarWidget::handleConnection(const QUrl &url)
{
    QString &&scheme = url.scheme();
    if (scheme != SchemeTypes::kSmb && scheme != SchemeTypes::kFtp && scheme != SchemeTypes::kSFtp)
        return false;

    // TODO(xust) MAY BE TEMP CODE
    if (url.host().isEmpty()) {
        dlgServ()->showErrorDialog("", tr("Mounting device error"));
        return true;
    }

    DeviceManagerInstance.invokeMountNetworkDevice(url.toString(), [](const QString &address) {
        // TODO(xust) TEMP CODE. for now, connection can only be connected, we should use api directly to mount and enter shares
        return dlgServ()->askInfoWhenMountingNetworkDevice(address);
    });

    return true;
}

void TitleBarWidget::onSearchButtonClicked()
{
    showAddrsssBar(QUrl());
    searchButton->hide();
}

void TitleBarWidget::onAddressBarJump(const QUrl &url)
{
    const AbstractFileInfoPointer &info = InfoFactory::create<AbstractFileInfo>(url);
    if (info && info->exists() && info->isFile()) {
        TitleBarEventCaller::sendOpenFile(this, url);
    } else {
        if (handleConnection(url))
            return;

        const QString &currentDir = QDir::currentPath();
        if (titlebarUrl.isLocalFile())
            QDir::setCurrent(titlebarUrl.toLocalFile());
        QDir::setCurrent(currentDir);
        TitleBarEventCaller::sendCd(this, url);
    }
}
