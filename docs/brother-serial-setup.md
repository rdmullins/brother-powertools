# Brother PowerTools Serial Login Setup

Brother PowerTools uses a USB-to-serial connection to communicate with the Brother PowerNote. Linux provides a login prompt on the serial port using `agetty`.

## The Problem

The first attempt used:

```text
getty@ttyUSB0.service

This is the generic virtual-console getty and is not the appropriate service for a serial device. The correct service is:

serial-getty@ttyUSB0.service

At one point, getty@ttyUSB0 was started manually and appeared to work, but it subsequently exited and entered a restart loop. The system journal showed repeated:

Started getty@ttyUSB0.service
Deactivated successfully.
Scheduled restart job...

The correct serial getty was already configured but had stopped running.

Correct Configuration

The serial getty uses a systemd drop-in override:

/etc/systemd/system/serial-getty@ttyUSB0.service.d/override.conf

The contents should be:

[Service]
ExecStart=
ExecStart=-/sbin/agetty -o '-p -- \\u' --keep-baud 9600 %I dumb

The empty ExecStart= clears the command supplied by the stock serial-getty@.service template. The following ExecStart= supplies the Brother-specific configuration:

9600 baud
dumb terminal type
agetty login prompt
\\u is required in the systemd unit file so that agetty receives the literal \u username substitution sequence.
Important Systemd Escaping Detail

The following is incorrect:

ExecStart=-/sbin/agetty -o '-p -- \u' --keep-baud 9600 %I dumb

Systemd interprets the backslash itself and reports:

Ignoring unknown escape sequences

Use:

ExecStart=-/sbin/agetty -o '-p -- \\u' --keep-baud 9600 %I dumb
Applying or Repairing the Configuration

After modifying the override, reload systemd:

sudo systemctl daemon-reload

Verify the unit:

sudo systemd-analyze verify serial-getty@ttyUSB0.service

A successful verification produces no output.

Start the serial getty:

sudo systemctl start serial-getty@ttyUSB0

Check its status:

systemctl status serial-getty@ttyUSB0 --no-pager

It should report:

Active: active (running)

Enable it so the Brother login prompt is available automatically after boot:

sudo systemctl enable serial-getty@ttyUSB0

Verify:

systemctl is-enabled serial-getty@ttyUSB0

Expected result:

enabled
Troubleshooting

Check that the serial device exists:

ls -l /dev/ttyUSB0

Check the serial port configuration:

stty -F /dev/ttyUSB0 -a

Check the serial getty journal:

journalctl -u serial-getty@ttyUSB0 -n 30 --no-pager

If serial-getty@ttyUSB0.service reports that the unit is bad, run:

sudo systemd-analyze verify serial-getty@ttyUSB0.service

This will usually identify syntax errors in the override file.

Known-Good Configuration

As of this writing, the known-good configuration is:

Device:       /dev/ttyUSB0
Baud rate:    9600
Getty:        serial-getty@ttyUSB0.service
Terminal:     dumb
Login:        agetty

Do not use getty@ttyUSB0.service for the Brother serial connection.

Notes

The Brother Communications program behaves more like a slow, teletype-style serial display than a modern ANSI terminal. ANSI escape sequences such as:

ESC [ 2 J
ESC [ H

are not interpreted as screen-control commands by the Brother; they are displayed as ordinary input.

Similarly, printing blank lines is not a practical way to clear the screen. The Brother's output is slow enough that filling the display with blank lines can take approximately ten seconds.

PowerTools should therefore avoid relying on terminal screen-clearing sequences and should minimize unnecessary output. The startup splash screen can be printed once, while subsequent menus should be compact and designed around the Brother's streaming display behavior.


Honestly, for a tiny `.md` file, **copy/paste into VS Code is probably faster anyway**. And now it's going s