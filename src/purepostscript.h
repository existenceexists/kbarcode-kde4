/***************************************************************************
                          purepostscript.h  -  description
                             -------------------
    begin                : Mon Jan 2 2006
    copyright            : (C) 2006 by Dominik Seichter
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


#ifndef PURE_POSTSCRIPT_H
#define PURE_POSTSCRIPT_H

#include "pixmapbarcode.h"
#include "barkode.h"

class PurePostscriptOptions : public BarkodeEngineOptions {
    public:
        PurePostscriptOptions();

        const BarkodeEngineOptions& operator=( const BarkodeEngineOptions & ps );
        void defaults();

        void load( const QDomElement* tag );
        void save( QDomElement* tag );  

        inline bool checksum() const { return m_check; }
        inline void setChecksum( bool b ) { m_check = b; }

    private: 
        bool m_check;
};

class PurePostscriptBarcode : public PixmapBarcode {
 public:
    PurePostscriptBarcode();
    ~PurePostscriptBarcode();

    /** Initialize PurePostscriptBarcode and set the path to
     *  the postscript program file or to QString::null if it cannot be found
     */
    static void init(); 

    static void initInfo( TBarcodeInfoList* info );

    static bool hasPurePostscriptBarcode();

    inline EEngine engine() const;

    inline BarkodeEngineOptions* options();

 private:
    virtual bool createPostscript( char** postscript, long* postscript_size );
    virtual QRect bbox( const char* postscript, long postscript_size );

    /** Create a program which can be executed by ghostscript from
     *  m_program and the barcodes data.
     *  \param prg the program will be written to this string
     */
    void createProgram( QString & prg );

 private:
    PurePostscriptOptions m_options;

    QString m_program;

    static QString s_path;
    
    static bool s_oldBWIPP;
};

EEngine PurePostscriptBarcode::engine() const
{
    return PURE_POSTSCRIPT;
}

BarkodeEngineOptions* PurePostscriptBarcode::options()
{
    return &m_options;
}

#endif /* PURE_POSTSCRIPT_H */
