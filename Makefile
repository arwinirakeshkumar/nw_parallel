make: nw-openmp.cpp
	g++ -O2 -fopenmp -DTILESIZE=128 nw-openmp.cpp -o nw-openmp-128
	g++ -O2 -fopenmp -DTILESIZE=256 nw-openmp.cpp -o nw-openmp-256
	g++ -O2 -fopenmp -DTILESIZE=512 nw-openmp.cpp -o nw-openmp-512
	g++ -O2 -fopenmp -DTILESIZE=1024 nw-openmp.cpp -o nw-openmp-1024
	mpic++ -O2 -DTILESIZE=128  core_algo.cpp nw-mpi.cpp -o nw-mpi-128
	mpic++ -O2 -DTILESIZE=256  core_algo.cpp nw-mpi.cpp -o nw-mpi-256
	mpic++ -O2 -DTILESIZE=512  core_algo.cpp nw-mpi.cpp -o nw-mpi-512
	mpic++ -O2 -DTILESIZE=1024 core_algo.cpp nw-mpi.cpp -o nw-mpi-1024
	g++ -O2 -DTILESIZE=1024 nw-sequential.cpp -o nw-sequential

