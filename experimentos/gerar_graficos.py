import os
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

plt.rcParams.update({
    "font.family": "sans-serif",
    "font.sans-serif": ["DejaVu Sans", "Arial", "Helvetica"],
    "font.size": 10,
    "axes.titlesize": 11,
    "axes.titleweight": "bold",
    "axes.labelsize": 10,
    "axes.labelweight": "medium",
    "axes.edgecolor": "#CCCCCC",
    "axes.linewidth": 0.9,
    "xtick.color": "#333333",
    "ytick.color": "#333333",
    "text.color": "#1F2937",
    "figure.facecolor": "white",
    "axes.facecolor": "#FAFAFA",
    "figure.dpi": 300
})

COR_MUTEX = "#2563EB"
COR_SPIN = "#DC2626"

def carregar_dados():
    df_win = pd.read_csv("dados/windows_bruto.csv")
    df_lin = pd.read_csv("dados/linux_bruto.csv")
    return pd.concat([df_win, df_lin], ignore_index=True)

def gerar_estatisticas(df):
    resumo = df.groupby(["so", "duracao", "primitiva", "threads"]).agg(
        repeticoes=("tempo_s", "count"),
        tempo_media=("tempo_s", "mean"),
        tempo_dp=("tempo_s", "std"),
        vazao_media=("vazao", "mean"),
        vazao_dp=("vazao", "std")
    ).reset_index()
    os.makedirs("experimentos", exist_ok=True)
    resumo.to_csv("experimentos/resumo_estatistico.csv", index=False)
    return resumo

def plot_grafico1_vazao(resumo):
    duracoes = ["curta", "media", "longa"]
    titulos_dur = {
        "curta": "Seção Crítica Curta\n(Incremento Simples)",
        "media": "Seção Crítica Média\n(~1 µs de trabalho)",
        "longa": "Seção Crítica Longa\n(~100 µs de trabalho)"
    }
    unidades = {
        "curta": ("Mops/s", 1e6),
        "media": ("kops/s", 1e3),
        "longa": ("kops/s", 1e3)
    }
    sistemas = [
        ("windows", "Windows (Win32 CRITICAL_SECTION vs. Spinlock)"),
        ("linux", "Linux (POSIX pthread_mutex vs. Spinlock)")
    ]

    for so, nome_so in sistemas:
        fig, axes = plt.subplots(1, 3, figsize=(15, 4.6))
        sub_so = resumo[resumo["so"] == so]

        for i, dur in enumerate(duracoes):
            ax = axes[i]
            sub_dur = sub_so[sub_so["duracao"] == dur]
            unidade_txt, fator = unidades[dur]

            for prim, label, marker, color, ls in [
                ("mutex", "Mutex", "o", COR_MUTEX, "-"),
                ("spinlock", "Spinlock", "s", COR_SPIN, "--")
            ]:
                dados = sub_dur[sub_dur["primitiva"] == prim].sort_values("threads")
                y = dados["vazao_media"] / fator
                yerr = dados["vazao_dp"] / fator

                ax.fill_between(dados["threads"], np.maximum(0, y - yerr), y + yerr, color=color, alpha=0.12)
                ax.errorbar(
                    dados["threads"], y, yerr=yerr,
                    label=label, marker=marker, color=color, linestyle=ls,
                    linewidth=2.2, markersize=6.5, markeredgecolor="white", markeredgewidth=1.2,
                    capsize=3.5, elinewidth=1.1, capthick=1.1
                )

            ax.set_title(titulos_dur[dur], pad=10, color="#111827")
            ax.set_xlabel("Número de Threads (N)", labelpad=6)
            ax.set_ylabel(f"Vazão Média ({unidade_txt})", labelpad=6)
            ax.set_xticks([1, 2, 4, 8, 16])
            ax.set_xlim(0.3, 16.7)
            ax.grid(True, linestyle="--", linewidth=0.6, color="#D1D5DB", alpha=0.7)
            ax.spines["top"].set_visible(False)
            ax.spines["right"].set_visible(False)
            ax.legend(frameon=True, facecolor="white", edgecolor="#E5E7EB", fontsize=9, loc="best")

        fig.suptitle(f"{nome_so} — Vazão vs. Número de Threads (Média ± DP, n = 30)",
                     fontsize=12.5, fontweight="bold", y=1.03, color="#111827")
        plt.tight_layout()
        plt.savefig(f"experimentos/grafico1_vazao_{so}.png", bbox_inches="tight")
        plt.close()

def plot_grafico2_inversao(resumo):
    fig, axes = plt.subplots(1, 2, figsize=(14, 4.7))
    cores_dur = {"curta": "#059669", "media": "#D97706", "longa": "#7C3AED"}
    nomes_dur = {"curta": "Curta (Incremento)", "media": "Média (~1 µs)", "longa": "Longa (~100 µs)"}
    marcadores = {"curta": "o", "media": "s", "longa": "^"}

    paineis = [
        ("windows", "Windows: Ponto de Inversão (Spinlock / Mutex)"),
        ("linux", "Linux: Ponto de Inversão (Spinlock / Mutex)")
    ]

    for idx, (so, titulo_so) in enumerate(paineis):
        ax = axes[idx]
        sub = resumo[resumo["so"] == so]
        piv = sub.pivot(index=["duracao", "threads"], columns="primitiva", values="vazao_media").reset_index()
        piv["razao"] = piv["spinlock"] / piv["mutex"]

        ax.axhspan(1.0, 2.9, color="#FEE2E2", alpha=0.35, zorder=0)
        ax.axhspan(0.65, 1.0, color="#DBEAFE", alpha=0.35, zorder=0)
        ax.axhline(1.0, color="#374151", linestyle="--", linewidth=1.4, zorder=1, label="Limiar de Empate (Razão = 1,0)")

        for dur in ["curta", "media", "longa"]:
            d_sub = piv[piv["duracao"] == dur].sort_values("threads")
            ax.plot(
                d_sub["threads"], d_sub["razao"],
                marker=marcadores[dur], linewidth=2.3, markersize=7,
                markeredgecolor="white", markeredgewidth=1.2,
                color=cores_dur[dur], label=nomes_dur[dur], zorder=3
            )

        ax.text(16.2, 2.72, "▲ Spinlock superior (> 1,0)", fontsize=8.5, fontweight="bold", color="#991B1B", ha="right")
        ax.text(16.2, 0.72, "▼ Mutex superior (< 1,0)", fontsize=8.5, fontweight="bold", color="#1E40AF", ha="right")

        ax.set_title(titulo_so, pad=10, color="#111827")
        ax.set_xlabel("Número de Threads (N)", labelpad=6)
        if idx == 0:
            ax.set_ylabel("Razão de Vazão (Spinlock / Mutex)", labelpad=6)
        ax.set_xticks([1, 2, 4, 8, 16])
        ax.set_xlim(0.4, 16.6)
        ax.set_ylim(0.65, 2.9)
        ax.grid(True, linestyle="--", linewidth=0.6, color="#D1D5DB", alpha=0.7)
        ax.spines["top"].set_visible(False)
        ax.spines["right"].set_visible(False)
        ax.legend(frameon=True, facecolor="white", edgecolor="#E5E7EB", fontsize=8.5, loc="upper left")

    plt.tight_layout()
    plt.savefig("experimentos/grafico2_ponto_inversao.png", bbox_inches="tight")
    plt.close()

if __name__ == "__main__":
    df = carregar_dados()
    resumo = gerar_estatisticas(df)
    plot_grafico1_vazao(resumo)
    plot_grafico2_inversao(resumo)
    print("Concluido! Graficos aprimorados salvos na pasta experimentos/.")