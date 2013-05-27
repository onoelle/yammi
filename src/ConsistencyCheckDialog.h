#ifndef CONSISTENCYCHECKDIALOG_H
#define CONSISTENCYCHECKDIALOG_H

#include "ui_ConsistencyCheckDialogBase.h"

class ConsistencyCheckParameter;
class MyList;
class YammiModel;


class ConsistencyCheckDialog : public QDialog, public Ui::CheckConsistencyDialogBase {
    Q_OBJECT

public:
    ConsistencyCheckDialog( QWidget* parent, ConsistencyCheckParameter* para, MyList* selectedSongs, YammiModel* model);
    ~ConsistencyCheckDialog();

public slots:
    void myAccept();
    void changeSetting();
    void showReplacements();

private:
    void startCheck();
    ConsistencyCheckParameter* p;
    void setParameter();
    MyList* selectedSongs;
    YammiModel* model;
};



#endif // CONSISTENCYCHECKDIALOG_H
