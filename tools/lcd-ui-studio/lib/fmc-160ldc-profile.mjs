/**
 * Normalized visible geometry for the FMC-160LDC-EX glass.
 *
 * Coordinates are percentages of the 61 x 30 mm visible area documented by
 * FMC-160LDC-EX.pdf and visually calibrated against the powered all-segments
 * and TTL/RATE hardware photographs. Electrical COM/SEG routing deliberately
 * remains in the repository LCD technical authority and is not duplicated
 * here.
 */
export const FMC_160LDC_PROFILE = Object.freeze({
  id: "fmc_segmented_lcd_v1",
  moduleMm: { width: 64, height: 33 },
  visibleAreaMm: { width: 61, height: 30 },
  zones: {
    top: { x: 4.5, y: 4, width: 92, height: 37 },
    bottom: { x: 3.5, y: 68, width: 61, height: 28 },
    alpha: { x: 68, y: 68, width: 15, height: 28 },
  },
  indicators: {
    POINT: { x: 0.8, y: 4.5, label: "", kind: "point" },
    BATTERY: { x: 3, y: 54, label: "+ −", kind: "battery" },
    POWER: { x: 17, y: 57, label: "POWER", kind: "text" },
    E: { x: 52, y: 45, label: "E", kind: "text" },
    BATCH: { x: 60, y: 45, label: "BATCH", kind: "text" },
    TTL: { x: 77, y: 45, label: "TTL", kind: "text" },
    ACM_TOP: { x: 87, y: 45, label: "ACM", kind: "text" },
    RATE: { x: 49, y: 57, label: "RATE", kind: "text" },
    ACM_BOTTOM: { x: 69, y: 57, label: "ACM", kind: "text" },
    SLASH: { x: 85.5, y: 68, label: "", kind: "slash" },
    H: { x: 89, y: 69, label: "H", kind: "unit" },
    D: { x: 94, y: 69, label: "D", kind: "unit" },
    S: { x: 89, y: 83, label: "S", kind: "unit" },
    M: { x: 94, y: 83, label: "M", kind: "unit" },
  },
});

export function profileRectStyle(rect) {
  return {
    left: `${rect.x}%`,
    top: `${rect.y}%`,
    width: `${rect.width}%`,
    height: `${rect.height}%`,
  };
}

export function profileIndicatorStyle(indicator) {
  return { left: `${indicator.x}%`, top: `${indicator.y}%` };
}
