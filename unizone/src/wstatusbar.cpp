#include "wstatusbar.h"

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

	setMaximumHeight(32);
}

WStatusBar::~WStatusBar()
{
	delete [] fText;
}

void
WStatusBar::setText(QString text, unsigned int index)
{
	if ((index >= 0) && (index < fColumns))
		fText[index]->setText(text);
}

QString
WStatusBar::text(unsigned int index)
{
	QString ret = QString::null;
	if ((index >= 0) && (index < fColumns))
		ret = fText[index]->text();
	return ret;
}