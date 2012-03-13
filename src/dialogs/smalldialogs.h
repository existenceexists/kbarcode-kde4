/***************************************************************************
                          smalldialogs.h  -  description
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

#ifndef SMALLDIALOGS_H
#define SMALLDIALOGS_H

#include <qwidget.h>
#include <kdialog.h>

class KCompletion;
class KIntNumInput;
class KLineEdit;

namespace DSSmallDialogs {

    class AddAllDialog : public KDialog {
        Q_OBJECT
        public:
            AddAllDialog(QWidget *parent=0);

            QString groupName() const;
            int numberLabels() const;

        protected:
            KLineEdit* group;
            KIntNumInput* number;
    };

    class AddItemsDialog : public KDialog {
        Q_OBJECT
        public:
            AddItemsDialog( QWidget* parent = 0 );
            AddItemsDialog( const QString & a, const QString & g, int c, QWidget* parent = 0, const char* name = 0 );

            void setGroupCompletion( KCompletion* c );

            int count() const;
            const QString articleNo() const;
            const QString groupName() const;
            
        signals:
            void add( const QString & article, const QString & group, int count);

        private slots:
            void setupSql();

        protected:
            KLineEdit* article;
            KLineEdit* group;
            KIntNumInput* number;

            void slotUser1();
            void init();
    };

}

#endif
