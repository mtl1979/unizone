#ifndef PREVIEW_H
#define PREVIEW_H

#include "preview.h"

class QImage;
class QString;

class Preview : public PreviewWindow
{
	Q_OBJECT
public:
	Preview( QWidget* parent = 0, const char* name = 0, WFlags fl = 0);
	~Preview();
	void setOwner(QWidget * owner);
protected slots:
	void PreviewImage();
	void Save();

protected:
	void resizeEvent(QResizeEvent *e);
	friend class ImageSplitter;
	void ClearPreview();
	void ShowImage(QImage *);


private:
	QImage * image;
	QPixmap * pixPreview;
	ImageSplitter * Splitter;
};
#endif
