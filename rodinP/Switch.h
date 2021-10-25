#pragma once

class Switch : public QAbstractButton
{
	Q_OBJECT
		Q_PROPERTY(int offset READ offset WRITE setOffset)

public:
	Switch(QWidget *parent = 0);
	Switch(const QBrush &brush, QWidget *parent = 0);

	//virtual QSize sizeHint() const override;

	QBrush brush() const 
	{
		return _brush;
	}
	void setBrush(const QBrush &brsh) 
	{
		_brush = brsh;
	}

	int offset() const
	{
		return _x;
	}
	void setOffset(int o)
	{
		_x = o;
		update();
	}

	bool isOn() { return _switch; };

	void manualMouseRelease();
signals:
	void signal_switchChanged(bool);
protected:
	void paintEvent(QPaintEvent *) override;
	void mouseReleaseEvent(QMouseEvent *e) override;
	void enterEvent(QEvent *) override;

private:
	bool _switch;
	qreal _opacity;
	int _x, _y, _height, _margin;
	QBrush _thumb, _track, _brush;
	QPropertyAnimation *_anim = nullptr;
};