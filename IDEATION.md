# Robot Ideation Guidance

Generate useful, interesting ideas for the current Robot project.

The project is an experimental multi-device ESP32-C6 robot system. Ideas should fit the existing architecture and should normally be achievable incrementally without redesigning the whole system.

## Project character

The system currently consists of cooperating device roles such as:

- Robot: motors, robot state, safety and autonomous/remote behavior
- Display: animated robot face and status
- Remote: touchscreen control and robot status

Communication currently uses ESP-NOW with shared discovery, heartbeat, message, state-store and readiness infrastructure.

The project already favors small classes, explicit responsibilities, non-blocking timing, fake hardware where useful, and simple working implementations before generalized frameworks.

## Prefer

Favor ideas in areas such as:

- New robot behaviors that can be built from existing motion/sensor capabilities
- Small autonomous experiments
- Better ways to use existing sensors
- New sensors or peripherals that add genuinely useful perception
- Obstacle detection, local exploration and environment sensing
- Head/sensor movement strategies
- Calibration and learning experiments
- Robot state, events and measurements that would be useful to an external AI backend
- Capability discovery and robot-control protocol ideas
- Safe, simple action primitives that an AI backend could compose into higher-level behavior
- Observability and diagnostics that make physical behavior easier to understand
- Face/display behavior that reflects meaningful robot state
- Remote-control features that improve experimentation or debugging
- New ESP32 device roles when they add a concrete capability
- Camera, still-image, microphone or audio-related experiments where they fit the available/planned hardware
- Small steps toward a robot that can expose actions, events and measurements to an external agent

## Especially interesting

Ideas are particularly valuable when they:

- Let an external AI agent learn how this particular robot behaves
- Help the robot or backend reduce uncertainty about the environment
- Turn raw sensor input into a useful event or measurement
- Support experiments such as learning motion characteristics, exploring surroundings, detecting changes or building a simple world model
- Reuse the existing communication and state architecture rather than creating parallel infrastructure
- Preserve robot-side safety while allowing higher-level intelligence to live outside the ESP32 firmware
- Can be tested incrementally using fake hardware or existing devices before requiring all planned hardware

## AI-backend direction

The longer-term architecture may include a separate AI backend.

The Robot firmware should normally expose simple capabilities rather than embed high-level AI behavior.

Examples of useful concepts include:

- Actions: forward, backward, turn, stop, move/rotate a sensor head, capture media where supported
- Events: obstacle detected, sound detected, readiness lost, noteworthy state change
- Measurements: distance, battery, orientation, environmental or other sensor data
- Capability discovery: describing which actions, events and measurements a particular robot/device supports

Do not assume this protocol already exists. Ideas may propose small, concrete steps toward it.

Prefer semantic robot-level operations over exposing low-level GPIO, PWM or implementation details to an external agent unless the idea is specifically for diagnostics.

## Safety

Robot-side safety is important.

Do not propose designs that move collision avoidance, dead-man behavior, communication-loss stopping or other immediate safety requirements exclusively into an external AI/backend.

An external agent may decide what it wants the robot to do, but the Robot must remain able to stop safely when local safety/readiness conditions require it.

## Avoid

Do not propose ideas whose main value is:

- Generic refactoring
- Architecture for architecture's sake
- Large framework rewrites
- Replacing established project abstractions with parallel abstractions
- Introducing unnecessary FreeRTOS tasks or concurrency
- Moving simple deterministic firmware behavior to cloud services without a concrete benefit
- Prematurely designing a universal robotics platform
- Complex navigation/SLAM stacks that require capabilities the current hardware does not have
- Test-only work unless it enables a concrete capability or experiment
- Removing or weakening existing safety behavior

Do not propose ideas already represented by open issues or existing implemented features.

## Scope

Prefer one clear capability, experiment or behavior per idea.

A good idea should normally be small or medium sized and suitable for one GitHub issue. It should state what becomes possible and why it is interesting, while leaving detailed implementation decisions to triage/investigation.

If no genuinely useful or interesting distinct idea fits the current project, return no idea rather than inventing filler.
