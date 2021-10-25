#include "stdafx.h"
#include "GCodePath.h"

GCodePath::GCodePath()
	: isExtruderMode(false)
	, isCurrentLayerOnly(false)
	, showTravelPath(true)
	, currentIndex(0)
	, pathTavelForDraw(nullptr)
{
	print_line_colors[PathMode::TRAVEL] = convertRGBtoMeshPoint(QColor(0, 50, 125));
	print_line_colors[PathMode::SKIRT] = convertRGBtoMeshPoint(QColor(0, 204, 204)); 
	print_line_colors[PathMode::SKIN] = convertRGBtoMeshPoint(QColor(204, 0, 204));
	print_line_colors[PathMode::WALL_INNER] = convertRGBtoMeshPoint(QColor(204, 204, 0));
	print_line_colors[PathMode::WALL_OUTER] = convertRGBtoMeshPoint(QColor(128, 0, 0));
	print_line_colors[PathMode::FILL] = convertRGBtoMeshPoint(QColor(0, 204, 0));
	print_line_colors[PathMode::SUPPORT_MAIN] = convertRGBtoMeshPoint(QColor(0, 178, 178));
	print_line_colors[PathMode::PRERETRACTION0] = convertRGBtoMeshPoint(QColor(128, 0, 0));
	print_line_colors[PathMode::PRERETRACTIONX] = convertRGBtoMeshPoint(QColor(204, 204, 0));
	print_line_colors[PathMode::OVERMOVE0] = convertRGBtoMeshPoint(QColor(128, 0, 0));
	print_line_colors[PathMode::OVERMOVEX] = convertRGBtoMeshPoint(QColor(204, 204, 0));
	print_line_colors[PathMode::RAFT] = convertRGBtoMeshPoint(QColor(0, 0, 255));
	print_line_colors[PathMode::SUPPORT_INTERFACE_ROOF] = convertRGBtoMeshPoint(QColor(192, 128, 0));
	print_line_colors[PathMode::SUPPORT_INTERFACE_FLOOR] = convertRGBtoMeshPoint(QColor(107, 188, 48));
	print_line_colors[PathMode::WIPE_TOWER] = convertRGBtoMeshPoint(QColor(128, 128, 192));
	print_line_colors[PathMode::ADJUST_Z_GAP] = convertRGBtoMeshPoint(QColor(102, 204, 0));
		
	print_rect_colors[PathMode::TRAVEL] = convertRGBtoMeshPoint(QColor(0, 50, 125));
	print_rect_colors[PathMode::SKIRT] = convertRGBtoMeshPoint(QColor(0, 255, 255));
	print_rect_colors[PathMode::SKIN] = convertRGBtoMeshPoint(QColor(255, 0, 255));
	print_rect_colors[PathMode::WALL_INNER] = convertRGBtoMeshPoint(QColor(255, 255, 0));
	print_rect_colors[PathMode::WALL_OUTER] = convertRGBtoMeshPoint(QColor(255, 0, 0));
	print_rect_colors[PathMode::FILL] = convertRGBtoMeshPoint(QColor(0, 255, 0));
	print_rect_colors[PathMode::SUPPORT_MAIN] = convertRGBtoMeshPoint(QColor(0, 230, 230));
	print_rect_colors[PathMode::PRERETRACTION0] = convertRGBtoMeshPoint(QColor(25, 25, 25));
	print_rect_colors[PathMode::PRERETRACTIONX] = convertRGBtoMeshPoint(QColor(255, 255, 0));
	print_rect_colors[PathMode::OVERMOVE0] = convertRGBtoMeshPoint(QColor(102, 0, 0));
	print_rect_colors[PathMode::OVERMOVEX] = convertRGBtoMeshPoint(QColor(255, 255, 0));
	print_rect_colors[PathMode::RAFT] = convertRGBtoMeshPoint(QColor(0, 0, 255));
	print_rect_colors[PathMode::SUPPORT_INTERFACE_ROOF] = convertRGBtoMeshPoint(QColor(255, 192, 65));
	print_rect_colors[PathMode::SUPPORT_INTERFACE_FLOOR] = convertRGBtoMeshPoint(QColor(172, 224, 133));
	print_rect_colors[PathMode::WIPE_TOWER] = convertRGBtoMeshPoint(QColor(153, 153, 217));
	print_rect_colors[PathMode::ADJUST_Z_GAP] = convertRGBtoMeshPoint(QColor(128, 255, 0));
	
	print_ex_colors[0] = convertRGBtoMeshPoint(QColor(128, 0, 0));
	print_ex_colors[1] = convertRGBtoMeshPoint(QColor(0, 204, 0));

	pathRenderer = new LineColorRenderer();
	pathBoldRenderer = new LineColorRenderer();
	pathTravelRenderer = new LineColorRenderer();
}

GCodePath::~GCodePath()
{
	for (auto it : paths)
		delete it;
	for (auto it : pathBolds)
		delete it;
	for (auto it : pathTravels)
		delete it;
	delete pathRenderer;
	delete pathBoldRenderer;
	delete pathTravelRenderer;
}

void GCodePath::updateGCodePath(std::vector<std::vector<GCodePathforDraw>> gcodePath_)
{
	//gcodePath = gcodePath_;
	/*gcodeLayers.clear();
	for (auto it : gcodePath_)
	{
		gcodeLayers.emplace_back(it);
	}*/
	int cnt = gcodePath_.size();
	for (auto it : paths)
		delete it;
	for (auto it : pathBolds)
		delete it;
	for (auto it : pathTravels)
		delete it;
	paths.clear();
	pathBolds.clear();
	pathTravels.clear();
	/*
	path.resize(cnt);
	pathBold.resize(cnt);
	pathTravel.resize(cnt);

	for (auto it : path)
		it = new Mesh();
	for (auto it : pathBold)
		it = new Mesh();
	for (auto it : pathTravel)
		it = new Mesh();
	for (auto it : pathRenderer)
		it = new LineColorRenderer();
	for (auto it : pathBoldRenderer)
		it = new LineColorRenderer();
	for (auto it : pathTravelRenderer)
		it = new LineColorRenderer();*/
	if (isExtruderMode)
		for (auto it : gcodePath_)
			updateBufferDataForEx(it);
	else
		for (auto it : gcodePath_)
			updateBufferData(it);
}
void GCodePath::updateBufferData(std::vector<GCodePathforDraw> gcodePath_)
{
	Mesh* path = new Mesh();
	Mesh* pathBold = new Mesh();
	Mesh* pathTravel = new Mesh();
	for (int j = 0; j < gcodePath_.size(); ++j)
	{
		if (gcodePath_[j].mode == TRAVEL)
		{
			Mesh::VertexHandle vh1 = pathTravel->add_vertex(gcodePath_[j].start);
			Mesh::VertexHandle vh2 = pathTravel->add_vertex(gcodePath_[j].end);
			pathTravel->set_color(vh1, print_line_colors[gcodePath_[j].mode]);
			pathTravel->set_color(vh2, print_line_colors[gcodePath_[j].mode]);
		}
		else
		{
			Mesh::VertexHandle vh1 = path->add_vertex(gcodePath_[j].start);
			Mesh::VertexHandle vh2 = path->add_vertex(gcodePath_[j].end);
			path->set_color(vh1, print_line_colors[gcodePath_[j].mode]);
			path->set_color(vh2, print_line_colors[gcodePath_[j].mode]);

			Mesh::VertexHandle vh3 = pathBold->add_vertex(gcodePath_[j].start);
			Mesh::VertexHandle vh4 = pathBold->add_vertex(gcodePath_[j].end);
			pathBold->set_color(vh3, print_rect_colors[gcodePath_[j].mode]);
			pathBold->set_color(vh4, print_rect_colors[gcodePath_[j].mode]);
		}
	}
	paths.push_back(path);
	pathBolds.push_back(pathBold);
	pathTravels.push_back(pathTravel);
}
void GCodePath::updateBufferDataForEx(std::vector<GCodePathforDraw> gcodePath_)
{
	Mesh* path = new Mesh();
	Mesh* pathBold = new Mesh();
	Mesh* pathTravel = new Mesh();
	for (int j = 0; j < gcodePath_.size(); ++j)
	{
		if (gcodePath_[j].mode == TRAVEL)
		{
			Mesh::VertexHandle vh1 = pathTravel->add_vertex(gcodePath_[j].start);
			Mesh::VertexHandle vh2 = pathTravel->add_vertex(gcodePath_[j].end);
			pathTravel->set_color(vh1, print_line_colors[gcodePath_[j].mode]);
			pathTravel->set_color(vh2, print_line_colors[gcodePath_[j].mode]);
		}
		else
		{
			Mesh::VertexHandle vh1 = path->add_vertex(gcodePath_[j].start);
			Mesh::VertexHandle vh2 = path->add_vertex(gcodePath_[j].end);
			path->set_color(vh1, print_ex_colors[gcodePath_[j].extruderNo]);
			path->set_color(vh2, print_ex_colors[gcodePath_[j].extruderNo]);

			Mesh::VertexHandle vh3 = pathBold->add_vertex(gcodePath_[j].start);
			Mesh::VertexHandle vh4 = pathBold->add_vertex(gcodePath_[j].end);
			pathBold->set_color(vh3, print_rect_colors[gcodePath_[j].extruderNo]);
			pathBold->set_color(vh4, print_rect_colors[gcodePath_[j].extruderNo]);
		}
	}
	paths.push_back(path);
	pathBolds.push_back(pathBold);
	pathTravels.push_back(pathTravel);
}

void GCodePath::drawOnScreenCanvas(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	MVPMatrices mvp(scaledFrame.getModelMatrix(), view_, proj_);
	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();

	//glDisable(GL_CULL_FACE);
	////glEnable(GL_CULL_FACE);
	//glEnable(GL_POLYGON_OFFSET_FILL);
	//glEnable(GL_POLYGON_OFFSET_LINE);
	//glEnable(GL_LINE_SMOOTH);

	glFuncs->glDepthMask(true);
	glFuncs->glEnable(GL_CULL_FACE);
	glFuncs->glDisable(GL_LINE_SMOOTH);
	glFuncs->glDisable(GL_LIGHTING);

	glFuncs->glLineWidth(3.0);
	pathBoldRenderer->draw(pathBoldsForDraw, mvp);
	glFuncs->glLineWidth(1.0);
	pathRenderer->draw(pathsForDraw, mvp);
	pathTravelRenderer->draw(pathTavelForDraw, mvp);
	if (pathTavelForDraw != nullptr)
		pathTravelRenderer->draw(pathTavelForDraw, mvp);
	glFuncs->glDisable(GL_CULL_FACE);
}

void GCodePath::setCurrentLayer(int layerNo_)
{
	if (layerNo_ < 0)
		layerNo_ = 0;
	else if (layerNo_ >= paths.size())
		layerNo_ = paths.size() - 1;

	currentIndex = layerNo_;
	updateMeshForDraw();
}

void GCodePath::toggleExtruderMode(bool flag_)
{
	isExtruderMode = flag_;
	//UpdateColorData();
}
void GCodePath::toggleCurrentLayerOnly(bool flag_)
{
	isCurrentLayerOnly = flag_;
	updateMeshForDraw();
}
void GCodePath::toggleShowTravelPath(bool flag_)
{
	showTravelPath = flag_;
	if (pathTravels.size() > 0)
	{
		if (showTravelPath)
			pathTavelForDraw = pathTravels[currentIndex];
		else
			pathTavelForDraw = nullptr;
	}
}
void GCodePath::updateMeshForDraw()
{
	if (currentIndex < 0)
		return;
	pathsForDraw.clear();
	pathBoldsForDraw.clear();
	pathRenderer->deleteReference();
	pathBoldRenderer->deleteReference();

	if (pathBolds.size() > 0)
	{
		if (isCurrentLayerOnly)
			pathBoldsForDraw.push_back(pathBolds[currentIndex]);
		else
		{
			for (int i = 0; i <= currentIndex; i++)
			{
				if (currentIndex - 3 < i)
					pathBoldsForDraw.push_back(pathBolds[i]);
				else
					pathsForDraw.push_back(paths[i]);
			}
		}
	}
	if (pathTravels.size() > 0)
	{
		if (showTravelPath)
			pathTavelForDraw = pathTravels[currentIndex];
		else
			pathTavelForDraw = nullptr;
	}
}

Mesh::Point GCodePath::convertRGBtoMeshPoint(QColor _color)
{
	return Mesh::Point(_color.redF(), _color.greenF(), _color.blueF());
}