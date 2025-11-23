#ifndef STEELPROFILE_H
#define STEELPROFILE_H

#include <QString>
#include <QMap>
#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>

class SteelProfile
{
public:
    enum ProfileType {
        IPE,        // European I-beam
        HEA,        // European wide flange (light)
        HEB,        // European wide flange (medium)
        HEM,        // European wide flange (heavy)
        RHS         // Rectangular Hollow Section
    };
    
    struct Dimensions {
        double height;          // h - overall height
        double width;           // b - flange width
        double webThickness;    // tw - web thickness
        double flangeThickness; // tf - flange thickness
        double radius;          // r - root radius
        
        // For RHS
        double thickness;       // t - wall thickness for RHS
    };
    
    static TopoDS_Shape createProfile(ProfileType type, const QString& size, 
                                      const gp_Pnt& start, const gp_Pnt& end);
    
    static QStringList getAvailableSizes(ProfileType type);
    static Dimensions getDimensions(ProfileType type, const QString& size);
    static QString getProfileName(ProfileType type, const QString& size);

private:
    static TopoDS_Shape createIProfile(const Dimensions& dim, const gp_Pnt& start, const gp_Pnt& end);
    static TopoDS_Shape createRHSProfile(const Dimensions& dim, const gp_Pnt& start, const gp_Pnt& end);
    
    static void initializeProfiles();
    static QMap<QString, Dimensions> s_ipeProfiles;
    static QMap<QString, Dimensions> s_heaProfiles;
    static QMap<QString, Dimensions> s_hebProfiles;
    static QMap<QString, Dimensions> s_hemProfiles;
    static QMap<QString, Dimensions> s_rhsProfiles;
    static bool s_initialized;
};

#endif // STEELPROFILE_H
