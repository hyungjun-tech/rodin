#include "stdafx.h"
#include "CommonWidgets.h"

labelControl::labelControl(QWidget *parent)
	: QLabel(parent)
{
}

void labelControl::mouseReleaseEvent(QMouseEvent * e)
{

	emit labelClicked();
}

SpinBoxControl::SpinBoxControl(QWidget *parent)
	: QSpinBox(parent)
{
}

void SpinBoxControl::keyPressEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
	{
		emit enterPressed();
	}
	else
	{
		QSpinBox::keyPressEvent(e);
	}
}

DoubleSpinBoxControl::DoubleSpinBoxControl(QWidget *parent)
	: QDoubleSpinBox(parent)
{
}

void DoubleSpinBoxControl::keyPressEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
	{
		emit enterPressed();
	}
	else
	{
		QDoubleSpinBox::keyPressEvent(e);
	}
}

ProgressIndicator::ProgressIndicator(QWidget* parent)
	: QWidget(parent),
	m_angle(0),
	m_timerId(-1),
	m_delay(40),
	m_displayedWhenStopped(false),
	m_color(Qt::black)
{
	setFocusPolicy(Qt::NoFocus);
}

bool ProgressIndicator::isAnimated() const
{
	return (m_timerId != -1);
}

void ProgressIndicator::setDisplayedWhenStopped(bool state)
{
	m_displayedWhenStopped = state;

	update();
}

bool ProgressIndicator::isDisplayedWhenStopped() const
{
	return m_displayedWhenStopped;
}

void ProgressIndicator::startAnimation()
{
	m_angle = 0;

	if (m_timerId == -1)
		m_timerId = startTimer(m_delay);
}

void ProgressIndicator::stopAnimation()
{
	if (m_timerId != -1)
		killTimer(m_timerId);

	m_timerId = -1;

	update();
}

void ProgressIndicator::setAnimationDelay(int delay)
{
	if (m_timerId != -1)
		killTimer(m_timerId);

	m_delay = delay;

	if (m_timerId != -1)
		m_timerId = startTimer(m_delay);
}

void ProgressIndicator::setColor(const QColor & color)
{
	m_color = color;

	update();
}

QSize ProgressIndicator::sizeHint() const
{
	return QSize(30, 30);
}

int ProgressIndicator::heightForWidth(int w) const
{
	return w;
}

void ProgressIndicator::timerEvent(QTimerEvent * /*event*/)
{
	m_angle = (m_angle + 30) % 360;

	update();
}

void ProgressIndicator::paintEvent(QPaintEvent * /*event*/)
{
	if (!m_displayedWhenStopped && !isAnimated())
		return;

	int width = qMin(this->width(), this->height());

	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing);

	int outerRadius = (width - 1)*0.5;
	int innerRadius = (width - 1)*0.5*0.38;

	int capsuleHeight = outerRadius - innerRadius;
	int capsuleWidth = (width > 32) ? capsuleHeight * .23 : capsuleHeight * .35;
	int capsuleRadius = capsuleWidth / 2;

	for (int i = 0; i < 12; i++)
	{
		QColor color = m_color;
		color.setAlphaF(1.0f - (i / 12.0f));
		p.setPen(Qt::NoPen);
		p.setBrush(color);
		p.save();
		p.translate(rect().center());
		p.rotate(m_angle - i * 30.0f);
		p.drawRoundedRect(-capsuleWidth * 0.5, -(innerRadius + capsuleHeight), capsuleWidth, capsuleHeight, capsuleRadius, capsuleRadius);
		p.restore();
	}
}