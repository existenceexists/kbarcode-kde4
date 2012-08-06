/***************************************************************************
                          label.h  -  description
                             -------------------
    begin                : Mon Apr 29 2002
    copyright            : (C) 2002 by Dominik Seichter
    email                : domseichter@web.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LABEL_H
#define LABEL_H

#include <qfont.h>
#include <QTextStream>

#include "labelutils.h"
#include "xmlutils.h"
#include "definition.h"
#include "tokenprovider.h"

class BarCode;
class Barkode;
class QDomDocument;
class QIODevice;
class QString;
class QPainter;
class QPaintDevice;
class QPrinter;
/** Creates a QPicture from the XML KBarcode file. If a SQL connections is available
  * the data from the SQL tables will be used, too.
  */
class Label : private LabelUtils, private XMLUtils, public TokenProvider {
    public:
        Label( Definition* _def, QIODevice* device, QString labelname, QPaintDevice* _printer, QString customer_id, QString _article_no, QString _group = "" );
        Label( Definition* _def, QIODevice* device, QString labelname, QPaintDevice* _printer );
        ~Label();

        void epcl( QTextStream* stream );
        void ipl( QTextStream* stream );
        void zpl( QTextStream* stream );

        void draw( QPainter* painter,int x, int y );
        
        int getId() const;  
        static void getXLabel( double x, double y, double width, double height, QPainter* painter, int mode, QString value = "" );

        //void setPrinter( QPrinter* p ) { m_printer = p; }

	/** reimplemented from TokenProvider
	 *  returns wether this label has to be regenerated everytime
	 */
	bool update();

    private:
        void load( QIODevice* device );
        void setBarcodeValue( Barkode* barcode );
        void InitBarcodes();
        bool drawThisItem( const DocumentItem* item );

    protected:
        DocumentItemList m_list;
        
        Definition* d;
        QPaintDevice* m_printer;

        bool m_sequence;
};

#endif
