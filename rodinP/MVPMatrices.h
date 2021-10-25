#pragma once

// Matrices for Rendering -------------------------------------------------------------------------
class MVPMatrices {
public:
	MVPMatrices(QMatrix4x4 m_, QMatrix4x4 v_, QMatrix4x4 p_);
	~MVPMatrices();

	QMatrix4x4 modelMatrix;
	QMatrix4x4 viewMatrix;
	QMatrix4x4 normalMatrix;
	QMatrix4x4 projectionMatrix;
};