#ifndef DEFINITIONDIALOG_H
#define DEFINITIONDIALOG_H

#include <qwidget.h>
#include <qdialog.h>
//Added by qt3to4:
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPaintEvent>
#include "measurements.h"

class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class KComboBox;
class KPushButton;
class KRestrictedLine;
class QLabel;
class QStringList;
class QRect;
class QPaintEvent;
/** Displays a preview of the given label measurements.
  * The current page size setting of the user is used.
  * The labels are drawn using QPainter. Only one page is previewed.
  */
class LabelPreview : public QWidget {
    Q_OBJECT
    public:
        LabelPreview( QWidget* parent = 0 );
        ~LabelPreview();

        void setRect( QRect label ) { size = label; }
        void setMeasurements( const Measurements & m ) { measure = m; }
        
        void setPrvEnabled( bool b ) { m_prv_enabled = b; }

    protected:
        void paintEvent( QPaintEvent* );
        QRect size;
        Measurements measure;
        
        bool m_prv_enabled;
};

/** A dialog which allows the user to create his/her own
  * label definitions easily.
  */
class DefinitionDialog : public QDialog
{ 
    Q_OBJECT

public:
    DefinitionDialog( QWidget* parent = 0, bool modal = true, Qt::WFlags fl = 0 );
    ~DefinitionDialog();
private:
    const Measurements getCurrentMeasure();

private slots:
    void add();
    void updateType();
    void updateText();
    void updatePreview();
    void toggleExtension();
    void drawGraphic();

protected:
    QLabel* l;
    QLabel* TextLabel1;
    QLabel* TextLabel2;
    QLabel* TextLabel3;
    QLabel* TextLabel4;
    QLabel* TextLabel5;
    QLabel* TextLabel6;
    QLabel* TextLabel7;
    QLabel* TextLabel8;
    QLabel* TextLabel9;
    QLabel* TextLabel10;
    KComboBox* comboProducer;
    KComboBox* comboType;
    KRestrictedLine* editWidth;
    KRestrictedLine* editHeight;
    KRestrictedLine* editHGap;
    KRestrictedLine* editVGap;
    KRestrictedLine* editTGap;
    KRestrictedLine* editLGap;
    KRestrictedLine* editNumH;
    KRestrictedLine* editNumV;
    KPushButton* buttonAdd;
    KPushButton* buttonCancel;
    KPushButton* buttonInfo;

    QStringList* types;
    LabelPreview* preview;

protected:
    QVBoxLayout* DefinitionDialogLayout;
    QHBoxLayout* Layout17;
    QVBoxLayout* Layout13;
    QVBoxLayout* Layout14;
    QHBoxLayout* Layout16;
};

#endif // DEFINITIONDIALOG_H
