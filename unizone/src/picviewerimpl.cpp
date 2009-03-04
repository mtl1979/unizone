#include <qapplication.h>
#include <qpushbutton.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qimage.h>
#include <qlayout.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qdir.h>
#include <QDropEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QEvent>
#include <QDragEnterEvent>
#include <QImageReader>

#include "picviewerimpl.h"
#include "picviewer.h"
#include "util.h"
#include "debugimpl.h"
#include "wfile.h"
#include "platform.h"
#include "global.h"
#include "settings.h"
#include "winsharewindow.h"

#include "support/MuscleSupport.h"

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
	connect(ui->btnSave, SIGNAL(clicked()), this, SLOT(SaveImage()));
	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(CloseImage()));

	ui->btnFirst->setEnabled(false);
	ui->btnPrevious->setEnabled(false);
	ui->btnNext->setEnabled(false);
	ui->btnLast->setEnabled(false);
	ui->btnClose->setEnabled(false);
	ui->btnSave->setEnabled(false);

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
    if (event->mimeData()->hasUrls() || event->mimeData()->hasImage())
         event->acceptProposedAction();
}

void 
WPicViewer::dropEvent(QDropEvent* event)
{
	if (event->source() != this)
	{
		if (event->mimeData()->hasUrls())
		{
			QList<QUrl> urls = event->mimeData()->urls();
			QList<QUrl>::iterator it = urls.begin();
			while (it != urls.end())
			{
				QUrl u = *it++;
				QString file = u.toLocalFile();
				PRINT("Drop: %S\n", file.utf16());
				LoadImage(file);
			}
		}
		else if (event->mimeData()->hasImage())
		{
			QVariant v = event->mimeData()->imageData();
			if (v.canConvert(QVariant::Image))
			{
				QImage img = v.value<QImage>();
				// Check for duplicates
				for (unsigned int x = 0; x < fImages.GetNumItems(); x++)
				{
					if (img == fImages[x])
					{
						event->setAccepted(false);
						return;
					}
				}
				fFiles.AddTail(QString::null);
				fImages.AddTail(img);
				nFiles++;
				LastImage();
			} 
		}
	}
}

void 
WPicViewer::startDrag()
{
	if (fFiles[cFile] != QString::null)
	{
		QDrag *drag = new QDrag(this);
		QMimeData *mimeData = new QMimeData;
		QUrl u = QUrl::fromLocalFile(fFiles[cFile]);
		QList<QUrl> list;
		list.append(u);
		mimeData->setUrls(list);
		drag->setMimeData(mimeData);
		drag->exec(Qt::CopyAction);
	}
	else if (!fImages[cFile].isNull())
	{
		QDrag *drag = new QDrag(this);
		QMimeData *mimeData = new QMimeData;
		mimeData->setImageData(fImages[cFile]);
		drag->setMimeData(mimeData);
		drag->exec(Qt::CopyAction);
	}
}

bool
WPicViewer::LoadImage(const QString &file)
{
	bool ret = false;
	QImage fImage;
	
	QImageReader reader(file);
	if (!reader.canRead())
		return false;
	const char * fmt = reader.format();
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
				if ((ret = LoadImage(buf, fmt, fImage)) == true)
				{
					// Check duplicates
					for (unsigned int x = 0; x < fImages.GetNumItems(); x++)
					{
						if (fImage == fImages[x])
							oldpos = x;
					}

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

	if (fFiles.GetItemAt(cFile, fFile) == B_OK)
	{
		ui->btnSave->setEnabled(true);
		if (!fFile.isEmpty())
		{
			setCaption(tr("Picture Viewer") + " - " + SimplifyPath(fFile));
			return;
		}
	}
	else
		ui->btnSave->setEnabled(false);

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
	QStringList files = QFileDialog::getOpenFileNames ( this, tr("Open image..."), lastdir, imageFormats());
	if (!files.isEmpty())
	{
		QStringList::Iterator iter = files.begin();
		while (iter != files.end())
			(void) LoadImage(*iter++);
	}
}

void
WPicViewer::SaveImage()
{
	QImage img;
	QString oldfile;
	if (fImages.GetItemAt(cFile, img) == B_OK && fFiles.GetItemAt(cFile, oldfile) == B_OK)
	{
		QString file = QFileDialog::getSaveFileName(this, tr("Save image..."), oldfile.isEmpty() ? lastdir : oldfile, "Images (*.bmp *.jpg *.jpeg *.png *.ppm *.pbm *.tiff *.xbm *.xpm)");
		if (!file.isEmpty())
		{
			if (img.save(file))
			{
				fFiles.ReplaceItemAt(cFile, file);
				UpdateName();
			}
		}
	}
}
