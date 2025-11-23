#ifndef TASSEMBLY_H
#define TASSEMBLY_H

#include "TGraphicObject.h"
#include <vector>
#include <memory>
#include <QString>
#include <TopoDS_Compound.hxx>

/**
 * @brief TAssembly - A collection of graphic objects (beams, columns, slabs, etc.)
 * 
 * This class represents a higher-level structural element composed of multiple parts.
 * Examples: floor assembly, frame structure, building module, etc.
 * 
 * Design Philosophy:
 * - Base objects: TBeam, TColumn, TSlab (single structural elements)
 * - Assembly objects: TAssembly (collection of structural elements)
 */
class TAssembly : public TGraphicObject {
public:
    TAssembly();
    virtual ~TAssembly();

    // Part management
    void addPart(std::shared_ptr<TGraphicObject> part);
    void removePart(std::shared_ptr<TGraphicObject> part);
    void removePartAt(int index);
    void clearParts();
    
    int getPartCount() const;
    std::shared_ptr<TGraphicObject> getPart(int index) const;
    const std::vector<std::shared_ptr<TGraphicObject>>& getParts() const;

    // Assembly properties
    void setAssemblyName(const QString& name);
    QString getAssemblyName() const;
    
    void setAssemblyType(const QString& type);
    QString getAssemblyType() const;

    // TGraphicObject interface implementation
    virtual TopoDS_Shape getShape() const override;
    virtual void setColor(const Quantity_Color& color) override;
    virtual Quantity_Color getColor() const override;
    virtual gp_Pnt getPosition() const override;
    virtual void move(const gp_Vec& vec) override;
    virtual void rotate(const gp_Ax1& axis, double angle) override;
    virtual std::shared_ptr<TGraphicObject> clone() const override;
    virtual QString getTypeName() const override;

    // Assembly-specific operations
    void updateCompound();  // Rebuild the compound shape from parts
    bool isEmpty() const;
    
    // Bounding box for entire assembly
    gp_Pnt getAssemblyCenter() const;
    void getAssemblyBounds(double& xmin, double& ymin, double& zmin,
                          double& xmax, double& ymax, double& zmax) const;

    // Apply transformation to all parts
    void applyTransformationToAll(const gp_Trsf& transformation);

protected:
    std::vector<std::shared_ptr<TGraphicObject>> m_parts;
    TopoDS_Compound m_compound;
    QString m_assemblyName;
    QString m_assemblyType;  // e.g., "Floor", "Frame", "Truss", "Module"
    bool m_needsUpdate;

    void rebuildCompound();
};

#endif // TASSEMBLY_H
