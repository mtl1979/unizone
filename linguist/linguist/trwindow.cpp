/**********************************************************************
**   Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
**   trwindow.cpp
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

/*  TRANSLATOR TrWindow

  This is the application's main window.
*/

#include "trwindow.h"
#include "finddialog.h"
#include "msgedit.h"
#include "phrasebookbox.h"
#include "printout.h"

#include <qaccel.h>
#include <qaction.h>
#include <qapplication.h>
#include <qdict.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlistview.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qsplitter.h>
#include <qstatusbar.h>
#include <qtextbrowser.h>
#include <qtextstream.h>
#include <qtoolbar.h>
#include <qwhatsthis.h>

#include <images.h>

static const char *logo_xpm[] = {
/* width height num_colors chars_per_pixel */
"21 16 213 2",
"   c white",
".  c #A3C511",
"+  c #A2C511",
"@  c #A2C611",
"#  c #A2C510",
"$  c #A2C513",
"%  c #A2C412",
"&  c #A2C413",
"*  c #A2C414",
"=  c #A2C515",
"-  c #A2C50F",
";  c #A3C510",
">  c #A2C410",
",  c #A2C411",
"'  c #A2C314",
")  c #A2C316",
"!  c #A2C416",
"~  c #A0C315",
"{  c #A1C313",
"]  c #A1C412",
"^  c #A2C40F",
"/  c #A1C410",
"(  c #A0C510",
"_  c #A0C511",
":  c #A1C414",
"<  c #9FC30E",
"[  c #98B51B",
"}  c #5F7609",
"|  c #5C6E0E",
"1  c #5B6E10",
"2  c #5C6C14",
"3  c #5A6E0A",
"4  c #839E16",
"5  c #A0C515",
"6  c #A0C513",
"7  c #A2C512",
"8  c #A1C512",
"9  c #A1C511",
"0  c #A1C50F",
"a  c #91AE12",
"b  c #505E11",
"c  c #1F2213",
"d  c #070606",
"e  c #040204",
"f  c #040306",
"g  c #15160F",
"h  c #2F3A0D",
"i  c #859F1B",
"j  c #A1C215",
"k  c #A0C50F",
"l  c #A1C510",
"m  c #A0C110",
"n  c #839C1B",
"o  c #1E240A",
"p  c #050205",
"q  c #030304",
"r  c #323917",
"s  c #556313",
"t  c #56680B",
"u  c #536609",
"v  c #4A561B",
"w  c #0B0D04",
"x  c #030208",
"y  c #090A05",
"z  c #5F6F18",
"A  c #A0C117",
"B  c #91AF10",
"C  c #1E2209",
"D  c #030205",
"E  c #17190D",
"F  c #7D981C",
"G  c #9ABA12",
"H  c #A3C411",
"I  c #A3C713",
"J  c #95B717",
"K  c #7F9A18",
"L  c #8FAE1B",
"M  c #394413",
"N  c #040305",
"O  c #090807",
"P  c #6C7E19",
"Q  c #A6C614",
"R  c #A1C411",
"S  c #64761F",
"T  c #030105",
"U  c #070707",
"V  c #728513",
"W  c #A2C40C",
"X  c #A2C70B",
"Y  c #89A519",
"Z  c #313B11",
"`  c #101409",
" . c #586A19",
".. c #97B620",
"+. c #1B2207",
"@. c #282D11",
"#. c #A6C41B",
"$. c #A1C413",
"%. c #A3C512",
"&. c #2E370B",
"*. c #030108",
"=. c #21260F",
"-. c #A5C21A",
";. c #A0C60D",
">. c #6D841A",
",. c #0F1007",
"'. c #040207",
"). c #0E1009",
"!. c #515F14",
"~. c #A2C41B",
"{. c #5E701B",
"]. c #030203",
"^. c #0B0B04",
"/. c #87A111",
"(. c #A0C411",
"_. c #A0C316",
":. c #212907",
"<. c #222C0B",
"[. c #A3C516",
"}. c #9CBE1A",
"|. c #5E6F1B",
"1. c #0E0F0B",
"2. c #040205",
"3. c #181B0D",
"4. c #93AE25",
"5. c #A0C610",
"6. c #617715",
"7. c #030306",
"8. c #070704",
"9. c #809818",
"0. c #A1C415",
"a. c #475416",
"b. c #030309",
"c. c #12170B",
"d. c #91B01E",
"e. c #5C721F",
"f. c #05050B",
"g. c #33371D",
"h. c #0E0F08",
"i. c #040405",
"j. c #758921",
"k. c #46511B",
"l. c #030207",
"m. c #131409",
"n. c #9FB921",
"o. c #859D21",
"p. c #080809",
"q. c #030305",
"r. c #46521C",
"s. c #8EB017",
"t. c #627713",
"u. c #4D5F17",
"v. c #97B71D",
"w. c #77901D",
"x. c #151708",
"y. c #0D0D0B",
"z. c #0C0B08",
"A. c #455216",
"B. c #A5C616",
"C. c #A0C114",
"D. c #556118",
"E. c #050307",
"F. c #050407",
"G. c #363E17",
"H. c #5D7309",
"I. c #A2BF28",
"J. c #A2C417",
"K. c #A4C620",
"L. c #60701D",
"M. c #030103",
"N. c #030303",
"O. c #809A1B",
"P. c #A0C310",
"Q. c #A0C410",
"R. c #A3C415",
"S. c #9CB913",
"T. c #6F801F",
"U. c #1A210A",
"V. c #1D1E0D",
"W. c #1D220F",
"X. c #1E210F",
"Y. c #0F0F07",
"Z. c #0E1007",
"`. c #090906",
" + c #2B360E",
".+ c #97B813",
"++ c #A2C50E",
"@+ c #A5C517",
"#+ c #90AD20",
"$+ c #5D6C1A",
"%+ c #394115",
"&+ c #050704",
"*+ c #040304",
"=+ c #202807",
"-+ c #5E6B21",
";+ c #728D0C",
">+ c #65791D",
",+ c #29330F",
"'+ c #7A911D",
")+ c #A2C614",
"!+ c #A1C513",
"~+ c #A3C50E",
"{+ c #A3C414",
"]+ c #9CBD11",
"^+ c #95B40C",
"/+ c #94B50F",
"(+ c #95B510",
"_+ c #99B913",
":+ c #A0C414",
"<+ c #9ABC11",
"[+ c #A0C314",
"}+ c #A1C40F",
"|+ c #A3C513",
". + + @ + # # $ % & * = & - + + + + + # # ",
"; > , > # > > $ ' ) ! ~ { ] ^ , - > , > # ",
"+ + / ( _ : < [ } | 1 2 3 4 5 6 : 7 8 # # ",
"+ 9 # ( 0 a b c d e e e f g h i j 9 k l + ",
"+ + > m n o p q r s t u v w x y z A & # # ",
"# % k B C D E F G H I J K L M N O P Q ] , ",
"$ R > S T U V W , X Y Z `  ...+.T @.#.$.] ",
"% %.* &.*.=.-.;.> >.,.'.).!.~.{.].^./.R 7 ",
"7 (._.:.D <.[.}.|.1.2.2.3.4.5.6.7.8.9._ 8 ",
". % 0.a.b.c.d.e.f.N g.h.2.i.j.k.l.m.n.$ # ",
"; + ; o.p.q.r.s.t.u.v.w.x.2.y.z.].A.B.l : ",
"# # R C.D.E.F.G.H.I.J.K.L.2.M.M.N.O.P.; l ",
"# / Q.R.S.T.U.].8.V.W.X.Y.e Z.`.]. +.+++7 ",
"+ + 9 / ; @+#+$+%+&+e *+=+-+;+>+,+'+)+, # ",
"# + > % & !+~+{+]+^+/+(+_+) Q.:+<+[+$ R # ",
"7 + > }+# % k |+8 + > + * $ _ / , 7 8 ] - "};

typedef QValueList<MetaTranslatorMessage> TML;

static const int Text0MaxLen = 25;
static const int MessageMS = 2500;

static QDict<Embed> *imageDict = 0;

enum Ending { End_None, End_FullStop, End_Interrobang, End_Colon,
	      End_Ellipsis };

static Ending ending( const QString& str )
{
    int ch = 0;
    if ( !str.isEmpty() )
	ch = str.right( 1 )[0].unicode();

    switch ( ch ) {
    case 0x002e: // full stop
	if ( str.right(3) == QString("...") )
	    return End_Ellipsis;
	else
	    return End_FullStop;
    case 0x0589: // armenian full stop
    case 0x06d4: // arabic full stop
    case 0x3002: // ideographic full stop
	return End_FullStop;
    case 0x0021: // exclamation mark
    case 0x003f: // question mark
    case 0x00a1: // inverted exclamation mark
    case 0x00bf: // inverted question mark
    case 0x01c3: // latin letter retroflex click
    case 0x037e: // greek question mark
    case 0x061f: // arabic question mark
    case 0x203c: // double exclamation mark
    case 0x203d: // interrobang
    case 0x2048: // question exclamation mark
    case 0x2049: // exclamation question mark
    case 0x2762: // heavy exclamation mark ornament
	return End_Interrobang;
    case 0x003a: // colon
	return End_Colon;
    case 0x2026: // horizontal ellipsis
	return End_Ellipsis;
    default:
	return End_None;
    }
}

class Action : public QAction
{
public:
    Action( QPopupMenu *pop, const QString& menuText, QObject *receiver,
	    const char *member, int accel = 0, bool toggle = FALSE );
    Action( QPopupMenu *pop, const QString& menuText, int accel = 0,
	    bool toggle = FALSE );

    virtual void setWhatsThis( const QString& whatsThis );

    bool addToToolbar( QToolBar *tb, const QString& text, const char *xpmName );
};

Action::Action( QPopupMenu *pop, const QString& menuText, QObject *receiver,
		const char *member, int accel, bool toggle )
    : QAction( pop->parent(), (const char *) 0, toggle )
{
    QAction::addTo( pop );
    setMenuText( menuText );
    setAccel( accel );
    connect( this, SIGNAL(activated()), receiver, member );
}

Action::Action( QPopupMenu *pop, const QString& menuText, int accel,
		bool toggle )
    : QAction( pop->parent(), (const char *) 0, toggle )
{
    QAction::addTo( pop );
    setMenuText( menuText );
    setAccel( accel );
}

void Action::setWhatsThis( const QString& whatsThis )
{
    QAction::setWhatsThis( whatsThis );
    setStatusTip( whatsThis );
}

bool Action::addToToolbar( QToolBar *tb, const QString& text,
			   const char *xpmName )
{
    Embed *ess = 0;
    Embed *ell = 0;

    setText( text );
    ess = imageDict->find( QString("small/") + QString(xpmName) );
    ell = imageDict->find( QString("large/") + QString(xpmName) );
    if ( ess != 0 && ell != 0 ) {
	QPixmap small, large;
	small.loadFromData( ess->data, ess->size );
	large.loadFromData( ell->data, ell->size );
	QIconSet s( small, large );
	setIconSet( s );
    }
    return QAction::addTo( tb );
}

static QString rho( int n )
{
    QString s;
    s.sprintf( "%.10d", n );
    return s;
}

class LVI : public QListViewItem
{
public:
    LVI( QListView *parent, QString text = QString::null )
	: QListViewItem( parent, text ) { setText( 2, rho(count++) ); }
    LVI( QListViewItem *parent, QString text = QString::null )
	: QListViewItem( parent, text ) { setText( 2, rho(count++) ); }

    virtual QString text( int column ) const;
    virtual QString key( int column, bool ascending ) const;
    virtual bool danger() const { return FALSE; }

private:
    static int count;
};

int LVI::count = 0;

QString LVI::text( int column ) const
{
    if ( column == 1 && danger() ) {
	return TrWindow::tr( "(?)" );
    } else {
	return QListViewItem::text( column );
    }
}

QString LVI::key( int column, bool /* ascending */ ) const
{
    // see Section 5, Exercice 4 in The Art of Computer Programming
    QString k = text( column ).lower();
    k.replace( QRegExp("&"), QString("") );
    k += QChar::null;
    k += text( column );
    return k;
}

class ContextLVI : public LVI
{
public:
    ContextLVI( QListView *lv, const QString& context );

    virtual bool danger() const { return dangerCount > 0; }

    void appendToComment( const QString& x );
    void incrementUnfinishedCount();
    void decrementUnfinishedCount();
    void incrementDangerCount();
    void decrementDangerCount();

    QString context() const { return text( 0 ); }
    QString comment() const { return com; }
    QString fullContext() const;
    bool finished() const { return unfinishedCount == 0; }

private:
    QString com;
    int unfinishedCount;
    int dangerCount;
};

ContextLVI::ContextLVI( QListView *lv, const QString& context  )
    : LVI( lv, context ), com( "" )
{
    unfinishedCount = 0;
    dangerCount = 0;
    setText( 1, TrWindow::tr("X") );
}

void ContextLVI::appendToComment( const QString& x )
{
    if ( !com.isEmpty() )
	com += QString( "\n\n" );
    com += x;
}

void ContextLVI::incrementUnfinishedCount()
{
    if ( unfinishedCount++ == 0 ) {
	setText( 1, QString("") );
	repaint();
    }
}

void ContextLVI::decrementUnfinishedCount()
{
    if ( --unfinishedCount == 0 ) {
	setText( 1, TrWindow::tr("X") );
	repaint();
    }
}

void ContextLVI::incrementDangerCount()
{
    if ( dangerCount++ == 0 )
	repaint();
}

void ContextLVI::decrementDangerCount()
{
    if ( --dangerCount == 0 )
	repaint();
}

QString ContextLVI::fullContext() const
{
    return QString( TrWindow::tr("Context %1.  %2") ).arg( context() )
	   .arg( comment() ).stripWhiteSpace();
}

class MessageLVI : public LVI
{
public:
    MessageLVI( ContextLVI *parent, const MetaTranslatorMessage& message,
		const QString& text, const QString& comment );

    virtual bool danger() const { return d; }

    void setTranslation( const QString& translation );
    void setFinished( bool finished );
    void setDanger( bool danger );

    QString context() const;
    QString sourceText() const { return tx; }
    QString comment() const { return com; }
    QString translation() const { return m.translation(); }
    bool finished() const { return fini; }
    MetaTranslatorMessage message() const;

private:
    MetaTranslatorMessage m;
    QString tx;
    QString com;
    bool fini;
    bool d;
};

MessageLVI::MessageLVI( ContextLVI *parent,
			const MetaTranslatorMessage& message,
			const QString& text, const QString& comment )
    : LVI( parent ), m( message ), tx( text ), com( comment )
{
    if ( m.translation().isEmpty() ) {
	QString t = "";
#if 0
	/*
	  Copy over the ending from the source to the translation by default.
	*/
	Ending end = ending( text );
	if ( end == End_None ) {
	    t = QString( "" );
	} else if ( end == End_Ellipsis ) {
	    t = QString( "..." );
	} else {
	    t = text.right( 1 );
	}
#endif
	m.setTranslation( t );
    }

    QString shortened = text.simplifyWhiteSpace();
    if ( (int) shortened.length() > Text0MaxLen ) {
	QString dots = TrWindow::tr( "..." );
	shortened.truncate( Text0MaxLen - dots.length() );
	shortened.append( dots );
    }
    setText( 0, shortened );
    fini = TRUE;
    d = FALSE;

    switch ( m.type() ) {
    case MetaTranslatorMessage::Finished:
	setText( 1, TrWindow::tr("X") );
	break;
    case MetaTranslatorMessage::Unfinished:
	setFinished( FALSE );
	break;
    case MetaTranslatorMessage::Obsolete:
	setText( 1, TrWindow::tr("(obs.)") );
    }
}

void MessageLVI::setTranslation( const QString& translation )
{
    m.setTranslation( translation );
}

void MessageLVI::setFinished( bool finished )
{
    if ( !fini && finished ) {
	m.setType( MetaTranslatorMessage::Finished );
	setText( 1, TrWindow::tr("X") );
	repaint();
	((ContextLVI *) parent())->decrementUnfinishedCount();
    } else if ( fini && !finished ) {
	m.setType( MetaTranslatorMessage::Unfinished );
	setText( 1, QString("") );
	repaint();
	((ContextLVI *) parent())->incrementUnfinishedCount();
    }
    fini = finished;
}

void MessageLVI::setDanger( bool danger )
{
    if ( !d && danger ) {
	repaint();
	((ContextLVI *) parent())->incrementDangerCount();
    } else if ( d && !danger ) {
	repaint();
	((ContextLVI *) parent())->decrementDangerCount();
    }
    d = danger;
}

QString MessageLVI::context() const
{
    return QString( m.context() );
}

MetaTranslatorMessage MessageLVI::message() const
{
    return m;
}

QWidget *TrWindow::splash()
{
    Embed *ess = 0;

    setupImageDict();
    ess = imageDict->find( QString("splash.png") );
    if ( ess == 0 )
	return 0;

    QPixmap pixmap;
    pixmap.loadFromData( ess->data, ess->size );

    QLabel *splash = new QLabel( 0, "splash", Qt::WDestructiveClose |
				 Qt::WStyle_Customize | Qt::WStyle_NoBorder );
    splash->setFrameStyle( QFrame::WinPanel | QFrame::Raised );
    splash->setPixmap( pixmap );
    splash->adjustSize();
    QRect r = QApplication::desktop()->geometry();
    splash->move( r.center() - splash->rect().center() );
    splash->show();
    splash->repaint( FALSE );
    QApplication::flushX();
    return splash;
}

TrWindow::TrWindow()
    : QMainWindow( 0, "translation window",
		   WType_TopLevel | WDestructiveClose )
{
    setIcon( QPixmap(logo_xpm) );

    setupImageDict();
    QSplitter *sp = new QSplitter( this );
    setCentralWidget( sp );

    lv = new QListView( sp, "source text list view" );
    lv->setRootIsDecorated( TRUE );
    lv->setAllColumnsShowFocus( TRUE );
    lv->setShowSortIndicator( TRUE );
    lv->setSorting( 2 );
    lv->addColumn( tr("Source text"), 120 );
    lv->addColumn( tr("Fin.") );
    lv->setColumnAlignment( 1, AlignCenter );

    me = new MsgEdit( &tor, sp, "translator message edit" );
    messageIsShown = FALSE;

    setupMenuBar();
    setupToolBars();
    progress = new QLabel( statusBar(), "progress" );
    statusBar()->addWidget( progress, 0, TRUE );
    modified = new QLabel( tr(" MOD "), statusBar(), "modified?" );
    statusBar()->addWidget( modified, 0, TRUE );

    numFinished = 0;
    numNonobsolete = 0;
    numMessages = 0;
    updateProgress();

    dirty = FALSE;
    updateCaption();

    phraseBooks.setAutoDelete( TRUE );

    ov = new QDialog( this, "overview dialog" );

    QTextBrowser *tv = new QTextBrowser( ov, "overview browser" );
    tv->setText( tr("<p> <b>Qt Linguist</b> is a utility for translating"
    " applications written with Qt. </p>\n\n"

    "<p> Translation of a Qt application is a three-step process: <ol>\n"
    " <li> Run <b>lupdate</b> to extract translatable text from the C++ source"
    " code of the Qt application, resulting in a source message file (a TS"
    " file).\n"
    " <li> Provide translations for the source texts in the TS file.\n"
    " <li> Run <b>lrelease</b> to obtain a light-weight message file (a QM"
    " file) from the TS file, suitable only for end use.\n"
    "</ol></p>\n\n"

    "<p> <b>Qt Linguist</b>'s role is to let you provide translations for"
    " source texts (step 2 above).  Each source text is characterized by the"
    " text itself, a <i>context</i> (usually the name of the C++ class"
    " containing the text), and a <i>comment</i> to help you distinguish the"
    " text.  Thus, distinct occurrences of terms such as \"Enabled\" may be"
    " translated differently according to the comment and the context in which"
    " they appear (\"Activado\", \"Activada\", etc., in Spanish).</p>\n\n"

    "<p> When translating an item, you have the following information close at"
    " hand:<ul>\n"
    " <li> the source text;\n"
    " <li> the comment for the source text;\n"
    " <li> the context in which the source text appears, possibly with an"
    " explanation of where to find the context in the Qt application or other"
    " comment;\n"
    " <li> relevant phrases from phrase books, with translations and"
    " definitions.\n"
    "</ul> Phrase books provide instant information about technical terms, and"
    " can be edited using <b>Qt Linguist</b>.  There is also a <i>finished</i>"
    " flag that you may turn on when you are satisfied with a translation, in"
    " which case the translation is used instead of the source text when"
    " running the Qt application with the QM file.</p>\n\n"

    "<p> When the Qt application evolves (say, from release 1.0 to release"
    " 1.1), the utility <b>lupdate</b> merges the current source texts with the"
    " previous message file, to reuse existing translations.  In some typical"
    " cases, <b>lupdate</b> may suggest translations.  These translations are"
    " marked as <i>unfinished</i>, so you can easily go over them.  In any"
    " case, you can press the <i>Guess Again</i> button and obtain existing"
    " translations for similar source text.</p>\n\n"

    "<p> Furthermore, <b>Qt Linguist</b> can flag the following three common"
    " errors: <ol>\n"
    " <li> An accelerator is present in the source text but is absent from the"
    " translation (or vice versa).  Thus, \"&amp;File\" translated as"
    " \"Fichier\" in French is flagged as suspicious (it should probably read"
    " \"&amp;Fichier\").\n"
    " <li> The translation does not end with the same punctuation as the source"
    " text.  Thus, \"Open...\" translated as \"Ouvrir\" is flagged as"
    " suspicious (it should probably read \"Ouvrir...\").\n"
    " <li> A suggested translation from the phrase book is ignored.  Thus,"
    " \"Save\" translated as \"Sauver\" is flagged as suspicious, if a phrase"
    " book entry gives \"Sauvegarder\" as the translation for \"Save\"."
    "</p>\n") );

    QPushButton *ok = new QPushButton( tr("OK"), ov, "ok overview" );
    ok->setDefault( TRUE );
    connect( ok, SIGNAL(clicked()), ov, SLOT(accept()) );

    QVBoxLayout *vlay = new QVBoxLayout( ov, 11, 11 );
    QHBoxLayout *hlay = new QHBoxLayout;
    vlay->add( tv );
    vlay->addLayout( hlay );
    hlay->addStretch( 1 );
    hlay->add( ok );
    hlay->addStretch( 1 );
    ov->resize( 480, 400 );

    f = new FindDialog( this, "find", FALSE );
    f->setCaption( tr("Qt Linguist") );
    findWhere = 0;
    findMatchCase = FALSE;
    foundItem = 0;
    foundWhere = 0;
    foundOffset = 0;

    connect( lv, SIGNAL(currentChanged(QListViewItem *)),
	    this, SLOT(showNewCurrent(QListViewItem *)) );
    connect( lv, SIGNAL(clicked(QListViewItem *, const QPoint&, int)),
	     this, SLOT(toggleFinished(QListViewItem *, const QPoint&, int)) );
    connect( me, SIGNAL(translationChanged(const QString&)),
	     this, SLOT(updateTranslation(const QString&)) );
    connect( me, SIGNAL(finished(bool)), this, SLOT(updateFinished(bool)) );
    connect( me, SIGNAL(next()), this, SLOT(next()) );
    connect( f, SIGNAL(findNext(const QString&, int, bool)),
	     this, SLOT(findNext(const QString&, int, bool)) );

    QWhatsThis::add( lv, tr("This panel lists the source texts grouped by"
			    " contexts.  Items that violate validation rules"
			    " are shown with a '(?)'.") );

    QWidget * desktop = QApplication::desktop();
    QSize s;
    if ( desktop->height() <= 768 )
	s.setHeight( desktop->height()-50 );
    else if ( desktop->height() <= 768+50 )
	s.setHeight( 768-50 );
    else if ( desktop->height() <= 1200 )
	s.setHeight( desktop->height()-100 );
    else
	s.setHeight( 1100 );
    s.setWidth( sizeHint().width() );
    if ( s.width() * 8 < s.height() * 5 && s.height() < desktop->width() - 30 )
	s.setWidth( s.height() * 5 / 8 );
    resize( s );
}

TrWindow::~TrWindow()
{
}

void TrWindow::openFile( const QString& name )
{
    if ( !name.isEmpty() ) {
	statusBar()->message( tr("Loading...") );
	qApp->processEvents();
	if ( tor.load(name) ) {
	    lv->clear();
	    lv->repaint();
	    lv->viewport()->repaint();
	    lv->setUpdatesEnabled( FALSE );
	    lv->viewport()->setUpdatesEnabled( FALSE );
	    setEnabled( FALSE );
	    numFinished = 0;
	    numNonobsolete = 0;
	    numMessages = 0;

	    TML all = tor.messages();
	    TML::Iterator it;
	    QDict<ContextLVI> contexts( 1009 );

	    for ( it = all.begin(); it != all.end(); ++it ) {
		qApp->processEvents();
		ContextLVI *c = contexts.find( QString((*it).context()) );
		if ( c == 0 ) {
		    c = new ContextLVI( lv, tor.toUnicode((*it).context()) );
		    contexts.insert( QString((*it).context()), c );
		}
		if ( (*it).sourceText()[0] == '\0' ) {
		    c->appendToComment( tor.toUnicode((*it).comment()) );
		} else {
		    (void) new MessageLVI( c, *it,
					   tor.toUnicode((*it).sourceText()),
					   tor.toUnicode((*it).comment()) );
		    if ( (*it).type() != MetaTranslatorMessage::Obsolete ) {
			numNonobsolete++;
			if ( (*it).type() == MetaTranslatorMessage::Finished )
			    numFinished++;
		    }
		    numMessages++;
		}
	    }
	    lv->viewport()->setUpdatesEnabled( TRUE );
	    lv->setUpdatesEnabled( TRUE );
	    setEnabled( TRUE );
	    lv->repaint();
	    lv->viewport()->repaint();
	    updateProgress();
	    filename = name;
	    dirty = FALSE;
	    updateCaption();
	    revalidate();
	    me->showNothing();
	    messageIsShown = FALSE;
	    statusBar()->message( tr("%1 message(s) loaded.").arg(numMessages),
				  MessageMS );

	    foundItem = 0;
	    foundWhere = 0;
	    foundOffset = 0;
	    findAct->setEnabled( TRUE );
	    findAgainAct->setEnabled( FALSE );
	} else {
	    statusBar()->clear();
	    QMessageBox::warning( this, tr("Qt Linguist"),
				  tr("Cannot open '%1'.").arg(name) );
	}
    }
}

void TrWindow::closeEvent( QCloseEvent *ev )
{
    maybeSave();
    QMainWindow::closeEvent( ev );
}

void TrWindow::open()
{
    maybeSave();

    QString newFilename = QFileDialog::getOpenFileName( filename,
	    tr("Qt translation source (*.ts)\n"
	       "All files (*)"), this );
    openFile( newFilename );
}

void TrWindow::save()
{
    if ( filename.isEmpty() )
	return;

    if ( tor.save(filename) ) {
	dirty = FALSE;
	updateCaption();
	statusBar()->message( tr("File saved."), MessageMS );
    } else {
	QMessageBox::warning( this, tr("Qt Linguist"), tr("Cannot save '%1'.")
			      .arg(filename) );
    }
}

void TrWindow::saveAs()
{
    QString newFilename = QFileDialog::getSaveFileName( filename,
	    tr("Qt translation source (*.ts)\n"
	       "All files (*)") );
    if ( !newFilename.isEmpty() ) {
	filename = newFilename;
	save();
	updateCaption();
    }
}

void TrWindow::release()
{
    QString newFilename = filename;
    newFilename.replace( QRegExp(".ts$"), "" );
    newFilename += QString( ".qm" );

    newFilename = QFileDialog::getSaveFileName( newFilename,
	    tr("Qt message files for end use (*.qm)\n"
	       "All files (*)") );
    if ( !newFilename.isEmpty() ) {
	if ( tor.release(newFilename) )
	    statusBar()->message( tr("File created."), MessageMS );
	else
	    QMessageBox::warning( this, tr("Qt Linguist"),
				  tr("Cannot save '%1'.").arg(newFilename) );
    }
}

void TrWindow::print()
{
    int pageNum = 0;

    if ( printer.setup(this) ) {
	printer.setDocName( filename );
	statusBar()->message( tr("Printing...") );
	PrintOut pout( &printer );
	ContextLVI *c = (ContextLVI *) lv->firstChild();
	while ( c != 0 ) {
	    pout.vskip();
	    pout.setRule( PrintOut::ThickRule );
	    pout.setGuide( c->context() );
	    pout.addBox( 100, tr("Context: %1").arg(c->context()),
			 PrintOut::Strong );
	    pout.flushLine();
	    pout.addBox( 4 );
	    pout.addBox( 92, c->comment(), PrintOut::Emphasis );
	    pout.flushLine();
	    pout.setRule( PrintOut::ThickRule );

	    MessageLVI *m = (MessageLVI *) c->firstChild();
	    while ( m != 0 ) {
		pout.setRule( PrintOut::ThinRule );

		QString type;
		switch ( m->message().type() ) {
		case MetaTranslatorMessage::Finished:
		    type = m->danger() ? tr( "(?)" ) : tr( "finished" );
		    break;
		case MetaTranslatorMessage::Obsolete:
		    type = tr( "obsolete" );
		    break;
		default:
		    type = QString( "" );
		}
		pout.addBox( 40, m->sourceText() );
		pout.addBox( 4 );
		pout.addBox( 40, m->translation() );
		pout.addBox( 4 );
		pout.addBox( 12, type, PrintOut::Normal, Qt::AlignRight );
		if ( !m->comment().isEmpty() ) {
		    pout.flushLine();
		    pout.addBox( 4 );
		    pout.addBox( 92, m->comment(), PrintOut::Emphasis );
		}
		pout.flushLine( TRUE );

		if ( pout.pageNum() != pageNum ) {
		    pageNum = pout.pageNum();
		    statusBar()->message( tr("Printing... (page %1)")
					  .arg(pageNum) );
		}
		m = (MessageLVI *) m->nextSibling();
	    }
	    c = (ContextLVI *) c->nextSibling();
	}
	pout.flushLine( TRUE );
	statusBar()->message( tr("Printing completed"), MessageMS );
    } else {
	statusBar()->message( tr("Printing aborted"), MessageMS );
    }
}

void TrWindow::find()
{
    f->show();
    f->setActiveWindow();
    f->raise();
}

void TrWindow::findAgain()
{
    QListViewItem *i = 0;
    QListViewItem *j = foundItem;
    int pass = 0;

    foundOffset++;

#if 1
    /*
      As long as we don't implement highlighting of the text in the QTextView,
      we may have only one match per message.
    */
    foundWhere = FindDialog::Comments;
    foundOffset = (int) 0x7fffffff;
#endif

    do {
	i = j;

	if ( j == 0 ) {
	    if ( foundItem != 0 )
		statusBar()->message( tr("Search wrapped."), MessageMS );
	    j = lv->firstChild();
	}

	if ( j != 0 ) {
	    ContextLVI *c = (ContextLVI *) j;
	    MessageLVI *m = (MessageLVI *) j;

	    switch ( foundWhere ) {
	    case 0:
		foundWhere = FindDialog::SourceText;
		foundOffset = 0;
		// fall through
	    case FindDialog::SourceText:
		if ( j->parent() != 0 && (findWhere & foundWhere) != 0 ) {
		    foundOffset = m->sourceText().find( findText, foundOffset,
							findMatchCase );
		    if ( foundOffset >= 0 ) {
			foundItem = j;
			setCurrentLVI( foundItem );
			return;
		    }
		}
		foundWhere = FindDialog::Translations;
		foundOffset = 0;
		// fall through
	    case FindDialog::Translations:
		if ( j->parent() != 0 && (findWhere & foundWhere) != 0 ) {
		    foundOffset = m->translation().find( findText, foundOffset,
							 findMatchCase );
		    if ( foundOffset >= 0 ) {
			foundItem = j;
			setCurrentLVI( foundItem );
			return;
		    }
		}
		foundWhere = FindDialog::Comments;
		foundOffset = 0;
		// fall through
	    case FindDialog::Comments:
		if ( (findWhere & foundWhere) != 0 ) {
		    if ( j->parent() != 0 ) {
			foundOffset = m->comment().find( findText, foundOffset,
							 findMatchCase );
			if ( foundOffset >= 0 ) {
			    foundItem = j;
			    setCurrentLVI( foundItem );
			    return;
			}
		    } else {
			foundOffset = c->comment().find( findText, foundOffset,
							 findMatchCase );
			if ( foundOffset >= 0 ) {
			    foundItem = j;
			    setCurrentLVI( foundItem );
			    return;
			}
		    }
		}
		foundWhere = 0;
		foundOffset = 0;
	    }

	    if ( j->firstChild() != 0 ) {
		j = j->firstChild();
	    } else {
		if ( j->nextSibling() == 0 )
		    j = i->parent();
		if ( j != 0 )
		    j = j->nextSibling();
	    }
	}
    } while ( i != foundItem || ++pass == 1 );

    statusBar()->message( tr("No match."), MessageMS );
    qApp->beep();
    foundItem = 0;
    foundWhere = 0;
    foundOffset = 0;
}

void TrWindow::newPhraseBook()
{
    QString name;
    while ( TRUE ) {
	name = QFileDialog::getSaveFileName( QString::null,
		       tr("Qt phrase books (*.qph)\n"
			  "All files (*)") );
	if ( !QFile::exists(name) )
	    break;
	QMessageBox::warning( this, tr("Qt Linguist"),
			      tr("There already exists a file named '%1'."
				 "  Please choose another name.").arg(name) );
    }
    if ( !name.isEmpty() ) {
	PhraseBook pb;
	if ( savePhraseBook(name, pb) ) {
	    if ( openPhraseBook(name) )
		statusBar()->message( tr("Phrase book created."), MessageMS );
	}
    }
}

void TrWindow::openPhraseBook()
{
    QString name = QFileDialog::getOpenFileName( QString::null,
	    tr("Qt phrase books (*.qph)\n"
	       "All files (*)") );

    if ( !name.isEmpty() && !phraseBookNames.contains(name) ) {
	if ( openPhraseBook(name) ) {
	    int n = phraseBooks.at( phraseBooks.count() - 1 )->count();
	    statusBar()->message( tr("%1 phrase(s) loaded.").arg(n),
				  MessageMS );
	}
    }
}

void TrWindow::closePhraseBook( int id )
{
    int index = closePhraseBookp->indexOf( id );
    phraseBooks.remove( index );
    phraseBookNames.remove( phraseBookNames.at(index) );
    updatePhraseDict();

    closePhraseBookp->removeItem( id );
    editPhraseBookp->removeItem( editPhraseBookp->idAt(index) );
    printPhraseBookp->removeItem( printPhraseBookp->idAt(index) );
}

void TrWindow::editPhraseBook( int id )
{
    int index = editPhraseBookp->indexOf( id );
    PhraseBookBox box( phraseBookNames[index], *phraseBooks.at(index), this,
		       "phrase book box", TRUE );
    box.setCaption( tr("%1 - %2").arg(tr("Qt Linguist"))
				 .arg(friendlyPhraseBookName(index)) );
    box.exec();
    *phraseBooks.at( index ) = box.phraseBook();
    updatePhraseDict();
}

void TrWindow::printPhraseBook( int id )
{
    int index = printPhraseBookp->indexOf( id );
    int pageNum = 0;

    if ( printer.setup(this) ) {
	printer.setDocName( phraseBookNames[index] );
	statusBar()->message( tr("Printing...") );
	PrintOut pout( &printer );
	PhraseBook *phraseBook = phraseBooks.at( index );
	PhraseBook::Iterator p;
	pout.setRule( PrintOut::ThinRule );
	for ( p = phraseBook->begin(); p != phraseBook->end(); ++p ) {
	    pout.setGuide( (*p).source() );
	    pout.addBox( 29, (*p).source() );
	    pout.addBox( 4 );
	    pout.addBox( 29, (*p).target() );
	    pout.addBox( 4 );
	    pout.addBox( 34, (*p).definition(), PrintOut::Emphasis );

	    if ( pout.pageNum() != pageNum ) {
		pageNum = pout.pageNum();
		statusBar()->message( tr("Printing... (page %1)")
				      .arg(pageNum) );
	    }
	    pout.setRule( PrintOut::ThinRule );
	    pout.flushLine( TRUE );
	}
	pout.flushLine( TRUE );
	statusBar()->message( tr("Printing completed"), MessageMS );
    } else {
	statusBar()->message( tr("Printing aborted"), MessageMS );
    }
}

void TrWindow::revertSorting()
{
    lv->setSorting( 2 );
}

void TrWindow::overview()
{
    ov->show();
    ov->setActiveWindow();
    ov->raise();
}

void TrWindow::about()
{
    Embed *ess = 0;

    setupImageDict();
    ess = imageDict->find( QString("splash.png") );
    if ( ess == 0 )
	return;

    QPixmap pixmap;
    pixmap.loadFromData( ess->data, ess->size );

    QDialog about( this, 0, TRUE );
    about.setCaption( tr("Qt Linguist") );
    about.resize( 447, 464 );

    QLabel *splash = new QLabel( &about );
    splash->setPixmap( pixmap );
    splash->setAlignment( QLabel::AlignCenter );

    QLabel *version = new QLabel( tr("Version pre1.0"), &about );
    version->setAlignment( QLabel::AlignCenter );
    QLabel *copyright = new QLabel( tr("Copyright (C) 2000 Trolltech AS"),
				    &about );
    copyright->setAlignment( QLabel::AlignCenter );
    QPushButton *ok = new QPushButton( tr("OK"), &about, "ok about" );
    ok->setDefault( TRUE );
    about.setFocusProxy( ok );
    connect( ok, SIGNAL(clicked()), &about, SLOT(accept()) );

    QVBoxLayout *vlay = new QVBoxLayout( &about, 11, 6 );
    QHBoxLayout *hlay = new QHBoxLayout;

    vlay->addWidget( splash );
    vlay->add( version );
    vlay->add( copyright );
    vlay->addStretch( 1 );
    vlay->addLayout( hlay );
    hlay->addStretch( 1 );
    hlay->add( ok );
    hlay->addStretch( 1 );

    about.exec();
}

void TrWindow::aboutQt()
{
    QMessageBox::aboutQt( this );
}

void TrWindow::setupPhrase()
{
    bool enabled = !phraseBooks.isEmpty();
    phrasep->setItemEnabled( closePhraseBookId, enabled );
    phrasep->setItemEnabled( editPhraseBookId, enabled );
    phrasep->setItemEnabled( printPhraseBookId, enabled );
}

void TrWindow::maybeSave()
{
    if ( dirty ) {
	switch ( QMessageBox::information(this, tr("Qt Linguist"),
				  tr("Do you want to save '%1'?")
				  .arg(filename),
				  QMessageBox::Yes | QMessageBox::Default,
				  QMessageBox::No) ) {
	case QMessageBox::Yes:
	    save();
	}
    }
}

void TrWindow::updateCaption()
{
    QString cap;
    bool enable = !filename.isEmpty();
    saveAct->setEnabled( enable );
    saveAsAct->setEnabled( enable );
    releaseAct->setEnabled( enable );
    printAct->setEnabled( enable );
    acceleratorsAct->setEnabled( enable );
    endingPunctuationAct->setEnabled( enable );
    phraseMatchesAct->setEnabled( enable );
    revertSortingAct->setEnabled( enable );

    if ( filename.isEmpty() )
	cap = tr( "Qt Linguist by Trolltech" );
    else
	cap = tr( "%1 - %2" ).arg( tr("Qt Linguist by Trolltech") )
			     .arg( filename );
    setCaption( cap );
    modified->setEnabled( dirty );
}

void TrWindow::showNewCurrent( QListViewItem *item )
{
    messageIsShown = ( item != 0 && item->parent() != 0 );

    if ( messageIsShown ) {
	ContextLVI *c = (ContextLVI *) item->parent();
	MessageLVI *m = (MessageLVI *) item;
	me->showMessage( m->sourceText(), m->comment(), c->fullContext(),
			 m->translation(), m->message().type(),
			 getPhrases(m->sourceText()) );
    } else {
	if ( item == 0 ) {
	    me->showNothing();
	} else {
	    ContextLVI *c = (ContextLVI *) item;
	    me->showContext( c->fullContext(), c->finished() );
	}
    }
    deleteAct->setEnabled( messageIsShown );
    selectAllAct->setEnabled( messageIsShown );
}

void TrWindow::updateTranslation( const QString& translation )
{
    QListViewItem *item = lv->currentItem();
    if ( item != 0 && item->parent() != 0 ) {
	MessageLVI *m = (MessageLVI *) item;
	QString stripped = translation.stripWhiteSpace();
	if ( stripped != m->translation() ) {
	    m->setTranslation( stripped );
	    m->setDanger( m->finished() &&
			  danger(m->sourceText(), m->translation(),
				 !m->danger()) );
	    tor.insert( m->message() );
	    if ( !dirty ) {
		dirty = TRUE;
		updateCaption();
	    }
	}
    }
}

void TrWindow::updateFinished( bool finished )
{
    QListViewItem *item = lv->currentItem();
    if ( item != 0 && item->parent() != 0 ) {
	MessageLVI *m = (MessageLVI *) item;
	if ( finished != m->finished() ) {
	    numFinished += finished ? +1 : -1;
	    updateProgress();
	    m->setFinished( finished );
	    bool oldDanger = m->danger();
	    m->setDanger( m->finished() &&
			  danger(m->sourceText(), m->translation(),
				 !oldDanger) );
	    if ( !oldDanger && m->danger() )
		qApp->beep();
	    tor.insert( m->message() );
	    if ( !dirty ) {
		dirty = TRUE;
		updateCaption();
	    }
	}
    }
}

void TrWindow::toggleFinished( QListViewItem *item, const QPoint& /* p */,
			       int column )
{
    if ( item != 0 && item->parent() != 0 && column == 1 ) {
	MessageLVI *m = (MessageLVI *) item;
	if ( m->message().type() != MetaTranslatorMessage::Obsolete ) {
	    setCurrentLVI( m );
	    me->setFinished( !m->finished() );
	}
    }
}

void TrWindow::next()
{
    /*
      First, if no message is selected, select the nearest one.
    */
    QListViewItem *item = lv->currentItem();
    if ( item == 0 ) {
	item = lv->firstChild();
	if ( item == 0 ) {
	    statusBar()->message( tr("No item to translate."), MessageMS );
	    qApp->beep();
	    return;
	}
    }
    if ( item->parent() == 0 ) {
	item = ((ContextLVI *) item)->firstChild();
	if ( item == 0 ) {
	    statusBar()->message( tr("This is impossible."), MessageMS );
	    qApp->beep();
	    return;
	} else if ( !((MessageLVI *) item)->finished() ) {
	    setCurrentLVI( item );
	    return;
	}
    }

    MessageLVI *m = (MessageLVI *) item;
    MessageLVI *n;
    ContextLVI *p = (ContextLVI *) item->parent();
    ContextLVI *q;

    /*
      Usual case:  Find the next Unfinished sibling within the same context.
    */
    m = (MessageLVI *) m->nextSibling();
    n = m;
    do {
	if ( n == 0 )
	    n = (MessageLVI *) p->firstChild();
	if ( !n->finished() ) {
	    setCurrentLVI( n );
	    return;
	}
	n = (MessageLVI *) n->nextSibling();
    } while ( n != m );

    /*
      If all siblings are Finished or Obsolete, look for the first Unfinished
      context.
    */
    p = (ContextLVI *) p->nextSibling();
    q = p;
    do {
	if ( q == 0 )
	    q = (ContextLVI *) lv->firstChild();
	if ( !q->finished() ) {
	    n = (MessageLVI *) q->firstChild();
	    while ( n->finished() )
		n = (MessageLVI *) n->nextSibling();
	    lv->setOpen( item->parent(), FALSE );
	    setCurrentLVI( n );
	    return;
	}
	q = (ContextLVI *) q->nextSibling();
    } while ( q != p );

    /*
      If no Unfinished message is left, the user has finished her job.  We
      congratulate her with this ringing bell.
    */
    statusBar()->message( tr("No unfinished message left."), MessageMS );
    qApp->beep();
}

void TrWindow::findNext( const QString& text, int where, bool matchCase )
{
    findText = text;
    if ( findText.isEmpty() )
	findText = QString( "magicwordthatyoushouldavoid" );
    findWhere = where;
    findMatchCase = matchCase;
    findAgainAct->setEnabled( TRUE );
    findAgain();
}

void TrWindow::revalidate()
{
    ContextLVI *c = (ContextLVI *) lv->firstChild();
    while ( c != 0 ) {
	MessageLVI *m = (MessageLVI *) c->firstChild();
	while ( m != 0 ) {
	    m->setDanger( danger(m->sourceText(), m->translation()) &&
		    m->message().type() == MetaTranslatorMessage::Finished );
	    m = (MessageLVI *) m->nextSibling();
	}
	c = (ContextLVI *) c->nextSibling();
    }
}

void TrWindow::setupImageDict()
{
    if ( imageDict == 0 ) {
	imageDict = new QDict<Embed>( 101 );
	Embed *em;
	for ( em = embed_vec; em->size > 0; em++ )
	    imageDict->insert( em->name, em );
    }
}

QString TrWindow::friendlyString( const QString& str )
{
    QString f = str.lower();
    f.replace( QRegExp(QString("[.,:;!?()-]")), QString(" ") );
    f.replace( QRegExp(QString("&")), QString("") );
    f = f.simplifyWhiteSpace();
    f = f.lower();
    return f;
}

void TrWindow::setupMenuBar()
{
    QMenuBar *m = menuBar();
    QPopupMenu *filep = new QPopupMenu( this );
    QPopupMenu *editp = new QPopupMenu( this );
    phrasep = new QPopupMenu( this );
    closePhraseBookp = new QPopupMenu( this );
    editPhraseBookp = new QPopupMenu( this );
    printPhraseBookp = new QPopupMenu( this );
    QPopupMenu *validationp = new QPopupMenu( this );
    validationp->setCheckable( TRUE );
    QPopupMenu *viewp = new QPopupMenu( this );
    viewp->setCheckable( TRUE );
    QPopupMenu *helpp = new QPopupMenu( this );

    m->insertItem( tr("&File"), filep );
    m->insertItem( tr("&Edit"), editp );
    m->insertItem( tr("&Phrase"), phrasep );
    m->insertItem( tr("V&alidation"), validationp );
    m->insertItem( tr("&View"), viewp );
    m->insertSeparator();
    m->insertItem( tr("&Help"), helpp );

    connect( closePhraseBookp, SIGNAL(activated(int)),
	     this, SLOT(closePhraseBook(int)) );
    connect( editPhraseBookp, SIGNAL(activated(int)),
	     this, SLOT(editPhraseBook(int)) );
    connect( printPhraseBookp, SIGNAL(activated(int)),
	     this, SLOT(printPhraseBook(int)) );

    openAct = new Action( filep, tr("&Open..."), this, SLOT(open()),
			  QAccel::stringToKey(tr("Ctrl+O")) );
    filep->insertSeparator();
    saveAct = new Action( filep, tr("&Save"), this, SLOT(save()),
			  QAccel::stringToKey(tr("Ctrl+S")) );
    saveAsAct = new Action( filep, tr("Save &As..."), this, SLOT(saveAs()) );
    releaseAct = new Action( filep, tr("&Release..."), this, SLOT(release()) );
    filep->insertSeparator();
    printAct = new Action( filep, tr("&Print..."), this, SLOT(print()),
			   QAccel::stringToKey(tr("Ctrl+P")) );
    filep->insertSeparator();
    exitAct = new Action( filep, tr("E&xit"), this, SLOT(close()),
			  QAccel::stringToKey(tr("Ctrl+Q")) );

    undoAct = new Action( editp, tr("&Undo"), me, SLOT(undo()), CTRL + Key_Z );
    undoAct->setEnabled( FALSE );
    connect( me, SIGNAL(undoAvailable(bool)), undoAct, SLOT(setEnabled(bool)) );
    redoAct = new Action( editp, tr("&Redo"), me, SLOT(redo()), CTRL + Key_Y );
    redoAct->setEnabled( FALSE );
    connect( me, SIGNAL(redoAvailable(bool)), redoAct, SLOT(setEnabled(bool)) );
    editp->insertSeparator();
    cutAct = new Action( editp, tr("Cu&t"), me, SLOT(cut()), CTRL + Key_X );
    cutAct->setEnabled( FALSE );
    connect( me, SIGNAL(cutAvailable(bool)), cutAct, SLOT(setEnabled(bool)) );
    copyAct = new Action( editp, tr("&Copy"), me, SLOT(copy()), CTRL + Key_C );
    copyAct->setEnabled( FALSE );
    connect( me, SIGNAL(copyAvailable(bool)), copyAct, SLOT(setEnabled(bool)) );
    pasteAct = new Action( editp, tr("&Paste"), me, SLOT(paste()),
			   CTRL + Key_V );
    pasteAct->setEnabled( FALSE );
    connect( me, SIGNAL(pasteAvailable(bool)),
	     pasteAct, SLOT(setEnabled(bool)) );
    deleteAct = new Action( editp, tr("&Delete"), me, SLOT(del()), Key_Delete );
    deleteAct->setEnabled( FALSE );
    selectAllAct = new Action( editp, tr("Select &All"), me, SLOT(selectAll()),
			       QAccel::stringToKey(tr("Ctrl+A")) );
    selectAllAct->setEnabled( FALSE );
    editp->insertSeparator();
    findAct = new Action( editp, tr("&Find..."), this, SLOT(find()),
			  QAccel::stringToKey(tr("Ctrl+F")) );
    findAct->setEnabled( FALSE );
    findAgainAct = new Action( editp, tr("Find &Next"),
			  this, SLOT(findAgain()), Key_F3 );
    findAgainAct->setEnabled( FALSE );

    newPhraseBookAct = new Action( phrasep, tr("&New Phrase Book..."),
				   this, SLOT(newPhraseBook()) );
    openPhraseBookAct = new Action( phrasep, tr("&Open Phrase Book..."),
				    this, SLOT(openPhraseBook()),
				    QAccel::stringToKey(tr("Ctrl+B")) );
    closePhraseBookId = phrasep->insertItem( tr("&Close Phrase Book"),
					     closePhraseBookp );
    phrasep->insertSeparator();
    editPhraseBookId = phrasep->insertItem( tr("&Edit Phrase Book..."),
					    editPhraseBookp );
    printPhraseBookId = phrasep->insertItem( tr("&Print Phrase Book..."),
					     printPhraseBookp );
    connect( phrasep, SIGNAL(aboutToShow()), this, SLOT(setupPhrase()) );

    acceleratorsAct = new Action( validationp, tr("&Accelerators"),
				  this, SLOT(revalidate()), 0, TRUE );
    acceleratorsAct->setOn( TRUE );
    endingPunctuationAct = new Action( validationp, tr("&Ending Punctuation"),
				       this, SLOT(revalidate()), 0, TRUE );
    endingPunctuationAct->setOn( TRUE );
    phraseMatchesAct = new Action( validationp, tr("&Phrase Matches"),
				   this, SLOT(revalidate()), 0, TRUE );
    phraseMatchesAct->setOn( TRUE );

    revertSortingAct = new Action( viewp, tr("&Revert Sorting"),
				   this, SLOT(revertSorting()) );
    viewp->insertSeparator();
    bigIconsAct = new Action( viewp, tr("&Big Icons"), 0, TRUE );
    connect( bigIconsAct, SIGNAL(toggled(bool)),
	     this, SLOT(setUsesBigPixmaps(bool)) );
    textLabelsAct = new Action( viewp, tr("&Text Labels"), 0, TRUE );
    connect( textLabelsAct, SIGNAL(toggled(bool)),
	     this, SLOT(setUsesTextLabel(bool)) );

    overviewAct = new Action( helpp, tr("&Overview..."),
			      this, SLOT(overview()) );
    aboutAct = new Action( helpp, tr("&About..."), this, SLOT(about()),
			   Key_F1 );
    aboutQtAct = new Action( helpp, tr("About &Qt..."), this, SLOT(aboutQt()) );
    helpp->insertSeparator();
    whatsThisAct = new Action( helpp, tr("&What's This?"),
			       this, SLOT(whatsThis()), SHIFT + Key_F1 );

    openAct->setWhatsThis( tr("Open a Qt message file (TS file) for"
			      " editing.") );
    saveAct->setWhatsThis( tr("Save changes made to this Qt message file.") );
    saveAsAct->setWhatsThis( tr("Save changes made to this Qt message file into"
				" a new file.") );
    releaseAct->setWhatsThis( tr("Create a Qt message file suitable for end use"
				 " from the current message file.") );
    printAct->setWhatsThis( tr("Print a list of all the messages in the current"
			       " Qt message file.") );
    exitAct->setWhatsThis( tr("Close this window and exit.") );

    undoAct->setWhatsThis( tr("Undo the last editing operation performed on the"
			      " translation.") );
    redoAct->setWhatsThis( tr("Redo an undone editing operation performed on"
			      " the translation.") );
    cutAct->setWhatsThis( tr("Copy the selected translation text to the"
			     " clipboard and deletes it.") );
    copyAct->setWhatsThis( tr("Copy the selected translation text to the"
			      " clipboard.") );
    pasteAct->setWhatsThis( tr("Paste the clipboard text into the"
			       " translation.") );
    deleteAct->setWhatsThis( tr("Delete the selection or the character on the"
				" right of the cursor.") );
    selectAllAct->setWhatsThis( tr("Select the whole translation text.") );
    findAct->setWhatsThis( tr("Search for some text in the message file.") );
    findAgainAct->setWhatsThis( tr("Continue the search where it was left.") );

    newPhraseBookAct->setWhatsThis( tr("Create a new phrase book.") );
    openPhraseBookAct->setWhatsThis( tr("Open a phrase book to assist"
					" translation.") );
    acceleratorsAct->setWhatsThis( tr("Enable or disable coherence checks of"
				      " accelerators.") );
    endingPunctuationAct->setWhatsThis( tr("Enable or disable coherence checks"
					   " of ending punctuation.") );
    phraseMatchesAct->setWhatsThis( tr("Enable or disable checking that phrase"
				       " suggestions are used.") );

    revertSortingAct->setWhatsThis( tr("Sort the items back in the same order"
				       " as in the message file.") );
    bigIconsAct->setWhatsThis( tr("Enable or disable use of big icons in"
				  " toolbars.") );
    textLabelsAct->setWhatsThis( tr("Enable or disable use of text labels in"
				    " toolbars.") );

    overviewAct->setWhatsThis( tr("Display an introduction to %1.")
			       .arg(tr("Qt Linguist")) );
    aboutAct->setWhatsThis( tr("Display information about %1.")
			    .arg(tr("Qt Linguist")) );
    aboutQtAct->setWhatsThis( tr("Display information about the Qt toolkit by"
				 " Trolltech.") );
    whatsThisAct->setWhatsThis( tr("Enter What's This? mode.") );
}

void TrWindow::setupToolBars()
{
    QToolBar *filet = new QToolBar( tr("File"), this );
    QToolBar *editt = new QToolBar( tr("Edit"), this );
    QToolBar *validationt = new QToolBar( tr("Validation"), this );
    QToolBar *helpt = new QToolBar( tr("Help"), this );

    openAct->addToToolbar( filet, tr("Open"), "fileopen.xpm" );
    saveAct->addToToolbar( filet, tr("Save"), "filesave.xpm" );
    printAct->addToToolbar( filet, tr("Print"), "print.xpm" );
    filet->addSeparator();
    openPhraseBookAct->addToToolbar( filet, tr("Open Phrase Book"),
				     "book.xpm" );

    undoAct->addToToolbar( editt, tr("Undo"), "undo.xpm" );
    redoAct->addToToolbar( editt, tr("Redo"), "redo.xpm" );
    editt->addSeparator();
    cutAct->addToToolbar( editt, tr("Cut"), "editcut.xpm" );
    copyAct->addToToolbar( editt, tr("Copy"), "editcopy.xpm" );
    pasteAct->addToToolbar( editt, tr("Paste"), "editpaste.xpm" );
    deleteAct->addToToolbar( editt, tr("Delete"), "editdelete.xpm" );
    editt->addSeparator();
    findAct->addToToolbar( editt, tr("Find"), "search.xpm" );

    acceleratorsAct->addToToolbar( validationt, tr("Accelerators"),
				   "accel.xpm" );
    endingPunctuationAct->addToToolbar( validationt, tr("Punctuation"),
					"endpunct.xpm" );
    phraseMatchesAct->addToToolbar( validationt, tr("Phrases"), "phrase.xpm" );

    whatsThisAct->addToToolbar( helpt, tr("What's This?"), "whatsthis.xpm" );
}

void TrWindow::setCurrentLVI( QListViewItem *item )
{
    QListViewItem *oldParent = lv->currentItem();
    QListViewItem *newParent = item;
    if ( oldParent != 0 && oldParent->parent() != 0 )
	oldParent = oldParent->parent();
    if ( newParent->parent() != 0 )
	newParent = newParent->parent();

    if ( newParent != oldParent ) {
	lv->setOpen( oldParent, FALSE );
	lv->setOpen( newParent, TRUE );
	lv->ensureItemVisible( newParent );
    }
    lv->ensureItemVisible( item );
    lv->setSelected( item, TRUE );
}

QString TrWindow::friendlyPhraseBookName( int k )
{
    return QFileInfo( phraseBookNames[k] ).fileName();
}

bool TrWindow::openPhraseBook( const QString& name )
{
    PhraseBook *pb = new PhraseBook;
    if ( !pb->load(name) ) {
	QMessageBox::warning( this, tr("Qt Linguist"),
			      tr("Cannot read from phrase book '%1'.")
			      .arg(name) );
	return FALSE;
    }

    int index = (int) phraseBooks.count();
    phraseBooks.append( pb );
    phraseBookNames.append( name );
    int id = closePhraseBookp->insertItem( friendlyPhraseBookName(index) );
    closePhraseBookp->setWhatsThis( id, tr("Close this phrase book.") );
    id = editPhraseBookp->insertItem( friendlyPhraseBookName(index) );
    editPhraseBookp->setWhatsThis( id, tr("Allow you to add, modify, or delete"
					  " phrases of this phrase book.") );
    id = printPhraseBookp->insertItem( friendlyPhraseBookName(index) );
    printPhraseBookp->setWhatsThis( id, tr("Print the entries of the phrase"
					   " book.") );
    updatePhraseDict();
    return TRUE;
}

bool TrWindow::savePhraseBook( const QString& name, const PhraseBook& pb )
{
    if ( !pb.save(name) ) {
	QMessageBox::warning( this, tr("Qt Linguist"),
			      tr("Cannot create phrase book '%1'.")
			      .arg(name) );
	return FALSE;
    }
    return TRUE;
}

void TrWindow::updateProgress()
{
    if ( numNonobsolete == 0 )
	progress->setText( QString("        ") );
    else
	progress->setText( QString(" %1/%2 ").arg(numFinished)
			   .arg(numNonobsolete) );
    me->setNextEnabled( numFinished != numNonobsolete );
}

void TrWindow::updatePhraseDict()
{
    QListIterator<PhraseBook> pb = phraseBooks;
    PhraseBook::Iterator p;
    PhraseBook *ent;
    phraseDict.clear();
    while ( pb.current() != 0 ) {
	for ( p = (*pb)->begin(); p != (*pb)->end(); ++p ) {
	    QString f = friendlyString( (*p).source() );
	    if ( f.length() > 0 ) {
		f = QStringList::split( QChar(' '), f ).first();
		ent = phraseDict.find( f );
		if ( ent == 0 ) {
		    ent = new PhraseBook;
		    phraseDict.insert( f, ent );
		}
		ent->append( *p );
	    }
	}
	++pb;
    }
    revalidate();
}

PhraseBook TrWindow::getPhrases( const QString& source )
{
    PhraseBook phrases;
    QString f = friendlyString( source );
    QStringList lookupWords = QStringList::split( QChar(' '), f );
    QStringList::Iterator w;
    PhraseBook::Iterator p;

    for ( w = lookupWords.begin(); w != lookupWords.end(); ++w ) {
	PhraseBook *ent = phraseDict.find( *w );
	if ( ent != 0 ) {
	    for ( p = ent->begin(); p != ent->end(); ++p ) {
		if ( f.find(friendlyString((*p).source())) >= 0 )
		    phrases.append( *p );
	    }
	}
    }
    return phrases;
}

bool TrWindow::danger( const QString& source, const QString& translation,
		       bool verbose )
{
    if ( acceleratorsAct->isOn() ) {
	int sk = QAccel::shortcutKey( source );
	int tk = QAccel::shortcutKey( translation );
	if ( sk == 0 && tk != 0 ) {
	    if ( verbose )
		statusBar()->message( tr("Accelerator possibly superfluous in"
					 " translation."), MessageMS );
	    return TRUE;
	} else if ( sk != 0 && tk == 0 ) {
	    if ( verbose )
		statusBar()->message( tr("Accelerator possibly missing in"
					  " translation."), MessageMS );
	    return TRUE;
	}
    }
    if ( endingPunctuationAct->isOn() ) {
	if ( ending(source) != ending(translation) ) {
	    if ( verbose )
		statusBar()->message( tr("Translation does not end with the"
					 " same punctuation as the source"
					 " text."), MessageMS );
	    return TRUE;
	}
    }
    if ( phraseMatchesAct->isOn() ) {
	QString fsource = friendlyString( source );
	QString ftranslation = friendlyString( translation );
	QStringList lookupWords = QStringList::split( QChar(' '), fsource );
	QStringList::Iterator w;
	PhraseBook::Iterator p;

	for ( w = lookupWords.begin(); w != lookupWords.end(); ++w ) {
	    PhraseBook *ent = phraseDict.find( *w );
	    if ( ent != 0 ) {
		for ( p = ent->begin(); p != ent->end(); ++p ) {
		    if ( fsource.find(friendlyString((*p).source())) < 0 ||
			 ftranslation.find(friendlyString((*p).target())) >= 0 )
			break;
		}
		if ( p == ent->end() ) {
		    if ( verbose )
			statusBar()->message( tr("A phrase book suggestion for"
						 " '%1' was ignored.")
						 .arg(*w), MessageMS );
		    return TRUE;
		}
	    }
	}
    }
    if ( verbose )
	statusBar()->clear();
    return FALSE;
}
