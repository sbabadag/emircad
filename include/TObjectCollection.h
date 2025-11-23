#ifndef TOBJECTCOLLECTION_H
#define TOBJECTCOLLECTION_H

#include "TGraphicObject.h"
#include <NCollection_Sequence.hxx>
#include <NCollection_DataMap.hxx>
#include <AIS_InteractiveContext.hxx>
#include <QString>
#include <QObject>

/**
 * @brief Master collection class for managing all graphic objects
 * 
 * Uses OpenCascade's NCollection for efficient storage and retrieval.
 * Provides centralized management of all objects in the CAD system.
 */
class TObjectCollection : public QObject
{
    Q_OBJECT
    
public:
    explicit TObjectCollection(const Handle(AIS_InteractiveContext)& context, QObject* parent = nullptr);
    virtual ~TObjectCollection();
    
    // Object management
    Standard_EXPORT bool AddObject(const Handle(TGraphicObject)& object);
    Standard_EXPORT bool RemoveObject(int objectID);
    Standard_EXPORT bool RemoveObject(const Handle(TGraphicObject)& object);
    Standard_EXPORT void Clear();
    
    // Object retrieval
    Standard_EXPORT Handle(TGraphicObject) FindObject(int objectID) const;
    Standard_EXPORT NCollection_Sequence<Handle(TGraphicObject)> GetAllObjects() const;
    Standard_EXPORT NCollection_Sequence<Handle(TGraphicObject)> GetObjectsByType(TGraphicObject::ObjectType type) const;
    Standard_EXPORT NCollection_Sequence<Handle(TGraphicObject)> GetObjectsByLayer(const QString& layer) const;
    Standard_EXPORT NCollection_Sequence<Handle(TGraphicObject)> GetObjectsByMaterial(const QString& material) const;
    
    // Selection management
    Standard_EXPORT void SelectObject(int objectID);
    Standard_EXPORT void DeselectObject(int objectID);
    Standard_EXPORT void SelectAll();
    Standard_EXPORT void DeselectAll();
    Standard_EXPORT NCollection_Sequence<Handle(TGraphicObject)> GetSelectedObjects() const;
    
    // Visibility management
    Standard_EXPORT void ShowObject(int objectID);
    Standard_EXPORT void HideObject(int objectID);
    Standard_EXPORT void ShowAll();
    Standard_EXPORT void HideAll();
    Standard_EXPORT void ShowByType(TGraphicObject::ObjectType type);
    Standard_EXPORT void HideByType(TGraphicObject::ObjectType type);
    Standard_EXPORT void ShowByLayer(const QString& layer);
    Standard_EXPORT void HideByLayer(const QString& layer);
    
    // Layer management
    Standard_EXPORT QStringList GetAllLayers() const;
    Standard_EXPORT void CreateLayer(const QString& layer);
    Standard_EXPORT void DeleteLayer(const QString& layer);
    Standard_EXPORT void MoveObjectsToLayer(const NCollection_Sequence<int>& objectIDs, const QString& layer);
    
    // Statistics
    Standard_EXPORT int GetObjectCount() const;
    Standard_EXPORT int GetObjectCountByType(TGraphicObject::ObjectType type) const;
    Standard_EXPORT double GetTotalVolume() const;
    Standard_EXPORT double GetTotalSurfaceArea() const;
    
    // Bulk operations
    Standard_EXPORT void TranslateObjects(const NCollection_Sequence<int>& objectIDs, const gp_Vec& vector);
    Standard_EXPORT void RotateObjects(const NCollection_Sequence<int>& objectIDs, const gp_Ax1& axis, double angle);
    Standard_EXPORT void ScaleObjects(const NCollection_Sequence<int>& objectIDs, const gp_Pnt& center, double factor);
    Standard_EXPORT void MirrorObjects(const NCollection_Sequence<int>& objectIDs, const gp_Ax2& plane);
    Standard_EXPORT NCollection_Sequence<Handle(TGraphicObject)> CopyObjects(const NCollection_Sequence<int>& objectIDs);
    
    // Undo/Redo support
    Standard_EXPORT void BeginTransaction(const QString& description);
    Standard_EXPORT void CommitTransaction();
    Standard_EXPORT void RollbackTransaction();
    Standard_EXPORT bool CanUndo() const;
    Standard_EXPORT bool CanRedo() const;
    Standard_EXPORT void Undo();
    Standard_EXPORT void Redo();
    
    // Serialization
    Standard_EXPORT bool SaveToFile(const QString& filename);
    Standard_EXPORT bool LoadFromFile(const QString& filename);
    Standard_EXPORT QString ExportToXML() const;
    Standard_EXPORT bool ImportFromXML(const QString& xml);
    
    // Search and filter
    Standard_EXPORT NCollection_Sequence<Handle(TGraphicObject)> FindObjects(
        const QString& searchText, 
        bool searchName = true, 
        bool searchDescription = true) const;
    
    Standard_EXPORT NCollection_Sequence<Handle(TGraphicObject)> FilterObjects(
        TGraphicObject::ObjectType type = TGraphicObject::TYPE_UNKNOWN,
        const QString& layer = QString(),
        const QString& material = QString(),
        bool visibleOnly = false) const;

signals:
    void objectAdded(int objectID);
    void objectRemoved(int objectID);
    void objectModified(int objectID);
    void selectionChanged();
    void collectionCleared();

private:
    Handle(AIS_InteractiveContext) m_context;
    NCollection_DataMap<int, Handle(TGraphicObject)> m_objects;
    NCollection_Sequence<int> m_selectedObjects;
    QStringList m_layers;
    
    // Helper methods
    void displayObject(const Handle(TGraphicObject)& object);
    void eraseObject(const Handle(TGraphicObject)& object);
    void updateDisplay(const Handle(TGraphicObject)& object);
};

#endif // TOBJECTCOLLECTION_H
