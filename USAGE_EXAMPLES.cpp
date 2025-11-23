#include "TObjectCollection.h"
#include "TBeam.h"
#include "TColumn.h"
#include "TSlab.h"
#include <QDebug>

/**
 * @file usage_examples.cpp
 * @brief Example code demonstrating the OCCT-compatible OOP architecture
 * 
 * This file contains practical examples of how to use the new object-oriented
 * system with OpenCascade-compatible handles and collections.
 */

class ExampleUsage
{
public:
    static void demonstrateBasicUsage(const Handle(AIS_InteractiveContext)& context)
    {
        qDebug() << "=== Basic Object Creation Example ===";
        
        // Create the master collection
        TObjectCollection* collection = new TObjectCollection(context);
        
        // Example 1: Create a simple beam
        Handle(TBeam) beam1 = new TBeam();
        beam1->SetStartPoint(gp_Pnt(0, 0, 0));
        beam1->SetEndPoint(gp_Pnt(5000, 0, 0));
        beam1->SetRectangularSection(200, 400);
        beam1->SetName("Beam B1");
        collection->AddObject(beam1);
        qDebug() << "Created beam:" << beam1->GetName() 
                 << "Length:" << beam1->GetLength() << "mm";
        
        // Example 2: Create a beam with steel profile
        Handle(TBeam) beam2 = new TBeam(gp_Pnt(0, 3000, 0), gp_Pnt(5000, 3000, 0));
        beam2->SetProfileSection(SteelProfile::IPE, "IPE 300");
        beam2->SetName("Beam B2");
        collection->AddObject(beam2);
        qDebug() << "Created steel beam:" << beam2->GetName() 
                 << "Profile:" << beam2->GetProfileSize();
        
        // Example 3: Create columns
        Handle(TColumn) col1 = new TColumn(gp_Pnt(0, 0, 0), 400, 400, 3000);
        col1->SetName("Column C1");
        col1->SetMaterial("Concrete C30/37");
        collection->AddObject(col1);
        
        Handle(TColumn) col2 = new TColumn(gp_Pnt(5000, 0, 0), 400, 400, 3000);
        col2->SetName("Column C2");
        col2->SetMaterial("Concrete C30/37");
        collection->AddObject(col2);
        qDebug() << "Created 2 columns";
        
        // Example 4: Create a slab
        Handle(TSlab) slab = new TSlab(
            gp_Pnt(0, 0, 3000),
            gp_Pnt(10000, 8000, 3000),
            200
        );
        slab->SetName("Floor Slab Level 1");
        slab->SetLayer("Structure");
        collection->AddObject(slab);
        qDebug() << "Created slab, area:" << slab->GetArea() / 1e6 << "m²";
        
        qDebug() << "Total objects in collection:" << collection->GetObjectCount();
    }
    
    static void demonstrateCollectionOperations(TObjectCollection* collection)
    {
        qDebug() << "\n=== Collection Operations Example ===";
        
        // Get all beams
        NCollection_Sequence<Handle(TGraphicObject)> beams = 
            collection->GetObjectsByType(TGraphicObject::TYPE_BEAM);
        qDebug() << "Number of beams:" << beams.Length();
        
        // Iterate through beams
        for (int i = 1; i <= beams.Length(); i++) {
            Handle(TBeam) beam = Handle(TBeam)::DownCast(beams.Value(i));
            if (!beam.IsNull()) {
                qDebug() << " -" << beam->GetName() 
                         << "Length:" << beam->GetLength() << "mm"
                         << "Volume:" << beam->GetVolume() / 1e9 << "m³";
            }
        }
        
        // Get all columns
        NCollection_Sequence<Handle(TGraphicObject)> columns = 
            collection->GetObjectsByType(TGraphicObject::TYPE_COLUMN);
        qDebug() << "Number of columns:" << columns.Length();
        
        // Get objects by layer
        NCollection_Sequence<Handle(TGraphicObject)> structureObjects = 
            collection->GetObjectsByLayer("Structure");
        qDebug() << "Objects in 'Structure' layer:" << structureObjects.Length();
        
        // Search by name
        NCollection_Sequence<Handle(TGraphicObject)> foundObjects = 
            collection->FindObjects("Beam", true, false);
        qDebug() << "Objects with 'Beam' in name:" << foundObjects.Length();
    }
    
    static void demonstrateTransformations(TObjectCollection* collection)
    {
        qDebug() << "\n=== Transformation Example ===";
        
        // Get all beams
        NCollection_Sequence<Handle(TGraphicObject)> beams = 
            collection->GetObjectsByType(TGraphicObject::TYPE_BEAM);
        
        if (beams.Length() > 0) {
            // Translate first beam
            Handle(TBeam) beam = Handle(TBeam)::DownCast(beams.Value(1));
            if (!beam.IsNull()) {
                gp_Pnt oldStart = beam->GetStartPoint();
                qDebug() << "Original start point:" << oldStart.X() << oldStart.Y() << oldStart.Z();
                
                beam->Translate(gp_Vec(0, 0, 1000));  // Move up 1 meter
                
                gp_Pnt newStart = beam->GetStartPoint();
                qDebug() << "New start point:" << newStart.X() << newStart.Y() << newStart.Z();
            }
        }
        
        // Bulk transformation example
        NCollection_Sequence<int> columnIDs;
        NCollection_Sequence<Handle(TGraphicObject)> columns = 
            collection->GetObjectsByType(TGraphicObject::TYPE_COLUMN);
        
        for (int i = 1; i <= columns.Length(); i++) {
            columnIDs.Append(columns.Value(i)->GetID());
        }
        
        if (columnIDs.Length() > 0) {
            qDebug() << "Translating" << columnIDs.Length() << "columns";
            collection->TranslateObjects(columnIDs, gp_Vec(1000, 0, 0));
        }
    }
    
    static void demonstrateSelectionManagement(TObjectCollection* collection)
    {
        qDebug() << "\n=== Selection Management Example ===";
        
        // Select all beams
        NCollection_Sequence<Handle(TGraphicObject)> beams = 
            collection->GetObjectsByType(TGraphicObject::TYPE_BEAM);
        
        for (int i = 1; i <= beams.Length(); i++) {
            collection->SelectObject(beams.Value(i)->GetID());
        }
        qDebug() << "Selected all beams";
        
        // Get selected objects
        NCollection_Sequence<Handle(TGraphicObject)> selected = 
            collection->GetSelectedObjects();
        qDebug() << "Number of selected objects:" << selected.Length();
        
        // Process selected
        for (int i = 1; i <= selected.Length(); i++) {
            qDebug() << " - Selected:" << selected.Value(i)->GetTypeName() 
                     << selected.Value(i)->GetName();
        }
        
        // Deselect all
        collection->DeselectAll();
        qDebug() << "Deselected all objects";
    }
    
    static void demonstrateLayerManagement(TObjectCollection* collection)
    {
        qDebug() << "\n=== Layer Management Example ===";
        
        // Create custom layers
        collection->CreateLayer("Foundations");
        collection->CreateLayer("Roof");
        collection->CreateLayer("MEP");
        
        QStringList layers = collection->GetAllLayers();
        qDebug() << "Available layers:" << layers.join(", ");
        
        // Move columns to Foundations layer
        NCollection_Sequence<Handle(TGraphicObject)> columns = 
            collection->GetObjectsByType(TGraphicObject::TYPE_COLUMN);
        NCollection_Sequence<int> columnIDs;
        
        for (int i = 1; i <= columns.Length(); i++) {
            columnIDs.Append(columns.Value(i)->GetID());
        }
        
        collection->MoveObjectsToLayer(columnIDs, "Foundations");
        qDebug() << "Moved" << columnIDs.Length() << "columns to Foundations layer";
        
        // Show/hide by layer
        collection->HideByLayer("Roof");
        qDebug() << "Hidden Roof layer";
        
        collection->ShowByLayer("Structure");
        qDebug() << "Shown Structure layer";
    }
    
    static void demonstrateStatistics(TObjectCollection* collection)
    {
        qDebug() << "\n=== Statistics Example ===";
        
        // Object counts
        int totalObjects = collection->GetObjectCount();
        int beamCount = collection->GetObjectCountByType(TGraphicObject::TYPE_BEAM);
        int columnCount = collection->GetObjectCountByType(TGraphicObject::TYPE_COLUMN);
        int slabCount = collection->GetObjectCountByType(TGraphicObject::TYPE_SLAB);
        
        qDebug() << "Total objects:" << totalObjects;
        qDebug() << "Beams:" << beamCount;
        qDebug() << "Columns:" << columnCount;
        qDebug() << "Slabs:" << slabCount;
        
        // Calculate volumes
        double totalVolume = collection->GetTotalVolume();
        double totalSurface = collection->GetTotalSurfaceArea();
        
        qDebug() << "Total volume:" << totalVolume / 1e9 << "m³";
        qDebug() << "Total surface area:" << totalSurface / 1e6 << "m²";
        
        // Calculate concrete volume (columns + slabs)
        double concreteVolume = 0.0;
        NCollection_Sequence<Handle(TGraphicObject)> concreteElements = 
            collection->GetObjectsByMaterial("Concrete");
        
        for (int i = 1; i <= concreteElements.Length(); i++) {
            concreteVolume += concreteElements.Value(i)->GetVolume();
        }
        
        qDebug() << "Concrete volume:" << concreteVolume / 1e9 << "m³";
    }
    
    static void demonstrateHandleOperations()
    {
        qDebug() << "\n=== OCCT Handle Operations Example ===";
        
        // Create objects
        Handle(TBeam) beam = new TBeam();
        beam->SetName("Test Beam");
        
        // Check if handle is valid
        if (!beam.IsNull()) {
            qDebug() << "Handle is valid";
        }
        
        // Get as base class
        Handle(TGraphicObject) baseObj = beam;
        qDebug() << "Stored as base class:" << baseObj->GetTypeName();
        
        // Downcast back to specific type
        Handle(TBeam) specificBeam = Handle(TBeam)::DownCast(baseObj);
        if (!specificBeam.IsNull()) {
            qDebug() << "Successfully downcast to TBeam";
            qDebug() << "Beam name:" << specificBeam->GetName();
        }
        
        // Type checking
        if (baseObj->IsKind(STANDARD_TYPE(TBeam))) {
            qDebug() << "Object is a TBeam";
        }
        
        if (baseObj->IsKind(STANDARD_TYPE(TGraphicObject))) {
            qDebug() << "Object is a TGraphicObject (base class)";
        }
    }
    
    static void demonstratePropertyAccess()
    {
        qDebug() << "\n=== Property Access Example ===";
        
        Handle(TBeam) beam = new TBeam(gp_Pnt(0, 0, 0), gp_Pnt(6000, 0, 0));
        
        // Set properties
        beam->SetName("Main Beam MB-1");
        beam->SetDescription("Primary structural beam for floor support");
        beam->SetLayer("Structure");
        beam->SetMaterial("Steel S355");
        beam->SetColor(255, 0, 0);  // Red
        
        // Read properties
        qDebug() << "ID:" << beam->GetID();
        qDebug() << "Name:" << beam->GetName();
        qDebug() << "Description:" << beam->GetDescription();
        qDebug() << "Layer:" << beam->GetLayer();
        qDebug() << "Material:" << beam->GetMaterial();
        
        int r, g, b;
        beam->GetColor(r, g, b);
        qDebug() << "Color: RGB(" << r << "," << g << "," << b << ")";
        
        // Geometry properties
        qDebug() << "Length:" << beam->GetLength() << "mm";
        qDebug() << "Volume:" << beam->GetVolume() / 1e9 << "m³";
        qDebug() << "Surface area:" << beam->GetSurfaceArea() / 1e6 << "m²";
        
        gp_Pnt center = beam->GetCenterPoint();
        qDebug() << "Center point: (" << center.X() << "," << center.Y() << "," << center.Z() << ")";
        
        double xmin, ymin, zmin, xmax, ymax, zmax;
        beam->GetBoundingBox(xmin, ymin, zmin, xmax, ymax, zmax);
        qDebug() << "Bounding box: X[" << xmin << "," << xmax << "]"
                 << "Y[" << ymin << "," << ymax << "]"
                 << "Z[" << zmin << "," << zmax << "]";
        
        // Timestamps
        qDebug() << "Created:" << beam->GetCreationTime().toString("yyyy-MM-dd hh:mm:ss");
        qDebug() << "Modified:" << beam->GetModificationTime().toString("yyyy-MM-dd hh:mm:ss");
    }
    
    static void demonstrateValidation()
    {
        qDebug() << "\n=== Validation Example ===";
        
        // Create valid beam
        Handle(TBeam) validBeam = new TBeam(gp_Pnt(0, 0, 0), gp_Pnt(5000, 0, 0));
        validBeam->SetRectangularSection(200, 400);
        
        if (validBeam->IsValid()) {
            qDebug() << "Valid beam created";
        } else {
            qDebug() << "Invalid beam:" << validBeam->GetValidationError();
        }
        
        // Create invalid beam (same start and end point)
        Handle(TBeam) invalidBeam = new TBeam(gp_Pnt(0, 0, 0), gp_Pnt(0, 0, 0));
        
        if (!invalidBeam->IsValid()) {
            qDebug() << "Invalid beam detected:" << invalidBeam->GetValidationError();
        }
        
        // Create invalid column (negative dimensions)
        Handle(TColumn) column = new TColumn();
        column->SetDimensions(-100, 400, 3000);  // Invalid width
        
        if (!column->IsValid()) {
            qDebug() << "Invalid column:" << column->GetValidationError();
        }
    }
};

/**
 * How to integrate these examples into your application:
 * 
 * 1. In MainWindow or similar class:
 * 
 *    TObjectCollection* m_objectCollection;
 *    
 *    MainWindow::MainWindow() {
 *        m_objectCollection = new TObjectCollection(m_viewer->getContext());
 *        
 *        // Run examples (for testing)
 *        ExampleUsage::demonstrateBasicUsage(m_viewer->getContext());
 *        ExampleUsage::demonstrateCollectionOperations(m_objectCollection);
 *    }
 * 
 * 2. When creating objects from commands:
 * 
 *    void MainWindow::onBeamCommandCompleted(const TopoDS_Shape& shape) {
 *        Handle(TBeam) beam = new TBeam(startPoint, endPoint);
 *        beam->SetProfileSection(profileType, profileSize);
 *        m_objectCollection->AddObject(beam);
 *    }
 * 
 * 3. For property panels:
 * 
 *    void PropertyPanel::displayObject(int objectID) {
 *        Handle(TGraphicObject) obj = m_collection->GetObject(objectID);
 *        if (!obj.IsNull()) {
 *            nameEdit->setText(obj->GetName());
 *            layerCombo->setCurrentText(obj->GetLayer());
 *            materialEdit->setText(obj->GetMaterial());
 *            
 *            // Type-specific properties
 *            Handle(TBeam) beam = Handle(TBeam)::DownCast(obj);
 *            if (!beam.IsNull()) {
 *                lengthLabel->setText(QString::number(beam->GetLength()));
 *            }
 *        }
 *    }
 */
