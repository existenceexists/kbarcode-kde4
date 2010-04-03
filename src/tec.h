//
// C++ Interface: tec
//
// Description: 
//
//
// Author: Dominik Seichter <domseichter@web.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef TEC_H
#define TEC_H
#include "mainwindow.h"
#include "labelutils.h"
#include "xmlutils.h"
#include "definition.h"

// Other includes
#include <stdlib.h>
#include <stdio.h>

// QT includes
#include <qclipboard.h>
#include <qcursor.h>
#include <q3groupbox.h>
#include <q3hbox.h>
#include <qlayout.h>
#include <qpainter.h>
#include <q3progressdialog.h>
#include <q3sqlcursor.h>
#include <qvalidator.h>

// KDE includes
#include <kapplication.h>
#include <kcombobox.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <k3listview.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <knuminput.h>
#include <qprinter.h>
#include <kurlrequester.h>

QString tec452(const QString &file, int count, const QString &art, const QString &group, const QString &id);
QString convZeros( int count, int zeros );
QString posConv( QString str, int zeros );
int rotation (int valor);
QString code39 (const QDomElement &e, int countBar);
QString code128 (const QDomElement &e, int countBar);
QString barcodeType(const QString &type);
QString withChecksum(const QDomElement &e);
QString sequence(const QDomElement &e);
QString serialWatch();
QString pdf417 (const QDomElement &e, int countBar);
QString getData(const QString &etext, const QString &art, const QString &group, const QString &id);
QString lineWidth(const QDomElement &e);
QString image (const QString &file);
QString getImage(const QString &id);


#endif
