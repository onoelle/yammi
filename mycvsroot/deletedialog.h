/****************************************************************************
** Form interface generated from reading ui file 'DeleteDialog.ui'
**
** Created: Thu Oct 4 17:49:11 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef DELETEDIALOG_H
#define DELETEDIALOG_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;

class DeleteDialog : public QDialog
{ 
    Q_OBJECT

public:
    DeleteDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~DeleteDialog();

    QCheckBox* CheckBoxDeleteFile;
    QCheckBox* CheckBoxDeleteDbEntry;
    QPushButton* PushButtonOK;
    QPushButton* PushButtonCancel;
    QLabel* TextLabel1;
    QComboBox* ComboBoxOnMedia;

protected:
    bool event( QEvent* );
};

#endif // DELETEDIALOG_H
