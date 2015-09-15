#ifndef __CINT__
#include <iostream>
#include <fstream>

#include <TApplication.h>
#include <TString.h>
#include <TSystem.h>

#include <bem.h>
#include <bem2d.h>
#else
#endif

#include "simulate.h"
#include <sys/stat.h>

std::ostream& logstream = cout;

void compute_potential(const std::set<int>& electrodes, const grid_info& info,
		const char* outfile) {

	gSystem->Load("libbemsolver");

	// Import electrodes into world and solve
	
	gSystem->mkdir("gen.cache", 0777);
	TString savedworldname("gen.cache/savedworld.data");

	D3ImportedElectrodes* impel = new D3ImportedElectrodes();
	TString importfilename("layout.dxf");
	if (!impel->Import(importfilename)) {
		return;
	}

	// Configure world, add electrodes
	D3world* wr = new D3world(savedworldname, 0.00001, 32, 6, 6);

	// Number of non-ground electrodes
	const int total_electrodes = 2 * info.num_electrodes + 2;

	// If electrodes is empty, just solve
	for (int i = 0; i < total_electrodes; i++) {
		stringstream stm;
		stm << i;
		wr->insert(&(impel->FindElectrode(stm.str().c_str())));
	}

	wr->insert(&(impel->FindElectrode("GROUND")));

	wr->refine(100);
	wr->correctNorm(0, 0, 0);

	logstream << "Started solving..." << std::endl;
	wr->solve();
	logstream << "Done solving" << std::endl;

	if (electrodes.empty()) {
		return;
	}

	// Calculate potential at grid points

	double xstep = (info.endx - info.startx) / double(info.dimx - 1);
	double ystep = (info.endy - info.starty) / double(info.dimy - 1);
	double zstep = (info.endz - info.startz) / double(info.dimz - 1);

	const int numPoints = info.dimx * info.dimy * info.dimz;

	for (int i = 0; i < total_electrodes; i++) {
		stringstream stm;
		stm << i;
		if (electrodes.count(i)) {
			impel->FindElectrode(stm.str().c_str()).SetVoltage(1);
		} else {
			impel->FindElectrode(stm.str().c_str()).SetVoltage(0);
		}
	}

	impel->FindElectrode("GROUND").SetVoltage(0);

	// Calculate potential on the grid
	logstream << "Writing to '" << outfile << "'" << endl;
	std::ofstream datafile(outfile);
	for (int i = 0; i < info.dimx; i++) {
		logstream << "  Processing point " << i * info.dimy * info.dimz <<
				" out of " << numPoints << std::endl;
		for (int j = 0; j < info.dimy; j++) {
			for (int k = 0; k < info.dimz; k++) {
				double phi = wr->calc_slow(info.startx + i * xstep, info.starty
						+ j * ystep, info.startz + k * zstep);
				datafile << phi << std::endl;
			}
		}
	}
	datafile.close();

	logstream << "Finished processing." << endl;
}
