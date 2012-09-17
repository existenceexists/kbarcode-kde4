/***************************************************************************
                          tokenprovider.h  -  description
                             -------------------
    begin                : Fre Sep 19 2003
    copyright            : (C) 2003 by Dominik Seichter
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

#ifndef TOKENPROVIDER_H
#define TOKENPROVIDER_H

// enable JAVASCRIPT interpreter
#define USE_JAVASCRIPT
// use KJS instead of 
#define NO_KJS_EMBED

#include <qstring.h>
#include <qregexp.h>
//Added by qt3to4:
#include <QList>
#include <time.h>

#include "documentitem.h"

namespace KABC {
    class Addressee;
};

class tToken {
    public:
        tToken() {};
        
        tToken( const QString & t, const QString & d, bool a = false )
        {
            token = t;
            description = d;
	    appendix = a;
        }
    
        void operator=( const tToken & rhs )
        {
            token = rhs.token;
            description = rhs.description;
	    appendix = rhs.appendix;
        }
        
        QString token;
        QString description;
	bool appendix;
};

struct tCategories {
    int category;
    QList<tToken> tokens;
};


#ifndef NO_KJS_EMBED
namespace KJSEmbed {
    class KJSEmbedPart;
}
#else
/*namespace KJS {
    class Interpreter;
}*/
class KJSInterpreter;

#endif // NO_KJS_EMBED

class QPaintDevice;

/** This class handles the replacement of tokens like [date] or [article_no].
  *
  *@author Dominik Seichter
  */
class TokenProvider {
    public: 
        enum ECategories { CAT_DATABASE, CAT_LABEL, CAT_CUSTOM, CAT_DATE, CAT_ADDRESS };

        TokenProvider( QPaintDevice* paintdevice );
        virtual ~TokenProvider();

	static bool hasJavaScript();

        static QList<tCategories>* getTokens();
        
        /**
         * Get a caption which can be displayed to the user from 
         * a ECategory enum.
         *
         * @param e the category which should be used
         *
         * @returns a translated caption which can be displayed to the user
         */
        static const QString captionForCategory( ECategories e ) {
            return s_captions[e];
        }

        inline void updateDone() { m_update = false; }
        inline virtual bool update() {
            return m_update && m_contains_update;
        }

        inline void setIndex( unsigned int index ) { m_index = index; m_update = true; }
        inline void setPage( unsigned int page ) { m_page = page; m_update = true; }
        inline void setArticleNo( const QString & t ) { article_no = t; m_update = true; }
        inline void setBarcodeNo( const QString & t ) { barcode_no = t; m_update = true; }
        inline void setCustomerNo( const QString & t ) { customer_no = t; m_update = true; }
        inline void setEncodingTypeName( const QString & t ) { encoding_type_name = t; m_update = true; }
        inline void setGroup( const QString & t ) { group = t; m_update = true; }
        inline void setLabelName( const QString & t ) { label_name = t; m_update = true; }
        inline void setCol( unsigned int c ) { col = c; m_update = true; }
        inline void setRow( unsigned int r ) { row = r; m_update = true; }
        inline void setSerial( const QString &t, unsigned int inc ) { m_serial = t; m_increment = inc; m_update = true; m_contains_update = true;}
        inline void setAddressee( KABC::Addressee* pAddressee ) { m_address = pAddressee; }

        inline int index() const { return m_index; }
        inline unsigned int page() const { return m_page; }
        inline const QString & articleNo() const { return article_no; }        
        inline const QString & barcodeNo() const { return barcode_no; }
        inline const QString & serial() const { return m_serial; }

        inline void setPaintDevice( QPaintDevice* paint ) { m_printer = paint; }
        inline QPaintDevice* paintDevice() const { return m_printer; }

        /**
	 * parse the given java script code and return its result
	 * @returns    either the result of the javascript code
	 *             or an error message
	 * @p script   javascript code to execute
	 */
	QString jsParse( const QString & script );

        /**
	 * parse the given java script code and return its result
	 * @returns    true or false
	 * @p script   javascript code to execute
	 */
	bool jsParseToBool( const QString & script );

        /**
          * parse the given text for tokens and return a
          * string with all tokens replaced correctly.
          * @param text QString look in this text for tokens to replace
          */
        QString parse( const QString & text );


        /**
         * set @p list as DocumentItemList which is used for @see listUserVars
         */
        inline void setCurrentDocumentItems( const DocumentItemList & list );

	/**
	  * parses all DocumentItems in @p list and returns a stringlist
	  * containing all user defined variables in these DocumentItems.
	  * A user defined variable is something like [$MyVar1]. Variable
	  * names are case insensitive.
	  */
	QStringList listUserVars();
	  

        inline void setUserVars( const QMap<QString,QString> & data );

    private:
        static void init();
        
        const QString createSerial();
	
	QString escapeText( const QString & t );
	QString unescapeText( const QString & t );

	/** A helper function called from parse
	  * which returns the value for every known 
	  * token @p text.
          */
	QString process( const QString & text );

	/** A helper function called from parse
	  * which returns the value for every known 
	  * token @p t.
          */
	QString processAddresses( const QString & t );

        /** a helper function needed by listUserVars.
	  * which adds all user defined variables to 
	  * m_findUserVars.
	  */
	QString processUserVars( const QString & t );

        void findBrackets( QString & text, QString (TokenProvider::*parserfunction)( const QString & ) );

        /**
          * run a SQL Query and return it result
          * @param query QString the SQL command to execute
          */
        QString query( const QString & query );

        unsigned int m_index;
        unsigned int m_page;
        unsigned int m_increment;

        DocumentItemList m_document_items;

	QMap<QString,QString> m_uservardata;

        QString article_no;
        QString barcode_no;
        QString customer_no;
        QString encoding_type_name;
        QString group;
        QString label_name;
        QString m_serial;
        unsigned int row;
        unsigned int col;

        bool m_update;
        bool m_contains_update;
        QPaintDevice* m_printer;
	QRegExp date_reg_exp ;

        KABC::Addressee* m_address;

	QStringList* m_findUserVarsList;
        static QList<tCategories> s_categories;
        static QMap<ECategories, QString> s_captions;

#ifdef NO_KJS_EMBED
	static KJSInterpreter* s_interpreter;
#else	
	static KJSEmbed::KJSEmbedPart* s_interpreter;
#endif // NO_KJS_EMBED
};

inline void TokenProvider::setCurrentDocumentItems( const DocumentItemList & list )
{
    m_document_items = list;
}

inline void TokenProvider::setUserVars( const QMap<QString,QString> & data )
{
    m_uservardata = data;
}

#endif
