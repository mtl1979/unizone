/**********************************************************************
**   Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
**   trwindow.h
**
**   This file is part of Qt Linguist.
**
**   See the file LICENSE included in the distribution for the usage
**   and distribution terms.
**
**   The file is provided AS IS with NO WARRANTY OF ANY KIND,
**   INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR
**   A PARTICULAR PURPOSE.
**
**********************************************************************/

#ifndef TRWINDOW_H
#define TRWINDOW_H

#include "phrase.h"

#include <qmainwindow.h>
#include <qlist.h>
#include <qdict.h>
#include <qprinter.h>
#include <qstringlist.h>

#include <metatranslator.h>

class QDialog;
class QLabel;
class QListView;
class QListViewItem;
class QPrinter;

class Action;
class FindDialog;
class MsgEdit;

class TrWindow : public QMainWindow
{
    Q_OBJECT
public:
    static QWidget *splash();

    TrWindow();
    ~TrWindow();

    void openFile( const QString& name );

protected:
    virtual void closeEvent( QCloseEvent *ev );

private slots:
    void open();
    void save();
    void saveAs();
    void release();
    void print();
    void find();
    void findAgain();
    void newPhraseBook();
    void openPhraseBook();
    void closePhraseBook( int id );
    void editPhraseBook( int id );
    void printPhraseBook( int id );
    void overview();
    void revertSorting();
    void about();
    void aboutQt();

    void setupPhrase();
    void maybeSave();
    void updateCaption();
    void showNewCurrent( QListViewItem *item );
    void updateTranslation( const QString& translation );
    void updateFinished( bool finished );
    void toggleFinished( QListViewItem *item, const QPoint& p, int column );
    void next();
    void findNext( const QString& text, int where, bool matchCase );
    void revalidate();

private:
    typedef QList<PhraseBook> PBL;
    typedef QDict<PhraseBook> PBD;

    static void setupImageDict();
    static QString friendlyString( const QString& str );

    void setupMenuBar();
    void setupToolBars();
    void setCurrentLVI( QListViewItem *item );
    QString friendlyPhraseBookName( int k );
    bool openPhraseBook( const QString& name );
    bool savePhraseBook( const QString& name, const PhraseBook& pb );
    void updateProgress();
    void updatePhraseDict();
    PhraseBook getPhrases( const QString& source );
    bool danger( const QString& source, const QString& translation,
		 bool verbose = FALSE );

    QListView *lv;
    MsgEdit *me;
    QLabel *progress;
    QLabel *modified;
    MetaTranslator tor;
    int numFinished;
    int numNonobsolete;
    int numMessages;
    QString filename;
    bool dirty;
    bool messageIsShown;

    PBD phraseDict;
    PBL phraseBooks;
    QStringList phraseBookNames;

    QPrinter printer;
    QDialog *ov;

    FindDialog *f;
    QString findText;
    int findWhere;
    bool findMatchCase;
    QListViewItem *foundItem;
    int foundWhere;
    int foundOffset;

    QPopupMenu *phrasep;
    QPopupMenu *closePhraseBookp;
    QPopupMenu *editPhraseBookp;
    QPopupMenu *printPhraseBookp;
    int closePhraseBookId;
    int editPhraseBookId;
    int printPhraseBookId;
    Action *openAct;
    Action *saveAct;
    Action *saveAsAct;
    Action *releaseAct;
    Action *printAct;
    Action *exitAct;
    Action *undoAct;
    Action *redoAct;
    Action *cutAct;
    Action *copyAct;
    Action *pasteAct;
    Action *deleteAct;
    Action *selectAllAct;
    Action *findAct;
    Action *findAgainAct;
    Action *newPhraseBookAct;
    Action *openPhraseBookAct;
    Action *acceleratorsAct;
    Action *endingPunctuationAct;
    Action *phraseMatchesAct;
    Action *revertSortingAct;
    Action *bigIconsAct;
    Action *textLabelsAct;
    Action *aboutAct;
    Action *aboutQtAct;
    Action *overviewAct;
    Action *whatsThisAct;
};

#endif
