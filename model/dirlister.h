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

#ifndef DIRLISTER_H
#define DIRLISTER_H

#include <QObject>
#include <KIO/ListJob>
#include <KIO/Job>

class DirLister : public QObject
{
    Q_OBJECT

public:
    explicit DirLister(QObject *parent = nullptr);
    ~DirLister() override;

    void openUrl(const QUrl &url);

private Q_SLOTS:
    void handleEntries(KIO::Job *job, const KIO::UDSEntryList &entries);
    void handleResult(KJob *job);
    void handleError(KJob *job);

Q_SIGNALS:
    void error(const QString &string);
    void newItems(const KIO::UDSEntryList &entries);
    void completed();
};

#endif // DIRLISTER_H