/****************************************************************************
** Form interface generated from reading ui file 'SongInfoDialog.ui'
**
** Created: Thu Oct 4 00:12:48 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef SONGINFODIALOG_H
#define SONGINFODIALOG_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;

class SongInfoDialog : public QDialog
{ 
    Q_OBJECT

public:
    SongInfoDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~SongInfoDialog();

    QGroupBox* GroupBox1;
    QLabel* LabelArtist;
    QLineEdit* LineEditArtist;
    QLineEdit* LineEditTitle;
    QLineEdit* LineEditAlbum;
    QLabel* LabelAlbum;
    QLineEdit* LineEditComment;
    QLabel* LabelComment;
    QLineEdit* LineEditYear;
    QLineEdit* LineEditTrack;
    QLabel* LabelYear;
    QLabel* LabelTrack;
    QLabel* LabelTitle;
    QGroupBox* GroupBox3;
    QComboBox* ComboBoxMedia;
    QLabel* ReadOnlyFilename;
    QLabel* ReadOnlyPath;
    QLabel* LabelMedia;
    QLabel* ReadOnlyLength;
    QLabel* ReadOnlySize;
    QLabel* LabelPath;
    QLabel* LabelFilename;
    QLabel* LabelSize;
    QLabel* LabelLength;
    QGroupBox* GroupBox2;
    QLineEdit* LineEditAddedTo;
    QLabel* LabelAddedTo;
    QLabel* LabelLastPlayed;
    QLineEdit* LineEditLastPlayed;
    QLabel* LabelFrequency;
    QLineEdit* LineEditFrequency;
    QLabel* LabelHeading;
    QPushButton* PushButtonOK;
    QPushButton* PushButtonCancel;

protected:
    bool event( QEvent* );
};

#endif // SONGINFODIALOG_H
