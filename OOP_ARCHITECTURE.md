# NEW OOP CAD SYSTEM - ARCHITECTURE DOCUMENTATION

## Overview
Complete object-oriented CAD system built from scratch with clean architecture and extensibility.

## Architecture Components

### 1. CADCommand (Base Class)
**File**: `CADCommand.h/cpp`
- Abstract base class for all CAD commands
- Handles preview display and shape rendering
- Signal-based communication with UI

**Key Methods**:
- `execute(gp_Pnt)` - Execute command with point input
- `isComplete()` - Check if command is finished
- `getPrompt()` - Get user prompt message
- `showPreview()` - Display yellow preview shape
- `clearPreview()` - Remove preview
- `displayShape()` - Add shape to scene

### 2. Concrete Commands

#### BeamCommand
**File**: `BeamCommand.h/cpp`
- Two-point beam creation
- Configurable width and height
- Automatic alignment and rotation
- Real-time length calculation

**Usage**:
```cpp
BeamCommand* cmd = new BeamCommand(context);
cmd->setDimensions(200.0, 400.0);  // width, height in mm
cmd->execute(startPoint);
cmd->execute(endPoint);  // Creates beam
```

#### ColumnCommand
**File**: `ColumnCommand.h/cpp`
- Single-point column placement
- Centered at base point
- Configurable width, depth, height

**Usage**:
```cpp
ColumnCommand* cmd = new ColumnCommand(context);
cmd->setDimensions(300.0, 300.0, 3000.0);  // width, depth, height
cmd->execute(basePoint);  // Creates column
```

#### SlabCommand
**File**: `SlabCommand.h/cpp`
- Two-corner slab creation
- Automatic area calculation
- Configurable thickness

**Usage**:
```cpp
SlabCommand* cmd = new SlabCommand(context);
cmd->setThickness(200.0);  // thickness in mm
cmd->execute(corner1);
cmd->execute(corner2);  // Creates slab
```

### 3. CADController
**File**: `CADController.h/cpp`
- Central coordinator for all commands
- Manages active command lifecycle
- Converts screen coordinates to 3D points
- Signal routing between commands and UI

**Key Methods**:
- `startBeamCommand()` - Start beam drawing
- `startColumnCommand()` - Start column placement
- `startSlabCommand()` - Start slab creation
- `handleClick(point)` - Process user clicks
- `convertViewToWorld(x, y, view)` - Screen to world conversion

### 4. Integration with MainWindow
- Commands triggered from menu/toolbar actions
- Ctrl+Click to place points (normal click rotates view)
- Status bar shows real-time feedback
- Automatic command continuation for multiple objects

## Usage Flow

1. **Start Command**: User clicks menu → `startBeamCommand()` called
2. **Status Update**: "Select beam start point (Width: 200mm, Height: 400mm)"
3. **User Interaction**: Ctrl+Click on viewport
4. **Coordinate Conversion**: Screen → 3D world point
5. **Command Execution**: `command->execute(point)`
6. **Progress Feedback**: "Start point selected. Select end point"
7. **Second Point**: Ctrl+Click again
8. **Shape Creation**: Beam created and displayed
9. **Command Continues**: Ready for next beam

## Key Features

### Signal-Based Communication
```cpp
signals:
    void commandCompleted(const TopoDS_Shape& shape);
    void commandCancelled();
    void statusUpdate(const QString& message);
```

### Preview System
- Yellow transparent preview during construction
- Automatic cleanup on completion/cancellation
- Real-time dimension display

### Extensibility
To add a new command:

1. Create `NewCommand.h` inheriting from `CADCommand`
2. Implement required virtuals:
   - `execute(point)`
   - `isComplete()`
   - `getPrompt()`
3. Create shape in `createShape()` method
4. Add to `CADController`:
   ```cpp
   void CADController::startNewCommand() {
       NewCommand* cmd = new NewCommand(m_context);
       setActiveCommand(cmd);
   }
   ```
5. Connect to UI in MainWindow

## Benefits

1. **Clean Separation**: Commands independent of UI
2. **Reusability**: Same command can be used programmatically
3. **Testability**: Commands can be unit tested
4. **Extensibility**: Easy to add new structural elements
5. **Maintainability**: Clear single-responsibility classes

## File Structure

```
include/
    CADCommand.h          // Base command class
    BeamCommand.h         // Beam drawing
    ColumnCommand.h       // Column placement
    SlabCommand.h         // Slab creation
    CADController.h       // Command controller
    MainWindow.h          // UI integration
    OCCTViewer.h          // 3D viewport

src/
    CADCommand.cpp
    BeamCommand.cpp
    ColumnCommand.cpp
    SlabCommand.cpp
    CADController.cpp
    MainWindow.cpp
    OCCTViewer.cpp
```

## CMakeLists.txt Configuration

All new files added to SOURCES and HEADERS lists for compilation.

## Future Enhancements

- Add WallCommand class
- Add FoundationCommand class
- Implement snap system (grid, endpoint, midpoint)
- Add modification commands (Move, Rotate, Copy)
- Add undo/redo support
- Add dimension annotations
- Add property editing
- Add file save/load

## How to Use

1. **Build**: `cmake --build build --config Release`
2. **Run**: `.\run.ps1`
3. **Draw Beam**: Create → Beam, then Ctrl+Click start and end points
4. **Draw Column**: Create → Column, then Ctrl+Click base point
5. **Draw Slab**: Create → Slab, then Ctrl+Click two corners
6. **Rotate View**: Normal left-click and drag
7. **Pan**: Middle-click and drag
8. **Zoom**: Mouse wheel

## Code Quality

- ✅ Clean OOP design
- ✅ SOLID principles
- ✅ Qt signals/slots
- ✅ OpenCascade best practices
- ✅ Memory management (Qt parent-child, OpenCascade handles)
- ✅ Error handling
- ✅ User feedback
- ✅ Extensible architecture
