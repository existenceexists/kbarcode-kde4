/***************************************************************************
                          smalldialogs.cpp  -  description
                             -------------------
    begin                : Son Jul 20 2003
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

#include "smalldialogs.h"
#include "sqltables.h"

// Qt includes
#include <qlabel.h>
#include <qlayout.h>
#include <q3sqlcursor.h>
//Added by qt3to4:
#include <QSqlQuery>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>

// KDE includes
#include <knuminput.h>
#include <klineedit.h>
#include <klocale.h>


using namespace DSSmallDialogs;

AddAllDialog::AddAllDialog(QWidget *parent)
    : KDialogBase( KDialogBase::Plain, i18n("Add Barcode_basic"),
      KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, parent)
{
    QVBoxLayout* layout = new QVBoxLayout( plainPage(), 6, 6 );

    group = new KLineEdit( plainPage() );
    number = new KIntNumInput( plainPage() );
    number->setLabel( i18n( "Number of labels:" ) );
    number->setRange( 1, 10000, 1, false );

    layout->addWidget( new QLabel( i18n("Group:"), plainPage() ) );
    layout->addWidget( group );
    layout->addWidget( number );
}

QString AddAllDialog::groupName() const
{
    return group->text();
}

int AddAllDialog::numberLabels() const
{
    return number->value();
}

AddItemsDialog::AddItemsDialog(QWidget *parent)
    : KDialogBase( KDialogBase::Plain, i18n("Add Items"),
      KDialogBase::User1 | KDialogBase::Close, KDialogBase::User1, parent)
{
    init();
}

AddItemsDialog::AddItemsDialog( const QString & a, const QString & g, int c, QWidget* parent)
    : KDialogBase( KDialogBase::Plain, i18n("Edit Item"),
      KDialogBase::Ok| KDialogBase::Close, KDialogBase::Ok, parent)
{
    init();
    article->setText( a );
    group->setText( g );
    number->setValue( c );
}

void AddItemsDialog::init()
{
    plainPage()->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
    plainPage()->setLineWidth( 2 );

    QHBoxLayout* layout = new QHBoxLayout( plainPage(), 6, 6 );

    group = new KLineEdit( plainPage() );
    article = new KLineEdit( plainPage() );

    number = new KIntNumInput( plainPage() );
    number->setLabel( i18n( "Number of labels:" ), Qt::AlignLeft | Qt::AlignVCenter );
    number->setRange( 1, 10000, 1, false );

    layout->addWidget( number );
    layout->addWidget( new QLabel( i18n("Article:" ), plainPage() ) );
    layout->addWidget( article );
    layout->addWidget( new QLabel( i18n("Group:"), plainPage() ) );
    layout->addWidget( group );

    setButtonText( KDialogBase::User1, i18n("&Add") );

    setupSql();

    connect( SqlTables::getInstance(), SIGNAL( tablesChanged() ), this, SLOT( setupSql() ) );
    connect( SqlTables::getInstance(), SIGNAL( connectedSQL() ), this, SLOT( setupSql() ) );
}

void AddItemsDialog::slotUser1()
{
    emit add( article->text(), group->text(), number->value() );

    number->setValue( 1 );
    article->setText( "" );
    group->setText( "" );
    article->setFocus();
}

void AddItemsDialog::setupSql()
{
    SqlTables* tables = SqlTables::getInstance();
    if( !tables->isConnected() )
        return;

    KCompletion* comp = article->completionObject();
    comp->clear();
    QSqlQuery query( "select article_no from " TABLE_BASIC " order by article_no" );
    QStringList slist;
    while ( query.next() )
        slist.append( query.value(0).toString() );

    comp->setItems( slist );
}

void AddItemsDialog::setGroupCompletion( KCompletion* c )
{
    group->setCompletionObject( c );
}

int AddItemsDialog::count() const
{
    return number->value();
}

const QString AddItemsDialog::articleNo() const
{
    return article->text();
}

const QString AddItemsDialog::groupName() const
{
    return group->text();
}

#include "smalldialogs.moc"
