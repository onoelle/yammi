#include "ConsistencyCheckDialog.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qlabel.h>

#include <kapplication.h>
#include <kprogress.h>
#include <klocale.h>
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
: CheckConsistencyDialogBase(parent, i18n("Check consistency - settings"), false) {
    p=para;
    this->model = model;
    this->selectedSongs = selectedSongs;
    setParameter();
    TextEditOutput->setReadOnly(true);
    TextLabelTitle->setText(QString(i18n("Checking %1 songs")).arg(selectedSongs->count()));
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
    startCheck();
}

void ConsistencyCheckDialog::startCheck() {
    p->resetStatistics();
    TextEditOutput->clear();

    KProgressDialog progress( this, 0, i18n("Yammi"), i18n("Checking consistency..."), true);
    progress.setLabel(i18n("Step 1: checking all songs in database..."));
    progress.progressBar()->setTotalSteps(selectedSongs->count());
    progress.progressBar()->setProgress(0);
    progress.setMinimumDuration(0);
    progress.setAutoReset(false);
    progress.setAutoClose(false);
    progress.setAllowCancel(true);
    progress.showCancelButton(true);
    kapp->processEvents();

    KTextEdit* output = TextEditOutput;
    output->append(i18n("Checking consistency of %1 songs...").arg(selectedSongs->count()));
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
                progress.progressBar()->setProgress(i);
            }
            QString diagnosis=s->checkConsistency(true, true, p->ignoreCaseInFilenames, true);
            if(diagnosis=="") {
                continue;
            }

            // okay, some kind of problem...

            if(diagnosis=="file not readable" && p->checkForExistence) {
                p->nonExisting++;
                output->append("! " + i18n("file not existing or readable: %1\n").arg(s->displayName()));
                bool onMedia=s->mediaName.count()>0;
                if(p->updateNonExisting) {
                    // if we update, there are two cases:
                    if(onMedia) {
                        // 1. update entry: set filename+path to ""
                        s->filename="";
                        s->path="";
                        p->nonExistingUpdated++;
                        model->problematicSongs.append(new SongEntryString(s, i18n("filename cleared in song entry")));
                        output->append("=> " + i18n("filename cleared in song entry"));
                    } else {
                        // 2. delete entry in database
                        output->append(i18n("=> deleting song entry %1\n").arg(s->displayName()));
                        gYammiGui->deleteEntry(s);
                        p->nonExistingDeleted++;
                    }
                } else {
                    model->problematicSongs.append(new SongEntryString(s, i18n("file not existing or readable")));
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
                            msg+=QString("year: %1, trackNr: %2, genreNr: %3)").arg(s->year).arg(s->trackNr).arg(s->genreNr);
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
                                model->problematicSongs.append(new SongEntryString(s, i18n("Yammi info written to file tags")));
                                output->append("=> " + i18n("Yammi info written to file tags"));
                            }
                        }
                        if(p->correctTagsDirection==p->TAGS2YAMMI) {
                            if(s->rereadTags()) {
                                p->tagsCorrected++;
                                model->problematicSongs.append(new SongEntryString(s, i18n("Tags reread from file and Yammi info updated")));
                                output->append("=> " + i18n("Tags reread from file and Yammi info updated"));
                            }
                        }
                    } else {
                        model->problematicSongs.append(new SongEntryString(s, "Yammi info and file tags not consistent"));
                        output->append("! " + i18n("Yammi info and file tags not consistent"));
                    }
                } else {
                    model->problematicSongs.append(new SongEntryString(s, i18n("Yammi info and file tags not consistent")));
                    output->append("! " + i18n("Yammi info and file tags not consistent"));
                }
            }



            if(diagnosis.contains("filename not consistent") && p->checkFilenames) {
                p->dirtyFilenames++;
                output->append("! " + i18n("Filename not consistent with Yammi info"));
                output->append("  " + i18n("expected: %1").arg(s->constructFilename()));
                output->append("  " + i18n("found: %1").arg(s->filename));
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
                            model->problematicSongs.append(new SongEntryString(s, i18n("Filename corrected")));
                            output->append("=> " + i18n("Filename corrected"));
                        }
                    } else {
                        model->problematicSongs.append(new SongEntryString(s, "Filename not consistent with Yammi info"));
                    }
                } else {
                    model->problematicSongs.append(new SongEntryString(s, i18n("Filename not consistent with Yammi info")));
                }
            }

            if(diagnosis.contains("directory not consistent") && p->checkDirectories) {
                p->dirtyDirectories++;
                output->append("! " + i18n("Directory not consistent with Yammi info"));
                output->append("  " + i18n("expected: %1").arg(s->constructPath()));
                output->append("  " + i18n("found: %1").arg(s->path));
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
                            model->problematicSongs.append(new SongEntryString(s, i18n("Directory corrected")));
                            output->append("=> " + i18n("Directory corrected"));
                            if(p->deleteEmptyDirectories) {
                                Util::deleteDirectoryIfEmpty(pathBefore, model->config()->scanDir);
                            }
                        }
                    } else {
                        model->problematicSongs.append(new SongEntryString(s, i18n("Directory not consistent with Yammi info")));
                    }
                } else {
                    model->problematicSongs.append(new SongEntryString(s, i18n("Directory not consistent with Yammi info")));
                }
            }

        }
    }


    // 2. check for songs contained twice
    if(!progress.wasCancelled() && p->checkDoubles) {
        progress.setLabel(i18n("Step 2: check for song entries pointing to same file"));
        progress.progressBar()->setTotalSteps(model->allSongs.count());
        progress.progressBar()->setProgress(0);
        kapp->processEvents();

        model->allSongs.setSortOrderAndSort(MyList::ByFilename + 16*(MyList::ByPath));
        Song* last=model->allSongs.firstSong();
        int i=0;
        for(Song* s=model->allSongs.nextSong(); s; s=model->allSongs.nextSong(), i++) {
            if(progress.wasCancelled()) {
                break;
            }
            if(i % 20 == 0) {
                progress.progressBar()->setProgress(i);
            }

            if(s->artist=="{wish}") {        // ignore wishes
                continue;
            }
            if(s->path=="" && s->filename=="") {    // ignore songs not on local harddisk
                continue;
            }

            // check for songs contained twice in database (but pointing to same file+path)
            if(last->location()==s->location()) {
                output->append("! " + i18n("two database entries pointing to same file: %1, deleting one").arg(s->filename));
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
        progress.setLabel(i18n("Step 3: check for songs with identical primary keys"));
        progress.progressBar()->setTotalSteps(model->allSongs.count());
        progress.progressBar()->setProgress(0);
        kapp->processEvents();

        model->allSongs.setSortOrderAndSort(MyList::ByKey);
        last=model->allSongs.firstSong();
        i=0;
        for(Song* s=model->allSongs.nextSong(); s; s=model->allSongs.nextSong(), i++) {
            if(progress.wasCancelled()) {
                break;
            }
            if(i % 20 == 0) {
                progress.progressBar()->setProgress(i);
            }
            if(s->artist=="{wish}") {
                continue;
            }
            if(s->sameAs(last)) {
                output->append("! " + i18n("2 songs with identical primary key\n"));
                output->append(QString("  1.: %1").arg(last->location()));
                output->append(QString("  2.: %1").arg(s->location()));
                model->problematicSongs.append(new SongEntryString(last, i18n("contained twice(1)")));
                model->problematicSongs.append(new SongEntryString(s, i18n("contained twice(2)")));
                p->doublesFound++;
            }
        }
    }
    // reset sortOrder
    model->allSongs.setSortOrderAndSort(MyList::ByKey);

    progress.close();
    if(!progress.wasCancelled()) {
        output->append(i18n("Consistency check finished\n"));
    } else {
        output->append(i18n("Consistency check was cancelled\n"));
    }
    if(model->problematicSongs.count()==0) {
        output->append("Your Yammi database is nice and clean!\n");
        return;
    } else {
        model->allSongsChanged(true);
        output->append("\n\n");
        output->append(i18n("Result of consistency check: (%1 songs)\n").arg(selectedSongs->count()));
        output->append("---------------------");
        output->append(i18n("%1 issues found, check folder \"Problematic Songs\" (won't be saved)\n\n").arg(model->problematicSongs.count()));
        output->append(i18n("- %1 songs not existing\n").arg(p->nonExisting));
        if(p->nonExisting > 0) {
            output->append(i18n("     %1 entries updated (filename cleared)\n").arg(p->nonExistingUpdated));
            output->append(i18n("     %1 entries deleted (because not existing on any media)\n").arg(p->nonExistingDeleted));
        }
        output->append(i18n("- %1 songs with inconsistent tags\n").arg(p->dirtyTags));
        if(p->dirtyTags> 0) {
            output->append(i18n("     %1 tags corrected\n").arg(p->tagsCorrected));
        }
        output->append(i18n("- %1 songs with inconsistent filename\n").arg(p->dirtyFilenames));
        if(p->dirtyFilenames> 0) {
            output->append(i18n("     %1 filenames corrected\n").arg(p->filenamesCorrected));
        }
        output->append(i18n("- %1 songs with inconsistent path\n").arg(p->dirtyDirectories));
        if(p->dirtyDirectories> 0) {
            output->append(i18n("   %1 paths corrected\n").arg(p->directoriesCorrected));
        }
        output->append(i18n("- %1 double entries found\n").arg(p->doublesFound));
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
    msg+=i18n("Replacements for filename/directory pattern:\n\n");
    msg+=Song::getReplacementsDescription();
    QMessageBox::information( this, "Yammi",msg);
}

