#include "ConsistencyCheckDialog.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qlabel.h>

#include <kapplication.h>
#include <qprogressdialog.h>
#include <ktextedit.h>

#include "ConsistencyCheckParameter.h"
#include "applytoalldialog.h"
#include "mylist.h"
#include "yammimodel.h"
#include "songentrystring.h"
#include "util.h"
#include "yammigui.h"

extern YammiGui* gYammiGui;

ConsistencyCheckDialog::ConsistencyCheckDialog(QWidget* parent, ConsistencyCheckParameter* para, MyList* selectedSongs, YammiModel* model)
: CheckConsistencyDialogBase(parent, tr("Check consistency - settings"), false) {
    p=para;
    this->model = model;
    this->selectedSongs = selectedSongs;
    setParameter();
    TextEditOutput->setReadOnly(true);
    TextLabelTitle->setText(QString(tr("Checking %1 songs")).arg(selectedSongs->count()));
    this->setModal(false);
    connect( CheckBoxCheckForExistence, SIGNAL( clicked() ), this, SLOT( changeSetting() ) );
    connect( CheckBoxCheckTags, SIGNAL( clicked() ), this, SLOT( changeSetting() ) );
    connect( CheckBoxCheckFilenames, SIGNAL( clicked() ), this, SLOT( changeSetting() ) );
    connect( CheckBoxCheckDirectories, SIGNAL( clicked() ), this, SLOT( changeSetting() ) );
    connect( CheckBoxCorrectDirectories, SIGNAL( clicked() ), this, SLOT( changeSetting() ) );
    connect( PushButtonOk, SIGNAL( clicked() ), this, SLOT( myAccept() ) );
    connect( PushButtonPatternReplacements, SIGNAL( clicked() ), this, SLOT( showReplacements() ) );
    changeSetting();
}


ConsistencyCheckDialog::~ConsistencyCheckDialog() {}

void ConsistencyCheckDialog::setParameter() {
    CheckBoxCheckForExistence->setChecked(p->checkForExistence);
    CheckBoxUpdateNonExisting->setChecked(p->updateNonExisting);
    CheckBoxCheckTags->setChecked(p->checkTags);
    CheckBoxIgnoreCase->setChecked(p->ignoreCaseInFilenames);
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

void ConsistencyCheckDialog::myAccept() {
    p->checkForExistence=CheckBoxCheckForExistence->isChecked();
    p->updateNonExisting=CheckBoxUpdateNonExisting->isChecked();
    p->checkTags=CheckBoxCheckTags->isChecked();
    p->ignoreCaseInFilenames=CheckBoxIgnoreCase->isChecked();
    p->correctTags=CheckBoxCorrectTags->isChecked();
    p->correctTagsDirection=ComboBoxCorrectTagsDirection->currentItem();
    p->checkFilenames=CheckBoxCheckFilenames->isChecked();
    p->correctFilenames=CheckBoxCorrectFilenames->isChecked();
    p->checkDirectories=CheckBoxCheckDirectories->isChecked();
    p->correctDirectories=CheckBoxCorrectDirectories->isChecked();
    p->deleteEmptyDirectories=CheckBoxDeleteEmptyDirectories->isChecked();
    p->checkDoubles=CheckBoxCheckDoubles->isChecked();
    QString directoryPattern = p->directoryPattern;
    QString filenamePattern = p->filenamePattern;
    p->directoryPattern=LineEditDirectoryPattern->text();
    p->filenamePattern=LineEditFilenamePattern->text();
    startCheck();
    // restore values that have impact on normal yammi operation
    // (too dangerous to leave them changed!)
    p->directoryPattern=directoryPattern;
    p->filenamePattern=filenamePattern;
}

void ConsistencyCheckDialog::startCheck() {
    p->resetStatistics();
    TextEditOutput->clear();

    QProgressDialog progress(this);
    progress.setLabelText(tr("Step 1: checking all songs in database..."));
    progress.setTotalSteps(selectedSongs->count());
    progress.setProgress(0);
    progress.setMinimumDuration(0);
    progress.setAutoReset(false);
    progress.setAutoClose(false);

    KTextEdit* output = TextEditOutput;
    output->append(tr("Checking consistency of %1 songs...").arg(selectedSongs->count()));
    model->problematicSongs.clear();

    // 1. iterate through all songs in database
    if(p->checkForExistence || p->checkTags || p->checkFilenames || p->checkDirectories) {
        int i=0;
        for(Song* s=selectedSongs->firstSong(); s; s=selectedSongs->nextSong(), i++) {
            kapp->processEvents();
            if(progress.wasCancelled()) {
                break;
            }
            output->append(QString(" - %1...\n").arg(s->displayName()));
            if(i % 10 == 0) {
                progress.setProgress(i);
            }
            QString diagnosis=s->checkConsistency(true, true, p->ignoreCaseInFilenames, true);
            if(diagnosis=="") {
                continue;
            }

            // okay, some kind of problem...

            if(diagnosis=="file not readable" && p->checkForExistence) {
                p->nonExisting++;
                output->append("! " + tr("file not existing or readable: %1\n").arg(s->displayName()));
                bool onMedia=s->mediaName.count()>0;
                if(p->updateNonExisting) {
                    // if we update, there are two cases:
                    if(onMedia) {
                        // 1. update entry: set filename+path to ""
                        s->filename="";
                        s->path="";
                        p->nonExistingUpdated++;
                        model->problematicSongs.append(new SongEntryString(s, tr("filename cleared in song entry")));
                        output->append("=> " + tr("filename cleared in song entry"));
                    } else {
                        // 2. delete entry in database
                        output->append(tr("=> deleting song entry %1\n").arg(s->displayName()));
                        gYammiGui->deleteEntry(s);
                        p->nonExistingDeleted++;
                    }
                } else {
                    model->problematicSongs.append(new SongEntryString(s, tr("file not existing or readable")));
                }
            }


            if(diagnosis.contains("tags not correct") && p->checkTags) {
                p->dirtyTags++;
                if(p->correctTags) {

                    bool reallyCorrect;
                    if(p->correctTagsConfirmed==-1) {
                        // warning dialog!
                        ApplyToAllDialog confirm(0);
                        QString msg=QString("Correct tags in file\n\n\t%1?\n\n").arg(s->filename);
                        if(p->correctTagsDirection==p->YAMMI2TAGS) {
                            msg+=QString("(Write yammi info to file tags:\n");
                            msg+=QString("artist: %1, title: %2\n").arg(s->artist).arg(s->title);
                            msg+=QString("album: %1, comment: %2\n").arg(s->album).arg(s->comment);
                            msg+=QString("year: %1, trackNr: %2, genre: %3)").arg(s->year).arg(s->trackNr).arg(s->genre);
                        }
                        if(p->correctTagsDirection==p->TAGS2YAMMI) {
                            msg+=QString("(Reread tags from filename and update Yammi info)");
                        }
                        confirm.TextLabel->setText(msg);
                        // show dialog
                        int result=confirm.exec();
                        if(result==42) {
                            progress.cancel();
                            break;
                        }
                        if(result==QDialog::Accepted) {
                            reallyCorrect=true;
                            if(confirm.CheckBoxApply->isChecked()) {
                                p->correctTagsConfirmed=1;
                            }
                        } else {
                            reallyCorrect=false;
                            if(confirm.CheckBoxApply->isChecked()) {
                                p->correctTagsConfirmed=0;
                            }
                        }
                    } else {
                        reallyCorrect=(p->correctTagsConfirmed==1);
                    }
                    if(reallyCorrect) {
                        if(p->correctTagsDirection==p->YAMMI2TAGS) {
                            if(s->saveTags()) {
                                p->tagsCorrected++;
                                model->problematicSongs.append(new SongEntryString(s, tr("Yammi info written to file tags")));
                                output->append("=> " + tr("Yammi info written to file tags"));
                            }
                        }
                        if(p->correctTagsDirection==p->TAGS2YAMMI) {
                            if(s->rereadTags()) {
                                p->tagsCorrected++;
                                model->problematicSongs.append(new SongEntryString(s, tr("Tags reread from file and Yammi info updated")));
                                output->append("=> " + tr("Tags reread from file and Yammi info updated"));
                            }
                        }
                    } else {
                        model->problematicSongs.append(new SongEntryString(s, "Yammi info and file tags not consistent"));
                        output->append("! " + tr("Yammi info and file tags not consistent"));
                    }
                } else {
                    model->problematicSongs.append(new SongEntryString(s, tr("Yammi info and file tags not consistent")));
                    output->append("! " + tr("Yammi info and file tags not consistent"));
                }
            }



            if(diagnosis.contains("filename not consistent") && p->checkFilenames) {
                p->dirtyFilenames++;
                output->append("! " + tr("Filename not consistent with Yammi info"));
                output->append("  " + tr("expected: %1").arg(s->constructFilename()));
                output->append("  " + tr("found: %1").arg(s->filename));
                if(p->correctFilenames) {
                    bool reallyCorrect;
                    if(p->correctFilenamesConfirmed==-1) {
                        // warning dialog!
                        ApplyToAllDialog confirm(0);
                        QString msg=QString("Correct filename from\n\t%1\n").arg(s->filename);
                        msg+=QString("to\n\t%1?").arg(s->constructFilename());
                        confirm.TextLabel->setText(msg);
                        // show dialog
                        int result=confirm.exec();
                        if(result==42) {
                            progress.cancel();
                            break;
                        }
                        if(result==QDialog::Accepted) {
                            reallyCorrect=true;
                            if(confirm.CheckBoxApply->isChecked()) {
                                p->correctFilenamesConfirmed=1;
                            }
                        } else {
                            reallyCorrect=false;
                            if(confirm.CheckBoxApply->isChecked()) {
                                p->correctFilenamesConfirmed=0;
                            }
                        }
                    } else {
                        reallyCorrect=(p->correctFilenamesConfirmed==1);
                    }
                    if(reallyCorrect) {
                        if(s->correctFilename()) {
                            p->filenamesCorrected++;
                            model->problematicSongs.append(new SongEntryString(s, tr("Filename corrected")));
                            output->append("=> " + tr("Filename corrected"));
                        }
                    } else {
                        model->problematicSongs.append(new SongEntryString(s, "Filename not consistent with Yammi info"));
                    }
                } else {
                    model->problematicSongs.append(new SongEntryString(s, tr("Filename not consistent with Yammi info")));
                }
            }

            if(diagnosis.contains("directory not consistent") && p->checkDirectories) {
                p->dirtyDirectories++;
                output->append("! " + tr("Directory not consistent with Yammi info"));
                output->append("  " + tr("expected: %1").arg(s->constructPath()));
                output->append("  " + tr("found: %1").arg(s->path));
                if(p->correctDirectories) {
                    bool reallyCorrect;
                    if(p->correctDirectoriesConfirmed==-1) {
                        // warning dialog!
                        ApplyToAllDialog confirm(0);
                        QString msg=QString("Correct path for file \n\t%1\n").arg(s->filename);
                        msg+=QString("from\n\t%1\n").arg(s->path);
                        msg+=QString("to\n\t%1?").arg(s->constructPath());
                        confirm.TextLabel->setText(msg);
                        // show dialog
                        int result=confirm.exec();
                        if(result==42) {
                            progress.cancel();
                            break;
                        }
                        if(result==QDialog::Accepted) {
                            reallyCorrect=true;
                            if(confirm.CheckBoxApply->isChecked()) {
                                p->correctDirectoriesConfirmed=1;
                            }
                        } else {
                            reallyCorrect=false;
                            if(confirm.CheckBoxApply->isChecked()) {
                                p->correctDirectoriesConfirmed=0;
                            }
                        }
                    } else {
                        reallyCorrect=(p->correctDirectoriesConfirmed==1);
                    }
                    if(reallyCorrect) {
                        QString pathBefore=s->path;
                        if(s->correctPath()) {
                            p->directoriesCorrected++;
                            model->problematicSongs.append(new SongEntryString(s, tr("Directory corrected")));
                            output->append("=> " + tr("Directory corrected"));
                            if(p->deleteEmptyDirectories) {
                                Util::deleteDirectoryIfEmpty(pathBefore, model->config()->scanDir);
                            }
                        }
                    } else {
                        model->problematicSongs.append(new SongEntryString(s, tr("Directory not consistent with Yammi info")));
                    }
                } else {
                    model->problematicSongs.append(new SongEntryString(s, tr("Directory not consistent with Yammi info")));
                }
            }

        }
    }


    // 2. check for songs contained twice
    if(!progress.wasCancelled() && p->checkDoubles) {
        progress.setLabelText(tr("Step 2: check for song entries pointing to same file"));
        progress.setTotalSteps(model->allSongs.count());
        progress.setProgress(0);
        kapp->processEvents();

        model->allSongs.setSortOrderAndSort(MyList::ByFilename + 16*(MyList::ByPath));
        Song* last=model->allSongs.firstSong();
        int i=0;
        for(Song* s=model->allSongs.nextSong(); s; s=model->allSongs.nextSong(), i++) {
            if(progress.wasCancelled()) {
                break;
            }
            if(i % 20 == 0) {
                progress.setProgress(i);
            }

            if(s->artist=="{wish}") {        // ignore wishes
                continue;
            }
            if(s->path=="" && s->filename=="") {    // ignore songs not on local harddisk
                continue;
            }

            // check for songs contained twice in database (but pointing to same file+path)
            if(last->location()==s->location()) {
                output->append("! " + tr("two database entries pointing to same file: %1, deleting one").arg(s->filename));
                output->append(QString("  1.: %1").arg(last->displayName()));
                output->append(QString("  2.: %1").arg(s->displayName()));
                model->allSongs.remove();   // TODO: check: could this cause a problem, because we are iterating through this list???
                model->allSongsChanged(true);
                p->doublesFound++;
                continue;
            }
            last=s;
        }

        // 3. check for two songs with identical primary key
        progress.setLabelText(tr("Step 3: check for songs with identical primary keys"));
        progress.setTotalSteps(model->allSongs.count());
        progress.setProgress(0);
        kapp->processEvents();

        model->allSongs.setSortOrderAndSort(MyList::ByKey);
        last=model->allSongs.firstSong();
        i=0;
        for(Song* s=model->allSongs.nextSong(); s; s=model->allSongs.nextSong(), i++) {
            if(progress.wasCancelled()) {
                break;
            }
            if(i % 20 == 0) {
                progress.setProgress(i);
            }
            if(s->artist=="{wish}") {
                continue;
            }
            if(s->sameAs(last)) {
                output->append("! " + tr("2 songs with identical primary key\n"));
                output->append(QString("  1.: %1").arg(last->location()));
                output->append(QString("  2.: %1").arg(s->location()));
                model->problematicSongs.append(new SongEntryString(last, tr("contained twice(1)")));
                model->problematicSongs.append(new SongEntryString(s, tr("contained twice(2)")));
                p->doublesFound++;
            }
        }
    }
    // reset sortOrder
    model->allSongs.setSortOrderAndSort(MyList::ByKey);

    progress.close();
    if(!progress.wasCancelled()) {
        output->append(tr("Consistency check finished\n"));
    } else {
        output->append(tr("Consistency check was cancelled\n"));
    }
    if(model->problematicSongs.count()==0) {
        output->append("Your Yammi database is nice and clean!\n");
        return;
    } else {
        model->allSongsChanged(true);
        output->append("\n\n");
        output->append(tr("Result of consistency check: (%1 songs)\n").arg(selectedSongs->count()));
        output->append("---------------------");
        output->append(tr("%1 issues found, check folder \"Problematic Songs\" (won't be saved)\n\n").arg(model->problematicSongs.count()));
        output->append(tr("- %1 songs not existing\n").arg(p->nonExisting));
        if(p->nonExisting > 0) {
            output->append(tr("     %1 entries updated (filename cleared)\n").arg(p->nonExistingUpdated));
            output->append(tr("     %1 entries deleted (because not existing on any media)\n").arg(p->nonExistingDeleted));
        }
        output->append(tr("- %1 songs with inconsistent tags\n").arg(p->dirtyTags));
        if(p->dirtyTags> 0) {
            output->append(tr("     %1 tags corrected\n").arg(p->tagsCorrected));
        }
        output->append(tr("- %1 songs with inconsistent filename\n").arg(p->dirtyFilenames));
        if(p->dirtyFilenames> 0) {
            output->append(tr("     %1 filenames corrected\n").arg(p->filenamesCorrected));
        }
        output->append(tr("- %1 songs with inconsistent path\n").arg(p->dirtyDirectories));
        if(p->dirtyDirectories> 0) {
            output->append(tr("   %1 paths corrected\n").arg(p->directoriesCorrected));
        }
        output->append(tr("- %1 double entries found\n").arg(p->doublesFound));
        return;
    }
}

void ConsistencyCheckDialog::changeSetting() {
    if(CheckBoxCheckForExistence->isChecked()) {
        CheckBoxUpdateNonExisting->setEnabled(true);
    } else {
        CheckBoxUpdateNonExisting->setEnabled(false);
    }

    if(CheckBoxCheckTags->isChecked()) {
        CheckBoxCorrectTags->setEnabled(true);
    } else {
        CheckBoxCorrectTags->setEnabled(false);
    }

    if(CheckBoxCheckFilenames->isChecked()) {
        CheckBoxCorrectFilenames->setEnabled(true);
    } else {
        CheckBoxCorrectFilenames->setEnabled(false);
    }
    if(CheckBoxCheckDirectories->isChecked()) {
        CheckBoxCorrectDirectories->setEnabled(true);
        if(CheckBoxCorrectDirectories->isChecked()) {
            CheckBoxDeleteEmptyDirectories->setEnabled(true);
        } else {
            CheckBoxDeleteEmptyDirectories->setEnabled(false);
        }
    } else {
        CheckBoxCorrectDirectories->setEnabled(false);
        CheckBoxDeleteEmptyDirectories->setEnabled(false);
    }
    if(!CheckBoxCheckForExistence->isChecked() && !CheckBoxCheckTags->isChecked() && !CheckBoxCheckFilenames->isChecked() && !CheckBoxCheckDoubles->isChecked() && !CheckBoxCheckDirectories->isChecked()) {
        PushButtonOk->setEnabled(false);
    } else {
        PushButtonOk->setEnabled(true);
    }

}

void ConsistencyCheckDialog::showReplacements() {
    QString msg;
    msg+=tr("Replacements for filename/directory pattern:\n\n");
    msg+=Song::getReplacementsDescription();
    QMessageBox::information( this, "Yammi",msg);
}
