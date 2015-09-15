#ifndef SIMULATE_H
#define SIMULATE_H

struct grid_info {
	int num_electrodes;

	// Points per axis
	int dimx;
	int dimy;
	int dimz;

	double startx;
	double endx;

	double starty; 
	double endy;

	double startz; 
	double endz;
};

void compute_potential(const std::set<int>& electrodes, const grid_info& info,
		const char* outfile);

#endif
