# Crankshaft UI Guidelines

Based on Google Design for Driving principles, this document provides UI design standards for the Crankshaft automotive infotainment system. These guidelines ensure that the application prioritises driver safety, minimises distraction, and maintains usability whilst driving.

## Table of Contents

1. [Interaction Design Principles](#interaction-design-principles)
2. [Visual Design Principles](#visual-design-principles)
3. [Typography](#typography)
4. [Spacing & Layout](#spacing--layout)
5. [Colours](#colours)
6. [Touch Targets](#touch-targets)
7. [Icons](#icons)
8. [Components](#components)
9. [Accessibility](#accessibility)

---

## Interaction Design Principles

### Keep Information Current and Glanceable

**Objective:** Drivers must understand tasks and system states by glancing at the screen for 2 seconds maximum before returning focus to the road.

#### Convey Tasks & States at a Glance
- Display only essential information relevant to the current driving context
- Use clear, immediately understandable visual indicators (icons, colour, animations)
- Avoid requiring drivers to read long explanations or navigate multiple screens
- Show system state changes immediately (e.g., connection status, active function)

#### Provide Quick Responses
- System response time after user input must not exceed **0.25 seconds**
- If content takes longer than 2 seconds to load, display a spinner or progress indicator
- Provide visual feedback (e.g., ripple animations) to confirm user input registration
- Example: When user taps a button, ripple animation should appear within 250ms

#### Provide Timely, Accurate Driving Information
- Show navigation directions, speed, fuel status, and warnings immediately
- Prioritise real-time display of malfunction or safety alerts
- Update traffic and navigation data without requiring user action
- Keep clock, temperature, and other status information current

### Encourage Hands-On Driving

**Objective:** Enable drivers to control the vehicle with both hands on the steering wheel at all times.

#### Use One-Handed Gestures
- All interactions must be completable with one hand
- Avoid gestures requiring hand removal from the steering wheel
- Use simple swipes, taps, and rotary inputs
- Example: Swipe right to navigate back, tap centre for confirm, rotate knob for volume

#### Allow Hands-Free Speech Interface
- Implement voice commands for critical functions
- Support hands-free speech for phone calls, navigation, media control
- Allow starting/ending voice sessions with simple button press or voice trigger
- Example: "Crankshaft, navigate to [location]" or "Call [contact]"

#### Simplify Voice Interactions
- Limit voice interaction sequences to 2–3 steps maximum
- Provide clear prompts and confirmations
- Avoid complex multi-step voice workflows
- Example: "Navigate to" → [destination selected] → confirmation (3 steps)

### Prioritise Driving Tasks

**Objective:** Ensure that driving-related information is always prioritised over entertainment or non-essential content.

#### Allow Driver to Control Pace
- Make all interaction sequences interruptible at logical points
- Allow drivers to suspend a task and resume later without penalty
- Never force completion of non-driving tasks
- Example: Media selection should not require completion before returning to navigation

#### Prioritise Driving-Related Tasks
- Information needed for safe vehicle control (navigation, speed, alerts) is always visible
- Entertainment, media, and non-driving information is secondary
- Arrange screens with driving information in primary viewing area
- Example: Navigation map occupies 60% of screen, media controls occupy 40%

#### Consider Non-Driving Content Carefully
- Minimise advertising, social media, emails, and web content while driving
- Non-driving content must be easily ignorable or dismissible
- Disable content that requires sustained attention during driving
- Example: News ticker should not auto-scroll; require user tap to advance

#### Prioritise Sound & Adjust Volume for Driving Tasks
- Navigation voice guidance overrides media volume automatically
- Safety alerts (warnings, incoming calls) override all other audio
- Drivers must be able to fully mute audio including alerts
- Implement audio ducking: lower media volume when navigation speaks

### Discourage Distraction

**Objective:** Eliminate or restrict features that could dangerously distract the driver.

#### Avoid Hazardous or Distracting Activities
- Do not allow games, continuous internet browsing, or fitness tracking during driving
- Restrict features requiring deep cognitive engagement
- Disable or significantly limit non-driving functions
- Example: Gaming mode disabled; web browser limited to quick searches only

#### Avoid Irrelevant Movement
- Do not display auto-scrolling text, videos, or auto-playing animations
- Use animations only to support driving tasks (e.g., turn-by-turn animations)
- Limit motion to driver-initiated actions
- Example: No animated advertisements; only turn indicators for navigation

---

## Visual Design Principles

### Make Content Easy to Read

**Objective:** Text, icons, and UI elements must be legible and glanceable under all driving conditions (sunlight, night, rain).

#### Display Legible Fonts
- **Primary text (e.g., song titles, contact names):** 32dp minimum
- **Secondary text (e.g., artist name, call type):** 24dp minimum
- **Labels and small text:** 16dp minimum
- Use sans-serif fonts (e.g., Roboto, Segoe UI) for clarity
- Avoid thin or decorative fonts
- Example:
  - Song Title: 32dp, bold
  - Artist Name: 24dp, regular
  - Call Type: 24dp, light

#### Limit Text Length
- Text items must not exceed **120 characters** (including punctuation and spaces)
- Truncate long text with ellipsis (…)
- Use concise, natural language
- Rationale: Longer text requires longer glances, increasing distraction
- Example: "Now Playing: 'The Middle' by Jimmy Eat World" (48 chars) ✓
- Avoid: "Now Playing: A really long song title that goes on and on" ✗

#### Follow Contrast Ratios for Text, Icons & Background
- **Minimum contrast ratio: 4.5:1** for all icons, text, and images
- Test contrast in both light and dark modes
- Redundant information (e.g., icon + text) needs only one element to meet ratio
- Example: White text (#FFFFFF) on dark grey background (#333333) = ~12:1 contrast ✓

#### Provide Glanceability at Night
- **Daytime:** Allow both positive (dark text on light) and negative (light text on dark) polarity
- **Nighttime:** Use negative polarity only (light text on dark background)
- Implement automatic day/night switching based on time or sensor input
- Avoid colours that cause glare or after-images in darkness
- Example:
  - Daytime: Black text on white background (positive)
  - Nighttime: White text on dark grey background (negative)

### Make Targets Easy to Touch

**Objective:** Touch targets must be large enough to easily tap whilst driving, even with vehicle vibration.

#### Display Appropriately Sized Touch Targets
- **Minimum touch target size: 76 × 76dp** (approximately 9mm × 9mm)
- For special cases, prioritise height over width to accommodate vehicle vibration
- Example: 76dp wide × 88dp tall for vertical gestures
- All buttons, sliders, and interactive elements must meet this requirement

#### Avoid Touch Target Overlap
- Touch targets must not overlap
- Maintain **minimum 23dp spacing** between touch targets where possible
- Use zoom or modal interfaces if necessary to separate targets
- Example: Button row with 12dp spacing between buttons ✓

### Keep UI Elements Consistent

**Objective:** Consistent, predictable interfaces reduce cognitive load and decision time.

#### Use Consistent Map Icons, Terminology & Interaction Patterns
- Map each icon to exactly one function (e.g., 🧭 = Navigation)
- Use consistent terminology across all screens
- Interaction patterns should be predictable and repeatable
- Example:
  - 🧭 always represents Navigation
  - ♫ always represents Music
  - ⚙ always represents Settings
  - Back arrow always navigates back one level

#### Clarify Active & Inactive Features While in Motion
- Clearly distinguish available features from unavailable features
- Dim or disable non-driving functions
- Use visual indicators (colour, opacity, icons) to show state
- Hide features not intended for use whilst driving
- Example:
  - Active: Bright, opaque buttons
  - Inactive: Dimmed (50% opacity), disabled state
  - Unavailable during driving: Hidden or greyed out (25% opacity)

---

## Typography

### Font Scale

| Usage | Size | Weight | Example |
|-------|------|--------|---------|
| Page Title | 32dp | Bold | "Navigation", "Media" |
| Section Header | 28dp | Bold | "Recent Contacts", "Playlists" |
| Primary Content | 24dp | Regular | Contact name, Song title, Address |
| Secondary Content | 20dp | Regular | Artist name, Call type, Distance |
| Labels | 16dp | Regular | "From:", "To:", "Distance:" |
| Small Text | 14dp | Light | Disclaimers, status messages |

### Font Family

- **Primary:** Roboto (Android standard)
- **Fallback:** Segoe UI, system sans-serif
- **Avoid:** Serif fonts, thin weights, decorative fonts

### Text Colour

- **Light Mode:** Dark grey (#333333) for primary, medium grey (#666666) for secondary
- **Dark Mode:** White (#FFFFFF) for primary, light grey (#CCCCCC) for secondary

---

## Spacing & Layout

### Padding & Margins

| Element | Spacing |
|---------|---------|
| Screen edges to content | 16dp |
| Section headers | 24dp top, 12dp bottom |
| Buttons within rows | 12dp between buttons |
| Touch targets | 76 × 76dp minimum, 23dp spacing |
| Card padding | 12dp on all sides |

### Layout Grid

- Use an **8dp grid system**
- All spacing must be multiples of 8dp (8, 16, 24, 32, 40, etc.)
- Ensures consistent alignment and scalability
- Example: 16dp margins (2 × 8dp), 24dp padding (3 × 8dp)

### Screen Layout Structure

```
┌─────────────────────────────────┐
│  [Icon] Status Bar (16dp top)   │  8dp total
├─────────────────────────────────┤
│  ┌─────────────────────────────┐ │
│  │  Primary Content Area       │ │  60% of screen
│  │  (Navigation, Current Info) │ │
│  └─────────────────────────────┘ │
│  (16dp padding)                  │
├─────────────────────────────────┤
│  ┌─────────────────────────────┐ │
│  │  Secondary Content Area     │ │  40% of screen
│  │  (Media, Controls)          │ │
│  └─────────────────────────────┘ │
└─────────────────────────────────┘
```

---

## Colours

### Light Mode

| Element | Colour | Usage |
|---------|--------|-------|
| Background | #FFFFFF | Main background |
| Text Primary | #333333 | Main text |
| Text Secondary | #666666 | Supporting text |
| Accent | #2196F3 | Interactive elements, highlights |
| Surface | #F5F5F5 | Cards, containers |
| Border | #E0E0E0 | Dividers, borders |
| Success | #4CAF50 | Positive actions, confirmations |
| Warning | #FF9800 | Warnings, alerts |
| Error | #F44336 | Errors, critical alerts |

### Dark Mode

| Element | Colour | Usage |
|---------|--------|-------|
| Background | #121212 | Main background |
| Text Primary | #FFFFFF | Main text |
| Text Secondary | #CCCCCC | Supporting text |
| Accent | #64B5F6 | Interactive elements, highlights |
| Surface | #1E1E1E | Cards, containers |
| Border | #404040 | Dividers, borders |
| Success | #81C784 | Positive actions, confirmations |
| Warning | #FFB74D | Warnings, alerts |
| Error | #EF5350 | Errors, critical alerts |

### Contrast Requirements

- All text and icons must maintain **4.5:1 minimum contrast** against their background
- Test all colour combinations in both light and dark modes
- Use colour contrast checking tools (e.g., WebAIM Contrast Checker)

---

## Touch Targets

### Button Sizes

| Button Type | Size | Spacing |
|------------|------|---------|
| Primary/Large | 76 × 76dp | 23dp between |
| Secondary/Small | 60 × 60dp minimum | 16dp between |
| Icon buttons | 56 × 56dp minimum | 12dp between |

### Touch Target Guidelines

1. **Minimum 76 × 76dp** for main interactive elements
2. **Maintain 23dp spacing** between targets to prevent accidental taps
3. **Height-priority layout** for vehicle vibration tolerance (76dp wide × 88dp tall)
4. **No overlapping targets** — use modal or zoom for dense controls
5. **Clear visual feedback** on tap (ripple, colour change, 250ms response)

---

## Icons

### Icon Set

The application uses Unicode-based icons for maximum compatibility and reliability. All icons are mapped to consistent meanings:

| Icon | Meaning | Usage |
|------|---------|-------|
| 🧭 | Navigation | Navigation screen access, direction indicators |
| ♫ | Music | Media player, music control |
| ☎ | Phone | Phone/call controls, contacts |
| ⚙ | Settings | Settings menu, preferences |
| ⌂ | Home | Home screen, dashboard |
| ✚ | Add | Add new item, create new |
| ✓ | Confirm | Confirm action, checkmark |
| ⚠ | Warning | Warning messages, alerts |
| ← | Back | Navigate to previous screen |
| → | Next | Navigate to next item |
| ★ | Favourite | Mark as favourite, favourite items |
| ♥ | Like | Like action, preferences |
| ⊙ | Power | Power on/off, system power |
| ⏸ | Pause | Pause playback, pause action |
| ▶ | Play | Play action, start playback |
| ◻ | Stop | Stop action, stop playback |

### Icon Guidelines

- **Minimum size:** 24dp for UI icons, 32dp for buttons
- **Consistency:** Same icon always represents same function
- **Colour:** Inherit from text colour or use accent colour
- **Contrast:** Must meet 4.5:1 minimum contrast ratio
- **Redundancy:** For critical functions, pair icon with text label

---

## Components

### Button Component

```qml
// Primary Button (for main actions)
Button {
    width: 76; height: 76
    text: "Next"
    onClicked: goToNextScreen()
    
    // Styling
    background: Rectangle {
        color: isHovered ? "#1976D2" : "#2196F3"  // Accent colour
        radius: 4
    }
    
    contentItem: Text {
        text: parent.text
        color: "#FFFFFF"
        font.pixelSize: 24
        horizontalAlignment: Text.AlignHCenter
    }
}

// Secondary Button (for secondary actions)
Button {
    width: 60; height: 60
    
    background: Rectangle {
        color: isHovered ? "#F5F5F5" : "#FFFFFF"
        border.color: "#E0E0E0"
        border.width: 1
    }
}
```

### Card Component

```qml
// Information Card
Card {
    padding: 12  // 12dp padding
    spacing: 8
    
    Text {
        text: "Contact Name"
        font.pixelSize: 32
        font.bold: true
        color: isDarkMode ? "#FFFFFF" : "#333333"
    }
    
    Text {
        text: "Mobile • +1 234 567 8900"
        font.pixelSize: 20
        color: isDarkMode ? "#CCCCCC" : "#666666"
    }
}
```

### Text Input Component

```qml
// Search/Input Field
TextField {
    width: 300
    height: 56  // Touch target sized
    
    placeholderText: "Search…"
    font.pixelSize: 20
    
    background: Rectangle {
        color: isDarkMode ? "#1E1E1E" : "#F5F5F5"
        border.color: "#2196F3"
        border.width: 2
        radius: 4
    }
}
```

### Toggle/Switch Component

```qml
// Settings Toggle
Switch {
    width: 56; height: 32
    checked: isDarkMode
    
    onCheckedChanged: toggleDarkMode()
    
    // High contrast for visibility
    indicator: Rectangle {
        color: checked ? "#4CAF50" : "#CCCCCC"
        implicitWidth: 56
        implicitHeight: 32
        radius: 16
    }
}
```

---

## Accessibility

### General Accessibility

1. **Keyboard Navigation:** All interactive elements must be keyboard accessible
2. **Screen Reader Support:** Provide meaningful labels and descriptions
3. **High Contrast Mode:** Support high-contrast colour schemes
4. **Text Scaling:** Allow font size adjustment (up to 200%)
5. **Colour Independence:** Do not rely solely on colour to convey information

### Automotive-Specific Accessibility

1. **Distraction Minimisation:** All text is glanceable within 2 seconds
2. **Safe Interaction:** All controls reachable without steering wheel removal
3. **Voice Support:** Voice commands for all critical functions
4. **Night Vision:** Dark mode with negative polarity for nighttime use
5. **Vibration Tolerance:** Large touch targets accommodating vehicle movement

### Testing Accessibility

- Test all text/icon contrast using WebAIM Contrast Checker
- Verify keyboard navigation on all screens
- Test with screen readers (e.g., NVDA, JAWS)
- Validate responsive scaling on various device sizes
- Test in simulated driving conditions (day/night, vibration)

---

## Implementation Checklist

Use this checklist when building or updating UI components:

- [ ] All text is 24dp minimum size (or 32dp for titles)
- [ ] All text is ≤120 characters (excl. spaces)
- [ ] Contrast ratio ≥4.5:1 for all text/icons
- [ ] All buttons/targets are ≥76 × 76dp
- [ ] Buttons spaced ≥23dp apart
- [ ] Response time ≤250ms for user input
- [ ] Loading states display spinner for >2s delays
- [ ] Icons map to consistent functions
- [ ] Day/night mode uses appropriate polarity
- [ ] One-handed interaction for all functions
- [ ] Voice commands for critical functions
- [ ] Driving-related info prioritised over non-driving
- [ ] No auto-scrolling or auto-playing content
- [ ] No overlapping touch targets
- [ ] All interactive elements keyboard accessible
- [ ] No games or distracting content during driving

---

## References

- [Google Design for Driving](https://developers.google.com/cars/design)
- [Interaction Principles](https://developers.google.com/cars/design/design-foundations/interaction-principles)
- [Visual Principles](https://developers.google.com/cars/design/design-foundations/visual-principles)
- [Android Accessibility Guidelines](https://developer.android.com/guide/topics/ui/accessibility)
- [Material Design 3](https://m3.material.io/)

---

**Last Updated:** 2025
**Version:** 1.0
**Status:** Approved for Implementation
