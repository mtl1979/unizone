#include "mainwindowimpl.h"
#include "previewimpl.h"
#include "menubar.h"
#include "jpegio.h"

#include <qimage.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qfiledialog.h>
#include <qapplication.h>

ImageSplitter::ImageSplitter( QWidget* parent, const char* name, WFlags fl)
: ImageSplitterBase(parent, name, fl)
{
	image = NULL;
	QString filename = QString::null;
	QString lastdir = QString::null;
	menuBar = new MenuBar(this);
	CHECK_PTR(menuBar);

	fPreview = new Preview(NULL);
	CHECK_PTR(fPreview);
	fPreview->setOwner(this);

	// Use our copy of JPEG IO if Qt doesn't have it ;)
#ifdef WIN32
	InitJpegIO();
#endif

	ClearImage();
};

ImageSplitter:: ~ImageSplitter()
{
}

void
ImageSplitter::Load()
{
	fFilename = QFileDialog::getOpenFileName ( lastdir, "*.png;*.bmp;*.xbm;*.xpm;*.pnm;*.jpg;*.jpeg;*.mng;*.gif", this);
	if (!fFilename.isEmpty())
	{
		ClearImage();
		image = new QImage();
		if (image->load(fFilename))
		{
			QPixmap pixCollage;
			pixCollage = *image;
			pxlCollage->setPixmap(pixCollage);
			fPreview->ShowImage(image);
			fPreview->show();
		}
		QFileInfo info(fFilename);
		lastdir = info.dirPath();
	}
}


void
ImageSplitter::ClearImage()
{
	if (image) 
	{
		delete image;
		image = NULL;
	}

	{
		QPixmap empty(pxlCollage->width(), pxlCollage->height());
		empty.fill(Qt::white);
		pxlCollage->setPixmap(empty);
	}
	
	// reset input boxes
	CollageSizeX->setText("0");
	CollageSizeY->setText("0");
	CollageOffsetTopX->setText("0");
	CollageOffsetTopY->setText("0");
	CollageOffsetBottomX->setText("0");
	CollageOffsetBottomY->setText("0");
	//
	OffsetIndexX->setText("0");
	OffsetIndexY->setText("0");
	//
	ImageOffsetTopX->setText("0");
	ImageOffsetTopY->setText("0");
	ImageOffsetBottomX->setText("0");
	ImageOffsetBottomY->setText("0");
	//
	fPreview->ClearPreview();
	fPreview->hide();
}

void
ImageSplitter::Exit()
{
	QApplication::exit(0);
}

void
ImageSplitter::resizeEvent(QResizeEvent *e)
{
	QSize s = e->size();
	QWidget * lwidget = dynamic_cast<QWidget *>(Layout14->parent());
	if (lwidget)
	{
		QRect r = lwidget->geometry();
		s.setHeight(s.height() - r.top());

		if (pxlCollage)
		{
			pxlCollage->setMaximumWidth(s.width() / 2 - 20);
			pxlCollage->setMaximumHeight(s.height());
		}
/*
		if (pxlPreview)
		{
			pxlPreview->setMaximumWidth(s.width() / 2 - 20);
			pxlPreview->setMaximumHeight(s.height() / 3);
		}
*/
//		lwidget->resize(s);
	}
	ImageSplitterBase::resizeEvent(e);
}
