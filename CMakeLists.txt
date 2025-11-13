cmake_minimum_required(VERSION 3.21)  # Qt6需要CMake 3.16以上，推荐3.21[citation:6]
project(cutefish-filemanager LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)  # Qt6推荐使用C++17[citation:6]
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt6包，使用无版本命令[citation:3]
find_package(Qt6 REQUIRED COMPONENTS Widgets Core DBus Quick LinguistTools)

# 查找KF6包，注意模块名称可能有所变化
find_package(KF6KIO REQUIRED)
find_package(KF6Solid REQUIRED)
find_package(KF6WindowSystem REQUIRED)
find_package(KF6Config REQUIRED)
find_package(KF6XmlGui REQUIRED)

# 使用Qt6的DBus适配器命令
qt6_add_dbus_adaptor(DBUS_SOURCES
                     com.cutefish.FileManager.xml
                     application.h Application)

add_executable(cutefish-filemanager
  main.cpp
  application.cpp
  window.cpp
  dbusinterface.cpp

  draganddrop/declarativedroparea.cpp
  draganddrop/declarativedragdropevent.cpp
  draganddrop/declarativemimedata.cpp

  model/foldermodel.cpp
  model/placesmodel.cpp
  model/placesitem.cpp
  model/pathbarmodel.cpp
  model/dirlister.cpp
  model/positioner.cpp

  cio/cfilejob.cpp
  cio/cfilesizejob.cpp

  dialogs/createfolderdialog.cpp
  dialogs/filepropertiesdialog.cpp
  dialogs/openwithdialog.cpp
  widgets/rubberband.cpp
  widgets/itemviewadapter.cpp

  desktop/desktop.cpp
  desktop/desktopview.cpp
  desktop/desktopsettings.cpp
  desktop/dockdbusinterface.cpp

  helper/datehelper.cpp
  helper/pathhistory.cpp
  helper/fm.cpp
  helper/shortcut.cpp
  helper/filelauncher.cpp
  helper/keyboardsearchmanager.cpp

  mimetype/mimeappmanager.cpp
  mimetype/xdgdesktopfile.cpp

  thumbnailer/thumbnailprovider.cpp
  thumbnailer/thumbnailcache.cpp

  desktopiconprovider.cpp

  qml.qrc

  ${DBUS_SOURCES}
)

target_link_libraries(cutefish-filemanager
  PRIVATE
  Qt6::Core
  Qt6::DBus
  Qt6::Quick
  Qt6::Widgets

  KF6::KIOWidgets
  KF6::KIOCore
  KF6::KIOGui
  KF6::XmlGui
  KF6::Solid
  KF6::WindowSystem
  KF6::ConfigCore
  KF6::ConfigGui

  # FishUI (如果可用，也需要更新到Qt6版本)
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