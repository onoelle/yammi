/****************************************************************************
** Form implementation generated from reading ui file 'PreferencesDialogBase.ui'
**
** Created: Thu Oct 4 00:21:00 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "preferencesdialogbase.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a Preferences which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
Preferences::Preferences( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "Preferences" );
    resize( 568, 480 ); 
    setCaption( tr( "Edit Preferences" ) );

    ButtonOK = new QPushButton( this, "ButtonOK" );
    ButtonOK->setGeometry( QRect( 110, 440, 102, 26 ) ); 
    ButtonOK->setText( tr( "OK" ) );
    QToolTip::add(  ButtonOK, tr( "Accept & Save configuration" ) );

    ButtonCancel = new QPushButton( this, "ButtonCancel" );
    ButtonCancel->setGeometry( QRect( 400, 440, 102, 26 ) ); 
    ButtonCancel->setText( tr( "Cancel" ) );
    QToolTip::add(  ButtonCancel, tr( "don't make any changes" ) );

    TabWidget2 = new QTabWidget( this, "TabWidget2" );
    TabWidget2->setGeometry( QRect( 0, 0, 600, 430 ) ); 
    TabWidget2->setBackgroundOrigin( QTabWidget::WidgetOrigin );
    TabWidget2->setTabShape( QTabWidget::Rounded );
    TabWidget2->setMargin( 0 );

    tab = new QWidget( TabWidget2, "tab" );

    CheckBoxChildSafe = new QCheckBox( tab, "CheckBoxChildSafe" );
    CheckBoxChildSafe->setGeometry( QRect( 20, 250, 150, 20 ) ); 
    CheckBoxChildSafe->setText( tr( "child safe mode" ) );
    QToolTip::add(  CheckBoxChildSafe, tr( "cuts the playlist in xmms to no more than 4 unplayed songs" ) );

    CheckBoxLogging = new QCheckBox( tab, "CheckBoxLogging" );
    CheckBoxLogging->setGeometry( QRect( 20, 220, 150, 20 ) ); 
    CheckBoxLogging->setText( tr( "logging of played files" ) );
    QToolTip::add(  CheckBoxLogging, tr( "cuts the playlist in xmms to no more than 4 unplayed songs" ) );

    CheckBoxCutPlaylist = new QCheckBox( tab, "CheckBoxCutPlaylist" );
    CheckBoxCutPlaylist->setGeometry( QRect( 20, 190, 150, 20 ) ); 
    CheckBoxCutPlaylist->setText( tr( "cut xmms playlist" ) );
    QToolTip::add(  CheckBoxCutPlaylist, tr( "cuts the playlist in xmms to no more than 4 unplayed songs" ) );

    GroupBox2 = new QGroupBox( tab, "GroupBox2" );
    GroupBox2->setGeometry( QRect( 10, 20, 551, 81 ) ); 
    GroupBox2->setTitle( tr( "Path to files" ) );

    LineEditBaseDir = new QLineEdit( GroupBox2, "LineEditBaseDir" );
    LineEditBaseDir->setGeometry( QRect( 182, 15, 350, 22 ) ); 

    LabelBaseDir = new QLabel( GroupBox2, "LabelBaseDir" );
    LabelBaseDir->setGeometry( QRect( 12, 15, 160, 20 ) ); 
    LabelBaseDir->setText( tr( "base directory for mp3 files" ) );

    LabelScanDir = new QLabel( GroupBox2, "LabelScanDir" );
    LabelScanDir->setGeometry( QRect( 12, 45, 160, 20 ) ); 
    LabelScanDir->setText( tr( "base directory for scanning" ) );

    LineEditScanDir = new QLineEdit( GroupBox2, "LineEditScanDir" );
    LineEditScanDir->setGeometry( QRect( 182, 45, 350, 22 ) ); 

    ConsistencyMode = new QGroupBox( tab, "ConsistencyMode" );
    ConsistencyMode->setGeometry( QRect( 10, 290, 210, 70 ) ); 
    ConsistencyMode->setTitle( tr( "Consistency Mode" ) );

    CheckBoxTagsConsistent = new QCheckBox( ConsistencyMode, "CheckBoxTagsConsistent" );
    CheckBoxTagsConsistent->setGeometry( QRect( 20, 20, 180, 20 ) ); 
    CheckBoxTagsConsistent->setText( tr( "keep tags consistent" ) );
    QToolTip::add(  CheckBoxTagsConsistent, tr( "cuts the playlist in xmms to no more than 4 unplayed songs" ) );

    CheckBoxFilenamesConsistent = new QCheckBox( ConsistencyMode, "CheckBoxFilenamesConsistent" );
    CheckBoxFilenamesConsistent->setGeometry( QRect( 20, 40, 180, 20 ) ); 
    CheckBoxFilenamesConsistent->setText( tr( "keep filenames consistent" ) );
    QToolTip::add(  CheckBoxFilenamesConsistent, tr( "cuts the playlist in xmms to no more than 4 unplayed songs" ) );

    GroupBox3 = new QGroupBox( tab, "GroupBox3" );
    GroupBox3->setGeometry( QRect( 8, 105, 551, 81 ) ); 
    GroupBox3->setTitle( tr( "Mouse configuration" ) );

    LabelDoubleClickAction = new QLabel( GroupBox3, "LabelDoubleClickAction" );
    LabelDoubleClickAction->setGeometry( QRect( 10, 20, 180, 20 ) ); 
    LabelDoubleClickAction->setText( tr( "default action for doubleclick" ) );

    ComboBoxMiddleClickAction = new QComboBox( FALSE, GroupBox3, "ComboBoxMiddleClickAction" );
    ComboBoxMiddleClickAction->setGeometry( QRect( 200, 50, 190, 21 ) ); 

    LabelMiddleClickAction = new QLabel( GroupBox3, "LabelMiddleClickAction" );
    LabelMiddleClickAction->setGeometry( QRect( 10, 50, 180, 20 ) ); 
    LabelMiddleClickAction->setText( tr( "default action for click middle" ) );

    ComboBoxDoubleClickAction = new QComboBox( FALSE, GroupBox3, "ComboBoxDoubleClickAction" );
    ComboBoxDoubleClickAction->setGeometry( QRect( 200, 20, 190, 21 ) ); 
    TabWidget2->insertTab( tab, tr( "General Configuration" ) );

    tab_2 = new QWidget( TabWidget2, "tab_2" );

    LabelCriticalSize = new QLabel( tab_2, "LabelCriticalSize" );
    LabelCriticalSize->setGeometry( QRect( 20, 30, 110, 20 ) ); 
    LabelCriticalSize->setText( tr( "size of media" ) );
    QToolTip::add(  LabelCriticalSize, tr( "indicates maximum size for media (eg. 700 MB on a CD-R)" ) );

    LabelBurnSortOrder = new QLabel( tab_2, "LabelBurnSortOrder" );
    LabelBurnSortOrder->setEnabled( FALSE );
    LabelBurnSortOrder->setGeometry( QRect( 20, 80, 110, 20 ) ); 
    LabelBurnSortOrder->setText( tr( "sort order" ) );
    QToolTip::add(  LabelBurnSortOrder, tr( "indicates maximum size for media (eg. 700 MB on a CD-R)" ) );

    ComboBoxBurnSortOrder = new QComboBox( FALSE, tab_2, "ComboBoxBurnSortOrder" );
    ComboBoxBurnSortOrder->setEnabled( FALSE );
    ComboBoxBurnSortOrder->setGeometry( QRect( 158, 80, 110, 22 ) ); 

    LineEditCriticalSize = new QLineEdit( tab_2, "LineEditCriticalSize" );
    LineEditCriticalSize->setGeometry( QRect( 160, 30, 110, 22 ) ); 

    CheckBoxCreateDirectoryStructure = new QCheckBox( tab_2, "CheckBoxCreateDirectoryStructure" );
    CheckBoxCreateDirectoryStructure->setEnabled( FALSE );
    CheckBoxCreateDirectoryStructure->setGeometry( QRect( 20, 120, 200, 20 ) ); 
    CheckBoxCreateDirectoryStructure->setText( tr( "create directory structure?" ) );
    TabWidget2->insertTab( tab_2, tr( "Burning folder" ) );

    // signals and slots connections
    connect( ButtonOK, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( ButtonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
Preferences::~Preferences()
{
    // no need to delete child widgets, Qt does it all for us
}

