#include "stdafx.h"
#include "SupportEditPlane.h"
#include "PickingIdxManager.h"

SupportEditPlane::SupportEditPlane()
	: editPlane(nullptr)
	, planeBoundary(nullptr)
	, supportLines(nullptr)
	, editPlaneRenderer(nullptr)
	, planeBoundaryRenderer(nullptr)
	, supportLinesRenderer(nullptr)
{
	editPlaneRenderer = new VertexColorRenderer();
	planeBoundaryRenderer = new LineRenderer();
	planeBoundaryRenderer->setColor(0.0, 0.0, 0.0, 0.8);
	supportLinesRenderer = new LineRenderer();
	supportLinesRenderer->setColor(1.0, 1.0, 1.0, 0.9);

	fboRenderer = new NonShadedRenderer();
	qglviewer::Vec fboColor = PickingIdxManager::encodeID(PickingIdxManager::PLANE_ID);
	fboRenderer->setColor(fboColor[0], fboColor[1], fboColor[2]);
}


SupportEditPlane::~SupportEditPlane()
{
	delete editPlane;
	delete planeBoundary;
	delete supportLines;
	delete editPlaneRenderer;
	delete planeBoundaryRenderer;
	delete supportLinesRenderer;
	delete fboRenderer;
}

void SupportEditPlane::setSupportData(SupportData* supportData_)
{
	supportData = supportData_;
}

void SupportEditPlane::createMesh()
{
	createPlaneBoundary();
	createEditPlane();
	createSupportLines();
}

void SupportEditPlane::updateBufferData(double z)
{
	qglviewer::Vec pos = scaledFrame.frame.position();
	scaledFrame.moveFrame(0, 0, z - pos.z);
	updateColorData();
}

void SupportEditPlane::updateColorData()
{
	OpenMesh::VPropHandleT<float> alpha;
	if (!editPlane->get_property_handle(alpha, "alpha"))
		editPlane->add_property(alpha, "alpha");

	for (int n = 0; n < supportData->b_inside.size(); ++n)
	{
		if (supportData->b_inside[n])
		{
			Mesh::VertexHandle vh = vhs[vertex_idx[n]];

			if (supportData->b_inside[n])
			{
				int t = supportData->getType(n);
				if (t == 0)
				{
					editPlane->set_color(vh, Mesh::Color(0.5, 0.5, 0.5));
					editPlane->property(alpha, vh) = 0.5f;
				}
				else if (t == 1)
				{
					editPlane->set_color(vh, Mesh::Color(0, 0, 1.0f));
					editPlane->property(alpha, vh) = 1.0f;
				}
				else
				{
					editPlane->set_color(vh, Mesh::Color(1.0f, 0, 0));
					editPlane->property(alpha, vh) = 1.0f;
				}
			}
			else
			{
				editPlane->set_color(vh, Mesh::Color(0.5, 0.5, 0.5));
				editPlane->property(alpha, vh) = 0.5f;
			}
		}
	}
	editPlaneRenderer->deleteReference();

	/*int insideCount = supportData->b_inside.size();
	std::vector<std::vector<float>> vertex_colors(insideCount, { 0.5f, 0.5f, 0.5f, 0.5f });

	for (int n = 0; n < insideCount; ++n)
	{
		if (supportData->b_inside[n])
		{
			Mesh::VertexHandle vh = vhs[vertex_idx[n]];

			if (supportData->b_inside[n])
			{
				int t = supportData->getType(n);
				if (t == 0)
					continue;
				else if (t == 1)
					vertex_colors[n] = { 0.f, 0.f, 1.f, 1.f };
				else
					vertex_colors[n] = { 1.f, 0.f, 0.f, 1.f };
			}
		}
	}

	std::vector<float> points;
	int w, h;
	w = supportData->gridWidth;
	h = supportData->gridHeight;
	for (int i = 0; i < w - 1; ++i)
	{
		for (int j = 0; j < h - 1; ++j)
		{
			int n1 = i + j * w;
			int n2 = i + 1 + j * w;
			int n3 = i + (j + 1) * w;
			int n4 = i + 1 + (j + 1) * w;

			double x = (supportData->gridOffset.X + supportData->gridScale*i)*0.001;
			double y = (supportData->gridOffset.Y + supportData->gridScale*j)*0.001;

			if (supportData->b_inside[n1] && supportData->b_inside[n2] && supportData->b_inside[n3] && supportData->b_inside[n4])
			{
				for (auto colors : vertex_colors[n1])
					points.push_back(colors);
				for (auto colors : vertex_colors[n2])
					points.push_back(colors);
				for (auto colors : vertex_colors[n4])
					points.push_back(colors);
				for (auto colors : vertex_colors[n1])
					points.push_back(colors);
				for (auto colors : vertex_colors[n4])
					points.push_back(colors);
				for (auto colors : vertex_colors[n3])
					points.push_back(colors);
			}

		}
	}

	editPlaneRenderer->vao->CreateColorBuffer(points);*/
}

void SupportEditPlane::drawOnScreenCanvas(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	MVPMatrices mvp(scaledFrame.getModelMatrix(), view_, proj_);
	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
	MVPMatrices mvpBottom(bottomFrame.getModelMatrix(), view_, proj_);

	//glFuncs->glDisable(GL_LINE_SMOOTH);
	glFuncs->glDisable(GL_LIGHTING);
	glFuncs->glLineWidth(1.0);
	planeBoundaryRenderer->draw(planeBoundary, mvp);

	glFuncs->glLineWidth(0.2);
	supportLinesRenderer->draw(supportLines, mvpBottom);
	glFuncs->glDepthMask(GL_FALSE);
	editPlaneRenderer->draw(editPlane, mvp);
	glFuncs->glDepthMask(GL_TRUE);
}
void SupportEditPlane::drawForColorPicking(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
	MVPMatrices mvp(scaledFrame.getModelMatrix(), view_, proj_);

	fboRenderer->draw(editPlane, mvp);
}

void SupportEditPlane::createPlaneBoundary()
{
	delete planeBoundary;
	planeBoundary = new Mesh();
	planeBoundaryRenderer->deleteReference();

	for (int i = 0; i < supportData->outlines.size(); ++i)
	{
		int nn = supportData->outlines[i].size();
		for (int j = 0; j < supportData->outlines[i].size(); ++j)
		{
			Mesh::Point pt1(INT2MM(supportData->outlines[i][j].X*supportData->gridScale + supportData->gridOffset.X),
				INT2MM(supportData->outlines[i][j].Y*supportData->gridScale + supportData->gridOffset.Y), 0);
			Mesh::Point pt2(INT2MM(supportData->outlines[i][(j + 1) % nn].X*supportData->gridScale + supportData->gridOffset.X),
				INT2MM(supportData->outlines[i][(j + 1) % nn].Y*supportData->gridScale + supportData->gridOffset.Y), 0);
			planeBoundary->add_vertex(pt1);
			planeBoundary->add_vertex(pt2);
		}
	}
}
void SupportEditPlane::createEditPlane()
{
	delete editPlane;
	editPlane = new Mesh();
	editPlaneRenderer->deleteReference();

	int w, h;
	w = supportData->gridWidth;
	h = supportData->gridHeight;
	vertex_idx.resize(w*h, -1);

	for (int i = 0; i < w; ++i)
	{
		for (int j = 0; j < h; ++j)
		{
			int n = i + j * w;
			if (supportData->b_inside.size() <= n || n < 0)
			{
				qDebug() << "size error : " << n << " : i : " << i << " : j : " << j;
				continue;
			}
			if (supportData->b_inside[n])
			{
				double x = (supportData->gridOffset.X + supportData->gridScale*i)*0.001;
				double y = (supportData->gridOffset.Y + supportData->gridScale*j)*0.001;

				vertex_idx[n] = vhs.size();
				vhs.push_back(editPlane->add_vertex(Mesh::Point(x, y, 0)));
			}
		}
	}

	for (int i = 0; i < w - 1; ++i)
	{
		for (int j = 0; j < h - 1; ++j)
		{
			int n1 = i + j * w;
			int n2 = i + 1 + j * w;
			int n3 = i + (j + 1) * w;
			int n4 = i + 1 + (j + 1) * w;

			double x = (supportData->gridOffset.X + supportData->gridScale*i)*0.001;
			double y = (supportData->gridOffset.Y + supportData->gridScale*j)*0.001;

			if (supportData->b_inside[n1] && supportData->b_inside[n2] && supportData->b_inside[n3] && supportData->b_inside[n4])
			{
				std::vector<Mesh::VertexHandle> face_vhandles;
				face_vhandles.push_back(vhs[vertex_idx[n1]]);
				face_vhandles.push_back(vhs[vertex_idx[n2]]);
				face_vhandles.push_back(vhs[vertex_idx[n4]]);
				editPlane->add_face(face_vhandles);
				face_vhandles.clear();
				face_vhandles.push_back(vhs[vertex_idx[n1]]);
				face_vhandles.push_back(vhs[vertex_idx[n4]]);
				face_vhandles.push_back(vhs[vertex_idx[n3]]);
				editPlane->add_face(face_vhandles);
			}

		}
	}
}

void SupportEditPlane::createSupportLines()
{
	delete supportLines;
	supportLines = new Mesh();

	int w, h;
	w = supportData->gridWidth;
	h = supportData->gridHeight;
	for (int i = 0; i < w*h; ++i)
	{
		int x = i % w;
		int y = i / w;

		double xx = INT2MM(supportData->gridOffset.X + supportData->gridScale*x);
		double yy = INT2MM(supportData->gridOffset.Y + supportData->gridScale*y);
		for (int j = 0; j < supportData->grid[i].size(); j++)
		{
			if (!supportData->b_grid[i][j])
				continue;

			supportLines->add_vertex(Mesh::Point(xx, yy, INT2MM(supportData->grid[i][j].z)));
			if (j == 0)
				supportLines->add_vertex(Mesh::Point(xx, yy, 0.0));
			else
				supportLines->add_vertex(Mesh::Point(xx, yy, INT2MM(supportData->grid[i][j - 1].z)));
		}
	}
	supportLinesRenderer->deleteReference();
}