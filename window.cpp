/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     Reion Wong <reion@cutefishos.com>
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

#include "window.h"
#include <QEvent>
#include <QDebug>
#include <QQuickWindow>
#include <QPixmapCache>
#include <QQuickImageProvider>
#include <QIcon>
#include <QPixmap>

// Simple IconThemeProvider for Qt6 compatibility
class IconThemeProvider : public QQuickImageProvider
{
public:
    IconThemeProvider() : QQuickImageProvider(QQuickImageProvider::Pixmap) {}

    QPixmap requestPixmap(const QString &id, QSize *realSize, const QSize &requestedSize) override
    {
        QSize size(requestedSize);
        if (size.width() <= 0)
            size.setWidth(32);
        if (size.height() <= 0)
            size.setHeight(32);
        
        if (realSize)
            *realSize = size;
        
        QIcon icon = QIcon::fromTheme(id);
        if (icon.isNull()) {
            // Fallback to generic icon
            icon = QIcon::fromTheme("application-x-executable");
        }
        
        return icon.pixmap(size);
    }
};

Window::Window(QObject *parent)
    : QQmlApplicationEngine(parent)
{
    // Register icon theme provider for Qt6 compatibility
    addImageProvider(QStringLiteral("icontheme"), new IconThemeProvider());
    
    connect(this, &QQmlApplicationEngine::objectCreated,
            this, &Window::onObjectCreated);
}

void Window::load(const QUrl &url)
{
    QQmlApplicationEngine::load(url);
}

void Window::onObjectCreated(QObject *object, const QUrl &url)
{
    Q_UNUSED(url);
    QQuickWindow *w = qobject_cast<QQuickWindow *>(object);
    if (w) {
        w->installEventFilter(this);
    }
}

bool Window::eventFilter(QObject *obj, QEvent *e)
{
    if (e->type() == QEvent::Close) {
        QPixmapCache::clear();
        clearComponentCache();
        deleteLater();
        e->accept();
    }

    return QObject::eventFilter(obj, e);
}
