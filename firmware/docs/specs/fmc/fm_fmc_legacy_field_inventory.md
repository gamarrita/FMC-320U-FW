# FMC Legacy Field Inventory

Purpose:
- working table to classify the legacy FMC header fields before freezing the new
  model contract
- this is not the new contract
- this is not the legacy source of truth
- this is a review artifact and may be freely edited, annotated, or overwritten

Source under review:
- [legacy_backup/libs/fm_fmc.h](/d:/githubs/FMC-320U-FW/firmware/legacy_backup/libs/fm_fmc.h)

Suggested classification vocabulary:
- `canonical config`
- `active environment config`
- `runtime-derived`
- `runtime backing state`
- `derived helper`
- `acquisition/runtime support`
- `status/quality`
- `presentation`
- `lateral/out-of-core`
- `unclear`

Suggested destination vocabulary:
- `fmc model`
- `fmc presentation`
- `lcd adapter`
- `acquisition layer`
- `workflow/reporting`
- `persistence`
- `drop from first slice`
- `unclear`

## Short Definitions

These definitions are intentionally short.
They exist to reduce interpretation drift during review.

| Term | Short definition |
|---|---|
| `canonical` | The preferred source of truth from which other values may be derived. |
| `config` | A value selected, programmed, or stored to govern later behavior. |
| `active environment config` | The currently active configuration used by runtime calculations and presentation. |
| `runtime` | State that changes while the instrument is operating. |
| `runtime-derived` | A runtime value computed from other state, not directly configured by the user. |
| `runtime backing state` | Runtime state kept because other visible values may need to be recomputed from it. |
| `derived helper` | A supporting value derived from more canonical state to simplify or speed up calculations. |
| `semantic` | Meaningful in domain terms, independent from UI or low-level implementation details. |
| `public state` | State that other layers are expected to observe through a module contract. |
| `presentation` | Anything whose main purpose is formatting, labeling, display policy, or visible rendering. |
| `presentation leakage` | A display or UI concern that accidentally appears inside a deeper domain layer. |
| `acquisition` | The part of the system that captures pulses, time intervals, or raw measurement inputs. |
| `status/quality` | State that describes confidence, alarm, acknowledgement, or health rather than the measured quantity itself. |
| `lateral/out-of-core` | Valid product data that exists, but does not belong to the minimum FMC core model. |
| `first slice` | The smallest useful first implementation boundary for this refactor. |
| `environment` | The active FMC state/context currently governing measurement behavior in the running product. |
| `editable configuration` | Configuration the operator or setup flow is allowed to modify before applying it to the environment. |
| `privileged reset` | A reset operation that belongs to admin/service/factory flow, not normal user interaction. |
| `copyable value type` | A struct intentionally shaped so it can be duplicated safely as plain state. |

## Legacy Types

### `ufp3_t`

| Item | Current classification | Why | Should survive refactor | Suggested destination | Open questions |
|---|---|---|---|---|---|
| `ufp3_t` | implementation detail | fixed-point storage strategy, not domain meaning | no as public semantic type | internal implementation only | keep fixed-point internally or move to another deterministic numeric type? |

dhs: este tipo de datos esta bien interpretado, pero me gusataria este separado en una libreria matematica, que declare el tipo de datos, que sepa hacer operaciones, luego otra libreria deberia saber imprimir estos tipos del datos en el lcd custom que tenemos en esta proyecto



### `sensors_list_t`

| Item | Current classification | Why | Should survive refactor | Suggested destination | Open questions |
|---|---|---|---|---|---|
| `FM_FACTORY_RAM_BACKUP` | persistence/bootstrap source | source used during init/bootstrap, not core FMC meaning | not in first slice | persistence/bootstrap layer | should this become part of a later config-loading boundary? |
| `FM_FACTORY_LAST_SETUP` | persistence/bootstrap source | same as above | not in first slice | persistence/bootstrap layer | same |
| `FM_FACTORY_SENSOR_0` | persistence/bootstrap source | factory preset selector | not in first slice | persistence/bootstrap layer | same |
| `FM_FACTORY_AI_25` | persistence/bootstrap source | factory preset selector | not in first slice | persistence/bootstrap layer | same |
| `FM_FACTORY_AI_80` | persistence/bootstrap source | factory preset selector | not in first slice | persistence/bootstrap layer | same |

La idea que entiendes esta bien. La extiendo, se trata de un conjutno inicial de valores de congiruracion tipicos, ejemplo una turbina de diamentro nominal 25 generalmente tiene un factor k y se la usa en litro y minuto, una de diametro nominal 80 tendra otro factor K y se la usa en metros cubicos hora. Los  valores de factor k que ser cargan son tipicos. La utilidad de lo anterior es que luego de flasher el micro, se tengan un conjunto de varlores usables, sin tener que configurar manualmente el instrumento, so lo usa para debug, testing manual.
Los valores FM_FACTORY_RAM_BACKUP y FM_FACTORY_LAST_SETUP, no recuedo ahora para que los usaba, pero estimo que tiene que ver con la ultima configuracion valida que se hizo manual, y la otra puede ser la de run time. De  todas maneras la forma esto era para mi forma de implementar, hay que repensarlo, si esto es lo mejor para lograr tener en flash un backup de lo que estaba en ram y lo ultimo configurado, que no esta clara su utilidad.


### `fm_fmc_fp_sel`

| Item | Current classification | Why | Should survive refactor | Suggested destination | Open questions |
|---|---|---|---|---|---|
| `FM_FMC_FP_SEL_0..3` | numeric policy shared by math and presentation | legacy fixed-point selection affects both visible decimal placement and pulse-to-volume arithmetic strategy | yes, but not in `fmc model` core | numeric library candidate, with presentation consuming the rendered meaning later | keep legacy "everything behaves like 3 decimals, presentation changes" simplification, or replace it with a clearer numeric contract? |

Reviewer clarification:
- this should not remain inside `fm_fmc_model`
- it is closer to a future numeric library boundary than to pure FMC semantics
- it also should not be owned by the LCD adapter directly
- legacy used one simplification:
  - treat values mathematically as if they always carried three decimals
  - then change presentation separately
- that simplification worked and kept the old computer stable
- but the refactor should re-evaluate it in favor of code clarity, not keep it
  by inertia

### `fm_fmc_vol_unit_t`

| Item | Current classification | Why | Should survive refactor | Suggested destination | Open questions |
|---|---|---|---|---|---|



| `VOL_UNIT_BLANK` | unsupported/custom unit placeholder | legacy placeholder intended to represent units not supported by the physical conversion table; conversion factor is `1`, and the loaded calibration must already be pulses per desired custom unit; presentation may render it as `--` | yes, renamed conceptually | fmc model placeholder | none |
| `VOL_UNIT_BR` | active environment config | product unit for US barrels; the model name should be `BBL_US` while the visible string remains `BR` in presentation | yes | fmc model | none |
| `VOL_UNIT_GL` | active environment config | normal physical unit supported through real conversion from liters in the legacy table | yes | fmc model | none at this stage |
| `VOL_UNIT_KG` | active environment config with intentional hack semantics | product supports KG as a selectable unit, but without density handling; legacy resolves it by calibrating through the liters-based process and then keeping a 1:1 operative conversion path | yes for the first slice, as a supported unit carried by the current hack | `fmc model` | keep as hack for now; re-evaluate later if explicit calibration-unit support is introduced |
| `VOL_UNIT_LT` | active environment config and normal calibration default | physical unit offered to the user like any other; the refactor should make the calibration-unit anchor explicit, with liters as the normal physical-conversion default and custom/unsupported units using a loaded factor for that unit | yes | fmc model | none for first slice |
| `VOL_UNIT_M3` | active environment config | normal physical unit supported through real conversion from liters in the legacy table | yes | fmc model | none at this stage |
| `VOL_UNIT_ME` | active environment config with intentional hack semantics | product-specific unit that enters the legacy table through the liters path and then uses conversion value `1`, like the current hack cases | yes for the first slice if the product still exposes it | fmc model | keep as current hack for now; re-evaluate later if explicit calibration-unit or unit-policy support is introduced |
| `VOL_UNIT_END` | enum sentinel | implementation helper only | no as semantic item | internal only | none |



### `fm_fmc_vol_data_t`

| Item | Current classification | Why | Should survive refactor | Suggested destination | Open questions |
|---|---|---|---|---|---|
| `unit_convert` | derived/helper metadata that should be split later | legacy mixed real physical conversions with hack/special-case entries in one table; the refactor direction is to separate physical unit conversion from special calibrated-unit handling | maybe, but not as one mixed public struct | internal unit-conversion / unit-policy support | split into physical unit conversion support plus special calibrated-unit handling |
| `name[3]` | presentation | short LCD label | yes, but not in model | fmc presentation | alpha-unit mapping likely belongs to presentation, not model |

### `fm_fmc_time_unit_t`

| Item | Current classification | Why | Should survive refactor | Suggested destination | Open questions |
|---|---|---|---|---|---|
| `TIME_UNIT_SECOND` | active environment config | active rate time base | yes | fmc model | public naming style in new API? |
| `TIME_UNIT_MINUTE` | active environment config | active rate time base | yes | fmc model | same |
| `TIME_UNIT_HOUR` | active environment config | active rate time base | yes | fmc model | same |
| `TIME_UNIT_DAY` | active environment config | active rate time base | yes | fmc model | same |
| `TIME_UNIT_END` | enum sentinel | implementation helper only | no as semantic item | internal only | none |

## Legacy Struct Fields

### `fm_fmc_rate_t`

| Item | Current classification | Why | Should survive refactor | Suggested destination | Open questions |
|---|---|---|---|---|---|
| `factor_r` | derived helper | active rate conversion helper from K + time base | maybe as helper, not as primary public state | helper/query in model or acquisition layer | should active derived factor become explicit in model environment? |
| `delta_t` | acquisition/runtime support | recent capture interval used to compute rate | not in first slice core contract | acquisition layer | does model need to know capture details at all? |
| `delta_p` | acquisition/runtime support | pulse delta used to compute rate | not in first slice core contract | acquisition layer | same |
| `rate` | runtime-derived | displayed/consumed flow value produced by acquisition/rate behavior | yes as FMC semantic, but not stored in the first core model | derived runtime view or later acquisition/rate seam | define the rate view when the acquisition/rate slice starts |
| `rate_pf_sel` | presentation | visible decimal selection for rate | yes, but not in core model | fmc presentation | keep as presentation policy only? |
| `limit_high` | status/quality or config | alarm threshold, not minimum FMC model | probably later, not first slice | alarm/config layer | is this part of core FMC or alarm manager? |
| `limit_low` | status/quality or config | alarm threshold, not minimum FMC model | probably later, not first slice | alarm/config layer | same |
| `filter` | acquisition/config | signal-processing config, not core first slice | probably later | acquisition layer | does first refactor need any filter semantics? |
| `ack` | status/quality | flow acknowledgement/status | maybe later | status/quality layer | what exact meaning does `ack` have in product behavior? |

### `fm_fmc_totalizer_t`

| Item | Current classification | Why | Should survive refactor | Suggested destination | Open questions |
|---|---|---|---|---|---|
| `acm` | runtime-derived visible value over canonical pulses | accumulated total displayed to operator; visible volume should be derived from `pulse_acm` and active measurement config | yes as semantic role, not cached canonical state | fmc model query / presentation view | none |
| `ttl` | runtime-derived visible value over canonical pulses | historical/privileged-reset total displayed to operator; visible volume should be derived from `pulse_ttl` and active measurement config | yes as semantic role, not cached canonical state | fmc model query / presentation view | reset policy now clarified as privileged, not user-level |
| `vol_pf_sel` | presentation | decimal placement for ACM/TTL display | yes, but not in core model | fmc presentation | should ACM/TTL share one precision policy or be separable later? |
| `pulse_acm` | runtime backing state | backing pulses for ACM recomputation | yes | fmc model | none |
| `pulse_ttl` | runtime backing state | backing pulses for TTL recomputation | yes | fmc model | none |
| `factor_cal` | strong calibration input with explicit calibration unit | the deployed legacy scheme keeps one practical liter-anchored path for known conversions, while unsupported/custom units use factor `1` and require a calibration already expressed in the desired unit; no UI/menu support for changing calibration unit is part of the first slice | yes | fmc model | none for first slice |
| `factor_k` | derived helper / operative factor view with low-power purpose | precomputes `factor_cal * conversion_value` so runtime arithmetic is cheaper in an ultra-low-power embedded target; for conversion factor `1`, `factor_k == factor_cal` | yes as query, not stored public truth | helper/query in model | none for first slice |
| `vol_unit` | active environment config | active shared volume unit; unsupported/custom selections should use the model placeholder and conversion factor `1` | yes | fmc model | none |
| `time_unit` | active environment config | active rate time base | yes | fmc model | none |
| `rate` | mixed type, needs split | bundles runtime-derived rate with acquisition, thresholds, and presentation | partially | split across model, presentation, acquisition, status | this is the main legacy knot to open |
| `ticket_number` | lateral/out-of-core | reporting workflow state, not FMC core measurement meaning | probably no in first slice | workflow/reporting or persistence | should it leave FMC entirely? |

## Preliminary Cleanup Targets

### Keep in first-slice `fmc model`

| Item | Reason |
|---|---|
| `acm` | first-class total role |
| `ttl` | first-class total role |
| `rate.value` | first-class flow role |
| `pulse_acm` | backing state for total recomputation |
| `pulse_ttl` | backing state for total recomputation |
| `factor_cal` | canonical calibration source |
| `vol_unit` | active environment config |
| `time_unit` | active environment config |
| reset policy by total role | domain semantic |

### Keep as derived query/helper, not equal-rank public truth

| Item | Reason |
|---|---|
| `factor_k` | derived from canonical calibration + active unit |
| `factor_r` | derived from active K + time base |

### Move out of core model

| Item | Reason | Destination |
|---|---|---|
| `vol_pf_sel` | presentation only | fmc presentation |
| `rate_pf_sel` | presentation only | fmc presentation |
| `name[3]` in unit metadata | presentation only | fmc presentation |
| `ticket_number` | workflow/reporting | workflow/reporting |

### Defer beyond first slice

| Item | Reason | Destination |
|---|---|---|
| `delta_t` | acquisition detail | acquisition layer |
| `delta_p` | acquisition detail | acquisition layer |
| `filter` | signal processing config | acquisition layer |
| `limit_high` | alarm/config concern | alarm/config layer |
| `limit_low` | alarm/config concern | alarm/config layer |
| `ack` | status/quality concern | status/quality layer |

## Reviewer Notes

- This file is intentionally writable and provisional.
- Prefer editing classifications here before freezing changes into
  `libs/fm_fmc_model.h`.
- If an item seems to belong to more than one category, that is usually a sign
  that the old module mixed responsibilities and the item should be split.
