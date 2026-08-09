/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 */

#include "application.h"
#include "registertypes.h"
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    // 禁用 QEventLoopLocker 自动退出锁：
    // 迁移 Qt6/KF6 后，KIO/KJob 结束会向主事件循环投递 Quit 导致应用提前退出
    QCoreApplication::setQuitLockEnabled(false);

    // 注册共享文件模型库的 QML 类型（Cutefish.FileManager / Cutefish.DragDrop）
    registerCutefishFileModelTypes();

    Application app(argc, argv);
    return app.run();
}
