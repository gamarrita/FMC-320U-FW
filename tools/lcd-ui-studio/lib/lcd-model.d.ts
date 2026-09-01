export const INDICATOR_IDS: string[];
export const BUTTONS: string[];
export function parseProject(source: string): unknown;
export function serializeProject(project: unknown): string;
export function validateProject(project: unknown): boolean;
export function formatField(spec: Record<string, unknown>, variables: Record<string, unknown>, fallbackWidth: number): { text: string; overflow: boolean };
export function resolveFrame(project: unknown, screenId: string, variableOverrides?: Record<string, unknown>): unknown;
export function applyEvent(project: unknown, screenId: string, event: string): { screenId: string; request: string | null; outcome: string };
