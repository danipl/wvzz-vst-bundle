## Description

<!-- Brief summary of proposed change and motivations -->

## OpenSpec Change Checklist

- [ ] Change proposal matches an existing or approved OpenSpec change (`openspec/changes/<change-id>/`)
- [ ] Spec delta files (`spec.md`, `design.md`, `tasks.md`) are updated and verified with `openspec validate`
- [ ] Documentation updated (`README.md`, `MANUAL.md`)

## SOLID & Real-Time Audio Architectural Checklist

- [ ] **Single Responsibility Principle (SRP):** DSP logic, parameters, and UI view are decoupled into dedicated classes.
- [ ] **Open/Closed Principle (OCP):** New modules or multi-instance slots are pluggable without modifying core host audio engine logic.
- [ ] **Liskov Substitution Principle (LSP):** All modules implement `IPluginModule` and `IDSPModule` polymorphically.
- [ ] **Interface Segregation Principle (ISP):** Distinct interfaces for DSP (`IDSPModule`), metering (`IMeterSource`), and UI views (`IModuleView`).
- [ ] **Dependency Inversion Principle (DIP):** Modules receive pre-allocated `ProcessContext` for audio buffers and modulation offsets.
- [ ] **Multi-Instance Parameter Prefixing:** Module parameters support custom `instancePrefix`.
- [ ] **Real-Time Safety:** Strictly zero heap allocations (`malloc`/`new`), zero locks, and `noexcept` inside `processBlock()` and `process()`.
- [ ] **Universal Binary Build:** `make test` passes 100%, and `make vst3` / `make standalone` produce valid `arm64` and `x86_64` Mach-O binaries.
