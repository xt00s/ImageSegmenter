#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "OpenFolderDialog.h"
#include "ZoomDialog.h"
#include "AboutDialog.h"
#include "PolygonTool.h"
#include "BrushTool.h"
#include "MagicWandTool.h"
#include "SegmentingPencilTool.h"
#include "Helper.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QMessageBox>
#include <QFileInfo>
#include <QImageReader>
#include <QImageWriter>
#include <QSettings>
#include <QDir>
#include <QWidgetAction>

#ifndef NO_VERSION_HEADER
#include "version.h"
#endif

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
	, zoomSlider_(0)
	, zoomLevelButton_(0)
	, sizeLabel_(0)
	, progressLabel_(0)
	, posLabel_(0)
	, toolToolbarsSeparator_(0)
	, keepZoomLevel_(false)
	, keepingZoom_(1)
{
	setup();

	connect(ui->imageList, &ImageList::selected, this, &MainWindow::imageSelected);
	connect(ui->imageList, &ImageList::progressChanged, this, &MainWindow::progressChanged);
	connect(ui->actionNext, &QAction::triggered, ui->imageList, &ImageList::next);
	connect(ui->actionPrevious, &QAction::triggered, ui->imageList, &ImageList::prev);
	connect(ui->actionZoomIn, &QAction::triggered, zoomSlider_, &ZoomSlider::zoomIn);
	connect(ui->actionZoomOut, &QAction::triggered, zoomSlider_, &ZoomSlider::zoomOut);
	connect(ui->schemeTree, &SchemeTree::selected, [this] (Category* category) { scene_.canvasItem()->setCategory(category); });
	connect(ui->schemeTree, &SchemeTree::clipChanged, [this] (Category* category) { scene_.canvasItem()->setClipRegion(category); });
	connect(ui->schemeTree, &SchemeTree::visibilityChanged, [this] (Category* category) { scene_.canvasItem()->update(); });
	connect(ui->schemeTree, &SchemeTree::schemeChanged, [this] (Scheme* scheme) { scene_.canvasItem()->setScheme(scheme); });
	connect(ui->segmentationView, &SegmentationView::zoomShifted, this, &MainWindow::zoomShifted);
	connect(ui->actionOpenFolder, &QAction::triggered, this, &MainWindow::openFolder);
	connect(ui->actionShowImage, &QAction::triggered, [this] (bool checked) { scene_.canvasItem()->setPixmapVisible(checked); });
	connect(ui->actionShowSegmentationMask, &QAction::triggered, [this] (bool checked) { scene_.canvasItem()->setMaskVisible(checked); });
	connect(ui->actionGrayscaleImage, &QAction::triggered, [this] (bool checked) { scene_.canvasItem()->setPixmapGray(checked); });
	connect(ui->actionAbout, &QAction::triggered, [this] () { AboutDialog().exec(); });
	connect(zoomSlider_, &ZoomSlider::zoomChanged, this, &MainWindow::zoomChanged);
	connect(zoomLevelButton_, &QToolButton::clicked, this, &MainWindow::openZoomOptions);
	connect(&scene_, &SegmentationScene::mousePosChanged, this, &MainWindow::mousePosChanged);
	connect(&scene_, &SegmentationScene::newCommand, [this] (QUndoCommand* command) { undoStack_.push(command); });

	imageSelected(QString());
	ui->imageList->setEnabled(false);

	restoreSettings();
}

MainWindow::~MainWindow()
{
	if (scene_.tool()) {
		scene_.tool()->deactivate();
	}
	qDeleteAll(tools_);
	saveMask();
	saveSettings();
	delete ui;
}

void MainWindow::setup()
{
	ui->setupUi(this);
	ui->schemeTree->setup();
	ui->imageList->setup();
	ui->segmentationView->setup();
	ui->segmentationView->setScene(&scene_);
	scene_.setup();
	ui->vSplitter->setStretchFactor(1, 1);
	ui->actionShowImage->setChecked(scene_.canvasItem()->isPixmapVisible());
	ui->actionShowSegmentationMask->setChecked(scene_.canvasItem()->isMaskVisible());
	ui->actionGrayscaleImage->setChecked(scene_.canvasItem()->isPixmapGray());

	setupTools();
	setupStatusBar();
	setupUndoRedo();
	setupOtherActions();
	setupStyle();
	setupTooltips();
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
	zoomSlider_ = new ZoomSlider(this);
	zoomSlider_->setObjectName("zoomSlider");
	zoomSlider_->setFixedSize(105, 18);
	zoomSlider_->setZoomRange(0.1, 32);
	zoomSlider_->setZoomTicks({.1, .25, .33, .5, .66, .75, 1, 1.25, 1.5, 2, 3, 4, 6, 8, 12, 16, 24, 32});

	zoomLevelButton_ = new QToolButton(this);
	zoomLevelButton_->setToolButtonStyle(Qt::ToolButtonTextOnly);
	zoomLevelButton_->setText("100%");
	zoomLevelButton_->setFixedWidth(40);

	auto spacer = new QWidget(this);
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	auto progressImage = new QLabel(this);
	progressImage->setPixmap(QIcon(":/image/icons/progress.svg").pixmap(16,16));
	progressImage->setToolTip("Segmentation progress");

	auto sizeImage = new QLabel(this);
	sizeImage->setPixmap(QIcon(":/image/icons/image-size.svg").pixmap(16,16));
	sizeImage->setToolTip("Image size");

	auto posImage = new QLabel(this);
	posImage->setPixmap(QIcon(":/image/icons/position.svg").pixmap(16,16));
	posImage->setToolTip("Image position");

	progressLabel_ = new QLabel(this);
	progressLabel_->setFixedWidth(150);
	progressLabel_->setIndent(5);

	sizeLabel_ = new QLabel(this);
	sizeLabel_->setFixedWidth(90);
	sizeLabel_->setIndent(5);

	posLabel_ = new QLabel(this);
	posLabel_->setFixedWidth(90);
	posLabel_->setIndent(5);

	auto statusToolBar = new QToolBar(this);
	statusToolBar->setObjectName("statusToolBar");
	statusToolBar->addWidget(spacer);
	statusToolBar->addWidget(progressImage);
	statusToolBar->addWidget(progressLabel_);
	statusToolBar->addWidget(sizeImage);
	statusToolBar->addWidget(sizeLabel_);
	statusToolBar->addWidget(posImage);
	statusToolBar->addWidget(posLabel_);
	statusToolBar->addWidget(zoomLevelButton_);
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

	tools_ << new PolygonTool(ui->actionPolygon, &scene_, this);
	tools_ << new BrushTool(ui->actionBrush, &scene_, this);
	tools_ << new MagicWandTool(ui->actionMagicWand, &scene_, this);
	tools_ << new SegmentingPencilTool(ui->actionSegmentingPencil, &scene_, this);

	for (auto& t : tools_) {
		if (t->toolbar()) {
			auto action = ui->toolBar->addWidget(t->toolbar());
			action->setVisible(false);
			connect(t, &Tool::activated, [this, action]{
				toolToolbarsSeparator_->setVisible(true);
				action->setVisible(true);
			});
			connect(t, &Tool::deactivated, [this, action]{
				toolToolbarsSeparator_->setVisible(false);
				action->setVisible(false);
			});
			toolToolbarActions_[t] = action;
		}
	}
	tools_[0]->activate();
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

void MainWindow::setupOtherActions()
{
	auto spacer = new QWidget(this);
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	auto opacitySlider = new Slider(this);
	opacitySlider->setObjectName("opacitySlider");
	opacitySlider->setFixedSize(105, 18);
	opacitySlider->setValue(1);
	connect(opacitySlider, &Slider::valueChanged, [this] (double value) { scene_.canvasItem()->setPixmapOpacity(value); });

	ui->toolBar->addWidget(spacer);
	ui->toolBar->addAction(ui->actionShowImage);
	ui->toolBar->addAction(ui->actionShowSegmentationMask);
	ui->toolBar->addAction(ui->actionGrayscaleImage);
	ui->toolBar->addWidget(new QLabel(" Opacity: ", this));
	ui->toolBar->addWidget(opacitySlider);
	ui->toolBar->addSeparator();
	ui->toolBar->addAction(ui->actionAbout);
}

void MainWindow::setupTooltips()
{
	for (auto& a : findChildren<QAction*>()) {
		if (!qobject_cast<QWidgetAction*>(a) && !a->isSeparator()) {
			auto shortcuts = a->shortcuts();
			auto tooltip = a->toolTip();
			if (tooltip.isEmpty()) {
				tooltip = a->text();
			}
			if (!shortcuts.empty() && !tooltip.isEmpty()) {
				if (!tooltip[tooltip.count()-1].isSpace()) {
					tooltip += ' ';
				}
				for (int i = 0; i < shortcuts.count(); i++) {
					if (i) {
						tooltip += " or ";
					}
					tooltip += R"(<font color="#909090">)" + shortcuts[i].toString() + "</font>";
				}
			}
			a->setToolTip(tooltip);
		}
	}
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
	help::WaitCursor wait;
	saveMask();

	QImage image, mask;
	if (!imagePath.isEmpty()) {
		QImageReader reader(imagePath);
		if (!reader.read(&image)) {
			help::DefaultCursor noWait;
			QMessageBox::warning(this, "",
								 QString("Can't read image '%1': %2").arg(imagePath).arg(reader.errorString()),
								 QMessageBox::Ok);
		}
		else {
			auto maskPath = help::segmentationMaskFilePath(imagePath, outputPath_, ui->schemeTree->scheme()->name());
			if (QFileInfo::exists(maskPath)) {
				reader.setFileName(maskPath);
				if (!reader.read(&mask)) {
					help::DefaultCursor noWait;
					QMessageBox::warning(this, "",
										 QString("Can't read mask '%1': %2").arg(imagePath).arg(reader.errorString()),
										 QMessageBox::Ok);
				}
			}
		}
	}
	scene_.canvasItem()->setPixmap(QPixmap::fromImage(image), QPixmap::fromImage(mask));
	scene_.overlayItem()->setSize(image.size());
	if (scene_.tool()) {
		scene_.tool()->clear();
	}
	scene_.updateSceneRect();
	if (!image.isNull()) {
		if (keepZoomLevel_) {
			zoomSlider_->setZoom(keepingZoom_);
			ui->segmentationView->centerOn(scene_.canvasItem());
		} else {
			if (ui->segmentationView->viewport()->rect().contains(image.rect())) {
				ui->segmentationView->setTransform(QTransform::fromScale(1, 1));
				ui->segmentationView->centerOn(scene_.canvasItem());
			} else {
				ui->segmentationView->fitInView(scene_.canvasItem(), Qt::KeepAspectRatio);
			}
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
	zoomLevelButton_->setText(QString::number(int(zoom * 100)) + '%');
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

void MainWindow::setToolsEnabled(bool enabled)
{
	for (auto& t : tools_) {
		t->action()->setEnabled(enabled);
	}
	if (toolToolbarActions_.contains(scene_.tool())) {
		toolToolbarsSeparator_->setVisible(enabled);
		toolToolbarActions_[scene_.tool()]->setVisible(enabled);
	}
}

void MainWindow::openFolder()
{
	OpenFolderDialog dialog(this);
	if (dialog.exec()) {
		open(dialog.schemePath(), dialog.folderPath(), dialog.outputPath());
	}
}

void MainWindow::openZoomOptions()
{
	ZoomDialog dialog(this);
	dialog.setZoom(zoomSlider_->zoom());
	dialog.setKeepZoomLevel(keepZoomLevel_);
	dialog.setKeepingZoom(keepZoomLevel_ ? keepingZoom_ : zoomSlider_->zoom());
	if (dialog.exec()) {
		zoomSlider_->setZoom(dialog.zoom());
		keepZoomLevel_ = dialog.keepZoomLevel();
		keepingZoom_ = dialog.keepingZoom();
	}
}

void MainWindow::progressChanged()
{
	if (ui->imageList->count()) {
		progressLabel_->setText(QString("%1% (%2 of %3)")
								.arg(double(ui->imageList->segmentedCount()) / ui->imageList->count() * 100, 0, 'f', 0)
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
		help::WaitCursor wait;
		if (!ui->schemeTree->open(schemePath)) {
			ui->imageList->clear();
			error = ui->schemeTree->errorString();
		} else {
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
