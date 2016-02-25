#include "wstatusbar.h"
#include "qglobal.h"
#include <qlayout.h>
#include <QResizeEvent>
#include <QLabel>

WStatusBar::WStatusBar(QWidget * parent, const char * name, unsigned int columns)
: QStatusBar(parent, name), fColumns(columns)
{
	fText = new QLabel *[columns];
	Q_CHECK_PTR(fText);
	for (unsigned int i = 0; i < columns; i++)
	{
		// Initialize array element
		fText[i] = new QLabel(this);
		Q_CHECK_PTR(fText[i]);
		fText[i]->setAlignment(Qt::AlignCenter);
		//
		// Add elements to status bar
		addWidget(fText[i], 1);
	}

	fLabels = new QString[columns];
	Q_CHECK_PTR(fLabels);

	setMaximumHeight(32);
}

WStatusBar::~WStatusBar()
{
	delete [] fText;
	delete [] fLabels;
}

void
WStatusBar::setText(const QString & text, unsigned int index)
{
	if (index < fColumns)
	{
		fLabels[index] = text;
		QString t = checkText(text, index);
		fText[index]->setText(t);
	}
}

QString
WStatusBar::text(unsigned int index)
{
	QString ret = QString::null;
	if (index < fColumns)
		ret = fLabels[index];
	return ret;
}

void
WStatusBar::resizeEvent(QResizeEvent *e)
{
	QStatusBar::resizeEvent(e);
	static bool inResize = false;
	if (inResize) // Make sure we don't recurse ;)
		return;

	inResize = true;
	for (unsigned int i = 0; i < fColumns; i++)
	{
		QString t = checkText(fLabels[i], i);
		if (t != fText[i]->text())
			fText[i]->setText(t);
	}
	inResize = false;
}

/*
 * Checks if text fits in to the space left for particular label and clips it, if not.
 *
 */

QString
WStatusBar::checkText(const QString &text, unsigned int index)
{
	QFontMetrics qfm = fText[index]->fontMetrics();
	int w = 0;

	QWidget * _parent = dynamic_cast<QWidget *>(parent());
	if (_parent)
	{
		w = _parent->width();
	}
	else
	{
		QLayout * parentLayout = dynamic_cast<QLayout *>(parent());
		if (parentLayout) w = parentLayout->geometry().width();
	}

	int columnWidth = w;
	if (columnWidth > 0) columnWidth /= fColumns;

	if ((qfm.width(text) > columnWidth) && (columnWidth > 0))
	{
		int i = text.length() - 1;
		while (
			(qfm.width(text.left(i) + "...") > columnWidth) &&
			(i > 0)
			)
		{
			i--;
		}
		return text.left(i) + "...";
	}
	else
	{
		return text;
	}
}
