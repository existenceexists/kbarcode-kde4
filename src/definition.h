/***************************************************************************
                          definition.h  -  description
                             -------------------
    begin                : Mit Nov 20 2002
    copyright            : (C) 2002 by Dominik Seichter
    email                : domseichter@web.de
 ***************************************************************************/

/***************************************************************************
                                                                          
    This program is free software; you can redistribute it and/or modify  
    it under the terms of the GNU General Public License as published by  
    the Free Software Foundation; either version 2 of the License, or     
    (at your option) any later version.                                   
                                                                          
 ***************************************************************************/

#ifndef DEFINITION_H
#define DEFINITION_H

#include "measurements.h"
#include "labelutils.h"
#include <qmap.h>
#include <qstring.h>
//Added by qt3to4:
#include <QSqlQuery>

#ifndef I2S
    #define I2S(x) QString::number(x)
#endif // I2S

class QWidget;
class QFile;
class QPaintDevice;
class QString;
class QStringList;
class QSqlQuery;
class Measurements;
class QProgressDialog;
/** @short A label definition;
  *
  * Represents a label definition. The definition is either read from
  * a SQL database or from a text file on the harddisk.
  * This class supports also writing new label definitions.
  * After you selected a definition, you can retrieve its measurements.
  *
  * @author Dominik Seichter
  */
class Definition : private LabelUtils {
    public:
        Definition( QWidget* parent = 0 );
        Definition( int label_def_id, QWidget* parent = 0 );
        Definition( const QString & label_def_id, QWidget* parent = 0 );
        Definition( const QString & producer, const QString & type, QWidget* parent = 0 );
        ~Definition();

        void setId( const QString & label_def_id );
        void setId( int label_def_id );

        const Measurements & getMeasurements() const;

        static int write( const Measurements & c, QString type, QString producer );
        static int getClosest( const QString & producer, const QString & type );
        
        static const QStringList getProducers();
        static const QStringList getTypes( QString producer );        
        /** reread the list of producers, because it has changed
          * mapType is also cleared and reread when necessary.
          */
        static void updateProducer();

        int getId() const { return id; }
        const QString getProducer() const { return producer; }
        const QString getType() const { return type; }

    private:
        void init( const QString & label_def_id );
        void getFileMeasurements( const QString & label_def_id );
	static void initProgress();
	static void increaseProgress();
	static void destroyProgress();

        static int writeSQL( const Measurements & c, QString type, QString producer );
        static int writeFile( const Measurements & c, QString type, QString producer );
        
        static bool showFileError();
        static bool openFile();
        static QStringList* listProducers;
        static QMap<QString,QStringList> mapTypes;
        static QFile* file;
        static QByteArray* array;
	static QProgressDialog* m_progress;

	QWidget* m_parent;
        int id;
        QString producer;
        QString type;
        Measurements m_measure;
        bool locked;
        static bool nodefmsg;
};

#endif
