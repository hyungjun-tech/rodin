#include "stdafx.h"
#include "PrinterBody.h"
#include "MeshLoader.h"
#include "UserProperties.h"
#include "AABB.h"

class MeshLoader;

PrinterBody::PrinterBody()
	: panel(nullptr),
	bed1(nullptr),
	bed2(nullptr),
	frontBar(nullptr),
	bedOffset(nullptr),

	bed1Renderer(nullptr),
	bed2Renderer(nullptr),
	panelRenderer(nullptr),
	frontBarRenderer(nullptr),
	bedOffsetRenderer(nullptr),
	printLimitRange_clip_L(nullptr),
	printLimitRange_clip_R(nullptr),
	printLimitRange_upper(nullptr),
	printLimitRange_clip_Renderer(nullptr),
	printLimitRange_upper_Renderer(nullptr),

	gridGap(10),
	zSinkOffset(-0.01)
{
}

PrinterBody::~PrinterBody()
{
	delete panel;
	delete bed1;
	delete bed2;
	delete frontBar;
	if (bedOffset)
		delete bedOffset;

	delete panelRenderer;
	delete bed1Renderer;
	delete bed2Renderer;
	delete frontBarRenderer;
	delete bedOffsetRenderer;

	delete printLimitRange_clip_L;
	delete printLimitRange_clip_R;
	delete printLimitRange_upper;
	delete printLimitRange_clip_Renderer;
	delete printLimitRange_upper_Renderer;
}
void PrinterBody::setSize()
{
	setSize(Profile::getMachineAABB(),
		Profile::machineProfile.machine_width_offset.value,
		Profile::machineProfile.machine_depth_offset.value);
}
void PrinterBody::init()
{
	panelRenderer = new LineRenderer();
	panelRenderer->setColor(1.0, 1.0, 1.0, 0.8);
	bed1Renderer = new NonShadedRenderer();
	bed2Renderer = new NonShadedRenderer();
	bed1Renderer->setColor(0.4, 0.4, 0.4, 0.3);
	bed2Renderer->setColor(0.12, 0.12, 0.12, 0.3);
	frontBarRenderer = new NonShadedRenderer();
	frontBarRenderer->setColor(0.5, 0.5, 0.5, 0.8);
	printLimitRange_clip_Renderer = new PhongShadedRenderer();
	printLimitRange_clip_Renderer->setColor(0.1, 0.1, 0.1, 0.4);

	printLimitRange_upper_Renderer = new NonShadedRenderer();
	printLimitRange_upper_Renderer->setColor(0.7, 0.7, 0.7, 0.2);
	bedOffsetRenderer = new NonShadedRenderer();
	bedOffsetRenderer->setColor(0.1, 0.1, 0.1, 0.2);

	setSize();
}
void PrinterBody::setSize(AABB machineBox, float offsetx_, float offsety_)
{
	width = machineBox.getLengthX();
	length = machineBox.getLengthY();
	height = machineBox.getLengthZ();
	offsetx = offsetx_;
	offsety = offsety_;
	qDebug() << "width : " << width << " : length : " << length << " : height : " << height;
	qDebug() << "offsetx : " << offsetx << " : offsety : " << offsety;

	/*bed1Renderer->DeleteReference();
	bed2Renderer->DeleteReference();
	panelRenderer->DeleteReference();
	frontBarRenderer->DeleteReference();
	printLimitRange_clip_Renderer->DeleteReference();
	printLimitRange_upper_Renderer->DeleteReference();
	bedOffsetRenderer->DeleteReference();*/

	createBed();
	createPanel();
	createFrontBar();
	creatPrintLimitRange();
	creatBedOffset();
}

void PrinterBody::drawOnScreenCanvas(QMatrix4x4 proj_, QMatrix4x4 view_)
{
	MVPMatrices mvp(scaledFrame.getModelMatrix(), view_, proj_);
	QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();

	glFuncs->glLineWidth(2.0);
	panelRenderer->draw(panel, mvp);

	bed1Renderer->draw(bed1, mvp);
	bed2Renderer->draw(bed2, mvp);

	frontBarRenderer->draw(frontBar, mvp);	

	bedOffsetRenderer->draw(bedOffset, mvp);

	glFuncs->glEnable(GL_CULL_FACE);
	glFuncs->glCullFace(GL_BACK);
	printLimitRange_clip_Renderer->draw(printLimitRange_clip_L, mvp);
	printLimitRange_clip_Renderer->draw(printLimitRange_clip_R, mvp);

	printLimitRange_upper_Renderer->draw(printLimitRange_upper, mvp);
	glFuncs->glDisable(GL_CULL_FACE);
}

void PrinterBody::createBed()
{
	if (bed1)
		delete bed1;
	if (bed2)
		delete bed2;

	bed1Renderer->deleteReference();
	bed2Renderer->deleteReference();
	
	//----------------generate check typ bed mesh
	bed1 = new Mesh();
	bed2 = new Mesh();

	int num_gridX = (width + 9) / gridGap;
	int num_gridY = (length + 9) / gridGap;
	for (int x = 0; x < num_gridX; ++x)
	{
		for (int y = 0; y < num_gridY; ++y)
		{
			float x0, x1, y0, y1;
			x0 = x * gridGap;
			x1 = ((x + 1) * gridGap > width ? width : (x + 1) * gridGap);
			y0 = y * gridGap;
			y1 = ((y + 1) * gridGap > length ? length : (y + 1) * gridGap);
			Mesh::Point p0(offsetx + x0, offsety + y0, zSinkOffset);
			Mesh::Point p1(offsetx + x1, offsety + y0, zSinkOffset);
			Mesh::Point p2(offsetx + x1, offsety + y1, zSinkOffset);
			Mesh::Point p3(offsetx + x0, offsety + y1, zSinkOffset);
			if ((x + y) % 2 == 0)
			{
				std::vector<Mesh::VertexHandle> vhs;
				vhs.push_back(bed1->add_vertex(p0));
				vhs.push_back(bed1->add_vertex(p1));
				vhs.push_back(bed1->add_vertex(p2));
				vhs.push_back(bed1->add_vertex(p3));

				std::vector<Mesh::VertexHandle> face_vhandles;
				face_vhandles.push_back(vhs[0]);
				face_vhandles.push_back(vhs[1]);
				face_vhandles.push_back(vhs[2]);
				bed1->add_face(face_vhandles);

				face_vhandles.clear();
				face_vhandles.push_back(vhs[0]);
				face_vhandles.push_back(vhs[2]);
				face_vhandles.push_back(vhs[3]);
				bed1->add_face(face_vhandles);
			}
			else
			{
				std::vector<Mesh::VertexHandle> vhs;
				vhs.push_back(bed2->add_vertex(p0));
				vhs.push_back(bed2->add_vertex(p1));
				vhs.push_back(bed2->add_vertex(p2));
				vhs.push_back(bed2->add_vertex(p3));

				std::vector<Mesh::VertexHandle> face_vhandles;
				face_vhandles.push_back(vhs[0]);
				face_vhandles.push_back(vhs[1]);
				face_vhandles.push_back(vhs[2]);
				bed2->add_face(face_vhandles);

				face_vhandles.clear();
				face_vhandles.push_back(vhs[0]);
				face_vhandles.push_back(vhs[2]);
				face_vhandles.push_back(vhs[3]);
				bed2->add_face(face_vhandles);
			}
		}
	}	
}

void PrinterBody::createPanel()
{
	delete panel;
	panel = new Mesh();
	panelRenderer->deleteReference();

	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			for (int k = 0; k < 2; k++)
			{
				panel->add_vertex(Mesh::Point(offsetx + i*width, offsety + j*length, k*height));		//상하 방향
			}

	for (int j = 0; j < 2; j++)
		for (int k = 0; k < 2; k++)			
			for (int i = 0; i < 2; i++)
			{
				panel->add_vertex(Mesh::Point(offsetx + i*width, offsety + j*length, k*height));		//좌우 방향			
			}
	/*panel->add_vertex(Mesh::Point(0, 0, 0));
	panel->add_vertex(Mesh::Point(width, 0, 0));	
	panel->add_vertex(Mesh::Point(0, length, 0));
	panel->add_vertex(Mesh::Point(width, length, 0));
	panel->add_vertex(Mesh::Point(0, 0, height));
	panel->add_vertex(Mesh::Point(width, 0, height));
	panel->add_vertex(Mesh::Point(0, length, height));
	panel->add_vertex(Mesh::Point(width, length, height));*/

	for (int k = 0; k < 2; k++)
		for (int i = 0; i < 2; i++)
			for (int j = 0; j < 2; j++) {
				panel->add_vertex(Mesh::Point(offsetx + i*width, offsety + j*length, k*height));		//전후 방향
			}
}

void PrinterBody::createFrontBar()
{
	delete frontBar;
	frontBar = new Mesh();
	frontBarRenderer->deleteReference();

	float margin = gridGap*0.1;
	float lineWidth = gridGap*0.3;
	Mesh::Point p0(0, -margin, 0);
	Mesh::Point p1(offsetx * 2 + width, -margin, 0);
	Mesh::Point p2(offsetx * 2 + width + margin, -margin - lineWidth, 0);
	//Mesh::Point p3(0 - margin, -margin - lineWidth, 0);
	Mesh::Point p3(-margin, -margin - lineWidth, 0);

	std::vector<Mesh::VertexHandle> vhs;
	vhs.push_back(frontBar->add_vertex(p0));
	vhs.push_back(frontBar->add_vertex(p1));
	vhs.push_back(frontBar->add_vertex(p2));
	vhs.push_back(frontBar->add_vertex(p3));

	std::vector<Mesh::VertexHandle> face_vhandles;
	face_vhandles.push_back(vhs[0]);
	face_vhandles.push_back(vhs[1]);
	face_vhandles.push_back(vhs[2]);
	frontBar->add_face(face_vhandles);

	face_vhandles.clear();
	face_vhandles.push_back(vhs[0]);
	face_vhandles.push_back(vhs[2]);
	face_vhandles.push_back(vhs[3]);
	frontBar->add_face(face_vhandles);

	p0 = Mesh::Point(offsetx + width / 2.0 - gridGap*0.5, -(margin + lineWidth), 0);
	p1 = Mesh::Point(offsetx + width / 2.0 + gridGap*0.5, -(margin + lineWidth), 0);
	p2 = Mesh::Point(offsetx + width / 2.0, -margin - gridGap*sqrt(3)*0.5, 0);
	vhs.clear();
	face_vhandles.clear();
	vhs.push_back(frontBar->add_vertex(p0));
	vhs.push_back(frontBar->add_vertex(p1));
	vhs.push_back(frontBar->add_vertex(p2));
	face_vhandles.push_back(vhs[0]);
	face_vhandles.push_back(vhs[1]);
	face_vhandles.push_back(vhs[2]);
	frontBar->add_face(face_vhandles);
}

void PrinterBody::creatPrintLimitRange()
{
	delete printLimitRange_clip_L;
	delete printLimitRange_clip_R;
	delete printLimitRange_upper;

	if (!Profile::machineProfile.restrict_rearQuadRegion_enabled.value)
	{
		printLimitRange_clip_L = nullptr;
		printLimitRange_clip_R = nullptr;
		printLimitRange_upper = nullptr;
		return;
	}
	qglviewer::Vec rearQuad = Profile::getRearQuad();
	qglviewer::Vec upperBox = Profile::getUpperRestrictedBox();
	int upper_H = Profile::getUpperRestrictedBox()[2];
	/// loading clip model mesh file.. not drawing..//
	MeshLoader loader;
	printLimitRange_clip_L = loader.loadFirstMesh(Generals::appPath + "/mesh/clip_DP200.stl");
	printLimitRange_clip_R = new Mesh(*printLimitRange_clip_L);//같은거니깐 복사해서 사용 loader.LoadFirstMesh(Generals::getAppPath() + "/mesh/clip_DP200.stl");
	printLimitRange_clip_L->translate(qglviewer::Vec(0, length - rearQuad[1], 0));
	printLimitRange_clip_R->translate(qglviewer::Vec(width - rearQuad[0], length - rearQuad[1], 0));

	printLimitRange_upper = loader.loadFirstMesh(Generals::appPath + "/mesh/upper_bar_DP200.stl");
	printLimitRange_upper->translate(qglviewer::Vec(0, 0, height - upper_H));

	printLimitRange_clip_Renderer->deleteReference();
	printLimitRange_upper_Renderer->deleteReference();

	/*
	delete printLimitRange_clip;
	printLimitRange_clip = new Mesh();
	

	//width(width_),
	//length(length_),
	//height(height_),

	/// QUAD_left
	Mesh::Point p0(0, length - quadH, 0.1);
	Mesh::Point p1(quadW, length - quadH, 0.1);
	Mesh::Point p2(quadW, length, 0.1);
	Mesh::Point p3(0, length, 0.1);

	std::vector<Mesh::VertexHandle> vhs;
	vhs.push_back(printLimitRange_clip->add_vertex(p0));
	vhs.push_back(printLimitRange_clip->add_vertex(p1));
	vhs.push_back(printLimitRange_clip->add_vertex(p2));
	vhs.push_back(printLimitRange_clip->add_vertex(p3));

	std::vector<Mesh::VertexHandle> face_vhandles;
	face_vhandles.push_back(vhs[0]);
	face_vhandles.push_back(vhs[1]);
	face_vhandles.push_back(vhs[2]);
	printLimitRange_clip->add_face(face_vhandles);

	face_vhandles.clear();
	face_vhandles.push_back(vhs[0]);
	face_vhandles.push_back(vhs[2]);
	face_vhandles.push_back(vhs[3]);
	printLimitRange_clip->add_face(face_vhandles);


	/// QUAD_right
	p0 += Mesh::Point(width - quadW, 0, 0);
	p1 += Mesh::Point(width - quadW, 0, 0);
	p2 += Mesh::Point(width - quadW, 0, 0);
	p3 += Mesh::Point(width - quadW, 0, 0);
		
	vhs.clear();
	vhs.push_back(printLimitRange_clip->add_vertex(p0));
	vhs.push_back(printLimitRange_clip->add_vertex(p1));
	vhs.push_back(printLimitRange_clip->add_vertex(p2));
	vhs.push_back(printLimitRange_clip->add_vertex(p3));

	face_vhandles.clear();
	face_vhandles.push_back(vhs[0]);
	face_vhandles.push_back(vhs[1]);
	face_vhandles.push_back(vhs[2]);
	printLimitRange_clip->add_face(face_vhandles);

	face_vhandles.clear();
	face_vhandles.push_back(vhs[0]);
	face_vhandles.push_back(vhs[2]);
	face_vhandles.push_back(vhs[3]);
	printLimitRange_clip->add_face(face_vhandles);
	*/
}

void PrinterBody::creatBedOffset()
{
	if (bedOffset)
	{
		delete bedOffset;
		bedOffset = nullptr;
	}
	if (offsetx == 0 && offsety == 0)
		return;
	bedOffset = new Mesh();
	bedOffsetRenderer->deleteReference();

	Mesh::Point p00(0, 0, zSinkOffset);
	Mesh::Point p01(0, offsety + length + offsety, zSinkOffset);
	Mesh::Point p11(offsetx + width + offsetx, offsety + length + offsety, zSinkOffset);
	Mesh::Point p10(offsetx + width + offsetx, 0, zSinkOffset);
	Mesh::Point p22(offsetx, offsety, zSinkOffset);
	Mesh::Point p23(offsetx, offsety + length, zSinkOffset);
	Mesh::Point p33(offsetx + width, offsety + length, zSinkOffset);
	Mesh::Point p32(offsetx + width, offsety, zSinkOffset);

	std::vector<Mesh::VertexHandle> vhs;
	vhs.push_back(bedOffset->add_vertex(p00)); //0
	vhs.push_back(bedOffset->add_vertex(p01)); //1
	vhs.push_back(bedOffset->add_vertex(p11)); //2
	vhs.push_back(bedOffset->add_vertex(p10)); //3
	vhs.push_back(bedOffset->add_vertex(p22)); //4
	vhs.push_back(bedOffset->add_vertex(p23)); //5
	vhs.push_back(bedOffset->add_vertex(p33)); //6
	vhs.push_back(bedOffset->add_vertex(p32)); //7

	std::vector<Mesh::VertexHandle> face_vhandles;
	face_vhandles.push_back(vhs[0]);
	face_vhandles.push_back(vhs[4]);
	face_vhandles.push_back(vhs[5]);
	bedOffset->add_face(face_vhandles);

	face_vhandles.clear();
	face_vhandles.push_back(vhs[1]);
	face_vhandles.push_back(vhs[5]);
	face_vhandles.push_back(vhs[6]);
	bedOffset->add_face(face_vhandles);

	face_vhandles.clear();
	face_vhandles.push_back(vhs[2]);
	face_vhandles.push_back(vhs[6]);
	face_vhandles.push_back(vhs[7]);
	bedOffset->add_face(face_vhandles);

	face_vhandles.clear();
	face_vhandles.push_back(vhs[3]);
	face_vhandles.push_back(vhs[7]);
	face_vhandles.push_back(vhs[4]);
	bedOffset->add_face(face_vhandles);

	face_vhandles.clear();
	face_vhandles.push_back(vhs[0]);
	face_vhandles.push_back(vhs[3]);
	face_vhandles.push_back(vhs[4]);
	bedOffset->add_face(face_vhandles);

	face_vhandles.clear();
	face_vhandles.push_back(vhs[3]);
	face_vhandles.push_back(vhs[2]);
	face_vhandles.push_back(vhs[7]);
	bedOffset->add_face(face_vhandles);

	face_vhandles.clear();
	face_vhandles.push_back(vhs[2]);
	face_vhandles.push_back(vhs[1]);
	face_vhandles.push_back(vhs[6]);
	bedOffset->add_face(face_vhandles);

	face_vhandles.clear();
	face_vhandles.push_back(vhs[1]);
	face_vhandles.push_back(vhs[0]);
	face_vhandles.push_back(vhs[5]);
	bedOffset->add_face(face_vhandles);
}