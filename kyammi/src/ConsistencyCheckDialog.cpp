
#include "ConsistencyCheckDialog.h"

#include <qcheckbox.h>
#include <qpushbutton.h>

CheckConsistencyDialog::CheckConsistencyDialog(QWidget *parent, const char *name)
  : CheckConsistencyDialogBase(parent, name, true)
{
	connect( CheckBoxCheckForExistence, SIGNAL( clicked() ), this, SLOT( changeSetting() ) );
	CheckBoxCheckForExistence->setChecked(true);
	connect( CheckBoxCheckTags, SIGNAL( clicked() ), this, SLOT( changeSetting() ) );
	CheckBoxCheckTags->setChecked(true);
	connect( CheckBoxCheckFilenames, SIGNAL( clicked() ), this, SLOT( changeSetting() ) );
	CheckBoxCheckFilenames->setChecked(true);
	connect( CheckBoxCheckDirectories, SIGNAL( clicked() ), this, SLOT( changeSetting() ) );
	CheckBoxCheckDirectories->setChecked(true);
	connect( CheckBoxCorrectDirectories, SIGNAL( clicked() ), this, SLOT( changeSetting() ) );
	CheckBoxCheckDoubles->setChecked(true);
	changeSetting();
}


CheckConsistencyDialog::~CheckConsistencyDialog()
{
}


void CheckConsistencyDialog::changeSetting()
{
  if(CheckBoxCheckForExistence->isChecked()) {
    CheckBoxUpdateNonExisting->setEnabled(true);
  }
  else {
    CheckBoxUpdateNonExisting->setEnabled(false);
  }

  if(CheckBoxCheckTags->isChecked()) {
    CheckBoxCorrectTags->setEnabled(true);
  }
  else {
    CheckBoxCorrectTags->setEnabled(false);
  }

  if(CheckBoxCheckFilenames->isChecked()) {
    CheckBoxCorrectFilenames->setEnabled(true);
  }
  else {
    CheckBoxCorrectFilenames->setEnabled(false);
  }
  if(CheckBoxCheckDirectories->isChecked()) {
    CheckBoxCorrectDirectories->setEnabled(true);
	if(CheckBoxCorrectDirectories->isChecked()) {
		CheckBoxDeleteEmptyDirectories->setEnabled(true);
	}
  	else {
    	CheckBoxDeleteEmptyDirectories->setEnabled(false);
	}	
  }
  else {
    CheckBoxCorrectDirectories->setEnabled(false);
    CheckBoxDeleteEmptyDirectories->setEnabled(false);
  }
  if(!CheckBoxCheckForExistence->isChecked() && !CheckBoxCheckTags->isChecked() && !CheckBoxCheckFilenames->isChecked() && !CheckBoxCheckDoubles->isChecked() && !CheckBoxCheckDirectories->isChecked()) {
    PushButtonOk->setEnabled(false);
  }
  else {
    PushButtonOk->setEnabled(true);
  }

}
