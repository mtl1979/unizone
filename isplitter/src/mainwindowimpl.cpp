#include "mainwindowimpl.h"
#include "mainwindow.h"
#include "previewimpl.h"
#include "menubar.h"
#include "platform.h"

#include <Q3MainWindow>
#include <qimage.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qfiledialog.h>
#include <qapplication.h>
#include <qfile.h>
#include <qstring.h>
#include <qtabwidget.h>
#include <QByteArray>
#include <QDropEvent>
#include <QResizeEvent>
#include <QPixmap>
#include <QMouseEvent>
#include <QEvent>
#include <QDragEnterEvent> 
#include <QUrl>

ImageSplitter::ImageSplitter( QWidget* parent, const char* name, Qt::WFlags fl)
: Q3MainWindow(parent, name, fl | Qt::WMouseNoMask)
{
	ui = new Ui_ImageSplitterBase();
	ui->setupUi(this);

	dragging = false;

	image = NULL;

	menuBar = new MenuBar(this);
	Q_CHECK_PTR(menuBar);

	LoadSettings();

	fPreview = new Preview(NULL);
	Q_CHECK_PTR(fPreview);
	fPreview->setOwner(this);

	setAcceptDrops(true);
	ui->pxlCollage->installEventFilter(this);

	ClearImage();
};

void 
ImageSplitter::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls())
         event->acceptProposedAction();
	Q3MainWindow::dragEnterEvent(event);
}

bool
ImageSplitter::eventFilter( QObject *o, QEvent *e )
{
	if (o == ui->pxlCollage)
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
	if (event->mimeData()->hasUrls())
	{
		QList<QUrl> urls = event->mimeData()->urls();
		QUrl u = urls.takeFirst();
		qDebug("Drop URL: %S", u.toString().utf16());
		QString file = u.toLocalFile();
		qDebug("Drop: %S", file.utf16());
		Load(file);
	}
}

void
ImageSplitter::mousePressEvent(QMouseEvent *e)
{
	if (e->button() & Qt::LeftButton)
	{
		qDebug("Started dragging...\n");
		dragging = true;
	}
	Q3MainWindow::mousePressEvent(e);
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
	Q3MainWindow::mouseMoveEvent(e);
}

void
ImageSplitter::mouseReleaseEvent(QMouseEvent *e)
{
	if (dragging)
	{
		qDebug("Stopped dragging...\n");
		dragging = false;
	}
	Q3MainWindow::mouseReleaseEvent(e);
}

void 
ImageSplitter::startDrag()
{
	if (fFilename != QString::null)
	{
		QList<QUrl> list;
		QUrl url = QUrl::fromLocalFile(fFilename);
		list.append(url);
		QDrag *drag = new QDrag(this);
		QMimeData *mimeData = new QMimeData;
		mimeData->setUrls(list);
		drag->setMimeData(mimeData);

                drag->exec(Qt::CopyAction);
	}
}

void
ImageSplitter::LoadSettings()
{
	QString filename = QString::null;
	lastdir = QString::null;
	QFile qf("isplitter.ini");
	if (qf.open(QIODevice::ReadOnly))
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
	if (qf.open(QIODevice::WriteOnly))
	{
		QByteArray temp = lastdir.utf8();
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
	QString filename = QFileDialog::getOpenFileName ( this, tr("Open image..."), lastdir, "*.png;*.bmp;*.xbm;*.xpm;*.pbm;*.pgm;*.ppm;*.jpg;*.jpeg;*.mng;*.gif;*.tiff");
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
	double neww = ui->pxlCollage->height() * ratio;
	if (neww > ui->pxlCollage->width())
	{
		width = ui->pxlCollage->width();
		double dh = ui->pxlCollage->width() / ratio;
		height = lrint(dh);
	}
	else
	{
		width = lrint(neww);
		height = ui->pxlCollage->height();
	}
}

void
ImageSplitter::scaleImage(const QImage * image, int &width, int &height)
{
	int oldw = image->width();
	int oldh = image->height();
	double ratio = (double) oldw / (double) oldh;
	double neww = ui->pxlCollage->height() * ratio;
	if (neww > ui->pxlCollage->width())
	{
		width = ui->pxlCollage->width();
		double dh = ui->pxlCollage->width() / ratio;
		height = lrint(dh);
	}
	else
	{
		width = lrint(neww);
		height = ui->pxlCollage->height();
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

		ui->pxlCollage->setPixmap(pixCollage);
		fPreview->ShowImage(image);
		fPreview->show();

		QFileInfo info(fFilename);
		lastdir = info.dirPath();

		setCaption( tr("Image Splitter") + " - " + QDir::convertSeparators(fFilename) );

		ui->TabWidget2->showPage(ui->tab1);
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

	ui->pxlCollage->clear();
	
    setCaption( tr( "Image Splitter" ) );

	// reset input boxes
	ui->CollageSizeX->setText("0");
	ui->CollageSizeY->setText("0");
	ui->CollageOffsetTopX->setText("0");
	ui->CollageOffsetTopY->setText("0");
	ui->CollageOffsetBottomX->setText("0");
	ui->CollageOffsetBottomY->setText("0");
	//
	ui->OffsetIndexX->setText("0");
	ui->OffsetIndexY->setText("0");
	//
	ui->ImageOffsetTopX->setText("0");
	ui->ImageOffsetTopY->setText("0");
	ui->ImageOffsetBottomX->setText("0");
	ui->ImageOffsetBottomY->setText("0");
	//
	ui->ImageRotate->setText("0");
	ui->ImageScale->setText("100");
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
	Q3MainWindow::resizeEvent(e);
}
