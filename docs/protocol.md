# F407 UART Protocol

F407 USB/UART1 debug/control settings: `115200 8N1`.

H750 WiFi bridge to F407 UART3 settings: `115200 8N1`.

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
G          print current wheel trims
R          print encoder counts
C          reset encoder counts
D          print closed-loop diagnostics
P 1        enable closed-loop mode
P 0        disable closed-loop mode
T n trim   set wheel trim, n is 1..4, trim is 500..1500
```

Current wheel trim values used during bring-up:

```text
TRIM 890 920 1270 1040
```
