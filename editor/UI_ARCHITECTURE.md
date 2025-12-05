# Nova Editor UI Architecture

## Overview
This document explains the current UI architecture and clarifies the relationship between different panel types.

## Panel Types

### 1. Static Frame Panels (JSON only, no C++ logic)

#### Inspector Panel (`inspector.json`)
- **Location:** Right side (x=3340, y=100)
- **Purpose:** Visual frame with header and placeholder text
- **Current Content:**
  - Title: "◆ INSPECTOR"
  - Subtitle: "View and edit entity properties"
  - Static text: "No Entity Selected"
  - Close button
- **Status:** ❌ No dynamic content - shows same text regardless of selection

#### Hierarchy Panel (`hierarchy.json`)
- **Location:** Bottom left (x=0, y=1200)
- **Purpose:** Visual frame with header and placeholder text
- **Current Content:**
  - Title: "◆ HIERARCHY"
  - Subtitle: "Scene structure and entity list"
  - Static text: "No Scene Loaded"
  - Close button
- **Status:** ❌ No dynamic content - shows same text even when scene is loaded

### 2. Functional Panels (JSON + C++ classes)

#### Entity Properties Panel (`entity_properties.json` + `EntityPropertiesPanel.hpp`)
- **Location:** Center modal overlay (x=1120, y=400)
- **Purpose:** Modal dialog for editing entity properties
- **C++ Class:** `EntityPropertiesPanel`
- **Features:**
  - Shows: ID, Position (X/Y), Scale (X/Y), Rotation, Layer, Texture
  - +/- buttons to modify values
  - Apply/Cancel buttons
- **Status:** ✅ FIXED - Now updates when entity selection changes
  - Added `updateEntityPropertiesPanel()` method
  - Called after entity selection from scene view or layers panel

#### Layers Panel (`layers_panel.json` + `LayersPanel.hpp`)
- **Location:** Right side, same position as Inspector (x=3340, y=100)
- **Purpose:** Shows entities grouped by layer
- **C++ Class:** `LayersPanel`
- **Features:**
  - 10 layer buttons (Layer 0-9) with entity counts
  - List of entities in currently selected layer
  - Click entity to select it
- **Status:** ✅ Works correctly - updates when scene loads

### 3. Other Functional Panels

#### Time of Day Panel (`time_of_day_panel.json` + `TimeOfDayPanel.hpp`)
- Lighting system controls
- ✅ Fully functional

#### Post-Process Panel (`postprocess_panel.json` + `PostProcessPanel.hpp`)
- Bloom and color grading controls
- ✅ Fully functional

#### Palette Panel (`palette.json`)
- Entity creation categories
- ✅ Has close button, professional design

## Current Issues

### Issue 1: Inspector Panel is Static ❌
**User Report:** "L'inspector est vide, et n'affiche jamais rien peu importe que je sélectionne un sprite"

**Root Cause:**
- `inspector.json` is just a visual frame with no C++ logic behind it
- No dynamic UI elements to populate with entity data
- The actual entity properties functionality is in `EntityPropertiesPanel` (modal dialog)

**Architecture Confusion:**
```
User expects:
Inspector (right side) → Shows selected entity properties inline

Current reality:
Inspector (right side) → Static placeholder text
EntityPropertiesPanel (modal) → Actually shows properties (now updates correctly)
```

**Possible Solutions:**
1. **Add dynamic content to inspector.json** - Add text elements (prop_value_0, prop_value_1, etc.) similar to entity_properties.json
2. **Create InspectorPanel C++ class** - Similar to EntityPropertiesPanel but for inline display
3. **Repurpose existing EntityPropertiesPanel** - Make it work with inspector.json groupID
4. **Remove inspector.json** - Direct users to use the EntityPropertiesPanel modal or LayersPanel

### Issue 2: Hierarchy Panel is Static ❌
**User Report:** "Pareil pour scene hiearchy"

**Root Cause:**
- `hierarchy.json` is just a visual frame with no C++ logic
- No dynamic entity list buttons
- The actual entity list functionality is in `LayersPanel` (different location)

**Architecture Confusion:**
```
User expects:
Hierarchy (bottom left) → Shows all entities in scene

Current reality:
Hierarchy (bottom left) → Static placeholder text
LayersPanel (right side) → Actually shows entity list (works correctly)
```

**Possible Solutions:**
1. **Add dynamic content to hierarchy.json** - Add entity buttons similar to layers_panel.json
2. **Create HierarchyPanel C++ class** - Show flat entity list or tree hierarchy
3. **Remove hierarchy.json** - Direct users to use LayersPanel instead
4. **Repurpose LayersPanel** - Make it work with both groupIDs

### Issue 3: Overlapping Panels
**Observation:** Inspector and LayersPanel are at the same position (x=3340, y=100)

This suggests they were designed to be mutually exclusive (toggle between them). However:
- Inspector is non-functional (static)
- LayersPanel is fully functional

**Current Workaround:** Users can use LayersPanel to select entities, which now correctly triggers EntityPropertiesPanel updates.

## What Was Fixed

### ✅ Entity Selection → Properties Panel Update
**Problem:** EntityPropertiesPanel had an `update()` method but it was never called

**Solution:**
```cpp
// Added to EditorPanelManager.hpp
void updateEntityPropertiesPanel() {
    if (m_entityPropertiesPanel) {
        m_entityPropertiesPanel->update();
    }
}

// Called from EditorApplication.cpp after:
1. Entity clicked in scene view
2. Entity selected from layers panel
3. Deselection (clicking empty space)
```

**Result:** Entity properties now update immediately when selection changes ✅

## Recommendations

### Short Term (Quick Fixes)
1. ✅ **EntityPropertiesPanel updates** - DONE
2. Add tooltips to clarify which panels do what
3. Hide or remove non-functional static panels (inspector.json, hierarchy.json)

### Medium Term (Architecture Improvements)
1. Create `InspectorPanel` C++ class for inline property display in inspector.json
2. Create `HierarchyPanel` C++ class for entity list in hierarchy.json
3. Or consolidate functionality - remove redundant panels

### Long Term (UI Redesign)
1. Decide on single source of truth for each feature:
   - Entity properties: Modal dialog OR inline inspector?
   - Entity list: Layers panel OR hierarchy panel?
2. Implement tree-based hierarchy (parent-child relationships)
3. Add search/filter functionality for entity lists

## Testing the Fix

To verify EntityPropertiesPanel now updates correctly:
1. Load a scene with entities
2. Click on an entity in the scene view
   - **Expected:** EntityPropertiesPanel modal should show entity's properties
3. OR: Open LayersPanel and click an entity button
   - **Expected:** EntityPropertiesPanel modal should show entity's properties
4. Click empty space to deselect
   - **Expected:** EntityPropertiesPanel should clear/hide

## Commits
- `4a060f5` - Fix: Wire entity properties panel updates after entity selection
- `f791035` - UI: Professional redesign with modern aesthetics and close buttons
- `f951ed5` - UI: Lighten text colors for better readability
- `cd8c0aa` - Fix: Support 'texts' (plural) in addition to 'text' (singular)
- `bacbb79` - UI: Complete professional interface refactoring with comprehensive text labels
- `6cefe2a` - Fix: Critical use-after-free bug in scene loading
