#ifndef IMAGESPLITTER_H
#define IMAGESPLITTER_H

#include <Q3MainWindow>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QEvent>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QPixmap>

class MenuBar;
class QImage;
class QString;
class Preview;
class QPoint;
class Ui_ImageSplitterBase;

class ImageSplitter : public Q3MainWindow
{
	Q_OBJECT
public:
	ImageSplitter( QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = 0);
	~ImageSplitter();
	QImage *getImage() {return image;}
	QString filename() {return fFilename;}
	void Load(const QString &filename);

	Ui_ImageSplitterBase * ui;

protected slots:
	void Load();
	void ClearImage();
	void Exit();

protected:
	void resizeEvent(QResizeEvent *e);
	void dragEnterEvent(QDragEnterEvent* event);
	void dropEvent(QDropEvent* event);
	void mousePressEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	bool eventFilter( QObject *o, QEvent *e );
	void startDrag();

	void LoadSettings();
	void SaveSettings();

private:
	QImage *image;
	QString fFilename;
	QString lastdir;
	MenuBar * menuBar;

	Preview * fPreview;

	bool dragging;
	QPoint startPos;

	void scalePixmap(const QPixmap * image, int &width, int &height);
	void scaleImage(const QImage * image, int &width, int &height);
};
#endif
