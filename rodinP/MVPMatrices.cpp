#include "stdafx.h"
#include "MVPMatrices.h"

// Matrices for Rendering -------------------------------------------------------------------------
MVPMatrices::MVPMatrices(QMatrix4x4 m_, QMatrix4x4 v_, QMatrix4x4 p_)
{
	modelMatrix = m_;
	viewMatrix = v_;
	normalMatrix = m_.inverted().transposed();
	projectionMatrix = p_;
}

MVPMatrices::~MVPMatrices()
{
}