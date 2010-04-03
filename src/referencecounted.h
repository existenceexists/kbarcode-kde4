/***************************************************************************
                         referencecounted.h  -  description
                             -------------------
    begin                : Mo Apr 18 2005
    copyright            : (C) 2005 by Dominik Seichter
    email                : domseichter@web.de
 ***************************************************************************/

/***************************************************************************
                                                                          
    This program is free software; you can redistribute it and/or modify  
    it under the terms of the GNU General Public License as published by  
    the Free Software Foundation; either version 2 of the License, or     
    (at your option) any later version.                                   
                                                                          
 ***************************************************************************/

#ifndef REFERENCE_COUNTED_H
#define REFERENCE_COUNTED_H

/** 
 * A class to make objects reference counted.
 * Most likely you will want to subclass this class.
 * @see TCanvasItem for an example.
 * 
 * Whenever you get a reference to a ReferenceCounted
 * object, call addRef(). Whenever you do not need
 * the object anymore, call remRef().
 *
 * When the reference count reaches 0, the object
 * deletes itself.
 */


class ReferenceCounted {
    public:
        ReferenceCounted():m_counter(0) {}
        virtual ~ReferenceCounted() {}

        inline void addRef()
        {
            m_counter++;
        }


        inline void remRef()
        {
            if(--m_counter == 0) delete this;
        }


        inline unsigned int refCount()
        {
            return m_counter;
        }

    private:
	unsigned int m_counter;

};

#endif // REFERENCE_COUNTED_H


