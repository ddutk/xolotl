// Includes
#include "HeCluster.h"
#include "PSIClusterReactionNetwork.h"
#include <Constants.h>
#include <iostream>

using namespace xolotlCore;

HeCluster::HeCluster(int nHe) :
		PSICluster(nHe) {
	// Set the reactant name appropriately
	name = "He";
}

HeCluster::~HeCluster() {
}

std::shared_ptr<Reactant> HeCluster::clone() {
	std::shared_ptr<Reactant> reactant(new HeCluster(*this));
	return reactant;
}

void HeCluster::createReactionConnectivity() {

	// Local Declarations - Note the reference to the properties map
	auto psiNetwork = std::dynamic_pointer_cast < PSIClusterReactionNetwork
			> (network);
	std::map<std::string, std::string> props = psiNetwork->getProperties();
	int thisIndex, indexOther, otherNumHe, otherNumV, otherNumI, networkSize =
			psiNetwork->size();
	int maxHeClusterSize = std::stoi(props["maxHeClusterSize"]);
	int maxVClusterSize = std::stoi(props["maxVClusterSize"]);
	int maxHeVClusterSize = std::stoi(props["maxHeVClusterSize"]);
	int maxHeIClusterSize = std::stoi(props["maxHeIClusterSize"]);
	int numHeVClusters = std::stoi(props["numHeVClusters"]);
	int numHeIClusters = std::stoi(props["numHeIClusters"]);
	int numIClusters = std::stoi(props["numIClusters"]);
	int totalSize = 1, firstSize = 0, secondSize = 0;
	int firstIndex = -1, secondIndex = -1, reactantVecSize = 0;
	std::map<std::string, int> composition;
	std::shared_ptr<PSICluster> psiCluster, firstReactant, secondReactant,
			productReactant;

	std::cout << std::endl << "Reactant Column: " << this->size << this->name << std::endl;

	// Reactants react with themselves, thus the connectivity of this reactant with itself is 1
	thisIndex = network->getReactantId(*this) - 1;
	reactionConnectivity[thisIndex] = 1;
	std::cout << this->size << this->name << ": " << "reactionConnectivity["<< thisIndex
			<< "] = " << reactionConnectivity[thisIndex] << std::endl;

	/*
	 * This section fills the array of reacting pairs that combine to produce
	 * this cluster. The only reactions that produce He clusters are those He
	 * clusters that are smaller than this.size. Each cluster i combines with
	 * a second cluster of size this.size - i.size.
	 *
	 * Total size starts with a value of one so that clusters of size one are
	 * not considered in this loop.
	 */
	while (totalSize < size) {
		// Increment the base sizes
		++firstSize;
		secondSize = size - firstSize;
		// Get the first and second reactants for the reaction
		// first + second = this.
		firstReactant = std::dynamic_pointer_cast < PSICluster
				> (psiNetwork->get("He", firstSize));
		secondReactant = std::dynamic_pointer_cast < PSICluster
				> (psiNetwork->get("He", secondSize));
		// Create a ReactingPair with the two reactants
		if (firstReactant && secondReactant) {
			ReactingPair pair;
			pair.first = firstReactant;
			pair.second = secondReactant;
			// Add the pair to the list
			reactingPairs.push_back(pair);
		}
		// Update the total size. Do not delete this or you'll have an infinite
		// loop!
		totalSize = firstSize + secondSize;
	}

	/* ----- He_a + He_b --> He_(a+b) -----
	 * This cluster should interact with all other clusters of the same type up
	 * to the max size minus the size of this one to produce larger clusters.
	 *
	 * All of these clusters are added to the set of combining reactants
	 * because they contribute to the flux due to combination reactions.
	 */
	auto reactants = psiNetwork->getAll("He");
	combineClusters(reactants,maxHeClusterSize,"He");

	/* -----  He_a + V_b --> (He_a)(V_b) -----
	 * Helium clusters can interact with any vacancy cluster so long as the sum
	 * of the number of helium atoms and vacancies does not produce a cluster
	 * with a size greater than the maximum mixed-species cluster size.
	 *
	 * All of these clusters are added to the set of combining reactants
	 * because they contribute to the flux due to combination reactions.
	 */
	reactants = psiNetwork->getAll("V");
	combineClusters(reactants,maxHeVClusterSize,"HeV");

	/* ----- He_a + I_b --> (He_a)(I_b)
	 * Helium clusters can interact with any interstitial cluster so long as
	 * the sum of the number of helium atoms and interstitials does not produce
	 * a cluster with a size greater than the maximum mixed-species cluster
	 * size.

	 * All of these clusters are added to the set of combining reactants
	 * because they contribute to the flux due to combination reactions.
	 */
	reactants = psiNetwork->getAll("I");
	combineClusters(reactants,maxHeIClusterSize,"HeI");

	/* ----- He_a + (He_b)(V_c) --> [He_(a+b)](V_c) -----
	 * Helium can interact with a mixed-species cluster so long as the sum of
	 * the number of helium atoms and the size of the mixed-species cluster
	 * does not exceed the maximum mixed-species cluster size.
	 *
	 * All of these clusters are added to the set of combining reactants
	 * because they contribute to the flux due to combination reactions.
	 *
	 * Find the clusters by looping over all size combinations of HeV clusters.
	 */
	if (numHeVClusters > 0) {
		reactants = psiNetwork->getAll("HeV");
		combineClusters(reactants,maxHeVClusterSize,"HeV");
	}

	/* ----- He_a + (He_b)(I_c) --> [He_(a+b)](I_c) -----
	 * Helium-interstitial clusters can absorb single-species helium clusters
	 * so long as the maximum cluster size limit is not violated.
	 *
	 * All of these clusters are added to the set of combining reactants
	 * because they contribute to the flux due to combination reactions.
	 *
	 * Find the clusters by looping over all size combinations of HeI clusters.
	 */
	if (numHeIClusters > 0) {
		reactants = psiNetwork->getAll("HeI");
		combineClusters(reactants,maxHeIClusterSize,"HeI");
	}

	return;
}

void HeCluster::createDissociationConnectivity() {

	// Local Declarations
	std::shared_ptr<Reactant> smallerHeReactant, singleHeReactant;

	// He dissociation
	smallerHeReactant = network->get("He", size - 1);
	singleHeReactant = network->get("He", 1);
	if (smallerHeReactant && singleHeReactant) {
		// Add the two reactants to the set. He has very simple dissociation
		// rules.
		id = network->getReactantId(*smallerHeReactant);
		dissociationConnectivity[id] = 1;
		id = network->getReactantId(*singleHeReactant);
		dissociationConnectivity[id] = 1;
	}

	return;
}

bool HeCluster::isProductReactant(const Reactant & reactantI,
		const Reactant & reactantJ) {

	// Local Declarations, integers for species number for I, J reactants
	int rI_I = 0, rJ_I = 0, rI_He = 0, rJ_He = 0, rI_V = 0, rJ_V = 0;

	// Get the compositions of the reactants
	auto reactantIMap = reactantI.getComposition();
	auto reactantJMap = reactantJ.getComposition();

	// Grab the numbers for each species
	// from each Reactant
	rI_I = reactantIMap["I"];
	rJ_I = reactantJMap["I"];
	rI_He = reactantIMap["He"];
	rJ_He = reactantJMap["He"];
	rI_V = reactantIMap["V"];
	rJ_V = reactantJMap["V"];

	// We should have no interstitials, a
	// total of size Helium, and a total of
	// 0 Vacancies
	return ((rI_I + rJ_I) == 0) && ((rI_He + rJ_He) == size)
			&& ((rI_V + rJ_V) == 0);
}

std::map<std::string, int> HeCluster::getClusterMap() {
	// Local Declarations
	std::map<std::string, int> clusterMap;

	// Set the number of each species
	clusterMap["He"] = size;
	clusterMap["V"] = 0;
	clusterMap["I"] = 0;

	// Return it
	return clusterMap;
}

std::map<std::string, int> HeCluster::getComposition() const {
	// Local Declarations
	std::map<std::string, int> clusterMap;

	// Set the number of each species
	clusterMap["He"] = size;
	clusterMap["V"] = 0;
	clusterMap["I"] = 0;

	// Return it
	return clusterMap;
}

double HeCluster::getReactionRadius() {
	double FourPi = 4.0 * xolotlCore::pi;
	double aCubed = pow(xolotlCore::latticeConstant, 3);
	double termOne = pow((3.0 / FourPi) * (1.0 / 10.0) * aCubed * size,
			(1.0 / 3.0));
	double termTwo = pow((3.0 / FourPi) * (1.0 / 10.0) * aCubed, (1.0 / 3.0));
	return 0.3 + termOne - termTwo;
}