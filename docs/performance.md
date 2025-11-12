# C/Tcl/Tk vs Go/ebiten

## Status

We have 2 clients.

- C/Tcl/Tk 
- Go/ebiten

We compare the use in energy over some time,
to determine which is more efficient.
There are 3 outcomes.

1) If the difference in energy use is higher than 20%,
development on the inefficient client is to be immediately dropped!  

2) If the difference is higher than 10%,
the inefficient client cannot define the language of the backend (Go vs C).
The inefficient client may see delayed, less, or even no development.  

3) If the difference is lower than 10%, both clients may be developed concurrently,
with ebiten being for mobile, and tk for desktop.  

However, if the ebiten client is not subject to case 1,
the backend very likely will be rewritten in Go.  

## Thesis

The theory is that the software rendering Tk will have a slight performance
penalty. Slight because we are talking about simple 2D graphics.
The Go backend should probably be slower though,
so the final outcome is not sure.  

## 1st test method

By measuring the energy consumption of a "Igel M340C",
with "AMD GX-424CC" as CPU, and "Radeon R5E Graphics" as GPU.
The hardware will be powered by Debian GNU/Linux Trixie, and the Mate desktop.
I would also reboot the machine after setting up the test envirnoment,
and each test.  

The machine will be started, then a terminal.
From there I cd into the relevant directory, and make run.  
The clients will both run for half an hour.  
The ebiten client will be from tag v0.6,
and tk will be from commit 36240751ab2e9e9d82bce945422920f498b4e65f.  

If the default tickrate of 120 always 100% poweruse, it may need to be lowered,
otherwise 100% is always 100% and it just lags.  
I must pay attention to ebiten client's different default simrate.
The `stdWldTRateFrac` must be set from 0.25 to 0.2, which would align with tk's
`SIM_SUBSAMPLE` of 5, thus both simulate on every 5th tick.
Tk needs to be changed to have the same world scale from 10 to 4,
and the size needs to go up 112x120 

## 1st test result

With just the mate terminal and mate system monitor running,
the hardware consistently used 6.4W of power, rarely going up to 6.5W.  
At max CPU, while compiling the ebiten client for example,
16.9W of power were needed.

Energy-use:
ebiten = 0.005 kWh
Tk     = 0.007 kWh

Well these are small values.
Maybe these are in some margin of error right?

## 2nd test method

Same hardware.
Just look at the power use for 10 seconds each time, and note down the highest.

## 2nd test results

Power-use max:
ebiten = 10.9 W
Tk     = 14.4 W

A 32% increase. I didn't think it would be that bad.

# unify dot updates into world.Tick

> This causes a reduction of individual loops, thus reducing None checks,
> theoretically improving performance.

old = 2d8ad3fc0bfcdb04b0a2f7019b9f807c48c76354
new = ee4e46212cfb5c8a7d02690ed71db8a5de69514f

## Test 1

- world: W 1000, H 1000
- ticks: 100
- 2 large brushes of oxygen and hydrogen

### old

```
File: mat.test
Type: cpu
Time: Jan 3, 2025 at 12:40pm (CET)
Duration: 4.82s, Total samples = 4.73s (98.19%)
Entering interactive mode (type "help" for commands, "o" for options)
(pprof) top20
Showing nodes accounting for 4.63s, 97.89% of 4.73s total
Dropped 29 nodes (cum <= 0.02s)
      flat  flat%   sum%        cum   cum%
     1.29s 27.27% 27.27%      1.29s 27.27%  github.com/SchokiCoder/hawps/mat.(*World).swapDots
     0.94s 19.87% 47.15%      0.97s 20.51%  github.com/SchokiCoder/hawps/mat.(*World).applyThermalConduction.func1
     0.68s 14.38% 61.52%      4.69s 99.15%  github.com/SchokiCoder/hawps/mat.(*World).Tick
     0.51s 10.78% 72.30%      0.51s 10.78%  github.com/SchokiCoder/hawps/mat.(*World).applyChemReactions.func1
     0.31s  6.55% 78.86%      0.31s  6.55%  runtime.duffcopy
     0.28s  5.92% 84.78%      2.05s 43.34%  github.com/SchokiCoder/hawps/mat.(*World).applyGravity.func1
     0.12s  2.54% 87.32%      0.12s  2.54%  github.com/SchokiCoder/hawps/mat.World.CanDisplace (inline)
     0.09s  1.90% 89.22%      0.09s  1.90%  github.com/SchokiCoder/hawps/mat.MeltPs (inline)
     0.07s  1.48% 90.70%      2.12s 44.82%  github.com/SchokiCoder/hawps/mat.(*World).applyGravity
     0.07s  1.48% 92.18%      0.07s  1.48%  github.com/SchokiCoder/hawps/mat.(*World).runSpawners (inline)
     0.06s  1.27% 93.45%      1.03s 21.78%  github.com/SchokiCoder/hawps/mat.(*World).applyThermalConduction
     0.05s  1.06% 94.50%      0.56s 11.84%  github.com/SchokiCoder/hawps/mat.(*World).applyChemReactions
     0.05s  1.06% 95.56%      1.77s 37.42%  github.com/SchokiCoder/hawps/mat.(*World).dropGas
     0.05s  1.06% 96.62%      0.05s  1.06%  github.com/SchokiCoder/hawps/mat.BoilPs (inline)
     0.03s  0.63% 97.25%      0.03s  0.63%  github.com/SchokiCoder/hawps/mat.GasGs (inline)
     0.03s  0.63% 97.89%      0.03s  0.63%  github.com/SchokiCoder/hawps/mat.ThCond (inline)
         0     0% 97.89%      4.70s 99.37%  github.com/SchokiCoder/hawps/mat.TestBench
         0     0% 97.89%      4.70s 99.37%  testing.tRunner

```

### new

```
File: mat.test
Type: cpu
Time: Jan 3, 2025 at 12:42pm (CET)
Duration: 4.91s, Total samples = 4.76s (96.92%)
Entering interactive mode (type "help" for commands, "o" for options)
(pprof) top20
Showing nodes accounting for 4.70s, 98.74% of 4.76s total
Dropped 12 nodes (cum <= 0.02s)
      flat  flat%   sum%        cum   cum%
     1.17s 24.58% 24.58%      1.19s 25.00%  github.com/SchokiCoder/hawps/mat.(*World).Tick.func3
        1s 21.01% 45.59%         1s 21.01%  github.com/SchokiCoder/hawps/mat.(*World).swapDots
     0.74s 15.55% 61.13%      0.97s 20.38%  github.com/SchokiCoder/hawps/mat.(*World).Tick.func5
     0.58s 12.18% 73.32%      4.75s 99.79%  github.com/SchokiCoder/hawps/mat.(*World).Tick
     0.48s 10.08% 83.40%      0.48s 10.08%  runtime.duffcopy
     0.25s  5.25% 88.66%      1.93s 40.55%  github.com/SchokiCoder/hawps/mat.(*World).Tick.func2
     0.15s  3.15% 91.81%      0.15s  3.15%  github.com/SchokiCoder/hawps/mat.World.CanDisplace (inline)
     0.08s  1.68% 93.49%      0.08s  1.68%  github.com/SchokiCoder/hawps/mat.(*World).Tick.func1 (inline)
     0.08s  1.68% 95.17%      0.08s  1.68%  github.com/SchokiCoder/hawps/mat.GasGs (inline)
     0.06s  1.26% 96.43%      0.06s  1.26%  github.com/SchokiCoder/hawps/mat.MeltPs (inline)
     0.05s  1.05% 97.48%      1.67s 35.08%  github.com/SchokiCoder/hawps/mat.(*World).dropGas
     0.03s  0.63% 98.11%      0.03s  0.63%  github.com/SchokiCoder/hawps/mat.GasRs (inline)
     0.03s  0.63% 98.74%      0.03s  0.63%  github.com/SchokiCoder/hawps/mat.SolidWeights (inline)
         0     0% 98.74%      4.75s 99.79%  github.com/SchokiCoder/hawps/mat.TestBench
         0     0% 98.74%      4.75s 99.79%  testing.tRunner

```

## Test 2

- world: W 1000, H 1000
- ticks: 150
- 2 large brushes of oxygen and hydrogen

### old

```
File: mat.test
Type: cpu
Time: Jan 3, 2025 at 12:45pm (CET)
Duration: 7.32s, Total samples = 7.16s (97.84%)
Entering interactive mode (type "help" for commands, "o" for options)
(pprof) top20
Showing nodes accounting for 7.04s, 98.32% of 7.16s total
Dropped 12 nodes (cum <= 0.04s)
      flat  flat%   sum%        cum   cum%
     1.87s 26.12% 26.12%      1.87s 26.12%  github.com/SchokiCoder/hawps/mat.(*World).swapDots
     1.51s 21.09% 47.21%      1.55s 21.65%  github.com/SchokiCoder/hawps/mat.(*World).applyThermalConduction.func1
     1.02s 14.25% 61.45%      7.14s 99.72%  github.com/SchokiCoder/hawps/mat.(*World).Tick
     0.76s 10.61% 72.07%      0.76s 10.61%  github.com/SchokiCoder/hawps/mat.(*World).applyChemReactions.func1
     0.59s  8.24% 80.31%      3.12s 43.58%  github.com/SchokiCoder/hawps/mat.(*World).applyGravity.func1
     0.45s  6.28% 86.59%      0.45s  6.28%  runtime.duffcopy
     0.16s  2.23% 88.83%      0.16s  2.23%  github.com/SchokiCoder/hawps/mat.(*World).runSpawners (inline)
     0.12s  1.68% 90.50%      2.52s 35.20%  github.com/SchokiCoder/hawps/mat.(*World).dropGas
     0.11s  1.54% 92.04%      1.66s 23.18%  github.com/SchokiCoder/hawps/mat.(*World).applyThermalConduction
     0.09s  1.26% 93.30%      0.09s  1.26%  github.com/SchokiCoder/hawps/mat.MeltPs (inline)
     0.08s  1.12% 94.41%      3.20s 44.69%  github.com/SchokiCoder/hawps/mat.(*World).applyGravity
     0.08s  1.12% 95.53%      0.08s  1.12%  github.com/SchokiCoder/hawps/mat.World.CanDisplace (inline)
     0.07s  0.98% 96.51%      0.83s 11.59%  github.com/SchokiCoder/hawps/mat.(*World).applyChemReactions
     0.07s  0.98% 97.49%      0.07s  0.98%  github.com/SchokiCoder/hawps/mat.BoilPs (inline)
     0.06s  0.84% 98.32%      0.06s  0.84%  github.com/SchokiCoder/hawps/mat.GasRs (inline)
         0     0% 98.32%      7.15s 99.86%  github.com/SchokiCoder/hawps/mat.TestBench
         0     0% 98.32%      7.15s 99.86%  testing.tRunner

```

### new

```
File: mat.test
Type: cpu
Time: Jan 3, 2025 at 12:43pm (CET)
Duration: 7.13s, Total samples = 6.99s (98.02%)
Entering interactive mode (type "help" for commands, "o" for options)
(pprof) top20
Showing nodes accounting for 6.87s, 98.28% of 6.99s total
Dropped 22 nodes (cum <= 0.03s)
      flat  flat%   sum%        cum   cum%
     1.71s 24.46% 24.46%      1.71s 24.46%  github.com/SchokiCoder/hawps/mat.(*World).swapDots
     1.63s 23.32% 47.78%      1.66s 23.75%  github.com/SchokiCoder/hawps/mat.(*World).Tick.func3
     1.11s 15.88% 63.66%      1.61s 23.03%  github.com/SchokiCoder/hawps/mat.(*World).Tick.func5
     0.71s 10.16% 73.82%      6.97s 99.71%  github.com/SchokiCoder/hawps/mat.(*World).Tick
     0.47s  6.72% 80.54%      0.47s  6.72%  runtime.duffcopy
     0.31s  4.43% 84.98%      2.76s 39.48%  github.com/SchokiCoder/hawps/mat.(*World).Tick.func2
     0.22s  3.15% 88.13%      0.22s  3.15%  github.com/SchokiCoder/hawps/mat.(*World).Tick.func1 (inline)
     0.20s  2.86% 90.99%      0.20s  2.86%  github.com/SchokiCoder/hawps/mat.MeltPs (inline)
     0.14s  2.00% 92.99%      2.45s 35.05%  github.com/SchokiCoder/hawps/mat.(*World).dropGas
     0.13s  1.86% 94.85%      0.13s  1.86%  github.com/SchokiCoder/hawps/mat.World.CanDisplace (inline)
     0.09s  1.29% 96.14%      0.09s  1.29%  github.com/SchokiCoder/hawps/mat.BoilPs (inline)
     0.09s  1.29% 97.42%      0.09s  1.29%  github.com/SchokiCoder/hawps/mat.GasGs (inline)
     0.06s  0.86% 98.28%      0.06s  0.86%  github.com/SchokiCoder/hawps/mat.GasRs (inline)
         0     0% 98.28%      6.98s 99.86%  github.com/SchokiCoder/hawps/mat.TestBench
         0     0% 98.28%      6.98s 99.86%  testing.tRunner

```

## Test 3

- world: W 1000, H 1000
- ticks: 200
- 2 large brushes of oxygen and hydrogen

### old

```
File: mat.test
Type: cpu
Time: Jan 3, 2025 at 12:16pm (CET)
Duration: 113.75s, Total samples = 113.74s (  100%)
Entering interactive mode (type "help" for commands, "o" for options)
(pprof) top20
Showing nodes accounting for 112.87s, 99.24% of 113.74s total
Dropped 34 nodes (cum <= 0.57s)
      flat  flat%   sum%        cum   cum%
    35.34s 31.07% 31.07%     35.34s 31.07%  github.com/SchokiCoder/hawps/mat.(*World).swapDots
    31.37s 27.58% 58.65%     31.37s 27.58%  runtime.duffcopy
    30.46s 26.78% 85.43%     30.46s 26.78%  github.com/SchokiCoder/hawps/mat.World.CanDisplace (inline)
     7.26s  6.38% 91.81%     96.39s 84.75%  github.com/SchokiCoder/hawps/mat.(*World).dropGas.func1
     2.49s  2.19% 94.00%    101.82s 89.52%  github.com/SchokiCoder/hawps/mat.(*World).dropGas
     1.82s  1.60% 95.60%      1.88s  1.65%  github.com/SchokiCoder/hawps/mat.(*World).applyThermalConduction.func1
     1.23s  1.08% 96.69%    113.54s 99.82%  github.com/SchokiCoder/hawps/mat.(*World).Tick
     1.02s   0.9% 97.58%    108.53s 95.42%  github.com/SchokiCoder/hawps/mat.(*World).applyGravity.func1
     0.95s  0.84% 98.42%      0.95s  0.84%  github.com/SchokiCoder/hawps/mat.(*World).applyChemReactions.func1
     0.51s  0.45% 98.87%      5.46s  4.80%  github.com/SchokiCoder/hawps/mat.(*World).dropLiquid.func1
     0.15s  0.13% 99.00%      2.03s  1.78%  github.com/SchokiCoder/hawps/mat.(*World).applyThermalConduction
     0.12s  0.11% 99.10%      1.07s  0.94%  github.com/SchokiCoder/hawps/mat.(*World).applyChemReactions
     0.08s  0.07% 99.17%      5.69s  5.00%  github.com/SchokiCoder/hawps/mat.(*World).dropLiquid
     0.07s 0.062% 99.24%    108.60s 95.48%  github.com/SchokiCoder/hawps/mat.(*World).applyGravity
         0     0% 99.24%    113.55s 99.83%  github.com/SchokiCoder/hawps/mat.TestBench
         0     0% 99.24%    113.55s 99.83%  testing.tRunner
```

### new

```
File: mat.test
Type: cpu
Time: Jan 3, 2025 at 12:13pm (CET)
Duration: 28.37s, Total samples = 28.23s (99.52%)
Entering interactive mode (type "help" for commands, "o" for options)
(pprof) top20
Showing nodes accounting for 27.64s, 97.91% of 28.23s total
Dropped 24 nodes (cum <= 0.14s)
      flat  flat%   sum%        cum   cum%
     7.33s 25.97% 25.97%      7.33s 25.97%  runtime.duffcopy
     6.80s 24.09% 50.05%      6.80s 24.09%  github.com/SchokiCoder/hawps/mat.World.CanDisplace (inline)
     5.19s 18.38% 68.44%      5.19s 18.38%  github.com/SchokiCoder/hawps/mat.(*World).swapDots
     2.39s  8.47% 76.90%      2.45s  8.68%  github.com/SchokiCoder/hawps/mat.(*World).Tick.func3
     1.40s  4.96% 81.86%     28.18s 99.82%  github.com/SchokiCoder/hawps/mat.(*World).Tick
     1.32s  4.68% 86.54%      1.80s  6.38%  github.com/SchokiCoder/hawps/mat.(*World).Tick.func5
     1.16s  4.11% 90.65%     10.80s 38.26%  github.com/SchokiCoder/hawps/mat.(*World).dropGas.func1
     0.65s  2.30% 92.95%      7.19s 25.47%  github.com/SchokiCoder/hawps/mat.(*World).dropLiquid.func1
     0.50s  1.77% 94.72%     22.21s 78.68%  github.com/SchokiCoder/hawps/mat.(*World).Tick.func2
     0.35s  1.24% 95.96%     14.25s 50.48%  github.com/SchokiCoder/hawps/mat.(*World).dropGas
     0.32s  1.13% 97.10%      0.32s  1.13%  github.com/SchokiCoder/hawps/mat.(*World).Tick.func1 (inline)
     0.23s  0.81% 97.91%      7.46s 26.43%  github.com/SchokiCoder/hawps/mat.(*World).dropLiquid
         0     0% 97.91%     28.19s 99.86%  github.com/SchokiCoder/hawps/mat.TestBench
         0     0% 97.91%     28.19s 99.86%  testing.tRunner
```

## Test 4

- world: W 100, H 100
- ticks: 5000
- 2 large brushes of oxygen and hydrogen

### old

```
File: mat.test
Type: cpu
Time: Jan 3, 2025 at 1:11pm (CET)
Duration: 31.37s, Total samples = 31.21s (99.48%)
Entering interactive mode (type "help" for commands, "o" for options)
(pprof) top20
Showing nodes accounting for 31.03s, 99.42% of 31.21s total
Dropped 24 nodes (cum <= 0.16s)
      flat  flat%   sum%        cum   cum%
    13.17s 42.20% 42.20%     13.17s 42.20%  runtime.duffcopy
    12.39s 39.70% 81.90%     12.39s 39.70%  github.com/SchokiCoder/hawps/mat.World.CanDisplace (inline)
     1.82s  5.83% 87.73%     14.98s 48.00%  github.com/SchokiCoder/hawps/mat.(*World).dropGas.func1
     1.52s  4.87% 92.60%     13.71s 43.93%  github.com/SchokiCoder/hawps/mat.(*World).dropLiquid.func1
     0.62s  1.99% 94.59%     15.70s 50.30%  github.com/SchokiCoder/hawps/mat.(*World).dropGas
     0.47s  1.51% 96.09%     14.32s 45.88%  github.com/SchokiCoder/hawps/mat.(*World).dropLiquid
     0.28s   0.9% 96.99%     31.18s 99.90%  github.com/SchokiCoder/hawps/mat.(*World).Tick
     0.25s   0.8% 97.79%      0.26s  0.83%  github.com/SchokiCoder/hawps/mat.(*World).applyThermalConduction.func1
     0.22s   0.7% 98.49%      0.22s   0.7%  github.com/SchokiCoder/hawps/mat.(*World).applyChemReactions.func1
     0.18s  0.58% 99.07%     30.20s 96.76%  github.com/SchokiCoder/hawps/mat.(*World).applyGravity.func1
     0.05s  0.16% 99.23%     30.25s 96.92%  github.com/SchokiCoder/hawps/mat.(*World).applyGravity
     0.04s  0.13% 99.36%      0.30s  0.96%  github.com/SchokiCoder/hawps/mat.(*World).applyThermalConduction
     0.02s 0.064% 99.42%      0.24s  0.77%  github.com/SchokiCoder/hawps/mat.(*World).applyChemReactions
         0     0% 99.42%     31.18s 99.90%  github.com/SchokiCoder/hawps/mat.TestBench
         0     0% 99.42%     31.18s 99.90%  testing.tRunner

```

### new

```
File: mat.test
Type: cpu
Time: Jan 3, 2025 at 1:10pm (CET)
Duration: 31.27s, Total samples = 31.21s (99.79%)
Entering interactive mode (type "help" for commands, "o" for options)
(pprof) top20
Showing nodes accounting for 30.92s, 99.07% of 31.21s total
Dropped 30 nodes (cum <= 0.16s)
      flat  flat%   sum%        cum   cum%
    13.09s 41.94% 41.94%     13.09s 41.94%  runtime.duffcopy
    12.49s 40.02% 81.96%     12.49s 40.02%  github.com/SchokiCoder/hawps/mat.World.CanDisplace (inline)
     1.69s  5.41% 87.38%     14.79s 47.39%  github.com/SchokiCoder/hawps/mat.(*World).dropGas.func1
     1.58s  5.06% 92.44%     13.85s 44.38%  github.com/SchokiCoder/hawps/mat.(*World).dropLiquid.func1
     0.68s  2.18% 94.62%     15.56s 49.86%  github.com/SchokiCoder/hawps/mat.(*World).dropGas
     0.38s  1.22% 95.83%     14.36s 46.01%  github.com/SchokiCoder/hawps/mat.(*World).dropLiquid
     0.36s  1.15% 96.99%      0.41s  1.31%  github.com/SchokiCoder/hawps/mat.(*World).Tick.func3
     0.30s  0.96% 97.95%      0.46s  1.47%  github.com/SchokiCoder/hawps/mat.(*World).Tick.func5
     0.29s  0.93% 98.88%     31.15s 99.81%  github.com/SchokiCoder/hawps/mat.(*World).Tick
     0.06s  0.19% 99.07%     29.98s 96.06%  github.com/SchokiCoder/hawps/mat.(*World).Tick.func2
         0     0% 99.07%     31.15s 99.81%  github.com/SchokiCoder/hawps/mat.TestBench
         0     0% 99.07%     31.15s 99.81%  testing.tRunner

```

## Test 5

- world: W 100, H 100
- ticks: 10000
- 2 large brushes of oxygen and hydrogen

### old

```
File: mat.test
Type: cpu
Time: Jan 3, 2025 at 1:04pm (CET)
Duration: 63.85s, Total samples = 63.80s (99.93%)
Entering interactive mode (type "help" for commands, "o" for options)
(pprof) top20
Showing nodes accounting for 63.34s, 99.28% of 63.80s total
Dropped 33 nodes (cum <= 0.32s)
      flat  flat%   sum%        cum   cum%
    27.45s 43.03% 43.03%     27.45s 43.03%  runtime.duffcopy
    24.35s 38.17% 81.19%     24.35s 38.17%  github.com/SchokiCoder/hawps/mat.World.CanDisplace (inline)
     3.98s  6.24% 87.43%     30.33s 47.54%  github.com/SchokiCoder/hawps/mat.(*World).dropGas.func1
     3.16s  4.95% 92.38%     28.13s 44.09%  github.com/SchokiCoder/hawps/mat.(*World).dropLiquid.func1
     1.21s  1.90% 94.28%     31.82s 49.87%  github.com/SchokiCoder/hawps/mat.(*World).dropGas
     1.06s  1.66% 95.94%     29.43s 46.13%  github.com/SchokiCoder/hawps/mat.(*World).dropLiquid
     0.54s  0.85% 96.79%      0.57s  0.89%  github.com/SchokiCoder/hawps/mat.(*World).applyThermalConduction.func1
     0.52s  0.82% 97.60%     61.77s 96.82%  github.com/SchokiCoder/hawps/mat.(*World).applyGravity.func1
     0.48s  0.75% 98.35%     63.69s 99.83%  github.com/SchokiCoder/hawps/mat.(*World).Tick
     0.40s  0.63% 98.98%      0.40s  0.63%  github.com/SchokiCoder/hawps/mat.(*World).applyChemReactions.func1
     0.08s  0.13% 99.11%      0.65s  1.02%  github.com/SchokiCoder/hawps/mat.(*World).applyThermalConduction
     0.06s 0.094% 99.20%     61.83s 96.91%  github.com/SchokiCoder/hawps/mat.(*World).applyGravity
     0.05s 0.078% 99.28%      0.45s  0.71%  github.com/SchokiCoder/hawps/mat.(*World).applyChemReactions
         0     0% 99.28%     63.69s 99.83%  github.com/SchokiCoder/hawps/mat.TestBench
         0     0% 99.28%     63.69s 99.83%  testing.tRunner

```

### new

```
File: mat.test
Type: cpu
Time: Jan 3, 2025 at 1:06pm (CET)
Duration: 64.05s, Total samples = 63.98s (99.89%)
Entering interactive mode (type "help" for commands, "o" for options)
(pprof) top20
Showing nodes accounting for 63.50s, 99.25% of 63.98s total
Dropped 30 nodes (cum <= 0.32s)
      flat  flat%   sum%        cum   cum%
    25.96s 40.58% 40.58%     25.96s 40.58%  runtime.duffcopy
    25.26s 39.48% 80.06%     25.26s 39.48%  github.com/SchokiCoder/hawps/mat.World.CanDisplace (inline)
     3.97s  6.21% 86.26%     30.26s 47.30%  github.com/SchokiCoder/hawps/mat.(*World).dropGas.func1
     3.49s  5.45% 91.72%     27.78s 43.42%  github.com/SchokiCoder/hawps/mat.(*World).dropLiquid.func1
     1.18s  1.84% 93.56%     31.72s 49.58%  github.com/SchokiCoder/hawps/mat.(*World).dropGas
     1.16s  1.81% 95.37%     29.30s 45.80%  github.com/SchokiCoder/hawps/mat.(*World).dropLiquid
     0.90s  1.41% 96.78%      0.99s  1.55%  github.com/SchokiCoder/hawps/mat.(*World).Tick.func3
     0.83s  1.30% 98.08%     63.88s 99.84%  github.com/SchokiCoder/hawps/mat.(*World).Tick
     0.59s  0.92% 99.00%      0.85s  1.33%  github.com/SchokiCoder/hawps/mat.(*World).Tick.func5
     0.16s  0.25% 99.25%     61.18s 95.62%  github.com/SchokiCoder/hawps/mat.(*World).Tick.func2
         0     0% 99.25%     63.88s 99.84%  github.com/SchokiCoder/hawps/mat.TestBench
         0     0% 99.25%     63.88s 99.84%  testing.tRunner

```

## Observations

The runtime doesn't increase gradual with the amount of ticks to be run,
especially in the old version.  
This is especially notable with larger worlds.  
