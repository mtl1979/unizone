/**********************************************************************
**   Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
**   msgedit.h
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

#ifndef MSGEDIT_H
#define MSGEDIT_H

#include <qsplitter.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>

#include <metatranslator.h>

#include "phrase.h"

class QCheckBox;
class QLabel;
class QListViewItem;
class QPushButton;
class QTextView;

class MED;
class PhraseLV;

class MsgEdit : public QSplitter
{
    Q_OBJECT
public:
    MsgEdit( MetaTranslator *t, QWidget *parent, const char *name = 0 );

    void showNothing();
    void showContext( const QString& fullContext, bool finished );
    void showMessage( const QString& text, const QString& comment,
		      const QString& fullContext, const QString& translation,
		      MetaTranslatorMessage::Type type,
		      const QValueList<Phrase>& phrases );
    void setFinished( bool finished );
    void setNextEnabled( bool enabled );

signals:
    void translationChanged( const QString& translation );
    void finished( bool finished );
    void next();

    void undoAvailable( bool avail );
    void redoAvailable( bool avail );
    void cutAvailable( bool avail );
    void copyAvailable( bool avail );
    void pasteAvailable( bool avail );

public slots:
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void del();
    void selectAll();

private slots:
    void emitTranslationChanged();
    void insertPhraseInTranslation( QListViewItem *item );
    void updateButtons();
    void startFromSource();
    void guessAgain();
    void finishAndNext();
    void updateCanPaste();

private:
    void setTranslation( const QString& translation, bool emitt );
    void setEditionEnabled( bool enabled );

    MetaTranslator *tor;
    QLabel *srcText;
    QTextView *tv;
    QLabel *translation;
    MED *translationMed;
    QLabel *phrases;
    PhraseLV *phraseLv;
    QCheckBox *finish;
    QPushButton *startFromSrc;
    QPushButton *guess;
    QPushButton *skip;
    QPushButton *finAndNxt;
    QString sourceText;
    QStringList guesses;
    bool mayOverwriteTranslation;
    bool canPaste;
};

#endif
