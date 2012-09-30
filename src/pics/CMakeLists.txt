# Created by František Brožka 7.9.2011 .

#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/logo.png ${CMAKE_CURRENT_BINARY_DIR}/logo.png COPYONLY)
#install(FILES 
#    logo.png 
#    hi16-app-kbarcode.png 
#    hi32-app-kbarcode.png 
#    hi64-app-kbarcode.png 
#    hi16-action-kbarcodelinetool.png 
#    hi32-action-kbarcoderect.png 
#    hi16-action-kbarcodeellipse.png 
#    hi16-action-kbarcodegrid.png 
#    hi16-action-barcode.png 
#    hi22-action-barcode.png 
#    hi32-action-barcode.png 
#    DESTINATION ${DATA_INSTALL_DIR}/${ProjectName})

#message(STATUS "\${CMAKE_CURRENT_SOURCE_DIR} is: ${CMAKE_CURRENT_SOURCE_DIR}")
#message(STATUS "\${CMAKE_CURRENT_BINARY_DIR} is: ${CMAKE_CURRENT_BINARY_DIR}")
kde4_install_icons( ${ICON_INSTALL_DIR} )

kde4_update_iconcache()
