#include "mainwindowimpl.h"
#include "menubar.h"
#include "jpegio.h"

#include <qimage.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qfiledialog.h>
#include <qpushbutton.h>
#include <qfileinfo.h>
#include <qapplication.h>

ImageSplitter::ImageSplitter( QWidget* parent, const char* name, WFlags fl)
: ImageSplitterBase(parent, name, fl)
{
	image = NULL;
	pixPreview = NULL;
	QString filename = QString::null;
	QString lastdir = QString::null;
	menuBar = new MenuBar(this);
	CHECK_PTR(menuBar);

	// Use our copy of JPEG IO if Qt doesn't have it ;)
#ifdef WIN32
	InitJpegIO();
#endif
	connect(PreviewButton, SIGNAL(clicked()), this, SLOT(Preview()));
	connect(SaveButton, SIGNAL(clicked()), this, SLOT(Save()));

	ClearImage();
};

ImageSplitter:: ~ImageSplitter()
{
}

void
ImageSplitter::Load()
{
	filename = QFileDialog::getOpenFileName ( lastdir, "*.png;*.bmp;*.xbm;*.xpm;*.pnm;*.jpg;*.jpeg;*.mng;*.gif", this);
	if (!filename.isEmpty())
	{
		ClearImage();
		image = new QImage();
		if (image->load(filename))
		{
			QPixmap pixCollage;
			pixCollage = *image;
			pxlCollage->setPixmap(pixCollage);
			PreviewButton->setEnabled(true);
		}
		QFileInfo info(filename);
		lastdir = info.dirPath();
	}
}

void
ImageSplitter::Preview()
{
	ClearPreview();

	int originalWidth = image->width();
	int originalHeight = image->height();
	//
	//
	//
	bool valid = false;

	int collageSizeX = CollageSizeX->text().toLong(&valid);
	if (!valid)
		return;

	int collageSizeY = CollageSizeY->text().toLong(&valid);
	if (!valid)
		return;

	int collageIndexX = OffsetIndexX->text().toLong(&valid);
	if (!valid)
		return;

	int collageIndexY = OffsetIndexY->text().toLong(&valid);
	if (!valid)
		return;
	//
	//
	//
	int collageOffsetTopX = CollageOffsetTopX->text().toLong(&valid);
	if (!valid)
		return;

	int collageOffsetTopY = CollageOffsetTopY->text().toLong(&valid);
	if (!valid)
		return;

	int collageOffsetBottomX = CollageOffsetBottomX->text().toLong(&valid);
	if (!valid)
		return;

	int collageOffsetBottomY = CollageOffsetBottomY->text().toLong(&valid);
	if (!valid)
		return;
//
	int imageOffsetTopX = ImageOffsetTopX->text().toLong(&valid);
	if (!valid)
		return;

	int imageOffsetTopY = ImageOffsetTopY->text().toLong(&valid);
	if (!valid)
		return;

	int imageOffsetBottomX = ImageOffsetBottomX->text().toLong(&valid);
	if (!valid)
		return;

	int imageOffsetBottomY = ImageOffsetBottomY->text().toLong(&valid);
	if (!valid)
		return;

	//
	//  Make sure we don't get 'divide by zero' error
	//

	if (collageSizeX == 0) collageSizeX = 1;
	if (collageSizeY == 0) collageSizeY = 1;

	if ((collageIndexX < 0) || (collageIndexX >= collageSizeX))
		return;

	if ((collageIndexY < 0) || (collageIndexY >= collageSizeY))
		return;
	
	//
	// Calculate subimage dimensions
	//

	originalWidth -= collageOffsetTopX + collageOffsetBottomX;
	originalHeight -= collageOffsetTopY + collageOffsetBottomY;
	int subOffsetX = (originalWidth / collageSizeX) * collageIndexX + imageOffsetTopX + collageOffsetTopX;
	int subOffsetY = (originalHeight / collageSizeY) * collageIndexY + imageOffsetTopY + collageOffsetTopY;
	int subWidth = (originalWidth / collageSizeX) - imageOffsetTopX - imageOffsetBottomX;
	int subHeight = (originalHeight / collageSizeY) - imageOffsetTopY - imageOffsetBottomY;

	//
	// Generate subimage
	//

	QImage imgPreview = image->copy(subOffsetX, subOffsetY, subWidth, subHeight);
	pixPreview = new QPixmap(imgPreview.size());
	if (pixPreview)
	{
		pixPreview->convertFromImage(imgPreview);
		
		//
		// Last Step: Draw It!
		//
		
		pxlPreview->setPixmap(*pixPreview);
		PreviewButton->setEnabled(true);
		SaveButton->setEnabled(true);
	}
}

void
ImageSplitter::Save()
{
	Preview();

	if (pixPreview)
	{
		const char * fmt = QImageIO::imageFormat(filename);
		QFileInfo info(filename);
		QString path = info.dirPath();
		QString base = info.baseName();
		QString ext = info.extension();
		QString newname = path + "/" + base + "_" + OffsetIndexX->text() + "_" + OffsetIndexY->text() + "." + ext;
		pixPreview->save(newname, fmt);
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
	PreviewButton->setEnabled(false);
	ClearPreview();
}

void
ImageSplitter::ClearPreview()
{
	SaveButton->setEnabled(false);

	if (pixPreview)
	{
		delete pixPreview;
		pixPreview = NULL;
	}

	{
		QPixmap empty(150, 150);
		empty.fill(Qt::white);
		pxlPreview->setPixmap(empty);
	}
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
	QWidget * lwidget = dynamic_cast<QWidget *>(Layout48->parent());
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
	ImageSplitterBase::resizeEvent(e);
}
