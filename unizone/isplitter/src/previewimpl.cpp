#include "previewimpl.h"
#include "mainwindowimpl.h"

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

    pxlPreview = new QLabel( this, "pxlPreview" );
    pxlPreview->setMinimumSize( QSize( 32, 32 ) );
    QPalette pal;
    QColorGroup cg;
    cg.setColor( QColorGroup::Foreground, black );
    cg.setColor( QColorGroup::Button, QColor( 236, 233, 216) );
    cg.setColor( QColorGroup::Light, white );
    cg.setColor( QColorGroup::Midlight, QColor( 245, 244, 235) );
    cg.setColor( QColorGroup::Dark, QColor( 118, 116, 108) );
    cg.setColor( QColorGroup::Mid, QColor( 157, 155, 143) );
    cg.setColor( QColorGroup::Text, black );
    cg.setColor( QColorGroup::BrightText, white );
    cg.setColor( QColorGroup::ButtonText, black );
    cg.setColor( QColorGroup::Base, white );
    cg.setColor( QColorGroup::Background, white );
    cg.setColor( QColorGroup::Shadow, black );
    cg.setColor( QColorGroup::Highlight, QColor( 49, 106, 197) );
    cg.setColor( QColorGroup::HighlightedText, white );
    pal.setActive( cg );
    cg.setColor( QColorGroup::Foreground, black );
    cg.setColor( QColorGroup::Button, QColor( 236, 233, 216) );
    cg.setColor( QColorGroup::Light, white );
    cg.setColor( QColorGroup::Midlight, QColor( 255, 254, 249) );
    cg.setColor( QColorGroup::Dark, QColor( 118, 116, 108) );
    cg.setColor( QColorGroup::Mid, QColor( 157, 155, 143) );
    cg.setColor( QColorGroup::Text, black );
    cg.setColor( QColorGroup::BrightText, white );
    cg.setColor( QColorGroup::ButtonText, black );
    cg.setColor( QColorGroup::Base, white );
    cg.setColor( QColorGroup::Background, white );
    cg.setColor( QColorGroup::Shadow, black );
    cg.setColor( QColorGroup::Highlight, QColor( 49, 106, 197) );
    cg.setColor( QColorGroup::HighlightedText, white );
    pal.setInactive( cg );
    cg.setColor( QColorGroup::Foreground, QColor( 128, 128, 128) );
    cg.setColor( QColorGroup::Button, QColor( 236, 233, 216) );
    cg.setColor( QColorGroup::Light, white );
    cg.setColor( QColorGroup::Midlight, QColor( 255, 254, 249) );
    cg.setColor( QColorGroup::Dark, QColor( 118, 116, 108) );
    cg.setColor( QColorGroup::Mid, QColor( 157, 155, 143) );
    cg.setColor( QColorGroup::Text, black );
    cg.setColor( QColorGroup::BrightText, white );
    cg.setColor( QColorGroup::ButtonText, QColor( 128, 128, 128) );
    cg.setColor( QColorGroup::Base, white );
    cg.setColor( QColorGroup::Background, white );
    cg.setColor( QColorGroup::Shadow, black );
    cg.setColor( QColorGroup::Highlight, QColor( 49, 106, 197) );
    cg.setColor( QColorGroup::HighlightedText, white );
    pal.setDisabled( cg );
    pxlPreview->setPalette( pal );
    pxlPreview->setFrameShape( QLabel::Box );
    pxlPreview->setMargin( 1 );
    pxlPreview->setScaledContents( TRUE );
	pxlPreview->installEventFilter(this);

    GridLayout->addMultiCellWidget( pxlPreview, 1, 1, 0, 2 );

    PreviewButton = new QPushButton( this, "PreviewButton" );
    PreviewButton->setText( tr( "Preview" ) );

    GridLayout->addWidget( PreviewButton, 0, 0 );
	connect(PreviewButton, SIGNAL(clicked()), this, SLOT(PreviewImage()));
	connect(SaveButton, SIGNAL(clicked()), this, SLOT(Save()));
	Splitter = NULL;
	pixPreview = NULL;

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

	double imageRotate = Splitter->ImageRotate->text().toDouble(&valid);
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
		QPixmap *pm = new QPixmap(image->size());
		if (pm)
		{
			if (imageRotate == 0)
			{
				timg = *image;
			}
			else
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
	pixPreview = new QPixmap(imgPreview.size());
	if (pixPreview)
	{
		pixPreview->convertFromImage(imgPreview);
		
		//
		// Last Step: Draw It!
		//
		
		pxlPreview->setPixmap(*pixPreview);
//		PreviewButton->setEnabled(true);
		SaveButton->setEnabled(true);
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
	Preview();

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
//	PreviewButton->setEnabled(false);
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
	GridLayout->setGeometry(QRect(0, 0, s.width(), s.height()));
	QWidget::resizeEvent(e);
}
