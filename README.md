# Greystone

A third-person action game built with Unreal Engine 5.4, featuring a spell-casting combat system with character abilities and dynamic gameplay mechanics.

## Overview

Greystone is an Unreal Engine project that implements a character-based action system with magical abilities, stamina management, and projectile-based combat. The game features a custom ability system with primary and secondary spells, complete with visual effects and player feedback.

Completed tutorial for Paragon Characters:
https://www.youtube.com/watch?v=p-Yzq32ydIw&list=PLmhwY3IQ4Myv9t71ow9jWrkjLIjz9-hO8&index=7

## Next Steps

### Make it an actual Game
- **Add Win/Loss Condition**
- **Fix Sprinting**: Holding down sprint key depletes sprint regardless of actually sprinting
- **Fix Spell Cast**: The Spell cast is spawned based on the camera. This was by design in the tutorial but the spell should spawn based on the actor rotation/location
- **Fix Attacking Animation**: If any animation is triggered that interrupts the basic attack animation, then isAttacking state gets stuck and attacking stops working.
- **Add Sound FX for abilities**: Both Buff and Deflect spell need proper Sound FX for when they are cast and on impact
- **Fix Buff Spell Deleting**: Currently a new Buff blueprint is created per cast. THere needs logic to determine when it gets destroyed
- **Setup what buff actually does**: Buff is just a visual. Add logic to increase attack speed temporarily.

