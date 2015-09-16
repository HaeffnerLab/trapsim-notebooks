#include "simulate.h"

// XXX: Important note: you can only run this in parallel if the solution cache
// already exists!

const char* FIELD_PREFIX = "field";
const char* FIELD_SUFFIX = ".txt";

void compute(int start, int stop) {

	template<typename T>
	T parse_param(std::ifstream& in) {
		std::string line;
		std::getline(in, line);

		// Get stuff to right of :
		size_t idx = line.find(':') + 1;
		std::string rhs = line.substr(idx);

		// Trim whitespace
		size_t begin = line.find_first_not_of(" \t");
		size_t end = line.find_last_not_of(" \t");
		std::string strval = rhs.substr(begin, end - begin + 1);

		// Read from string
		std::stringstream stm(strval);
		T val;
		stm >> val;

		return val;
	}

	// ROOT seems unable to run a YAML parser, so brute force it
	grid_info read_grid_info(const char* f) {
		grid_info info;

		std::ifstream in(f);
		std::string dummy;
		// ---
		std::getline(in, dummy);
		// num_electrodes: -
		info.num_electrodes = parse_param<int>(in);
		// dimx: -
		info.dimx = parse_param<int>(in);
		info.dimy = parse_param<int>(in);
		info.dimz = parse_param<int>(in);
		// startx: -
		info.startx = parse_param<double>(in);
		info.starty = parse_param<double>(in);
		info.startz = parse_param<double>(in);
		// endx: -
		info.endx = parse_param<double>(in);
		info.endy = parse_param<double>(in);
		info.endz = parse_param<double>(in);

		return info;
	};

	gROOT->ProcessLine(".L ../simulate.cpp");

	grid_info info = read_grid_info("grid.txt");

	if (start == stop) {
		cout << "Writing solve cache" << endl;

		std::set<int> electrodes;
		compute_potential(electrodes, info, NULL);
	} else {
		for (int i = start; i < stop; i++) {
			cout << "Starting on electrode " << i << endl;

			std::set<int> electrodes;
			electrodes.insert(i);
			if (i > 0) {
				electrodes.insert(info.num_electrodes + i);
			}

			stringstream stm;
			stm << FIELD_PREFIX << i << FIELD_SUFFIX;
			outfile = stm.str();

			compute_potential(electrodes, info, outfile.c_str());
		}
	}
}
