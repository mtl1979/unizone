#include "wstatusbar.h"
#include <qlayout.h>

WStatusBar::WStatusBar(QWidget * parent, const char * name, unsigned int columns)
: QStatusBar(parent, name), fColumns(columns)
{
	fText = new QLabel *[columns];
	CHECK_PTR(fText);
	for (unsigned int i = 0; i < columns; i++)
	{
		// Initialize array element
		fText[i] = new QLabel(this);
		CHECK_PTR(fText[i]);
		fText[i]->setAlignment(AlignCenter);
		//
		// Add elements to status bar
		addWidget(fText[i], 1);
	}

	fLabels = new QString[columns];
	CHECK_PTR(fLabels);

	setMaximumHeight(32);
}

WStatusBar::~WStatusBar()
{
	delete [] fText;
	delete [] fLabels;
}

void
WStatusBar::setText(QString text, unsigned int index)
{
	if ((index >= 0) && (index < fColumns))
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
	if ((index >= 0) && (index < fColumns))
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

QString
WStatusBar::checkText(const QString &text, unsigned int index)
{
		QFontMetrics qfm = fText[index]->fontMetrics();
		int w;
		QWidget * _parent = dynamic_cast<QWidget *>(parent());
		if (_parent) 
			w = _parent->width();
		else
		{
			QLayout * parentLayout = dynamic_cast<QLayout *>(parent());
			if (parentLayout)
				w = parentLayout->geometry().width();
		}
		if (qfm.width(text) > (int) (w / fColumns))
		{
			int i = text.length() - 1;
			while ( (qfm.width(text.left(i) + "...") > (int) (w / fColumns)) && (i > 0)) i--;
			return text.left(i) + "...";
		}
		else
		{
			return text;
		}

}