#ifndef CHECKCONSISTENCYDIALOG_H
#define CHECKCONSISTENCYDIALOG_H

#include <qwidget.h>
#include <ConsistencyCheckDialogBase.h>

class CheckConsistencyDialog : public CheckConsistencyDialogBase
{ 
    Q_OBJECT

public:
    CheckConsistencyDialog( QWidget* parent = 0, const char* name = 0);
    ~CheckConsistencyDialog();

public slots:
  void changeSetting();
};



#endif // CHECKCONSISTENCYDIALOG_H
