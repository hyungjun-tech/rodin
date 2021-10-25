#include "stdafx.h"
#include "ScaledFrame.h"

ScaledFrame::ScaledFrame()
	: scale(qglviewer::Vec(1.0f, 1.0f, 1.0f))
{
}

ScaledFrame::ScaledFrame(ScaledFrame &scaledFrame_)
	: frame(scaledFrame_.frame),
	scale(scaledFrame_.scale)
{

}

ScaledFrame::~ScaledFrame()
{
}

QMatrix4x4 ScaledFrame::getModelMatrix()
{
	double mat[4][4];
	frame.getWorldMatrix(mat);

	QMatrix4x4 worldMat;
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			worldMat(i, j) = mat[j][i];

	QMatrix4x4 scaleMat;
	scaleMat(0, 0) = scale[0];
	scaleMat(1, 1) = scale[1];
	scaleMat(2, 2) = scale[2];

	return worldMat*scaleMat;
}

qglviewer::Vec ScaledFrame::toLocalCoords(qglviewer::Vec world_)
{
	qglviewer::Vec c = frame.coordinatesOf(
		qglviewer::Vec(world_[0], world_[1], world_[2]));

	return qglviewer::Vec(c[0] / scale[0], c[1] / scale[1], c[2] / scale[2]);
}

qglviewer::Vec ScaledFrame::toWorldCoords(qglviewer::Vec local_)
{
	qglviewer::Vec c = frame.inverseCoordinatesOf(
		qglviewer::Vec(
			local_[0] * scale[0], local_[1] * scale[1], local_[2] * scale[2]));

	return qglviewer::Vec(c[0], c[1], c[2]);
}

qglviewer::Vec ScaledFrame::toLocalVector(qglviewer::Vec world_)
{
	qglviewer::Vec c = frame.transformOf(
		qglviewer::Vec(world_[0], world_[1], world_[2]));
	qglviewer::Vec v(c[0] * scale[0], c[1] * scale[1], c[2] * scale[2]);
	v.normalize();
	return v;
}

qglviewer::Vec ScaledFrame::toWorldVector(qglviewer::Vec local_)
{
	qglviewer::Vec c(local_[0] / scale[0], 
			local_[1] / scale[1], 
			local_[2] / scale[2]);
	qglviewer::Vec v = frame.inverseTransformOf(c);
	v.normalize();
	return v;
}

void ScaledFrame::moveFrame(float dx_, float dy_, float dz_)
{
	frame.translate(qglviewer::Vec(dx_, dy_, dz_));
}

void ScaledFrame::rotateFrame(qglviewer::Vec wolrdAxis_, float rotAngle_)
{
	qglviewer::Vec localAxis(frame.transformOf(wolrdAxis_));

	qglviewer::Quaternion qObject(localAxis, rotAngle_);
	frame.rotate(qObject);
}
void ScaledFrame::setScale(qglviewer::Vec scale_)
{
	for (int i = 0; i < 3; i++)
		scale[i] = scale_[i];
}