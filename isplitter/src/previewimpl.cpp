#include "previewimpl.h"
#include "mainwindowimpl.h"
#include "platform.h"

#include <qapplication.h>
#include <qpushbutton.h>
#include <qfileinfo.h>
#include <qimage.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qwmatrix.h>
#include <qdragobject.h>

static const unsigned char erase_xpm_data[] = {
    0x2f,0x2a,0x20,0x58,0x50,0x4d,0x20,0x2a,0x2f,0x0d,0x0a,0x73,0x74,0x61,
    0x74,0x69,0x63,0x20,0x63,0x68,0x61,0x72,0x20,0x2a,0x65,0x72,0x61,0x73,
    0x65,0x5b,0x5d,0x3d,0x7b,0x0d,0x0a,0x22,0x31,0x36,0x20,0x31,0x36,0x20,
    0x32,0x20,0x31,0x22,0x2c,0x0d,0x0a,0x22,0x23,0x20,0x63,0x20,0x23,0x30,
    0x30,0x30,0x30,0x30,0x30,0x22,0x2c,0x0d,0x0a,0x22,0x2e,0x20,0x63,0x20,
    0x23,0x66,0x66,0x66,0x66,0x66,0x66,0x22,0x2c,0x0d,0x0a,0x22,0x2e,0x2e,
    0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,
    0x22,0x2c,0x0d,0x0a,0x22,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x23,
    0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x22,0x2c,0x0d,0x0a,0x22,0x2e,0x2e,
    0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,
    0x22,0x2c,0x0d,0x0a,0x22,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x23,
    0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x22,0x2c,0x0d,0x0a,0x22,0x2e,0x2e,
    0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,
    0x22,0x2c,0x0d,0x0a,0x22,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x23,
    0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x22,0x2c,0x0d,0x0a,0x22,0x2e,0x2e,
    0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,
    0x22,0x2c,0x0d,0x0a,0x22,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x23,
    0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x22,0x2c,0x0d,0x0a,0x22,0x23,0x23,
    0x23,0x23,0x23,0x23,0x23,0x23,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,
    0x22,0x2c,0x0d,0x0a,0x22,0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x2e,
    0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x22,0x2c,0x0d,0x0a,0x22,0x23,0x23,
    0x23,0x23,0x23,0x23,0x23,0x23,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,
    0x22,0x2c,0x0d,0x0a,0x22,0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x2e,
    0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x22,0x2c,0x0d,0x0a,0x22,0x23,0x23,
    0x23,0x23,0x23,0x23,0x23,0x23,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,
    0x22,0x2c,0x0d,0x0a,0x22,0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x2e,
    0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x22,0x2c,0x0d,0x0a,0x22,0x23,0x23,
    0x23,0x23,0x23,0x23,0x23,0x23,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,
    0x22,0x2c,0x0d,0x0a,0x22,0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x2e,
    0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x2e,0x22,0x7d,0x3b,0x0d,0x0a
};

Preview::Preview(QWidget* parent, const char* name, WFlags fl)
:QWidget(parent, name, fl)
{
    if ( !name )
	setName( "PreviewBase" );
    resize( 596, 480 ); 
    setCaption( tr( "Preview" ) );

    GridLayout = new QGridLayout( this ); 
    GridLayout->setGeometry( QRect( 0, 0, 596, 480 ) ); 
    GridLayout->setSpacing( 0 );
    GridLayout->setMargin( 0 );

    SaveButton = new QPushButton( this, "SaveButton" );
    SaveButton->setText( tr( "Save" ) );

    GridLayout->addWidget( SaveButton, 0, 2 );

	PreviewWidget = new QWidget(this, "PreviewWidget");

    pxlPreview = new QLabel( PreviewWidget, "pxlPreview" );
    pxlPreview->setMinimumSize( QSize( 32, 32 ) );
    pxlPreview->setFrameShape( QLabel::NoFrame );
    pxlPreview->setMargin( 0 );
    pxlPreview->setScaledContents( FALSE );
	pxlPreview->setAlignment(AlignVCenter | AlignHCenter);
	pxlPreview->installEventFilter(this);
	pxlPreview->setBackgroundMode(NoBackground);
	pxlPreview->hide();

	//
	// Create erase pixmap
	//
	QPixmap erase;
	if (erase.loadFromData(erase_xpm_data, sizeof(erase_xpm_data), "XPM"))
		PreviewWidget->setBackgroundPixmap(erase);
	//
    GridLayout->addMultiCellWidget( PreviewWidget, 1, 1, 0, 2 );

	// Center
	int pw = (PreviewWidget->width() - pxlPreview->width()) / 2;
	int ph = (PreviewWidget->height() - pxlPreview->height()) / 2;
	pxlPreview->move(pw, ph);

    
	PreviewButton = new QPushButton( this, "PreviewButton" );
    PreviewButton->setText( tr( "Preview" ) );

    GridLayout->addWidget( PreviewButton, 0, 0 );
	connect(PreviewButton, SIGNAL(clicked()), this, SLOT(PreviewImage()));
	connect(SaveButton, SIGNAL(clicked()), this, SLOT(Save()));
	Splitter = NULL;
	pixPreview = NULL;
	image = NULL;

	dragging = false;
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
	if (img)
	{
		image = img;
		PreviewButton->setEnabled(true);
	}
}

template <typename T> 
bool clamp(T & a, const T b, const T c)
{
	bool ret = false;
	if (a < b) 
	{
		a = b;
		ret = true;
	}
	else if (a > c)
	{
		a = c;
		ret = true;
	}
	return ret;
}

void
Preview::scalePixmap(double oldw, double oldh, int &width, int &height)
{
	double ratio = oldw / oldh;
	double neww = PreviewWidget->height() * ratio;
	if (neww > PreviewWidget->width())
	{
		width = PreviewWidget->width();
		double dh = PreviewWidget->width() / ratio;
		height = lrint(dh);
	}
	else
	{
		width = lrint(neww);
		height = PreviewWidget->height();
	}
}

void
Preview::PreviewImage()
{
	if (!image)
		return;

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

	double imageRotate = Splitter->ImageRotate->text().toDouble(&valid);
	if (!valid)
		return;

	double imageScale = Splitter->ImageScale->text().toDouble(&valid);
	if (!valid)
		return;

	//
	//  Make sure we don't get 'divide by zero' error
	//
		
	if (collageSizeX == 0) 
	{ 
		collageSizeX = 1;
		Splitter->CollageSizeX->setText("1");
	}
		
	if (collageSizeY == 0) 
	{
		collageSizeY = 1;
		Splitter->CollageSizeY->setText("1");
	}
		
	if (clamp(collageIndexX, 0, collageSizeX - 1))
		Splitter->OffsetIndexX->setText(QString::number(collageIndexX));
		
	if (clamp(collageIndexY, 0, collageSizeY - 1))
		Splitter->OffsetIndexY->setText(QString::number(collageIndexY));
		
	//
	// Calculate subimage dimensions
	//
	
	originalWidth -= collageOffsetTopX + collageOffsetBottomX;
	originalHeight -= collageOffsetTopY + collageOffsetBottomY;
	int subOffsetX = (originalWidth / collageSizeX) * collageIndexX + imageOffsetTopX + collageOffsetTopX;
	int subOffsetY = (originalHeight / collageSizeY) * collageIndexY + imageOffsetTopY + collageOffsetTopY;
	int subWidth = (originalWidth / collageSizeX) - imageOffsetTopX - imageOffsetBottomX;
	int subHeight = (originalHeight / collageSizeY) - imageOffsetTopY - imageOffsetBottomY;
		
	// Rotate original
		
	QImage timg;
	{
		if (imageRotate == 0)
		{
			timg = *image;
		}
		else
		{
			QPixmap *pm = new QPixmap(image->size());
			if (pm)
			{
				pm->convertFromImage(*image);
				QWMatrix wm;
				wm.rotate(imageRotate);
				timg = pm->xForm(wm);
			}
			delete pm;
		}
	}
		
	//
	// Generate subimage
	//
		
	QImage imgPreview = timg.copy(subOffsetX, subOffsetY, subWidth, subHeight, 0);

	double nh, nw;
	if (imageScale != 100.0f)
	{
		nw = ((double) imgPreview.width() * imageScale)/100.0f;
		nh = ((double) imgPreview.height() * imageScale)/100.0f;
	}
	else
	{
		nw = imgPreview.width();
		nh = imgPreview.height();
	}
	pixPreview = new QPixmap(QSize(lrint(nw), lrint(nh)));
	if (pixPreview)
	{
		//
		// Last Step: Draw It!
		//
			
		pxlPreview->show();
		int w, h;
		scalePixmap(nw, nh, w, h);
		if (imageScale != 100.0f)
		{
			QImage simg = imgPreview.smoothScale(lrint(nw), lrint(nh));
			pixPreview->convertFromImage(simg);
		}
		else
		{
			pixPreview->convertFromImage(imgPreview);
		}
		//
		QPixmap tmpPreview;
		{
			// Use temporary pixmap, so we don't save scaled pixmap
			QImage nimg = imgPreview.smoothScale(w, h);
			tmpPreview = nimg;	
		}
		pxlPreview->resize(w, h);
		// Center
		int ww = PreviewWidget->width();
		int wh = PreviewWidget->height();
		int pw = (PreviewWidget->width() - pxlPreview->width()) / 2;
		int ph = (PreviewWidget->height() - pxlPreview->height()) / 2;
		pxlPreview->move(pw, ph);
		pxlPreview->setPixmap( tmpPreview );
		SaveButton->setEnabled(true);
		setCaption(tr("Preview") + " - " + tr("%1 x %2").arg(pixPreview->width()).arg(pixPreview->height()));
	}
}

bool
Preview::eventFilter( QObject *o, QEvent *e )
{
	if (o == pxlPreview)
	{
		switch(e->type())
		{
		case QEvent::MouseButtonPress:
			mousePressEvent((QMouseEvent *) e);
			return true;
		case QEvent::MouseMove:
			mouseMoveEvent((QMouseEvent *) e);
			return true;
		case QEvent::MouseButtonRelease:
			mouseReleaseEvent((QMouseEvent *) e);
			return true;
		case QEvent::Drop:
			dropEvent((QDropEvent *) e);
			return true;
		}
	}
	return false;
}

void
Preview::mousePressEvent(QMouseEvent *e)
{
	if (e->button() & LeftButton)
	{
		qDebug("Started dragging...\n");
		dragging = true;
	}
	QWidget::mousePressEvent(e);
}

void
Preview::mouseMoveEvent(QMouseEvent *e)
{
	if (dragging)
	{
		QPoint currPos = e->pos();
		if ( ( startPos - currPos ).manhattanLength() > QApplication::startDragDistance() )
			startDrag();
	}
	QWidget::mouseMoveEvent(e);
}

void
Preview::mouseReleaseEvent(QMouseEvent *e)
{
	if (dragging)
	{
		qDebug("Stopped dragging...\n");
		dragging = false;
	}
	QWidget::mouseReleaseEvent(e);
}

void 
Preview::startDrag()
{
	if (pixPreview)
	{
		QImage img;
		img = *pixPreview;
		QImageDrag *d = new QImageDrag(img, this);
		d->dragCopy();
	}
}

void
Preview::Save()
{
	PreviewImage();

	if (pixPreview)
	{
		const char * fmt = QImageIO::imageFormat(Splitter->filename());
		QFileInfo info(Splitter->filename());
		QString path = info.dirPath();
		QString base = info.baseName();
		QString ext = info.extension();
		QString newname = path + "/" + base + "_" + Splitter->OffsetIndexX->text() + "_" + Splitter->OffsetIndexY->text() + "." + ext;
		if (!pixPreview->save(newname, fmt))
		{
			// If original plugin doesn't support writing, try jpeg plugin
			//
			bool ret = false;

			if (strcmp(fmt, "JPEG") != 0)
			{
				newname = path + "/" + base + "_" + Splitter->OffsetIndexX->text() + "_" + Splitter->OffsetIndexY->text() + ".jpg";
				ret = pixPreview->save(newname, "JPEG");
			}

			if (!ret)
				QMessageBox::critical(this, tr("Save"), tr("Unable to save output!"));
		}
	}
}

void
Preview::ClearPreview()
{
	SaveButton->setEnabled(false);

	if (pixPreview)
	{
		delete pixPreview;
		pixPreview = NULL;
	}

	{
//		pxlPreview->clear();
		pxlPreview->hide();
	}

	setCaption(tr("Preview"));
}

void
Preview::resizeEvent(QResizeEvent *e)
{
	QSize s = e->size();
	GridLayout->setGeometry(QRect(0, 0, s.width(), s.height()));
	if (pxlPreview->pixmap())
	{
		PreviewImage();
	}
	QWidget::resizeEvent(e);
}

