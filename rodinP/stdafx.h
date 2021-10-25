#pragma once
//Project Headers
#include "AppInfo.h"
#include "Logger.h"
#include "Generals.h"
#include "Profile.h"
#include "CommonDialog.h"
#include "CommonMessages.h"
#include "UserProperties.h"

// Standart C++ Headers ---------------------------------------------------------
#include <vector>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <omp.h>
#include <cmath>

// libQGLViewer Headers ---------------------------------------------------------
#include <QGLViewer/qglviewer.h>
#include <QGLViewer/camera.h>
#include <QGLViewer/manipulatedCameraFrame.h>
#include <QGLViewer/frame.h>
#include <QGLViewer/manipulatedFrame.h>
#include <QGLViewer/constraint.h>


// Qt Headers -------------------------------------------------------------------
#include <QtCore/QCoreApplication> 
#include <QtCore/QPoint>
#include <QtCore/QEvent>
#include <QtCore/QDebug>

#include <QtGui/QOpenGLVertexArrayObject>
#include <QtGui/QOpenGLFramebufferObject>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLFunctions>
#include <QtGui/QOpenGLBuffer>

#include <QtWidgets/QtWidgets>
#include <QtNetwork/QtNetwork>
#include <QtXML/QtXml>

#include <QtGui/QMatrix4x4>
#include <QtGui/QVector3D>
#include <QtOpenGL/QGLFramebufferObject>


// Cryptopp Headers
#include <Cryptopp/filters.h>
#include <Cryptopp/pkcspad.h>
#include <Cryptopp/modes.h>
#include <Cryptopp/aes.h>
#include <Cryptopp/base64.h>

// FileLoader Headers
#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/ProgressHandler.hpp>
#include <lib3mf/lib3mf_implicit.hpp>

// OpenMesh Headers -------------------------------------------------------------
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>

// OpenCV Headers ---------------------------------------------------------------
//#include <opencv2/opencv.hpp>
//
//#include <opencv2/core/core.hpp>
//#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/imgproc/imgproc.hpp>

// CGAL Headers -----------------------------------------------------------------
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/convex_hull_3.h>

#include <CGAL/Polygon_mesh_processing/triangulate_hole.h>
#include <CGAL/Polygon_mesh_processing/corefinement.h>
#include <CGAL/Polygon_mesh_processing/remesh.h>

#include <CGAL/boost/graph/selection.h>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_triangle_primitive.h>
#include <CGAL/Homogeneous.h>
#include <CGAL/Polyhedron_traits_with_normals_3.h>
#include <CGAL/algorithm.h>
#include <CGAL/Polyhedron_3.h>

// boost Headers -----------------------------------------------------------------
#include <boost/config.hpp>
#include <boost/utility.hpp>                // for boost::tie
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/exterior_property.hpp>
#include <boost/graph/property_maps/constant_property_map.hpp>
#include <boost/graph/floyd_warshall_shortest.hpp>
#include <boost/graph/closeness_centrality.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/assume_abstract.hpp>