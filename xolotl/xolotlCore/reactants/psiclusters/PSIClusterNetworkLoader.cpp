/*
 * PSIClusterNetworkLoader.cpp
 *
 *  Created on: Mar 30, 2013
 *      Author: jaybilly
 */

#include <fstream>
#include "PSIClusterNetworkLoader.h"
#include <TokenizedLineReader.h>
#include <HeCluster.h>
#include <VCluster.h>
#include <InterstitialCluster.h>
#include <HeVCluster.h>
#include <PSISuperCluster.h>
// #include <HeInterstitialCluster.h>
#include <PSIClusterReactionNetwork.h>
#include <xolotlPerf.h>
#include <MathUtils.h>
#include <cassert>

using namespace xolotlCore;

/**
 * This operation converts a string to a double, taking in to account the fact
 * that the input file may contain keys such as "infinite."
 *
 * @param inString the string to be converted
 * @return the string as a double
 */
static inline double convertStrToDouble(const std::string& inString) {
	return (inString.compare("infinite") == 0) ?
			std::numeric_limits<double>::infinity() :
			strtod(inString.c_str(), NULL);
}

std::unique_ptr<PSICluster> PSIClusterNetworkLoader::createPSICluster(int numHe,
		int numV, int numI, IReactionNetwork& network) const {

	// Local Declarations
	PSICluster* cluster = nullptr;

	// Determine the type of the cluster given the number of each species.
	// Create a new cluster by that type and specify the names of the
	// property keys.
	if (numHe > 0 && numV > 0) {
		// Create a new HeVCluster
		cluster = new HeVCluster(numHe, numV, network, handlerRegistry);
	} else if (numHe > 0 && numI > 0) {
		throw std::string("HeliumInterstitialCluster is not yet implemented.");
		// FIXME! Add code to add it to the list
	} else if (numHe > 0) {
		// Create a new HeCluster
		cluster = new HeCluster(numHe, network, handlerRegistry);
	} else if (numV > 0) {
		// Create a new VCluster
		cluster = new VCluster(numV, network, handlerRegistry);
	} else if (numI > 0) {
		// Create a new ICluster
		cluster = new InterstitialCluster(numI, network, handlerRegistry);
	}
	assert(cluster != nullptr);

	// TODO Once we have widespread C++14 support, use std::make_unique
	// instead of two steps (and two memory allocations).
	return std::unique_ptr<PSICluster>(cluster);
}

PSIClusterNetworkLoader::PSIClusterNetworkLoader(
		std::shared_ptr<xolotlPerf::IHandlerRegistry> registry) {
	networkStream = nullptr;
	handlerRegistry = registry;
	fileName = "";
	dummyReactions = false;
	vMin = 1000000;

	return;
}

PSIClusterNetworkLoader::PSIClusterNetworkLoader(
		const std::shared_ptr<std::istream> stream,
		std::shared_ptr<xolotlPerf::IHandlerRegistry> registry) {
	networkStream = nullptr;
	handlerRegistry = registry;
	fileName = "";
	dummyReactions = false;
	vMin = 1000000;

	return;
}

std::unique_ptr<IReactionNetwork> PSIClusterNetworkLoader::load(
		const IOptions& options) const {
	// Local Declarations
	TokenizedLineReader<std::string> reader;
	std::vector<std::string> loadedLine;

	// TODO Once we have C++14, use std::make_unique.
	std::unique_ptr<PSIClusterReactionNetwork> network(
			new PSIClusterReactionNetwork(handlerRegistry));

	std::string error(
			"PSIClusterNetworkLoader Exception: Insufficient or erroneous data.");
	int numHe = 0, numV = 0, numI = 0;
	double formationEnergy = 0.0, migrationEnergy = 0.0;
	double diffusionFactor = 0.0;
	std::vector<std::reference_wrapper<Reactant> > reactants;

	// Load the network if the stream is available
	if (networkStream != NULL) {
		// Load the stream
		reader.setInputStream(networkStream);

		// Loop over each line of the file, which should each be PSIClusters.
		loadedLine = reader.loadLine();
		while (loadedLine.size() > 0) {
			// Check the size of the loaded line
			if (loadedLine.size() < 6)
				// And notify the calling function if the size is insufficient
				throw error;
			// Load the sizes
			if (loadedLine[0][0] != '#') {
				numHe = std::stoi(loadedLine[0]);
				numV = std::stoi(loadedLine[1]);
				numI = std::stoi(loadedLine[2]);
				// Create the cluster
				auto nextCluster = createPSICluster(numHe, numV, numI,
						*network);
				// Load the energies
				formationEnergy = convertStrToDouble(loadedLine[3]);
				migrationEnergy = convertStrToDouble(loadedLine[4]);
				diffusionFactor = convertStrToDouble(loadedLine[5]);
				// Set the formation energy
				nextCluster->setFormationEnergy(formationEnergy);
				// Set the diffusion factor and migration energy
				nextCluster->setMigrationEnergy(migrationEnergy);
				nextCluster->setDiffusionFactor(diffusionFactor);

				// Save access to it so we can trigger updates after
				// we add all to the network.
				reactants.emplace_back(*nextCluster);

				// Give the cluster to the network
				network->add(std::move(nextCluster));
			}

			// Load the next line
			loadedLine = reader.loadLine();
		}

		// Update reactants now that they are in network.
		for (IReactant& currCluster : reactants) {
			currCluster.updateFromNetwork();
		}
	}

	// Check if we want dummy reactions
	if (!dummyReactions) {
		// Apply sectional grouping
		applySectionalGrouping(*network);
	}

	// Create the reactions
	network->createReactionConnectivity();

	// Recompute Ids and network size and redefine the connectivities
	network->reinitializeNetwork();

	// Dump the network we've created, if desired.
	auto netDebugOpts = options.getNetworkDebugOptions();
	if (netDebugOpts.first) {
		int rank;
		MPI_Comm_rank(MPI_COMM_WORLD, &rank);
		if (rank == 0) {
			// Dump the network we've created for comparison with baseline.
			std::ofstream networkStream(netDebugOpts.second);
			network->dumpTo(networkStream);
		}
	}

	// Need to use move() because return type uses smart pointer to base class,
	// not derived class that we created.
	// Some C++11 compilers accept it without the move, but apparently
	// that is not correct behavior until C++14.
	return std::move(network);
}

std::unique_ptr<IReactionNetwork> PSIClusterNetworkLoader::generate(
		const IOptions &options, std::vector<std::vector<double> > & padeVector,
		std::map<std::string, int> & idMap) {
	// Initial declarations
	maxI = options.getMaxI(), maxHe = options.getMaxImpurity(), maxV =
			options.getMaxV();
	bool usePhaseCut = options.usePhaseCut();
	int numHe = 0, numV = 0, numI = 0;
	double formationEnergy = 0.0, migrationEnergy = 0.0;
	double diffusionFactor = 0.0;

	// Once we have C++14, use std::make_unique.
	std::unique_ptr<PSIClusterReactionNetwork> network(
			new PSIClusterReactionNetwork(handlerRegistry));
	std::vector<std::reference_wrapper<Reactant> > reactants;

	// I formation energies in eV
	std::vector<double> iFormationEnergies = { 0.0 };
	// I diffusion factors in nm^2/s
	std::vector<double> iDiffusion = { 1.0e+11 };
	// I migration energies in eV
	std::vector<double> iMigration = { 0.34 };

	// He formation energies in eV
	std::vector<double> heFormationEnergies = { 0.0 };
	// He diffusion factors in nm^2/s
	std::vector<double> heDiffusion = { 1.0e+11, 5.0e+10, 3.3e+10 };
	// He migration energies in eV
	std::vector<double> heMigration = { 0.06, 0.06, 0.06 };

	// V formation energies in eV
	std::vector<double> vFormationEnergies = { 0.0 };
	// V diffusion factors in nm^2/s
	std::vector<double> vDiffusion = { 1.0e+11, 5.0e+10, 3.3e+10, 2.5e+10 };
	// V migration energies in eV
	std::vector<double> vMigration = { 0.67, 0.62, 0.37, 0.48 };

	// Generate the I clusters
	for (int i = 1; i <= maxI; ++i) {
		// Set the composition
		numI = i;
		// Create the cluster
		auto nextCluster = createPSICluster(numHe, numV, numI, *network);

		// Set the other attributes
//		if (i <= iFormationEnergies.size())
//			nextCluster->setFormationEnergy(0.0);
//		else
//			nextCluster->setFormationEnergy(0.0);
		if (i <= iDiffusion.size()) {
			nextCluster->setDiffusionFactor(iDiffusion[i - 1]);
			nextCluster->setMigrationEnergy(iMigration[i - 1]);
		} else {
			nextCluster->setDiffusionFactor(0.0);
			nextCluster->setMigrationEnergy(
					std::numeric_limits<double>::infinity());
		}

		// Save access to it so we can trigger updates once all are
		// added to the network.
		reactants.emplace_back(*nextCluster);

		// Give the cluster to the network
		network->add(std::move(nextCluster));
	}

	// Reset the I composition
	numI = 0;

	// Generate the He clusters
	for (int i = 1; i <= 8; ++i) {
		// Set the composition
		numHe = i;
		// Create the cluster
		auto nextCluster = createPSICluster(numHe, numV, numI, *network);

		// Set the other attributes
//		nextCluster->setFormationEnergy(0.0);
		if (i <= heDiffusion.size()) {
			nextCluster->setDiffusionFactor(heDiffusion[i - 1]);
			nextCluster->setMigrationEnergy(heMigration[i - 1]);
		} else {
			nextCluster->setDiffusionFactor(0.0);
			nextCluster->setMigrationEnergy(
					std::numeric_limits<double>::infinity());
		}

		// Save access to it so we can trigger updates once
		// added to the network.
		reactants.emplace_back(*nextCluster);

		// Give the cluster to the network
		network->add(std::move(nextCluster));
	}

	// Reset the He composition
	numHe = 0;

	// Loop over vacancies in the outer loop.
	// This creates V and HeV up to the maximum size in the
	// maxHePerV array.
	for (int i = 1; i <= maxV; ++i) {
		// Create the V cluster
		numV = i;
		if (numV < 11) {
			auto nextCluster = createPSICluster(numHe, numV, numI, *network);

			// Set its other attributes
//			if (i <= vFormationEnergies.size())
//				nextCluster->setFormationEnergy(vFormationEnergies[i - 1]);
//			else
//				nextCluster->setFormationEnergy(getHeVFormationEnergy(0, i));
			if (i <= vDiffusion.size()) {
				nextCluster->setDiffusionFactor(vDiffusion[i - 1]);
				nextCluster->setMigrationEnergy(vMigration[i - 1]);
			} else {
				nextCluster->setDiffusionFactor(0.0);
				nextCluster->setMigrationEnergy(
						std::numeric_limits<double>::infinity());
			}

			// Save access to it so we can trigger updates once
			// all are added to the network.
			reactants.emplace_back(*nextCluster);

			// Give the cluster to the network
			network->add(std::move(nextCluster));
		}

		// Loop on the helium number
		for (int j = 1; j <= maxHe; j++) {
			numHe = j;
			// Create the cluster only if it is not going to be grouped
			if (numHe < vMin && numV < vMin) {
				auto nextCluster = createPSICluster(numHe, numV, numI,
						*network);
				// Set its attributes
//				nextCluster->setFormationEnergy(0.0);
				nextCluster->setDiffusionFactor(0.0);
				nextCluster->setMigrationEnergy(
						std::numeric_limits<double>::infinity());

				// Save access to it so we can trigger updates once
				// all are added to the network.
				reactants.emplace_back(*nextCluster);

				// Give the cluster to the network
				network->add(std::move(nextCluster));
			}
		}

		// Reset the helium composition
		numHe = 0;
	}

	// Update reactants now that they are in network.
	for (IReactant& currCluster : reactants) {
		currCluster.updateFromNetwork();
	}

	// Check if we want dummy reactions
	if (!dummyReactions) {
		// Apply sectional grouping
		applySectionalGrouping(*network);
	}

	// Set the pade coefficients
//	network->setPade(padeVector, idMap);

	// Create the reactions
	network->createReactionConnectivity();

	// Recompute Ids and network size and redefine the connectivities
	network->reinitializeNetwork();

	// Dump the network we've created, if desired.
	auto netDebugOpts = options.getNetworkDebugOptions();
	if (netDebugOpts.first) {
		int rank;
		MPI_Comm_rank(MPI_COMM_WORLD, &rank);
		if (rank == 0) {
			// Dump the network we've created for comparison with baseline.
			std::ofstream networkStream(netDebugOpts.second);
			network->dumpTo(networkStream);
		}
	}

	// Need to use move() because return type uses smart pointer to base class,
	// not derived class that we created.
	// Some C++11 compilers accept it without the move, but apparently
	// that is not correct behavior until C++14.
	return std::move(network);
}

double PSIClusterNetworkLoader::getHeVFormationEnergy(int numHe, int numV) {
	// Coefficients for the Legendre polynomial fit
	// Low means V <= 27
	// Coefficients for c_0 in the 2D E_f,HeV fit
	std::vector<double> c0CoefficientsLow = { 253.35, 435.36, 336.50, 198.92,
			95.154, 21.544 };
	// Coefficients for c_1 in the 2D E_f,HeV fit
	std::vector<double> c1CoefficientsLow = { 493.29, 1061.3, 1023.9, 662.92,
			294.24, 66.962 };
	// Coefficients for c_2 in the 2D E_f,HeV fit
	std::vector<double> c2CoefficientsLow = { 410.40, 994.89, 1044.6, 689.41,
			286.52, 60.712 };
	// Coefficients for c_3 in the 2D E_f,HeV fit
	std::vector<double> c3CoefficientsLow = { 152.99, 353.16, 356.10, 225.75,
			87.077, 15.640 };
	// High means V > 27
	// Coefficients for c_0 in the 2D E_f,HeV fit
	std::vector<double> c0CoefficientsHigh = { -847.90, -3346.9, -4510.3,
			-3094.7, -971.18, -83.770 };
	// Coefficients for c_1 in the 2D E_f,HeV fit
	std::vector<double> c1CoefficientsHigh = { -1589.3, -4894.6, -6001.8,
			-4057.5, -1376.4, -161.91 };
	// Coefficients for c_2 in the 2D E_f,HeV fit
	std::vector<double> c2CoefficientsHigh = { 834.91, 1981.8, 1885.7, 1027.1,
			296.69, 29.902 };
	// Coefficients for c_3 in the 2D E_f,HeV fit
	std::vector<double> c3CoefficientsHigh = { 1547.2, 3532.3, 3383.6, 1969.2,
			695.17, 119.23 };

	/**
	 * The formation energies for He_xV_1. The entry at i = 0 is for a single
	 * vacancy (He_0V_1) and is there as a buffer. Like the formation energies,
	 * i = heSize.
	 */
	std::vector<double> heV1FormationEnergies = { 5.14166, 8.20919, 11.5304,
			14.8829, 18.6971, 22.2847, 26.3631, 30.1049, 34.0081, 38.2069,
			42.4217, 46.7378, 51.1551, 55.6738 };

	/**
	 * The formation energies for He_xV_2. The entry at i = 0 is for a
	 * di-vacancy (He_0V_2) and is there as a buffer. Like the formation
	 * energies, i = heSize.
	 */
	std::vector<double> heV2FormationEnergies =
			{ 7.10098, 8.39913, 9.41133, 11.8748, 14.8296, 17.7259, 20.7747,
					23.7993, 26.7984, 30.0626, 33.0385, 36.5173, 39.9406, 43.48,
					46.8537, 50.4484, 54.0879, 57.7939 };

	// Initial declarations
	double energy = -std::numeric_limits<double>::infinity();
	// The following coefficients are computed using the above and are used
	// to evaluate the full function f(x,y).
	std::vector<double> coefficients = { 0.0, 0.0, 0.0, 0.0 };

	// Check to see if the vacancy size is large enough that the energy can
	// be computed from the fit or if it is so small that the exact values
	// must be used instead.
	if (numV > 2) {
		// Get the He/V ratio
		double x = 2.0 * (((double) numHe / (double) numV) / 9.0) - 1.0;
		// Initialize the vacancy number
		double y = 0.0;

		// We have 2 fits, one for low V and one for high V
		if (numV <= 27) {
			// Compute the vacancy number
			y = 2.0 * (((double) numV - 1.0) / 26.0) - 1.0;
			// Get the coefficients
			coefficients[0] = compute5thOrderLegendre(x, c0CoefficientsLow);
			coefficients[1] = compute5thOrderLegendre(x, c1CoefficientsLow);
			coefficients[2] = compute5thOrderLegendre(x, c2CoefficientsLow);
			coefficients[3] = compute5thOrderLegendre(x, c3CoefficientsLow);
		} else {
			// Compute the vacancy number
			y = 2.0 * (((double) numV - 1.0) / 451.0) - 1.0;
			// Get the coefficients
			coefficients[0] = compute5thOrderLegendre(x, c0CoefficientsHigh);
			coefficients[1] = compute5thOrderLegendre(x, c1CoefficientsHigh);
			coefficients[2] = compute5thOrderLegendre(x, c2CoefficientsHigh);
			coefficients[3] = compute5thOrderLegendre(x, c3CoefficientsHigh);
		}
		// Get the energy
		energy = compute3rdOrderLegendre(y, coefficients);

	} else if ((numV == 1 && numHe <= heV1FormationEnergies.size())
			|| (numV == 2 && numHe <= heV2FormationEnergies.size())) {
		// Get the exact energy
		energy =
				(numV == 1) ?
						heV1FormationEnergies[numHe - 1] :
						heV2FormationEnergies[numHe - 1];
	}

	return energy;
}

void PSIClusterNetworkLoader::applySectionalGrouping(
		PSIClusterReactionNetwork& network) const {

	// Create a temporary vector for the loop
	std::vector<std::pair<int, int> > tempVector;

	// Initialize variables for the loop
	int count = 0;
	double heSize = 0.0, vSize = 0.0;

	// Loop on the vacancy groups
	for (int k = 0; k < vSectionBounds.size() - 1; k++) {

		// Loop on the helium groups
		for (int j = 0; j < heSectionBounds.size() - 1; j++) {
			// To check if the group is full
			int heLow = maxHe, heHigh = -1, vLow = maxV, vHigh = -1;

			// Loop within the group
			for (int n = vSectionBounds[k]; n < vSectionBounds[k + 1]; n++) {
				if (n > maxV)
					continue;
				for (int m = heSectionBounds[j]; m < heSectionBounds[j + 1];
						m++) {

					// Skip the ungrouped
					if (m < vMin && n < vMin)
						continue;

					// Get the corresponding cluster
					auto pair = std::make_pair(m, n);

					// Will be used to know if the group was full
					if (m < heLow)
						heLow = m;
					if (m > heHigh)
						heHigh = m;
					if (n < vLow)
						vLow = n;
					if (n > vHigh)
						vHigh = n;

					// Increment the counter
					count++;

					// Add this cluster to the temporary vector
					tempVector.push_back(pair);
					heSize += (double) m;
					vSize += (double) n;
				}
			}

			if (count == 0)
				continue;

			// Average all values
			heSize = heSize / (double) count;
			vSize = vSize / (double) count;

			// Create the super cluster
			PSISuperCluster* rawSuperCluster = nullptr;
			rawSuperCluster = new PSISuperCluster(heSize, vSize, count,
					heHigh - heLow + 1, vHigh - vLow + 1, network,
					handlerRegistry);

//			std::cout << rawSuperCluster->getName() << " " << heHigh - heLow + 1
//					<< " " << vHigh - vLow + 1 << std::endl;

			assert(rawSuperCluster != nullptr);
			auto superCluster = std::unique_ptr<PSISuperCluster>(
					rawSuperCluster);

			// Save access to the cluster so we can trigger updates
			// after we give it to the network.
			auto& scref = *superCluster;

			// Give the cluster to the network.
			network.add(std::move(superCluster));

			// Trigger cluster updates now it is in the network.
			scref.updateFromNetwork();

			// Set the HeV vector
			scref.setHeVVector(tempVector);

			// Reinitialize everything
			heSize = 0.0, vSize = 0.0;
			count = 0;
			tempVector.clear();
		}
	}

	// Now that we have the bound vector defined, tell the network to
	// build its lookup map for super clusters
	network.buildSuperClusterMap(heSectionBounds, vSectionBounds);

	return;
}
