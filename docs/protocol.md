# F407 UART Protocol

Serial settings: `115200 8N1`.

Commands are ASCII lines terminated by newline.

## Motion

```text
V <vx> <vy> <wz>
```

- `vx > 0`: forward
- `vx < 0`: backward
- `vy > 0`: right
- `vy < 0`: left
- `wz > 0`: rotate right
- `wz < 0`: rotate left

Examples:

```text
V 600 0 0
V 0 -600 0
V 0 0 600
S
```

## Single Motor Test

```text
M <index> <speed>
```

`index` is motor number `1..4`.

## Control And Diagnostics

```text
S          stop all motors
G          print current status
T          print current trims/status
R          enable closed-loop mode
C          disable closed-loop mode
D          run diagnostics/demo if enabled
P 1        enable periodic telemetry
P 0        disable periodic telemetry
```

Current wheel trim values used during bring-up:

```text
TRIM 890 920 1270 1040
```
