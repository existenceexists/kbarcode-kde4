/***************************************************************************
                          tokenprovider.cpp  -  description
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

#include "tokenprovider.h"
#include "sqltables.h"
#include "kbarcodesettings.h"

#include "barcodeitem.h"
#include "textitem.h"
#include "textlineitem.h"

// Qt includes
#include <qdatetime.h>
#include <qsqlquery.h>
#include <qregexp.h>
#include <QList>
#include <time.h>

#include <klocale.h>
#include <kabc/addressee.h>

#ifdef USE_JAVASCRIPT
#ifdef NO_KJS_EMBED
#include <kjs/interpreter.h>
#else
#include <kjsembed/kjsembedpart.h>
#endif // NO_KJS_EMBED
#endif // USE_JAVASCRIPT

#define NUM_FIELDS 10

const char* TOK_ARTICLE_NO_CUSTOMER = "article_no_customer";
const char* TOK_BARCODE_NO_CUSTOMER = "barcode_no_customer";
const char* TOK_ARTICLE_DESC = "article_desc";
const char* TOK_ARTICLE_NO = "article_no";
const char* TOK_CUSTOMER_NO = "customer_no";
const char* TOK_CUSTOEMR_NAME = "customer_name";
const char* TOK_BARCODE_NO = "barcode_no";
const char* TOK_BARCODE_NO_BASIC = "barcode_no_basic";
const char* TOK_GROUP = "group";
const char* TOK_ENCODING_TYPE = "encoding_type";
const char* TOK_ENCODING_TYPE_NAME = "encoding_type_name";
const char* TOK_LINE = "line%1";
const char* TOK_FIELD = "field%1";

const char* TOK_FILENAME = "filename";
const char* TOK_INDEX = "index";
const char* TOK_COL = "col";
const char* TOK_ROW = "row";
const char* TOK_PAGE = "page";
const char* TOK_RESOLUTION = "resolution";
const char* TOK_SERIAL = "serial";

const char* TOK_JS = "js:";
const char* TOK_SQL = "sqlquery:";
const char* TOK_VAR = "$";

const char* TOK_DATE = "date";
const char* TOK_DATE_a = "date:%a";
const char* TOK_DATE_A = "date:%A";
const char* TOK_DATE_b = "date:%b";
const char* TOK_DATE_B = "date:%B";
const char* TOK_DATE_c = "date:%c";
const char* TOK_DATE_C = "date:%C";
const char* TOK_DATE_d = "date:%d";
const char* TOK_DATE_D = "date:%D";
const char* TOK_DATE_e = "date:%e";
const char* TOK_DATE_F = "date:%F";
const char* TOK_DATE_g = "date:%g";
const char* TOK_DATE_G = "date:%G";
const char* TOK_DATE_H = "date:%H";
const char* TOK_DATE_I = "date:%I";
const char* TOK_DATE_J = "date:%J";
const char* TOK_DATE_m = "date:%m";
const char* TOK_DATE_M = "date:%M";
const char* TOK_DATE_p = "date:%p";
const char* TOK_DATE_r = "date:%r";
const char* TOK_DATE_R = "date:%R";
const char* TOK_DATE_S = "date:%S";
const char* TOK_DATE_T = "date:%T";
const char* TOK_DATE_u = "date:%u";
const char* TOK_DATE_U = "date:%U";
const char* TOK_DATE_V = "date:%V";
const char* TOK_DATE_w = "date:%w";
const char* TOK_DATE_W = "date:%W";
const char* TOK_DATE_x = "date:%x";
const char* TOK_DATE_X = "date:%X";
const char* TOK_DATE_y = "date:%y";
const char* TOK_DATE_Y = "date:%Y";
const char* TOK_DATE_z = "date:%z";
const char* TOK_DATE_Z = "date:%Z";

const char* TOK_ADDRESS_NAME = "address_name";
const char* TOK_ADDRESS_GIVEN_NAME = "address_givenname";
const char* TOK_ADDRESS_FAMILY_NAME = "address_familyname";
const char* TOK_ADDRESS_EMAIL = "address_email";
const char* TOK_ADDRESS_FULL_EMAIL = "address_fullemail";
const char* TOK_ADDRESS_PHONE_PREF = "address_phone";
const char* TOK_ADDRESS_PHONE_HOME = "address_phonehome";
const char* TOK_ADDRESS_PHONE_WORK = "address_phonework";
const char* TOK_ADDRESS_PHONE_MSG = "address_phonemsg";
const char* TOK_ADDRESS_PHONE_VOICE = "address_phonevoice";
const char* TOK_ADDRESS_PHONE_FAX = "address_phonefax";
const char* TOK_ADDRESS_PHONE_CELL = "address_phonemobile";
const char* TOK_ADDRESS_PHONE_VIDEO = "address_phonevideo";
const char* TOK_ADDRESS_PHONE_BBS = "address_phonebbs";
const char* TOK_ADDRESS_PHONE_MODEM = "address_phonemodem";
const char* TOK_ADDRESS_PHONE_CAR = "address_phonecar";
const char* TOK_ADDRESS_PHONE_ISDN = "address_phoneisdn";
const char* TOK_ADDRESS_PHONE_PCS = "address_phonepcs";
const char* TOK_ADDRESS_PHONE_PAGER = "address_phonepager";
const char* TOK_ADDRESS_ADDRESS_POB = "address_postofficebox";
const char* TOK_ADDRESS_ADDRESS_EXTENDED = "address_extended";
const char* TOK_ADDRESS_ADDRESS_STREET = "address_street";
const char* TOK_ADDRESS_ADDRESS_LOCALITY = "address_locality";
const char* TOK_ADDRESS_ADDRESS_REGION = "address_region";
const char* TOK_ADDRESS_ADDRESS_POSTALCODE = "address_postalcode";
const char* TOK_ADDRESS_ADDRESS_COUNTRY = "address_country";
const char* TOK_ADDRESS_ADDRESS_LABEL = "address_label";
const char* TOK_ADDRESS = "address";
// ---
const char* TOK_ADDRESS_URL = "address_url";
const char* TOK_ADDRESS_NICK = "address_nick";
const char* TOK_ADDRESS_ADDITIONAL_NAME = "address_addtionalname";
const char* TOK_ADDRESS_PREFIX = "address_prefix";
const char* TOK_ADDRESS_SUFFIX = "address_suffix";
const char* TOK_ADDRESS_BIRTHDAY = "address_birthday";
const char* TOK_ADDRESS_TITLE = "address_title";
const char* TOK_ADDRESS_ROLE = "address_role";
const char* TOK_ADDRESS_NOTE = "address_note";
const char* TOK_ADDRESS_ORGANIZATION = "address_organization";

TokenProvider::TokenProvider( QPaintDevice* paintdevice )
    : m_printer( paintdevice )
{
    m_index = 0;
    m_page = 0;
    m_update = false;
    m_contains_update = false;
    m_findUserVarsList = NULL;
    m_address = NULL;

    // default value for serial that should work with all barcodes
    m_serial = "0000000";

    row = col = 0;

    article_no =  barcode_no =  customer_no =  encoding_type_name = group = label_name = QString::null;
    
    date_reg_exp.setPattern("date:(%[a-zA-Z])");

#ifdef USE_JAVASCRIPT
#ifdef NO_KJS_EMBED
    s_interpreter = new KJS::Interpreter();
#else
    s_interpreter = new KJSEmbed::KJSEmbedPart();
#endif // NO_KJS_EMBED
#endif // USE_JAVASCRIPT    
}

TokenProvider::~TokenProvider()
{
}

QList<tCategories> TokenProvider::s_categories;
QMap<TokenProvider::ECategories,QString> TokenProvider::s_captions;
#ifdef NO_KJS_EMBED
KJS::Interpreter* TokenProvider::s_interpreter = NULL;
#else
KJSEmbed::KJSEmbedPart* TokenProvider::s_interpreter = NULL;
#endif // NO_KJS_EMBED

bool TokenProvider::hasJavaScript()
{
#ifdef USE_JAVASCRIPT
    return true;
#else
    return false;
#endif // USE_JAVASCRIPT
}

QList<tCategories>* TokenProvider::getTokens()
{
    TokenProvider::init(); 
    return &s_categories;
}

void TokenProvider::init()
{
    if( s_categories.isEmpty() )
    {
        s_captions[CAT_DATABASE] = i18n("Database");
        s_captions[CAT_LABEL] = i18n("Label");
        s_captions[CAT_CUSTOM] = i18n("Custom Values");
        s_captions[CAT_DATE] = i18n("Date & Time");
        s_captions[CAT_ADDRESS] = i18n("Addressbook");
        tCategories category;
        
        category.tokens.append( tToken( TOK_ARTICLE_NO_CUSTOMER, QString::null ) );
        category.tokens.append( tToken( TOK_BARCODE_NO_CUSTOMER, QString::null ) );
        category.tokens.append( tToken( TOK_ARTICLE_DESC, i18n("Article description from barcode_basic") ) );
        category.tokens.append( tToken( TOK_ARTICLE_NO, i18n("Article number from barcode_basic") ) );
        for( int i = 0; i < NUM_FIELDS; i++ )
            category.tokens.append( tToken( QString( TOK_LINE ).arg( i ), 
                                            QString( TOK_LINE ).arg( i ) + " from customer_text" ) );
        for( int i = 0; i < NUM_FIELDS; i++ )
            category.tokens.append( tToken( QString( TOK_FIELD ).arg( i ), 
                                            QString( TOK_FIELD ).arg( i ) + " from barcode_basic" ) );
        category.tokens.append( tToken( TOK_CUSTOMER_NO, i18n("customer number of the current customer") ) );
        category.tokens.append( tToken( TOK_CUSTOEMR_NAME, i18n("name of the current customer") ) );
        category.tokens.append( tToken( TOK_BARCODE_NO, i18n("Barcode number from barcode_basic") ) );
        category.tokens.append( tToken( TOK_BARCODE_NO_BASIC, QString::null ) );
        category.tokens.append( tToken( TOK_GROUP, i18n("group of the current article") ) );
        category.tokens.append( tToken( TOK_ENCODING_TYPE, i18n("Barcode encoding type from barcode_basic") ) );
        category.tokens.append( tToken( TOK_ENCODING_TYPE_NAME, i18n("User readable barcode encoding type from barcode_basic") ) );
        category.category = CAT_DATABASE;
        s_categories.append( category );

        category.tokens.clear();
        category.tokens.append( tToken( TOK_FILENAME, i18n("Path and filename of this file") ) );
        category.tokens.append( tToken( TOK_INDEX, i18n("number of labels currently printed during this printout") ) );
        category.tokens.append( tToken( TOK_COL, i18n("current column on the page") ) );
        category.tokens.append( tToken( TOK_ROW, i18n("current row on the page") ) );
        category.tokens.append( tToken( TOK_PAGE, i18n("Current page") ) );
        category.tokens.append( tToken( TOK_RESOLUTION, i18n("Current resolution") ) );
        category.tokens.append( tToken( TOK_SERIAL, i18n("Include a serial number on your labels.") ) );
        category.category = CAT_LABEL;
        s_categories.append( category );

        category.tokens.clear();
	if( hasJavaScript() )
	    category.tokens.append( tToken( TOK_JS, i18n("Execute java script code"), true ) );
	category.tokens.append( tToken( TOK_SQL, i18n("Insert the result of a custom sql query"), true ) );
	category.tokens.append( tToken( TOK_VAR, i18n("Insert a custom variable"), true ) );
	category.category = CAT_CUSTOM;
	s_categories.append( category );

        category.tokens.clear();
        category.tokens.append( tToken( TOK_DATE, i18n("Date formated as configured in the preferences") ) );
        category.tokens.append( tToken( TOK_DATE_a, i18n("Short Weekday Name") ) );
        category.tokens.append( tToken( TOK_DATE_A, i18n("Full Weekday Name") ) );
        category.tokens.append( tToken( TOK_DATE_b, i18n("Short Month Name") ) );
        category.tokens.append( tToken( TOK_DATE_B, i18n("Full Month Name") ) );
        category.tokens.append( tToken( TOK_DATE_c, i18n("Date-Time") ) );
        category.tokens.append( tToken( TOK_DATE_C, i18n("Century") ) );
        category.tokens.append( tToken( TOK_DATE_d, i18n("Day Number (01-31)") ) );
        category.tokens.append( tToken( TOK_DATE_D, i18n("Month-Day-year") ) );
        category.tokens.append( tToken( TOK_DATE_e, i18n("Day Number ( 1-31)") ) );
        category.tokens.append( tToken( TOK_DATE_F, i18n("Year-Month-day") ) );
        category.tokens.append( tToken( TOK_DATE_g, i18n("Week Year (yy)") ) );
        category.tokens.append( tToken( TOK_DATE_G, i18n("Week Year (yyyy)") ) );
        category.tokens.append( tToken( TOK_DATE_H, i18n("Hour (01-24)") ) );
        category.tokens.append( tToken( TOK_DATE_I, i18n("Hour (1-12)") ) );
        category.tokens.append( tToken( TOK_DATE_J, i18n("Day of Year") ) );
        category.tokens.append( tToken( TOK_DATE_m, i18n("Month Number (01-12)") ) );
        category.tokens.append( tToken( TOK_DATE_M, i18n("Minute (00-59)") ) );
        category.tokens.append( tToken( TOK_DATE_p, i18n("a.m./p.m.") ) );
        category.tokens.append( tToken( TOK_DATE_r, i18n("Time (h:m:s am/pm)") ) );
        category.tokens.append( tToken( TOK_DATE_R, i18n("Time (HH:MM)") ) );
        category.tokens.append( tToken( TOK_DATE_S, i18n("Seconds (00-59)") ) );
        category.tokens.append( tToken( TOK_DATE_T, i18n("Time (HH:MM:SS)") ) );
        category.tokens.append( tToken( TOK_DATE_u, i18n("Weekday (1=Monday...)") ) );
        category.tokens.append( tToken( TOK_DATE_U, i18n("Week Number (00-53)") ) );
        category.tokens.append( tToken( TOK_DATE_V, i18n("Week Number (01-53)") ) );
        category.tokens.append( tToken( TOK_DATE_w, i18n("Weekday (0=Sunday...)") ) );
        category.tokens.append( tToken( TOK_DATE_W, i18n("Week Number (00-53)") ) );
        category.tokens.append( tToken( TOK_DATE_x, i18n("Local Date") ) );
        category.tokens.append( tToken( TOK_DATE_X, i18n("Local Time") ) );
        category.tokens.append( tToken( TOK_DATE_y, i18n("Year (00-99)") ) );        
        category.tokens.append( tToken( TOK_DATE_Y, i18n("Year (YYYY)") ) );
        category.tokens.append( tToken( TOK_DATE_z, i18n("Offset from UTC") ) );
        category.tokens.append( tToken( TOK_DATE_Z, i18n("Timezone Name") ) );
        category.category = CAT_DATE;
        s_categories.append( category );

        category.tokens.clear();
        category.tokens.append( tToken( TOK_ADDRESS_NAME, i18n("Name") ) );
        category.tokens.append( tToken( TOK_ADDRESS_GIVEN_NAME, KABC::Addressee::givenNameLabel() ) );
        category.tokens.append( tToken( TOK_ADDRESS_FAMILY_NAME, KABC::Addressee::familyNameLabel() ) );
        category.tokens.append( tToken( TOK_ADDRESS_EMAIL, KABC::Addressee::emailLabel() ) );
        category.tokens.append( tToken( TOK_ADDRESS_FULL_EMAIL, i18n("Full E-Mail") ) );
        category.tokens.append( tToken( TOK_ADDRESS_PHONE_PREF, KABC::PhoneNumber::label( KABC::PhoneNumber::Pref ) ) );
        category.tokens.append( tToken( TOK_ADDRESS_PHONE_HOME, KABC::PhoneNumber::label( KABC::PhoneNumber::Home ) ) );
        category.tokens.append( tToken( TOK_ADDRESS_PHONE_WORK, KABC::PhoneNumber::label( KABC::PhoneNumber::Work ) ) );
        category.tokens.append( tToken( TOK_ADDRESS_PHONE_MSG, KABC::PhoneNumber::label( KABC::PhoneNumber::Msg ) ) );
        category.tokens.append( tToken( TOK_ADDRESS_PHONE_VOICE, KABC::PhoneNumber::label( KABC::PhoneNumber::Voice ) ) );
        category.tokens.append( tToken( TOK_ADDRESS_PHONE_FAX, KABC::PhoneNumber::label( KABC::PhoneNumber::Fax ) ) );
        category.tokens.append( tToken( TOK_ADDRESS_PHONE_CELL, KABC::PhoneNumber::label( KABC::PhoneNumber::Cell ) ) );
        category.tokens.append( tToken( TOK_ADDRESS_PHONE_VIDEO, KABC::PhoneNumber::label( KABC::PhoneNumber::Video ) ) );
        category.tokens.append( tToken( TOK_ADDRESS_PHONE_BBS, KABC::PhoneNumber::label( KABC::PhoneNumber::Bbs ) ) ); 
        category.tokens.append( tToken( TOK_ADDRESS_PHONE_MODEM, KABC::PhoneNumber::label( KABC::PhoneNumber::Modem ) ) );
        category.tokens.append( tToken( TOK_ADDRESS_PHONE_CAR, KABC::PhoneNumber::label( KABC::PhoneNumber::Car ) ) );
        category.tokens.append( tToken( TOK_ADDRESS_PHONE_ISDN, KABC::PhoneNumber::label( KABC::PhoneNumber::Isdn ) ) );
        category.tokens.append( tToken( TOK_ADDRESS_PHONE_PCS, KABC::PhoneNumber::label( KABC::PhoneNumber::Pcs ) ) );
        category.tokens.append( tToken( TOK_ADDRESS_PHONE_PAGER, KABC::PhoneNumber::label( KABC::PhoneNumber::Pager ) ) );
        category.tokens.append( tToken( TOK_ADDRESS_ADDRESS_POB, KABC::Address::postOfficeBoxLabel() ) );
        category.tokens.append( tToken( TOK_ADDRESS_ADDRESS_EXTENDED, KABC::Address::extendedLabel() ) );
        category.tokens.append( tToken( TOK_ADDRESS_ADDRESS_STREET, KABC::Address::streetLabel() ) );
        category.tokens.append( tToken( TOK_ADDRESS_ADDRESS_LOCALITY, KABC::Address::localityLabel() ) );
        category.tokens.append( tToken( TOK_ADDRESS_ADDRESS_REGION, KABC::Address::regionLabel() ) );
        category.tokens.append( tToken( TOK_ADDRESS_ADDRESS_POSTALCODE, KABC::Address::postalCodeLabel() ) );
        category.tokens.append( tToken( TOK_ADDRESS_ADDRESS_COUNTRY, KABC::Address::countryLabel() ) );
        category.tokens.append( tToken( TOK_ADDRESS_ADDRESS_LABEL, KABC::Address::labelLabel() ) );
        category.tokens.append( tToken( TOK_ADDRESS, i18n("Formatted Address") ) );
        category.tokens.append( tToken( TOK_ADDRESS_URL, KABC::Addressee::urlLabel() ) );
        category.tokens.append( tToken( TOK_ADDRESS_NICK, KABC::Addressee::nickNameLabel() ) );
        category.tokens.append( tToken( TOK_ADDRESS_ADDITIONAL_NAME, KABC::Addressee::additionalNameLabel() ) );
        category.tokens.append( tToken( TOK_ADDRESS_PREFIX, KABC::Addressee::prefixLabel() ) );
        category.tokens.append( tToken( TOK_ADDRESS_SUFFIX, KABC::Addressee::suffixLabel() ) );
        category.tokens.append( tToken( TOK_ADDRESS_BIRTHDAY, KABC::Addressee::birthdayLabel() ) );
        category.tokens.append( tToken( TOK_ADDRESS_TITLE, KABC::Addressee::titleLabel() ) );
        category.tokens.append( tToken( TOK_ADDRESS_ROLE, KABC::Addressee::roleLabel() ) );
        category.tokens.append( tToken( TOK_ADDRESS_NOTE, KABC::Addressee::noteLabel() ) );
        category.tokens.append( tToken( TOK_ADDRESS_ORGANIZATION, KABC::Addressee::organizationLabel() ) );
        category.category = CAT_ADDRESS;
        s_categories.append( category );
    }
}


void TokenProvider::findBrackets( QString & text, QString (TokenProvider::*parserfunction)( const QString & ) )
{
    /*
     * looks for a statement in brackets [ ]
     * and calls findToken() with this statement.
     */

    int num, pos = -1, a;
    QString token;

    if( text.contains("]", FALSE) <= 0 || text.isEmpty() )
        return;

    num = text.contains("[", FALSE);
    if(num <= 0 )
        return;

    pos = text.findRev("[", pos);
    a = text.find("]", pos );
    if( a < 0 && pos >= 0 )
        return;

    if( pos < 0 && a >= 0 )
        return;

     if( pos >= 0 && a >= 0 ) {
        token = text.mid( pos+1, (a-pos)-1 );
        
        // support [4-[length]]
	findBrackets( token, parserfunction );

        token = unescapeText( token );
        token = escapeText( (*this.*parserfunction)( token ) );
	if( !token.isNull() )
	{
	    text.remove( pos, (a-pos)+1 );
	    text.insert( pos, token );
	}
	else
            // TODO: if we have a textfield with two variables:
            // [index] [non_existant], index will not be parsed
            // because of the non_existant field (which might be 
            // caused by a typo...

	    // otherwise we would end up in and endless recursion
	    // no one wants that...
	    return;
    }
    
     findBrackets( text, parserfunction );
}

QString TokenProvider::parse( const QString & text )
{
    QString t = QString( text );
    findBrackets( t, &TokenProvider::process );
    return t;
}

QStringList TokenProvider::listUserVars()
{
    unsigned int i;
    DocumentItem* item;
    QStringList lst;
    QString t;

    m_findUserVarsList = &lst;

    for( i=0;i<m_document_items.count();i++ ) 
    {
	item = m_document_items.at(i);
	t = QString::null;
	if( item->rtti() == eRtti_Barcode )
	    t = ((BarcodeItem*)item)->value();
	else if( item->rtti() == eRtti_Text )
	    t = ((TextItem*)item)->text();
	else if( item->rtti() == eRtti_TextLine )
	    t = ((TextLineItem*)item)->text();

	if( !t.isNull() )
	    findBrackets( t, &TokenProvider::processUserVars );
    }

    m_findUserVarsList = NULL;

    lst.sort();
    return lst;
}

QString TokenProvider::processUserVars( const QString & t )
{
    QStringList::Iterator it;
    bool found = false;

    if( m_findUserVarsList )
    {
	if( t.startsWith( TOK_VAR ) )
	{
	    // every token starting with $ is a user defined variable
	    for(it = m_findUserVarsList->begin(); it != m_findUserVarsList->end(); ++it ) 
		if( (*it).toLower() == t.toLower() )
		{
		    found = true;
		    break;
		}

	    if( !found ) 
		m_findUserVarsList->append( t );
	}
    }
    
    return t;
}

QString TokenProvider::process( const QString & t )
{
    QString ret = QString::null;
    QString addr;
    int i;

    // process all token that require an sql connection
    if( SqlTables::isConnected()  )
    {
	if( !article_no.isEmpty() )
	{
	    for( i = 0; i < NUM_FIELDS; i++ ) 
	    {
		const QString c = QString( TOK_LINE ).arg( i );
		if( t == c )
		    ret = query( "SELECT " +  c + " FROM " + TABLE_CUSTOMER_TEXT + " WHERE article_no='" +  article_no  + 
				 "' AND customer_no='" + customer_no + "'");
	    }

	    for( i = 0; i < NUM_FIELDS; i++ ) 
	    {
		const QString c = QString( TOK_FIELD ).arg( i );
		if( t == c )
		    ret = query("SELECT " + c + " FROM " + TABLE_BASIC + " WHERE article_no='" +  article_no  + "'");
	    }
    
    
	    if( t == TOK_ARTICLE_NO_CUSTOMER ) 
		ret = query( QString("SELECT article_no_customer FROM ") + TABLE_CUSTOMER_TEXT + " WHERE article_no='" +
			     article_no  + "' AND customer_no='" + customer_no + "'");

	    if( t == TOK_BARCODE_NO_CUSTOMER )
		ret = query( QString("SELECT barcode_no FROM ") + TABLE_CUSTOMER_TEXT + " WHERE article_no='" + 
			 article_no  + "' AND customer_no='" + customer_no + "'");
	    
	    if( t == TOK_ARTICLE_DESC )
		ret = query( QString("SELECT article_desc FROM ") + TABLE_BASIC  + " WHERE article_no='" +  article_no  + "'");

	    if( t == TOK_BARCODE_NO )
		ret = query( QString("SELECT barcode_no FROM ") + TABLE_BASIC  + " WHERE article_no='" +  article_no  + "'");

	    if( t == TOK_ARTICLE_NO )
		ret = article_no;
	}

	if( !customer_no.isEmpty() && t == TOK_CUSTOEMR_NAME )
	    ret = query( QString("SELECT customer_name  FROM ") + TABLE_CUSTOMER + " WHERE customer_no='" +  customer_no  + "'");

	const QString sqlq = TOK_SQL;
	if( t.startsWith( sqlq ) )
	    ret = query( t.right( t.length() - sqlq.length() ) );
    }

    if( !customer_no.isEmpty() && t == TOK_CUSTOMER_NO )
        ret = customer_no;

    if( !barcode_no.isEmpty() && t == TOK_BARCODE_NO )
        ret = barcode_no;

    if( !group.isEmpty() && t == TOK_GROUP )
	ret = group;

    if( !encoding_type_name.isEmpty() && t == TOK_ENCODING_TYPE_NAME )
        ret = encoding_type_name;

    if( !label_name.isEmpty() && t == TOK_FILENAME )
        ret = label_name;

    if( t == TOK_DATE )
        ret = QDateTime::currentDateTime().toString( KBarcodeSettings::getDateFormat() );
    
    if( date_reg_exp.search(t,0) != -1 ) 
    {
	time_t label_time;
	struct tm label_time_struct;
	char temp_time_str[50] ;

	label_time = time(&label_time) ;
	localtime_r(&label_time,&label_time_struct) ;

        strftime(temp_time_str,sizeof temp_time_str - 1, date_reg_exp.cap(1).ascii(),&label_time_struct);
        ret = temp_time_str;
    }

    if( t == TOK_INDEX )
    {
        m_contains_update = true;
        ret = QString::number( m_index + 1 );
    }

    if( t == TOK_ROW )
    {
        m_contains_update = true;
        ret = QString::number( row + 1 );
    }

    if( t == TOK_COL )
    {
        m_contains_update = true;
        ret = QString::number( col + 1 );
    }

    if( t == TOK_PAGE ) 
    {
        m_contains_update = true;
        ret = QString::number( m_page + 1 );
    }

    if( m_printer && t == TOK_RESOLUTION ) 
    {
	ret = QString( "%1dpi" ).arg( m_printer->logicalDpiY() );
    }

    if( !m_serial.isEmpty() && t == TOK_SERIAL )
    {
        m_contains_update = true;
        ret = createSerial();
    }


    addr = processAddresses( t );
    if( !addr.isNull() )
        ret = addr;

    // let's parse java script code
    const QString js = TOK_JS;
    if( t.startsWith( js ) )
	ret = jsParse( t.right( t.length() - js.length() ) );
    
    // everything starting with a $ is a user defined variable
    if( !m_uservardata.isEmpty() ) 
    {
	QString tok = t.right( t.length() - 1 );
	if( m_uservardata.contains( tok ) )
        {
	    ret = m_uservardata[tok];
            if( ret.isNull() )
                ret = ""; // we want an empty string and not a null string
        }
    }

    return ret;
}

QString TokenProvider::processAddresses( const QString & t )
{
    QString ret = QString::null;
    bool found = false;

    if( !m_address || !t.startsWith( "address" ) )
        return ret;

    KABC::Address addr = m_address->address( KABC::Address::Pref );
    if( addr.isEmpty() )
    {
        KABC::Address::List list = m_address->addresses();
        for( int i=0;i<list.count();i++ )
            if( !list[i].isEmpty() ) 
            {
                addr = list[i];
                break;
            }
    }

    if( t == TOK_ADDRESS_NAME )  
    {
        ret = m_address->realName();
        found = true;
    } 
    else if( t == TOK_ADDRESS_GIVEN_NAME )
    {
        ret = m_address->givenName();
        found = true;
    }
    else if( t == TOK_ADDRESS_FAMILY_NAME )
    {
        ret = m_address->familyName();
        found = true;
    }
    else if( t == TOK_ADDRESS_EMAIL )
    {
        ret = m_address->preferredEmail();
        found = true;
    }
    else if( t == TOK_ADDRESS_FULL_EMAIL )
    {
        ret = m_address->fullEmail();
        found = true;
    }
    else if( t == TOK_ADDRESS_PHONE_PREF )
    {
        ret = m_address->phoneNumber( KABC::PhoneNumber::Pref ).number();
        found = true;
    }
    else if( t == TOK_ADDRESS_PHONE_HOME )
    {
        ret = m_address->phoneNumber( KABC::PhoneNumber::Home ).number();
        found = true;
    }
    else if( t == TOK_ADDRESS_PHONE_WORK )
    {
        ret = m_address->phoneNumber( KABC::PhoneNumber::Work ).number();
        found = true;
    }
    else if( t == TOK_ADDRESS_PHONE_MSG )
    {
        ret = m_address->phoneNumber( KABC::PhoneNumber::Msg ).number();
        found = true;
    }
    else if( t == TOK_ADDRESS_PHONE_VOICE )
    {
        ret = m_address->phoneNumber( KABC::PhoneNumber::Voice ).number();
        found = true;
    }
    else if( t == TOK_ADDRESS_PHONE_FAX )
    {
        ret = m_address->phoneNumber( KABC::PhoneNumber::Fax ).number();
        found = true;
    }
    else if( t == TOK_ADDRESS_PHONE_CELL )
    {
        ret = m_address->phoneNumber( KABC::PhoneNumber::Cell ).number();
        found = true;
    }
    else if( t == TOK_ADDRESS_PHONE_VIDEO )
    {
        ret = m_address->phoneNumber( KABC::PhoneNumber::Video ).number();
        found = true;
    }
    else if( t == TOK_ADDRESS_PHONE_BBS )
    {
        ret = m_address->phoneNumber( KABC::PhoneNumber::Bbs ).number();
        found = true;
    }
    else if( t == TOK_ADDRESS_PHONE_MODEM )
    {
        ret = m_address->phoneNumber( KABC::PhoneNumber::Modem ).number();
        found = true;
    }
    else if( t == TOK_ADDRESS_PHONE_CAR )
    {
        ret = m_address->phoneNumber( KABC::PhoneNumber::Car ).number();
        found = true;
    }
    else if( t == TOK_ADDRESS_PHONE_ISDN )
    {
        ret = m_address->phoneNumber( KABC::PhoneNumber::Isdn ).number();
        found = true;
    }
    else if( t == TOK_ADDRESS_PHONE_PCS )
    {
        ret = m_address->phoneNumber( KABC::PhoneNumber::Pcs ).number();
        found = true;
    }
    else if( t == TOK_ADDRESS_PHONE_PAGER )
    {
        ret = m_address->phoneNumber( KABC::PhoneNumber::Pager ).number();        
        found = true;
    }
    else if( t == TOK_ADDRESS_ADDRESS_POB )
    {
        ret = addr.postOfficeBox();
        found = true;
    }
    else if( t == TOK_ADDRESS_ADDRESS_EXTENDED )
    {
        ret = addr.extended();
        found = true;
    }
    else if( t == TOK_ADDRESS_ADDRESS_STREET )
    {
        ret = addr.street();
        found = true;
    }
    else if( t == TOK_ADDRESS_ADDRESS_LOCALITY )    
    {
        ret = addr.locality();
        found = true;
    }
    else if( t == TOK_ADDRESS_ADDRESS_REGION )
    {
        ret = addr.region();
        found = true;
    }
    else if( t == TOK_ADDRESS_ADDRESS_POSTALCODE )
    {
        ret = addr.postalCode();
        found = true;
    }
    else if( t == TOK_ADDRESS_ADDRESS_COUNTRY )
    {
        ret = addr.country();
        found = true;
    }
    else if( t == TOK_ADDRESS_ADDRESS_LABEL )
    {
        ret = addr.label();
        found = true;
    }
    else if( t == TOK_ADDRESS )
    {
        ret = addr.formattedAddress( m_address->realName(), m_address->organization() );
        found = true;
    }
    else if( t == TOK_ADDRESS_URL )
    {
        ret = m_address->url().prettyUrl();
        found = true;
    }
    else if( t == TOK_ADDRESS_NICK )
    {
        ret = m_address->nickName();
        found = true;
    }
    else if( t == TOK_ADDRESS_ADDITIONAL_NAME )
    {
        ret = m_address->additionalName();
        found = true;
    }
    else if( t == TOK_ADDRESS_PREFIX )
    {
        ret = m_address->prefix();
        found = true;
    }
    else if( t == TOK_ADDRESS_SUFFIX )
    {
        ret = m_address->suffix();
        found = true;
    }
    else if( t == TOK_ADDRESS_BIRTHDAY )
    {
        ret = m_address->birthday().date().toString();
        found = true;
    }
    else if( t == TOK_ADDRESS_TITLE )
    {
        ret = m_address->title();
        found = true;
    }
    else if( t == TOK_ADDRESS_ROLE )
    {
        ret = m_address->role();
        found = true;
    }
    else if( t == TOK_ADDRESS_NOTE )
    {
        ret = m_address->note();
        found = true;
    }
    else if( t == TOK_ADDRESS_ORGANIZATION )
    {
        ret = m_address->organization();
        found = true;
    }
    else
        ret = t;

    if( found && ret.isNull() )
        ret = ""; // empty string, not null string

    return ret;
}

QString TokenProvider::query( const QString & query )
{
    if( !query.isEmpty() )
    {
	QSqlQuery q( query );
	while( q.next() )
	    return q.value( 0 ).toString();
	
	return QString::null;
    }
    else
	return i18n("ERROR: Empty SQL Query");
}

QString TokenProvider::jsParse( const QString & script )
{
    QString ret = QString::null;

    ret = "ERROR: No JavaScript interpreter present!";

#ifdef USE_JAVASCRIPT
#if 0
    KJS::Interpreter *js = KParts::ComponentFactory::createInstanceFromQuery<KJS::Interpreter>( "KJSEmbed/KJSEmbed" );
    qDebug("JavaScript: %p", js );
    // KJS::Interpreter *js = KJSEmbed::JSFactory::defaultJS()
    // KJS::Interpreter *js = new KJS::Interpreter(); //KJSEmbed::JSFactory::defaultJS();
    KJS::Completion comp;
    KJS::Value self;
    
    bool ok = 0;//js->execute( comp, script, self );
    if( ok && comp.isValueCompletion() )
    {
	KJS::UString s = comp.value().toString( js->globalExec() );
	ret = s.qstring();
    }

    //delete js;
//#else
    KJS::Interpreter *js = new KJS::Interpreter(); //KJSEmbed::JSFactory::defaultJS();
    KJS::Completion comp;

    comp = js->evaluate( KJS::UString( script.toLatin1() ) );
    if( comp.complType() == KJS::ReturnValue || comp.complType() == KJS::Normal )
    {
	ret = comp.value().toString( js->globalExec() ).cstring().c_str();
    }

#else
    // Maybe we need no Completion object for KJSEmbed
    KJS::Completion comp = s_interpreter->evaluate( KJS::UString( script.toLatin1() ) );
    KJS::Value val = comp.value();
    if( val.isValid() )
	ret = val.toString( s_interpreter->globalExec() ).cstring().c_str();
#endif

#endif // USE_JAVASCRIPT

    return ret;
}

bool TokenProvider::jsParseToBool( const QString & script )
{
#ifdef USE_JAVASCRIPT
    // Maybe we need no Completion object for KJSEmbed
    KJS::Completion comp = s_interpreter->evaluate( KJS::UString( script.toLatin1() ) );
    KJS::Value val = comp.value();
    if( val.isValid() )
    {
	return val.toBoolean( s_interpreter->globalExec() );
    }
#endif // USE_JAVASCRIPT

    return false;
}

const QString TokenProvider::createSerial()
{
    unsigned long tmp;
    QString s = QString::null;
    // Split string into prenum, number and postnum parts
    QRegExp splitit("(\\D*)(\\d+)(.*)");
    int pos = splitit.search(m_serial);

    // Is there anything to increment ?
    if (pos > -1) {
        QString prenum = splitit.cap(1);

        tmp = splitit.cap(2).toULong();
        QString postnum = splitit.cap(3);
        tmp += (m_increment*m_index);
        QString tmpstr;
        tmpstr.setNum(splitit.cap(2).length());
        QString formatstring = "%0" + tmpstr + "lu";

        s = prenum + tmpstr.sprintf(formatstring, tmp) + postnum;
        m_update = true;
    }

    return s;
}

QString TokenProvider::unescapeText( const QString & t )
{
    QString tmp = t;
    tmp = tmp.replace( DSREPLACE( "&lt;" ), "<" );
    tmp = tmp.replace( DSREPLACE( "&gt;" ), ">" );
    tmp = tmp.replace( DSREPLACE( "&amp;" ), "&" );
    tmp = tmp.replace( DSREPLACE( "&quot;" ), "\"" );

    return tmp;
}

QString TokenProvider::escapeText( const QString & t )
{
    QString tmp = t;
    tmp = tmp.replace( DSREPLACE( "<" ), "&lt;" );
    tmp = tmp.replace( DSREPLACE( ">" ), "&gt;" );
    tmp = tmp.replace( DSREPLACE( "&" ), "&amp;"  );
    tmp = tmp.replace( DSREPLACE( "\"" ), "&quot;" );
    tmp = tmp.replace( DSREPLACE( "\n" ), "<br />" );

    return tmp;
}
