#ifndef TGRAPHICOBJECT_H
#define TGRAPHICOBJECT_H

#include <Standard.hxx>
#include <Standard_Type.hxx>
#include <Standard_Transient.hxx>
#include <TopoDS_Shape.hxx>
#include <AIS_Shape.hxx>
#include <QString>
#include <QDateTime>
#include <gp_Pnt.hxx>

// Forward declaration for OCCT handle system
class TGraphicObject;
DEFINE_STANDARD_HANDLE(TGraphicObject, Standard_Transient)

/**
 * @brief Base class for all graphic objects in the CAD system
 * 
 * This class uses OpenCascade's handle system (Standard_Transient)
 * for proper memory management and compatibility with OCCT collections.
 */
class TGraphicObject : public Standard_Transient
{
public:
    // Object types
    enum ObjectType {
        TYPE_UNKNOWN = 0,
        TYPE_BEAM = 1,
        TYPE_COLUMN = 2,
        TYPE_SLAB = 3,
        TYPE_WALL = 4,
        TYPE_FOUNDATION = 5,
        TYPE_BRACE = 6,
        TYPE_PLATE = 7
    };
    
    // Object state
    enum ObjectState {
        STATE_NORMAL = 0,
        STATE_SELECTED = 1,
        STATE_HIGHLIGHTED = 2,
        STATE_HIDDEN = 3,
        STATE_LOCKED = 4
    };

public:
    // Constructor
    Standard_EXPORT TGraphicObject();
    
    // Destructor
    Standard_EXPORT virtual ~TGraphicObject();
    
    // Type information (required for OCCT RTTI)
    DEFINE_STANDARD_RTTIEXT(TGraphicObject, Standard_Transient)
    
    // Pure virtual methods - must be implemented by derived classes
    Standard_EXPORT virtual ObjectType GetType() const = 0;
    Standard_EXPORT virtual QString GetTypeName() const = 0;
    Standard_EXPORT virtual TopoDS_Shape BuildShape() = 0;
    Standard_EXPORT virtual Handle(AIS_Shape) GetAISShape() = 0;
    
    // Common properties
    Standard_EXPORT void SetID(int id) { m_id = id; }
    Standard_EXPORT int GetID() const { return m_id; }
    
    Standard_EXPORT void SetName(const QString& name) { m_name = name; }
    Standard_EXPORT QString GetName() const { return m_name; }
    
    Standard_EXPORT void SetDescription(const QString& desc) { m_description = desc; }
    Standard_EXPORT QString GetDescription() const { return m_description; }
    
    Standard_EXPORT void SetLayer(const QString& layer) { m_layer = layer; }
    Standard_EXPORT QString GetLayer() const { return m_layer; }
    
    Standard_EXPORT void SetMaterial(const QString& material) { m_material = material; }
    Standard_EXPORT QString GetMaterial() const { return m_material; }
    
    Standard_EXPORT void SetState(ObjectState state) { m_state = state; }
    Standard_EXPORT ObjectState GetState() const { return m_state; }
    
    Standard_EXPORT void SetVisible(bool visible) { m_visible = visible; }
    Standard_EXPORT bool IsVisible() const { return m_visible; }
    
    Standard_EXPORT void SetLocked(bool locked) { m_locked = locked; }
    Standard_EXPORT bool IsLocked() const { return m_locked; }
    
    Standard_EXPORT void SetColor(int r, int g, int b);
    Standard_EXPORT void GetColor(int& r, int& g, int& b) const;
    
    // Timestamps
    Standard_EXPORT QDateTime GetCreationTime() const { return m_creationTime; }
    Standard_EXPORT QDateTime GetModificationTime() const { return m_modificationTime; }
    Standard_EXPORT void UpdateModificationTime() { m_modificationTime = QDateTime::currentDateTime(); }
    
    // Geometry queries
    Standard_EXPORT virtual gp_Pnt GetCenterPoint() const;
    Standard_EXPORT virtual double GetVolume() const;
    Standard_EXPORT virtual double GetSurfaceArea() const;
    Standard_EXPORT virtual void GetBoundingBox(double& xmin, double& ymin, double& zmin,
                                                 double& xmax, double& ymax, double& zmax) const;
    
    // Snap points management
    struct SnapPoint {
        gp_Pnt point;
        int type;  // 0x01=Endpoint, 0x02=Midpoint, 0x04=Center
        QString description;
        
        SnapPoint() : type(0) {}
        SnapPoint(const gp_Pnt& p, int t, const QString& desc) 
            : point(p), type(t), description(desc) {}
    };
    
    Standard_EXPORT virtual QList<SnapPoint> GetSnapPoints() const { return m_snapPoints; }
    Standard_EXPORT virtual void ClearSnapPoints() { m_snapPoints.clear(); }
    Standard_EXPORT virtual void AddSnapPoint(const gp_Pnt& point, int type, const QString& description);
    Standard_EXPORT virtual SnapPoint FindNearestSnapPoint(const gp_Pnt& cursor, double tolerance) const;
    
    // Transformations
    Standard_EXPORT virtual void Translate(const gp_Vec& vector);
    Standard_EXPORT virtual void Rotate(const gp_Ax1& axis, double angle);
    Standard_EXPORT virtual void Scale(const gp_Pnt& center, double factor);
    Standard_EXPORT virtual void Mirror(const gp_Ax2& plane);
    
    // Serialization
    Standard_EXPORT virtual QString Serialize() const;
    Standard_EXPORT virtual bool Deserialize(const QString& data);
    
    // Validation
    Standard_EXPORT virtual bool IsValid() const;
    Standard_EXPORT virtual QString GetValidationError() const { return m_validationError; }

protected:
    // Common member variables
    int m_id;
    QString m_name;
    QString m_description;
    QString m_layer;
    QString m_material;
    ObjectState m_state;
    bool m_visible;
    bool m_locked;
    int m_colorR, m_colorG, m_colorB;
    
    QDateTime m_creationTime;
    QDateTime m_modificationTime;
    
    TopoDS_Shape m_shape;
    Handle(AIS_Shape) m_aisShape;
    
    QList<SnapPoint> m_snapPoints;  // Cached snap points
    
    mutable QString m_validationError;
    
    // Static ID counter
    static int s_nextID;
};

#endif // TGRAPHICOBJECT_H
