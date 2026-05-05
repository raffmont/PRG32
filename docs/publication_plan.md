# PRG32 Publication Plan

## Target

Conference or journal on:

- Computer science education
- Embedded systems education
- Systems programming pedagogy

Examples:
- SIGCSE
- ITiCSE
- EDUCON

## Paper title (candidate)

"Teaching Low-Level Systems Through Game-Based RISC-V Runtime Abstractions"

## Core idea

PRG32 bridges the gap between:

- raw assembly programming
- full application-level development

by introducing a constrained, game-oriented runtime with a well-defined ABI.

## Contributions

1. A lightweight runtime enabling structured assembly programs
2. A cartridge abstraction for modular deployment
3. A QEMU-based workflow for scalable teaching
4. A gamified learning pipeline
5. Empirical evaluation of student engagement and outcomes

## Evaluation design

### Quantitative

- Assignment completion rate
- Final project success rate
- Error rates in early labs vs later labs
- Frame time / performance improvements

### Qualitative

- Student surveys
- Perceived difficulty
- Engagement level

### Comparative study (optional)

Compare:

- Traditional assembly course
- PRG32-based course

## Artifacts

- GitHub repository (PRG32)
- Teaching materials
- Example student projects
- Evaluation dataset (anonymized)

## Figures to include

- Runtime architecture diagram
- Cartridge pipeline
- Teaching progression
- Example game screenshots

## Timeline

1. Stabilize repository (engineering hardening)
2. Run course pilot
3. Collect data
4. Analyze results
5. Submit paper

## Risks

- Toolchain complexity
- Hardware availability
- Student onboarding difficulty

Mitigation:

- Strong QEMU path
- Clear documentation
- Guided labs

## Expected impact

- Improved engagement in low-level courses
- Better understanding of runtime/ABI concepts
- Reusable teaching framework
