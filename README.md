# NodeMCU ESP8266 Dual Relay Wireless Switch #

## BOM ##

- NodeMCU module (1x)
- Power adapter for NodeMCU module (1x)
- Protoboard 40 mm x 60 mm (1x)
- Finder 36.11.9.003.4011 3V relay or equivalent (2x)
- Set of terminal blocks (4 screws at least)
- Resistor 850 - 1000 Ω (2x)
- BC337-16 NPN Amplifier Transistor in TO-92 package or equivalent (2x)
- 50 cm of insulated wires (preferably three different colors)

## Tools ##

- Decent soldering station
- Patience

## Basic Understanding ##

ESP8266 is my controller of choice because it's extremely cheap, has WiFi
capability and can be programmed using Arduino IDE. NodeMCU has a voltage
regulator, so we can power it from any micro USB power adapter supplying 5 volts
at 0.3 Amps (which is basically any micro USB power adapter even with a
lousy cable).

We are not switching with transistors, but we use relays instead. They are also
cheap (1.5 - 2 €/piece) and capable of switching heavier loads than needed in
most applications (230V at 10 Amps). If in any case you want to switch more than
200 Watt loads with this device, you should definitely test all soldered paths
twice before using it in production.

Relay consist of a low power coil which is opening/closing a circuit going
through the relay. The circuit is normally closed and opens when enough current
is supplied to the coil. So you can decide whether your device will have both
circuits open or closed by default. Or you can have one open and one closed,
that's entirely up to you.

Enough current in our case means more than NodeMCU can supply. That's what we
need the transistors for. The middle leg of each transistor is connected to
a current limiting resistor (~ 1KΩ) which is connected to GPIO. We use D2 and D3
to control relay 1 and relay 2 respectively.

The collector leg of each transistor is connected to 3.3V pin of NodeMCU and
the emitter leg is connected to the coil. Both coils are connected to GND on
the other side.

When you put the digital IO pin (D2 or D3) HIGH, a relatively low current (some
milliamps) flows to the transistor's _gate_. Transistor amplifies the current
flowing from collector to more than 150 mA. And that's high enough for the coil
to flip the switch.

And that's basically it.

# Warranty #
This document comes with ABSOLUTELY NO WARRANTY, to the extent permitted by
applicable law.
