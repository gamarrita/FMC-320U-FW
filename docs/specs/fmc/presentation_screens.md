# FMC presentation screens

Status: working specification for `fmc_presentation.*`

## 1. Purpose and scope

This document normalizes the user-visible FMC screen behavior needed to design a pure semantic presentation layer.

`fmc_presentation.*` shall:

- consume FMC-domain values and presentation policy inputs;
- select semantic screen content, row roles, legends, unit cues, and decimal policy;
- produce an adapter-neutral presentation description;
- remain testable without LCD hardware.

`fmc_presentation.*` shall not:

- call `FM_LCD_*`;
- map characters or indicators to LCD RAM/segments;
- own screen navigation, key handling, long-press detection, timers, RTOS state, persistence, authorization, pulse capture, alarms, printing, Bluetooth, or RTC updates;
- duplicate physical conversion policy already owned by `fmc_units.*`;
- calculate rate from pulse/time windows, which remains owned by `fmc_rate.*`.

Repository constraints used:

- `fmc_presentation.*` owns mode identity, row roles, legends, unit cues, and decimal policy.
- The shared product view uses the top row for total-oriented data and the bottom row for rate-oriented data.
- The LCD foundation exposes an 8-position top row, a 7-position bottom row, a 2-character alphanumeric field, and standalone indicators.
- `BBL_US` is a model unit; `BR` is a presentation label.
- `EQUIV_M3` is a model unit; `MC` is a presentation label.

## 2. Source traceability convention

Word references use `Word p.N` and refer to `100 - Casos de uso - revisión V(1).docx`.

Repository references use repository-relative paths.

Requirement status:

- **Firm**: directly stated by the Word and consistent with the repository.
- **Inference**: derived from the drawings, LCD capabilities, or multiple consistent statements.
- **Open**: ambiguous or contradictory and requires human confirmation.

## 3. First-slice screen catalog

| Screen ID | Class | First-slice status | Purpose | Word source |
|---|---|---:|---|---|
| `FMC_PRESENTATION_SCREEN_TTL_RATE` | operation | required | Historical total plus instantaneous rate | pp. 10-12 |
| `FMC_PRESENTATION_SCREEN_ACM_RATE` | operation | required | User-resettable accumulated total plus instantaneous rate | pp. 13-14 |
| `FMC_PRESENTATION_SCREEN_ALL_SEGMENTS` | startup/test | optional contract extension | LCD segment test | p. 7 |
| `FMC_PRESENTATION_SCREEN_VERSION` | startup | optional contract extension | Firmware version | p. 8 |
| `FMC_PRESENTATION_SCREEN_DATE_TIME` | information | future | Date and time | p. 19 |
| `FMC_PRESENTATION_SCREEN_PRINTER` | workflow | future | Print value and progress/result | p. 16 |
| `FMC_PRESENTATION_SCREEN_ACM_PT100` | operation, not implemented | future | Temperature plus uncompensated batch control value | p. 15 |
| `FMC_PRESENTATION_SCREEN_BLUETOOTH` | workflow, not implemented | future | Bluetooth connection/download countdown | p. 18 |
| Configuration screens | configuration | future information only | Presentation of editable values | pp. 21-31 |

The smallest useful implementation slice is `TTL_RATE` plus `ACM_RATE`, with shared total/rate formatting helpers and unit-label policy.

## 4. Operation screens

### 4.1 TTL + RATE

**Purpose:** show the non-user-resettable historical total and the instantaneous rate.

| Element | Normalized requirement | Status | Traceability |
|---|---|---|---|
| Top row role | TTL visible value | Firm | Word p. 10 |
| Top row width | Up to 8 numeric positions, including any decimal-point placement | Firm | Word p. 10; `fm_lcd_layout.h` |
| Top legend | `TTL` on | Firm | Word p. 10 drawing |
| Top-row unit text | No volume label adjacent to TTL | Firm | Word p. 10 |
| Bottom row role | RATE visible value | Firm | Word p. 10 |
| Bottom legend | `RATE` on | Firm | Word p. 10 drawing |
| Alpha/unit field | Active volume label, typically two characters | Firm/inference | Word p. 10 drawing; LCD alpha pair |
| Rate separator | `/` on | Firm | Word p. 10 drawing |
| Time-base indicator | Exactly one of `S`, `M`, `H`, `D` on | Firm | Word pp. 10, 26; LCD layout |
| Pulse witness | Standalone point blinks when valid primary-sensor pulses arrive | Firm | Word pp. 10-11, 20 |
| Total decimals | 0, 1, 2, or 3 fractional digits | Firm | Word p. 10 |
| Rate decimals | 0, 1, 2, or 3 fractional digits are discussed; exact ownership/configuration is not fully consistent | Open | Word pp. 10, 26 |

Examples explicitly shown or tested:

| Semantic values | Expected visible concept | Source |
|---|---|---|
| TTL `73.56`, RATE `0.00`, volume `L`, time base minute | top `73.56` + `TTL`; bottom `0.00`; alpha `LT`/`L?`; `/` + `M`; `RATE` | Word p. 10 drawing |
| RATE `1.0` L/s | bottom `1.0`, volume label, `/`, `S`, `RATE` | Word p. 11 test |
| RATE `100.0` L/s | bottom `100.0`, volume label, `/`, `S`, `RATE` | Word p. 11 test |
| RATE `1.0` L/min | bottom `1.0`, volume label, `/`, `M`, `RATE` | Word pp. 11-12 test |
| RATE `100.0` L/min | bottom `100.0`, volume label, `/`, `M`, `RATE` | Word pp. 11-12 test |

TTL numeric ranges stated by the Word:

| Fractional digits | Stated range |
|---:|---|
| 0 | `0` to `9999999` |
| 1 | `0.0` to `9999999.9` |
| 2 | `0.00` to `999999.99` |
| 3 | `0.000` to `99999.999` |

**Open inconsistency:** an 8-position row cannot display `9999999.9` as eight numeric characters plus a decimal point unless the point is not counted as a position, which is consistent with the LCD API. The Word's first range also appears to contain seven digits despite describing an eight-digit field. The presentation contract should specify field capacity in LCD-visible digits rather than reproduce those textual maxima as arithmetic limits.

### 4.2 ACM + RATE

**Purpose:** show the user-resettable accumulated total and the same instantaneous rate presentation used by the TTL screen.

| Element | Normalized requirement | Status | Traceability |
|---|---|---|---|
| Top row role | ACM visible value | Firm | Word p. 13 |
| Top legend | `ACM` on | Firm | Word p. 13 drawing |
| Bottom row role | RATE visible value | Firm | Word p. 13 |
| Bottom legend | `RATE` on | Firm | Word p. 13 drawing |
| Volume/time cues | Same policy as TTL + RATE | Firm | Word p. 13 |
| Decimal policies | Same total and rate decimal policies as TTL + RATE | Firm | Word p. 13 |
| Pulse witness | Same behavior as TTL + RATE | Firm | Word p. 13 |

Example explicitly shown:

- ACM `0.00` on the top row with `ACM` on.
- RATE `0.00` on the bottom row with `RATE`, volume label, `/`, and `M` on.

The reset action does not belong to `fmc_presentation.*`. The presentation input merely contains the current derived ACM value.

### 4.3 Visual difference between TTL and ACM

The two operation screens are not simultaneous. The Word defines separate screens and navigation between them.

The semantic visual difference is limited to:

- selected total value: TTL versus ACM;
- selected total legend: `TTL` versus `ACM`;
- total reset policy belongs outside presentation.

RATE, active unit cues, time-base cues, decimal policy, pulse witness, and alarm overlays are shared.

## 5. Units and visible labels

ACM, TTL, and RATE share one active volume unit in the current model.

| Model enum | Domain meaning | Recommended visible alpha label | Status | Notes |
|---|---|---|---|---|
| `FMC_MODEL_VOLUME_UNIT_L` | litre | `LT` | Firm from Word | The Word consistently names litres as `LT`; the user request mentions `L`. Confirm whether the product label must remain `LT` or be shortened to `L` plus blank. |
| `FMC_MODEL_VOLUME_UNIT_M3` | cubic metre | `M3` | Firm | Two-character alpha field can represent `M3`. |
| `FMC_MODEL_VOLUME_UNIT_GAL_US` | US gallon | `GL` | Firm from Word | Model name is explicitly US gallon; visible legacy label is `GL`. |
| `FMC_MODEL_VOLUME_UNIT_BBL_US` | US barrel | `BR` | Firm from repo context and Word text | Word also notes a conflicting observed `BL`; treat `BL` as unresolved legacy evidence, not the preferred contract. |
| `FMC_MODEL_VOLUME_UNIT_KG` | calibrated 1:1 kilogram mode | `KG` | Firm | No density conversion is implied by presentation. |
| `FMC_MODEL_VOLUME_UNIT_EQUIV_M3` | equivalent cubic metre, 1:1 special mode | `MC` | Firm from repo context and Word | Not the same semantic unit as physical `M3`. |
| `FMC_MODEL_VOLUME_UNIT_CUSTOM` | custom calibrated unit without firmware-defined label | `--` | Firm from model comments/inventory | Alternative is blank alpha field; repository currently says presentation may render `--`. |

### 5.1 Unit-label policy

Recommended first-slice rule:

1. Normalize the model unit through existing unit policy before presentation.
2. Map the normalized model enum to a presentation-level two-character label.
3. Do not return LCD segment information.
4. Keep label identity separate from whether the adapter displays it in the alpha pair.

Suggested semantic enum:

```text
FMC_PRESENTATION_VOLUME_LABEL_NONE
FMC_PRESENTATION_VOLUME_LABEL_LT
FMC_PRESENTATION_VOLUME_LABEL_M3
FMC_PRESENTATION_VOLUME_LABEL_GL
FMC_PRESENTATION_VOLUME_LABEL_BR
FMC_PRESENTATION_VOLUME_LABEL_KG
FMC_PRESENTATION_VOLUME_LABEL_MC
FMC_PRESENTATION_VOLUME_LABEL_CUSTOM
```

The output may additionally carry canonical text (`"LT"`, `"M3"`, etc.) for tests and non-LCD consumers. The enum should remain the normative identity.

## 6. RATE presentation

### 6.1 Semantic composition

RATE is presented as:

```text
<numeric rate> <volume label> / <time-base cue>
```

On the current LCD this decomposes into:

- bottom numeric row: numeric rate;
- `RATE` indicator: on;
- alpha pair: volume label;
- `/` indicator: on;
- exactly one time-base indicator: on.

### 6.2 Time bases

| Model time base | Visible cue | Other time cues off |
|---|---|---|
| `FMC_MODEL_TIME_BASE_SECOND` | `S` | `M`, `H`, `D` |
| `FMC_MODEL_TIME_BASE_MINUTE` | `M` | `S`, `H`, `D` |
| `FMC_MODEL_TIME_BASE_HOUR` | `H` | `S`, `M`, `D` |
| `FMC_MODEL_TIME_BASE_DAY` | `D` | `S`, `M`, `H` |

The presentation layer selects the semantic cue. The LCD adapter maps it to `FM_LCD_LAYOUT_INDICATOR_S/M/H/D`.

### 6.3 Rate decimal policy

The Word supports visible resolutions of 0, 1, 2, and 3 fractional digits, and examples show 1 and 3 fractional digits.

Recommended contract:

```text
fractional_digits: integer in [0, 3]
```

The caller provides the active rate decimal policy until ownership of operator configuration is finalized. `fmc_presentation.*` validates/normalizes it and formats the value consistently.

**Open:** Word p. 10 says the resolution is changed from the operation screen. Word p. 26 describes changing it in a configuration screen, then contains a correction saying it is not done there. This affects navigation/configuration ownership, not the semantic presentation shape.

## 7. Totals presentation

### 7.1 Shared policy

TTL and ACM:

- are derived from pulse-backed total state plus active measurement configuration;
- share the active volume unit;
- share one total decimal policy in the legacy behavior;
- use the 8-position top numeric row;
- do not show the volume alpha label adjacent to the top row on the operation screens;
- select exactly one total legend according to screen mode.

### 7.2 TTL

- meaning: historical total;
- reset policy: privileged/non-user flow, outside presentation;
- indicator: `TTL` on;
- `ACM_TOP` and `ACM_BOTTOM` off unless a later screen explicitly requires them.

### 7.3 ACM

- meaning: accumulated/user-resettable total;
- reset action: outside presentation;
- operation-screen indicator: the ACM indicator associated with the top total field.

**Open:** `fm_lcd_layout.h` defines both `ACM_TOP` and `ACM_BOTTOM`. The operation-screen drawing places ACM near the top value, so `ACM_TOP` is the expected adapter mapping. The precise purpose of `ACM_BOTTOM` should be confirmed before freezing a presentation-to-LCD mapping.

## 8. Numeric formatting policy

### 8.1 Presentation responsibility

`fmc_presentation.*` should decide:

- requested fractional digit count;
- deterministic rounding policy;
- overflow/invalid representation semantic state;
- whether a row is blank, numeric, or error-like;
- right alignment as a semantic recommendation for numeric rows.

The LCD adapter should decide:

- exact `FM_LCD_WriteText()` calls;
- clipping behavior mandated by the LCD API;
- row and alpha alignment API arguments;
- decimal segment activation from formatted text;
- unsupported-character handling.

### 8.2 Recommended semantic numeric output

Use a structured value rather than only an arbitrary string:

```text
value
fractional_digits
valid
negative_allowed
format_status: OK | OVERFLOW | INVALID
```

A convenience formatted string may also be returned for deterministic tests, with capacities sized for:

- top: 8 digits plus optional decimal point and terminator;
- bottom: 7 digits plus optional decimal point and terminator.

### 8.3 Overflow

The Word does not define overflow behavior. Do not silently invent wrap, saturation, scientific notation, or dashes.

First-slice recommendation:

- return `FMC_PRESENTATION_FORMAT_OVERFLOW`;
- leave the adapter-visible fallback unresolved until a human decision is made.

## 9. Indicators and overlays

### 9.1 Operation-screen base indicators

| Indicator | TTL + RATE | ACM + RATE |
|---|---:|---:|
| `TTL` | on | off |
| `ACM_TOP` | off | on |
| `RATE` | on | on |
| `/` | on | on |
| one of `S/M/H/D` | on | on |
| pulse point | runtime overlay | runtime overlay |
| battery | runtime overlay | runtime overlay |
| `E` | runtime overlay | runtime overlay |

### 9.2 Pulse witness

The Word requires the standalone point to blink while valid primary-sensor pulses are being received.

Recommended ownership split:

- runtime/acquisition provides `pulse_activity` or a separately timed witness state;
- presentation exposes whether the point is semantically enabled/visible for the current frame;
- blink timing/phase remains outside `fmc_presentation.*`;
- LCD adapter or blink-policy layer applies the point indicator state.

### 9.3 Alarm overlays

The Word describes future/not-implemented behavior:

- `E` blinks for high rate, temperature limits, or unexpected reset;
- low-battery icon blinks below a threshold;
- an erroneous displayed rate/temperature blinks with `E`.

These are not required for the initial total/rate formatting slice. Preserve extension points in the input/output contract without implementing alarm policy.

## 10. Startup and information screens

These screens are useful presentation cases but are not required to implement the initial total/rate contract.

### 10.1 All-segments screen

- all numeric, alpha, decimal, icon, legend, and indicator segments on;
- duration and automatic transition are runtime/navigation concerns;
- backlight behavior is outside presentation.

This mode may be represented as a dedicated semantic command `all_segments_on` rather than constructing normal fields.

### 10.2 Firmware version

- top row: blank;
- bottom row: firmware version;
- alpha pair or suffix: `VE` according to the drawing;
- example: `0100.13 VE` shown on Word p. 8;
- duration and transition are outside presentation.

**Open:** the prose says version followed by `VE` in the lower row, while the LCD has a separate alpha pair. The drawing suggests `VE` is in the alpha pair. Confirm the intended field ownership.

### 10.3 Date/time

Future presentation information:

- top row: date `dd.mm.yyyy`;
- bottom row: time `hh.mm.ss`;
- Word example: `29.08.2007` and `9.30.00`;
- Bluetooth enabling, three-minute timeout, and navigation do not belong to presentation.

**Open:** an eight-position top numeric row cannot display ten visible date digits (`dd.mm.yyyy`) if the year is four digits. The drawing appears to use `29.08.2007`, which requires eight digits plus two decimal points and therefore fits because decimal points are separate segments. This is valid on the current glass.

### 10.4 Printer

Future workflow presentation:

- top row: ACM value to be printed;
- `ACM` legend on;
- lower field: progress code `03`, `02`, `01`, success `--`, or error `E1`;
- alpha pair: `PR`;
- print initiation and state machine do not belong to presentation.

### 10.5 ACM + PT100

Future/not-implemented presentation:

- top row: uncompensated batch control value;
- `BATCH` indicator on;
- bottom row: PT100 temperature;
- alpha field/segments show degree Celsius;
- temperature precision depends on selected range: 0.1 C in ambient range and 1 C in extended range.

Do not include in the first `fmc_presentation.*` slice unless temperature semantics are separately defined.

### 10.6 Bluetooth window

Future/not-implemented presentation:

- bottom numeric row: countdown value;
- alpha pair: `BT`;
- connection/download state machine and countdown ownership remain outside presentation.

## 11. Configuration screens: presentation-relevant future information

The configuration menu itself is out of scope. The following visual facts may later reuse presentation primitives:

| Screen | Presentation-only information |
|---|---|
| Password | top `PASS`; bottom four entry positions; entered/selected state may blink |
| Factor K | top editable numeric factor; bottom/alpha `K`; selected digit blinks |
| Linearization Kx/Fx | top factor; bottom frequency; labels `F1`..`F5` and `HZ`; selected field/digit blinks |
| Volume unit | visible shared TTL/ACM decimal sample; `TTL` and `ACM` legends; selected volume label |
| Rate time unit | bottom rate sample; `RATE`; selected time cue; selected decimal point/unit may blink |
| Date/time | date and time; selected field blinks |
| High-rate alarm | rate threshold using active unit/time cues |
| K_OUT | top value; alpha `KO` |
| Span | top value; rate unit/time cues |
| Expansion coefficient | top coefficient; bottom temperature with degree Celsius |

Editing cursor, key processing, validation, authorization, persistence, and menu order are explicitly outside `fmc_presentation.*`.

## 12. Suggested firmware contract

The following is a design proposal, not C implementation.

### 12.1 Screen selection

```text
fmc_presentation_screen_t
  TTL_RATE
  ACM_RATE
  ALL_SEGMENTS          // optional extension
  VERSION               // optional extension
  DATE_TIME             // future
  PRINTER               // future
  ACM_PT100              // future
  BLUETOOTH              // future
```

The requested screen should be explicit. `fmc_presentation.*` should not infer navigation state.

### 12.2 Input snapshot

Recommended first-slice input:

```text
fmc_presentation_input_t
  screen
  measurement
    active_volume_unit
    active_time_base
  values
    ttl_volume
    acm_volume
    rate
  precision
    total_fractional_digits
    rate_fractional_digits
  overlays
    pulse_witness_on
    low_battery_on       // extension point
    error_on             // extension point
    total_value_blink    // extension point
    rate_value_blink     // extension point
```

Notes:

- The input should receive already-derived visible volumes and rate, or a dedicated snapshot containing them.
- Presentation should not recalculate rate from pulse windows.
- If the implementation instead receives `fmc_model_t`, total-volume derivation must use existing unit helpers and should not duplicate conversion tables.
- `pulse_witness_on` is a frame-level semantic state; timer ownership remains external.

### 12.3 Output frame

Recommended output:

```text
fmc_presentation_frame_t
  mode
  top
    role: NONE | TTL | ACM | VALUE | DATE
    numeric_value
    fractional_digits
    format_status
    alignment: RIGHT
  bottom
    role: NONE | RATE | VALUE | TIME
    numeric_value
    fractional_digits
    format_status
    alignment: RIGHT
  alpha
    label_id
    text[3]              // optional convenience representation
  legends
    ttl
    acm_top
    acm_bottom
    rate
    batch
    power
  unit_cues
    slash
    second
    minute
    hour
    day
  overlays
    point
    battery
    error
  blink_semantics
    top_value
    bottom_value
    alpha
    selected_indicator_mask
```

The output must not expose:

- `fm_lcd_layout_indicator_t`;
- `fm_lcd_layout_row_t`;
- LCD RAM addresses, COM/SEG identities, bit masks, or driver status codes.

### 12.4 Necessary enums

Minimum recommended public enums:

- `fmc_presentation_screen_t`;
- `fmc_presentation_row_role_t`;
- `fmc_presentation_volume_label_t`;
- `fmc_presentation_format_status_t`;
- optionally `fmc_presentation_alignment_t` if alignment is considered semantic rather than adapter policy.

The existing `fmc_model_volume_unit_t` and `fmc_model_time_base_t` should be reused as inputs, not duplicated.

### 12.5 Decisions owned by `fmc_presentation.*`

- screen semantic identity;
- which domain value occupies each row;
- total legend selection (`TTL` or `ACM`);
- `RATE` legend selection;
- active volume label identity;
- slash and time-base cue selection;
- decimal-count validation and numeric formatting policy;
- semantic blank/valid/overflow/invalid states;
- which values/fields are eligible to blink, when such status is supplied by the caller.

### 12.6 Decisions left to the LCD adapter

- clear/write ordering;
- calls to `FM_LCD_Clear`, `FM_LCD_WriteText`, `FM_LCD_WriteAlpha`, and `FM_LCD_SetIndicator`;
- mapping semantic legends to `FM_LCD_LAYOUT_INDICATOR_*`;
- choosing `ACM_TOP` versus any other physical ACM legend after mapping is confirmed;
- translating formatted text into decimal segments;
- logical blink range construction and application;
- flush/dirty handling;
- hardware error propagation.

## 13. Recommended first implementation and tests

Implement only:

1. volume-unit to presentation-label mapping;
2. time-base to one-of-four cue mapping;
3. total numeric formatting for 0..3 fractional digits and top-row capacity;
4. rate numeric formatting for 0..3 fractional digits and bottom-row capacity;
5. TTL + RATE frame composition;
6. ACM + RATE frame composition;
7. validation tests for all units, all time bases, all decimal settings, invalid enums, and overflow.

Representative tests:

- TTL screen selects TTL, not ACM.
- ACM screen selects ACM, not TTL.
- Both screens select RATE, slash, active volume label, and exactly one time cue.
- `BBL_US -> BR`.
- `EQUIV_M3 -> MC`.
- `CUSTOM -> --` or the finally approved custom-label policy.
- second/minute/hour/day activate only `S/M/H/D`, respectively.
- totals use top capacity; rate uses bottom capacity.
- decimal policy is deterministic at 0, 1, 2, and 3 digits.
- invalid volume enum follows existing normalization policy and presents litres.
- overflow is reported, not silently clipped by presentation.

## 14. Decisions firm enough to implement

- The first semantic operation modes are separate `TTL_RATE` and `ACM_RATE` screens.
- Top row is total-oriented; bottom row is rate-oriented.
- TTL and ACM share the active volume unit and total decimal policy.
- RATE uses the same active volume unit plus one time base.
- RATE composition requires `RATE`, volume label, `/`, and exactly one of `S/M/H/D`.
- `BBL_US` displays as `BR`.
- `EQUIV_M3` displays as `MC`.
- `CUSTOM` is an operational model unit without a firmware-defined physical name; repository guidance allows `--`.
- The module is pure and does not call the LCD, own navigation, persistence, RTOS, or acquisition.
- LCD-specific enums and segment mapping remain in the adapter.

## 15. Explicit inferences

1. **Inference:** numeric rows should be right-aligned because every operation-screen drawing uses right-aligned values and the LCD API exposes right alignment.
2. **Inference:** the two-character alpha field is the intended location for `LT`, `M3`, `GL`, `BR`, `KG`, `MC`, and `--`.
3. **Inference:** the operation ACM screen should map to the physical `ACM_TOP` legend because the ACM legend is drawn adjacent to the top total.
4. **Inference:** decimal-point glyphs do not consume numeric character positions; this follows the LCD row model and explains how date and decimal examples fit.
5. **Inference:** explicit screen selection is preferable to inferred mode because navigation is outside the module boundary.
6. **Inference:** presentation should report overflow rather than rely on LCD clipping, because silent clipping would destroy numeric meaning.

## 16. Open questions for human confirmation

1. Is the litre label definitively `LT`, or should the new presentation contract use `L` with the second alpha position blank?
2. Confirm that US gallons must display as `GL`, not `GAL` or another label.
3. Confirm that US barrels must display as `BR`; should the observed `BL` be treated as a legacy bug?
4. For `CUSTOM`, should the visible label be `--`, two blanks, or a configurable two-character label supplied by a higher layer?
5. Does ACM on the operation screen map to the physical `ACM_TOP` indicator? What product use is intended for `ACM_BOTTOM`?
6. Are total and rate decimal settings always independently configurable, with totals shared between TTL and ACM?
7. What exact rounding rule is required: nearest, truncate, or legacy fixed-point behavior?
8. What should the operator see on numeric overflow: all `9`, dashes, blank, an error code, or another convention?
9. What should the operator see for NaN/invalid/unavailable rate or total values?
10. Should negative values ever be supported on total/rate rows, or only on future temperature screens?
11. For the version screen, is `VE` written in the alpha pair or as part of the lower numeric row representation?
12. Is the pulse point merely on/off per presentation frame, or must `fmc_presentation.*` expose a blink request distinct from blink phase?
13. Should low-battery and `E` alarm overlays be included in the first presentation contract as passive input flags, despite alarm policy remaining future work?
14. Is rate resolution changed only from the operation screen in the current product behavior, only from configuration, or both?
15. Are startup and version screens part of the first `fmc_presentation.*` implementation, or should the initial module contain only TTL/ACM + RATE?

## 17. Summary of detected screens

Detected in the Word:

- 2 primary operation screens: TTL + RATE, ACM + RATE;
- 2 startup screens: all segments, firmware version;
- 3 additional user/information/workflow screens: printer, date/time, optional PT100;
- 1 optional Bluetooth screen;
- password, calibration, unit, time-base, date/time, alarm, output, span, and compensation configuration screens.

For the current workstream, only TTL + RATE and ACM + RATE are sufficiently central and bounded to define the first implementation slice. The remaining screens are retained as future presentation information without importing their navigation or workflow behavior.
