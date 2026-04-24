UI buttons TODO / ideas
=======================

1) Make button availability stateful
- Keep a per-button enabled/disabled state tied to plugin state (runway loaded, radar transmitting, etc).
- Draw disabled buttons dimmed and ignore click handlers while disabled.

2) Range controls left/right buttons
- Wire "<" and ">" to cycle through [1, 3, 5, 10, 15, 20] NM.
- Clamp at first/last value and keep the selected range button highlighted.

3) Display toggles
- Add bool state for Wx/Obs/Map/WHI/Hist/Bird Areas.
- Clicking should toggle state and immediately RequestRefresh().
- Drawing code should read those booleans to show/hide layers.

4) Radar controls behavior
- "Radiate" should gate target drawing/interpolation updates.
- "Maint Mode" should disable operational controls and show a clear status cue.
- "Rain Mode" should adjust clutter/smoothing profile (future hook).

5) Runway selection UX
- Preserve selected runway across refreshes and ASR load/save.
- Reject empty/inop runway slots on click and provide a message.

6) Input safety and consistency
- Keep all button index parsing bounded (already started).
- Centralize button lookup/dispatch to remove duplicated switch logic.

