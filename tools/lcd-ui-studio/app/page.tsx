"use client";

import { ChangeEvent, CSSProperties, useEffect, useMemo, useRef, useState } from "react";
import { applyEvent, INDICATOR_IDS, parseProject, resolveFrame, serializeProject } from "@/lib/lcd-model.mjs";
import { FMC_160LDC_PROFILE, profileIndicatorStyle, profileRectStyle } from "@/lib/fmc-160ldc-profile.mjs";

type Variable = { label: string; type: "number" | "boolean" | "text"; value: number | boolean | string; decimals?: number; unit?: string };
type Screen = { label: string; description?: string; frame: Record<string, unknown>; interactions?: Record<string, string> };
type Project = {
  schema_version: number;
  project: { id: string; name: string; device: string; initial_screen: string; description?: string };
  variables?: Record<string, Variable>;
  screens: Record<string, Screen>;
};
type Frame = {
  id: string; label: string; description: string; allSegments: boolean; top: string; bottom: string; alpha: string;
  indicators: Record<string, boolean>; overflow: Record<string, boolean>;
};

const DIGIT_SEGMENTS: Record<string, string> = {
  "0": "abcdef", "1": "bc", "2": "abdeg", "3": "abcdg", "4": "bcfg",
  "5": "acdfg", "6": "acdefg", "7": "abc", "8": "abcdefg", "9": "abcdfg",
  A: "abcefg", B: "cdefg", C: "adef", D: "bcdeg", E: "adefg", F: "aefg",
  L: "def", O: "abcdef", P: "abefg", R: "eg", T: "defg", "-": "g", " ": "",
};
const ALPHA_SEGMENTS: Record<string, string[]> = {
  "0": ["a", "b", "c", "d", "e", "f", "h", "m"],
  "8": ["a", "b", "c", "d", "e", "f", "g1", "g2", "h", "m"],
  B: ["a", "b", "c", "d", "g1", "g2", "i", "l"], D: ["a", "b", "c", "d", "i", "l"],
  L: ["d", "e", "f"], P: ["a", "b", "e", "f", "g1", "g2"],
  R: ["a", "b", "e", "f", "g1", "g2", "l"], T: ["a", "i", "l"],
  O: ["a", "b", "c", "d", "e", "f"], F: ["a", "e", "f", "g1", "g2"],
  t: ["a", "i", "l"], "-": ["g1", "g2"], " ": [],
};
const ALPHA_PARTS = ["a", "b", "c", "d", "e", "f", "g1", "g2", "h", "i", "j", "k", "l", "m"];
const BUTTON_LABELS: Record<string, string> = { ESC: "ESC", UP: "↑", DOWN: "↓", ENTER: "↵", EXT1: "EXT 1", EXT2: "EXT 2" };

function splitCells(text: string, count: number) {
  const cells: { char: string; point: boolean }[] = [];
  for (const char of text) {
    if (char === "." && cells.length) cells[cells.length - 1].point = true;
    else cells.push({ char, point: false });
  }
  while (cells.length < count) cells.push({ char: " ", point: false });
  return cells.slice(0, count);
}

function SevenSegmentRow({ text, count, compact = false }: { text: string; count: number; compact?: boolean }) {
  return <div className={`segment-row ${compact ? "segment-row--compact" : ""}`} aria-label={text.trim() || "vacío"}>
    {splitCells(text, count).map((cell, index) => {
      const active = DIGIT_SEGMENTS[cell.char.toUpperCase()] ?? "";
      return <span className="digit" key={`${index}-${cell.char}-${cell.point}`}>
        {["a", "b", "c", "d", "e", "f", "g"].map((part) => <i key={part} className={`seg seg-${part} ${active.includes(part) ? "is-on" : ""}`} />)}
        <i className={`decimal ${cell.point ? "is-on" : ""}`} />
      </span>;
    })}
  </div>;
}

function AlphaRow({ text, allSegments = false }: { text: string; allSegments?: boolean }) {
  return <div className="alpha-row" aria-label={text.trim() || "vacío"}>
    {text.padEnd(2).slice(0, 2).split("").map((char, index) => {
      const active = allSegments ? ALPHA_PARTS : ALPHA_SEGMENTS[char] ?? ALPHA_SEGMENTS[char.toUpperCase()] ?? [];
      return <span className="alpha-cell" key={`${index}-${char}`}>
        {ALPHA_PARTS.map((part) => <i key={part} className={`alpha-seg alpha-${part} ${active.includes(part) ? "is-on" : ""}`} />)}
      </span>;
    })}
  </div>;
}

function Indicator({ id, active }: { id: string; active: boolean }) {
  const spec = FMC_160LDC_PROFILE.indicators[id];
  return <span
    className={`indicator indicator--${spec.kind} ${active ? "is-on" : ""}`}
    style={profileIndicatorStyle(spec) as CSSProperties}
    aria-label={id}
  >{spec.label}</span>;
}

function LcdGlass({ frame, miniature = false }: { frame: Frame; miniature?: boolean }) {
  return <div className={`lcd-glass ${miniature ? "lcd-glass--mini" : ""}`}>
    <div className="lcd-grain" />
    <div className="lcd-top-row" style={profileRectStyle(FMC_160LDC_PROFILE.zones.top) as CSSProperties}><SevenSegmentRow text={frame.top} count={8} compact={miniature} /></div>
    <div className="lcd-bottom-row" style={profileRectStyle(FMC_160LDC_PROFILE.zones.bottom) as CSSProperties}><SevenSegmentRow text={frame.bottom} count={7} compact={miniature} /></div>
    <div className="lcd-alpha" style={profileRectStyle(FMC_160LDC_PROFILE.zones.alpha) as CSSProperties}><AlphaRow text={frame.alpha} allSegments={frame.allSegments} /></div>
    {INDICATOR_IDS.map((id: string) => <Indicator key={id} id={id} active={frame.indicators[id]} />)}
  </div>;
}

function Instrument({ frame, onPress, onExternal }: { frame: Frame; onPress: (key: string) => void; onExternal: (key: string) => void }) {
  return <div className="instrument-rig">
    <div className="instrument-shell">
      {['top-left', 'top-right', 'bottom-left', 'bottom-right'].map((position) => <i className={`mount mount--${position}`} key={position} />)}
      <i className="access-hole access-hole--left" /><i className="access-hole access-hole--right" />
      <div className="bezel"><LcdGlass frame={frame} /></div>
      <div className="keypad" aria-label="Botones frontales del instrumento">
        {["ESC", "ENTER", "DOWN", "UP"].map((key) => <button className={`device-key device-key--${key.toLowerCase()}`} key={key} onClick={() => onPress(key)} aria-label={key}>
          <span>{BUTTON_LABELS[key]}</span>
        </button>)}
      </div>
    </div>
    <div className="external-controls" aria-label="Pulsadores externos">
      <span>ENTRADAS<br />EXTERNAS</span>
      {["EXT1", "EXT2"].map((key) => <button key={key} onClick={() => onExternal(key)}><i /><strong>{BUTTON_LABELS[key]}</strong><small>pulsación corta</small></button>)}
    </div>
  </div>;
}

export default function Home() {
  const [project, setProject] = useState<Project | null>(null);
  const [yamlSource, setYamlSource] = useState("");
  const [screenId, setScreenId] = useState("");
  const [variableValues, setVariableValues] = useState<Record<string, unknown>>({});
  const [pressMode, setPressMode] = useState("SHORT");
  const [status, setStatus] = useState("Cargando proyecto de referencia…");
  const [error, setError] = useState("");
  const [catalogOpen, setCatalogOpen] = useState(false);
  const [sourceOpen, setSourceOpen] = useState(false);
  const [eventLog, setEventLog] = useState<string[]>([]);
  const importRef = useRef<HTMLInputElement>(null);

  const loadSource = (source: string, message: string) => {
    try {
      const parsed = parseProject(source) as Project;
      setProject(parsed);
      setYamlSource(serializeProject(parsed));
      setScreenId(parsed.project.initial_screen);
      setVariableValues(Object.fromEntries(Object.entries(parsed.variables ?? {}).map(([id, item]) => [id, item.value])));
      setEventLog([]); setError(""); setStatus(message);
    } catch (reason) {
      setError(reason instanceof Error ? reason.message : "No se pudo interpretar el proyecto.");
    }
  };

  useEffect(() => {
    fetch("/projects/fmc-phase8.yaml").then((response) => response.text())
      .then((source) => loadSource(source, "Proyecto de referencia cargado"))
      .catch((reason) => setError(String(reason)));
  }, []);

  const resetProject = () => {
    fetch("/projects/fmc-phase8.yaml").then((response) => response.text())
      .then((source) => loadSource(source, "Proyecto restablecido a la referencia de Fase 8"))
      .catch((reason) => setError(String(reason)));
  };

  const frame = useMemo(() => project && screenId ? resolveFrame(project, screenId, variableValues) as Frame : null, [project, screenId, variableValues]);

  const dispatchInput = (event: string) => {
    if (!project) return;
    const result = applyEvent(project, screenId, event);
    setScreenId(result.screenId); setStatus(`${event} · ${result.outcome}`);
    setEventLog((items) => [`${event} → ${result.outcome}`, ...items].slice(0, 5));
  };

  const pressButton = (key: string) => dispatchInput(`${pressMode}_${key}`);
  const pressExternal = (key: string) => dispatchInput(`SHORT_${key}`);

  const importFile = (event: ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0];
    if (!file) return;
    file.text().then((source) => loadSource(source, `Importado: ${file.name}`));
    event.target.value = "";
  };

  const exportYaml = () => {
    if (!project) return;
    const blob = new Blob([serializeProject(project)], { type: "application/yaml" });
    const url = URL.createObjectURL(blob);
    const anchor = document.createElement("a");
    anchor.href = url; anchor.download = `${project.project.id}.yaml`; anchor.click();
    URL.revokeObjectURL(url); setStatus("Proyecto YAML exportado");
  };

  if (!project || !frame) return <main className="loading-state"><span className="eyebrow">LCD UI STUDIO</span><h1>{error || status}</h1></main>;

  return <main className="studio-shell">
    <header className="topbar">
      <div className="title-lockup"><span className="mark">FM</span><div><p>LCD UI STUDIO</p><h1>{project.project.name}</h1></div></div>
      <div className="toolbar">
        <button onClick={() => setCatalogOpen(true)}>Catálogo</button><button onClick={() => setSourceOpen((open) => !open)}>YAML</button>
        <button onClick={resetProject}>Restablecer</button>
        <button onClick={() => importRef.current?.click()}>Importar</button><button className="primary-action" onClick={exportYaml}>Exportar</button>
        <input ref={importRef} type="file" accept=".yaml,.yml,text/yaml" hidden onChange={importFile} />
      </div>
    </header>

    <section className="workspace">
      <aside className="screen-rail">
        <div className="rail-heading"><span>PANTALLAS</span><strong>{Object.keys(project.screens).length}</strong></div>
        <nav>{Object.entries(project.screens).map(([id, screen], index) => <button className={screenId === id ? "active" : ""} key={id} onClick={() => { setScreenId(id); setStatus(`Vista directa · ${screen.label}`); }}>
          <small>{String(index + 1).padStart(2, "0")}</small><span>{screen.label}</span>
        </button>)}</nav>
        <div className="authority-note"><span>AUTORIDAD FÍSICA</span><p>lcd_true_source.yaml</p><small>8 + 7 dígitos · 2 alfanuméricos · 14 indicadores</small></div>
      </aside>

      <section className="stage">
        <div className="stage-heading">
          <div><span className="eyebrow">SIMULADOR INTERACTIVO</span><h2>{frame.label}</h2><p>{frame.description}</p></div>
          <div className="mode-switch" aria-label="Duración de pulsación">{["SHORT", "LONG"].map((mode) => <button className={pressMode === mode ? "active" : ""} key={mode} onClick={() => setPressMode(mode)}>{mode === "SHORT" ? "CORTA" : "LARGA"}</button>)}</div>
        </div>
        <div className="instrument-stage"><Instrument frame={frame} onPress={pressButton} onExternal={pressExternal} /></div>
        <div className="status-strip"><span className="status-dot" /><strong>ÚLTIMO EVENTO</strong><p>{status}</p><code>{screenId}</code></div>
      </section>

      <aside className="inspector">
        <section><div className="section-label"><span>DATOS SIMULADOS</span><small>No son configuración del instrumento</small></div>
          <div className="variable-list">{Object.entries(project.variables ?? {}).map(([id, variable]) => <label key={id}>
            <span>{variable.label}<small>{id}{variable.unit ? ` · ${variable.unit}` : ""}</small></span>
            {variable.type === "boolean" ? <input type="checkbox" checked={Boolean(variableValues[id])} onChange={(event) => setVariableValues({ ...variableValues, [id]: event.target.checked })} />
              : <input type={variable.type === "number" ? "number" : "text"} step={variable.decimals ? 1 / 10 ** variable.decimals : undefined} value={String(variableValues[id] ?? "")} onChange={(event) => setVariableValues({ ...variableValues, [id]: variable.type === "number" ? Number(event.target.value) : event.target.value })} />}
          </label>)}</div>
        </section>
        <section><div className="section-label"><span>TRAZA DE BOTONES</span><small>Eventos semánticos</small></div>
          <ol className="event-log">{eventLog.length ? eventLog.map((item, index) => <li key={`${item}-${index}`}>{item}</li>) : <li className="muted">Usá los botones del instrumento.</li>}</ol>
        </section>
      </aside>
    </section>

    {sourceOpen && <aside className="source-panel">
      <div><span className="eyebrow">CONTRATO PORTABLE</span><h2>Proyecto YAML</h2><button onClick={() => setSourceOpen(false)}>Cerrar</button></div>
      <textarea spellCheck={false} value={yamlSource} onChange={(event) => setYamlSource(event.target.value)} />
      {error && <pre className="validation-error">{error}</pre>}
      <button className="apply-button" onClick={() => loadSource(yamlSource, "YAML validado y aplicado")}>Validar y aplicar</button>
    </aside>}

    {catalogOpen && <div className="modal-backdrop" onMouseDown={() => setCatalogOpen(false)}><section className="catalog" onMouseDown={(event) => event.stopPropagation()}>
      <header><div><span className="eyebrow">REVISIÓN HUMANO–AGENTE</span><h2>Catálogo completo de pantallas</h2></div><button onClick={() => setCatalogOpen(false)}>Cerrar</button></header>
      <div className="catalog-grid">{Object.keys(project.screens).map((id) => {
        const itemFrame = resolveFrame(project, id, variableValues) as Frame;
        return <button key={id} onClick={() => { setScreenId(id); setCatalogOpen(false); }}><LcdGlass frame={itemFrame} miniature /><strong>{itemFrame.label}</strong><code>{id}</code></button>;
      })}</div>
    </section></div>}
  </main>;
}
