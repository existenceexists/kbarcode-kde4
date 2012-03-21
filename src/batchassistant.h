/***************************************************************************
                          batchassistant.h  -  description
                             -------------------
    begin                : Sun Mar 20 2005
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

#ifndef BATCHASSISTANT_H
#define BATCHASSISTANT_H

#include <kassistantdialog.h>
#include <QLabel>

class BatchPrinter;
class EncodingCombo;
class KComboBox;
class KCompletion;
class KIntNumInput;
class KLineEdit;
class K3ListBox;
class QTreeWidget;
class KPushButton;
class KUrlRequester;
class QCheckBox;
class QWidget;
class QLabel;
class QTreeWidget;
class QTreeWidgetItem;
class QRadioButton;
class QTableWidget;
class QStackedWidget;
class KPageWidgetItem;

class BatchAssistant : public KAssistantDialog {
    Q_OBJECT
    public:
        BatchAssistant( QWidget* parent = NULL );
        ~BatchAssistant();

    	/** reimplemented DCOP method from BatchIface 
	 *  Allow the user to add an article to print to KBarcode
	 *  @p article article number to add
	 *  @p group   use group as group name for this article
	 *  @p count   number of labels to print for this article
	 *  @p msgbox  show a messagebox if adding the item fails
	 */
        bool addItem( const QString & article, const QString & group, int count, bool msgbox = true );

    	/** reimplemented DCOP method from BatchIface 
	 *  tests wether the arctile is existing in KBarcodes database
	 *  @p article article number to test for
	 */
	virtual bool existsArticle( const QString & article );

	/** reimplemented DCOP method from BatchIface 
	 *  import articles from a file. The user is asked for the filename
	 */
	virtual void loadFromFile( const QString & url );

	/** reimplemented DCOP method from BatchIface 
	 *  print immediately with current settings
	 *  @p printer  print to this printer (must be known to the system)
         *  @p bUserInteraction if false no dialog will pop up besides the printer dialog
	 */
	virtual void printNow( const QString & printer, bool bUserInteraction = true );

    	/** reimplemented DCOP method from BatchIface 
	 *  to allow the user to set the label to be used
	 *  @p url  path of the label which shall get printed
	 */
	virtual void setFilename( const QString & url );

	/** reimplemented DCOP method from BatchIface 
	 *  Set the batchassistants mode to print from imported data
	 *  and import the data from the csv file @p filname
	 *  @p filename csv file to import
	 */
	virtual void setImportCsvFile( const QString & filename );

	/** reimplemented DCOP method from BatchIface 
	 *  Set the batchassistants mode to print from imported data
	 *  and import the data from the sql query @p query
	 *  @p query sql query to execute to get on user defined variables
	 */
	virtual void setImportSqlQuery( const QString & query );

	/** reimplemented DCOP method from BatchIface 
	 *  set the numbers of labels to print.
	 *  the option to print without data import is activated too using
	 *  this option
	 *  @p n number of labels to print
	 */
	virtual void setNumLabels( const int n );

	/** reimplemented DCOP method from BatchIface 
	 *  sets wether the user wants to print to a postscript printer,
	 *  a barcode printer or to image files.
	 *  @p e is of datatype BatchPrinter::EOutputFormat
	 */
	virtual void setOutputFormat( const int e );

	/** reimplemented DCOP method from BatchIface 
	 *  allow the user to set the value and start value for
	 *  the serial number that can be used on labels using the [serial] token
	 *  @p val  value of the serial number (e.g. TEST0002)
	 *  @p inc  the serial number is increased that much for every label
	 */
	virtual void setSerialNumber( const QString & val, int inc );

	public slots:
	/** reimplemented DCOP method from BatchIface 
	 *  import articles from the clipboard
	 */
	virtual void loadFromClipboard();


    private slots:
	void addAllItems();
	void enableControls();
        void customerIdChanged( int index );
	void customerNameChanged( int index );
	void addItem();
        bool slotAddItem( const QString & article, const QString & group, int count );
	void changeItem( QTreeWidgetItem* item, int column );
        /*void changeItem( Q3ListViewItem* item, const QPoint &, int );*/// -!F: del
	void editItem();
	void removeItem();

	void slotTableInsert();
	void slotTableRemove();
        void loadFromFile();

        void slotAddAddress();
        void slotAddAllAddress();
        void slotRemoveAddress();
        void slotRemoveAllAddress();
        
        void next();
        void back();

    private:
        void moveAddress( QTreeWidget* src, QTreeWidget* dst, bool bAll = false );
        /*void moveAddress( Q3ListView* src, Q3ListView* dst, bool bAll = false );*/// -!F: delete

	void fillByteArray();
	void fillVarList();
	bool fillVarTable();
        void fillAddressList();

        void setupPage1();
        void setupPage2();
	void setupPage3();
	void setupPage4();
	void setupPage5();
	void setupPage10();

	void setupStackPage1();
	void setupStackPage2();
	void setupStackPage3();
	void setupStackPage4();

	void setupSql();
	void setupBatchPrinter( BatchPrinter* batch, int m );

	void addGroupCompletion( const QString & group );
	void loadData( const QString & data );

    protected:
	void accept();
	void configureCurrentPage( KPageWidgetItem* page );

    private:
	/** m_bytearray_filename is set whenever
	 *  the kbarcide label file has been read into 
	 *  the buffer m_bytearray. Therefore we have only
	 *  to re-read the bytearray when the filename was 
	 *  changed by the user.
	 */
	QString m_bytearray_filename;
	QByteArray m_bytearray;

	QWidget* page1;
	QWidget* page2;
	QStackedWidget* page3;
	QWidget* page4;
	QWidget* page5;
	QWidget* page10;
        
        KPageWidgetItem * page4Item;
        KPageWidgetItem * page3Item;

	QWidget* stack1;
	QWidget* stack2;
	QWidget* stack3;
        QWidget* stack4;

	QWidget* imageBox;

	QLabel* labelInfo;
	QLabel* labelSqlQuery;
	QLabel* labelCsvFile;
        QLabel* labelEncoding;

	QRadioButton* radioSqlArticles;
	QRadioButton* radioVarImport;
	QRadioButton* radioSimple;
	QRadioButton* radioAddressBook;

	QRadioButton* radioPrinter;
	QRadioButton* radioImage;
	QRadioButton* radioBarcode;

	QRadioButton* radioImportSql;
	QRadioButton* radioImportCSV;
	QRadioButton* radioImportManual;

	QRadioButton* radioImageFilenameArticle;
	QRadioButton* radioImageFilenameBarcode;
	QRadioButton* radioImageFilenameCustom;
	KLineEdit* editImageFilename;
	KUrlRequester* imageDirPath;

	KLineEdit* importSqlQuery;
	KUrlRequester* importCsvFile;

	KComboBox* customerName;
	KComboBox* customerId;
	KComboBox* comboFormat;
        EncodingCombo* comboEncoding;

	KCompletion* compGroup;

	KIntNumInput* serialInc;
	KIntNumInput* numLabels;

	/*QTableWidget* m_varTable;*/// -!F: original, uncomment
        QTableWidget* m_varTable;
	K3ListBox* m_varList;

        QTreeWidget* listAddress;
        QTreeWidget* listSelectedAddress;
        /*K3ListView* listAddress;
        K3ListView* listSelectedAddress;*/// -!F: delete

	QTreeWidget* sqlList;
	KLineEdit* serialStart;

        QCheckBox* checkKeepOpen;

        KPushButton* buttonAddAddress;
        KPushButton* buttonRemoveAddress;
        KPushButton* buttonAddAllAddress;
        KPushButton* buttonRemoveAllAddress;

        KPushButton* buttonAdd;
	KPushButton* buttonImport;
        KPushButton* buttonEdit;
        KPushButton* buttonRemove;
        KPushButton* buttonRemoveAll;

	KPushButton* buttonTableInsert;
	KPushButton* buttonTableRemove;

	KUrlRequester* m_url;
        
        bool page1NextButtonEnable;
        bool page3NextButtonEnable;
        bool page10FinishButtonEnable;
};

#endif // BATCHASSISTANT_H
