import { parse, stringify } from "yaml";

export const INDICATOR_IDS = [
  "POINT",
  "BATTERY",
  "POWER",
  "RATE",
  "E",
  "BATCH",
  "ACM_TOP",
  "TTL",
  "SLASH",
  "ACM_BOTTOM",
  "H",
  "D",
  "S",
  "M",
];

export const BUTTONS = ["ESC", "UP", "DOWN", "ENTER", "EXT1", "EXT2"];

const FIELD_WIDTHS = { top: 8, bottom: 7, alpha: 2 };

export function parseProject(source) {
  const project = parse(source);
  validateProject(project);
  return project;
}

export function serializeProject(project) {
  validateProject(project);
  return stringify(project, { indent: 2, lineWidth: 100 });
}

export function validateProject(project) {
  const errors = [];

  if (!project || typeof project !== "object") errors.push("El documento YAML debe ser un objeto.");
  if (project?.schema_version !== 1) errors.push("schema_version debe ser 1.");
  if (!project?.project?.id) errors.push("project.id es obligatorio.");
  if (!project?.project?.name) errors.push("project.name es obligatorio.");
  if (!project?.project?.initial_screen) errors.push("project.initial_screen es obligatorio.");
  if (!project?.screens || typeof project.screens !== "object") errors.push("screens es obligatorio.");

  const screenIds = Object.keys(project?.screens ?? {});
  if (screenIds.length === 0) errors.push("El proyecto debe declarar al menos una pantalla.");
  if (project?.project?.initial_screen && !screenIds.includes(project.project.initial_screen)) {
    errors.push(`La pantalla inicial '${project.project.initial_screen}' no existe.`);
  }

  for (const [screenId, screen] of Object.entries(project?.screens ?? {})) {
    if (!screen.label) errors.push(`screens.${screenId}.label es obligatorio.`);
    if (!screen.frame || typeof screen.frame !== "object") {
      errors.push(`screens.${screenId}.frame es obligatorio.`);
      continue;
    }

    for (const [field, width] of Object.entries(FIELD_WIDTHS)) {
      const spec = screen.frame[field];
      if (!spec || typeof spec !== "object") {
        errors.push(`screens.${screenId}.frame.${field} es obligatorio.`);
        continue;
      }
      if (spec.variable && !project.variables?.[spec.variable]) {
        errors.push(`La variable '${spec.variable}' usada por ${screenId}.${field} no existe.`);
      }
      if (spec.width !== undefined && spec.width !== width) {
        errors.push(`${screenId}.${field}.width debe ser ${width} para este vidrio.`);
      }
    }

    for (const indicatorId of Object.keys(screen.frame.indicators ?? {})) {
      if (!INDICATOR_IDS.includes(indicatorId)) {
        errors.push(`El indicador '${indicatorId}' de ${screenId} no existe en lcd_true_source.yaml.`);
      }
    }

    for (const [event, target] of Object.entries(screen.interactions ?? {})) {
      if (!/^(SHORT|LONG)_(ESC|UP|DOWN|ENTER|EXT1|EXT2)$/.test(event)) {
        errors.push(`El evento '${event}' de ${screenId} no es un evento semántico válido.`);
      }
      if (target !== "stay" && !String(target).startsWith("request:") && !screenIds.includes(target)) {
        errors.push(`El destino '${target}' de ${screenId}.${event} no existe.`);
      }
    }
  }

  if (errors.length) throw new Error(errors.join("\n"));
  return true;
}

function cellLength(value) {
  return [...value].filter((char) => char !== ".").length;
}

export function formatField(spec, variables, fallbackWidth) {
  const width = spec.width ?? fallbackWidth;
  let raw = spec.literal ?? variables?.[spec.variable]?.value ?? "";

  if (typeof raw === "number") {
    const decimals = spec.decimals ?? variables?.[spec.variable]?.decimals;
    raw = Number.isInteger(decimals) ? raw.toFixed(decimals) : String(raw);
  } else if (typeof raw === "boolean") {
    raw = raw ? "1" : "0";
  } else {
    raw = String(raw);
  }

  const usedCells = cellLength(raw);
  if (usedCells > width) {
    return { text: "-".repeat(width), overflow: true };
  }

  const padding = " ".repeat(width - usedCells);
  const align = spec.align ?? "right";
  return {
    text: align === "left" ? `${raw}${padding}` : `${padding}${raw}`,
    overflow: false,
  };
}

export function resolveFrame(project, screenId, variableOverrides = {}) {
  const screen = project.screens[screenId];
  if (!screen) throw new Error(`La pantalla '${screenId}' no existe.`);

  const variables = Object.fromEntries(
    Object.entries(project.variables ?? {}).map(([id, variable]) => [
      id,
      { ...variable, value: variableOverrides[id] ?? variable.value },
    ]),
  );

  const allSegments = Boolean(screen.frame.all_segments);
  const top = formatField(screen.frame.top, variables, FIELD_WIDTHS.top);
  const bottom = formatField(screen.frame.bottom, variables, FIELD_WIDTHS.bottom);
  const alpha = formatField(screen.frame.alpha, variables, FIELD_WIDTHS.alpha);
  const indicators = Object.fromEntries(
    INDICATOR_IDS.map((id) => {
      const source = screen.frame.indicators?.[id] ?? false;
      const value = typeof source === "string" ? variables[source]?.value : source;
      return [id, allSegments || Boolean(value)];
    }),
  );

  return {
    id: screenId,
    label: screen.label,
    description: screen.description ?? "",
    allSegments,
    top: allSegments ? "8.8.8.8.8.8.8.8" : top.text,
    bottom: allSegments ? "8.8.8.8.8.8.8" : bottom.text,
    alpha: allSegments ? "88" : alpha.text,
    indicators,
    overflow: { top: top.overflow, bottom: bottom.overflow, alpha: alpha.overflow },
  };
}

export function applyEvent(project, screenId, event) {
  const target = project.screens[screenId]?.interactions?.[event] ?? "stay";
  if (target === "stay") return { screenId, request: null, outcome: "Sin cambio" };
  if (String(target).startsWith("request:")) {
    return {
      screenId,
      request: String(target).slice("request:".length),
      outcome: `Solicitud: ${String(target).slice("request:".length)}`,
    };
  }
  return { screenId: target, request: null, outcome: `Navegó a ${project.screens[target].label}` };
}
