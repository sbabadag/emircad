#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QDockWidget>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <TopoDS_Face.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include "OCCTViewer.h"
#include "CADController.h"
#include "PropertiesPanel.h"
#include "TObjectCollection.h"
#include "WorkPlaneDialog.h"
#include "SnapToolbar.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // File menu actions
    void onNewProject();
    void onOpenProject();
    void onSaveProject();
    void onExport();
    void onExit();

    // Create menu actions
    void onCreateBeam();
    void onCreateColumn();
    void onCreateSlab();
    void onCreateWall();
    void onCreateFoundation();

    // View menu actions
    void onViewTop();
    void onViewFront();
    void onViewRight();
    void onViewIsometric();
    void onViewFit();

    // Edit menu actions
    void onSelectMode();
    void onMoveMode();
    void onRotateMode();
    void onDeleteSelected();

    // Analysis menu actions
    void onCheckInterferences();
    void onShowDimensions();
    
    // Work plane actions
    void onSetWorkPlane();
    void onFaceHovered(int x, int y);
    void onFaceClicked(int x, int y, Qt::MouseButton button);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createDockWidgets();
    void setupUI();
    void updatePropertiesPanel();
    void enterFacePickingMode();
    void exitFacePickingMode();
    bool extractFaceGeometry(const TopoDS_Face& face, gp_Pnt& origin, gp_Dir& normal);

    // Central widget
    OCCTViewer *m_viewer;
    
    // CAD Controller
    CADController *m_controller;
    
    // Object collection manager
    TObjectCollection *m_objectCollection;

    // Dock widgets
    QDockWidget *m_projectTreeDock;
    QTreeWidget *m_projectTree;
    PropertiesPanel *m_propertiesPanel;
    SnapToolbar *m_snapToolbar;

    // Menus
    QMenu *m_fileMenu;
    QMenu *m_editMenu;
    QMenu *m_createMenu;
    QMenu *m_viewMenu;
    QMenu *m_analysisMenu;
    QMenu *m_helpMenu;

    // Toolbars
    QToolBar *m_fileToolBar;
    QToolBar *m_createToolBar;
    QToolBar *m_viewToolBar;
    QToolBar *m_editToolBar;

    // File menu actions
    QAction *m_newAction;
    QAction *m_openAction;
    QAction *m_saveAction;
    QAction *m_exportAction;
    QAction *m_exitAction;

    // Create menu actions
    QAction *m_createBeamAction;
    QAction *m_createColumnAction;
    QAction *m_createSlabAction;
    QAction *m_createWallAction;
    QAction *m_createFoundationAction;

    // View menu actions
    QAction *m_viewTopAction;
    QAction *m_viewFrontAction;
    QAction *m_viewRightAction;
    QAction *m_viewIsoAction;
    QAction *m_viewFitAction;

    // Edit menu actions
    QAction *m_selectAction;
    QAction *m_moveAction;
    QAction *m_rotateAction;
    QAction *m_deleteAction;

    // Analysis menu actions
    QAction *m_checkInterferencesAction;
    QAction *m_showDimensionsAction;
    
    // Face picking mode for workplane
    bool m_facePickingMode;
};

#endif // MAINWINDOW_H
