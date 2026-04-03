# bsp

## Intent
Encapsulate concrete board and physical hardware details.

## Should contain
- access or adaptation to specific board hardware,
- details of pins, buses, devices, or wiring,
- elements that should not leak into higher layers.

## Should NOT contain
- application logic,
- use cases,
- general system services,
- regenerable logic that should clearly remain in `../cube/`.

## Relationship with other folders
`bsp/` knows the board.  
Higher layers should not carry these details directly if they can be abstracted.

## Practical guideline
If the code knows too much about the physical board, it likely belongs here.