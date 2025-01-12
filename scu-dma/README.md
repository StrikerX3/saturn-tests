# SCU DMA tests

This test runs SCU DMA transfers on all three levels between WRAM and B-Bus (VDP1 VRAM).

The following transfer parameters are tweakable:
- (up/down) Source and destination strides
- (left/right) Source and destination offset in bytes (from 0 to 3), to test unaligned transfers
- (B/C) Transfer length from 1 to 32 bytes

Press A to toggle between tweaking the source and the destination parameters.

Additionally, you can now navigate through the rest of the output buffers one byte at a time with L/R, page by page with X/Z or go back to the start with Y.

## Hardware testing

Results from hardware tests taken from an earlier version of the test app:
- Same-bus transfers are completely ignored (buffers are not touched, filled with `FF`s)
- All DMA levels behave exactly the same

|Test name|Output|
|--|--|
|`WRAM->BBus +4`|`FFFFFFFFFFFFFFFFFFFFFFFF`|
|`BBus->WRAM +4`|`0405060708090A0BFFFFFFFF`|
|`WRAM->BBus +0`|`0203FFFFFFFFFFFFFFFFFFFF`|
|`BBus->WRAM +0`|`08090A0BFFFFFFFFFFFFFFFF`|

More tests and explanations of the test results are TODO.

<!--
## Explanation

TODO
-->
