#ifndef GEOMETRYBUILDER_H
#define GEOMETRYBUILDER_H

#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>
#include <Quantity_Color.hxx>
#include <Quantity_NameOfColor.hxx>

class GeometryBuilder
{
public:
    explicit GeometryBuilder(const Handle(AIS_InteractiveContext)& context);
    ~GeometryBuilder();

    // Create structural elements
    Handle(AIS_Shape) createBeam(double x, double y, double z, 
                                  double length, double width, double height,
                                  const Quantity_Color& color = Quantity_Color(0.5, 0.5, 0.5, Quantity_TOC_RGB));
    
    Handle(AIS_Shape) createColumn(double x, double y, double z,
                                     double width, double depth, double height,
                                     const Quantity_Color& color = Quantity_Color(0.6, 0.6, 0.6, Quantity_TOC_RGB));
    
    Handle(AIS_Shape) createSlab(double x, double y, double z,
                                  double length, double width, double thickness,
                                  const Quantity_Color& color = Quantity_Color(0.8, 0.8, 0.8, Quantity_TOC_RGB));
    
    Handle(AIS_Shape) createWall(double x, double y, double z,
                                  double length, double thickness, double height,
                                  const Quantity_Color& color = Quantity_Color(0.96, 0.96, 0.86, Quantity_TOC_RGB));
    
    Handle(AIS_Shape) createFoundation(double x, double y, double z,
                                        double length, double width, double height,
                                        const Quantity_Color& color = Quantity_Color(0.4, 0.4, 0.4, Quantity_TOC_RGB));

    // Create basic shapes
    Handle(AIS_Shape) createBox(const gp_Pnt& position, 
                                 double dx, double dy, double dz,
                                 const Quantity_Color& color = Quantity_Color(1.0, 1.0, 0.0, Quantity_TOC_RGB));
    
    Handle(AIS_Shape) createCylinder(const gp_Pnt& position,
                                      double radius, double height,
                                      const Quantity_Color& color = Quantity_Color(0.0, 0.0, 1.0, Quantity_TOC_RGB));

    // I-Beam profile
    Handle(AIS_Shape) createIBeam(double x, double y, double z,
                                   double length, double height, double flangeWidth,
                                   double webThickness, double flangeThickness,
                                   const Quantity_Color& color = Quantity_Color(0.5, 0.5, 0.5, Quantity_TOC_RGB));

private:
    Handle(AIS_InteractiveContext) m_context;
    
    // Helper methods
    void displayShape(const Handle(AIS_Shape)& aisShape, const Quantity_Color& color);
    TopoDS_Shape createBoxShape(const gp_Pnt& position, double dx, double dy, double dz);
};

#endif // GEOMETRYBUILDER_H
