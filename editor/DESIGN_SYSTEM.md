# NOVA EDITOR - TOP SECRET DESIGN SYSTEM

## Color Palette

### Dark Theme (Top Secret)
```
Background Levels:
- L0 (Deepest):     10,14,20     rgb(10,14,20)      #0a0e14
- L1 (Panel BG):    21,27,36     rgb(21,27,36)      #151b24
- L2 (Header):      31,41,55     rgb(31,41,55)      #1f2937
- L3 (Elevated):    45,55,72     rgb(45,55,72)      #2d3748

Accents:
- Cyan (Primary):   0,217,255    rgb(0,217,255)     #00d9ff
- Green (Success):  0,255,136    rgb(0,255,136)     #00ff88
- Red (Danger):     255,0,85     rgb(255,0,85)      #ff0055
- Yellow (Warning): 255,204,0    rgb(255,204,0)     #ffcc00

Text:
- Primary:          229,231,235  rgb(229,231,235)   #e5e7eb
- Secondary:        156,163,175  rgb(156,163,175)   #9ca3af
- Tertiary:         107,114,128  rgb(107,114,128)   #6b7280
- Disabled:         75,85,99     rgb(75,85,99)      #4b5563

Borders:
- Subtle:           31,41,55     rgb(31,41,55)      #1f2937
- Medium:           55,65,81     rgb(55,65,81)      #374151
- Strong:           0,217,255    rgb(0,217,255)     #00d9ff (accent)
```

## Typography

### Fonts
- Headers: Arial Bold / C:/Windows/Fonts/arialbd.ttf
- Body: Arial / C:/Windows/Fonts/arial.ttf
- Monospace (values): Courier New / C:/Windows/Fonts/cour.ttf

### Sizes
- Title (Panel Headers): 20px
- Subtitle: 14px
- Body: 16px
- Small: 13px
- Button: 15px

## Layout Guidelines

### Panel Structure
```
┌─────────────────────────────────┐
│ ███ TITLE            [ X ]      │ ← Header (40px height)
├─────────────────────────────────┤
│ Content Area                    │
│                                 │
│ - 10px padding all sides        │
│ - 15px spacing between sections │
│                                 │
└─────────────────────────────────┘
```

### Button Specs
- Height: 35px
- Padding: 10px horizontal
- Border: 1px solid #374151
- Hover: Border #00d9ff, Text #00d9ff
- Active: Background #1f2937

### Spacing System
- XS: 5px
- SM: 10px
- MD: 15px
- LG: 20px
- XL: 30px

## Panel Specifications

### Toolbar (Top, Always Visible)
- Position: Top center
- Width: 100% or centered group
- Height: 50px
- Background: #151b24 with 2px bottom border #1f2937
- Contains: Quick action buttons with icons

### Inspector (Right Side)
- Position: x=3340, y=100
- Size: 500x900px
- Background: #151b24
- Border: 1px solid #1f2937
- Shows: Selected entity properties inline

### Hierarchy (Bottom Left)
- Position: x=20, y=1200
- Size: 480x900px
- Background: #151b24
- Border: 1px solid #1f2937
- Shows: Scene entity list

### Palette (Left Side)
- Position: x=20, y=100
- Size: 300x600px
- Background: #151b24
- Border: 1px solid #1f2937
- Shows: Entity creation categories

### Layers Panel (Right Side, toggle with Inspector)
- Same position as Inspector
- Shows: Entity layers organization

## Visual Effects
- All panels: subtle inner shadow for depth
- Buttons: glow on hover (#00d9ff @ 30% opacity)
- Selected items: cyan left border (3px)
- Separators: 1px lines with #1f2937

## Icons (Text-based)
- Close: ×
- Minimize: −
- Menu: ≡
- Add: +
- Remove: −
- Refresh: ↻
- Settings: ⚙
- Info: ⓘ
