/****************************************************************************
** Form implementation generated from reading ui file 'SongInfoDialog.ui'
**
** Created: Thu Oct 4 00:12:57 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "songinfodialog.h"

#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a SongInfoDialog which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
SongInfoDialog::SongInfoDialog( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "SongInfoDialog" );
    resize( 524, 529 ); 
    setCaption( tr( "show and edit song info" ) );

    GroupBox1 = new QGroupBox( this, "GroupBox1" );
    GroupBox1->setGeometry( QRect( 10, 60, 520, 170 ) ); 
    GroupBox1->setTitle( tr( "id3 tag info (v1.1)" ) );

    LabelArtist = new QLabel( GroupBox1, "LabelArtist" );
    LabelArtist->setGeometry( QRect( 20, 20, 80, 20 ) ); 
    LabelArtist->setText( tr( "Aritst" ) );

    LineEditArtist = new QLineEdit( GroupBox1, "LineEditArtist" );
    LineEditArtist->setGeometry( QRect( 110, 20, 400, 22 ) ); 

    LineEditTitle = new QLineEdit( GroupBox1, "LineEditTitle" );
    LineEditTitle->setGeometry( QRect( 110, 50, 400, 22 ) ); 

    LineEditAlbum = new QLineEdit( GroupBox1, "LineEditAlbum" );
    LineEditAlbum->setGeometry( QRect( 111, 80, 400, 22 ) ); 

    LabelAlbum = new QLabel( GroupBox1, "LabelAlbum" );
    LabelAlbum->setGeometry( QRect( 20, 80, 80, 20 ) ); 
    LabelAlbum->setText( tr( "Album" ) );

    LineEditComment = new QLineEdit( GroupBox1, "LineEditComment" );
    LineEditComment->setGeometry( QRect( 110, 110, 400, 22 ) ); 

    LabelComment = new QLabel( GroupBox1, "LabelComment" );
    LabelComment->setGeometry( QRect( 19, 110, 80, 20 ) ); 
    LabelComment->setText( tr( "Comment" ) );

    LineEditYear = new QLineEdit( GroupBox1, "LineEditYear" );
    LineEditYear->setGeometry( QRect( 110, 140, 60, 22 ) ); 

    LineEditTrack = new QLineEdit( GroupBox1, "LineEditTrack" );
    LineEditTrack->setGeometry( QRect( 260, 140, 30, 22 ) ); 

    LabelYear = new QLabel( GroupBox1, "LabelYear" );
    LabelYear->setGeometry( QRect( 20, 140, 30, 20 ) ); 
    LabelYear->setText( tr( "Year" ) );

    LabelTrack = new QLabel( GroupBox1, "LabelTrack" );
    LabelTrack->setGeometry( QRect( 210, 140, 40, 20 ) ); 
    LabelTrack->setText( tr( "Track" ) );

    LabelTitle = new QLabel( GroupBox1, "LabelTitle" );
    LabelTitle->setGeometry( QRect( 19, 50, 80, 20 ) ); 
    LabelTitle->setText( tr( "Title" ) );

    GroupBox3 = new QGroupBox( this, "GroupBox3" );
    GroupBox3->setGeometry( QRect( 10, 230, 520, 140 ) ); 
    GroupBox3->setTitle( tr( "read only" ) );

    ComboBoxMedia = new QComboBox( FALSE, GroupBox3, "ComboBoxMedia" );
    ComboBoxMedia->setGeometry( QRect( 300, 20, 211, 20 ) ); 

    ReadOnlyFilename = new QLabel( GroupBox3, "ReadOnlyFilename" );
    ReadOnlyFilename->setGeometry( QRect( 110, 110, 400, 21 ) ); 
    ReadOnlyFilename->setText( tr( "unknown" ) );

    ReadOnlyPath = new QLabel( GroupBox3, "ReadOnlyPath" );
    ReadOnlyPath->setGeometry( QRect( 110, 80, 400, 21 ) ); 
    ReadOnlyPath->setText( tr( "unknown" ) );

    LabelMedia = new QLabel( GroupBox3, "LabelMedia" );
    LabelMedia->setGeometry( QRect( 250, 20, 40, 20 ) ); 
    LabelMedia->setText( tr( "Media" ) );

    ReadOnlyLength = new QLabel( GroupBox3, "ReadOnlyLength" );
    ReadOnlyLength->setGeometry( QRect( 110, 20, 130, 21 ) ); 
    ReadOnlyLength->setText( tr( "unknown" ) );

    ReadOnlySize = new QLabel( GroupBox3, "ReadOnlySize" );
    ReadOnlySize->setGeometry( QRect( 110, 50, 400, 20 ) ); 
    ReadOnlySize->setText( tr( "unknown" ) );

    LabelPath = new QLabel( GroupBox3, "LabelPath" );
    LabelPath->setGeometry( QRect( 20, 80, 80, 20 ) ); 
    LabelPath->setText( tr( "Path" ) );

    LabelFilename = new QLabel( GroupBox3, "LabelFilename" );
    LabelFilename->setGeometry( QRect( 20, 110, 80, 20 ) ); 
    LabelFilename->setText( tr( "Filename" ) );

    LabelSize = new QLabel( GroupBox3, "LabelSize" );
    LabelSize->setGeometry( QRect( 20, 50, 80, 20 ) ); 
    LabelSize->setText( tr( "Size" ) );

    LabelLength = new QLabel( GroupBox3, "LabelLength" );
    LabelLength->setGeometry( QRect( 20, 20, 80, 20 ) ); 
    LabelLength->setText( tr( "Length" ) );

    GroupBox2 = new QGroupBox( this, "GroupBox2" );
    GroupBox2->setGeometry( QRect( 10, 370, 520, 90 ) ); 
    GroupBox2->setTitle( tr( "database info" ) );

    LineEditAddedTo = new QLineEdit( GroupBox2, "LineEditAddedTo" );
    LineEditAddedTo->setGeometry( QRect( 140, 20, 100, 22 ) ); 
    QToolTip::add(  LineEditAddedTo, tr( "Indicates the date when this song was added to the database" ) );

    LabelAddedTo = new QLabel( GroupBox2, "LabelAddedTo" );
    LabelAddedTo->setGeometry( QRect( 20, 20, 110, 20 ) ); 
    LabelAddedTo->setText( tr( "Added to database" ) );

    LabelLastPlayed = new QLabel( GroupBox2, "LabelLastPlayed" );
    LabelLastPlayed->setEnabled( FALSE );
    LabelLastPlayed->setGeometry( QRect( 290, 20, 100, 20 ) ); 
    LabelLastPlayed->setText( tr( "Last time played" ) );

    LineEditLastPlayed = new QLineEdit( GroupBox2, "LineEditLastPlayed" );
    LineEditLastPlayed->setEnabled( FALSE );
    LineEditLastPlayed->setGeometry( QRect( 400, 20, 100, 22 ) ); 
    QToolTip::add(  LineEditLastPlayed, tr( "not implemented yet..." ) );

    LabelFrequency = new QLabel( GroupBox2, "LabelFrequency" );
    LabelFrequency->setEnabled( FALSE );
    LabelFrequency->setGeometry( QRect( 290, 50, 100, 20 ) ); 
    LabelFrequency->setText( tr( "Play Frequency" ) );

    LineEditFrequency = new QLineEdit( GroupBox2, "LineEditFrequency" );
    LineEditFrequency->setEnabled( FALSE );
    LineEditFrequency->setGeometry( QRect( 400, 50, 100, 22 ) ); 
    QToolTip::add(  LineEditFrequency, tr( "not implemented yet..." ) );

    LabelHeading = new QLabel( this, "LabelHeading" );
    LabelHeading->setGeometry( QRect( 30, 10, 470, 31 ) ); 
    QFont LabelHeading_font(  LabelHeading->font() );
    LabelHeading_font.setPointSize( 15 );
    LabelHeading_font.setBold( TRUE );
    LabelHeading_font.setItalic( TRUE );
    LabelHeading->setFont( LabelHeading_font ); 
    LabelHeading->setText( tr( "song info..." ) );

    PushButtonOK = new QPushButton( this, "PushButtonOK" );
    PushButtonOK->setGeometry( QRect( 150, 490, 102, 26 ) ); 
    PushButtonOK->setText( tr( "OK" ) );

    PushButtonCancel = new QPushButton( this, "PushButtonCancel" );
    PushButtonCancel->setGeometry( QRect( 300, 490, 102, 26 ) ); 
    PushButtonCancel->setText( tr( "Cancel" ) );

    // signals and slots connections
    connect( PushButtonOK, SIGNAL( clicked() ), PushButtonOK, SLOT( toggle() ) );
    connect( PushButtonOK, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( PushButtonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
SongInfoDialog::~SongInfoDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

/*  
 *  Main event handler. Reimplemented to handle application
 *  font changes
 */
bool SongInfoDialog::event( QEvent* ev )
{
    bool ret = QDialog::event( ev ); 
    if ( ev->type() == QEvent::ApplicationFontChange ) {
	QFont LabelHeading_font(  LabelHeading->font() );
	LabelHeading_font.setPointSize( 15 );
	LabelHeading_font.setBold( TRUE );
	LabelHeading_font.setItalic( TRUE );
	LabelHeading->setFont( LabelHeading_font ); 
    }
    return ret;
}

