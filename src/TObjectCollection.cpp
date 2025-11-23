#include "TObjectCollection.h"
#include <Quantity_Color.hxx>

TObjectCollection::TObjectCollection(const Handle(AIS_InteractiveContext)& context, QObject* parent)
    : QObject(parent)
    , m_context(context)
{
    m_layers.append("Default");
    m_layers.append("Structure");
    m_layers.append("Architecture");
    m_layers.append("Foundation");
}

TObjectCollection::~TObjectCollection()
{
    Clear();
}

bool TObjectCollection::AddObject(const Handle(TGraphicObject)& object)
{
    if (object.IsNull()) {
        return false;
    }
    
    int id = object->GetID();
    if (m_objects.IsBound(id)) {
        return false; // Object already exists
    }
    
    m_objects.Bind(id, object);
    displayObject(object);
    
    emit objectAdded(id);
    return true;
}

bool TObjectCollection::RemoveObject(int objectID)
{
    if (!m_objects.IsBound(objectID)) {
        return false;
    }
    
    Handle(TGraphicObject) object = m_objects.Find(objectID);
    eraseObject(object);
    m_objects.UnBind(objectID);
    
    // Remove from selection if selected
    for (int i = 1; i <= m_selectedObjects.Length(); i++) {
        if (m_selectedObjects.Value(i) == objectID) {
            m_selectedObjects.Remove(i);
            break;
        }
    }
    
    emit objectRemoved(objectID);
    return true;
}

bool TObjectCollection::RemoveObject(const Handle(TGraphicObject)& object)
{
    if (object.IsNull()) {
        return false;
    }
    return RemoveObject(object->GetID());
}

void TObjectCollection::Clear()
{
    NCollection_DataMap<int, Handle(TGraphicObject)>::Iterator it(m_objects);
    for (; it.More(); it.Next()) {
        eraseObject(it.Value());
    }
    
    m_objects.Clear();
    m_selectedObjects.Clear();
    
    emit collectionCleared();
}

Handle(TGraphicObject) TObjectCollection::FindObject(int objectID) const
{
    if (m_objects.IsBound(objectID)) {
        return m_objects.Find(objectID);
    }
    return Handle(TGraphicObject)();
}

NCollection_Sequence<Handle(TGraphicObject)> TObjectCollection::GetAllObjects() const
{
    NCollection_Sequence<Handle(TGraphicObject)> result;
    NCollection_DataMap<int, Handle(TGraphicObject)>::Iterator it(m_objects);
    for (; it.More(); it.Next()) {
        result.Append(it.Value());
    }
    return result;
}

NCollection_Sequence<Handle(TGraphicObject)> TObjectCollection::GetObjectsByType(TGraphicObject::ObjectType type) const
{
    NCollection_Sequence<Handle(TGraphicObject)> result;
    NCollection_DataMap<int, Handle(TGraphicObject)>::Iterator it(m_objects);
    for (; it.More(); it.Next()) {
        if (it.Value()->GetType() == type) {
            result.Append(it.Value());
        }
    }
    return result;
}

NCollection_Sequence<Handle(TGraphicObject)> TObjectCollection::GetObjectsByLayer(const QString& layer) const
{
    NCollection_Sequence<Handle(TGraphicObject)> result;
    NCollection_DataMap<int, Handle(TGraphicObject)>::Iterator it(m_objects);
    for (; it.More(); it.Next()) {
        if (it.Value()->GetLayer() == layer) {
            result.Append(it.Value());
        }
    }
    return result;
}

NCollection_Sequence<Handle(TGraphicObject)> TObjectCollection::GetObjectsByMaterial(const QString& material) const
{
    NCollection_Sequence<Handle(TGraphicObject)> result;
    NCollection_DataMap<int, Handle(TGraphicObject)>::Iterator it(m_objects);
    for (; it.More(); it.Next()) {
        if (it.Value()->GetMaterial() == material) {
            result.Append(it.Value());
        }
    }
    return result;
}

void TObjectCollection::SelectObject(int objectID)
{
    if (!m_objects.IsBound(objectID)) {
        return;
    }
    
    // Check if already selected
    for (int i = 1; i <= m_selectedObjects.Length(); i++) {
        if (m_selectedObjects.Value(i) == objectID) {
            return;
        }
    }
    
    m_selectedObjects.Append(objectID);
    Handle(TGraphicObject) object = m_objects.Find(objectID);
    object->SetState(TGraphicObject::STATE_SELECTED);
    
    Handle(AIS_Shape) aisShape = object->GetAISShape();
    if (!aisShape.IsNull() && !m_context.IsNull()) {
        m_context->SetSelected(aisShape, Standard_False);
    }
    
    emit selectionChanged();
}

void TObjectCollection::DeselectObject(int objectID)
{
    for (int i = 1; i <= m_selectedObjects.Length(); i++) {
        if (m_selectedObjects.Value(i) == objectID) {
            m_selectedObjects.Remove(i);
            
            if (m_objects.IsBound(objectID)) {
                Handle(TGraphicObject) object = m_objects.Find(objectID);
                object->SetState(TGraphicObject::STATE_NORMAL);
                
                Handle(AIS_Shape) aisShape = object->GetAISShape();
                if (!aisShape.IsNull() && !m_context.IsNull()) {
                    m_context->AddOrRemoveSelected(aisShape, Standard_False);
                }
            }
            
            emit selectionChanged();
            break;
        }
    }
}

void TObjectCollection::SelectAll()
{
    m_selectedObjects.Clear();
    NCollection_DataMap<int, Handle(TGraphicObject)>::Iterator it(m_objects);
    for (; it.More(); it.Next()) {
        m_selectedObjects.Append(it.Key());
        it.Value()->SetState(TGraphicObject::STATE_SELECTED);
    }
    emit selectionChanged();
}

void TObjectCollection::DeselectAll()
{
    for (int i = 1; i <= m_selectedObjects.Length(); i++) {
        int id = m_selectedObjects.Value(i);
        if (m_objects.IsBound(id)) {
            m_objects.Find(id)->SetState(TGraphicObject::STATE_NORMAL);
        }
    }
    m_selectedObjects.Clear();
    
    if (!m_context.IsNull()) {
        m_context->ClearSelected(Standard_False);
    }
    
    emit selectionChanged();
}

NCollection_Sequence<Handle(TGraphicObject)> TObjectCollection::GetSelectedObjects() const
{
    NCollection_Sequence<Handle(TGraphicObject)> result;
    for (int i = 1; i <= m_selectedObjects.Length(); i++) {
        int id = m_selectedObjects.Value(i);
        if (m_objects.IsBound(id)) {
            result.Append(m_objects.Find(id));
        }
    }
    return result;
}

void TObjectCollection::ShowObject(int objectID)
{
    if (!m_objects.IsBound(objectID)) {
        return;
    }
    
    Handle(TGraphicObject) object = m_objects.Find(objectID);
    object->SetVisible(true);
    
    Handle(AIS_Shape) aisShape = object->GetAISShape();
    if (!aisShape.IsNull() && !m_context.IsNull()) {
        m_context->Display(aisShape, Standard_False);
    }
}

void TObjectCollection::HideObject(int objectID)
{
    if (!m_objects.IsBound(objectID)) {
        return;
    }
    
    Handle(TGraphicObject) object = m_objects.Find(objectID);
    object->SetVisible(false);
    
    Handle(AIS_Shape) aisShape = object->GetAISShape();
    if (!aisShape.IsNull() && !m_context.IsNull()) {
        m_context->Erase(aisShape, Standard_False);
    }
}

void TObjectCollection::ShowAll()
{
    NCollection_DataMap<int, Handle(TGraphicObject)>::Iterator it(m_objects);
    for (; it.More(); it.Next()) {
        ShowObject(it.Key());
    }
    if (!m_context.IsNull()) {
        m_context->UpdateCurrentViewer();
    }
}

void TObjectCollection::HideAll()
{
    NCollection_DataMap<int, Handle(TGraphicObject)>::Iterator it(m_objects);
    for (; it.More(); it.Next()) {
        HideObject(it.Key());
    }
    if (!m_context.IsNull()) {
        m_context->UpdateCurrentViewer();
    }
}

int TObjectCollection::GetObjectCount() const
{
    return m_objects.Extent();
}

int TObjectCollection::GetObjectCountByType(TGraphicObject::ObjectType type) const
{
    return GetObjectsByType(type).Length();
}

QStringList TObjectCollection::GetAllLayers() const
{
    return m_layers;
}

void TObjectCollection::CreateLayer(const QString& layer)
{
    if (!m_layers.contains(layer)) {
        m_layers.append(layer);
    }
}

void TObjectCollection::displayObject(const Handle(TGraphicObject)& object)
{
    if (object.IsNull() || m_context.IsNull()) {
        return;
    }
    
    Handle(AIS_Shape) aisShape = object->GetAISShape();
    if (!aisShape.IsNull()) {
        int r, g, b;
        object->GetColor(r, g, b);
        Quantity_Color color(r / 255.0, g / 255.0, b / 255.0, Quantity_TOC_RGB);
        m_context->SetColor(aisShape, color, Standard_False);
        m_context->Display(aisShape, Standard_False);
    }
}

void TObjectCollection::eraseObject(const Handle(TGraphicObject)& object)
{
    if (object.IsNull() || m_context.IsNull()) {
        return;
    }
    
    Handle(AIS_Shape) aisShape = object->GetAISShape();
    if (!aisShape.IsNull()) {
        m_context->Remove(aisShape, Standard_False);
    }
}

void TObjectCollection::updateDisplay(const Handle(TGraphicObject)& object)
{
    if (object.IsNull() || m_context.IsNull()) {
        return;
    }
    
    Handle(AIS_Shape) aisShape = object->GetAISShape();
    if (!aisShape.IsNull()) {
        m_context->Redisplay(aisShape, Standard_False);
    }
}

bool TObjectCollection::SaveToFile(const QString& filename)
{
    Q_UNUSED(filename);
    // To be implemented
    return false;
}

bool TObjectCollection::LoadFromFile(const QString& filename)
{
    Q_UNUSED(filename);
    // To be implemented
    return false;
}

NCollection_Sequence<Handle(TGraphicObject)> TObjectCollection::FindObjects(
    const QString& searchText, bool searchName, bool searchDescription) const
{
    NCollection_Sequence<Handle(TGraphicObject)> result;
    NCollection_DataMap<int, Handle(TGraphicObject)>::Iterator it(m_objects);
    
    for (; it.More(); it.Next()) {
        bool match = false;
        
        if (searchName && it.Value()->GetName().contains(searchText, Qt::CaseInsensitive)) {
            match = true;
        }
        if (searchDescription && it.Value()->GetDescription().contains(searchText, Qt::CaseInsensitive)) {
            match = true;
        }
        
        if (match) {
            result.Append(it.Value());
        }
    }
    
    return result;
}

void TObjectCollection::ShowByType(TGraphicObject::ObjectType type)
{
    NCollection_Sequence<Handle(TGraphicObject)> objects = GetObjectsByType(type);
    for (int i = 1; i <= objects.Length(); i++) {
        ShowObject(objects.Value(i)->GetID());
    }
    if (!m_context.IsNull()) {
        m_context->UpdateCurrentViewer();
    }
}

void TObjectCollection::HideByType(TGraphicObject::ObjectType type)
{
    NCollection_Sequence<Handle(TGraphicObject)> objects = GetObjectsByType(type);
    for (int i = 1; i <= objects.Length(); i++) {
        HideObject(objects.Value(i)->GetID());
    }
    if (!m_context.IsNull()) {
        m_context->UpdateCurrentViewer();
    }
}

void TObjectCollection::ShowByLayer(const QString& layer)
{
    NCollection_Sequence<Handle(TGraphicObject)> objects = GetObjectsByLayer(layer);
    for (int i = 1; i <= objects.Length(); i++) {
        ShowObject(objects.Value(i)->GetID());
    }
    if (!m_context.IsNull()) {
        m_context->UpdateCurrentViewer();
    }
}

void TObjectCollection::HideByLayer(const QString& layer)
{
    NCollection_Sequence<Handle(TGraphicObject)> objects = GetObjectsByLayer(layer);
    for (int i = 1; i <= objects.Length(); i++) {
        HideObject(objects.Value(i)->GetID());
    }
    if (!m_context.IsNull()) {
        m_context->UpdateCurrentViewer();
    }
}

double TObjectCollection::GetTotalVolume() const
{
    double total = 0.0;
    NCollection_DataMap<int, Handle(TGraphicObject)>::Iterator it(m_objects);
    for (; it.More(); it.Next()) {
        total += it.Value()->GetVolume();
    }
    return total;
}

double TObjectCollection::GetTotalSurfaceArea() const
{
    double total = 0.0;
    NCollection_DataMap<int, Handle(TGraphicObject)>::Iterator it(m_objects);
    for (; it.More(); it.Next()) {
        total += it.Value()->GetSurfaceArea();
    }
    return total;
}

void TObjectCollection::TranslateObjects(const NCollection_Sequence<int>& objectIDs, const gp_Vec& vector)
{
    for (int i = 1; i <= objectIDs.Length(); i++) {
        int id = objectIDs.Value(i);
        if (m_objects.IsBound(id)) {
            Handle(TGraphicObject) obj = m_objects.Find(id);
            obj->Translate(vector);
            updateDisplay(obj);
            emit objectModified(id);
        }
    }
    if (!m_context.IsNull()) {
        m_context->UpdateCurrentViewer();
    }
}

void TObjectCollection::RotateObjects(const NCollection_Sequence<int>& objectIDs, const gp_Ax1& axis, double angle)
{
    for (int i = 1; i <= objectIDs.Length(); i++) {
        int id = objectIDs.Value(i);
        if (m_objects.IsBound(id)) {
            Handle(TGraphicObject) obj = m_objects.Find(id);
            obj->Rotate(axis, angle);
            updateDisplay(obj);
            emit objectModified(id);
        }
    }
    if (!m_context.IsNull()) {
        m_context->UpdateCurrentViewer();
    }
}

bool TObjectCollection::CanUndo() const
{
    // To be implemented with undo/redo stack
    return false;
}

bool TObjectCollection::CanRedo() const
{
    // To be implemented with undo/redo stack
    return false;
}

void TObjectCollection::Undo()
{
    // To be implemented
}

void TObjectCollection::Redo()
{
    // To be implemented
}

void TObjectCollection::BeginTransaction(const QString& description)
{
    Q_UNUSED(description);
    // To be implemented
}

void TObjectCollection::CommitTransaction()
{
    // To be implemented
}

void TObjectCollection::RollbackTransaction()
{
    // To be implemented
}

QString TObjectCollection::ExportToXML() const
{
    // To be implemented
    return QString();
}

bool TObjectCollection::ImportFromXML(const QString& xml)
{
    Q_UNUSED(xml);
    // To be implemented
    return false;
}

NCollection_Sequence<Handle(TGraphicObject)> TObjectCollection::FilterObjects(
    TGraphicObject::ObjectType type, const QString& layer, const QString& material, bool visibleOnly) const
{
    NCollection_Sequence<Handle(TGraphicObject)> result;
    NCollection_DataMap<int, Handle(TGraphicObject)>::Iterator it(m_objects);
    
    for (; it.More(); it.Next()) {
        Handle(TGraphicObject) obj = it.Value();
        bool match = true;
        
        if (type != TGraphicObject::TYPE_UNKNOWN && obj->GetType() != type) {
            match = false;
        }
        if (!layer.isEmpty() && obj->GetLayer() != layer) {
            match = false;
        }
        if (!material.isEmpty() && obj->GetMaterial() != material) {
            match = false;
        }
        if (visibleOnly && !obj->IsVisible()) {
            match = false;
        }
        
        if (match) {
            result.Append(obj);
        }
    }
    
    return result;
}

void TObjectCollection::MoveObjectsToLayer(const NCollection_Sequence<int>& objectIDs, const QString& layer)
{
    if (!m_layers.contains(layer)) {
        CreateLayer(layer);
    }
    
    for (int i = 1; i <= objectIDs.Length(); i++) {
        int id = objectIDs.Value(i);
        if (m_objects.IsBound(id)) {
            m_objects.Find(id)->SetLayer(layer);
            emit objectModified(id);
        }
    }
}

void TObjectCollection::DeleteLayer(const QString& layer)
{
    // Move all objects from this layer to Default
    NCollection_Sequence<Handle(TGraphicObject)> objects = GetObjectsByLayer(layer);
    for (int i = 1; i <= objects.Length(); i++) {
        objects.Value(i)->SetLayer("Default");
    }
    
    m_layers.removeAll(layer);
}

NCollection_Sequence<Handle(TGraphicObject)> TObjectCollection::CopyObjects(const NCollection_Sequence<int>& objectIDs)
{
    Q_UNUSED(objectIDs);
    // To be implemented - needs deep copy of objects
    return NCollection_Sequence<Handle(TGraphicObject)>();
}

void TObjectCollection::ScaleObjects(const NCollection_Sequence<int>& objectIDs, const gp_Pnt& center, double factor)
{
    for (int i = 1; i <= objectIDs.Length(); i++) {
        int id = objectIDs.Value(i);
        if (m_objects.IsBound(id)) {
            Handle(TGraphicObject) obj = m_objects.Find(id);
            obj->Scale(center, factor);
            updateDisplay(obj);
            emit objectModified(id);
        }
    }
    if (!m_context.IsNull()) {
        m_context->UpdateCurrentViewer();
    }
}

void TObjectCollection::MirrorObjects(const NCollection_Sequence<int>& objectIDs, const gp_Ax2& plane)
{
    for (int i = 1; i <= objectIDs.Length(); i++) {
        int id = objectIDs.Value(i);
        if (m_objects.IsBound(id)) {
            Handle(TGraphicObject) obj = m_objects.Find(id);
            obj->Mirror(plane);
            updateDisplay(obj);
            emit objectModified(id);
        }
    }
    if (!m_context.IsNull()) {
        m_context->UpdateCurrentViewer();
    }
}
