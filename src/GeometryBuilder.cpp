#include "GeometryBuilder.h"
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <gp_Vec.hxx>
#include <gp_Ax2.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Face.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <GC_MakeSegment.hxx>
#include <Geom_TrimmedCurve.hxx>

GeometryBuilder::GeometryBuilder(const Handle(AIS_InteractiveContext)& context)
    : m_context(context)
{
}

GeometryBuilder::~GeometryBuilder()
{
}

void GeometryBuilder::displayShape(const Handle(AIS_Shape)& aisShape, const Quantity_Color& color)
{
    if (!m_context.IsNull() && !aisShape.IsNull()) {
        m_context->SetColor(aisShape, color, Standard_False);
        m_context->SetMaterial(aisShape, Graphic3d_NOM_ALUMINIUM, Standard_False);
        m_context->Display(aisShape, Standard_True);
    }
}

TopoDS_Shape GeometryBuilder::createBoxShape(const gp_Pnt& position, double dx, double dy, double dz)
{
    BRepPrimAPI_MakeBox boxMaker(position, dx, dy, dz);
    return boxMaker.Shape();
}

Handle(AIS_Shape) GeometryBuilder::createBeam(double x, double y, double z,
                                                double length, double width, double height,
                                                const Quantity_Color& color)
{
    gp_Pnt position(x, y, z);
    TopoDS_Shape beamShape = createBoxShape(position, length, width, height);
    
    Handle(AIS_Shape) aisBeam = new AIS_Shape(beamShape);
    displayShape(aisBeam, color);
    
    return aisBeam;
}

Handle(AIS_Shape) GeometryBuilder::createColumn(double x, double y, double z,
                                                 double width, double depth, double height,
                                                 const Quantity_Color& color)
{
    gp_Pnt position(x, y, z);
    TopoDS_Shape columnShape = createBoxShape(position, width, depth, height);
    
    Handle(AIS_Shape) aisColumn = new AIS_Shape(columnShape);
    displayShape(aisColumn, color);
    
    return aisColumn;
}

Handle(AIS_Shape) GeometryBuilder::createSlab(double x, double y, double z,
                                                double length, double width, double thickness,
                                                const Quantity_Color& color)
{
    gp_Pnt position(x, y, z);
    TopoDS_Shape slabShape = createBoxShape(position, length, width, thickness);
    
    Handle(AIS_Shape) aisSlab = new AIS_Shape(slabShape);
    displayShape(aisSlab, color);
    
    return aisSlab;
}

Handle(AIS_Shape) GeometryBuilder::createWall(double x, double y, double z,
                                                double length, double thickness, double height,
                                                const Quantity_Color& color)
{
    gp_Pnt position(x, y, z);
    TopoDS_Shape wallShape = createBoxShape(position, length, thickness, height);
    
    Handle(AIS_Shape) aisWall = new AIS_Shape(wallShape);
    displayShape(aisWall, color);
    
    return aisWall;
}

Handle(AIS_Shape) GeometryBuilder::createFoundation(double x, double y, double z,
                                                      double length, double width, double height,
                                                      const Quantity_Color& color)
{
    gp_Pnt position(x, y, z);
    TopoDS_Shape foundationShape = createBoxShape(position, length, width, height);
    
    Handle(AIS_Shape) aisFoundation = new AIS_Shape(foundationShape);
    displayShape(aisFoundation, color);
    
    return aisFoundation;
}

Handle(AIS_Shape) GeometryBuilder::createBox(const gp_Pnt& position,
                                               double dx, double dy, double dz,
                                               const Quantity_Color& color)
{
    TopoDS_Shape boxShape = createBoxShape(position, dx, dy, dz);
    
    Handle(AIS_Shape) aisBox = new AIS_Shape(boxShape);
    displayShape(aisBox, color);
    
    return aisBox;
}

Handle(AIS_Shape) GeometryBuilder::createCylinder(const gp_Pnt& position,
                                                    double radius, double height,
                                                    const Quantity_Color& color)
{
    gp_Ax2 axis(position, gp_Dir(0, 0, 1));
    BRepPrimAPI_MakeCylinder cylinderMaker(axis, radius, height);
    TopoDS_Shape cylinderShape = cylinderMaker.Shape();
    
    Handle(AIS_Shape) aisCylinder = new AIS_Shape(cylinderShape);
    displayShape(aisCylinder, color);
    
    return aisCylinder;
}

Handle(AIS_Shape) GeometryBuilder::createIBeam(double x, double y, double z,
                                                 double length, double height, double flangeWidth,
                                                 double webThickness, double flangeThickness,
                                                 const Quantity_Color& color)
{
    // Create I-beam profile as a 2D sketch, then extrude it
    
    // Define profile points for I-beam cross-section (simplified)
    // Bottom flange
    gp_Pnt p1(0, 0, 0);
    gp_Pnt p2(flangeWidth, 0, 0);
    gp_Pnt p3(flangeWidth, flangeThickness, 0);
    
    // Web bottom connection
    double webOffset = (flangeWidth - webThickness) / 2.0;
    gp_Pnt p4(webOffset + webThickness, flangeThickness, 0);
    gp_Pnt p5(webOffset + webThickness, height - flangeThickness, 0);
    
    // Top flange
    gp_Pnt p6(flangeWidth, height - flangeThickness, 0);
    gp_Pnt p7(flangeWidth, height, 0);
    gp_Pnt p8(0, height, 0);
    gp_Pnt p9(0, height - flangeThickness, 0);
    
    // Web top connection
    gp_Pnt p10(webOffset, height - flangeThickness, 0);
    gp_Pnt p11(webOffset, flangeThickness, 0);
    
    // Create edges
    TopoDS_Edge e1 = BRepBuilderAPI_MakeEdge(p1, p2);
    TopoDS_Edge e2 = BRepBuilderAPI_MakeEdge(p2, p3);
    TopoDS_Edge e3 = BRepBuilderAPI_MakeEdge(p3, p4);
    TopoDS_Edge e4 = BRepBuilderAPI_MakeEdge(p4, p5);
    TopoDS_Edge e5 = BRepBuilderAPI_MakeEdge(p5, p6);
    TopoDS_Edge e6 = BRepBuilderAPI_MakeEdge(p6, p7);
    TopoDS_Edge e7 = BRepBuilderAPI_MakeEdge(p7, p8);
    TopoDS_Edge e8 = BRepBuilderAPI_MakeEdge(p8, p9);
    TopoDS_Edge e9 = BRepBuilderAPI_MakeEdge(p9, p10);
    TopoDS_Edge e10 = BRepBuilderAPI_MakeEdge(p10, p11);
    TopoDS_Edge e11 = BRepBuilderAPI_MakeEdge(p11, p1);
    
    // Create wire from edges
    BRepBuilderAPI_MakeWire wireMaker;
    wireMaker.Add(e1);
    wireMaker.Add(e2);
    wireMaker.Add(e3);
    wireMaker.Add(e4);
    wireMaker.Add(e5);
    wireMaker.Add(e6);
    wireMaker.Add(e7);
    wireMaker.Add(e8);
    wireMaker.Add(e9);
    wireMaker.Add(e10);
    wireMaker.Add(e11);
    
    TopoDS_Wire wire = wireMaker.Wire();
    
    // Create face from wire
    TopoDS_Face profile = BRepBuilderAPI_MakeFace(wire);
    
    // Extrude the profile to create the beam
    gp_Vec extrusionVec(length, 0, 0);
    BRepPrimAPI_MakePrism prismMaker(profile, extrusionVec);
    TopoDS_Shape beamShape = prismMaker.Shape();
    
    // Translate to the desired position
    gp_Trsf transformation;
    transformation.SetTranslation(gp_Vec(x, y - flangeWidth/2, z));
    beamShape.Move(TopLoc_Location(transformation));
    
    Handle(AIS_Shape) aisBeam = new AIS_Shape(beamShape);
    displayShape(aisBeam, color);
    
    return aisBeam;
}
