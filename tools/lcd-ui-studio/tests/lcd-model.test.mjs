import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { applyEvent, parseProject, resolveFrame, serializeProject, validateProject } from "../lib/lcd-model.mjs";
import { FMC_160LDC_PROFILE } from "../lib/fmc-160ldc-profile.mjs";

const source = await readFile(new URL("../public/projects/fmc-phase8.yaml", import.meta.url), "utf8");
const project = parseProject(source);

test("el proyecto de referencia cumple el contrato v1", () => {
  assert.equal(project.schema_version, 1);
  assert.equal(Object.keys(project.screens).length, 7);
  assert.equal(project.project.initial_screen, "all_segments");
  assert.equal(validateProject(project), true);
});

test("el YAML se puede serializar y volver a importar", () => {
  const roundTrip = parseProject(serializeProject(project));
  assert.deepEqual(roundTrip, project);
});

test("la pantalla TTL reproduce el frame aceptado en Fase 8", () => {
  const frame = resolveFrame(project, "ttl_flow_rate");
  assert.equal(frame.top, "     42.5");
  assert.equal(frame.bottom, "   100.0");
  assert.equal(frame.alpha, "Lt");
  assert.equal(frame.indicators.TTL, true);
  assert.equal(frame.indicators.RATE, true);
  assert.equal(frame.indicators.SLASH, true);
  assert.equal(frame.indicators.S, true);
});

test("la navegación DOWN recorre el menú y queda acotada al final", () => {
  assert.equal(applyEvent(project, "ttl_flow_rate", "SHORT_DOWN").screenId, "acm_flow_rate");
  assert.equal(applyEvent(project, "acm_flow_rate", "SHORT_DOWN").screenId, "print_status");
  assert.equal(applyEvent(project, "date_time", "SHORT_DOWN").screenId, "date_time");
});

test("un reset ACM es una solicitud y no una mutación de pantalla", () => {
  const result = applyEvent(project, "acm_flow_rate", "LONG_ENTER");
  assert.equal(result.screenId, "acm_flow_rate");
  assert.equal(result.request, "reset_acm_confirmed");
});

test("el validador rechaza indicadores inexistentes", () => {
  const invalid = structuredClone(project);
  invalid.screens.ttl_flow_rate.frame.indicators.UNKNOWN = true;
  assert.throws(() => validateProject(invalid), /no existe en lcd_true_source/);
});

test("el perfil físico cubre cada indicador lógico del vidrio", () => {
  const allSegments = resolveFrame(project, "all_segments");
  assert.deepEqual(
    Object.keys(FMC_160LDC_PROFILE.indicators).sort(),
    Object.keys(allSegments.indicators).sort(),
  );
  assert.deepEqual(FMC_160LDC_PROFILE.moduleMm, { width: 64, height: 33 });
  assert.deepEqual(FMC_160LDC_PROFILE.visibleAreaMm, { width: 61, height: 30 });
});
