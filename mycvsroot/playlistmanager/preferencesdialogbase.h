/****************************************************************************
** Form interface generated from reading ui file 'PreferencesDialogBase.ui'
**
** Created: Thu Oct 4 00:20:57 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTabWidget;
class QWidget;

class Preferences : public QDialog
{ 
    Q_OBJECT

public:
    Preferences( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~Preferences();

    QPushButton* ButtonOK;
    QPushButton* ButtonCancel;
    QTabWidget* TabWidget2;
    QWidget* tab;
    QCheckBox* CheckBoxChildSafe;
    QCheckBox* CheckBoxLogging;
    QCheckBox* CheckBoxCutPlaylist;
    QGroupBox* GroupBox2;
    QLineEdit* LineEditBaseDir;
    QLabel* LabelBaseDir;
    QLabel* LabelScanDir;
    QLineEdit* LineEditScanDir;
    QGroupBox* ConsistencyMode;
    QCheckBox* CheckBoxTagsConsistent;
    QCheckBox* CheckBoxFilenamesConsistent;
    QGroupBox* GroupBox3;
    QLabel* LabelDoubleClickAction;
    QComboBox* ComboBoxMiddleClickAction;
    QLabel* LabelMiddleClickAction;
    QComboBox* ComboBoxDoubleClickAction;
    QWidget* tab_2;
    QLabel* LabelCriticalSize;
    QLabel* LabelBurnSortOrder;
    QComboBox* ComboBoxBurnSortOrder;
    QLineEdit* LineEditCriticalSize;
    QCheckBox* CheckBoxCreateDirectoryStructure;

};

#endif // PREFERENCES_H
