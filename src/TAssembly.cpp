#include "TAssembly.h"
#include <BRep_Builder.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <TopoDS_Iterator.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>

TAssembly::TAssembly()
    : TGraphicObject()
    , m_assemblyName("Assembly")
    , m_assemblyType("Generic")
    , m_needsUpdate(false)
{
    // Initialize empty compound
    BRep_Builder builder;
    builder.MakeCompound(m_compound);
}

TAssembly::~TAssembly() {
    clearParts();
}

void TAssembly::addPart(std::shared_ptr<TGraphicObject> part) {
    if (!part) return;
    
    m_parts.push_back(part);
    m_needsUpdate = true;
    rebuildCompound();
}

void TAssembly::removePart(std::shared_ptr<TGraphicObject> part) {
    auto it = std::find(m_parts.begin(), m_parts.end(), part);
    if (it != m_parts.end()) {
        m_parts.erase(it);
        m_needsUpdate = true;
        rebuildCompound();
    }
}

void TAssembly::removePartAt(int index) {
    if (index >= 0 && index < static_cast<int>(m_parts.size())) {
        m_parts.erase(m_parts.begin() + index);
        m_needsUpdate = true;
        rebuildCompound();
    }
}

void TAssembly::clearParts() {
    m_parts.clear();
    BRep_Builder builder;
    builder.MakeCompound(m_compound);
    m_needsUpdate = false;
}

int TAssembly::getPartCount() const {
    return static_cast<int>(m_parts.size());
}

std::shared_ptr<TGraphicObject> TAssembly::getPart(int index) const {
    if (index >= 0 && index < static_cast<int>(m_parts.size())) {
        return m_parts[index];
    }
    return nullptr;
}

const std::vector<std::shared_ptr<TGraphicObject>>& TAssembly::getParts() const {
    return m_parts;
}

void TAssembly::setAssemblyName(const QString& name) {
    m_assemblyName = name;
}

QString TAssembly::getAssemblyName() const {
    return m_assemblyName;
}

void TAssembly::setAssemblyType(const QString& type) {
    m_assemblyType = type;
}

QString TAssembly::getAssemblyType() const {
    return m_assemblyType;
}

TopoDS_Shape TAssembly::getShape() const {
    return m_compound;
}

void TAssembly::setColor(const Quantity_Color& color) {
    TGraphicObject::setColor(color);
    
    // Optionally apply to all parts
    for (auto& part : m_parts) {
        if (part) {
            part->setColor(color);
        }
    }
}

Quantity_Color TAssembly::getColor() const {
    return TGraphicObject::getColor();
}

gp_Pnt TAssembly::getPosition() const {
    if (m_parts.empty()) {
        return gp_Pnt(0, 0, 0);
    }
    
    // Return center of assembly
    return getAssemblyCenter();
}

void TAssembly::move(const gp_Vec& vec) {
    // Move all parts
    for (auto& part : m_parts) {
        if (part) {
            part->move(vec);
        }
    }
    m_needsUpdate = true;
    rebuildCompound();
}

void TAssembly::rotate(const gp_Ax1& axis, double angle) {
    // Rotate all parts around the same axis
    for (auto& part : m_parts) {
        if (part) {
            part->rotate(axis, angle);
        }
    }
    m_needsUpdate = true;
    rebuildCompound();
}

std::shared_ptr<TGraphicObject> TAssembly::clone() const {
    auto newAssembly = std::make_shared<TAssembly>();
    newAssembly->setAssemblyName(m_assemblyName);
    newAssembly->setAssemblyType(m_assemblyType);
    newAssembly->setColor(getColor());
    
    // Clone all parts
    for (const auto& part : m_parts) {
        if (part) {
            newAssembly->addPart(part->clone());
        }
    }
    
    return newAssembly;
}

QString TAssembly::getTypeName() const {
    return QString("Assembly (%1)").arg(m_assemblyType);
}

void TAssembly::updateCompound() {
    if (m_needsUpdate) {
        rebuildCompound();
    }
}

bool TAssembly::isEmpty() const {
    return m_parts.empty();
}

gp_Pnt TAssembly::getAssemblyCenter() const {
    if (m_parts.empty()) {
        return gp_Pnt(0, 0, 0);
    }
    
    GProp_GProps props;
    BRepGProp::VolumeProperties(m_compound, props);
    return props.CentreOfMass();
}

void TAssembly::getAssemblyBounds(double& xmin, double& ymin, double& zmin,
                                  double& xmax, double& ymax, double& zmax) const {
    if (m_parts.empty()) {
        xmin = ymin = zmin = xmax = ymax = zmax = 0.0;
        return;
    }
    
    Bnd_Box box;
    BRepBndLib::Add(m_compound, box);
    
    if (!box.IsVoid()) {
        box.Get(xmin, ymin, zmin, xmax, ymax, zmax);
    } else {
        xmin = ymin = zmin = xmax = ymax = zmax = 0.0;
    }
}

void TAssembly::applyTransformationToAll(const gp_Trsf& transformation) {
    for (auto& part : m_parts) {
        if (part) {
            TopoDS_Shape shape = part->getShape();
            BRepBuilderAPI_Transform transform(shape, transformation, Standard_True);
            // Note: You may need to update each part's internal state
            // This is a simplified version
        }
    }
    m_needsUpdate = true;
    rebuildCompound();
}

void TAssembly::rebuildCompound() {
    // Create new compound
    BRep_Builder builder;
    builder.MakeCompound(m_compound);
    
    // Add all part shapes to compound
    for (const auto& part : m_parts) {
        if (part) {
            TopoDS_Shape shape = part->getShape();
            if (!shape.IsNull()) {
                builder.Add(m_compound, shape);
            }
        }
    }
    
    m_needsUpdate = false;
}
