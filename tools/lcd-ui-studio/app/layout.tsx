import type { Metadata } from "next";
import "./globals.css";

export const metadata: Metadata = {
  title: "FMC LCD UI Studio",
  description: "Simulador y contrato visual del LCD segmentado FMC-320U",
};

export default function RootLayout({ children }: Readonly<{ children: React.ReactNode }>) {
  return <html lang="es"><body>{children}</body></html>;
}
