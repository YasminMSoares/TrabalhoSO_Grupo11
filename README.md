# GBC045 — Sistemas Operacionais: Threads, Sincronização e Avaliação Multiplataforma (Tema 1)

Repositório oficial do **Grupo 11** para o projeto prático da disciplina **GBC045 — Sistemas Operacionais** 

## Integrantes (Grupo 11)
* **Gabriel Vieira Gama** — Matrícula: 12421BCC035 
* **Giovani Borges Chaban** — Matrícula: 12511BCC045 
* **Marco Thulio De Santi Roncolato** — Matrícula: 12421BCC036
* **Yasmin Moreira Soares** — Matrícula: 12421BCC046

> **Vídeo de Apresentação:** Disponível na pasta `video/`, 5–7 minutos, resolução $\ge$ 720p).

---

## 1. Objetivo e Tema Escolhido (Tema 1 — Mutex e Spinlock)

Este projeto implementa uma camada de abstração multiplataforma em C (`code/platform_thread.h` e `code/platform_thread.c`) que unifica a criação de threads e os mecanismos de sincronização utilizando **exclusivamente APIs nativas** de cada sistema operacional (sem camadas intermediárias como `winpthreads`, Cygwin ou WSL2), confrontando os subsistemas de threads e escalonadores reais do **Windows** e do **Linux**.

### Mapeamento da Camada de Abstração (`platform_thread.h`)

| Interface Unificada | Implementação Windows (Win32 API) | Implementação Linux (POSIX Threads) |
| :--- | :--- | :--- |
| **`mine_thread_t`**<br>`create_mine_thread`<br>`wait_mine_thread` | `HANDLE`<br>`CreateThread`<br>`WaitForSingleObject` + `CloseHandle` | `pthread_t`<br>`pthread_create`<br>`pthread_join` |
| **`mine_mutex_t` / `pt_mutex_t`**<br>`init_mine_mutex`<br>`lock_mine_mutex`<br>`unlock_mine_mutex`<br>`destroy_mine_mutex` | `CRITICAL_SECTION`<br>`InitializeCriticalSection`<br>`EnterCriticalSection`<br>`LeaveCriticalSection`<br>`DeleteCriticalSection` | `pthread_mutex_t`<br>`pthread_mutex_init`<br>`pthread_mutex_lock`<br>`pthread_mutex_unlock`<br>`pthread_mutex_destroy` |
| **`mine_spinlock_t` / `pt_spinlock_t`**<br>`init_mine_spinlock`<br>`lock_mine_spinlock`<br>`unlock_mine_spinlock`<br>`destroy_mine_spinlock` | `atomic_flag` (C11 `<stdatomic.h>`)<br>`atomic_flag_test_and_set` em laço com cessão voluntária via `SwitchToThread()`<br>`atomic_flag_clear` | `atomic_flag` (C11 `<stdatomic.h>`)<br>`atomic_flag_test_and_set` em laço com cessão voluntária via `sched_yield()`<br>`atomic_flag_clear` |

---

## 2. Desenho Experimental

* **Carga de Trabalho:** Cada rodada executa um montante fixo de **$16.000$ aquisições** do lock distribuídas igualmente entre $N$ threads ($16.000 / N$ aquisições por thread), incrementando um contador global protegido e executando a carga interna correspondente.
* **Variáveis Controladas:**
  * **Primitiva de sincronização:** `mutex` vs. `spinlock`.
  * **Número de threads ($N$):** `1`, `2`, `4`, `8` e `16`.
  * **Duração da seção crítica:**
    * `curta`: apenas incremento de variável compartilhada (`g_counter++`);
    * `media`: trabalho ativo calibrado em $\approx 1\,\mu\text{s}$ (`busy_wait(1e-6)`);
    * `longa`: trabalho ativo calibrado em $\approx 100\,\mu\text{s}$ (`busy_wait(100e-6)`).
* **Rigor Estatístico:** **30 repetições** independentes para cada combinação ($2 \times 5 \times 3 \times 30 = 900$ execuções por sistema operacional, totalizando $1.800$ execuções preservadas nos arquivos CSV brutos).
* **Métricas Coletadas:**
  * **Tempo total de parede ($T$, em segundos):** aferido via `QueryPerformanceCounter` no Windows e `clock_gettime(CLOCK_MONOTONIC)` no Linux.
  * **Vazão ($\lambda$, em aquisições/segundo):** calculada como $\lambda = 16.000 / T$.

---

## 3. Estrutura do Repositório

```text
TrabalhoSO_Grupo11/
├── README.md                                  # Documentação principal, metodologia e roteiro de reprodução
├── video/                                     # Apresentação técnica do grupo (5–7 min, >= 720p)
├── code/                                      # Código-fonte da camada de abstração e do benchmark
│   ├── platform_thread.h                      # Cabeçalho da API unificada (threads, mutex e spinlock)
│   ├── platform_thread.c                      # Implementação nativa Win32 (Windows) e POSIX (Linux)
│   └── main.c                                 # Programa principal de benchmark parametrizável via CLI
├── testes/                                    # Testes unitários de validação funcional
│   └── basic_tests.c                          # Verificação de ponteiros nulos, threads e integridade de locks
├── dados/                                     # Dados brutos originais sem edição manual (CSV)
│   ├── windows_bruto.csv                      # 900 execuções coletadas nativamente no Windows
│   └── linux_bruto.csv                        # 900 execuções coletadas nativamente no Linux
└── experimentos/                              # Análise estatística, especificações de hardware e gráficos
    ├── ambiente_linux.txt                     # Saída detalhada de kernel, CPU, memória e GCC no Linux
    ├── gerar_graficos.py                      # Script Python para cálculo de média/DP e geração das figuras
    ├── resumo_estatistico.csv                 # Tabela consolidada (média e desvio padrão das 30 repetições)
    ├── grafico1_vazao_windows.png             # Gráfico obrigatório 1: Vazão vs. Threads (Windows)
    ├── grafico1_vazao_linux.png               # Gráfico obrigatório 1: Vazão vs. Threads (Linux)
    └── grafico2_ponto_inversao.png            # Gráfico obrigatório 2: Ponto de inversão Spinlock vs. Mutex
```

---

## 4. Ambientes de Teste

| Especificação | Ambiente Windows (Nativo) | Ambiente Linux (Nativo) |
| :--- | :--- | :--- |
| **Sistema Operacional** | Windows 11 (x86_64) | Ubuntu 26.04.1 LTS (`Linux 7.0.0-44-generic SMP PREEMPT_DYNAMIC x86_64`) |
| **Processador (CPU)** | `[Ex: Intel Core i5 / AMD Ryzen 5 — X núcleos / Y threads]` | 8 CPUs lógicas (`0–7`, 2 threads por núcleo) |
| **Memória RAM** | `[Ex: 8 GB / 16 GB DDR4]` | 11 GiB RAM + 4,0 GiB Swap |
| **Compilador** | GCC (MSYS2 UCRT64) | `gcc (Ubuntu 15.2.0-16ubuntu1) 15.2.0` |

---

## 5. Roteiro Completo de Reprodução

### Passo 1: Compilação do Benchmark e dos Testes Unitários
A partir da raiz do repositório (`TrabalhoSO_Grupo11/`):

**No Windows (MSYS2 UCRT64 / MinGW):**
```bash
gcc -O2 -Wall code/platform_thread.c code/main.c -o benchmark_win.exe
gcc -O2 -Wall code/platform_thread.c testes/basic_tests.c -o testes_win.exe
```

**No Linux:**
```bash
gcc -O2 -Wall -pthread code/platform_thread.c code/main.c -o benchmark_linux
gcc -O2 -Wall -pthread code/platform_thread.c testes/basic_tests.c -o testes_linux
```

### Passo 2: Execução dos Testes Unitários
Valida o tratamento de erros (`NULL`), criação e espera de threads (`create_mine_thread` / `wait_mine_thread`) e a exclusão mútua sob concorrência ($100.000$ incrementos) tanto para `mine_mutex_t` quanto para `mine_spinlock_t`:

```bash
# Windows
./testes_win.exe

# Linux
./testes_linux
```
*Saída esperada:*
```text
[OK] Validation of null arguments!
[OK] Thread creation and waiting!
[OK] Operations from mutex(100000)
[OK] Operations from spinlock(100000)
All basic tests have passed!
```

### Passo 3: Execução do Benchmark Individual
O executável recebe os parâmetros `<threads> <curta|media|longa> <mutex|spinlock> [total_aquisicoes]` e imprime `<tempo_s>,<vazao>`:

```bash
# Exemplo: 4 threads, seção crítica média (~1 us), spinlock, 16.000 aquisições
./benchmark_win.exe 4 media spinlock 16000
```

Os dados completos das 30 repetições por configuração já estão preservados e prontos para análise em `dados/windows_bruto.csv` e `dados/linux_bruto.csv`.

### Passo 4: Geração das Estatísticas e dos Gráficos
Certifique-se de possuir Python 3 com `pandas`, `numpy` e `matplotlib` instalados e execute a partir da raiz do repositório:

```bash
python experimentos/gerar_graficos.py
```
O script processa os 1.800 registros brutos de `dados/` e gera:
* `experimentos/resumo_estatistico.csv` (média e desvio padrão de tempo e vazão);
* `experimentos/grafico1_vazao_windows.png` e `experimentos/grafico1_vazao_linux.png` (Vazão vs. $N$);
* `experimentos/grafico2_ponto_inversao.png` (Razão $\text{Vazão}_{\text{Spinlock}} / \text{Vazão}_{\text{Mutex}}$ destacando os pontos de cruzamento).

---

## 6. Síntese dos Resultados Experimentais

* **Windows (`CRITICAL_SECTION` vs. `mine_spinlock_t`):**
  * **Seção curta:** Apresenta duas inversões. Sem contenção ($N=1$), `CRITICAL_SECTION` vence ($52{,}02\text{ Mops/s}$ vs. $42{,}25\text{ Mops/s}$). Entre $N=2$ e $N=8$, o Spinlock assume a dianteira (atingindo $11{,}32\text{ Mops/s}$ vs. $7{,}84\text{ Mops/s}$ em $N=8$). Em $N=16$ (*oversubscription*), ocorre nova inversão a favor do `CRITICAL_SECTION` ($15{,}37\text{ Mops/s}$ vs. $12{,}85\text{ Mops/s}$).
  * **Seção média ($\approx 1\,\mu\text{s}$):** O ponto de inversão ocorre **entre $N=2$ e $N=4$**. Até $N=2$, o Spinlock é superior ($926{,}3\text{ kops/s}$ vs. $858{,}9\text{ kops/s}$); a partir de $N=4$, o `CRITICAL_SECTION` domina até $N=16$.
* **Linux (`pthread_mutex_t` vs. `mine_spinlock_t`):**
  * **Seção curta e média:** Em $N=1$, ambas as primitivas empatam ($\approx 25{,}4\text{ Mops/s}$ na curta; $\approx 900\text{ kops/s}$ na média). Para qualquer grau de concorrência ($N \ge 2$), o Spinlock com `sched_yield()` supera o `pthread_mutex_t` em todas as configurações ($25{,}90\text{ Mops/s}$ vs. $11{,}39\text{ Mops/s}$ em $N=2$ curta; $891{,}8\text{ kops/s}$ vs. $661{,}7\text{ kops/s}$ em $N=2$ média), evitando o custo de bloqueio/desbloqueio via chamadas `futex` ao kernel.
* **Seção Crítica Longa ($\approx 100\,\mu\text{s}$):** Em ambos os sistemas operacionais, o tempo total permanece limitado pelo piso físico sequencial de $16.000 \times 100\,\mu\text{s} \approx 1{,}60\text{ s}$ ($\approx 9{,}9\text{ kops/s}$), independentemente do número de threads.
