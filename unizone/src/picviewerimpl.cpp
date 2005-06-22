#include "picviewerimpl.h"
#include "support/MuscleSupport.h"

#include <qapplication.h>
#include <qpushbutton.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qimage.h>
#include <qlayout.h>
#include <qfile.h>
#include <qdragobject.h>
#include <qurl.h>
#ifdef WIN32
#include "jpegio.h"
#endif

#include "util.h"
#include "debugimpl.h"

WPicViewer::WPicViewer(QWidget* parent, const char* name, bool modal, WFlags fl)
: WPicViewerBase(parent, name, modal, fl)
{
	if (!name)
		setName("WPicViewer");

	dragging = false;

	connect(btnFirst, SIGNAL(pressed()), this, SLOT(FirstImage()));
	connect(btnPrevious, SIGNAL(pressed()), this, SLOT(PreviousImage()));
	connect(btnNext, SIGNAL(pressed()), this, SLOT(NextImage()));
	connect(btnLast, SIGNAL(pressed()), this, SLOT(LastImage()));
	btnFirst->setEnabled(false);
	btnPrevious->setEnabled(false);
	btnNext->setEnabled(false);
	btnLast->setEnabled(false);

	// Use our copy of JPEG IO if Qt doesn't have it ;)
#ifdef WIN32
	InitJpegIO();
#endif

	setAcceptDrops(TRUE);
	pxlPixmap->installEventFilter(this);
}

WPicViewer::~WPicViewer()
{
	fImages.Clear();
	fFiles.Clear();
	fFormats.Clear();
}

bool
WPicViewer::eventFilter( QObject *o, QEvent *e )
{
	if (o == pxlPixmap)
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
		default: 
			break; // empty
		}
	}
	return false;
}

void
WPicViewer::mousePressEvent(QMouseEvent *e)
{
	if (e->button() & LeftButton)
	{
		PRINT("Started dragging...\n");
		dragging = true;
	}
	WPicViewerBase::mousePressEvent(e);
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
	WPicViewerBase::mouseMoveEvent(e);
}

void
WPicViewer::mouseReleaseEvent(QMouseEvent *e)
{
	if (dragging)
	{
		PRINT("Stopped dragging...\n");
		dragging = false;
	}
	WPicViewerBase::mouseReleaseEvent(e);
}

void 
WPicViewer::dragEnterEvent(QDragEnterEvent* event)
{
    event->accept( QUriDrag::canDecode(event) );
}

void 
WPicViewer::dropEvent(QDropEvent* event)
{
	if (event->source() != this)
	{
		QStringList list;
		
		if ( QUriDrag::decodeLocalFiles(event, list) ) {
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
		QUriDrag *d = new QUriDrag(this);
		d->setFilenames(list);
		d->dragCopy();
	}
}

bool
WPicViewer::LoadImage(const QString &file)
{
	bool ret = false;
	QImage fImage;
	const char * fmt = QImageIO::imageFormat(file);
	ByteBufferRef buf = GetByteBufferFromPool();
	QFile f(file);
	int oldpos = -1;
	for (unsigned int x = 0; x < fFiles.GetNumItems(); x++)
	{
		if (fFiles[x] == file)
		{
			oldpos = x;
			break;
		}
	}
	if (f.open(IO_ReadOnly))
	{
		if (buf()->SetNumBytes(f.size(), false) == B_OK)
		{
			if (f.readBlock((char *) buf()->GetBuffer(), f.size()) == (int) f.size())
			{
				if (ret = LoadImage(buf, fmt))
				{
					if (oldpos == -1)
					{
						fImages.AddTail(buf);
						fFiles.AddTail(file);
						fFormats.AddTail(QString::fromLocal8Bit(fmt));
						UpdatePosition(fImages.GetNumItems() - 1);
					}
					else
					{
						fImages.ReplaceItemAt(oldpos, buf);
						fFiles.ReplaceItemAt(oldpos, file);
						fFormats.ReplaceItemAt(oldpos, QString::fromLocal8Bit(fmt));
						UpdatePosition(oldpos);
					}
				}
			}
		}
		f.close();
	}
	return ret;
}

bool
WPicViewer::LoadImage(const ByteBufferRef &buffer, const QString &fmt)
{
	bool ret;
	QPixmap fImage;
	ret = fImage.loadFromData(buffer()->GetBuffer(),buffer()->GetNumBytes(), fmt.local8Bit());
	if (ret)
		DrawImage(fImage);
	return ret;
}

void
WPicViewer::UpdatePosition(int pos)
{
	nFiles = fImages.GetNumItems();
	cFile = muscleClamp(pos, 0, (int) (fImages.GetNumItems() - 1));
	btnFirst->setEnabled(pos == 0 ? false : true);
	btnPrevious->setEnabled(pos == 0 ? false : true);
	btnNext->setEnabled((cFile + 1) != nFiles ? true : false);
	btnLast->setEnabled((cFile + 1) != nFiles ? true : false);

	txtItems->setText(tr("%1/%2").arg(cFile + 1).arg(nFiles));
	QString fFile;
	if ( fFiles.GetItemAt(pos, fFile) == B_OK)
		txtFile->setText(fFile);
}

bool
WPicViewer::LoadImage(int pos)
{
	bool ret = false;
	ByteBufferRef fBuffer;
	QString fFormat;
	cFile = pos;
	if ((fImages.GetItemAt(pos, fBuffer) == B_OK) && (fFormats.GetItemAt(pos, fFormat) == B_OK))
	{
		ret = LoadImage(fBuffer, fFormat);
		UpdatePosition(pos);
	}
	return ret;
}

void
WPicViewer::FirstImage()
{
	(void) LoadImage(0);
}

void
WPicViewer::PreviousImage()
{
	(void) LoadImage(--cFile);
}

void
WPicViewer::NextImage()
{
	(void) LoadImage(++cFile);
}

void
WPicViewer::LastImage()
{
	(void) LoadImage(nFiles - 1);
}

void
WPicViewer::DrawImage(const QPixmap &image)
{
	pxlPixmap->setPixmap(image);
}

void
WPicViewer::resizeEvent(QResizeEvent *e)
{
	QWidget * lwidget = dynamic_cast<QWidget *>(Layout9->parent());
	if (lwidget)
		lwidget->resize(e->size());
	WPicViewerBase::resizeEvent(e);
}
