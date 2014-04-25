#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Regression

#include <boost/test/included/unit_test.hpp>
#include <PSIClusterNetworkLoader.h>
#include <memory>
#include <typeinfo>
#include <limits>
#include <PSIClusterNetworkLoader.h>
#include <PSIClusterReactionNetwork.h>
#include <XolotlConfig.h>
#include <DummyHandlerRegistry.h>

using namespace std;
using namespace xolotlCore;

/**
 * The test suite configuration
 */BOOST_AUTO_TEST_SUITE (TungstenIntegrationTester_testSuite)

/**
 * This operation checks the fluxs from the reactant as best as is possible
 * given that it requires external data.
 */
BOOST_AUTO_TEST_CASE(checkGetReactantFluxes) {
	// Local Declarations
	string sourceDir(XolotlSourceDirectory);
	string pathToFile("/tests/reactants/testfiles/tungsten.txt");
	string networkFilename = sourceDir + pathToFile;

	BOOST_TEST_MESSAGE(
			"TungstenIntegrationTester Message: Network filename is: " << networkFilename);

	// Load the input file from the master task
	shared_ptr<istream> networkStream = make_shared<ifstream>(networkFilename);

	// Create a network loader and set the istream on every MPI task
	shared_ptr<PSIClusterNetworkLoader> networkLoader = make_shared<
			PSIClusterNetworkLoader>(
			std::make_shared<xolotlPerf::DummyHandlerRegistry>());
	networkLoader->setInputstream(networkStream);
	// Load the network
	shared_ptr<ReactionNetwork> network = networkLoader->load();

	BOOST_TEST_MESSAGE("TungstenIntegrationTester Message: Network loaded");

	int nReactants = network->size();
	auto reactants = network->getAll();

	BOOST_TEST_MESSAGE(
			"TungstenIntegrationTester Message: Size of the network is: " << nReactants);

	for (int i = 0; i < nReactants; ++i) {
		shared_ptr<PSICluster> reactant = dynamic_pointer_cast<PSICluster>(
				reactants->at(i));
		double flux = reactant->getTotalFlux(273.0);
		auto partials = reactant->getPartialDerivatives(273.0);
	}

}
BOOST_AUTO_TEST_SUITE_END()
