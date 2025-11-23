# Steel Profile System Guide

## Overview
The CAD application now supports standard structural steel profiles including:
- **IPE** - European I-Beams
- **HEA** - European Wide Flange (Light)
- **HEB** - European Wide Flange (Medium)
- **HEM** - European Wide Flange (Heavy)
- **RHS** - Rectangular Hollow Sections

## Available Profiles

### IPE Profiles (European I-Beams)
- IPE 80, 100, 120, 140, 160, 180, 200, 220, 240, 270, 300
- IPE 330, 360, 400, 450, 500, 550, 600

### HEA Profiles (Wide Flange - Light)
- HEA 100, 120, 140, 160, 180, 200, 220, 240, 260, 280
- HEA 300, 320, 340, 360, 400, 450, 500

### HEB Profiles (Wide Flange - Medium)
- HEB 100, 120, 140, 160, 180, 200, 220, 240, 260, 280
- HEB 300, 320, 340, 360, 400, 450, 500

### HEM Profiles (Wide Flange - Heavy)
- HEM 100, 120, 140, 160, 180, 200, 220, 240, 260, 280
- HEM 300, 320, 340, 360

### RHS Profiles (Rectangular Hollow Sections)
- RHS 50x30x3, 60x40x3, 80x40x3, 80x60x3
- RHS 100x50x4, 100x60x4, 120x80x5, 140x80x5
- RHS 150x100x5, 160x80x5, 180x100x6, 200x100x6
- RHS 200x120x6, 250x150x8, 300x200x10

## How to Use

### Creating a Beam with Steel Profile

1. **Open Profile Selection Dialog**
   - Go to menu: `Create → Beam`
   - The Profile Selection Dialog will appear

2. **Select Profile Type**
   - Choose from dropdown: IPE, HEA, HEB, HEM, or RHS
   - Size list will update automatically

3. **Select Size**
   - Browse available sizes in the list
   - Click to select and see dimensions
   - Double-click to accept quickly

4. **View Dimensions**
   - For I-profiles (IPE, HEA, HEB, HEM):
     * Height (h) - Overall height
     * Width (b) - Flange width
     * Web Thickness (tw)
     * Flange Thickness (tf)
     * Root Radius (r)
   
   - For RHS profiles:
     * Height
     * Width
     * Wall Thickness

5. **Place the Beam**
   - Click OK to confirm selection
   - Status bar shows: "Select beam start point (Profile: XXX)"
   - **Ctrl + Left Click** on 3D view for start point
   - **Ctrl + Left Click** again for end point
   - Beam with selected profile is created

### Using Rectangular Section

If you prefer a simple rectangular beam instead of a standard profile:
1. Open Profile Selection Dialog
2. Click **"Use Rectangular Section"** button
3. Default dimensions: 200mm x 400mm
4. Place beam using Ctrl + Click

## Profile Geometry

### I-Profile Cross-Section
The I-profiles (IPE, HEA, HEB, HEM) are created with accurate geometry:
- Top and bottom flanges
- Central web
- Proper thickness for web and flanges
- Correct dimensions according to European standards

### RHS Cross-Section
Rectangular Hollow Sections have:
- Hollow rectangular cross-section
- Uniform wall thickness
- Inner dimensions calculated from outer dimensions minus 2× wall thickness

## Technical Details

### Dimensions
All dimensions are in **millimeters (mm)** matching European steel standards.

### Profile Alignment
Beams are automatically aligned between start and end points:
- Profile is extruded along beam axis
- Automatic rotation to match beam direction
- Centered on beam axis

### Coordinate System
- X-axis: Beam length direction
- Y-axis: Profile width
- Z-axis: Profile height

## Code Architecture

### Key Classes

**SteelProfile** (`SteelProfile.h/cpp`)
- Static class managing profile database
- Creates 3D geometry for each profile type
- Methods:
  - `createProfile()` - Generate 3D shape
  - `getAvailableSizes()` - List sizes for profile type
  - `getDimensions()` - Get dimensions for specific profile

**ProfileSelectionDialog** (`ProfileSelectionDialog.h/cpp`)
- Qt dialog for selecting profiles
- Interactive profile browser
- Real-time dimension display
- Returns selected profile type and size

**BeamCommand** (`BeamCommand.h/cpp`)
- Enhanced with profile support
- Methods:
  - `setProfile()` - Set steel profile
  - `setDimensions()` - Set rectangular dimensions
  - `createBeam()` - Creates geometry (profile or rectangular)

## Example Usage in Code

```cpp
// Create a beam with IPE 300 profile
BeamCommand* cmd = new BeamCommand(context);
cmd->setProfile(SteelProfile::IPE, "IPE 300");
cmd->execute(startPoint);
cmd->execute(endPoint);

// Create a beam with HEB 200 profile
cmd->setProfile(SteelProfile::HEB, "HEB 200");

// Create a beam with rectangular section
cmd->setDimensions(250, 400);  // width, height in mm
```

## Future Enhancements

Potential additions to the profile system:
- **CHS** - Circular Hollow Sections
- **SHS** - Square Hollow Sections
- **UPE/UPN** - European Channels
- **L-profiles** - Angle sections
- **T-profiles** - T-sections
- Custom profile creation
- Profile rotation around beam axis
- Profile library import/export
- Material properties database
- Section property calculations (Area, Inertia, etc.)

## Standards Reference

Profiles follow European standards:
- **EN 10365** - Hot rolled steel channels, I and H sections
- **EN 10219** - Cold formed welded structural hollow sections
- Dimensions match Eurocodes for structural design

## Tips

1. **Quick Selection**: Double-click on a size to select and close dialog
2. **Profile Preview**: Future version will show 3D preview in dialog
3. **Keyboard Navigation**: Use arrow keys to browse profile list
4. **Default Profile**: IPE 200 is selected by default (most common)
5. **Profile Naming**: Names follow European convention (e.g., "IPE 300" = 300mm height)

## Troubleshooting

**Profile not visible after creation?**
- Check zoom level (use mouse wheel)
- Verify start and end points are different
- Check status bar for creation confirmation

**Dialog not opening?**
- Ensure application is properly compiled
- Check that ProfileSelectionDialog is linked

**Wrong profile dimensions?**
- All dimensions are in millimeters
- Verify against EN standards if needed
- Check dimensions display in dialog before confirming
