/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     Reion Wong <reionwong@gmail.com>
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

#include "desktop.h"
#include <QQmlContext>
#include <QQmlEngine>

#include <QGuiApplication>
#include <QDBusServiceWatcher>
#include <QTimer>
#include <QDebug>

Desktop::Desktop(QObject *parent)
    : QObject(parent)
{
    for (QScreen *screen : QGuiApplication::screens()) {
        screenAdded(screen);
    }

    connect(qApp, &QGuiApplication::screenAdded, this, &Desktop::screenAdded);
    connect(qApp, &QGuiApplication::screenRemoved, this, &Desktop::screenRemoved);
}

void Desktop::screenAdded(QScreen *screen)
{
    if (!m_list.contains(screen)) {
        DesktopView *view = new DesktopView(screen);
        // 先隐藏窗口，然后延迟显示，确保窗口管理器正确设置窗口属性
        view->hide();
        m_list.insert(screen, view);
        
        // 使用定时器延迟显示，避免窗口在初始化过程中显示
        QTimer::singleShot(1000, view, [view]() {
            view->show();
        });
    }
}

void Desktop::screenRemoved(QScreen *screen)
{
    if (m_list.contains(screen)) {
        DesktopView *view = m_list.find(screen).value();
        view->setVisible(false);
        view->deleteLater();
        m_list.remove(screen);
    }
}
