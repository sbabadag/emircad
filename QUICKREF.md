# 🏗️ Tekla-Like CAD - Quick Reference

## 🚀 Launch Application
```powershell
.\run.ps1
```

## 🖱️ Mouse Controls
| Action | Control |
|--------|---------|
| **Rotate View** | Left Mouse + Drag |
| **Pan View** | Middle Mouse + Drag |
| **Zoom** | Mouse Wheel |
| **Select Object** | Right Mouse Click |

## ⌨️ Keyboard Shortcuts
| Key | Action |
|-----|--------|
| `F` | Fit All (zoom to fit all objects) |
| `Ctrl+N` | New Project |
| `Ctrl+O` | Open Project |
| `Ctrl+S` | Save Project |
| `Delete` | Delete Selected Object |

## 🏗️ Creating Elements

### Via Menu: **Create → [Element]**
- **Beam** - Horizontal structural element (5000 × 300 × 400 mm)
- **Column** - Vertical support (300 × 300 × 3000 mm)
- **Slab** - Floor/ceiling (5000 × 4000 × 200 mm)
- **Wall** - Vertical partition (5000 × 200 × 3000 mm)
- **Foundation** - Base support (2000 × 2000 × 500 mm)

## 👁️ View Options

### Via Menu: **View → [View Type]**
- **Top View** - Plan view from above (Z-axis)
- **Front View** - Front elevation (Y-axis)
- **Right View** - Right elevation (X-axis)
- **Isometric View** - 3D perspective (default)
- **Fit All** - Zoom to fit all objects

## 📁 Menu Structure

### File Menu
- New Project
- Open Project
- Save Project
- Export (STEP, IGES, STL)
- Exit

### Edit Menu
- Select Mode
- Move Mode
- Rotate Mode
- Delete Selected

### Create Menu
- Create Beam
- Create Column
- Create Slab
- Create Wall
- Create Foundation

### View Menu
- Top View
- Front View
- Right View
- Isometric View
- Fit All

### Analysis Menu
- Check Interferences
- Show Dimensions

## 🎨 User Interface

```
┌─────────────────────────────────────────────────────┐
│ File  Edit  Create  View  Analysis  Help            │ ← Menu Bar
├─────────────────────────────────────────────────────┤
│ [New] [Open] [Save] │ [Beam] [Column] [Slab] [Wall] │ ← Toolbars
├───────┬─────────────────────────────────┬───────────┤
│Project│                                 │Properties │
│Tree   │     3D Viewport                 │Panel      │
│       │                                 │           │
│Beams  │                                 │Name:      │
│Columns│     [3D Model Here]             │Type:      │
│Slabs  │                                 │Length:    │
│       │                                 │Width:     │
│       │                                 │Height:    │
└───────┴─────────────────────────────────┴───────────┘
```

## 🎯 Getting Started

1. **Launch**: Run `.\run.ps1`
2. **Create**: Use Create menu to add a beam
3. **Navigate**: Left-drag to rotate, wheel to zoom
4. **View**: Try different view angles (View menu)
5. **Add More**: Create columns, slabs, etc.

## 🔧 Customization Tips

### Change Element Sizes
Edit `src/MainWindow.cpp`:
```cpp
void MainWindow::onCreateBeam() {
    GeometryBuilder builder(m_viewer->getContext());
    // Modify dimensions (in millimeters):
    builder.createBeam(
        0, 0, 0,          // X, Y, Z position
        10000,            // Length
        400,              // Width
        500               // Height
    );
    m_viewer->fitAll();
}
```

### Change Colors
Edit `include/GeometryBuilder.h` default color parameters:
```cpp
const Quantity_Color& color = Quantity_Color(
    0.5, 0.5, 0.5,    // R, G, B (0.0 to 1.0)
    Quantity_TOC_RGB
)
```

## 🐛 Troubleshooting

| Problem | Solution |
|---------|----------|
| App won't start | Run `.\run.ps1` (not the .exe directly) |
| Missing DLLs | Run `.\run.ps1` - it auto-copies DLLs |
| Black screen | Update graphics drivers |
| Rebuild needed | Run `.\build.ps1` |

## 📂 Project Files

```
OCC/
├── build/bin/Release/TeklaLikeCAD.exe  ← Your app
├── src/                                 ← Source code
│   ├── main.cpp
│   ├── MainWindow.cpp
│   ├── OCCTViewer.cpp
│   └── GeometryBuilder.cpp
├── include/                             ← Headers
│   ├── MainWindow.h
│   ├── OCCTViewer.h
│   └── GeometryBuilder.h
├── run.ps1                             ← Run script
├── build.ps1                           ← Build script
└── README.md                           ← Full docs
```

## 💡 Tips

- **Multiple Elements**: Create several elements before fitting view
- **Experimentation**: Try different view angles to understand structure
- **Organization**: Use project tree to manage elements
- **Performance**: Fewer elements = smoother navigation
- **Colors**: Different element types have different colors for clarity

## 📚 Learn More

- Full Documentation: `README.md`
- Setup Guide: `SETUP.md`
- Success Guide: `SUCCESS.md`

---
**Have fun building 3D structures! 🏗️**
