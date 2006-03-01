#include "mainwindowimpl.h"
#include "previewimpl.h"
#include "menubar.h"
#include "platform.h"
#ifdef WIN32
#include "jpegio.h"
#endif

#include <qimage.h>
#include <qdragobject.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qfiledialog.h>
#include <qapplication.h>
#include <qfile.h>
#include <qstring.h>
#include <qtabwidget.h>

ImageSplitter::ImageSplitter( QWidget* parent, const char* name, WFlags fl)
: ImageSplitterBase(parent, name, fl | WMouseNoMask)
{
	dragging = false;

	image = NULL;

	menuBar = new MenuBar(this);
	CHECK_PTR(menuBar);

	LoadSettings();

	fPreview = new Preview(NULL);
	CHECK_PTR(fPreview);
	fPreview->setOwner(this);

	setAcceptDrops(true);
	pxlCollage->installEventFilter(this);

	// Use our copy of JPEG IO if Qt doesn't have it ;)
#if (QT_VERSION < 0x030000) || defined(QT_NO_IMAGEIO_JPEG)
	InitJpegIO();
#endif

	ClearImage();
};

void 
ImageSplitter::dragEnterEvent(QDragEnterEvent* event)
{
    event->accept( QUriDrag::canDecode(event));
	ImageSplitterBase::dragEnterEvent(event);
}

bool
ImageSplitter::eventFilter( QObject *o, QEvent *e )
{
	if (o == pxlCollage)
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
ImageSplitter::dropEvent(QDropEvent* event)
{
    QStringList list;
	
    if ( QUriDrag::decodeLocalFiles(event, list) ) {
		QString filename = list[0];
		Load(filename);
    }
}

void
ImageSplitter::mousePressEvent(QMouseEvent *e)
{
	if (e->button() & LeftButton)
	{
		qDebug("Started dragging...\n");
		dragging = true;
	}
	ImageSplitterBase::mousePressEvent(e);
}

void
ImageSplitter::mouseMoveEvent(QMouseEvent *e)
{
	if (dragging)
	{
		QPoint currPos = e->pos();
		if ( ( startPos - currPos ).manhattanLength() > QApplication::startDragDistance() )
			startDrag();
	}
	ImageSplitterBase::mouseMoveEvent(e);
}

void
ImageSplitter::mouseReleaseEvent(QMouseEvent *e)
{
	if (dragging)
	{
		qDebug("Stopped dragging...\n");
		dragging = false;
	}
	ImageSplitterBase::mouseReleaseEvent(e);
}

void 
ImageSplitter::startDrag()
{
	if (fFilename != QString::null)
	{
		QStringList list;
		list.append(fFilename);
		QUriDrag *d = new QUriDrag(this);
		d->setFilenames(list);
		d->dragCopy();
	}
}

void
ImageSplitter::LoadSettings()
{
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
}

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
	QString filename = QFileDialog::getOpenFileName ( lastdir, "*.png;*.bmp;*.xbm;*.xpm;*.pnm;*.jpg;*.jpeg;*.mng;*.gif", this);
	if (!filename.isEmpty())
	{
		Load(filename);
	}
}

void
ImageSplitter::scalePixmap(const QPixmap * image, int &width, int &height)
{
	int oldw = image->width();
	int oldh = image->height();
	double ratio = (double) oldw / (double) oldh;
	double neww = pxlCollage->height() * ratio;
	if (neww > pxlCollage->width())
	{
		width = pxlCollage->width();
		double dh = pxlCollage->width() / ratio;
		height = lrint(dh);
	}
	else
	{
		width = lrint(neww);
		height = pxlCollage->height();
	}
}

void
ImageSplitter::scaleImage(const QImage * image, int &width, int &height)
{
	int oldw = image->width();
	int oldh = image->height();
	double ratio = (double) oldw / (double) oldh;
	double neww = pxlCollage->height() * ratio;
	if (neww > pxlCollage->width())
	{
		width = pxlCollage->width();
		double dh = pxlCollage->width() / ratio;
		height = lrint(dh);
	}
	else
	{
		width = lrint(neww);
		height = pxlCollage->height();
	}
}

void
ImageSplitter::Load(const QString &filename)
{
	QImage * img = new QImage();
	if (img->load(filename))
	{
		ClearImage();

		image = img;
		fFilename = filename;

		int w, h;
		scaleImage(image, w, h);
		QImage nimg = image->smoothScale(w, h);
		QPixmap pixCollage;
		pixCollage.convertFromImage(nimg);

		pxlCollage->setPixmap(pixCollage);
		fPreview->ShowImage(image);
		fPreview->show();

		QFileInfo info(fFilename);
		lastdir = info.dirPath();

		setCaption( tr("Image Splitter") + " - " + QDir::convertSeparators(fFilename) );

		TabWidget2->showPage(tab);
	}
	else
		delete img;
}

void
ImageSplitter::ClearImage()
{
	if (image) 
	{
		delete image;
		image = NULL;
	}

	pxlCollage->clear();
	
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
	ImageRotate->setText("0");
	ImageScale->setText("100");
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
	ImageSplitterBase::resizeEvent(e);
}
