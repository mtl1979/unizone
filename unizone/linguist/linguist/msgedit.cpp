/**********************************************************************
**   Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
**   msgedit.cpp
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

/*  TRANSLATOR MsgEdit

  This is the right panel of the main window.
*/

#include <qapplication.h>
#include <qcheckbox.h>
#include <qclipboard.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmultilineedit.h>
#include <qpalette.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qstylesheet.h>
#include <qtextview.h>
#include <qwhatsthis.h>
#include <qvbox.h>

#include "msgedit.h"
#include "phraselv.h"

// defined in simtexth.cpp
extern QStringList similarTextHeuristicCandidates( const MetaTranslator *tor,
						   const char *text,
						   int maxCandidates );

static const int MaxCandidates = 10;

class MED : public QMultiLineEdit
{
public:
    MED( QWidget *parent, const char *name = 0 )
	: QMultiLineEdit( parent, name ) { }

    void del() { QMultiLineEdit::del(); }
    virtual QSize sizeHint() const;
};

QSize MED::sizeHint() const
{
    return QSize( 5, 5 ); // looks right
}

QString richMeta( const QString& text )
{
    return QString( "<small><font color=blue>(" ) + text +
	   QString( ")</font></small>" );
}

QString richText( const QString& text )
{
    const char backTab[] = "\a\b\f\n\r\t";
    const char * const friendlyBackTab[] = {
	QT_TRANSLATE_NOOP( "MsgEdit", "bell" ),
	QT_TRANSLATE_NOOP( "MsgEdit", "backspace" ),
	QT_TRANSLATE_NOOP( "MsgEdit", "new page" ),
	QT_TRANSLATE_NOOP( "MsgEdit", "new line" ),
	QT_TRANSLATE_NOOP( "MsgEdit", "carriage return" ),
	QT_TRANSLATE_NOOP( "MsgEdit", "tab" )
    };
    QString rich;
    int lastSpace = -2;

    for ( int i = 0; i < (int) text.length(); i++ ) {
	int ch = text[i].unicode();

	if ( ch < 0x20 ) {
	    const char *p = strchr( backTab, ch );
	    if ( p == 0 )
		rich += richMeta( QString::number(ch, 16) );
	    else
		rich += richMeta( MsgEdit::tr(friendlyBackTab[p - backTab]) );
	} else if ( ch == '<' ) {
	    rich += QString( "&lt;" );
	} else if ( ch == '>' ) {
	    rich += QString( "&gt;" );
	} else if ( ch == '&' ) {
	    rich += QString( "&amp;" );
	} else if ( ch == ' ' ) {
	    if ( lastSpace == i - 1 )
		rich += QChar( 0x00a0 );
	    else
		rich += ' ';
	    lastSpace = i;
	} else {
	    rich += QChar( ch );
	}
	if ( ch == '\n' )
	    rich += QString( "<br>" );
    }
    return rich;
}

QString richTextForMessage( const QString& text, const QString& comment,
			    const QString& fullContext )
{
    return QString( "<p><tt>" ) + richText( text ) +
	   QString( "</tt></p><hr>" ) +
	   richText( comment.simplifyWhiteSpace() ) + QString( "<hr><i>" ) +
	   richText( fullContext.simplifyWhiteSpace() ) + QString( "</i>" );
}

MsgEdit::MsgEdit( MetaTranslator *t, QWidget *parent, const char *name )
    : QSplitter( Vertical, parent, name )
{
    tor = t;

    QVBox * v;
    v = new QVBox( this );
    v->setMargin( 6 );
    srcText = new QLabel( tr("Source text and comments:"), v );
    tv = new QTextView( v, "text/comment/context view" );
    v->setStretchFactor( tv, 10 );
    tv->setHScrollBarMode( QScrollView::AlwaysOff );
    setResizeMode( v, Stretch );

    v = new QVBox( this );
    v->setMargin( 6 );
    translation = new QLabel( tr("Translation:"), v );
    translationMed = new MED( v, "translation" );
    v->setStretchFactor( translationMed, 10 );
    translationMed->setWordWrap( MED::WidgetWidth );
    
    setResizeMode( v, Stretch );

    QWidget * w;
    w = new QWidget( this );
    setResizeMode( w, FollowSizeHint );
    QHBoxLayout *hl = new QHBoxLayout( w, 6 );
    QVBoxLayout *vl = new QVBoxLayout( 6 );

    phrases = new QLabel( tr("Relevant phrases:"), w );
    phraseLv = new PhraseLV( w, "phrase list view" );
    hl->addLayout( vl );
    vl->addWidget( phrases );
    vl->addWidget( phraseLv );
    
    vl = new QVBoxLayout( 6 );
    hl->addLayout( vl );
    
    finish = new QCheckBox( tr("F&inished"), w, "finish" );
    startFromSrc = new QPushButton( tr("&Start from Source"), w,
				    "start from src" );
    guess = new QPushButton( tr("&Guess Again"), w, "guess again" );
    skip = new QPushButton( tr("S&kip"), w, "skip" );
    finAndNxt = new QPushButton( tr("Finish && Ne&xt"), w,
				 "finish and next" );

    vl->addWidget( finish );
    vl->addWidget( startFromSrc );
    vl->addWidget( guess );
    vl->addWidget( skip );
    vl->addWidget( finAndNxt );

    mayOverwriteTranslation = FALSE;
    canPaste = FALSE;

    tv->setFocusPolicy( NoFocus );
    phraseLv->setFocusPolicy( NoFocus );
    finish->setFocusPolicy( NoFocus );
    startFromSrc->setFocusPolicy( NoFocus );
    guess->setFocusPolicy( NoFocus );
    skip->setFocusPolicy( NoFocus );
    finAndNxt->setFocusPolicy( NoFocus );

    srcText->setAlignment( AlignLeft | AlignBottom );
    translation->setAlignment( AlignLeft | AlignBottom );
    phrases->setAlignment( AlignLeft | AlignBottom );

    tv->setTextFormat( QTextView::RichText );

    QValueList<int> sizes;
    sizes.append( 10 );
    sizes.append( 20 );
    sizes.append( 10 );
    setSizes( sizes );

    showNothing();
    //setFrameStyle( StyledPanel | Sunken );
    //setLineWidth( style().defaultFrameWidth() );
    //setOpaqueResize( TRUE );

    connect( translationMed, SIGNAL(textChanged()),
	     this, SLOT(emitTranslationChanged()) );
    connect( translationMed, SIGNAL(textChanged()),
	     this, SLOT(updateButtons()) );
    connect( translationMed, SIGNAL(undoAvailable(bool)),
	     this, SIGNAL(undoAvailable(bool)) );
    connect( translationMed, SIGNAL(redoAvailable(bool)),
	     this, SIGNAL(redoAvailable(bool)) );
    connect( translationMed, SIGNAL(copyAvailable(bool)),
	     this, SIGNAL(cutAvailable(bool)) );
    connect( translationMed, SIGNAL(copyAvailable(bool)),
	     this, SIGNAL(copyAvailable(bool)) );
    connect( qApp->clipboard(), SIGNAL(dataChanged()),
	     this, SLOT(updateCanPaste()) );
    connect( phraseLv, SIGNAL(doubleClicked(QListViewItem *)),
	     this, SLOT(insertPhraseInTranslation(QListViewItem *)) );
    connect( finish, SIGNAL(toggled(bool)), this, SIGNAL(finished(bool)) );
    connect( startFromSrc, SIGNAL(clicked()), this, SLOT(startFromSource()) );
    connect( guess, SIGNAL(clicked()), this, SLOT(guessAgain()) );
    connect( skip, SIGNAL(clicked()), this, SIGNAL(next()) );
    connect( finAndNxt, SIGNAL(clicked()), this, SLOT(finishAndNext()) );

    QWhatsThis::add( this, tr("This whole panel allows you to view and edit the"
			      " translation of some source text.") );
    QWhatsThis::add( tv, tr("This area shows the source text, a comment that"
			    " may guide you, and the context in which the text"
			    " occurs.") );
    QWhatsThis::add( translationMed, tr("This is where you can enter or modify"
					" the translation of some source"
					" text.") );
    QWhatsThis::add( finish, tr("Click here to switch between <i>finished</i>"
				" and <i>unfinished</i> states.  You should set"
				" this to <i>finished</i> when a translation"
				" sastisfies you.") );
    QWhatsThis::add( startFromSrc, tr("Click here to copy the source text into"
				      " the translation field.") );
    QWhatsThis::add( guess, tr("Click here to copy a guess translation into the"
			       " translation field.  The guess is made with the"
			       " philosophy that similar source texts have"
			       " similar translations." ) );
    QWhatsThis::add( skip, tr("Click here to move to the next unfinished"
			      " item.") );
    QWhatsThis::add( finAndNxt, tr("Click here to mark this item finished and"
				   " move to the next unfinished item.") );
}

void MsgEdit::showNothing()
{
    showContext( QString(""), FALSE );
}

void MsgEdit::showContext( const QString& fullContext, bool finished )
{
    setEditionEnabled( FALSE );
    sourceText = QString::null;
    guesses.clear();

    tv->setText( QString("<i>") + richText(fullContext.simplifyWhiteSpace()) +
		 QString("</i>") );
    setTranslation( QString(""), FALSE );
    setFinished( finished );
    finish->setEnabled( FALSE );
    finAndNxt->setEnabled( FALSE );
    phraseLv->clear();
}

void MsgEdit::showMessage( const QString& text, const QString& comment,
			   const QString& fullContext,
			   const QString& translation,
			   MetaTranslatorMessage::Type type,
			   const QValueList<Phrase>& phrases )
{
    bool obsolete = ( type == MetaTranslatorMessage::Obsolete );
    setEditionEnabled( !obsolete );
    sourceText = text;
    guesses.clear();

    tv->setText( richTextForMessage(text, comment, fullContext) );
    setTranslation( translation, FALSE );
    setFinished( type != MetaTranslatorMessage::Unfinished );

    finish->setEnabled( !obsolete );
    finAndNxt->setEnabled( !obsolete );

    // clear the undo and redo stacks
    translationMed->setUndoEnabled( FALSE );
    translationMed->setUndoEnabled( TRUE );

    QValueList<Phrase>::ConstIterator p;
    phraseLv->clear();
    for ( p = phrases.begin(); p != phrases.end(); ++p )
	(void) new PhraseLVI( phraseLv, *p );
}

void MsgEdit::setFinished( bool finished )
{
    if ( finished != finish->isOn() )
	finish->toggle();
}

void MsgEdit::setNextEnabled( bool enabled )
{
    skip->setEnabled( enabled );
}

void MsgEdit::undo()
{
    translationMed->undo();
}

void MsgEdit::redo()
{
    translationMed->redo();
}

void MsgEdit::cut()
{
    translationMed->cut();
}

void MsgEdit::copy()
{
    translationMed->copy();
}

void MsgEdit::paste()
{
    translationMed->paste();
}

void MsgEdit::del()
{
    translationMed->del();
}

void MsgEdit::selectAll()
{
    translationMed->selectAll();
}

void MsgEdit::emitTranslationChanged()
{
    emit translationChanged( translationMed->text() );
}

void MsgEdit::insertPhraseInTranslation( QListViewItem *item )
{
    translationMed->insert( ((PhraseLVI *) item)->phrase().target() );
    emitTranslationChanged();
}

void MsgEdit::updateButtons()
{
    bool overwrite = ( !translationMed->isReadOnly() &&
		       (translationMed->text().stripWhiteSpace().isEmpty() ||
			mayOverwriteTranslation) );
    mayOverwriteTranslation = FALSE;
    startFromSrc->setEnabled( overwrite );
    guess->setEnabled( overwrite );
}

void MsgEdit::startFromSource()
{
    mayOverwriteTranslation = TRUE;
    setTranslation( sourceText, TRUE );
}

void MsgEdit::guessAgain()
{
    setFinished( FALSE );
    if ( guesses.isEmpty() )
	guesses = similarTextHeuristicCandidates( tor, sourceText.latin1(),
						  MaxCandidates );
    if ( guesses.isEmpty() ) {
	qApp->beep();
    } else {
	QString translation = guesses.first();
	mayOverwriteTranslation = TRUE;
	setTranslation( translation, TRUE );
	guesses.remove( guesses.begin() );
	guesses.append( translation );
    }
}

void MsgEdit::finishAndNext()
{
    setFinished( TRUE );
    emit next();
}

void MsgEdit::updateCanPaste()
{
    bool oldCanPaste = canPaste;
    canPaste = ( !translationMed->isReadOnly() &&
		 !qApp->clipboard()->text().isNull() );
    if ( canPaste != oldCanPaste )
	emit pasteAvailable( canPaste );
}

void MsgEdit::setTranslation( const QString& translation, bool emitt )
{
    translationMed->blockSignals( !emitt );
    translationMed->setText( translation );
    translationMed->blockSignals( FALSE );
    if ( !emitt )
	updateButtons();
    emit cutAvailable( FALSE );
    emit copyAvailable( FALSE );
}

void MsgEdit::setEditionEnabled( bool enabled )
{
    translation->setEnabled( enabled );
    translationMed->setReadOnly( !enabled );
    phrases->setEnabled( enabled );
    phraseLv->setEnabled( enabled );
    QPalette pal = translationMed->palette();
    pal.setColor( QColorGroup::Text,
		  (enabled ? pal.active() : pal.disabled()).foreground() );
    translationMed->setPalette( pal );
    updateCanPaste();
}
