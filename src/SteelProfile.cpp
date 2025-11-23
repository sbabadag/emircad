#include "SteelProfile.h"
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>
#include <gp_Trsf.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax3.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepFilletAPI_MakeFillet2d.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Wire.hxx>
#include <TopExp_Explorer.hxx>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <cmath>

QMap<QString, SteelProfile::Dimensions> SteelProfile::s_ipeProfiles;
QMap<QString, SteelProfile::Dimensions> SteelProfile::s_heaProfiles;
QMap<QString, SteelProfile::Dimensions> SteelProfile::s_hebProfiles;
QMap<QString, SteelProfile::Dimensions> SteelProfile::s_hemProfiles;
QMap<QString, SteelProfile::Dimensions> SteelProfile::s_rhsProfiles;
bool SteelProfile::s_initialized = false;

void SteelProfile::initializeProfiles()
{
    if (s_initialized) return;
    
    // IPE Profiles (European I-beams)
    s_ipeProfiles["IPE 80"]  = {80,  46,  3.8, 5.2, 5, 0};
    s_ipeProfiles["IPE 100"] = {100, 55,  4.1, 5.7, 7, 0};
    s_ipeProfiles["IPE 120"] = {120, 64,  4.4, 6.3, 7, 0};
    s_ipeProfiles["IPE 140"] = {140, 73,  4.7, 6.9, 7, 0};
    s_ipeProfiles["IPE 160"] = {160, 82,  5.0, 7.4, 9, 0};
    s_ipeProfiles["IPE 180"] = {180, 91,  5.3, 8.0, 9, 0};
    s_ipeProfiles["IPE 200"] = {200, 100, 5.6, 8.5, 12, 0};
    s_ipeProfiles["IPE 220"] = {220, 110, 5.9, 9.2, 12, 0};
    s_ipeProfiles["IPE 240"] = {240, 120, 6.2, 9.8, 15, 0};
    s_ipeProfiles["IPE 270"] = {270, 135, 6.6, 10.2, 15, 0};
    s_ipeProfiles["IPE 300"] = {300, 150, 7.1, 10.7, 15, 0};
    s_ipeProfiles["IPE 330"] = {330, 160, 7.5, 11.5, 18, 0};
    s_ipeProfiles["IPE 360"] = {360, 170, 8.0, 12.7, 18, 0};
    s_ipeProfiles["IPE 400"] = {400, 180, 8.6, 13.5, 21, 0};
    s_ipeProfiles["IPE 450"] = {450, 190, 9.4, 14.6, 21, 0};
    s_ipeProfiles["IPE 500"] = {500, 200, 10.2, 16.0, 21, 0};
    s_ipeProfiles["IPE 550"] = {550, 210, 11.1, 17.2, 24, 0};
    s_ipeProfiles["IPE 600"] = {600, 220, 12.0, 19.0, 24, 0};
    
    // HEA Profiles (European wide flange - light)
    s_heaProfiles["HEA 100"] = {96,  100, 5.0, 8.0,  12, 0};
    s_heaProfiles["HEA 120"] = {114, 120, 5.0, 8.0,  12, 0};
    s_heaProfiles["HEA 140"] = {133, 140, 5.5, 8.5,  12, 0};
    s_heaProfiles["HEA 160"] = {152, 160, 6.0, 9.0,  15, 0};
    s_heaProfiles["HEA 180"] = {171, 180, 6.0, 9.5,  15, 0};
    s_heaProfiles["HEA 200"] = {190, 200, 6.5, 10.0, 18, 0};
    s_heaProfiles["HEA 220"] = {210, 220, 7.0, 11.0, 18, 0};
    s_heaProfiles["HEA 240"] = {230, 240, 7.5, 12.0, 21, 0};
    s_heaProfiles["HEA 260"] = {250, 260, 7.5, 12.5, 24, 0};
    s_heaProfiles["HEA 280"] = {270, 280, 8.0, 13.0, 24, 0};
    s_heaProfiles["HEA 300"] = {290, 300, 8.5, 14.0, 27, 0};
    s_heaProfiles["HEA 320"] = {310, 300, 9.0, 15.5, 27, 0};
    s_heaProfiles["HEA 340"] = {330, 300, 9.5, 16.5, 27, 0};
    s_heaProfiles["HEA 360"] = {350, 300, 10.0, 17.5, 27, 0};
    s_heaProfiles["HEA 400"] = {390, 300, 11.0, 19.0, 27, 0};
    s_heaProfiles["HEA 450"] = {440, 300, 11.5, 21.0, 27, 0};
    s_heaProfiles["HEA 500"] = {490, 300, 12.0, 23.0, 27, 0};
    
    // HEB Profiles (European wide flange - medium)
    s_hebProfiles["HEB 100"] = {100, 100, 6.0, 10.0, 12, 0};
    s_hebProfiles["HEB 120"] = {120, 120, 6.5, 11.0, 12, 0};
    s_hebProfiles["HEB 140"] = {140, 140, 7.0, 12.0, 12, 0};
    s_hebProfiles["HEB 160"] = {160, 160, 8.0, 13.0, 15, 0};
    s_hebProfiles["HEB 180"] = {180, 180, 8.5, 14.0, 15, 0};
    s_hebProfiles["HEB 200"] = {200, 200, 9.0, 15.0, 18, 0};
    s_hebProfiles["HEB 220"] = {220, 220, 9.5, 16.0, 18, 0};
    s_hebProfiles["HEB 240"] = {240, 240, 10.0, 17.0, 21, 0};
    s_hebProfiles["HEB 260"] = {260, 260, 10.0, 17.5, 24, 0};
    s_hebProfiles["HEB 280"] = {280, 280, 10.5, 18.0, 24, 0};
    s_hebProfiles["HEB 300"] = {300, 300, 11.0, 19.0, 27, 0};
    s_hebProfiles["HEB 320"] = {320, 300, 11.5, 20.5, 27, 0};
    s_hebProfiles["HEB 340"] = {340, 300, 12.0, 21.5, 27, 0};
    s_hebProfiles["HEB 360"] = {360, 300, 12.5, 22.5, 27, 0};
    s_hebProfiles["HEB 400"] = {400, 300, 13.5, 24.0, 27, 0};
    s_hebProfiles["HEB 450"] = {450, 300, 14.0, 26.0, 27, 0};
    s_hebProfiles["HEB 500"] = {500, 300, 14.5, 28.0, 27, 0};
    
    // HEM Profiles (European wide flange - heavy)
    s_hemProfiles["HEM 100"] = {120, 106, 12.0, 20.0, 12, 0};
    s_hemProfiles["HEM 120"] = {140, 126, 12.5, 21.0, 12, 0};
    s_hemProfiles["HEM 140"] = {160, 146, 13.0, 22.0, 12, 0};
    s_hemProfiles["HEM 160"] = {180, 166, 14.0, 23.0, 15, 0};
    s_hemProfiles["HEM 180"] = {200, 186, 14.5, 24.0, 15, 0};
    s_hemProfiles["HEM 200"] = {220, 206, 15.0, 25.0, 18, 0};
    s_hemProfiles["HEM 220"] = {240, 226, 15.5, 26.0, 18, 0};
    s_hemProfiles["HEM 240"] = {270, 248, 18.0, 32.0, 21, 0};
    s_hemProfiles["HEM 260"] = {290, 268, 18.0, 32.5, 24, 0};
    s_hemProfiles["HEM 280"] = {310, 288, 18.5, 33.0, 24, 0};
    s_hemProfiles["HEM 300"] = {340, 310, 21.0, 39.0, 27, 0};
    s_hemProfiles["HEM 320"] = {359, 309, 21.0, 40.0, 27, 0};
    s_hemProfiles["HEM 340"] = {377, 309, 21.0, 40.0, 27, 0};
    s_hemProfiles["HEM 360"] = {395, 308, 21.0, 40.0, 27, 0};
    
    // RHS Profiles (Rectangular Hollow Sections)
    s_rhsProfiles["RHS 50x30x3"]   = {50,  30,  0, 0, 0, 3.0};
    s_rhsProfiles["RHS 60x40x3"]   = {60,  40,  0, 0, 0, 3.0};
    s_rhsProfiles["RHS 80x40x3"]   = {80,  40,  0, 0, 0, 3.0};
    s_rhsProfiles["RHS 80x60x3"]   = {80,  60,  0, 0, 0, 3.0};
    s_rhsProfiles["RHS 100x50x4"]  = {100, 50,  0, 0, 0, 4.0};
    s_rhsProfiles["RHS 100x60x4"]  = {100, 60,  0, 0, 0, 4.0};
    s_rhsProfiles["RHS 120x80x5"]  = {120, 80,  0, 0, 0, 5.0};
    s_rhsProfiles["RHS 140x80x5"]  = {140, 80,  0, 0, 0, 5.0};
    s_rhsProfiles["RHS 150x100x5"] = {150, 100, 0, 0, 0, 5.0};
    s_rhsProfiles["RHS 160x80x5"]  = {160, 80,  0, 0, 0, 5.0};
    s_rhsProfiles["RHS 180x100x6"] = {180, 100, 0, 0, 0, 6.0};
    s_rhsProfiles["RHS 200x100x6"] = {200, 100, 0, 0, 0, 6.0};
    s_rhsProfiles["RHS 200x120x6"] = {200, 120, 0, 0, 0, 6.0};
    s_rhsProfiles["RHS 250x150x8"] = {250, 150, 0, 0, 0, 8.0};
    s_rhsProfiles["RHS 300x200x10"]= {300, 200, 0, 0, 0, 10.0};
    
    s_initialized = true;
}

TopoDS_Shape SteelProfile::createProfile(ProfileType type, const QString& size, 
                                         const gp_Pnt& start, const gp_Pnt& end)
{
    initializeProfiles();
    
    Dimensions dim = getDimensions(type, size);
    
    if (type == RHS) {
        return createRHSProfile(dim, start, end);
    } else {
        return createIProfile(dim, start, end);
    }
}

TopoDS_Shape SteelProfile::createIProfile(const Dimensions& dim, const gp_Pnt& start, const gp_Pnt& end)
{
    double h = dim.height;      // Total height (vertical)
    double b = dim.width;       // Flange width (horizontal)
    double tw = dim.webThickness;
    double tf = dim.flangeThickness;
    
    // Create I-profile cross-section in YZ plane
    // The profile will be extruded along X axis
    // Y = horizontal (flange width), Z = vertical (height)
    // Bottom of profile at Z=0, top at Z=h
    
    // Bottom flange
    gp_Pnt p1(0, -b/2, 0);          // Bottom left of bottom flange
    gp_Pnt p2(0, b/2, 0);           // Bottom right of bottom flange
    gp_Pnt p3(0, b/2, tf);          // Top right of bottom flange
    gp_Pnt p4(0, tw/2, tf);         // Web connection right
    
    // Web right side
    gp_Pnt p5(0, tw/2, h - tf);     // Top of web right
    gp_Pnt p6(0, b/2, h - tf);      // Bottom right of top flange
    
    // Top flange
    gp_Pnt p7(0, b/2, h);           // Top right of top flange
    gp_Pnt p8(0, -b/2, h);          // Top left of top flange
    gp_Pnt p9(0, -b/2, h - tf);     // Bottom left of top flange
    gp_Pnt p10(0, -tw/2, h - tf);   // Top of web left
    
    // Web left side
    gp_Pnt p11(0, -tw/2, tf);       // Bottom of web left
    gp_Pnt p12(0, -b/2, tf);        // Top left of bottom flange
    
    BRepBuilderAPI_MakeWire wiremaker;
    wiremaker.Add(BRepBuilderAPI_MakeEdge(p1, p2));
    wiremaker.Add(BRepBuilderAPI_MakeEdge(p2, p3));
    wiremaker.Add(BRepBuilderAPI_MakeEdge(p3, p4));
    wiremaker.Add(BRepBuilderAPI_MakeEdge(p4, p5));
    wiremaker.Add(BRepBuilderAPI_MakeEdge(p5, p6));
    wiremaker.Add(BRepBuilderAPI_MakeEdge(p6, p7));
    wiremaker.Add(BRepBuilderAPI_MakeEdge(p7, p8));
    wiremaker.Add(BRepBuilderAPI_MakeEdge(p8, p9));
    wiremaker.Add(BRepBuilderAPI_MakeEdge(p9, p10));
    wiremaker.Add(BRepBuilderAPI_MakeEdge(p10, p11));
    wiremaker.Add(BRepBuilderAPI_MakeEdge(p11, p12));
    wiremaker.Add(BRepBuilderAPI_MakeEdge(p12, p1));
    
    TopoDS_Wire wire = wiremaker.Wire();
    TopoDS_Face face = BRepBuilderAPI_MakeFace(wire);
    
    // Calculate beam direction and length
    gp_Vec direction(start, end);
    double length = direction.Magnitude();
    
    // Log to file
    QFile logFile("beam_debug.log");
    if (logFile.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&logFile);
        out << "  SteelProfile::createIProfile called\n";
        out << "  Start: (" << start.X() << ", " << start.Y() << ", " << start.Z() << ")\n";
        out << "  End: (" << end.X() << ", " << end.Y() << ", " << end.Z() << ")\n";
        out << "  Length: " << length << " mm\n";
        out << "  Profile: h=" << h << " b=" << b << " tw=" << tw << " tf=" << tf << "\n";
        logFile.close();
    }
    
    qDebug() << "SteelProfile: Start point" << start.X() << start.Y() << start.Z();
    qDebug() << "SteelProfile: End point" << end.X() << end.Y() << end.Z();
    qDebug() << "SteelProfile: Length" << length;
    qDebug() << "SteelProfile: Profile dimensions - h:" << h << "b:" << b << "tw:" << tw << "tf:" << tf;
    
    if (length < 1e-6) {
        qDebug() << "SteelProfile: ERROR - Length too small:" << length;
        return TopoDS_Shape();
    }
    
    // Extrude along X-axis (profile is in YZ plane)
    gp_Vec extrudeVec(length, 0, 0);
    TopoDS_Shape profile = BRepPrimAPI_MakePrism(face, extrudeVec);
    
    // Create transformation to align with beam direction
    // The profile's local X-axis should align with beam direction
    gp_Trsf transform;
    
    // First translate to start point
    transform.SetTranslation(gp_Vec(gp_Pnt(0,0,0), start));
    BRepBuilderAPI_Transform step1(profile, transform, Standard_False);
    profile = step1.Shape();
    
    // Now rotate to align with beam direction
    gp_Vec localX(1, 0, 0);  // Profile's extrusion direction
    direction.Normalize();
    
    double angle = localX.Angle(direction);
    if (angle > 1e-6) {
        gp_Vec rotAxis = localX.Crossed(direction);
        if (rotAxis.Magnitude() > 1e-6) {
            rotAxis.Normalize();
            gp_Trsf rotation;
            rotation.SetRotation(gp_Ax1(start, gp_Dir(rotAxis)), angle);
            BRepBuilderAPI_Transform step2(profile, rotation, Standard_False);
            profile = step2.Shape();
        }
    }
    
    return profile;
}

TopoDS_Shape SteelProfile::createRHSProfile(const Dimensions& dim, const gp_Pnt& start, const gp_Pnt& end)
{
    double h = dim.height;
    double b = dim.width;
    double t = dim.thickness;
    
    // Create outer rectangle in YZ plane
    // Bottom at Z=0, top at Z=h
    gp_Pnt p1(0, -b/2, 0);
    gp_Pnt p2(0, b/2, 0);
    gp_Pnt p3(0, b/2, h);
    gp_Pnt p4(0, -b/2, h);
    
    BRepBuilderAPI_MakeWire outerWire;
    outerWire.Add(BRepBuilderAPI_MakeEdge(p1, p2));
    outerWire.Add(BRepBuilderAPI_MakeEdge(p2, p3));
    outerWire.Add(BRepBuilderAPI_MakeEdge(p3, p4));
    outerWire.Add(BRepBuilderAPI_MakeEdge(p4, p1));
    
    // Create inner rectangle (hollow part) in YZ plane
    double bi = b - 2*t;
    double hi = h - 2*t;
    gp_Pnt pi1(0, -bi/2, t);
    gp_Pnt pi2(0, bi/2, t);
    gp_Pnt pi3(0, bi/2, h - t);
    gp_Pnt pi4(0, -bi/2, h - t);
    
    BRepBuilderAPI_MakeWire innerWire;
    innerWire.Add(BRepBuilderAPI_MakeEdge(pi1, pi2));
    innerWire.Add(BRepBuilderAPI_MakeEdge(pi2, pi3));
    innerWire.Add(BRepBuilderAPI_MakeEdge(pi3, pi4));
    innerWire.Add(BRepBuilderAPI_MakeEdge(pi4, pi1));
    
    // Create face with hole
    BRepBuilderAPI_MakeFace facemaker(outerWire.Wire());
    facemaker.Add(innerWire.Wire());
    TopoDS_Face face = facemaker.Face();
    
    // Calculate beam direction and length
    gp_Vec direction(start, end);
    double length = direction.Magnitude();
    
    if (length < 1e-6) {
        return TopoDS_Shape();
    }
    
    // Extrude along X-axis (profile is in YZ plane)
    gp_Vec extrudeVec(length, 0, 0);
    TopoDS_Shape profile = BRepPrimAPI_MakePrism(face, extrudeVec);
    
    // Create transformation to align with beam direction
    gp_Trsf transform;
    
    // First translate to start point
    transform.SetTranslation(gp_Vec(gp_Pnt(0,0,0), start));
    BRepBuilderAPI_Transform step1(profile, transform, Standard_False);
    profile = step1.Shape();
    
    // Now rotate to align with beam direction
    gp_Vec localX(1, 0, 0);  // Profile's extrusion direction
    direction.Normalize();
    
    double angle = localX.Angle(direction);
    if (angle > 1e-6) {
        gp_Vec rotAxis = localX.Crossed(direction);
        if (rotAxis.Magnitude() > 1e-6) {
            rotAxis.Normalize();
            gp_Trsf rotation;
            rotation.SetRotation(gp_Ax1(start, gp_Dir(rotAxis)), angle);
            BRepBuilderAPI_Transform step2(profile, rotation, Standard_False);
            profile = step2.Shape();
        }
    }
    
    return profile;
}

QStringList SteelProfile::getAvailableSizes(ProfileType type)
{
    initializeProfiles();
    
    QStringList sizes;
    switch (type) {
        case IPE:
            sizes = s_ipeProfiles.keys();
            break;
        case HEA:
            sizes = s_heaProfiles.keys();
            break;
        case HEB:
            sizes = s_hebProfiles.keys();
            break;
        case HEM:
            sizes = s_hemProfiles.keys();
            break;
        case RHS:
            sizes = s_rhsProfiles.keys();
            break;
    }
    sizes.sort();
    return sizes;
}

SteelProfile::Dimensions SteelProfile::getDimensions(ProfileType type, const QString& size)
{
    initializeProfiles();
    
    switch (type) {
        case IPE:
            return s_ipeProfiles.value(size, {200, 100, 5.6, 8.5, 12, 0});
        case HEA:
            return s_heaProfiles.value(size, {200, 200, 6.5, 10.0, 18, 0});
        case HEB:
            return s_hebProfiles.value(size, {200, 200, 9.0, 15.0, 18, 0});
        case HEM:
            return s_hemProfiles.value(size, {220, 206, 15.0, 25.0, 18, 0});
        case RHS:
            return s_rhsProfiles.value(size, {100, 50, 0, 0, 0, 4.0});
    }
    return {200, 100, 5.6, 8.5, 12, 0};
}

QString SteelProfile::getProfileName(ProfileType type, const QString& size)
{
    return size;
}
