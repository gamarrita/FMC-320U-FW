# app

## Intent
Contain application logic and product use cases.

## Should contain
- functional behavior,
- coordination between services,
- decisions visible at the product or feature level.

## Should NOT contain
- board-specific details,
- platform adaptation,
- regenerable logic,
- direct hardware access except in well-justified cases.

## Relationship with other folders
`app/` expresses what the system does.  
Ideally, it consumes capabilities from lower layers without absorbing unnecessary physical details.

## Practical guideline
If the code answers what the product does, it likely belongs here.