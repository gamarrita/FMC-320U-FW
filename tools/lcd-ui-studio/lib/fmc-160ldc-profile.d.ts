export type ProfileRect = { x: number; y: number; width: number; height: number };
export type ProfileIndicator = { x: number; y: number; label: string; kind: string };
export const FMC_160LDC_PROFILE: {
  id: string;
  moduleMm: { width: number; height: number };
  visibleAreaMm: { width: number; height: number };
  zones: { top: ProfileRect; bottom: ProfileRect; alpha: ProfileRect };
  indicators: Record<string, ProfileIndicator>;
};
export function profileRectStyle(rect: ProfileRect): Record<string, string>;
export function profileIndicatorStyle(indicator: ProfileIndicator): Record<string, string>;
