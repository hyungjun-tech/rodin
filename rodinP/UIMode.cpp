#include "stdafx.h"
#include "UIMode.h"

QColor UIMode::translateUIColor = QColor(0.9 * 255, 0.9 * 255, 0.95 * 255, 1.0 * 255);
QColor UIMode::rotateUIColor = QColor(0.9 * 255, 0.9 * 255, 0.95 * 255, 1.0 * 255);
QColor UIMode::hollowUIColor = QColor(0.9 * 255, 0.9 * 255, 0.95 * 255, 1.0 * 255);
QColor UIMode::previewUIColor = QColor(0.9 * 255, 0.9 * 255, 0.95 * 255, 1.0 * 255);
QColor UIMode::supportUIColor = QColor(0.9 * 255, 0.9 * 255, 0.95 * 255, 1.0 * 255);
QColor UIMode::changeBottomUIColor = QColor(0.9 * 255, 0.9 * 255, 0.95 * 255, 1.0 * 255);
QColor UIMode::analysisUIColor = QColor(0.9 * 255, 0.9 * 255, 0.95 * 255, 1.0 * 255);
QColor UIMode::layerColorUIColor = QColor(0.9 * 255, 0.9 * 255, 0.95 * 255, 1.0 * 255);
/*QColor UIMode::rotateUIColor = QColor(0.95 * 255, 0.9 * 255, 0.9 * 255, 1.0 * 255);
QColor UIMode::hollowUIColor = QColor(0.9 * 255, 0.95 * 255, 0.9 * 255, 1.0 * 255);
QColor UIMode::previewUIColor = QColor(0.95 * 255, 0.9 * 255, 0.95 * 255, 1.0 * 255);
QColor UIMode::supportUIColor = QColor(0.95 * 255, 0.95 * 255, 0.9 * 255, 1.0 * 255);
QColor UIMode::changeBottomUIColor = QColor(0.95 * 255, 0.9 * 255, 0.9 * 255, 1.0 * 255);	// added //ijeong 2019 08
QColor UIMode::analysisUIColor = QColor(0.7 * 255, 0.7 * 255, 0.8 * 255, 1.0 * 255);
QColor UIMode::layerColorUIColor = QColor(0.9 * 255, 0.9 * 255, 0.95 * 255, 1.0 * 255);*/

// UIMode ---------------------------------------------------------------------------------
UIMode::UIMode()
{
}
UIMode::UIMode(ViewerModule* engine_)
{
	engine = engine_;
	modelContainer = engine_->modelContainer;
	camera = engine_->camera();
	manipulatedFrame = engine_->manipulatedFrame;
}

UIMode::~UIMode()
{
}

void UIMode::mousePressEvent(QMouseEvent* e)
{
	pressed = e->pos() * engine->devicePixelRatioF();
}
void UIMode::mouseReleaseEvent(QMouseEvent* e)
{
	bool clicked = false;
	QPoint released = e->pos() * engine->devicePixelRatioF();
	if ((pressed - released).manhattanLength() < dragThreshold)
		clicked = true;

	if (clicked)
		if (e->modifiers() == Qt::NoModifier && e->button() == Qt::MidButton)
			engine->fitToModel();
}
void UIMode::mouseDoubleClickEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton)
		engine->fitToModel();
}