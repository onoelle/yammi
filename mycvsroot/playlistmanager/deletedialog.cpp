/****************************************************************************
** Form implementation generated from reading ui file 'DeleteDialog.ui'
**
** Created: Thu Oct 4 17:49:48 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "deletedialog.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a DeleteDialog which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
DeleteDialog::DeleteDialog( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "DeleteDialog" );
    resize( 381, 203 ); 
    setCaption( tr( "DeleteDialog" ) );

    CheckBoxDeleteFile = new QCheckBox( this, "CheckBoxDeleteFile" );
    CheckBoxDeleteFile->setGeometry( QRect( 10, 100, 190, 20 ) ); 
    CheckBoxDeleteFile->setText( tr( "delete file" ) );

    CheckBoxDeleteDbEntry = new QCheckBox( this, "CheckBoxDeleteDbEntry" );
    CheckBoxDeleteDbEntry->setGeometry( QRect( 10, 70, 190, 20 ) ); 
    CheckBoxDeleteDbEntry->setText( tr( "delete database entry" ) );

    PushButtonOK = new QPushButton( this, "PushButtonOK" );
    PushButtonOK->setGeometry( QRect( 20, 150, 101, 31 ) ); 
    PushButtonOK->setText( tr( "OK" ) );

    PushButtonCancel = new QPushButton( this, "PushButtonCancel" );
    PushButtonCancel->setGeometry( QRect( 140, 150, 101, 31 ) ); 
    PushButtonCancel->setText( tr( "Cancel" ) );

    TextLabel1 = new QLabel( this, "TextLabel1" );
    TextLabel1->setGeometry( QRect( 70, 20, 171, 20 ) ); 
    QFont TextLabel1_font(  TextLabel1->font() );
    TextLabel1_font.setPointSize( 14 );
    TextLabel1_font.setBold( TRUE );
    TextLabel1->setFont( TextLabel1_font ); 
    TextLabel1->setText( tr( "delete a song" ) );

    ComboBoxOnMedia = new QComboBox( FALSE, this, "ComboBoxOnMedia" );
    ComboBoxOnMedia->setGeometry( QRect( 230, 100, 141, 20 ) ); 

    // signals and slots connections
    connect( PushButtonOK, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( PushButtonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
DeleteDialog::~DeleteDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

/*  
 *  Main event handler. Reimplemented to handle application
 *  font changes
 */
bool DeleteDialog::event( QEvent* ev )
{
    bool ret = QDialog::event( ev ); 
    if ( ev->type() == QEvent::ApplicationFontChange ) {
	QFont TextLabel1_font(  TextLabel1->font() );
	TextLabel1_font.setPointSize( 14 );
	TextLabel1_font.setBold( TRUE );
	TextLabel1->setFont( TextLabel1_font ); 
    }
    return ret;
}

