1. GenMul: unsigned, wallace tree, RCA

2. yosys
read_verilog xxx.v; synth; write_blif -top xxxx -impltf xxx.blif;

3. abc
read xxx.blif; strash; write xxx.aig;