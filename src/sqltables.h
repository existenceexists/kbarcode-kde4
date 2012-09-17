/***************************************************************************
                          sqltables.h  -  description
                             -------------------
    begin                : Son Dez 29 2002
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

#ifndef SQLTABLES_H
#define SQLTABLES_H

#include <qobject.h>
#include <qwidget.h>
#include <qstring.h>
//Added by qt3to4:
#include <QSqlQuery>


/*  Change these values to match your environment: */
#define TABLE_CUSTOMER "customer"
#define TABLE_CUSTOMER_TEXT "customer_text"
#define TABLE_LABEL_DEF "label_def"
#define TABLE_BASIC "barcode_basic"
/* -------- */

// a macro to support QString::replace( QString, QString )
// also on QT 3.0.x
#if QT_VERSION <= 0x030100
    #include <qregexp.h>
    #define DSREPLACE( x ) QRegExp( x )
#else
    #define DSREPLACE( x ) x
#endif

// Holds all information
// necessary for connecting
// to a database
/*typedef struct mysqldata {*/
struct mysqldata {
    QString username;
    QString password;
    QString hostname;   // localhost
    QString database;   // kbarcode
    QString driver;     // QMYSQL3
    bool autoconnect;

    void operator=( const mysqldata & rhs ) {
        username = rhs.username;
        password = rhs.password;
        hostname = rhs.hostname;
        database = rhs.database;
        driver   = rhs.driver;

        autoconnect = rhs.autoconnect;        
    }
};


/**
  * This class describes a SQL database (e.g. MySQL or PostgreSQL)
  * and its features.
  * @author Dominik Seichter
  */
class SqlDescription {
    public:
        virtual ~SqlDescription() {}
        
        /** return the name of the database we can always
          * use to connect to this database.
          */
        virtual const QString initdb( QString ) const = 0;

        /** return the auto_increment or serial command used for this db.
          */
        virtual const QString autoIncrement() const = 0;

        /** return the command to show all fields of table for this db.
          */
        virtual const QString showColumns( const QString & table ) const  = 0;
};

class QSqlDatabase;
class QSqlQuery;
/**
  * This class is used to access a database, create it and save its settings to the
  * application configuration file. Also the labeldefinitions and exmplate data
  * are imported using this class.
  *
  * @short This singleton is used to access a database in KBarcode.
  * @author Dominik Seichter
  */
class SqlTables : public QObject {
    Q_OBJECT
    public:
        /** get a mysqldata object which contains the current database settings.
          * this object is for all sql databases, not only for mysql. The name is just a
          * relict from the past ;-)
          * @return mysqldata*
          */
        const mysqldata & getData() const {
            return sqldata;
        }

        /** Set the database settings
         */
        void setData( const mysqldata & data ) {
            sqldata = data;
        }
    
        /** get an instance of this singleton class. Always use this method as there is
          * no public constructor.
          * @return SqlTables*
          */
        static SqlTables* getInstance();

        /** returns true if KBarcode is connected to a database at the moment. Returns false if not.
          * @return bool
          */
        static bool isConnected() {
            return getInstance()->connected;
        }

        const SqlDescription* driver() const;

    public slots:
        const QString getBarcodeMaxLength( const QString & name );
        
        void loadConfig();
        void saveConfig();

        /** Connect to a SQL database using breviously configured
          * database settings.
          * @return bool returns true on success
          */
        bool connectMySQL();
        
        /**
         * Close an SQL database connection.
         */
        void closeDatabaseConnection();
        
        bool newTables();
        bool newTables( const QString & username, const QString & password, const QString & hostname, const QString & database, const QString & driver );
        void importLabelDef();
        void importExampleData();
        void importData( const QString & filename, QSqlDatabase dbase, const QString & progressDialogText = QString() );
        bool testSettings( const QString & username, const QString & password, const QString & hostname, const QString & database, const QString & driver );

	inline QSqlDatabase database() const;

    signals:
        void tablesChanged();
        void connectedSQL();

    private:
        SqlTables( QObject* parent = 0 );
        ~SqlTables();

        void exec( QSqlQuery* query, const QString & text );
        void updateTables();
        
        bool connected;

        //QSqlDatabase db;
        mysqldata sqldata;

        static SqlTables* instance;
};

inline QSqlDatabase SqlTables::database() const
{
    return QSqlDatabase::database();
}

class KComboBox;
class KLineEdit;
class KPushButton;
class QCheckBox;
/** A widget for configuring a SQL connection.
  * It has child widgets for entering data like the
  * user name, database name, host name, password
  * and the driver. Optionally the user can also
  * test if his settings will work.
  *
  * @author Dominik Seichter
  */
class SqlWidget : public QWidget {
    Q_OBJECT
    public:
        SqlWidget( bool showlabel, QWidget* parent = 0, const char* name = 0 );
        ~SqlWidget();

        const QString username() const;
        const QString driver() const;
        const QString database() const;
        const QString hostname() const;
        const QString password() const;
        bool autoconnect() const;

        int driverCount() const;

        void save( bool usedb = true );

    signals:
        void databaseWorking( bool b );
        
    private slots:
        void testSettings();
                
    private:
        KComboBox* m_driver;
    
        KLineEdit* m_username;
        KLineEdit* m_database;
        KLineEdit* m_hostname;
        KLineEdit* m_password;

        QCheckBox* m_autoconnect;

        KPushButton* buttonTest;        
};

#endif
