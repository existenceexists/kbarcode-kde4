/***************************************************************************
                          measurements.h  -  description
                             -------------------
    begin                : Mit Dec 24 2003
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

#ifndef MEASUREMENTS_H
#define MEASUREMENTS_H

#include <klocale.h>

class QPaintDevice;

/**
  * This class keeps all measurements required to correctly draw a label.
  * The measurements are available in pixels, milimeter and inch.
  * This class allows also for transformations from inch to mm or pixels
  * (and the other way round). Internally, everything is handled in mm.
  *
  * @author Dominik Seichter
  */
class Measurements {
    public:
        enum {
            DpiX,
            DpiY
        };

        enum {
            Metric = KLocale::Metric,
            Imperial = KLocale::Imperial,
            None
        };

        Measurements();
        Measurements( const Measurements & rhs );
        ~Measurements();

        int numH() const { return num_h; }
        int numV() const { return num_v; }

        double gapLeftMM() const { return gap_left; }
        double gapTopMM() const { return gap_top; }
        double gapVMM() const { return gap_v; }
        double gapHMM() const { return gap_h; }
        double widthMM() const { return m_width; }
        double heightMM() const { return m_height; }

        // return inch or milimeters according to the
        // users preferrences
        double gapLeft() const;
        double gapTop() const;
        double gapV() const;
        double gapH() const;
        double width() const;
        double height() const;

        double gapLeft( const QPaintDevice* device ) const;
        double gapTop( const QPaintDevice* device ) const;
        double gapV( const QPaintDevice* device ) const;
        double gapH( const QPaintDevice* device ) const;
        double width( const QPaintDevice* device ) const;
        double height( const QPaintDevice* device ) const;

        void setNumH( int n ) { num_h = n; }
        void setNumV( int n ) { num_v = n; }

        // use milimeters for all of the setter methods
        void setGapLeftMM( double d ) { gap_left = d; }
        void setGapTopMM( double d ) { gap_top = d; }
        void setGapVMM( double d ) { gap_v = d; }
        void setGapHMM( double d ) { gap_h = d; }
        void setWidthMM( double d ) { m_width = d; }
        void setHeightMM( double d ) { m_height = d; }

        // inch or milimeters are taken as input
        // according to the users preferrences
        void setGapLeft( double d );
        void setGapTop( double d );
        void setGapV( double d );
        void setGapH( double d );
        void setWidth( double d );
        void setHeight( double d );

        void operator=(const Measurements & rhs );

        /** return the localized string that should be appended
          * to a measurement number visible to the user. I.e.
          * "mm" or "in".
          */
        static const QString & system() { Measurements::init(); return m_string; }

        /** return the measurements system to be used.
          */
        static int measurementSystem() { Measurements::init(); return m_system; }

    private:
        /** initialize measurements with the correctly
          * measurements system to be used from KDE.
          */
        static void init();

        void defaultMeasurements();

        double mmToPixel( double mm, const QPaintDevice* device, int mode = DpiX ) const;

        /** Measurement system to use:
          * milimeters or inch
          */
        static int m_system;
        static QString m_string;

        int num_h;
        int num_v;
        double gap_left;
        double gap_top;
        double gap_v;
        double gap_h;
        double m_width;
        double m_height;
};

#endif
