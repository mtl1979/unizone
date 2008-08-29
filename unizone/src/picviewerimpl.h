#ifndef WPICVIEWERIMPL_H
#define WPICVIEWERIMPL_H

#include <qimage.h>

#include "picviewer.h"

#include "util/Queue.h"
#include "util/ByteBuffer.h"

using namespace muscle;

class WPicViewer : public WPicViewerBase
{
	Q_OBJECT
public:
	WPicViewer( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = QWidget::WStyle_Customize | QWidget::WStyle_NormalBorder | QWidget::WStyle_SysMenu | QWidget::WStyle_MinMax | QWidget::WStyle_Title);
	~WPicViewer();

	bool LoadImage(const QString &file);

public slots:
	void FirstImage();
	void PreviousImage();
	void NextImage();
	void LastImage();
	void OpenImage();
	void CloseImage();

protected:
	void resizeEvent(QResizeEvent * e);
	void dragEnterEvent(QDragEnterEvent* event);
	void dropEvent(QDropEvent* event);
	void startDrag();
	void mousePressEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	bool eventFilter( QObject *o, QEvent *e );

   friend class WinShareWindow;
   void LoadSettings();
   void SaveSettings();

private:
	bool LoadImage(const ByteBufferRef &buffer, const char *format, QImage &image);
	bool LoadImage(int pos);
	void DrawImage(const QImage &image);

	void UpdatePosition(int pos);
	void UpdateName();
	QImage scaleImage(const QImage &image);

	int cFile, nFiles;
	Queue<QImage> fImages;
	Queue<QString> fFiles;

	bool dragging;
	QPoint startPos;

	QString lastdir;
};

#endif
