/***************************************************************************
                          batchprinter.h  -  description
                             -------------------
    begin                : Sat Jan 10 2004
    copyright            : (C) 2004 by Dominik Seichter
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

#ifndef BATCHPRINTER_H
#define BATCHPRINTER_H

#include "measurements.h"

#include <qmap.h>
#include <QList>

namespace KABC {
    class AddresseeList;
}

class Definition;
class Label;
class QPrinter;
class Measurements;
class QBuffer;
class QPainter;
class QProgressDialog;
class QWidget;
class QPaintDevice;

typedef QList< QMap<QString,QString> > TVariableList;
/**
    This class is responsible for batch printing of articles.
    It is also responsible for creating images of all articles.
    @author Dominik Seichter
*/
class BatchPrinter
{
    public:
        enum EOutputFormat { POSTSCRIPT = 0, IMAGE, BCP };
        enum EImageFileName { E_ARTICLE, E_BARCODE, E_CUSTOM };

        struct data {
            int number;
            QString article_no;
            QString group;
        };

        /** Use this constructor if you want to print to
          * a printer.
          */
        BatchPrinter( QPrinter* p, QWidget* _parent );
        /** This constructor is used in image gernaration mode.
          * @p path is the path to an existing directory where
          * the images are saved.
          */
        BatchPrinter( const QString & path, QWidget* p );
        
        /** Use this constructor to print to a barcode printer @p path
          * with format @p format
          */
        BatchPrinter( const QString & path, int format, QWidget* _parent );
        
        ~BatchPrinter();

        inline void setBuffer( QBuffer* b ) { buffer = b; }
        void setCustomer( const QString & s ) { m_customer = s; }

        /** Specify the label on which is printed first on the page.
         *  The first label on the page (top/left) has the index 1, 
         *  the second label is index 2 ...
         */
        inline void setMove( int m ) { 
            m_move = m > 0 ? --m : 0;
        }

        /** set the sql data for the batchprinter
         *  @p list will be deleted by the batchprinter
         */
        void setData( QList<data>* list );

        /** set the variable data for the batchprinter
         *  @p list will be deleted by the batchprinter
         */
        void setData( TVariableList* list );

        /** set the address data for the batchprinter
         *  @p list will be deleted by the batchprinter
         */
        void setData( KABC::AddresseeList* list );

	inline void setImageFilename( const EImageFileName e ) { m_image_filename = e; }
	inline void setImageCustomFilename( const QString & name ) { m_image_custom_filename = name; };
	inline void setImageFormat( const QString & f ) { m_image_format = f; }
        inline void setSerial( const QString & s, unsigned int inc ) { m_serial = s; m_increment = inc; }
        inline void setName( const QString & s ) { m_name = s; }
        inline void setDefinition( Definition* d ) { def = d; }
        inline void setLabels( int l ) { m_labels = l; }
        inline void setEvents( bool b ) { m_events = b; }

        /** start the batch printing on a printer
          */
        void start();
        /** start image generation
          */
        void startImages();
        /** start printing to a barcode printer
          */
        void startBCP();
    private:
	/** create a label which gets intialized either with m_vardata or m_data
	 *  @p number is set to the number of times this label should be printed.
	 */
	Label* initLabel( int* number = NULL );

        /** starts a new page on the printer, if the
          * current label does not fit anymore on the current pageh
          */
        void checkForNewPage( Label* label );
        /** draw a border around the current label using QPainter @p painter
          */
        void drawBorders();
        /** start a "new line" on the printer,
          * i.e. beginn the second row of labels
          * on the page.
          */
        void changeLine();
        void proccessEvents( int lb, QString value = QString::null, Label* label = 0 );
        void printXLabel( int lb, Label* label, const QString & value );
        void moveLabels();

	/** print the labels using data from the m_data structure
	  */
	void startPrintData( QProgressDialog* progress );
	/** print the labels using data from the m_vardata or m_addrdata structure
	  */
	void startPrintVarData( QProgressDialog* progress );

        /** Create and return a progressdialog
          */
        QProgressDialog* createProgressDialog( const QString & caption );
        bool checkProgressDialog( QProgressDialog* progress );

        Definition* def;
        Measurements m_measure;
        QList<data>* m_data;
	TVariableList* m_vardata;
        KABC::AddresseeList* m_addrdata;

        QString m_serial;
        QString m_name;
        QString m_customer;
        /** Path for image output
          * or device for bcp output
          */
        QString m_path;
        /** defines the image format to be used
          */
	QString m_image_format;

        int m_cur_data_count;

        /** format for bcp output
          */
        int m_bcp_format;

        /** defines the format of the filename for images
         */
	EImageFileName m_image_filename;

	/** prefix for custom filenames if m_image_filename = E_CUSTOM
	 */
	QString m_image_custom_filename;

        /** enable/disable events
          */
        bool m_events;

        unsigned int m_increment;
        unsigned int m_move;
        /** number of labels to print for the progress dialog
          */
        unsigned int m_labels;

        /** height of the current page in pixel.
          */
        double pageh;
        double curw;
        double curh;
        int c_h;
        int c_w;

	QPaintDevice* m_paintDevice;

        QBuffer* buffer;
        QPrinter* printer;
        QPainter* painter;
        /** parent for dialogs
          */
        QWidget* parent;
};

#endif
