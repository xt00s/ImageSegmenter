#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "BrushToolBar.h"
#include "OpenFolderDialog.h"
#include "SegmentationProgressBar.h"
#include "AboutDialog.h"
#include "Helper.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>
//#include <QTimer>
#include <QMessageBox>
#include <QFileInfo>
#include <QImageReader>
#include <QImageWriter>
#include <QSettings>
#include <QDir>

#ifndef NO_VERSION_HEADER
#include "version.h"
#endif

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
	, zoomSlider_(0)
	, zoomLabel_(0)
	, sizeLabel_(0)
	, progressLabel_(0)
	, progressBar_(0)
	, posLabel_(0)
	, currentTool_(0)
	, toolToolbarsSeparator_(0)
{
	setup();

	connect(ui->imageList, &ImageList::selected, this, &MainWindow::imageSelected);
	connect(ui->imageList, &ImageList::progressChanged, this, &MainWindow::progressChanged);
	connect(ui->actionNext, &QAction::triggered, ui->imageList, &ImageList::next);
	connect(ui->actionPrevious, &QAction::triggered, ui->imageList, &ImageList::prev);
	connect(ui->actionZoomIn, &QAction::triggered, zoomSlider_, &ZoomSlider::zoomIn);
	connect(ui->actionZoomOut, &QAction::triggered, zoomSlider_, &ZoomSlider::zoomOut);
	connect(ui->schemeTree, &SchemeTree::selected, [this] (Category* category) { scene_.canvasItem()->setCategory(category); });
	connect(ui->schemeTree, &SchemeTree::visibilityChanged, [this] (Category* category) { scene_.canvasItem()->update(); });
	connect(ui->schemeTree, &SchemeTree::schemeChanged, [this] (Scheme* scheme) { scene_.canvasItem()->setScheme(scheme); });
	connect(ui->segmentationView, &SegmentationView::zoomShifted, this, &MainWindow::zoomShifted);
	connect(ui->actionPolygon, &QAction::triggered, this, &MainWindow::toolChanged);
	connect(ui->actionBrush, &QAction::triggered, this, &MainWindow::toolChanged);
	connect(ui->actionOpenFolder, &QAction::triggered, this, &MainWindow::openFolder);
	connect(ui->actionShowImage, &QAction::triggered, [this] (bool checked) { scene_.canvasItem()->setPixmapVisible(checked); });
	connect(ui->actionBrightenImage, &QAction::triggered, [this] (bool checked) { scene_.canvasItem()->setPixmapBright(checked); });
	connect(ui->actionAbout, &QAction::triggered, [this] () { AboutDialog().exec(); });
	connect(zoomSlider_, &ZoomSlider::zoomChanged, this, &MainWindow::zoomChanged);
	connect(&scene_, &SegmentationScene::mousePosChanged, this, &MainWindow::mousePosChanged);
	connect(&scene_, &SegmentationScene::newCommand, [this] (QUndoCommand* command) { undoStack_.push(command); });

	imageSelected(QString());
	ui->imageList->setEnabled(false);

//	QTimer::singleShot(0, [this]
//	{
//		open(R"(d:\projects2\ImageSegmenter\extra\eye_scheme.json)", R"(d:\down\testdb\faces)", R"(d:\down\testdb\faces\output)");
//	});
	restoreSettings();
}

MainWindow::~MainWindow()
{
	saveMask();
	saveSettings();
	delete ui;
}

void MainWindow::setup()
{
	ui->setupUi(this);
	ui->schemeTree->setup();
	ui->segmentationView->setup();
	ui->segmentationView->setScene(&scene_);
	scene_.setup();
	ui->vSplitter->setStretchFactor(1, 1);
	ui->actionShowImage->setChecked(scene_.canvasItem()->isPixmapVisible());
	ui->actionBrightenImage->setChecked(scene_.canvasItem()->isPixmapBright());

	setupTools();
	setupStatusBar();
	setupUndoRedo();
	setupAbout();
	setupStyle();
	QApplication::setOrganizationName("Ruslan Rumiantsau");
	QApplication::setApplicationName(windowTitle());
	QApplication::setApplicationVersion(BUILD_NUMBER);
}

void MainWindow::setupStyle()
{
	QFile qss(":/style/ImageSegmenter.qss");
	if (!qss.open(QFile::ReadOnly))
		qDebug() << "Can't read styles: " << qss.errorString();

	qApp->setStyleSheet(QTextStream(&qss).readAll());
}

void MainWindow::setupStatusBar()
{
	auto segmentedLabel = new QLabel("Segmented: ", this);
	segmentedLabel->setIndent(5);

	progressBar_ = new SegmentationProgressBar(this);
	progressBar_->setObjectName("progressBar");
	progressBar_->setFixedSize(150, 15);
	progressBar_->setTextVisible(false);

	progressLabel_ = new QLabel(progressBar_);
	progressLabel_->setObjectName("progressLabel");
	progressLabel_->setAlignment(Qt::AlignCenter);

	auto progressBarLayout = new QHBoxLayout(progressBar_);
	progressBarLayout->setContentsMargins(0,0,0,0);
	progressBarLayout->addWidget(progressLabel_);

	zoomSlider_ = new ZoomSlider(this);
	zoomSlider_->setObjectName("zoomSlider");
	zoomSlider_->setFixedSize(105, 18);
	zoomSlider_->setZoomRange(0.1, 32);
	zoomSlider_->setZoomTicks({.1, .25, .33, .5, .66, .75, 1, 1.25, 1.5, 2, 3, 4, 6, 8, 12, 16, 24, 32});

	zoomLabel_ = new QLabel("100%", this);
	zoomLabel_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	zoomLabel_->setIndent(5);
	zoomLabel_->setFixedWidth(50);

	auto spacer = new QWidget(this);
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	auto sizeImage = new QLabel(this);
	sizeImage->setPixmap(QPixmap(":/image/icons/image_size.png"));

	auto posImage = new QLabel(this);
	posImage->setPixmap(QPixmap(":/image/icons/position.png"));

	sizeLabel_ = new QLabel(this);
	sizeLabel_->setFixedWidth(90);
	sizeLabel_->setIndent(5);

	posLabel_ = new QLabel(this);
	posLabel_->setFixedWidth(90);
	posLabel_->setIndent(5);

	auto statusToolBar = new QToolBar(this);
	statusToolBar->setObjectName("statusToolBar");
	statusToolBar->addWidget(segmentedLabel);
	statusToolBar->addWidget(progressBar_);
	statusToolBar->addWidget(spacer);
	statusToolBar->addWidget(sizeImage);
	statusToolBar->addWidget(sizeLabel_);
	statusToolBar->addWidget(posImage);
	statusToolBar->addWidget(posLabel_);
	statusToolBar->addWidget(zoomLabel_);
	statusToolBar->addAction(ui->actionZoomOut);
	statusToolBar->addWidget(zoomSlider_);
	statusToolBar->addAction(ui->actionZoomIn);
	statusToolBar->setIconSize(QSize(16, 16));
	statusToolBar->setStyleSheet("padding: 0;");

	ui->statusBar->addPermanentWidget(statusToolBar, 1);
}

void MainWindow::setupTools()
{
	toolToolbarsSeparator_ = ui->toolBar->addSeparator();
	toolToolbarsSeparator_->setVisible(false);

	BrushToolBar* brushToolbar = new BrushToolBar(this);
	connect(brushToolbar->widthCombo, &QComboBox::currentTextChanged, [this] (const QString& width) { scene_.setBrushWidth(width.toInt()); });

	auto brushToolbarAction = ui->toolBar->addWidget(brushToolbar);
	brushToolbarAction->setVisible(false);
	brushToolbarAction->setData(qVariantFromValue(static_cast<void*>(brushToolbar)));
	toolToolbarActions_[ui->actionBrush] = brushToolbarAction;

	ui->actionPolygon->setData(int(SegmentationScene::Tool::Polygon));
	ui->actionBrush->setData(int(SegmentationScene::Tool::Brush));

	tools_ << ui->actionPolygon
		   << ui->actionBrush;

	for (auto& t : tools_) {
		if (t->data().toInt() == int(scene_.tool())) {
			t->setChecked(true);
			currentTool_ = t;
			break;
		}
	}
}

void MainWindow::setupUndoRedo()
{
	auto undoAction = undoStack_.createUndoAction(this);
	undoAction->setIcon(QIcon(":/image/icons/undo.svg"));
	undoAction->setShortcut(QKeySequence::Undo);
	auto redoAction = undoStack_.createRedoAction(this);
	redoAction->setIcon(QIcon(":/image/icons/redo.svg"));
	redoAction->setShortcut(QKeySequence::Redo);
	auto sep = ui->toolBar->insertSeparator(ui->actionPolygon);
	ui->toolBar->insertActions(sep, QList<QAction*>() << undoAction << redoAction);
}

void MainWindow::setupAbout()
{
	auto spacer = new QWidget(this);
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	ui->toolBar->addWidget(spacer);
	ui->toolBar->addAction(ui->actionAbout);
}

void MainWindow::updateSizeLabel()
{
	auto size = scene_.canvasItem()->pixmapSize();
	if (!size.isNull()) {
		sizeLabel_->setText(QString("%1 x %2").arg(size.width()).arg(size.height()));
	} else {
		sizeLabel_->setText("");
	}
}

void MainWindow::updateTitle()
{
	QString prefix;
	if (!imagePath_.isEmpty()) {
		prefix += QFileInfo(imagePath_).fileName() + " - ";
	}
	if (ui->schemeTree->scheme()) {
		prefix += ui->schemeTree->scheme()->name() + " - ";
	}
	setWindowTitle(prefix + QApplication::applicationName());
}

void MainWindow::updateZoomFromView()
{
	auto unity = ui->segmentationView->transform().mapRect(QRectF(0,0,1,1));
	zoomSlider_->setZoom(unity.width());
}

void MainWindow::saveSettings()
{
	QSettings settings;
	settings.setValue(objectName() + "State", int(windowState()));
	if (!(windowState() & (Qt::WindowMaximized|Qt::WindowFullScreen))) {
		settings.setValue(objectName() + "Geometry", geometry());
	}
	settings.sync();
}

void MainWindow::restoreSettings()
{
	QSettings settings;
	if (settings.contains(objectName() + "State")) {
		setWindowState(static_cast<Qt::WindowState>(settings.value(objectName() + "State").toInt()));
	}
	if (settings.contains(objectName() + "Geometry")) {
		setGeometry(settings.value(objectName() + "Geometry").toRect());
	}
}

void MainWindow::saveMask()
{
	if (!scene_.canvasItem()->pixmapSize().isEmpty() && undoStack_.index() > 0) {
		auto maskPath = help::segmentationMaskFilePath(imagePath_, outputPath_, ui->schemeTree->scheme()->name());
		QImageWriter writer(maskPath);
		if (!writer.write(scene_.canvasItem()->mask().toImage())) {
			QMessageBox::warning(this, "",
								 QString("Can't write mask '%1: %2'").arg(maskPath).arg(writer.errorString()),
								 QMessageBox::Ok);
		}
	}
}

void MainWindow::imageSelected(const QString &imagePath)
{
	help::WaitCursor waitCursor;
	saveMask();

	QImage image, mask;
	if (!imagePath.isEmpty()) {
		QImageReader reader(imagePath);
		if (!reader.read(&image)) {
			QMessageBox::warning(this, "",
								 QString("Can't read image '%1': %2").arg(imagePath).arg(reader.errorString()),
								 QMessageBox::Ok);
		}
		else {
			auto maskPath = help::segmentationMaskFilePath(imagePath, outputPath_, ui->schemeTree->scheme()->name());
			if (QFileInfo::exists(maskPath)) {
				reader.setFileName(maskPath);
				if (!reader.read(&mask)) {
					QMessageBox::warning(this, "",
										 QString("Can't read mask '%1': %2").arg(imagePath).arg(reader.errorString()),
										 QMessageBox::Ok);
				}
			}
		}
	}
	scene_.canvasItem()->setPixmap(QPixmap::fromImage(image), QPixmap::fromImage(mask));
	scene_.clearToolState();
	scene_.updateSceneRect();
	if (!image.isNull()) {
		if (ui->segmentationView->viewport()->rect().contains(image.rect())) {
			ui->segmentationView->setTransform(QTransform::fromScale(1, 1));
			ui->segmentationView->centerOn(scene_.canvasItem());
		} else {
			ui->segmentationView->fitInView(scene_.canvasItem(), Qt::KeepAspectRatio);
		}
	}
	imagePath_ = imagePath;
	undoStack_.clear();
	ui->schemeTree->resetCategoryStates();
	ui->segmentationView->setEnabled(!image.isNull());
	ui->schemeTree->setEnabled(!image.isNull());
	ui->actionNext->setEnabled(ui->imageList->isNextAvailable());
	ui->actionPrevious->setEnabled(ui->imageList->isPrevAvailable());
	posLabel_->setText("");
	setToolsEnabled(!image.isNull());
	updateZoomFromView();
	updateSizeLabel();
	updateTitle();
}

void MainWindow::zoomChanged(double zoom)
{
	zoomLabel_->setText(QString::number(zoom * 100, 'f', 0) + '%');
	ui->segmentationView->setTransform(QTransform::fromScale(zoom, zoom));
}

void MainWindow::zoomShifted(int delta)
{
	for (int i = 0; i < qAbs(delta); i++) {
		if (delta > 0)
			zoomSlider_->zoomIn();
		else
			zoomSlider_->zoomOut();
	}
}

void MainWindow::mousePosChanged(const QPoint& pos)
{
	posLabel_->setText(QString("%1, %2").arg(pos.x()).arg(pos.y()));
}

void MainWindow::toolChanged()
{
	auto action = qobject_cast<QAction*>(sender());
	if (currentTool_ == action) {
		action->setChecked(true);
		return;
	}
	if (currentTool_) {
		currentTool_->setChecked(false);
		if (toolToolbarActions_.contains(currentTool_)) {
			toolToolbarsSeparator_->setVisible(false);
			toolToolbarActions_[currentTool_]->setVisible(false);
		}
	}
	scene_.setTool(static_cast<SegmentationScene::Tool>(action->data().toInt()));
	if (toolToolbarActions_.contains(action)) {
		toolToolbarsSeparator_->setVisible(true);
		auto toolbarAction = toolToolbarActions_[action];
		toolbarAction->setVisible(true);
		static_cast<ToolToolBar*>(toolbarAction->data().value<void*>())->activate(scene_);
	}
	currentTool_ = action;
}

void MainWindow::setToolsEnabled(bool enabled)
{
	if (tools_[0]->isEnabled() == enabled)
		return;
	for (auto& t : tools_) {
		t->setEnabled(enabled);
	}
	if (toolToolbarActions_.contains(currentTool_)) {
		toolToolbarsSeparator_->setVisible(enabled);
		toolToolbarActions_[currentTool_]->setVisible(enabled);
	}
}

void MainWindow::openFolder()
{
	OpenFolderDialog dialog(this);
	if (dialog.exec()) {
		open(dialog.schemePath(), dialog.folderPath(), dialog.outputPath());
	}
}

void MainWindow::progressChanged()
{
	if (progressBar_->maximum() != ui->imageList->count()) {
		progressBar_->setRange(0, ui->imageList->count());
	}
	progressBar_->setValue(ui->imageList->segmentedCount());
	if (ui->imageList->count()) {
		progressLabel_->setText(QString("%1 of %2")
								.arg(ui->imageList->segmentedCount())
								.arg(ui->imageList->count()));
	} else {
		progressLabel_->setText("");
	}
}

void MainWindow::open(const QString& schemePath, const QString& folderPath, const QString& outputPath)
{
	QString error;
	if (!QDir().mkpath(outputPath)) {
		error = QString("Can't make output path '%1'").arg(outputPath);
		ui->schemeTree->clear();
		ui->imageList->clear();
	}
	else {
		if (!ui->schemeTree->open(schemePath)) {
			ui->imageList->clear();
			error = ui->schemeTree->errorString();
		} else {
			help::WaitCursor waitCursor;
			ui->imageList->open(folderPath, outputPath, ui->schemeTree->scheme()->name());
		}
	}
	outputPath_ = error.isEmpty() ? outputPath : QString();
	ui->imageList->setEnabled(ui->schemeTree->scheme() != 0);
	ui->actionNext->setEnabled(ui->imageList->isNextAvailable());
	ui->actionPrevious->setEnabled(ui->imageList->isPrevAvailable());
	if (!error.isEmpty()) {
		QMessageBox::warning(this, "", error, QMessageBox::Ok);
	}
}
