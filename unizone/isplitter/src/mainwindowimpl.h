#ifndef IMAGESPLITTER_H
#define IMAGESPLITTER_H

#include "mainwindow.h"

class MenuBar;
class QImage;
class QString;
class Preview;

class ImageSplitter : public ImageSplitterBase
{
	Q_OBJECT
public:
	ImageSplitter( QWidget* parent = 0, const char* name = 0, WFlags fl = 0);
	~ImageSplitter();
	QImage *getImage() {return image;}
	QString filename() {return fFilename;}


protected slots:
	void Load();
	void ClearImage();
	void Exit();

protected:
	void resizeEvent(QResizeEvent *e);
	void SaveSettings();

private:
	QImage *image;
	QString fFilename;
	QString lastdir;
	MenuBar * menuBar;

	Preview * fPreview;
};
#endif
