# Spec Delta

## ADDED Requirements

### Requirement: Delay Tap Display
WvZz Delay SHALL show a delay-tap display as its custom center tab: a time axis with one marker per repeat for the left and the right channel, where marker height follows the feedback decay. The display SHALL read the delay state through lock-free atomics published by the audio thread.

#### Scenario: Taps follow time and feedback
- **WHEN** the delay time or feedback changes
- **THEN** the markers move to the new repeat times and their heights follow the new feedback

#### Scenario: Ping-pong shows alternating sides
- **WHEN** `dly_routing` is `Ping-Pong`
- **THEN** the markers alternate between the left lane and the right lane

### Requirement: WvZz Delay Editor Layout
The WvZz Delay editor SHALL use the shared `1080 x 680` shell with the header toolbar and one card per module: Delay in the left column, the modulation and monitor card in the center, and Grain and Master in the right column.

#### Scenario: Editor opens at the base size
- **WHEN** the editor is created
- **THEN** its size is `1080 x 680` and it shows the Delay, Grain, center, and Master cards and exactly one delay-tap display
