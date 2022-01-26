# Rand
This directory houses the [Engine's](../readme.md) random number generation 
features. There are two types of randomness in [Videogame](../../../readme.md):
1. [Randomness](#sigma-checks) about whether an action occurs (not meant to be 
manipulated)
2. [Randomness](#pseudo-randomness) about generating an integer (meant to be 
manipulated by skilled players)

## Sigma Checks
The game determines whether some action will occur with a sigma check. As the 
name might imply, a sigma check tells the game to generate a variable on a 
normal distribution and compare it with another value: the required value of 
sigma to pass the check.

Whether the check passes is simple: if we generate a higher number than required
or we generate a number with an absolute value greater than or equal to ten, we
pass the check.

Sigma checks will be used in game to determine if the engine behaves "normally" 
and if pass / fail checks indeed, pass or fail. 

For example, say the game has already determined that the player will immenently
receive a fatal blow. The game will then roll a sigma-check against -5 (this 
value almost always succeeds). If the game somehow rolled a -5.1, then the 
player would not get hit.

As a design method, the game should find some creative way of telling the 
player when a sigma check fails. Using the above hypothetical, the game would
give some message like:

> The enemy's blow dealt 500 damage. <br>
> You died. <br>
>
> However. Through divine intervention, you survived? The world is a 
> strange place.

## Pseudo Randomness
This method of RNG is inspired by the pivotal videogame DOOM. DOOM's RNG consists
of a simple lookup table. [Videogame's](../../../readme.md) PRNG will also consist
of a lookup table. However, unlike in DOOM, this PRNG will have an unpredictable
seed, using hardware entropy if available, and will reseed itself when it repeats.

This form of RNG will be used to determine more numerical components to the game,
such as damage, loot, etc.