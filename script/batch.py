import os

benchmarks = ['adder', 'bar', 'max', 'arbiter', 'cavlc', 'ctrl', 'dec', 'i2c', 'int2float', 'priority', 'router']
benchmark_folder = './input/benchmark/'
output_path = './result/test0408-1000/'

if not os.path.exists(output_path):
    os.makedirs(output_path)

for benchmark in benchmarks:
    # comm = './circ_gen.out -i input/benchmark/adder.aig -o result/test0408-1000/ > result/test0408-1000/adder.log'
    comm = './circ_gen.out -i ' + benchmark_folder + benchmark + '.aig -o ' + output_path + ' > ' + output_path + benchmark + '.txt'
    print(comm)
    os.system(comm)