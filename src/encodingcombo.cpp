/***************************************************************************
                          encodingcombo.h  -  description
                             -------------------
    begin                : Fri Dec 23 2005
    copyright            : (C) 2005 by Dominik Seichter
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

#include "encodingcombo.h"

#include <kcharsets.h>
#include <kglobal.h>
#include <klocale.h>
#include <qtextcodec.h>

EncodingCombo::EncodingCombo( QWidget* parent )
    : KComboBox( parent )
{
    QString sEncoding = QString( KGlobal::locale()->encoding() );
    QStringList encodings (KGlobal::charsets()->availableEncodingNames());

    this->insertStringList( encodings );

    for (int i=0; i < encodings.count(); i++)
    {
        bool found = false;
        QTextCodec *codecForEnc = KGlobal::charsets()->codecForName(encodings[i], found);
        if (found && ( (codecForEnc->name() == sEncoding) || (encodings[i] == sEncoding) ) )
        {
            this->setCurrentIndex(i);
            break;
        }
    }
}

EncodingCombo::~EncodingCombo()
{
}
