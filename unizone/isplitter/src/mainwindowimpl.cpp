#include "mainwindowimpl.h"
#include "previewimpl.h"
#include "menubar.h"
#ifdef WIN32
#include "jpegio.h"
#endif

#include <qimage.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qfiledialog.h>
#include <qapplication.h>
#include <qfile.h>
#include <qstring.h>

ImageSplitter::ImageSplitter( QWidget* parent, const char* name, WFlags fl)
: ImageSplitterBase(parent, name, fl)
{
	image = NULL;
	QString filename = QString::null;
	lastdir = QString::null;
	QFile qf("isplitter.ini");
	if (qf.open(IO_ReadOnly))
	{
		QByteArray temp(256);
		if (qf.readLine(temp.data(), 255) > 0)
			lastdir = QString::fromUtf8(temp);
		qf.close();
	}
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

void
ImageSplitter::SaveSettings()
{
	QFile qf("isplitter.ini");
	if (qf.open(IO_WriteOnly))
	{
		QCString temp = lastdir.utf8();
		qf.writeBlock(temp, temp.length()); 
		qf.close();
	}
}

ImageSplitter:: ~ImageSplitter()
{
	SaveSettings();
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

		setCaption( fFilename );
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
	
    setCaption( tr( "Image Splitter" ) );

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
	SaveSettings();
	QApplication::exit(0);
}

void
ImageSplitter::resizeEvent(QResizeEvent *e)
{
/*
	QSize s = e->size();
	QWidget * lwidget = dynamic_cast<QWidget *>(Layout27->parent());
	if (lwidget)
	{
		QRect r = lwidget->geometry();
		s.setHeight(s.height() - r.top());

		if (pxlCollage)
		{
			pxlCollage->setMaximumWidth(s.width() / 2 - 20);
			pxlCollage->setMaximumHeight(s.height());
		}
		if (pxlPreview)
		{
			pxlPreview->setMaximumWidth(s.width() / 2 - 20);
			pxlPreview->setMaximumHeight(s.height() / 3);
		}
		lwidget->resize(s);
	}
*/
	ImageSplitterBase::resizeEvent(e);
}
