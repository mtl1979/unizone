#include "previewimpl.h"
#include "mainwindowimpl.h"
#include <qpushbutton.h>
#include <qfileinfo.h>
#include <qimage.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qlayout.h>

Preview::Preview(QWidget* parent, const char* name, WFlags fl)
:PreviewWindow(parent, name, fl)
{
	connect(PreviewButton, SIGNAL(clicked()), this, SLOT(PreviewImage()));
	connect(SaveButton, SIGNAL(clicked()), this, SLOT(Save()));
	Splitter = NULL;
	pixPreview = NULL;
}

Preview::~Preview()
{
}

void
Preview::setOwner(QWidget *owner)
{
	Splitter = dynamic_cast<ImageSplitter *>(owner);
}

void
Preview::ShowImage(QImage *img)
{
	image = img;
	if (image)
		PreviewButton->setEnabled(true);
}

void
Preview::PreviewImage()
{
	ClearPreview();

	int originalWidth = image->width();
	int originalHeight = image->height();
	//
	//
	//
	bool valid = false;

	int collageSizeX = Splitter->CollageSizeX->text().toLong(&valid);
	if (!valid)
		return;

	int collageSizeY = Splitter->CollageSizeY->text().toLong(&valid);
	if (!valid)
		return;

	int collageIndexX = Splitter->OffsetIndexX->text().toLong(&valid);
	if (!valid)
		return;

	int collageIndexY = Splitter->OffsetIndexY->text().toLong(&valid);
	if (!valid)
		return;
	//
	//
	//
	int collageOffsetTopX = Splitter->CollageOffsetTopX->text().toLong(&valid);
	if (!valid)
		return;

	int collageOffsetTopY = Splitter->CollageOffsetTopY->text().toLong(&valid);
	if (!valid)
		return;

	int collageOffsetBottomX = Splitter->CollageOffsetBottomX->text().toLong(&valid);
	if (!valid)
		return;

	int collageOffsetBottomY = Splitter->CollageOffsetBottomY->text().toLong(&valid);
	if (!valid)
		return;
//
	int imageOffsetTopX = Splitter->ImageOffsetTopX->text().toLong(&valid);
	if (!valid)
		return;

	int imageOffsetTopY = Splitter->ImageOffsetTopY->text().toLong(&valid);
	if (!valid)
		return;

	int imageOffsetBottomX = Splitter->ImageOffsetBottomX->text().toLong(&valid);
	if (!valid)
		return;

	int imageOffsetBottomY = Splitter->ImageOffsetBottomY->text().toLong(&valid);
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
Preview::Save()
{
	Preview();

	if (pixPreview)
	{
		const char * fmt = QImageIO::imageFormat(Splitter->filename());
		QFileInfo info(Splitter->filename());
		QString path = info.dirPath();
		QString base = info.baseName();
		QString ext = info.extension();
		QString newname = path + "/" + base + "_" + Splitter->OffsetIndexX->text() + "_" + Splitter->OffsetIndexY->text() + "." + ext;
		pixPreview->save(newname, fmt);
	}
}

void
Preview::ClearPreview()
{
	PreviewButton->setEnabled(false);
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
Preview::resizeEvent(QResizeEvent *e)
{
	QSize s = e->size();
	QWidget * lwidget = dynamic_cast<QWidget *>(Layout23->parent());
	if (lwidget)
	{
		QRect r = lwidget->geometry();
		s.setHeight(s.height() - r.top());

		if (pxlPreview)
		{
			pxlPreview->setMaximumWidth(s.width() - 20);
			pxlPreview->setMaximumHeight(s.height() - 20);
		}
		lwidget->resize(s);
	}
	PreviewWindow::resizeEvent(e);
}
