#include "picviewerimpl.h"
#include "support/MuscleSupport.h"

#include <qapplication.h>
#include <qpushbutton.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qimage.h>
#include <qlayout.h>
#include <qfile.h>
#include <q3filedialog.h>
#include <q3dragobject.h>
#include <q3url.h>
#include <qdir.h>
#include <QDropEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QEvent>
#include <QDragEnterEvent>
#include <QImageReader>
#if defined(QT_NO_IMAGEIO_JPEG)
#include "jpegio.h"
#endif

#include "util.h"
#include "debugimpl.h"
#include "wfile.h"
#include "platform.h"
#include "global.h"
#include "settings.h"
#include "winsharewindow.h"

WPicViewer::WPicViewer(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
: QDialog(parent, name, modal, fl)
{
	ui = new Ui_WPicViewerBase();
	ui->setupUi(this);

	if (!name)
		setName("WPicViewer");

	dragging = false;

	connect(ui->btnFirst, SIGNAL(clicked()), this, SLOT(FirstImage()));
	connect(ui->btnPrevious, SIGNAL(clicked()), this, SLOT(PreviousImage()));
	connect(ui->btnNext, SIGNAL(clicked()), this, SLOT(NextImage()));
	connect(ui->btnLast, SIGNAL(clicked()), this, SLOT(LastImage()));
	connect(ui->btnOpen, SIGNAL(clicked()), this, SLOT(OpenImage()));
	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(CloseImage()));

	ui->btnFirst->setEnabled(false);
	ui->btnPrevious->setEnabled(false);
	ui->btnNext->setEnabled(false);
	ui->btnLast->setEnabled(false);
	ui->btnClose->setEnabled(false);

	// Use our copy of JPEG IO if Qt doesn't have it ;)
#if defined(QT_NO_IMAGEIO_JPEG)
	InitJpegIO();
#endif

	setAcceptDrops(TRUE);
	ui->pxlPixmap->installEventFilter(this);

	cFile = 0;
	nFiles = 0;

	lastdir = downloadDir();
}

WPicViewer::~WPicViewer()
{
	fImages.Clear();
	fFiles.Clear();
}

void
WPicViewer::LoadSettings()
{
   lastdir = gWin->fSettings->GetLastImageDir();
}

void
WPicViewer::SaveSettings()
{
   gWin->fSettings->SetLastImageDir(lastdir);
}

bool
WPicViewer::eventFilter( QObject *o, QEvent *e )
{
	if (o == ui->pxlPixmap)
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
		default: 
			break; // empty
		}
	}
	return false;
}

void
WPicViewer::mousePressEvent(QMouseEvent *e)
{
	if (e->button() & Qt::LeftButton)
	{
		PRINT("Started dragging...\n");
		dragging = true;
	}
	QDialog::mousePressEvent(e);
}

void
WPicViewer::mouseMoveEvent(QMouseEvent *e)
{
	if (dragging)
	{
		QPoint currPos = e->pos();
		if ( ( startPos - currPos ).manhattanLength() > QApplication::startDragDistance() )
			startDrag();
	}
	QDialog::mouseMoveEvent(e);
}

void
WPicViewer::mouseReleaseEvent(QMouseEvent *e)
{
	if (dragging)
	{
		PRINT("Stopped dragging...\n");
		dragging = false;
	}
	QDialog::mouseReleaseEvent(e);
}

void 
WPicViewer::dragEnterEvent(QDragEnterEvent* event)
{
    event->accept( Q3UriDrag::canDecode(event) );
}

void 
WPicViewer::dropEvent(QDropEvent* event)
{
	if (event->source() != this)
	{
		QStringList list;
		
		if ( Q3UriDrag::decodeLocalFiles(event, list) ) {
			QStringList::Iterator it = list.begin();
			while (it != list.end())
			{
				QString filename = *it;
				LoadImage(filename);
				it++;
			}
		}
	}
}

void 
WPicViewer::startDrag()
{
	if (fFiles[cFile] != QString::null)
	{
		QStringList list;
		list.append(fFiles[cFile]);
		Q3UriDrag *d = new Q3UriDrag(this);
		d->setFilenames(list);
		d->dragCopy();
	}
}

bool
WPicViewer::LoadImage(const QString &file)
{
	bool ret = false;
	QImage fImage;
	const char * fmt = QImageReader(file).format();
	ByteBufferRef buf = GetByteBufferFromPool();
	WFile f;
	int oldpos = -1;
	for (unsigned int x = 0; x < fFiles.GetNumItems(); x++)
	{
		if (fFiles[x] == file)
		{
			oldpos = x;
			break;
		}
	}
	if (f.Open(file, QIODevice::ReadOnly))
	{
		if (buf()->SetNumBytes((uint32) f.Size(), false) == B_OK)
		{
			CheckSize(f.Size());
			if (f.ReadBlock32((char *) buf()->GetBuffer(), (uint32) f.Size()) == (int32) f.Size())
			{
				if (ret = LoadImage(buf, fmt, fImage))
				{
					if (oldpos == -1)
					{
						fImages.AddTail(fImage);
						fFiles.AddTail(file);
						UpdatePosition(nFiles);
					}
					else
					{
						fImages.ReplaceItemAt(oldpos, fImage);
						fFiles.ReplaceItemAt(oldpos, file);
						UpdatePosition(oldpos);
					}
				}
			}
		}
		f.Close();
	}

	QFileInfo info(file);
	lastdir = info.dirPath();

	return ret;
}

bool
WPicViewer::LoadImage(const ByteBufferRef &buffer, const char *fmt, QImage &image)
{
	bool ret;
	ret = image.loadFromData(buffer()->GetBuffer(),buffer()->GetNumBytes(), fmt);
	if (ret)
		DrawImage(image);
	return ret;
}

void
WPicViewer::UpdateName()
{
	QString fFile, fName;

	if ( fFiles.GetItemAt(cFile, fFile) == B_OK)
		setCaption(tr("Picture Viewer") + " - " + SimplifyPath(fFile));
	else
		setCaption(tr("Picture Viewer"));
}

void
WPicViewer::UpdatePosition(int pos)
{
	nFiles = fImages.GetNumItems();
	cFile = muscleClamp(pos, 0, (int) (nFiles - 1));
	ui->btnFirst->setEnabled(pos == 0 ? false : true);
	ui->btnPrevious->setEnabled(pos == 0 ? false : true);
	ui->btnNext->setEnabled((cFile + 1) != nFiles ? true : false);
	ui->btnLast->setEnabled((cFile + 1) != nFiles ? true : false);
	ui->btnClose->setEnabled(nFiles == 0 ? false : true);

	if (nFiles > 0)
		ui->txtItems->setText(tr("%1/%2").arg(cFile + 1).arg(nFiles));
	else
		ui->txtItems->setText(tr("No File"));
	UpdateName();
}

bool
WPicViewer::LoadImage(int pos)
{
	cFile = pos;
	QImage image;
	if ((fImages.GetItemAt(pos, image) == B_OK))
	{
		DrawImage(image);
		UpdatePosition(pos);
		return true;
	}
	return false;
}

void
WPicViewer::FirstImage()
{
	(void) LoadImage(0);
}

void
WPicViewer::PreviousImage()
{
	(void) LoadImage(cFile - 1);
}

void
WPicViewer::NextImage()
{
	(void) LoadImage(cFile + 1);
}

void
WPicViewer::LastImage()
{
	(void) LoadImage(nFiles - 1);
}

QImage
WPicViewer::scaleImage(const QImage & image)
{
	int width, height;
	int oldw = image.width();
	int oldh = image.height();
	double ratio = (double) oldw / (double) oldh;
	int neww = lrint((double) ((double) ui->pxlPixmap->height() * ratio));
	if (neww > ui->pxlPixmap->width())
	{
		width = ui->pxlPixmap->width();
		height = lrint((double) ((double) ui->pxlPixmap->width() / ratio));
	}
	else
	{
		width = neww;
		height = ui->pxlPixmap->height();
	}
	return image.smoothScale(width, height);
}

void
WPicViewer::DrawImage(const QImage &image)
{
	// Scale image to fit whole area
	QImage nimg = scaleImage(image);
	QPixmap temp;
	if (temp.convertFromImage(nimg))
	{
		ui->pxlPixmap->setPixmap( temp );
		//
		UpdateName();
	}
}

void
WPicViewer::resizeEvent(QResizeEvent *e)
{
	QWidget * lwidget = dynamic_cast<QWidget *>(ui->Layout5->parent());
	if (lwidget)
		lwidget->resize(e->size());
	//
	if (ui->pxlPixmap->pixmap())
	{
		QImage fImage;
		if (fImages.GetItemAt(cFile, fImage) == B_OK)
		{
			DrawImage(fImage);		
		}
	}
	//
	QDialog::resizeEvent(e);
}

void
WPicViewer::CloseImage()
{
	if (nFiles != 0)
	{
		fImages.RemoveItemAt(cFile);
		fFiles.RemoveItemAt(cFile);
		nFiles--;
		if (nFiles == 0)
		{
			ui->pxlPixmap->clear();
			UpdatePosition(0);
		}
		else
		{
			LoadImage(muscleClamp(cFile, 0, (nFiles - 1)));
		}
	}
}

void
WPicViewer::OpenImage()
{
	QStringList files = Q3FileDialog::getOpenFileNames ( imageFormats(), lastdir, this);
	if (!files.isEmpty())
	{
		QStringList::Iterator iter = files.begin();
		while (iter != files.end())
			(void) LoadImage(*iter++);
	}
}
