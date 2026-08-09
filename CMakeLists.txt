cmake_minimum_required(VERSION 3.21)
project(cutefish-filemanager LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt6包
find_package(Qt6 REQUIRED COMPONENTS Widgets Core DBus Quick QuickControls2 LinguistTools)

# 查找KF6包
find_package(KF6KIO REQUIRED)
find_package(KF6Solid REQUIRED)
find_package(KF6WindowSystem REQUIRED)
find_package(KF6Config REQUIRED)
find_package(KF6XmlGui REQUIRED)

# 共享文件模型库（模型/视图/拖拽/缩略图/对话框等）
# 桌面进程(cutefish-desktop)与本文件管理器共用
add_subdirectory(../filemodel filemodel)

# 使用Qt6的DBus适配器命令
qt6_add_dbus_adaptor(DBUS_SOURCES
                     com.cutefish.FileManager.xml
                     application.h Application)

add_executable(cutefish-filemanager
  main.cpp
  application.cpp
  dbusinterface.cpp

  qml.qrc

  ${DBUS_SOURCES}
)

target_link_libraries(cutefish-filemanager
  PRIVATE
  cutefish-filemodel

  Qt6::Core
  Qt6::DBus
  Qt6::Quick
  Qt6::QuickControls2
  Qt6::Widgets

  KF6::KIOWidgets
  KF6::KIOCore
  KF6::KIOGui
  KF6::XmlGui
  KF6::Solid
  KF6::WindowSystem
  KF6::ConfigCore
  KF6::ConfigGui
)

# 使用Qt6的翻译命令
file(GLOB TS_FILES translations/*.ts)

set(QM_FILES "")
foreach(_ts ${TS_FILES})
    get_filename_component(_name ${_ts} NAME_WE)
    set(_qm ${CMAKE_CURRENT_BINARY_DIR}/${_name}.qm)

    add_custom_command(
        OUTPUT ${_qm}
        COMMAND Qt6::lrelease ${_ts} -qm ${_qm}
        DEPENDS ${_ts}
        COMMENT "Generating ${_qm}"
    )

    list(APPEND QM_FILES ${_qm})
endforeach()

add_custom_target(translations ALL DEPENDS ${QM_FILES})
add_dependencies(${PROJECT_NAME} translations)


install(TARGETS cutefish-filemanager RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES cutefish-filemanager.desktop DESTINATION "/usr/share/applications")
install(FILES ${QM_FILES} DESTINATION /usr/share/cutefish-filemanager/translations)

