
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
  }
  else {
    CheckBoxCorrectDirectories->setEnabled(false);
  }
  if(!CheckBoxCheckForExistence->isChecked() && !CheckBoxCheckTags->isChecked()
      && !CheckBoxCheckFilenames->isChecked() && !CheckBoxCheckDoubles->isChecked()) {
    PushButtonOk->setEnabled(false);
  }
  else {
    PushButtonOk->setEnabled(true);
  }

}
