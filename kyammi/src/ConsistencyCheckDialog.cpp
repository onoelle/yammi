#include "ConsistencyCheckDialog.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include "ConsistencyCheckParameter.h"
#include <klocale.h>

CheckConsistencyDialog::CheckConsistencyDialog(QWidget* parent, ConsistencyCheckParameter* para)
  : CheckConsistencyDialogBase(parent, i18n("Check consistency - settings"), true)
{
	p=para;
	setParameter();
	connect( CheckBoxCheckForExistence, SIGNAL( clicked() ), this, SLOT( changeSetting() ) );
	connect( CheckBoxCheckTags, SIGNAL( clicked() ), this, SLOT( changeSetting() ) );
	connect( CheckBoxCheckFilenames, SIGNAL( clicked() ), this, SLOT( changeSetting() ) );
	connect( CheckBoxCheckDirectories, SIGNAL( clicked() ), this, SLOT( changeSetting() ) );
	connect( CheckBoxCorrectDirectories, SIGNAL( clicked() ), this, SLOT( changeSetting() ) );
	connect( PushButtonOk, SIGNAL( clicked() ), this, SLOT( myAccept() ) );	
	connect( PushButtonPatternReplacements, SIGNAL( clicked() ), this, SLOT( showReplacements() ) );	
	changeSetting();
}


CheckConsistencyDialog::~CheckConsistencyDialog()
{
}

void CheckConsistencyDialog::setParameter()
{
	CheckBoxCheckForExistence->setChecked(p->checkForExistence);
	CheckBoxUpdateNonExisting->setChecked(p->updateNonExisting);
	CheckBoxCheckTags->setChecked(p->checkTags);
	CheckBoxCorrectTags->setChecked(p->correctTags);
	CheckBoxCheckFilenames->setChecked(p->checkFilenames);
	CheckBoxCorrectFilenames->setChecked(p->correctFilenames);
	CheckBoxCheckDirectories->setChecked(p->checkDirectories);
	CheckBoxCorrectDirectories->setChecked(p->correctDirectories);
	CheckBoxDeleteEmptyDirectories->setChecked(p->deleteEmptyDirectories);
	CheckBoxCheckDoubles->setChecked(p->checkDoubles);
	LineEditDirectoryPattern->setText(p->directoryPattern);
	LineEditFilenamePattern->setText(p->filenamePattern);
}

void CheckConsistencyDialog::myAccept()
{
    p->checkForExistence=CheckBoxCheckForExistence->isChecked();
    p->updateNonExisting=CheckBoxUpdateNonExisting->isChecked();
    p->checkTags=CheckBoxCheckTags->isChecked();
    p->correctTags=CheckBoxCorrectTags->isChecked();
    p->correctTagsDirection=ComboBoxCorrectTagsDirection->currentItem();
    p->checkFilenames=CheckBoxCheckFilenames->isChecked();
// TODO: fix!
//    p->ignoreCaseInFilenames=m_config.ignoreCaseInFilenames;
    p->correctFilenames=CheckBoxCorrectFilenames->isChecked();
    p->checkDirectories=CheckBoxCheckDirectories->isChecked();
    p->correctDirectories=CheckBoxCorrectDirectories->isChecked();
    p->deleteEmptyDirectories=CheckBoxDeleteEmptyDirectories->isChecked();
    p->checkDoubles=CheckBoxCheckDoubles->isChecked();
	p->directoryPattern=LineEditDirectoryPattern->text();
	p->filenamePattern=LineEditFilenamePattern->text();
	CheckConsistencyDialogBase::accept();
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

void CheckConsistencyDialog::showReplacements()
{
  QString msg("");
  msg+="Replacements for filename/directory pattern:\n\n";
// TODO: dry this out
//  msg+=Song::singleReplacements;
  msg+="{filename} (without path)\n";
  msg+="{absoluteFilename} (including path)\n";
  msg+="{filenameWithoutSuffix} (without path, without suffix)\n";
  msg+="{suffix} (without leading dot)\n";
  msg+="{path} (without filename)\n";
  msg+="{artist}, {title}, {album}, {comment} (corresponding to the tags)\n";
  msg+="{bitrate} (in kbps)\n";
  msg+="{length} (length in format mm:ss)\n";
  msg+="{lengthInSeconds} (length in seconds)\n";
  msg+="{mediaList (list of media on which song is contained)\n";
  msg+="{trackNr} (Track number)\n";
  msg+="{trackNr2Digit} (as above, but padded with zero if necessary)\n";
  QMessageBox::information( this, "Yammi",msg);
}

