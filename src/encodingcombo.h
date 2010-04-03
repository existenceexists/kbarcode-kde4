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

#ifndef _ENCODING_COMBO_H_
#define _ENCODING_COMBO_H_

#include <kcombobox.h>

class EncodingCombo : public KComboBox {
    Q_OBJECT
    public:
    EncodingCombo( QWidget* parent );
    ~EncodingCombo();




};

#endif //  _ENCODING_COMBO_H_

