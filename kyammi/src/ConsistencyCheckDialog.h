#ifndef CHECKCONSISTENCYDIALOG_H
#define CHECKCONSISTENCYDIALOG_H

#include <qwidget.h>
#include <ConsistencyCheckDialogBase.h>

class ConsistencyCheckParameter;

class CheckConsistencyDialog : public CheckConsistencyDialogBase
{ 
    Q_OBJECT

public:
	CheckConsistencyDialog( QWidget* parent, ConsistencyCheckParameter* para);
	~CheckConsistencyDialog();

	
public slots:
	void myAccept();
	void changeSetting();
	void showReplacements();
  
private:
	ConsistencyCheckParameter* p;
	void setParameter();
};



#endif // CHECKCONSISTENCYDIALOG_H
