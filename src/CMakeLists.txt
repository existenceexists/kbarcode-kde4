
add_subdirectory( pics )
add_subdirectory( xml )

include_directories( ${CMAKE_SOURCE_DIR} ${KDE4_INCLUDE_DIR} ${KDE4_INCLUDES}
  ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} dialogs )
add_definitions( ${KDE4_DEFINITIONS} )

########### next target ###############

set(kbarcode_SRCS
 # barcodecombo.cpp
 # barcodegenerator.cpp
 # barcodeitem.cpp
 # barkode.cpp
 # barkodeengine.cpp
 # batchprinter.cpp
 # batchassistant.cpp
 # commands.cpp
 # confassistant.cpp
 # csvfile.cpp
 # databasebrowser.cpp
 # definition.cpp
 # documentitem.cpp
 # dsrichtext.cpp
 # dstextedit.cpp
 # encodingcombo.cpp
 # gnubarcode.cpp
 # gnubarkodeengine.cpp
 # imageitem.cpp
  kbarcode.cpp
 # kbarcodesettings.cpp
 # label.cpp
 # labeleditor.cpp
 # labelutils.cpp
 # lineitem.cpp
  main.cpp
  mainwindow.cpp
 # measurements.cpp
 # mimesources.cpp
 # mycanvasitem.cpp
 # mycanvasview.cpp
 # mydatatable.cpp
 # newlabel.cpp
 # pixmapbarcode.cpp
 # printersettings.cpp
 # propertywidget.cpp
 # purepostscript.cpp
 # rectitem.cpp
 # sqltables.cpp
 # tbarcode2.cpp
 # tcanvasitem.cpp
 # tec.cpp
 # textitem.cpp
 # textlineedit.cpp
 # textlineitem.cpp
 # tokenprovider.cpp
 # xmlutils.cpp
 # zplutils.cpp
 # dialogs/barcodedialogs.cpp
 # dialogs/barcodeprinterdlg.cpp
 # dialogs/configdialog.cpp
 # dialogs/csvimportdlg.cpp
 # dialogs/definitiondialog.cpp
 # dialogs/documentitemdlg.cpp
 # dialogs/multilineeditdlg.cpp
 # dialogs/previewdialog.cpp
 # dialogs/printlabeldlg.cpp
 # dialogs/rectsettingsdlg.cpp
 # dialogs/smalldialogs.cpp
 # dialogs/tokendialog.cpp  
  )

kde4_add_ui_files( kbarcode_SRCS
  )

message("\${ProjectName} is: ${ProjectName}")

if(ProjectName)
  
  kde4_add_executable(${ProjectName} ${kbarcode_SRCS})
  
  target_link_libraries(${ProjectName} ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS}
  			       ${KDE4_KIO_LIBS} ${KDE4_KMETADATA}
  			       ${KDE4_KJS_LIBS} ${QT_QT3SUPPORT_LIBRARY})
  
  install(TARGETS ${ProjectName} DESTINATION ${BIN_INSTALL_DIR})
  #install(FILES xml/mainwindowui.rc DESTINATION  ${DATA_INSTALL_DIR}/kbarcode002)

endif(ProjectName)

########### install files ###############

#install( FILES  kbarcode.desktop DESTINATION ${XDG_APPS_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR} )
