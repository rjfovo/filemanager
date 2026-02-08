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

#include "desktopsettings.h"

#include <QDBusServiceWatcher>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <algorithm>

DesktopSettings::DesktopSettings(QObject *parent)
    : QObject(parent)
    , m_interface("com.cutefish.Settings",
                  "/Theme", "com.cutefish.Theme",
                  QDBusConnection::sessionBus(), this)
{
    QDBusServiceWatcher *watcher = new QDBusServiceWatcher(this);
    watcher->setConnection(QDBusConnection::sessionBus());
    watcher->addWatchedService("com.cutefish.Settings");
    connect(watcher, &QDBusServiceWatcher::serviceRegistered, this, &DesktopSettings::init);

    init();
}

QString DesktopSettings::wallpaper() const
{
    // 如果从 D-Bus 获取的路径为空，使用默认壁纸
    if (m_wallpaper.isEmpty()) {
        // 首先尝试cutefishos壁纸目录
        QDir wallpaperDir("/usr/share/backgrounds/cutefishos");
        if (wallpaperDir.exists()) {
            QStringList filters;
            filters << "*.jpg" << "*.jpeg" << "*.png" << "*.bmp";
            QStringList files = wallpaperDir.entryList(filters, QDir::Files);
            if (!files.isEmpty()) {
                // 选择第一张壁纸（按文件名排序）
                files.sort();
                return wallpaperDir.absoluteFilePath(files.first());
            }
        }
        
        // 如果cutefishos目录没有壁纸，尝试其他壁纸目录
        QDir otherWallpaperDir("/usr/share/wallpapers");
        if (otherWallpaperDir.exists()) {
            // 查找任何图片文件
            QStringList filters;
            filters << "*.jpg" << "*.jpeg" << "*.png" << "*.bmp";
            
            // 递归查找所有子目录
            QDirIterator it("/usr/share/wallpapers", filters, QDir::Files, QDirIterator::Subdirectories);
            if (it.hasNext()) {
                return it.next();
            }
        }
        
        // 如果所有尝试都失败，返回空字符串
        return QString();
    }
    
    return m_wallpaper;
}

bool DesktopSettings::backgroundVisible() const
{
    if (m_interface.isValid()) {
        return m_interface.property("backgroundVisible").toBool();
    }
    // 默认显示背景
    return true;
}

bool DesktopSettings::dimsWallpaper() const
{
    if (m_interface.isValid()) {
        return m_interface.property("darkModeDimsWallpaper").toBool();
    }
    // 默认不调暗壁纸
    return false;
}

int DesktopSettings::backgroundType() const
{
    if (m_interface.isValid()) {
        return m_interface.property("backgroundType").toInt();
    }
    // 默认使用壁纸类型（0=壁纸，1=纯色）
    return 0;
}

QString DesktopSettings::backgroundColor() const
{
    if (m_interface.isValid()) {
        return m_interface.property("backgroundColor").toString();
    }
    // 默认背景颜色
    return "#2B8ADA";
}

void DesktopSettings::launch(const QString &command, const QStringList &args)
{
    QProcess process;
    process.setProgram(command);
    process.setArguments(args);
    process.startDetached();
}

void DesktopSettings::init()
{
    if (m_interface.isValid()) {
        connect(&m_interface, SIGNAL(wallpaperChanged(QString)), this, SLOT(onWallpaperChanged(QString)));
        connect(&m_interface, SIGNAL(darkModeDimsWallpaperChanged()), this, SIGNAL(dimsWallpaperChanged()));
        connect(&m_interface, SIGNAL(backgroundTypeChanged()), this, SIGNAL(backgroundTypeChanged()));
        connect(&m_interface, SIGNAL(backgroundColorChanged()), this, SIGNAL(backgroundColorChanged()));
        connect(&m_interface, SIGNAL(backgroundVisibleChanged()), this, SIGNAL(backgroundVisibleChanged()));
        m_wallpaper = m_interface.property("wallpaper").toString();
        emit wallpaperChanged();
    } else {
        // 如果DBus服务不可用，设置一个默认壁纸路径
        // wallpaper()方法会在被调用时返回实际的默认壁纸
        m_wallpaper = QString();
        emit wallpaperChanged();
    }
}

void DesktopSettings::onWallpaperChanged(QString path)
{
    if (path != m_wallpaper) {
        m_wallpaper = path;
        emit wallpaperChanged();
    }
}
