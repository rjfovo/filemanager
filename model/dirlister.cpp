/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
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

#include "dirlister.h"
#include <KIO/ListJob>

DirLister::DirLister(QObject *parent)
    : QObject(parent)
{
}

DirLister::~DirLister()
{
}

void DirLister::openUrl(const QUrl &url)
{
    KIO::ListJob *job = KIO::listDir(url);
    connect(job, &KIO::ListJob::entries, this, &DirLister::handleEntries);
    connect(job, &KJob::result, this, &DirLister::handleResult);
}

void DirLister::handleEntries(KIO::Job *job, const KIO::UDSEntryList &entries)
{
    Q_UNUSED(job)
    emit newItems(entries);
}

void DirLister::handleResult(KJob *job)
{
    if (job->error()) {
        handleError(job);
    } else {
        emit completed();
    }
}

void DirLister::handleError(KJob *job)
{
    emit error(job->errorString());
}