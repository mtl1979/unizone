#include "wstatusbar.h"

WStatusBar::WStatusBar(QWidget * parent, const char * name)
: QStatusBar(parent, name)
{
	fText = new QLabel *[3];
	CHECK_PTR(fText);
	// Initialize array elements
	fText[0] = new QLabel(this);
	CHECK_PTR(fText[0]);
	fText[0]->setAlignment(AlignCenter);
	//
	fText[1] = new QLabel(this);
	CHECK_PTR(fText[1]);
	fText[1]->setAlignment(AlignCenter);
	//
	fText[2] = new QLabel(this);
	CHECK_PTR(fText[2]);
	fText[2]->setAlignment(AlignCenter);

	// Add elements to status bar
	addWidget(fText[0], 1);
	addWidget(fText[1], 1);
	addWidget(fText[2], 1);
}

WStatusBar::~WStatusBar()
{
	delete [] fText;
}

void
WStatusBar::setText(QString text, int index)
{
	if ((index >= 0) && (index <= 2))
		fText[index]->setText(text);
}

QString
WStatusBar::text(int index)
{
	QString ret = QString::null;
	if ((index >= 0) && (index <= 2))
		ret = fText[index]->text();
	return ret;
}