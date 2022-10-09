/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#include "generalmodelfilter.h"

#include "filters/hiddenfilefilter.h"
#include "filters/innerdesktopappfilter.h"

using namespace ddplugin_organizer;
GeneralModelFilter::GeneralModelFilter() : ModelDataHandler()
{
    // the default filters
    modelFilters << QSharedPointer<ModelDataHandler>(new HiddenFileFilter());
    modelFilters << QSharedPointer<ModelDataHandler>(new InnerDesktopAppFilter());
}

bool GeneralModelFilter::installFilter(const QSharedPointer<ModelDataHandler> &filter)
{
    if (filter.get() && modelFilters.contains(filter))
        return false;
    modelFilters.append(filter);
    return true;
}

void GeneralModelFilter::removeFilter(const QSharedPointer<ModelDataHandler> &filter)
{
    modelFilters.removeAll(filter);
}

bool GeneralModelFilter::acceptInsert(const QUrl &url)
{
    for (const auto &filter : modelFilters)
        if (!filter->acceptInsert(url))
            return false;

    return ModelDataHandler::acceptInsert(url);
}

QList<QUrl> GeneralModelFilter::acceptReset(const QList<QUrl> &urls)
{
    QList<QUrl> ret = urls;
    for (const auto &filter : modelFilters)
         ret = filter->acceptReset(ret);

    return ModelDataHandler::acceptReset(ret);
}

bool GeneralModelFilter::acceptRename(const QUrl &oldUrl, const QUrl &newUrl)
{
    bool ret = true;
    for (const auto &filter : modelFilters)
        ret = ret && filter->acceptRename(oldUrl, newUrl);

    return ModelDataHandler::acceptRename(oldUrl, newUrl) && ret;
}

bool GeneralModelFilter::acceptUpdate(const QUrl &url)
{
    // these filters is like Notifier.
    // so it will don't interrupt when some one return true.
    bool ret = true;
    for (const auto &filter : modelFilters)
        ret = ret && filter->acceptUpdate(url);

    return ModelDataHandler::acceptUpdate(url) && ret;
}
