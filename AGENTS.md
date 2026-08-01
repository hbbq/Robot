# Robot Project – Agent Instructions

## Project overview

This is a PlatformIO / Arduino ESP32 project for a small robot system consisting of multiple cooperating ESP32-C6 devices.

The system currently has three main device types:

1. **Robot**
   - Controls the motors.
   - Runs autonomous and remote-control behaviors.
   - Owns the authoritative robot state.
   - Reports mode and motion to other devices.
   - Must stop when required communication/readiness conditions are lost.

2. **Display**
   - Displays an animated robot face.
   - Receives robot state over the network.
   - Changes the face based on readiness, robot mode and robot motion.
   - Does not control the robot.

3. **Remote**
   - Uses a Waveshare ESP32-C6 Touch LCD 1.69.
   - Provides a touchscreen joystick.
   - Can request robot modes.
   - Displays robot connection status, mode and motion.
   - Sends continuous drive commands while the joystick is active.

Communication between devices currently uses ESP-NOW.

---

# General architecture

Prefer small classes with clearly defined responsibilities.

Application classes such as:

- `RobotApp`
- `DisplayApp`
- `RemoteApp`

act primarily as **composition roots and lifecycle coordinators**.

They should:

- construct dependencies;
- call `begin()`;
- call `update()`;
- coordinate high-level application state where appropriate.

They should NOT accumulate hardware implementation, protocol parsing, UI rendering or motor-control details.

Avoid putting substantial logic in `main.cpp`.

`main.cpp` should normally only select/create the correct application and call its lifecycle methods.

---

# Device architecture

The general dependency direction should be approximately:

    Hardware drivers
          ↓
    Services / state stores
          ↓
    Controllers
          ↓
    Behaviors / UI
          ↓
    App

Avoid dependencies in the opposite direction.

Hardware-specific classes should implement interfaces where practical.

Examples include:

- `IDisplayDriver`
- `ITouchController`
- `IDriveController`
- motor controller interfaces
- clock/time abstractions

This makes it possible to use fake implementations while hardware is unavailable.

---

# Communication

Communication is handled through shared networking infrastructure.

Important concepts include:

- `EspNowManager`
- `DeviceNetworkService`
- `DeviceRegistry`
- `MessageDispatcher`
- message structs
- sequence numbers
- announcements
- heartbeats

`DeviceNetworkService` should provide semantic operations such as:

    sendAnnouncement()
    sendHeartbeat()
    sendRobotState(...)
    sendDriveCommand(...)
    sendSetRobotMode(...)

Callers should generally not manually construct or manage sequence numbers.

Sequence numbers belong to the networking/message layer.

---

# Device discovery and heartbeat

Devices announce themselves and send periodic heartbeats.

`DeviceRegistry` tracks known devices and when they were last seen.

Readiness logic uses this information to determine whether required devices are available.

ESP-NOW packets may be lost.

Therefore, important state must not rely exclusively on one-shot state-change events.

---

# Robot state

The Robot is authoritative for its current state.

Important state currently includes:

    RobotMode
    RobotMotion

Current robot modes include:

    Idle
    Autonomous
    RemoteControl

Robot motion includes concepts such as:

    Stopped
    Forward
    Backward
    TurningLeft
    TurningRight
    Curve

Do not infer robot mode on Display or Remote.

The Robot reports its actual state using `RobotStateMessage`.

`RobotStateMessage` is a **state snapshot**, not merely an event.

The Robot should send robot state:

- immediately when mode changes;
- immediately when motion changes;
- periodically even when nothing changes.

The current periodic interval is approximately 2 seconds.

This allows receivers to recover from lost ESP-NOW packets and late startup.

---

# State stores

Received network state should generally be written into small state-store classes rather than directly triggering unrelated application behavior.

Examples include:

    RobotStateStore
    RemoteDriveState
    RobotModeRequestStore

`MessageDispatcher` should primarily:

1. validate/deserialize messages;
2. update the appropriate state/store.

It should not directly switch robot behaviors or control motors.

---

# Robot mode changes

Remote devices send a `SetRobotModeMessage`.

This is a **request**, not authoritative state.

The Robot receives the request into `RobotModeRequestStore`.

`RobotApp` processes the request and performs the actual mode/behavior transition.

After changing mode, the Robot reports the resulting mode through `RobotStateMessage`.

Remote and Display should use the reported robot state as truth.

---

# Behaviors

Robot behaviors encapsulate high-level robot actions.

Examples currently include:

    IdleBehavior
    RandomDriveBehavior
    RemoteControlBehavior

A `BehaviorController` manages the active behavior.

Autonomous behaviors should normally use `MotionController` for higher-level movements such as:

    forward(distance)
    turn(degrees)

`MotionController` implements timed movement and exposes robot motion.

`MotionController::update()` is driven by the application lifecycle rather than individual behaviors.

---

# Remote control

`RemoteControlBehavior` is different from autonomous movement.

Remote control receives continuous:

    linear
    angular

values, normally in the range:

    -1.0 .. +1.0

These are sent directly to `IDriveController` / `DriveController`.

Remote control should NOT convert every joystick update into a timed `MotionController` command.

The remote sends drive commands periodically while the joystick is active.

The current target interval is approximately:

    100 ms (10 Hz)

When touch is released, the Remote immediately sends:

    linear = 0
    angular = 0

---

# Remote-control safety

The Robot must never depend solely on receiving the explicit `0,0` stop packet.

`RemoteControlBehavior` implements a dead-man timeout.

If no fresh drive command has arrived within approximately:

    500 ms

the Robot must stop.

This protects against:

- lost packets;
- remote power loss;
- network loss;
- application failure.

Do not remove this timeout when modifying remote-control behavior.

---

# Readiness and safety

Devices use readiness logic to determine whether required cooperating devices are available.

The Robot currently requires communication with the Display before it is considered ready to operate.

If required readiness is lost while the Robot is moving, it must stop.

Safety checks belong on the Robot side.

Do not rely on Remote or Display to stop the Robot correctly.

Readiness may optionally control a status LED.

The readiness implementation must also work on devices without a status LED.

Remote currently has no readiness LED.

---

# LEDs

There is an existing `LedController`.

It supports modes such as steady and blinking status indication.

Do not implement blocking LED blinking with `delay()`.

LED behavior should remain non-blocking.

Some GPIOs conflict with planned motor-driver pins.

Fake motor controllers may be used until the physical motor hardware is installed.

---

# Motor control

The physical motor driver is planned around a TB6612-based controller.

There is also a `FakeMotorController` used during development.

Prefer testing higher-level behavior against the fake implementation until the actual hardware is connected.

Do not introduce hardware assumptions into behavior classes.

---

# Display architecture

Display hardware is abstracted through `IDisplayDriver`.

The interface contains basic drawing primitives rather than application-specific concepts.

Typical operations include:

    begin()
    flush()

    width()
    height()

    setRotation()
    setBrightness()

    clear()

    drawPixel()
    drawLine()
    drawRect()
    fillRect()
    drawCircle()
    fillCircle()
    drawTriangle()
    fillTriangle()

    setCursor()
    setTextColor()
    setTextSize()
    print()

Do not introduce application-specific types such as UI `Rect` or application color constants into the hardware interface unless there is a strong reason.

Use conventional graphics parameter naming:

    x0, y0
    x1, y1
    x2, y2

for endpoints/vertices, and:

    x, y, width, height

for rectangles.

---

# Display buffering

The current displays use Arduino GFX with `Arduino_Canvas`.

Drawing happens into the canvas.

Changes are not visible until:

    flush()

is called.

Be careful when debugging an apparently blank display: missing `flush()` has previously caused this.

---

# Waveshare 1.47 Touch display

The Display device uses a Waveshare ESP32-C6 Touch LCD 1.47.

This panel requires a special LCD register initialization sequence.

Important:

`Arduino_Canvas::begin()` reinitializes the underlying display.

Therefore the custom panel-specific `lcd_reg_init()` must run AFTER `canvas->begin()`.

Do not remove or reorder this without verifying the physical display.

The display has previously shown incorrect orientation/colors when initialized incorrectly.

The 1.47 display driver should remain separate from the 1.69 driver.

---

# Waveshare 1.69 Touch display

The Remote uses:

    Waveshare ESP32-C6 Touch LCD 1.69

Display:

    ST7789
    240 x 280
    rotation = 0

LCD pins:

    SCK  = GPIO 1
    MOSI = GPIO 2
    CS   = GPIO 5
    DC   = GPIO 3
    RST  = GPIO 4
    BL   = GPIO 6

Display configuration:

    IPS = true

    column offset rotation 0 = 0
    row offset rotation 0    = 20

    column offset rotation 2 = 0
    row offset rotation 2    = 20

Unlike the 1.47 panel, this display does not currently require the custom `lcd_reg_init()` sequence.

Keep this as a separate display driver.

---

# Touch – Remote

The 1.69 Remote uses a CST816T touch controller.

It is wrapped by:

    Cst816TouchDriver

and exposed through:

    ITouchController

Current touch pins:

    SDA = GPIO 8
    SCL = GPIO 7
    IRQ = GPIO 11

The implementation uses SensorLib / `TouchDrvCSTXXX`.

Touch coordinates have been verified to correspond correctly to the 240x280 display when using rotation 0.

`newTouch(x, y)` currently remains true while the screen is held, which is useful for continuous joystick control.

---

# Remote UI

The Remote UI is coordinated by:

    RemoteUiController

It should be the single owner/coordinator of:

- touch processing;
- screen rendering;
- mode buttons;
- joystick interaction;
- drive-command transmission.

Do not create multiple independent controllers that simultaneously consume `ITouchController` events or clear/flush the same display.

The joystick mathematics/state is separated into:

    JoystickModel

`JoystickModel` owns:

- joystick center;
- radius;
- knob position;
- active state;
- linear value;
- angular value;
- dead zone.

It does not access hardware, networking or the display.

---

# Remote UI state

Remote UI should clearly display whether the Robot is connected.

Connection status comes from readiness/device heartbeat state.

The Remote supports three mode controls:

    IDLE
    AUTO
    REMOTE

Button selection must reflect:

    RobotStateStore::mode()

not the last mode requested by the Remote.

This ensures the UI displays the mode that the Robot actually reports.

When in Autonomous mode, the Remote may display the Robot's current `RobotMotion` as text, for example:

    Stopped
    Forward
    Backward
    Turning left
    Turning right

Joystick control should only be active when the Robot reports:

    RobotMode::RemoteControl

---

# Face display

The Display device contains a `FaceController`.

The face depends primarily on:

    readiness
    RobotMode
    RobotMotion

The face should display a not-ready state when communication/readiness requirements are not satisfied.

Remote-control mode has a distinct face.

Motion can affect eye direction/expression.

Face animations are non-blocking and driven through `update()`.

Avoid `delay()` in face animations.

Rendering should only occur when required (`dirty` state / animation frame) rather than continuously clearing/flushing the display.

---

# Timing

Use non-blocking timing based on `millis()` or the project's `IClock`.

Avoid blocking delays in normal runtime logic.

Do not use delay() inside:
- update() loops
- controllers
- behaviors
- networking
- animations
- UI interaction

A short delay() during application startup / begin() is acceptable when
needed for serial initialization, hardware stabilization, or debugging.

Use wrap-safe elapsed-time checks:

    nowMs - previousMs >= intervalMs

rather than comparing absolute future timestamps.

---

# ESP32 / concurrency

Avoid unnecessary FreeRTOS tasks.

Prefer the normal application `update()` loop for lightweight state machines and periodic work.

Use atomics/volatile/interrupt-safe code only where actually required.

Interrupt handlers should do minimal work, typically setting a flag.

---

# C++ conventions

Prefer:

- `enum class`
- `constexpr`
- constructor injection
- references for required non-owning dependencies
- value members for objects owned by a class
- `const` where appropriate
- fixed-width integer types for protocol data
- explicit ownership

Forward declarations are appropriate for pointer/reference members.

If a class owns another class by value:

    Foo _foo;

the header must include the complete definition of `Foo`.

Avoid unnecessary dynamic allocation.

---

# Headers and implementation files

Non-trivial classes should normally be split into:

    ClassName.h
    ClassName.cpp

Avoid putting global hardware objects in headers.

Hardware objects should preferably be members of their driver class.

Keep hardware-specific constants in the implementation file when they do not need to be public.

---

# Protocol structs

Network message structs should remain simple and deterministic.

Prefer:

- fixed-width integer types;
- explicit enum underlying types;
- trivially copyable structs;
- reserved bytes when useful for stable layout;
- validation of protocol version/type/length.

Do not put `std::string`, pointers, virtual methods or dynamically allocated data into raw ESP-NOW message structs.

---

# Logging

Serial logging is useful during development.

Use clear component prefixes, for example:

    [Motion]
    [Robot]
    [Remote]
    [Display]
    [Network]

Avoid high-frequency logging in final hot paths unless it is useful for diagnostics.

---

# When modifying the project

Before making architectural changes:

1. Inspect the existing implementation.
2. Follow the established naming and directory structure.
3. Prefer extending existing abstractions over creating parallel ones.
4. Check whether a class/service already provides the needed functionality.
5. Avoid duplicating sequence-number, heartbeat, readiness, timing or state logic.
6. Keep Robot, Display and Remote behavior consistent with the shared protocol.
7. Preserve robot-side safety behavior.

Do not assume a class or method exists merely because it would be convenient.

Search the repository first.

---

# Build and validation

This is a PlatformIO project.

After non-trivial code changes, build the affected PlatformIO environment(s).

When changing shared code, build all affected device environments when practical.

Do not treat successful compilation of one device as proof that shared changes compile for Robot, Display and Remote.

When hardware behavior cannot be validated locally, clearly distinguish:

- compile-time verification;
- simulated/fake verification;
- physical hardware verification.

---

# Current development philosophy

Prefer simple working implementations first.

Do not prematurely introduce generalized frameworks or deep abstraction hierarchies.

For example:

- separate 1.47 and 1.69 display drivers are currently preferred;
- a shared display-driver base class should only be introduced if repeated stable behavior clearly justifies it;
- fake hardware implementations are preferred while physical hardware is unavailable;
- UI abstractions should emerge from actual needs rather than being designed far in advance.

The project is experimental and evolving.

Preserve clarity and debuggability over cleverness.


# PlatformIO

PlatformIO may not be available on PATH.

On Windows, use the PlatformIO executable directly:

    %USERPROFILE%\.platformio\penv\Scripts\pio.exe

Build environments separately when necessary to avoid truncated output:

    pio.exe run -e robot
    pio.exe run -e display
    pio.exe run -e remote

After changes to shared code, verify all three environments.