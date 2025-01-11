# SCU DMA tests

This test runs the following SCU DMA transfers on all three levels:

|Test name|Source|Destination|Increments|Length|
|--|--|--|--|--|
|`WRAM->WRAM +4`|High WRAM|High WRAM|+4/+4|12 bytes|
|`WRAM->BBus +4`|High WRAM|VDP1 VRAM|+4/+4|12 bytes|
|`BBus->WRAM +4`|VDP1 VRAM|High WRAM|+4/+4|12 bytes|
|`BBus->BBus +4`|VDP1 VRAM|VDP1 VRAM|+4/+4|12 bytes|
|`WRAM->WRAM +0`|High WRAM|High WRAM|+0/+0|12 bytes|
|`WRAM->BBus +0`|High WRAM|VDP1 VRAM|+0/+0|12 bytes|
|`BBus->WRAM +0`|VDP1 VRAM|High WRAM|+0/+0|12 bytes|
|`BBus->BBus +0`|VDP1 VRAM|VDP1 VRAM|+0/+0|12 bytes|

Expected results for each test (taken from hardware, identical on all levels):
|Test name|Output|
|--|--|
|`WRAM->WRAM +4`|`FFFFFFFFFFFFFFFFFFFFFFFF`|
|`WRAM->BBus +4`|`FFFFFFFFFFFFFFFFFFFFFFFF`|
|`BBus->WRAM +4`|`0405060708090A0BFFFFFFFF`|
|`BBus->BBus +4`|`FFFFFFFFFFFFFFFFFFFFFFFF`|
|`WRAM->WRAM +0`|`FFFFFFFFFFFFFFFFFFFFFFFF`|
|`WRAM->BBus +0`|`0203FFFFFFFFFFFFFFFFFFFF`|
|`BBus->WRAM +0`|`08090A0BFFFFFFFFFFFFFFFF`|
|`BBus->BBus +0`|`FFFFFFFFFFFFFFFFFFFFFFFF`|

More tests and explanations of the test results are TODO.

<!--
## Explanation

TODO
-->
