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
	WPicViewer( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = QWidget::WStyle_Customize | QWidget::WStyle_NormalBorder | QWidget::WStyle_SysMenu | QWidget::WStyle_MinMax );
	~WPicViewer();

	bool LoadImage(const QString &file);

public slots:
	void FirstImage();
	void PreviousImage();
	void NextImage();
	void LastImage();

protected:
	void resizeEvent(QResizeEvent * e);

private:
	bool LoadImage(const ByteBufferRef &buffer, const QString &format);
	bool LoadImage(int pos);
	void DrawImage(const QPixmap &image);

	void UpdatePosition(int pos);

	int cFile, nFiles;
	Queue<ByteBufferRef> fImages;
	Queue<QString> fFiles;
	Queue<QString> fFormats;
};

#endif
