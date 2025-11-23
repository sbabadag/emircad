# OCCT-Compatible OOP Architecture

## Overview

The CAD system now uses a professional OOP architecture fully compatible with OpenCascade Technology (OCCT) class system and collections. All graphic objects inherit from `TGraphicObject` base class and use OCCT's handle-based memory management system.

## Core Architecture

### Class Hierarchy

```
Standard_Transient (OCCT Base)
    └── TGraphicObject (Abstract Base Class)
        ├── TBeam
        ├── TColumn
        ├── TSlab
        ├── TWall (future)
        ├── TFoundation (future)
        └── ... (extensible)
```

### Master Collection

**TObjectCollection** - Central management class for all objects
- Uses OCCT's `NCollection_DataMap` and `NCollection_Sequence`
- Provides comprehensive object lifecycle management
- Integrates with AIS_InteractiveContext for display

## Key Classes

### 1. TGraphicObject (Base Class)

**File**: `TGraphicObject.h/cpp`

**Purpose**: Abstract base class for all CAD objects

**Key Features**:
- Uses OCCT handle system (`Standard_Transient`)
- RTTI support via `DEFINE_STANDARD_RTTIEXT`
- Common properties: ID, name, layer, material, color, state
- Virtual methods for polymorphism
- Transformation support (translate, rotate, scale, mirror)
- Serialization/deserialization
- Validation system

**Object Types**:
```cpp
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
```

**Object States**:
```cpp
enum ObjectState {
    STATE_NORMAL = 0,
    STATE_SELECTED = 1,
    STATE_HIGHLIGHTED = 2,
    STATE_HIDDEN = 3,
    STATE_LOCKED = 4
};
```

**Common Properties**:
- `int m_id` - Unique object identifier (auto-generated)
- `QString m_name` - Object name
- `QString m_description` - Object description
- `QString m_layer` - Layer assignment
- `QString m_material` - Material type
- `ObjectState m_state` - Current state
- `bool m_visible` - Visibility flag
- `bool m_locked` - Lock flag (prevents editing)
- `int m_colorR, m_colorG, m_colorB` - RGB color
- `QDateTime m_creationTime` - Creation timestamp
- `QDateTime m_modificationTime` - Last modification timestamp
- `TopoDS_Shape m_shape` - OpenCascade geometry
- `Handle(AIS_Shape) m_aisShape` - Interactive display object

**Pure Virtual Methods** (must be implemented by derived classes):
```cpp
virtual ObjectType GetType() const = 0;
virtual QString GetTypeName() const = 0;
virtual TopoDS_Shape BuildShape() = 0;
virtual Handle(AIS_Shape) GetAISShape() = 0;
```

**Common Methods**:
```cpp
// Property access
void SetID(int id);
int GetID() const;
void SetName(const QString& name);
QString GetName() const;
void SetLayer(const QString& layer);
QString GetLayer() const;
void SetMaterial(const QString& material);
QString GetMaterial() const;
void SetColor(int r, int g, int b);
void GetColor(int& r, int& g, int& b) const;

// State management
void SetState(ObjectState state);
ObjectState GetState() const;
void SetVisible(bool visible);
bool IsVisible() const;
void SetLocked(bool locked);
bool IsLocked() const;

// Geometry queries
gp_Pnt GetCenterPoint() const;
double GetVolume() const;
double GetSurfaceArea() const;
void GetBoundingBox(double& xmin, double& ymin, double& zmin,
                   double& xmax, double& ymax, double& zmax) const;

// Transformations
void Translate(const gp_Vec& vector);
void Rotate(const gp_Ax1& axis, double angle);
void Scale(const gp_Pnt& center, double factor);
void Mirror(const gp_Ax2& plane);

// Serialization
QString Serialize() const;
bool Deserialize(const QString& data);

// Validation
bool IsValid() const;
QString GetValidationError() const;
```

### 2. TObjectCollection (Master Collection)

**File**: `TObjectCollection.h/cpp`

**Purpose**: Centralized management of all graphic objects

**Storage**: 
- `NCollection_DataMap<int, Handle(TGraphicObject)>` - Main object storage (ID → Object)
- `NCollection_Sequence<int>` - Selected objects list
- Compatible with OCCT collections and algorithms

**Key Features**:

#### Object Management
```cpp
bool AddObject(const Handle(TGraphicObject)& object);
bool RemoveObject(int objectID);
void Clear();
Handle(TGraphicObject) GetObject(int objectID) const;
```

#### Retrieval & Filtering
```cpp
NCollection_Sequence<Handle(TGraphicObject)> GetAllObjects() const;
NCollection_Sequence<Handle(TGraphicObject)> GetObjectsByType(ObjectType type) const;
NCollection_Sequence<Handle(TGraphicObject)> GetObjectsByLayer(const QString& layer) const;
NCollection_Sequence<Handle(TGraphicObject)> GetObjectsByMaterial(const QString& material) const;

NCollection_Sequence<Handle(TGraphicObject)> FindObjects(
    const QString& searchText, 
    bool searchName = true, 
    bool searchDescription = true) const;

NCollection_Sequence<Handle(TGraphicObject)> FilterObjects(
    ObjectType type = TYPE_UNKNOWN,
    const QString& layer = QString(),
    const QString& material = QString(),
    bool visibleOnly = false) const;
```

#### Selection Management
```cpp
void SelectObject(int objectID);
void DeselectObject(int objectID);
void SelectAll();
void DeselectAll();
NCollection_Sequence<Handle(TGraphicObject)> GetSelectedObjects() const;
```

#### Visibility Management
```cpp
void ShowObject(int objectID);
void HideObject(int objectID);
void ShowAll();
void HideAll();
void ShowByType(ObjectType type);
void HideByType(ObjectType type);
void ShowByLayer(const QString& layer);
void HideByLayer(const QString& layer);
```

#### Layer Management
```cpp
QStringList GetAllLayers() const;
void CreateLayer(const QString& layer);
void DeleteLayer(const QString& layer);
void MoveObjectsToLayer(const NCollection_Sequence<int>& objectIDs, const QString& layer);
```

#### Bulk Operations
```cpp
void TranslateObjects(const NCollection_Sequence<int>& objectIDs, const gp_Vec& vector);
void RotateObjects(const NCollection_Sequence<int>& objectIDs, const gp_Ax1& axis, double angle);
void ScaleObjects(const NCollection_Sequence<int>& objectIDs, const gp_Pnt& center, double factor);
void MirrorObjects(const NCollection_Sequence<int>& objectIDs, const gp_Ax2& plane);
NCollection_Sequence<Handle(TGraphicObject)> CopyObjects(const NCollection_Sequence<int>& objectIDs);
```

#### Statistics
```cpp
int GetObjectCount() const;
int GetObjectCountByType(ObjectType type) const;
double GetTotalVolume() const;
double GetTotalSurfaceArea() const;
```

#### Undo/Redo Support (Framework Ready)
```cpp
void BeginTransaction(const QString& description);
void CommitTransaction();
void RollbackTransaction();
bool CanUndo() const;
bool CanRedo() const;
void Undo();
void Redo();
```

#### Serialization
```cpp
bool SaveToFile(const QString& filename);
bool LoadFromFile(const QString& filename);
QString ExportToXML() const;
bool ImportFromXML(const QString& xml);
```

**Qt Signals**:
```cpp
signals:
    void objectAdded(int objectID);
    void objectRemoved(int objectID);
    void objectModified(int objectID);
    void selectionChanged();
    void collectionCleared();
```

### 3. TBeam (Beam Objects)

**File**: `TBeam.h/cpp`

**Inherits**: TGraphicObject

**Specific Properties**:
- `gp_Pnt m_startPoint` - Beam start point
- `gp_Pnt m_endPoint` - Beam end point
- `double m_sectionWidth` - Width for rectangular section
- `double m_sectionHeight` - Height for rectangular section
- `bool m_useProfile` - Flag for steel profile vs rectangular
- `SteelProfile::ProfileType m_profileType` - Profile type (IPE, HEA, etc.)
- `QString m_profileSize` - Profile size designation

**Methods**:
```cpp
void SetStartPoint(const gp_Pnt& point);
gp_Pnt GetStartPoint() const;
void SetEndPoint(const gp_Pnt& point);
gp_Pnt GetEndPoint() const;
double GetLength() const;
gp_Vec GetDirection() const;

void SetRectangularSection(double width, double height);
void SetProfileSection(SteelProfile::ProfileType type, const QString& size);
void GetSectionDimensions(double& width, double& height) const;
```

**Default Values**:
- Layer: "Structure"
- Material: "Steel"
- Color: RGB(150, 150, 200) - Light blue

### 4. TColumn (Column Objects)

**File**: `TColumn.h/cpp`

**Inherits**: TGraphicObject

**Specific Properties**:
- `gp_Pnt m_basePoint` - Column base center point
- `double m_width` - Column width
- `double m_depth` - Column depth
- `double m_height` - Column height

**Methods**:
```cpp
void SetBasePoint(const gp_Pnt& point);
gp_Pnt GetBasePoint() const;
void SetDimensions(double width, double depth, double height);
void GetDimensions(double& width, double& depth, double& height) const;
```

**Default Values**:
- Layer: "Structure"
- Material: "Concrete"
- Color: RGB(180, 180, 180) - Gray
- Dimensions: 400×400×3000 mm

### 5. TSlab (Slab Objects)

**File**: `TSlab.h/cpp`

**Inherits**: TGraphicObject

**Specific Properties**:
- `gp_Pnt m_corner1` - First corner point
- `gp_Pnt m_corner2` - Opposite corner point
- `double m_thickness` - Slab thickness

**Methods**:
```cpp
void SetCorners(const gp_Pnt& corner1, const gp_Pnt& corner2);
void GetCorners(gp_Pnt& corner1, gp_Pnt& corner2) const;
void SetThickness(double thickness);
double GetThickness() const;
double GetArea() const;
```

**Default Values**:
- Layer: "Structure"
- Material: "Concrete"
- Color: RGB(200, 200, 180) - Light gray
- Thickness: 200 mm

## OCCT Integration

### Handle System

All objects use OCCT's handle-based memory management:

```cpp
// Forward declaration
class TBeam;
DEFINE_STANDARD_HANDLE(TBeam, TGraphicObject)

// Class definition with RTTI
class TBeam : public TGraphicObject
{
    DEFINE_STANDARD_RTTIEXT(TBeam, TGraphicObject)
    // ... class members
};

// Implementation file
IMPLEMENT_STANDARD_RTTIEXT(TBeam, TGraphicObject)
```

**Benefits**:
- Automatic reference counting
- Type-safe downcasting
- Memory leak prevention
- Compatible with all OCCT containers

### Using Handles

```cpp
// Create object
Handle(TBeam) beam = new TBeam(startPoint, endPoint);

// Pass by handle
collection->AddObject(beam);

// Check if valid
if (!beam.IsNull()) {
    // Safe to use
}

// Type checking
if (beam->IsKind(STANDARD_TYPE(TGraphicObject))) {
    // It's a TGraphicObject
}

// Dynamic casting
Handle(TGraphicObject) obj = collection->GetObject(id);
Handle(TBeam) beam = Handle(TBeam)::DownCast(obj);
if (!beam.IsNull()) {
    // Successfully cast to TBeam
    double length = beam->GetLength();
}
```

### OCCT Collections

The system uses OCCT's optimized collection classes:

**NCollection_Sequence** - Dynamic array
```cpp
NCollection_Sequence<Handle(TGraphicObject)> objects;
objects.Append(beam);
objects.Append(column);

// Iterate
for (int i = 1; i <= objects.Length(); i++) {
    Handle(TGraphicObject) obj = objects.Value(i);
    // Process object
}
```

**NCollection_DataMap** - Hash map
```cpp
NCollection_DataMap<int, Handle(TGraphicObject)> objectMap;
objectMap.Bind(id, object);

if (objectMap.IsBound(id)) {
    Handle(TGraphicObject) obj = objectMap.Find(id);
}
```

## Usage Examples

### Creating Objects

```cpp
// Create collection
Handle(AIS_InteractiveContext) context = viewer->getContext();
TObjectCollection* collection = new TObjectCollection(context);

// Create a beam with steel profile
Handle(TBeam) beam = new TBeam();
beam->SetStartPoint(gp_Pnt(0, 0, 0));
beam->SetEndPoint(gp_Pnt(5000, 0, 0));
beam->SetProfileSection(SteelProfile::IPE, "IPE 300");
beam->SetName("Main Beam 1");
beam->SetLayer("Structure");
collection->AddObject(beam);

// Create a column
Handle(TColumn) column = new TColumn(gp_Pnt(0, 0, 0), 400, 400, 3000);
column->SetName("Column C1");
column->SetMaterial("Concrete C30/37");
collection->AddObject(column);

// Create a slab
Handle(TSlab) slab = new TSlab(
    gp_Pnt(0, 0, 3000),
    gp_Pnt(10000, 8000, 3000),
    200
);
slab->SetName("Floor Slab L1");
collection->AddObject(slab);
```

### Retrieving Objects

```cpp
// Get all beams
NCollection_Sequence<Handle(TGraphicObject)> beams = 
    collection->GetObjectsByType(TGraphicObject::TYPE_BEAM);

// Iterate and process
for (int i = 1; i <= beams.Length(); i++) {
    Handle(TBeam) beam = Handle(TBeam)::DownCast(beams.Value(i));
    if (!beam.IsNull()) {
        qDebug() << "Beam" << beam->GetName() 
                 << "length:" << beam->GetLength() << "mm";
    }
}

// Get objects by layer
NCollection_Sequence<Handle(TGraphicObject)> structureObjects = 
    collection->GetObjectsByLayer("Structure");

// Search by name
NCollection_Sequence<Handle(TGraphicObject)> mainBeams = 
    collection->FindObjects("Main", true, false);
```

### Selection Management

```cpp
// Select object
collection->SelectObject(beam->GetID());

// Get selected objects
NCollection_Sequence<Handle(TGraphicObject)> selected = 
    collection->GetSelectedObjects();

// Process selected
for (int i = 1; i <= selected.Length(); i++) {
    Handle(TGraphicObject) obj = selected.Value(i);
    qDebug() << obj->GetTypeName() << obj->GetName();
}

// Deselect all
collection->DeselectAll();
```

### Transformations

```cpp
// Translate selected objects
NCollection_Sequence<Handle(TGraphicObject)> selected = 
    collection->GetSelectedObjects();
NCollection_Sequence<int> ids;
for (int i = 1; i <= selected.Length(); i++) {
    ids.Append(selected.Value(i)->GetID());
}
collection->TranslateObjects(ids, gp_Vec(1000, 0, 0)); // Move 1m in X

// Rotate objects
gp_Ax1 axis(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)); // Z-axis
collection->RotateObjects(ids, axis, M_PI/4); // Rotate 45 degrees

// Mirror objects
gp_Ax2 plane(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0)); // YZ plane
collection->MirrorObjects(ids, plane);
```

### Layer Management

```cpp
// Create custom layers
collection->CreateLayer("Foundations");
collection->CreateLayer("Roof");

// Move objects to layer
NCollection_Sequence<int> columnIDs;
// ... add column IDs ...
collection->MoveObjectsToLayer(columnIDs, "Foundations");

// Show/hide by layer
collection->HideByLayer("Roof");
collection->ShowByLayer("Structure");

// Get all layers
QStringList layers = collection->GetAllLayers();
```

### Statistics

```cpp
// Count objects
int totalObjects = collection->GetObjectCount();
int beamCount = collection->GetObjectCountByType(TGraphicObject::TYPE_BEAM);
int columnCount = collection->GetObjectCountByType(TGraphicObject::TYPE_COLUMN);

// Calculate volumes
double totalVolume = collection->GetTotalVolume();
double totalSurface = collection->GetTotalSurfaceArea();

qDebug() << "Total concrete volume:" << totalVolume / 1e9 << "m³";
```

## Benefits of This Architecture

1. **OCCT Compatibility**: Full integration with OpenCascade collections and algorithms
2. **Memory Safety**: Handle-based system prevents memory leaks
3. **Type Safety**: RTTI support for safe downcasting
4. **Extensibility**: Easy to add new object types
5. **Centralized Management**: Single collection manages all objects
6. **Polymorphism**: Virtual methods enable generic operations
7. **Layer System**: Professional CAD-style layer management
8. **Selection System**: Built-in selection tracking
9. **Bulk Operations**: Efficient multi-object transformations
10. **Future-Ready**: Framework for undo/redo, serialization

## Future Extensions

### Planned Object Types
- `TWall` - Wall objects
- `TFoundation` - Foundation elements
- `TBrace` - Bracing members
- `TPlate` - Steel plates
- `TConnection` - Connection details

### Planned Features
- Full undo/redo stack implementation
- XML-based file save/load
- Database integration
- Material property database
- Section property calculations
- Structural analysis integration
- BIM integration
- Collision detection
- Automatic dimensioning

## Migration Guide

To update existing code to use the new architecture:

```cpp
// Old approach
GeometryBuilder::createBeam(context, start, end, width, height);

// New approach
Handle(TBeam) beam = new TBeam(start, end);
beam->SetRectangularSection(width, height);
collection->AddObject(beam);
```

The new system provides much more control and features while maintaining clean, object-oriented design principles fully compatible with OCCT standards.
