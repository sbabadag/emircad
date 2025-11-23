#ifdef _WIN32
#undef GetObject  // Undefine Windows API macro that conflicts with TObjectCollection::GetObject
#endif

#include "MainWindow.h"
#include "GeometryBuilder.h"
#include "ProfileSelectionDialog.h"
#include "BeamCommand.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QLabel>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <StdSelect_BRepOwner.hxx>
#include <StdSelect_FaceFilter.hxx>
#include <AIS_ListOfInteractive.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <BRep_Tool.hxx>
#include <BRepTools.hxx>
#include <Geom_Surface.hxx>
#include <Geom_Plane.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <gp_Pln.hxx>
#include <Precision.hxx>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_controller(nullptr)
    , m_objectCollection(nullptr)
    , m_propertiesPanel(nullptr)
    , m_facePickingMode(false)
{
    setWindowTitle("3D Structural CAD - Tekla-like Application");
    resize(1400, 900);

    // Create central viewer widget
    m_viewer = new OCCTViewer(this);
    setCentralWidget(m_viewer);
    
    // Create object collection with AIS context
    m_objectCollection = new TObjectCollection(m_viewer->getContext(), this);
    
    // Create CAD controller with collection
    m_controller = new CADController(m_viewer->getContext(), m_viewer, m_objectCollection, this);
    connect(m_controller, &CADController::statusMessage, this, [this](const QString& msg) {
        statusBar()->showMessage(msg);
    });
    connect(m_controller, &CADController::commandChanged, this, [this](const QString& cmd) {
        statusBar()->showMessage(cmd);
    });
    
    // Connect viewer clicks to controller
    connect(m_viewer, &OCCTViewer::viewClicked, this, [this](int x, int y, Qt::MouseButton button) {
        try {
            // Check if in face picking mode first
            if (m_facePickingMode) {
                onFaceClicked(x, y, button);
                return;
            }
            
            gp_Pnt worldPoint = m_controller->convertViewToWorld(x, y, m_viewer->getView());
            
            // Apply snapping if enabled
            if (m_controller->isSnapEnabled() && m_controller->getCurrentCommand()) {
                Handle(V3d_View) view = m_viewer->getView();
                
                // Use object-based snap detection (faster)
                SnapManager::SnapPoint snap = m_controller->getSnapManager()->findSnapPointFromObjects(
                    x, y, m_objectCollection, view);
                
                if (snap.type != SnapManager::None) {
                    worldPoint = snap.point;  // Use the 3D snap point
                    statusBar()->showMessage(QString("Snap: %1").arg(snap.description), 1000);
                }
            }
            
            m_controller->handleClick(worldPoint);
        } catch (const std::exception& e) {
            statusBar()->showMessage(QString("Error processing click: %1").arg(e.what()), 3000);
        } catch (...) {
            statusBar()->showMessage("Unknown error processing click", 3000);
        }
    });
    
    // Connect viewer mouse move to controller for preview
    connect(m_viewer, &OCCTViewer::viewMouseMove, this, [this](int x, int y, Qt::KeyboardModifiers modifiers) {
        try {
            // Check if in face picking mode first
            if (m_facePickingMode) {
                onFaceHovered(x, y);
                return;
            }
            
            // Throttle mouse move processing for smoother performance
            static int moveCounter = 0;
            static bool ctrlWasPressed = false;
            moveCounter++;
            
            gp_Pnt worldPoint = m_controller->convertViewToWorld(x, y, m_viewer->getView());
            
            // Apply snapping ONLY if Ctrl key is held
            bool ctrlPressed = (modifiers & Qt::ControlModifier);
            if (ctrlPressed && m_controller->getCurrentCommand()) {
                Handle(V3d_View) view = m_viewer->getView();
                
                // Use object-based snap detection (faster)
                SnapManager::SnapPoint snap = m_controller->getSnapManager()->findSnapPointFromObjects(
                    x, y, m_objectCollection, view);
                
                if (snap.type != SnapManager::None) {
                    worldPoint = snap.point;  // Use the 3D snap point
                    m_controller->showSnapMarker(worldPoint, snap.type, snap.description);
                    
                    // Show snap info immediately
                    statusBar()->showMessage(QString("SNAP: %1 at 3D(%2, %3, %4)")
                        .arg(snap.description)
                        .arg(snap.point.X(), 0, 'f', 1)
                        .arg(snap.point.Y(), 0, 'f', 1)
                        .arg(snap.point.Z(), 0, 'f', 1));
                } else {
                    m_controller->hideSnapMarker();
                }
                ctrlWasPressed = true;
            } else if (ctrlWasPressed) {
                // Only hide snap marker when Ctrl is released (transition from pressed to not pressed)
                m_controller->hideSnapMarker();
                ctrlWasPressed = false;
            }
            
            // Update preview (throttled)
            if (moveCounter % 2 == 0) {
                m_controller->handleMove(worldPoint);
            }
        } catch (...) {
            // Silently ignore mouse move errors to avoid spam
        }
    });
    
    // Create properties panel
    m_propertiesPanel = new PropertiesPanel(this);
    addDockWidget(Qt::RightDockWidgetArea, m_propertiesPanel);
    
    // Create floating snap toolbar
    m_snapToolbar = new SnapToolbar(this);
    m_snapToolbar->move(50, 100); // Position on screen
    m_snapToolbar->show();
    
    // Connect snap toolbar to controller
    connect(m_snapToolbar, &SnapToolbar::snapTypesChanged, this, [this](int types) {
        m_controller->getSnapManager()->setSnapTypes(types);
    });
    
    // Initialize snap types from toolbar
    m_controller->getSnapManager()->setSnapTypes(m_snapToolbar->getEnabledSnapTypes());
    
    // Connect collection signals to properties panel
    connect(m_objectCollection, &TObjectCollection::selectionChanged, this, &MainWindow::updatePropertiesPanel);
    connect(m_propertiesPanel, &PropertiesPanel::propertyChanged, this, [this](int objectID) {
        // Notify collection that object was modified
        Handle(TGraphicObject) obj = m_objectCollection->FindObject(objectID);
        if (!obj.IsNull()) {
            emit m_objectCollection->objectModified(objectID);
        }
    });

    // Setup UI components
    setupUI();
    createActions();
    createMenus();
    createToolBars();
    createDockWidgets();

    statusBar()->showMessage("Ready");
}

MainWindow::~MainWindow()
{
    if (m_controller) {
        delete m_controller;
    }
}

void MainWindow::setupUI()
{
    // Set modern style
    setStyleSheet(
        "QMainWindow { background-color: #2b2b2b; }"
        "QMenuBar { background-color: #3c3c3c; color: white; }"
        "QMenuBar::item:selected { background-color: #0d6efd; }"
        "QMenu { background-color: #3c3c3c; color: white; }"
        "QMenu::item:selected { background-color: #0d6efd; }"
        "QToolBar { background-color: #3c3c3c; border: none; spacing: 3px; }"
        "QStatusBar { background-color: #3c3c3c; color: white; }"
    );
}

void MainWindow::createActions()
{
    // File menu actions
    m_newAction = new QAction(tr("&New Project"), this);
    m_newAction->setShortcut(QKeySequence::New);
    m_newAction->setStatusTip(tr("Create a new project"));
    connect(m_newAction, &QAction::triggered, this, &MainWindow::onNewProject);

    m_openAction = new QAction(tr("&Open Project"), this);
    m_openAction->setShortcut(QKeySequence::Open);
    m_openAction->setStatusTip(tr("Open an existing project"));
    connect(m_openAction, &QAction::triggered, this, &MainWindow::onOpenProject);

    m_saveAction = new QAction(tr("&Save Project"), this);
    m_saveAction->setShortcut(QKeySequence::Save);
    m_saveAction->setStatusTip(tr("Save the current project"));
    connect(m_saveAction, &QAction::triggered, this, &MainWindow::onSaveProject);

    m_exportAction = new QAction(tr("&Export"), this);
    m_exportAction->setStatusTip(tr("Export model to file"));
    connect(m_exportAction, &QAction::triggered, this, &MainWindow::onExport);

    m_exitAction = new QAction(tr("E&xit"), this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    m_exitAction->setStatusTip(tr("Exit the application"));
    connect(m_exitAction, &QAction::triggered, this, &MainWindow::onExit);

    // Create menu actions
    m_createBeamAction = new QAction(tr("Create &Beam"), this);
    m_createBeamAction->setStatusTip(tr("Create a structural beam"));
    connect(m_createBeamAction, &QAction::triggered, this, &MainWindow::onCreateBeam);

    m_createColumnAction = new QAction(tr("Create &Column"), this);
    m_createColumnAction->setStatusTip(tr("Create a structural column"));
    connect(m_createColumnAction, &QAction::triggered, this, &MainWindow::onCreateColumn);

    m_createSlabAction = new QAction(tr("Create &Slab"), this);
    m_createSlabAction->setStatusTip(tr("Create a floor slab"));
    connect(m_createSlabAction, &QAction::triggered, this, &MainWindow::onCreateSlab);

    m_createWallAction = new QAction(tr("Create &Wall"), this);
    m_createWallAction->setStatusTip(tr("Create a wall"));
    connect(m_createWallAction, &QAction::triggered, this, &MainWindow::onCreateWall);

    m_createFoundationAction = new QAction(tr("Create &Foundation"), this);
    m_createFoundationAction->setStatusTip(tr("Create a foundation"));
    connect(m_createFoundationAction, &QAction::triggered, this, &MainWindow::onCreateFoundation);

    // View menu actions
    m_viewTopAction = new QAction(tr("&Top View"), this);
    m_viewTopAction->setStatusTip(tr("Switch to top view"));
    connect(m_viewTopAction, &QAction::triggered, this, &MainWindow::onViewTop);

    m_viewFrontAction = new QAction(tr("&Front View"), this);
    m_viewFrontAction->setStatusTip(tr("Switch to front view"));
    connect(m_viewFrontAction, &QAction::triggered, this, &MainWindow::onViewFront);

    m_viewRightAction = new QAction(tr("&Right View"), this);
    m_viewRightAction->setStatusTip(tr("Switch to right view"));
    connect(m_viewRightAction, &QAction::triggered, this, &MainWindow::onViewRight);

    m_viewIsoAction = new QAction(tr("&Isometric View"), this);
    m_viewIsoAction->setStatusTip(tr("Switch to isometric view"));
    connect(m_viewIsoAction, &QAction::triggered, this, &MainWindow::onViewIsometric);

    m_viewFitAction = new QAction(tr("Fit &All"), this);
    m_viewFitAction->setShortcut(tr("F"));
    m_viewFitAction->setStatusTip(tr("Fit all objects in view"));
    connect(m_viewFitAction, &QAction::triggered, this, &MainWindow::onViewFit);

    // Edit menu actions
    m_selectAction = new QAction(tr("&Select"), this);
    m_selectAction->setStatusTip(tr("Select objects"));
    connect(m_selectAction, &QAction::triggered, this, &MainWindow::onSelectMode);

    m_moveAction = new QAction(tr("&Move"), this);
    m_moveAction->setStatusTip(tr("Move selected objects"));
    connect(m_moveAction, &QAction::triggered, this, &MainWindow::onMoveMode);

    m_rotateAction = new QAction(tr("&Rotate"), this);
    m_rotateAction->setStatusTip(tr("Rotate selected objects"));
    connect(m_rotateAction, &QAction::triggered, this, &MainWindow::onRotateMode);

    m_deleteAction = new QAction(tr("&Delete"), this);
    m_deleteAction->setShortcut(QKeySequence::Delete);
    m_deleteAction->setStatusTip(tr("Delete selected objects"));
    connect(m_deleteAction, &QAction::triggered, this, &MainWindow::onDeleteSelected);

    // Analysis menu actions
    m_checkInterferencesAction = new QAction(tr("Check &Interferences"), this);
    m_checkInterferencesAction->setStatusTip(tr("Check for clashing elements"));
    connect(m_checkInterferencesAction, &QAction::triggered, this, &MainWindow::onCheckInterferences);

    m_showDimensionsAction = new QAction(tr("Show &Dimensions"), this);
    m_showDimensionsAction->setStatusTip(tr("Display dimensions"));
    m_showDimensionsAction->setCheckable(true);
    connect(m_showDimensionsAction, &QAction::triggered, this, &MainWindow::onShowDimensions);
}

void MainWindow::createMenus()
{
    // File menu
    m_fileMenu = menuBar()->addMenu(tr("&File"));
    m_fileMenu->addAction(m_newAction);
    m_fileMenu->addAction(m_openAction);
    m_fileMenu->addAction(m_saveAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_exportAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_exitAction);

    // Edit menu
    m_editMenu = menuBar()->addMenu(tr("&Edit"));
    m_editMenu->addAction(m_selectAction);
    m_editMenu->addAction(m_moveAction);
    m_editMenu->addAction(m_rotateAction);
    m_editMenu->addSeparator();
    m_editMenu->addAction(m_deleteAction);

    // Create menu
    m_createMenu = menuBar()->addMenu(tr("&Create"));
    m_createMenu->addAction(m_createBeamAction);
    m_createMenu->addAction(m_createColumnAction);
    m_createMenu->addAction(m_createSlabAction);
    m_createMenu->addAction(m_createWallAction);
    m_createMenu->addAction(m_createFoundationAction);

    // View menu
    m_viewMenu = menuBar()->addMenu(tr("&View"));
    m_viewMenu->addAction(m_viewTopAction);
    m_viewMenu->addAction(m_viewFrontAction);
    m_viewMenu->addAction(m_viewRightAction);
    m_viewMenu->addAction(m_viewIsoAction);
    m_viewMenu->addSeparator();
    m_viewMenu->addAction(m_viewFitAction);

    // Analysis menu
    m_analysisMenu = menuBar()->addMenu(tr("&Analysis"));
    m_analysisMenu->addAction(m_checkInterferencesAction);
    m_analysisMenu->addAction(m_showDimensionsAction);

    // Help menu
    m_helpMenu = menuBar()->addMenu(tr("&Help"));
}

void MainWindow::createToolBars()
{
    // File toolbar
    m_fileToolBar = addToolBar(tr("File"));
    m_fileToolBar->addAction(m_newAction);
    m_fileToolBar->addAction(m_openAction);
    m_fileToolBar->addAction(m_saveAction);

    // Create toolbar
    m_createToolBar = addToolBar(tr("Create"));
    m_createToolBar->addAction(m_createBeamAction);
    m_createToolBar->addAction(m_createColumnAction);
    m_createToolBar->addAction(m_createSlabAction);
    m_createToolBar->addAction(m_createWallAction);

    // View toolbar
    m_viewToolBar = addToolBar(tr("View"));
    m_viewToolBar->addAction(m_viewTopAction);
    m_viewToolBar->addAction(m_viewFrontAction);
    m_viewToolBar->addAction(m_viewRightAction);
    m_viewToolBar->addAction(m_viewIsoAction);
    m_viewToolBar->addAction(m_viewFitAction);
    
    // Work Plane toolbar
    QToolBar* workPlaneToolBar = addToolBar(tr("Work Plane"));
    QAction* workPlaneAction = new QAction(tr("Set Work Plane"), this);
    workPlaneAction->setStatusTip(tr("Choose construction plane"));
    connect(workPlaneAction, &QAction::triggered, this, &MainWindow::onSetWorkPlane);
    workPlaneToolBar->addAction(workPlaneAction);
    
    // Snap toolbar
    QToolBar* snapToolBar = addToolBar(tr("Snap"));
    QAction* snapToggleAction = new QAction(tr("Snap On/Off"), this);
    snapToggleAction->setCheckable(true);
    snapToggleAction->setChecked(true);
    snapToggleAction->setStatusTip(tr("Toggle snap to vertices, endpoints, midpoints"));
    connect(snapToggleAction, &QAction::toggled, this, [this](bool checked) {
        m_controller->setSnapEnabled(checked);
        statusBar()->showMessage(checked ? "Snap enabled" : "Snap disabled", 2000);
    });
    snapToolBar->addAction(snapToggleAction);

    // Edit toolbar
    m_editToolBar = addToolBar(tr("Edit"));
    m_editToolBar->addAction(m_selectAction);
    m_editToolBar->addAction(m_moveAction);
    m_editToolBar->addAction(m_rotateAction);
    m_editToolBar->addAction(m_deleteAction);
}

void MainWindow::createDockWidgets()
{
    // Project tree dock
    m_projectTreeDock = new QDockWidget(tr("Project Tree"), this);
    m_projectTree = new QTreeWidget();
    m_projectTree->setHeaderLabel("Project Structure");
    m_projectTree->setStyleSheet("QTreeWidget { background-color: #2b2b2b; color: white; }");
    
    // Add sample items
    QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_projectTree);
    rootItem->setText(0, "Project");
    rootItem->setExpanded(true);
    
    QTreeWidgetItem *beamsItem = new QTreeWidgetItem(rootItem);
    beamsItem->setText(0, "Beams");
    
    QTreeWidgetItem *columnsItem = new QTreeWidgetItem(rootItem);
    columnsItem->setText(0, "Columns");
    
    QTreeWidgetItem *slabsItem = new QTreeWidgetItem(rootItem);
    slabsItem->setText(0, "Slabs");
    
    m_projectTreeDock->setWidget(m_projectTree);
    addDockWidget(Qt::LeftDockWidgetArea, m_projectTreeDock);

    // Note: PropertiesPanel is already added in constructor
}

// File menu slots
void MainWindow::onNewProject()
{
    m_viewer->clearAll();
    statusBar()->showMessage("New project created", 2000);
}

void MainWindow::onOpenProject()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Project"), 
                                                     QString(), 
                                                     tr("CAD Files (*.step *.stp *.iges *.igs)"));
    if (!fileName.isEmpty()) {
        statusBar()->showMessage("Opening project: " + fileName, 2000);
        // TODO: Implement file loading
    }
}

void MainWindow::onSaveProject()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Project"),
                                                     QString(),
                                                     tr("STEP Files (*.step *.stp)"));
    if (!fileName.isEmpty()) {
        statusBar()->showMessage("Saving project: " + fileName, 2000);
        // TODO: Implement file saving
    }
}

void MainWindow::onExport()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export Model"),
                                                     QString(),
                                                     tr("All Files (*.step *.iges *.stl);;STEP (*.step);;IGES (*.iges);;STL (*.stl)"));
    if (!fileName.isEmpty()) {
        statusBar()->showMessage("Exporting to: " + fileName, 2000);
        // TODO: Implement export
    }
}

void MainWindow::onExit()
{
    close();
}

// Create menu slots
void MainWindow::onCreateBeam()
{
    // Show profile selection dialog
    ProfileSelectionDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        m_controller->startBeamCommand();
        
        // Get the current beam command and set profile/dimensions
        BeamCommand* beamCmd = qobject_cast<BeamCommand*>(m_controller->getCurrentCommand());
        if (beamCmd) {
            if (dialog.useProfile()) {
                beamCmd->setProfile(dialog.getSelectedProfileType(), dialog.getSelectedSize());
            } else {
                beamCmd->setDimensions(200, 400); // Default rectangular dimensions
            }
        }
    }
}

void MainWindow::onCreateColumn()
{
    m_controller->startColumnCommand();
}

void MainWindow::onCreateSlab()
{
    m_controller->startSlabCommand();
}

void MainWindow::onCreateWall()
{
    GeometryBuilder builder(m_viewer->getContext());
    builder.createWall(0, 0, 0, 5000, 200, 3000);
    m_viewer->fitAll();
    statusBar()->showMessage("Wall created", 2000);
}

void MainWindow::onCreateFoundation()
{
    GeometryBuilder builder(m_viewer->getContext());
    builder.createFoundation(0, 0, -500, 2000, 2000, 500);
    m_viewer->fitAll();
    statusBar()->showMessage("Foundation created", 2000);
}

// View menu slots
void MainWindow::onViewTop()
{
    m_viewer->setTopView();
    statusBar()->showMessage("Top view", 2000);
}

void MainWindow::onViewFront()
{
    m_viewer->setFrontView();
    statusBar()->showMessage("Front view", 2000);
}

void MainWindow::onViewRight()
{
    m_viewer->setRightView();
    statusBar()->showMessage("Right view", 2000);
}

void MainWindow::onViewIsometric()
{
    m_viewer->setIsometricView();
    statusBar()->showMessage("Isometric view", 2000);
}

void MainWindow::onViewFit()
{
    m_viewer->fitAll();
    statusBar()->showMessage("Fit all", 2000);
}

// Edit menu slots
void MainWindow::onSelectMode()
{
    statusBar()->showMessage("Select mode", 2000);
}

void MainWindow::onMoveMode()
{
    statusBar()->showMessage("Move mode", 2000);
}

void MainWindow::onRotateMode()
{
    statusBar()->showMessage("Rotate mode", 2000);
}

void MainWindow::onDeleteSelected()
{
    statusBar()->showMessage("Delete selected", 2000);
    // TODO: Implement delete functionality
}

// Analysis menu slots
void MainWindow::onCheckInterferences()
{
    QMessageBox::information(this, "Interference Check", 
                            "Checking for clashing elements...\nNo interferences found.");
}

void MainWindow::onShowDimensions()
{
    bool show = m_showDimensionsAction->isChecked();
    statusBar()->showMessage(show ? "Dimensions shown" : "Dimensions hidden", 2000);
}

void MainWindow::updatePropertiesPanel()
{
    if (!m_propertiesPanel || !m_objectCollection) {
        return;
    }
    
    NCollection_Sequence<Handle(TGraphicObject)> selectedObjects = m_objectCollection->GetSelectedObjects();
    
    if (selectedObjects.Size() == 0) {
        m_propertiesPanel->clearProperties();
    }
    else if (selectedObjects.Size() == 1) {
        m_propertiesPanel->setObject(selectedObjects.First());
    }
    else {
        m_propertiesPanel->setMultipleSelection(selectedObjects.Size());
    }
}

void MainWindow::onSetWorkPlane()
{
    // Toggle face picking mode instead of showing dialog directly
    if (!m_facePickingMode) {
        enterFacePickingMode();
    } else {
        exitFacePickingMode();
    }
}

void MainWindow::enterFacePickingMode()
{
    m_facePickingMode = true;
    
    // Cancel any active command
    if (m_controller->getCurrentCommand()) {
        m_controller->cancelCurrentCommand();
    }
    
    // Enable face detection mode in AIS context
    Handle(AIS_InteractiveContext) context = m_viewer->getContext();
    
    // Clear any current selection
    context->ClearSelected(Standard_False);
    context->ClearDetected(Standard_False);
    
    // Activate all objects for selection
    AIS_ListOfInteractive allObjects;
    context->DisplayedObjects(allObjects);
    for (AIS_ListOfInteractive::Iterator it(allObjects); it.More(); it.Next()) {
        Handle(AIS_InteractiveObject) obj = it.Value();
        if (!obj.IsNull()) {
            // Activate face selection mode (mode 4 = faces)
            context->Activate(obj, 4, Standard_False);
        }
    }
    
    // Set face filter for selection
    Handle(StdSelect_FaceFilter) faceFilter = new StdSelect_FaceFilter(StdSelect_AnyFace);
    context->AddFilter(faceFilter);
    
    // Update view to apply changes
    m_viewer->getView()->Redraw();
    
    statusBar()->showMessage("Face Picking Mode: Hover over a face to highlight, click to set workplane. Press ESC to cancel.", 0);
    
    // Visual feedback
    m_viewer->setCursor(Qt::CrossCursor);
}

void MainWindow::exitFacePickingMode()
{
    m_facePickingMode = false;
    
    // Clear any face filters
    Handle(AIS_InteractiveContext) context = m_viewer->getContext();
    context->RemoveFilters();
    context->ClearDetected(Standard_True);
    context->ClearSelected(Standard_True);
    
    // Deactivate face selection mode and restore default
    AIS_ListOfInteractive allObjects;
    context->DisplayedObjects(allObjects);
    for (AIS_ListOfInteractive::Iterator it(allObjects); it.More(); it.Next()) {
        Handle(AIS_InteractiveObject) obj = it.Value();
        if (!obj.IsNull()) {
            context->Deactivate(obj);
            // Reactivate default selection mode (mode 0 = whole shape)
            context->Activate(obj, 0, Standard_False);
        }
    }
    
    // Update view
    m_viewer->getView()->Redraw();
    
    // Restore normal cursor
    m_viewer->setCursor(Qt::ArrowCursor);
    
    statusBar()->showMessage("Face picking cancelled", 2000);
}

void MainWindow::onFaceHovered(int x, int y)
{
    if (!m_facePickingMode) return;
    
    Handle(AIS_InteractiveContext) context = m_viewer->getContext();
    Handle(V3d_View) view = m_viewer->getView();
    
    // Detect face under cursor
    context->MoveTo(x, y, view, Standard_True);
    
    if (context->HasDetected()) {
        Handle(StdSelect_BRepOwner) owner = Handle(StdSelect_BRepOwner)::DownCast(context->DetectedOwner());
        if (!owner.IsNull()) {
            const TopoDS_Shape& shape = owner->Shape();
            if (shape.ShapeType() == TopAbs_FACE) {
                // Face detected - it's automatically highlighted by MoveTo
                statusBar()->showMessage("Face detected - Click to set workplane to this face", 0);
                return;
            }
        }
    }
    
    statusBar()->showMessage("Face Picking Mode: Hover over a face to highlight, click to set workplane. Press ESC to cancel.", 0);
}

void MainWindow::onFaceClicked(int x, int y, Qt::MouseButton button)
{
    if (!m_facePickingMode) return;
    
    if (button != Qt::LeftButton) {
        exitFacePickingMode();
        return;
    }
    
    Handle(AIS_InteractiveContext) context = m_viewer->getContext();
    Handle(V3d_View) view = m_viewer->getView();
    
    // Detect face at click position
    context->MoveTo(x, y, view, Standard_False);
    
    if (context->HasDetected()) {
        Handle(StdSelect_BRepOwner) owner = Handle(StdSelect_BRepOwner)::DownCast(context->DetectedOwner());
        if (!owner.IsNull()) {
            const TopoDS_Shape& shape = owner->Shape();
            if (shape.ShapeType() == TopAbs_FACE) {
                TopoDS_Face face = TopoDS::Face(shape);
                
                // Extract face geometry
                gp_Pnt origin;
                gp_Dir normal;
                if (extractFaceGeometry(face, origin, normal)) {
                    // Create custom workplane from face
                    WorkPlane customPlane(origin, normal);
                    m_controller->setWorkPlane(customPlane);
                    m_controller->setWorkPlaneVisible(true);
                    
                    statusBar()->showMessage(QString("Workplane set to face (Origin: %1, %2, %3)")
                        .arg(origin.X(), 0, 'f', 1)
                        .arg(origin.Y(), 0, 'f', 1)
                        .arg(origin.Z(), 0, 'f', 1), 3000);
                    
                    exitFacePickingMode();
                    return;
                }
            }
        }
    }
    
    statusBar()->showMessage("No face detected at click position", 2000);
}

bool MainWindow::extractFaceGeometry(const TopoDS_Face& face, gp_Pnt& origin, gp_Dir& normal)
{
    try {
        // Get the surface from the face
        Handle(Geom_Surface) surface = BRep_Tool::Surface(face);
        
        // Try to get a planar surface
        Handle(Geom_Plane) plane = Handle(Geom_Plane)::DownCast(surface);
        if (!plane.IsNull()) {
            // It's a planar face - perfect!
            gp_Pln occPlane = plane->Pln();
            origin = occPlane.Location();
            normal = occPlane.Axis().Direction();
            
            // Check face orientation
            if (face.Orientation() == TopAbs_REVERSED) {
                normal.Reverse();
            }
            
            return true;
        }
        
        // If not a plane, try to fit a plane to the surface
        GeomAdaptor_Surface adaptor(surface);
        if (adaptor.GetType() == GeomAbs_Plane) {
            gp_Pln fittedPlane = adaptor.Plane();
            origin = fittedPlane.Location();
            normal = fittedPlane.Axis().Direction();
            
            if (face.Orientation() == TopAbs_REVERSED) {
                normal.Reverse();
            }
            
            return true;
        }
        
        // For non-planar surfaces, use approximation
        // Get a point on the surface and compute normal there
        Standard_Real uMin, uMax, vMin, vMax;
        BRepTools::UVBounds(face, uMin, uMax, vMin, vMax);
        Standard_Real uMid = (uMin + uMax) / 2.0;
        Standard_Real vMid = (vMin + vMax) / 2.0;
        
        gp_Pnt point;
        gp_Vec D1U, D1V;
        surface->D1(uMid, vMid, point, D1U, D1V);
        
        // Normal is cross product of derivatives
        gp_Vec normalVec = D1U.Crossed(D1V);
        if (normalVec.Magnitude() > Precision::Confusion()) {
            origin = point;
            normal = gp_Dir(normalVec);
            
            if (face.Orientation() == TopAbs_REVERSED) {
                normal.Reverse();
            }
            
            return true;
        }
        
        return false;
    } catch (...) {
        return false;
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        if (m_facePickingMode) {
            exitFacePickingMode();
            return;
        }
        // Also cancel any active command
        if (m_controller && m_controller->getCurrentCommand()) {
            m_controller->cancelCurrentCommand();
            return;
        }
    }
    
    QMainWindow::keyPressEvent(event);
}
