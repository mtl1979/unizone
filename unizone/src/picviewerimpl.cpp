#include "picviewerimpl.h"
#include "support/MuscleSupport.h"

#include <qpushbutton.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qimage.h>
#include <qlayout.h>
#include <qfile.h>
#ifdef WIN32
#include "jpegio.h"
#endif

WPicViewer::WPicViewer(QWidget* parent, const char* name, bool modal, WFlags fl)
: WPicViewerBase(parent, name, modal, fl)
{
	if (!name)
		setName("WPicViewer");

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
}

WPicViewer::~WPicViewer()
{
	fImages.Clear();
	fFiles.Clear();
	fFormats.Clear();
}

bool
WPicViewer::LoadImage(const QString &file)
{
	bool ret = false;
	QImage fImage;
	const char * fmt = QImageIO::imageFormat(file);
	ByteBufferRef buf = GetByteBufferFromPool();
	QFile f(file);
	if (f.open(IO_ReadOnly))
	{
		if (buf()->SetNumBytes(f.size(), false) == B_OK)
		{
			if (f.readBlock((char *) buf()->GetBuffer(), f.size()) == (int) f.size())
			{
				if (ret = LoadImage(buf, fmt))
				{
					fImages.AddTail(buf);
					fFiles.AddTail(file);
					fFormats.AddTail(QString::fromLocal8Bit(fmt));
					UpdatePosition(fImages.GetNumItems() - 1);
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
	QImage fImage;
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
	if (pos == 0)
	{
		btnFirst->setEnabled(false);
		btnPrevious->setEnabled(false);
	}
	else
	{
		btnFirst->setEnabled(true);
		btnPrevious->setEnabled(true);
	}
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
WPicViewer::DrawImage(const QImage &image)
{
	QPixmap pm;
	pm = image;
	pxlPixmap->setPixmap(pm);
//	pxlPixmap->update();
}

void
WPicViewer::resizeEvent(QResizeEvent *e)
{
	QWidget * lwidget = dynamic_cast<QWidget *>(Layout9->parent());
	if (lwidget)
		lwidget->resize(e->size());
	WPicViewerBase::resizeEvent(e);
}
