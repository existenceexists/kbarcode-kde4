//
// C++ Implementation: tec
//
// Description:
//
//
// Author: Dominik Seichter <domseichter@web.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "tec.h"
#include "kbarcodesettings.h"
#include "tokenprovider.h"

// Other includes
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <math.h>
#include <unistd.h>

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
#include <QDomDocument>
#include <QDomNode>
#include <QDomElement>
//Added by qt3to4:
#include <QSqlQuery>

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

#define MAXDATASIZE 32550

QString tec452(const QString &url, int count, const QString &art, const QString &group, const QString &id)
{
    QString filename;
    QString result;
    int countBar=0;
    int countText=0;

//    if( url.isEmpty() ) {
//        return open();
//    }

    filename = url;
    //setCaption( filename, false );

    QFile f( filename );
    if ( !f.open( QIODevice::ReadOnly ) )
        return false;

    //clearLabel();

    QDomDocument doc( "KBarcodeLabel" );
    if ( !doc.setContent( &f ) ) {
        f.close();
        return false;
    }
    f.close();

    QDomNode n = doc.documentElement().firstChild();
    // this should not be neccessary,
    // but <label><id> needs to be processed first
    while( !n.isNull() ) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if( !e.isNull() )
            // label has to be the first xml element!
            if( e.tagName() == "label" ) {
                QDomNode n = e.firstChild();
                while( !n.isNull() ) {
                    QDomElement e = n.toElement(); // try to convert the node to an element.
                    if( !e.isNull() )
                        if( e.tagName() == "id" ) {
				result += QString("{D%1,").arg( convZeros((e.attribute( "height" ).toInt() + e.attribute( "gap_v" ).toInt())*10,4)); // Altura + Gap vertical
				result += QString("%1,").arg( convZeros(e.attribute( "width" ).toInt()*10,4) ); // Anchura
			        result += QString("%1|}\n").arg( convZeros(e.attribute( "height" ).toInt()*10,4) ); // Altura
				result += QString("{C|}\n"); // Clear buffer
				result += QString("{U2;0130|}\n"); // BackFeed
				result += QString("{D%1,").arg( convZeros((e.attribute( "height" ).toInt() + e.attribute( "gap_v" ).toInt())*10,4)); // Altura + Gap vertical
				result += QString("%1,").arg( convZeros(e.attribute( "width" ).toInt()*10,4) ); // Anchura
			        result += QString("%1|}\n").arg( convZeros(e.attribute( "height" ).toInt()*10,4) ); // Altura
				result += QString("{AX;+000,+000,+00|}\n"); // Position Fine adjust
			    	result += QString("{AY;+04,1|}\n"); // Print density fine adjust

                        }

                    n = n.nextSibling();
                }
            }
        n = n.nextSibling();
    }

    n = doc.documentElement().firstChild();
    while( !n.isNull() ) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if( !e.isNull() )
            if( e.tagName() == "barcode" ) {

	    	if ( e.attribute( "type" ) == "i25" or e.attribute( "type" ) == "i25 -c" or e.attribute( "type" ) == "code39" or e.attribute( "type" ) == "code39 -c" or e.attribute( "type" ) == "b7" or e.attribute( "type" ) == "b8" or e.attribute( "type" ) == "b9" or e.attribute( "type" ) == "pls" or e.attribute( "type" ) == "msi" or e.attribute( "type" ) == "b33") result += code39(e, countBar);

		if ( e.attribute( "type" ) == "ean" or e.attribute( "type" ) == "upc" or e.attribute( "type" ) == "code93" or e.attribute( "type" ) == "code128" or e.attribute( "type" ) == "b10" or e.attribute( "type" ) == "b11" or e.attribute( "type" ) == "b15" or e.attribute( "type" ) == "b14" or e.attribute( "type" ) == "b34" or e.attribute( "type" ) == "b35" or e.attribute( "type" ) == "b36" or e.attribute( "type" ) == "b37" or e.attribute( "type" ) == "b39" or e.attribute( "type" ) == "b39" or e.attribute( "type" ) == "b13" or e.attribute( "type" ) == "b12" or e.attribute( "type" ) == "b20" or e.attribute( "type" ) == "b25") result += code128(e, countBar);

		if ( e.attribute( "type" ) == "b55" ) result += pdf417(e, countBar);

	        QString data;
                QDomNode n = e.firstChild();
                while( !n.isNull() ) {
                    QDomElement e = n.toElement(); // try to convert the node to an element.
                    if( !e.isNull() )
                        if( e.tagName() == "value" ) data = e.text();
                    n = n.nextSibling();
                }
		result += QString("{RB%1;").arg( convZeros(countBar, 2) ); // count de RB
	        result += QString("%1|}\n").arg( data ); // Data + fin
		countBar++;

	    } else if( e.tagName() == "textfield" ) {

		result += QString("{PC%1;").arg( convZeros(countText, 3) ); // count
	        result += QString("%1,").arg( posConv(e.attribute( "x_mm"), 4 ) ); // X position
	        result += QString("%1,").arg( posConv(e.attribute( "y_mm"), 4 ) ); // Y position
	        result += QString("%1,").arg( "10" );
	        result += QString("%1,").arg( "15" );
	        result += QString("%1,").arg( "J" );
	        result += QString("%1,").arg( "00" );
	        result += QString("%1|}\n").arg( "B" );

                QString etext;
                QDomNode n = e.firstChild();
                while( !n.isNull() ) {
                    QDomElement e = n.toElement(); // try to convert the node to an element.
                    if( !e.isNull() )
                        if( e.tagName() == "text" )
                            etext = e.text();
                    n = n.nextSibling();
                }

		result += QString("{RC%1;").arg( convZeros(countText, 3) ); // count de RB
		etext=getData(etext, art, group, id );
		result += QString("%1|}\n").arg( etext ); // Data + fin
		countText++;
            }
            else if( e.tagName() == "rect" ) {

	    	if (e.attribute("colorr")=="0" and e.attribute("colorg")=="0" and e.attribute("colorb")=="0") {

			result += QString("{XR;"); // XR
			result += QString("%1,").arg( posConv(e.attribute( "x_mm"), 4 ) ); // X position
	        	result += QString("%1,").arg( posConv(e.attribute( "y_mm"), 4 ) ); // Y position
	        	result += QString("%1,").arg( convZeros(posConv(e.attribute( "x_mm"), 4 ).toInt()+posConv(e.attribute( "width_mm"),4 ).toInt(),4)); // X position
		        result += QString("%1,").arg( convZeros(posConv(e.attribute( "y_mm"), 4 ).toInt()+posConv(e.attribute( "height_mm"),4 ).toInt(),4)); // Y position
	        	result += QString("A"); // Type of Clear
			result += QString("|}\n"); // FIN
			result += QString("{XR;"); // XR
			result += QString("%1,").arg( posConv(e.attribute( "x_mm"), 4 ) ); // X position
	        	result += QString("%1,").arg( posConv(e.attribute( "y_mm"), 4 ) ); // Y position
	        	result += QString("%1,").arg( convZeros(posConv(e.attribute( "x_mm"), 4 ).toInt()+posConv(e.attribute( "width_mm"),4 ).toInt(),4)); // X position
		        result += QString("%1,").arg( convZeros(posConv(e.attribute( "y_mm"), 4 ).toInt()+posConv(e.attribute( "height_mm"),4 ).toInt(),4)); // Y position
	        	result += QString("B"); // Type of Clear
			result += QString("|}\n"); // FIN
	    		}
		else {
			result += QString("{LC;"); // LC
	        	result += QString("%1,").arg( posConv(e.attribute( "x_mm"), 4 ) ); // X position
	        	result += QString("%1,").arg( posConv(e.attribute( "y_mm"), 4 ) ); // Y position
	        	result += QString("%1,").arg( convZeros(posConv(e.attribute( "x_mm"), 4 ).toInt()+posConv(e.attribute( "width_mm"),4 ).toInt(),4)); // X position
		        result += QString("%1,").arg( convZeros(posConv(e.attribute( "y_mm"), 4 ).toInt()+posConv(e.attribute( "height_mm"),4 ).toInt(),4)); // Y position
	        	result += QString("1,"); // Type of line 0=line 1=Rectangulo 2=Jagged line 3=Rectangle with jagged lines
			result += QString("%1").arg(lineWidth(e)); // No. of line width dots 1-9
			//result += QString("999"); // Radius of rounded corners of a rectangle
	    		result += QString("|}\n"); // FIN
	    		}
	    }
	    else if( e.tagName() == "line" ) {
	    		result += QString("{LC;"); // LC
	        	result += QString("%1,").arg( posConv(e.attribute( "x_mm"), 4 ) ); // X position
	        	result += QString("%1,").arg( posConv(e.attribute( "y_mm"), 4 ) ); // Y position
	        	result += QString("%1,").arg( convZeros(posConv(e.attribute( "x_mm"), 4 ).toInt()+posConv(e.attribute( "width_mm"),4 ).toInt(),4)); // X position
		        result += QString("%1,").arg( convZeros(posConv(e.attribute( "y_mm"), 4 ).toInt()+posConv(e.attribute( "height_mm"),4 ).toInt(),4)); // Y position
	        	result += QString("0,"); // Type of line 0=line 1=Rectangulo 2=Jagged line 3=Rectangle with jagged lines
			result += QString("%1").arg(lineWidth(e)); // No. of line width dots 1-9
			//result += QString("999"); // Radius of rounded corners of a rectangle
	    		result += QString("|}\n"); // FIN
	    }
        n = n.nextSibling();
    }

    result += QString("{U1;0130|}\n"); // ForwardFeed
    result += QString("{XS;I,%1,0000C2000|}\n").arg(convZeros(count,4)); // Issue Command

    return result;
}


QString convZeros( int count, int zeros )
{
QString str= QString("%1").arg(count);
int l=str.length(); //get the string length
for(;l<zeros;l++) str = QString("0") + str;
return str;
}

QString posConv( QString str, int zeros )
{
int point=str.find(".",0);
QString dec= QString("%1").arg(str.mid(point+1,1));
str = str.mid(0,point);
str = QString("%1%2").arg(str).arg(dec);
return convZeros (str.toInt(), zeros);
}

int rotation (int valor){
if (valor==90) return 1;
else if  (valor==180) return 2;
else if  (valor==270) return 3;
else return 0;
}


QString code39 (const QDomElement &e, int countBar) {

QString result;
		result += QString("{XB%1;").arg( convZeros(countBar, 2) ); // count
	        result += QString("%1").arg( posConv(e.attribute( "x_mm"), 4 ) ); // X position
	        result += QString(",%1").arg( posConv(e.attribute( "y_mm"), 4 ) ); // Y position
	        result += QString(",%1").arg( barcodeType( e.attribute( "type") ) ); // Type of bar code
	        result += QString(",%1").arg( withChecksum(e) ); // Type of check digit
	        result += QString(",%1").arg( "03" ); // Narrow bar width
	        result += QString(",%1").arg( "03" ); // Narrow space width
	        result += QString(",%1").arg( "06" ); // Wide bar width
	        result += QString(",%1").arg( "06" ); // Wide space width
	        result += QString(",%1").arg( "03" ); // Character to Character space width
	        result += QString(",%1").arg( rotation(e.attribute( "rotation", "0" ).toInt())); // Rotation angel of barcode
	        result += QString(",%1").arg( "0063" ); // height of barcode
	        result += QString(",+%1").arg( sequence(e) ); // Increment / Decrement
	        result += QString(",%1").arg( e.attribute( "text", "1" ).toInt() ); // Selection of print or non print of numerals under bars
		result += QString(",00"); //Number of Zeros to be suppressed
	        result += QString("|}\n"); // FIN

return result;
}

QString barcodeType(const QString &type){
if (type == "i25") return "2";
if (type == "i25 -c") return "2";
if (type == "msi") return "1";
if (type == "code39") return "3";
if (type == "code39 -c") return "3";
if (type == "b8") return "3";
if (type == "ean") return "0";
if (type == "upc") return "K";
if (type == "code128") return "9";
if (type == "code93") return "C";
if (type == "pls") return "1";
if (type == "b7") return "O";
if (type == "b9") return "B";
if (type == "b10") return "0";
if (type == "b11") return "I";
if (type == "b12") return "J";
if (type == "b13") return "5";
if (type == "b14") return "7";
if (type == "b15") return "8";
if (type == "b34") return "K";
if (type == "b35") return "L";
if (type == "b36") return "M";
if (type == "b37") return "6";
if (type == "b38") return "G";
if (type == "b39") return "H";
if (type == "b20") return "9";
if (type == "b25") return "C";
if (type == "b47") return "1";
//never
return "0";
}


QString withChecksum( const QDomElement &e) {
if (e.attribute("type")=="code39 -c") return "1";
if (e.attribute("type")=="code39") return "3";
if (e.attribute("type")=="ean") return "3";
if (e.attribute("type")=="upc") return "3";
if (e.attribute("type")=="code128") return "2";
if (e.attribute("type")=="code93") return "2";
if (e.attribute("type")=="i25 -c") return "1";
if (e.attribute("type")=="i25") return "3";
if (e.attribute("type")=="msi") return "1";
if (e.attribute("type")=="pls") return "3";
if (e.attribute("type")=="b7" )
	{if (e.attribute("tbarcode.checksum")=="0") return "1";
	else return "3";
	}
if (e.attribute("type")=="b8" )
	{if (e.attribute("tbarcode.checksum")=="0") return "1";
	else return "3";
	}
if (e.attribute("type")=="b9" )
	{if (e.attribute("tbarcode.checksum")=="0") return "1";
	else return "3";
	}

if (e.attribute("type")=="b10" ) return "3";
if (e.attribute("type")=="b11" ) return "3";
if (e.attribute("type")=="b12" ) return "3";
if (e.attribute("type")=="b13" ) return "3";
if (e.attribute("type")=="b14" ) return "3";
if (e.attribute("type")=="b15" ) return "3";
if (e.attribute("type")=="b34" )
	{if (e.attribute("tbarcode.checksum")=="0") return "1";
	else return "3";
	}
if (e.attribute("type")=="b35" )
	{if (e.attribute("tbarcode.checksum")=="0") return "1";
	else return "3";
	}
if (e.attribute("type")=="b36" )
	{if (e.attribute("tbarcode.checksum")=="0") return "1";
	else return "3";
	}
if (e.attribute("type")=="b37" ) return "3";
if (e.attribute("type")=="b38" ) return "3";
if (e.attribute("type")=="b39" ) return "3";
if (e.attribute("type")=="b20" ) return "1";
if (e.attribute("type")=="b25" ) return "1";
if (e.attribute("type")=="b47" )
	{if (e.attribute("tbarcode.checksum")=="10") return "3";
	else if (e.attribute("tbarcode.checksum")=="11") return "4";
	else if (e.attribute("tbarcode.checksum")=="2") return "5";
	else if (e.attribute("tbarcode.checksum")=="1") return "3";
	return "1";
	}
if (e.attribute("type")=="b55" )
	{if (e.attribute("pdf417.err")=="0") return "00";
	else if (e.attribute("pdf417.err")=="1") return "01";
	else if (e.attribute("pdf417.err")=="2") return "02";
	else if (e.attribute("pdf417.err")=="3") return "03";
	else if (e.attribute("pdf417.err")=="4") return "04";
	else if (e.attribute("pdf417.err")=="5") return "05";
	else if (e.attribute("pdf417.err")=="6") return "06";
	else if (e.attribute("pdf417.err")=="7") return "07";
	else if (e.attribute("pdf417.err")=="8") return "08";
	}
//never
return "0";
}


QString sequence(const QDomElement &e){
return convZeros(e.attribute("sequencestep" ,"0").toInt() ,10);
}

QString code128 (const QDomElement &e, int countBar) {

QString result;
		result += QString("{XB%1;").arg( convZeros(countBar, 2) ); // count
	        result += QString("%1").arg( posConv(e.attribute( "x_mm"), 4 ) ); // X position
	        result += QString(",%1").arg( posConv(e.attribute( "y_mm"), 4 ) ); // Y position
	        result += QString(",%1").arg( barcodeType( e.attribute( "type") ) ); // Type of bar code
	        result += QString(",%1").arg( withChecksum(e) ); // Type of check digit
	        result += QString(",%1").arg( "03" ); // 1 module width
	        result += QString(",%1").arg( rotation(e.attribute( "rotation", "0" ).toInt())); // Rotation angel of barcode
	        result += QString(",%1").arg( "0063" ); // height of barcode
	        result += QString(",+%1").arg( sequence(e) ); // Increment / Decrement
		result += QString(",000"); // Length of th WPC guard bar
	        result += QString(",%1").arg( e.attribute( "text", "1" ).toInt() ); // Selection of print or non print of numerals under bars
		result += QString(",00"); //Number of Zeros to be suppressed
	        result += QString("|}\n"); // FIN

return result;
}

#if 0 
QString serialWatch(){

int fd, res, exit=1;
struct termios oldtio,newtio;
char buf[MAXDATASIZE];
QString result,temp;


//fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY );
fd = open("/dev/ttyS1", O_RDWR | O_NOCTTY );
if (fd <0) return "Port Error.";
tcgetattr(fd,&oldtio); /* save current serial port settings */
bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */
newtio.c_cflag = B9600 | CRTSCTS | CS8 | CLOCAL | CREAD;
newtio.c_iflag = IGNPAR; // | ICRNL;
newtio.c_oflag = 0;
newtio.c_lflag =0;// ICANON;
newtio.c_cc[VINTR] = 0; /* Ctrl-c */
newtio.c_cc[VQUIT] = 0; /* Ctrl-\ */
newtio.c_cc[VERASE] = 0; /* del */
newtio.c_cc[VKILL] = 0; /* @ */
newtio.c_cc[VEOF] = 4; /* Ctrl-d */
newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
newtio.c_cc[VMIN] = 1; /* blocking read until 1 character arrives */
newtio.c_cc[VSWTC] = 0; /* '\0' */
newtio.c_cc[VSTART] = 0; /* Ctrl-q */
newtio.c_cc[VSTOP] = 0; /* Ctrl-s */
newtio.c_cc[VSUSP] = 0; /* Ctrl-z */
newtio.c_cc[VEOL] = 0; /* '\0' */
newtio.c_cc[VREPRINT] = 0; /* Ctrl-r */
newtio.c_cc[VDISCARD] = 0; /* Ctrl-u */
newtio.c_cc[VWERASE] = 0; /* Ctrl-w */
newtio.c_cc[VLNEXT] = 0; /* Ctrl-v */
newtio.c_cc[VEOL2] = 0; /* '\0' */

fcntl(fd, F_SETFL, FNDELAY);
tcflush(fd, TCIFLUSH);
tcsetattr(fd,TCSADRAIN,&newtio);

while (exit){
res=read(fd,buf,MAXDATASIZE);
while (res ==-1 ){
	//sleep(10);
 	res=read(fd,buf,MAXDATASIZE);
    	}
temp = QString(buf);
//qDebug ("serial: "+temp+"\n");
// LF =    = 10
// CR = \r = 13

if (temp.find(QString("\r"),0,1)==-1) result+= temp;
else 	{
	result+= temp;
	exit=0;
	}
}
::close(fd);
return result;
}
#endif // 0





QString pdf417 (const QDomElement &e, int countBar) {

QString result;
		result += QString("{XB%1;").arg( convZeros(countBar, 2) ); // count
	        result += QString("%1").arg( posConv(e.attribute( "x_mm"), 4 ) ); // X position
	        result += QString(",%1").arg( posConv(e.attribute( "y_mm"), 4 ) ); // Y position
	        result += QString(",P"); // Type of bar code ALWAYS PDF417
	        result += QString(",%1").arg( withChecksum(e) ); // Type of check digit
	        result += QString(",%1").arg( "03" ); // Module Width
	        result += QString(",%1").arg( convZeros(e.attribute("pdf417.col").toInt(),2 )); // Number of columns
	        result += QString(",%1").arg( rotation(e.attribute( "rotation", "0" ).toInt())); // Rotation angel of barcode
	        result += QString(",%1").arg( "0063" ); // height of barcode
	        result += QString("|}\n"); // FIN

return result;
}

QString getData(const QString &etext, const QString &art, const QString &group, const QString &id){
    TokenProvider token( NULL );
    token.setArticleNo( art );
    token.setGroup( group );
    token.setCustomerNo( id );
    token.setGroup( group );
    // TODO:
    // commented out, because the code is not so clean and only
    // used for TEC, maybe we can make this generic for all components of KBarcode
//    token.setSerialWatch( serialWatch() );
    token.setIndex( 0 ); // should be increased for every label

    return token.parse( etext );
/*

QString temp;


if (etext.contains("[serial_watch]") ) etext.replace(  "[serial_watch]" , serialWatch() );
if (etext.contains("[customer_no]") ) etext.replace(  "[customer_no]" , id );
if (etext.contains("[customer_name]") ) etext.replace(  "[customer_name]" , name );
if (etext.contains("[group]") ) etext.replace(  "[group]" , group );
if (etext.contains("[article_no]") ) etext.replace(  "[article_no]" , art );
if (etext.contains("[line1]") ) {
	QSqlQuery query("SELECT line1 FROM customer_text where customer_no='" + id + "'"  );
	while( query.next() ) temp = QString (query.value( 0 ).toString());
	etext.replace(  "[line1]" , temp );
	}
if (etext.contains("[line0]") ) {
	QSqlQuery query("SELECT line0 FROM customer_text where customer_no='" + id + "'"  );
	while( query.next() ) temp = QString (query.value( 0 ).toString());
	etext.replace(  "[line0]" , temp );
	}
if (etext.contains("[line2]") ) {
	QSqlQuery query("SELECT line2 FROM customer_text where customer_no='" + id + "'"  );
	while( query.next() ) temp = QString (query.value( 0 ).toString());
	etext.replace(  "[line2]" , temp );
	}
if (etext.contains("[line3]") ) {
	QSqlQuery query("SELECT line3 FROM customer_text where customer_no='" + id + "'"  );
	while( query.next() ) temp = QString (query.value( 0 ).toString());
	etext.replace(  "[line3]" , temp );
	}
if (etext.contains("[line4]") ) {
	QSqlQuery query("SELECT line4 FROM customer_text where customer_no='" + id + "'"  );
	while( query.next() ) temp = QString (query.value( 0 ).toString());
	etext.replace(  "[line4]" , temp );
	}
if (etext.contains("[line5]") ) {
	QSqlQuery query("SELECT line5 FROM customer_text where customer_no='" + id + "'"  );
	while( query.next() ) temp = QString (query.value( 0 ).toString());
	etext.replace(  "[line5]" , temp );
	}
if (etext.contains("[line6]") ) {
	QSqlQuery query("SELECT line6 FROM customer_text where customer_no='" + id + "'"  );
	while( query.next() ) temp = QString (query.value( 0 ).toString());
	etext.replace(  "[line6]" , temp );
	}
if (etext.contains("[line7]") ) {
	QSqlQuery query("SELECT line7 FROM customer_text where customer_no='" + id + "'"  );
	while( query.next() ) temp = QString (query.value( 0 ).toString());
	etext.replace(  "[line7]" , temp );
	}
if (etext.contains("[line8]") ) {
	QSqlQuery query("SELECT line8 FROM customer_text where customer_no='" + id + "'"  );
	while( query.next() ) temp = QString (query.value( 0 ).toString());
	etext.replace(  "[line8]" , temp );
	}
if (etext.contains("[line9]") ) {
	QSqlQuery query("SELECT line9 FROM customer_text where customer_no='" + id + "'"  );
	while( query.next() ) temp = QString (query.value( 0 ).toString());
	etext.replace(  "[line9]" , temp );
	}
if (etext.contains("[field0]") ) {
	QSqlQuery query("SELECT field0 FROM barcode_basic where article_no='" + art + "'"  );
	while( query.next() ) temp = QString (query.value( 0 ).toString());
	etext.replace(  "[field0]" , temp );
	}
if (etext.contains("[field1]") ) {
	QSqlQuery query("SELECT field1 FROM barcode_basic where article_no='" + art + "'"  );
	while( query.next() ) temp = QString (query.value( 0 ).toString());
	etext.replace(  "[field1]" , temp );
	}
if (etext.contains("[field2]") ) {
	QSqlQuery query("SELECT field2 FROM barcode_basic where article_no='" + art + "'"  );
	while( query.next() ) temp = QString (query.value( 0 ).toString());
	etext.replace(  "[field2]" , temp );
	}
if (etext.contains("[field3]") ) {
	QSqlQuery query("SELECT field3 FROM barcode_basic where article_no='" + art + "'"  );
	while( query.next() ) temp = QString (query.value( 0 ).toString());
	etext.replace(  "[field3]" , temp );
	}
if (etext.contains("[field4]") ) {
	QSqlQuery query("SELECT field4 FROM barcode_basic where article_no='" + art + "'"  );
	while( query.next() ) temp = QString (query.value( 0 ).toString());
	etext.replace(  "[field4]" , temp );
	}
if (etext.contains("[field5]") ) {
	QSqlQuery query("SELECT field5 FROM barcode_basic where article_no='" + art + "'"  );
	while( query.next() ) temp = QString (query.value( 0 ).toString());
	etext.replace(  "[field5]" , temp );
	}
if (etext.contains("[field6]") ) {
	QSqlQuery query("SELECT field6 FROM barcode_basic where article_no='" + art + "'"  );
	while( query.next() ) temp = QString (query.value( 0 ).toString());
	etext.replace(  "[field6]" , temp );
	}
if (etext.contains("[field7]") ) {
	QSqlQuery query("SELECT field7 FROM barcode_basic where article_no='" + art + "'"  );
	while( query.next() ) temp = QString (query.value( 0 ).toString());
	etext.replace(  "[field7]" , temp );
	}
if (etext.contains("[field8]") ) {
	QSqlQuery query("SELECT field8 FROM barcode_basic where article_no='" + art + "'"  );
	while( query.next() ) temp = QString (query.value( 0 ).toString());
	etext.replace(  "[field8]" , temp );
	}
if (etext.contains("[field9]") ) {
	QSqlQuery query("SELECT field9 FROM barcode_basic where article_no='" + art + "'"  );
	while( query.next() ) temp = QString (query.value( 0 ).toString());
	etext.replace(  "[field9]" , temp );
	}
if (etext.contains("[article_desc]") ) {
	QSqlQuery query("SELECT article_desc FROM barcode_basic where article_no='" + art + "'"  );
	while( query.next() ) temp = QString (query.value( 0 ).toString());
	etext.replace(  "[article_desc]" , temp );
	}
if (etext.contains("[barcode_no]") ) {
	QSqlQuery query("SELECT barcode_no FROM barcode_basic where article_no='" + art + "'"  );
	while( query.next() ) temp = QString (query.value( 0 ).toString());
	etext.replace(  "[barcode_no]" , temp );
	}
if (etext.contains("[field9]") ) {
	QSqlQuery query("SELECT field9 FROM barcode_basic where article_no='" + art + "'"  );
	while( query.next() ) temp = QString (query.value( 0 ).toString());
	etext.replace(  "[field9]" , temp );
	}
if (etext.contains("[article_no_customer]") ) {
	QSqlQuery query("SELECT article_no_customer FROM customer_text where customer_no='" + id + "'"  );
	while( query.next() ) temp = QString (query.value( 0 ).toString());
	etext.replace( "[article_no_customer]" , temp );
	}
if (etext.contains("[barcode_no_customer]") ){
	QSqlQuery query("SELECT barcode_no FROM customer_text where customer_no='" + id + "'"  );
	while( query.next() ) temp = QString (query.value( 0 ).toString());
	etext.replace("[barcode_no_customer]" , temp );
	}
if (etext.contains("[date]") ) etext.replace( "[date]" ,QDateTime::currentDateTime().toString( KBarcodeSettings::getDateFormat() ));
if (etext.contains("[time]") ) etext.replace( "[time]" ,QTime::currentTime().toString());

return etext;*/
}




QString lineWidth(const QDomElement &e){
QString result;
int width = e.attribute("line-width").toInt();
if (width > 0 and width < 10) return result.setNum(width);
else return "9"; //Max width supported
}

QString image (const QString &file){
int fd, res;
char buf[1];
QString result;

QByteArray baFile = file.toLocal8Bit();
char * charFile = baFile.data();

fd = open(charFile, O_RDONLY);
if (fd <0) return "Port Error.";
res=read(fd,buf,1);
while (res == 1){
result += QString(buf);
res=read(fd,buf,1);
}
return result;
}

QString getImage( const QString &id){
QString file;
QSqlQuery query("SELECT line9 FROM customer_text where customer_no='" + id + "'"  );
while( query.next() ) file = QString (query.value( 0 ).toString());
return image(file);
}
