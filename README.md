# Spaceium — Cryogenic Fluid Transfer Simulator

> Embedded flight software prototype for an autonomous cryogenic propellant transfer control system. Written in **C99** targeting a bare-metal / RTOS environment, with a hardware abstraction layer (HAL) that isolates all hardware I/O behind a clean functional API — enabling the same controller binary to run on hardware or in simulation without a single `#ifdef`.

---

## Overview

Cryogenic propellants (liquid oxygen, liquid hydrogen, liquid methane) are the backbone of modern launch vehicles. Transferring them safely between spacecraft in orbit demands precise, deterministic control logic with **fail-safe guarantees**: pressures must be equalized before flow begins, lines must be pre-chilled to prevent hammer shock, and any overpressure event must trigger an immediate, latched emergency shutdown.

This project implements:

- A **6-state deterministic state machine** managing the full transfer sequence
- A **real-time safety interlock** that checks pressure invariants on every control tick before executing any state logic
- A **hardware abstraction layer** mapping controller commands to SPI, PWM, and GPIO peripheral drivers
- An **automated firmware test bench** validating nominal transitions and fault injection scenarios

---

## System Architecture

### Hardware-to-Software Stack

```
┌─────────────────────────────────────────────────────────────────────┐
│                      PHYSICAL HARDWARE LAYER                        │
│                                                                     │
│   ┌──────────────────────┐  ┌────────────────────────┐  ┌────────┐ │
│   │  SPI Cryo Transducer │  │  PWM Proportional Valve│  │  GPIO  │ │
│   │   (pressure sensor)  │  │   (cryogenic flow ctrl)│  │  Vent  │ │
│   └──────────┬───────────┘  └───────────┬────────────┘  └───┬────┘ │
└──────────────│───────────────────────────│────────────────────│─────┘
               │  Reads Registers /        │  Writes Registers /│
               │  Drives Voltages          │  Flips Pin States  │
┌──────────────▼───────────────────────────▼────────────────────▼─────┐
│              HARDWARE ABSTRACTION LAYER  [cryo_hal.c]               │
│                                                                     │
│   HAL_read_pressure_sensor_SPI()                                    │
│   HAL_write_cryo_valve_PWM(duty_cycle)                              │
│   HAL_write_vent_valve_GPIO(state)                                  │
│                                                                     │
│                    Exposes Safe Functional API                       │
└─────────────────────────────┬───────────────────────────────────────┘
                              │
┌─────────────────────────────▼───────────────────────────────────────┐
│              CORE LOGIC ENGINE  [cryo_controller.c]                 │
│                                                                     │
│   CRYO_check_safety_invariants()   ← runs first, every tick        │
│   CRYO_run_control_logic()         ← state machine dispatcher      │
└─────────────────────────────────────────────────────────────────────┘
```

The HAL boundary means the controller never touches a hardware register directly. Swapping from simulation to real silicon requires only a new `cryo_hal.c` — the controller and all tests remain unchanged.

---

### State Machine

```
                        ┌─────────┐
                        │  START  │
                        └────┬────┘
                             │
                             ▼
                    ┌─────────────────┐
          ┌────────►│   STATE_IDLE    │◄──────────────────────────┐
          │         └────────┬────────┘                           │
          │                  │ Command Received                    │
          │                  ▼                                     │
          │        ┌──────────────────────┐                       │
          │        │   STATE_PRE_CHILL    │                       │
          │        │  (5% PWM, vent open) │                       │
          │        └──────────┬───────────┘                       │
          │                   │ Temperature Satisfied              │
          │                   ▼                                    │
          │   ┌───────────────────────────────────┐               │
          │   │   STATE_PRESSURE_EQUALIZATION     │               │
          │   │      (15% PWM, vent closed)       │               │
          │   └────────────────┬──────────────────┘               │
          │                    │ Pressure Match                    │
          │                    ▼                                   │
          │      ┌──────────────────────────┐                     │
          │      │    STATE_TRANSFERRING    │                     │
          │      │  (100% PWM, full flow)   │                     │
          │      └──────────┬───────────────┘                     │
          │                 │ Transfer Complete                    │
          │                 ▼                                      │
          │      ┌──────────────────────────┐   Flush Finished    │
          └──────│     STATE_PURGING        │────────────────────►┘
                 │  (0% PWM, vent open)     │
                 └──────────────────────────┘


  ──────────── At ANY state, if tank_pressure ≥ 300 PSI ────────────►

                 ┌──────────────────────────┐
                 │   STATE_EMERGENCY_FAULT  │  (latched — cryo valve
                 │   0% PWM  |  vent open   │   closed, vent open)
                 └──────────────────────────┘
```

**Safety guarantee:** `CRYO_check_safety_invariants()` executes unconditionally at the top of every control tick, before any state logic runs. An overpressure event transitions the system to `STATE_EMERGENCY_FAULT` and latches it there — no further state transitions are possible without a system reset.

---

## Project Structure

```
.
├── cryo_defs.h           # CryoSystem_t struct, CryoState_t enum, safety thresholds
├── cryo_hal.h            # HAL interface declarations
├── cryo_hal.c            # HAL implementation (simulated I/O with test override)
├── cryo_controller.h     # Controller API declarations
├── cryo_controller.c     # Safety interlock + state machine implementation
├── main.c                # Production simulator entry point (mission executive loop)
├── test_cryo.c           # Firmware test bench (nominal + fault injection)
├── CMakeLists.txt        # Build system (produces sim and testbench binaries)
├── State_Machine.drawio          # State machine architecture diagram
└── Hardware-to-Software.drawio   # Hardware-to-software stack diagram
```

---

## Key Design Decisions

| Decision | Rationale |
|---|---|
| HAL isolation behind a pure functional API | Controller logic is hardware-agnostic; the same binary can target simulation, FPGA, or silicon |
| Safety check runs before state dispatch, every tick | Guarantees no state transition can proceed under a fault condition — no reachable code path bypasses the interlock |
| `HAL_Test_Override_Pressure` global for test injection | Lets the test bench inject arbitrary sensor values without modifying controller or HAL source |
| `-Wall -Wextra -Werror` enforced at compile time | Treats all warnings as errors — enforces code hygiene across the embedded codebase |
| C99 with `stdint.h` / `stdbool.h` | Explicit-width integer types and boolean semantics with zero ambiguity across target architectures |

---

## Build

**Prerequisites:** CMake ≥ 3.10, GCC or Clang with C99 support.

```bash
# Configure
cmake -S . -B build

# Build both targets
cmake --build build
```

This produces two binaries in `build/`:

| Binary | Description |
|---|---|
| `spaceium_cryo_sim` | Production flight simulator — runs the mission executive loop |
| `spaceium_cryo_testbench` | Firmware test bench — runs all automated test cases |

---

## Run

### Production Simulator

```bash
./build/spaceium_cryo_sim
```

Runs a 4-tick mission executive loop, cycling the cryogenic system from `STATE_IDLE` through the transfer sequence. Each tick prints HAL-level valve commands as the state machine advances.

### Firmware Test Bench

```bash
./build/spaceium_cryo_testbench
```

Executes the automated test suite and reports pass/fail with diagnostic output:

```
[PASS] Nominal Transition to PRE_CHILL
[PASS] Nominal Transition to EQUALIZATION
[PASS] Overpressure Trapped and Handled Successfully

Tests run: 3  |  Passed: 3  |  Failed: 0
```

**Test coverage:**

| Test | Description |
|---|---|
| `Test_Nominal_State_Transitions` | Verifies IDLE → PRE_CHILL → PRESSURE_EQUALIZATION under safe pressure conditions |
| `Test_Overpressure_Safety_Interlock` | Injects 345 PSI via `HAL_Test_Override_Pressure` and verifies immediate latch to `STATE_EMERGENCY_FAULT` |

---

## Safety Thresholds

| Parameter | Value | Source |
|---|---|---|
| Max safe tank pressure | 300.0 PSI | `MAX_SAFE_PRESSURE_PSI` in `cryo_defs.h` |
| Critical boil-off temperature | 90.0 K | `CRITICAL_BOIL_OFF_TEMP_K` in `cryo_defs.h` |

---

## Author

**Prajwal Neupane**  
prajwalneupane.apag@gmail.com
