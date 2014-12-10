#ifndef SOLVERHANDLER_H
#define SOLVERHANDLER_H

// Includes
#include "ISolverHandler.h"

namespace xolotlSolver {

/**
 * This class realizes the ISolverHandler interface to solve the
 * advection-diffusion-reaction problem with the PETSc solvers from Argonne
 * National Laboratory.
 *
 * This class does NOT implement most of the methods that are needed by the
 * PetscSolver. Only subclasses of this class must be used by the PetscSolver.
 */
class SolverHandler: public ISolverHandler {
protected:

	//! The name of the network file
	std::string networkName;

	//! The original network created from the network loader.
	xolotlCore::PSIClusterReactionNetwork *network;

	//! The grid step size.
	double h;

	//! The initial vacancy concentration.
	double initialVConc;

	//! The surface position index.
	int surfacePos;

	//! The void portion.
	double voidPortion;

	//! The original flux handler created.
	xolotlCore::IFluxHandler *fluxHandler;

	//! The original temperature handler created.
	xolotlCore::ITemperatureHandler *temperatureHandler;

	//! The original diffusion handler created.
	xolotlCore::IDiffusionHandler *diffusionHandler;

	//! The original advection handler created.
	xolotlCore::IAdvectionHandler *advectionHandler;

	//! The original advection handler created.
	xolotlCore::IBubbleBurstingHandler *bubbleBurstingHandler;

public:

	/**
	 * Initialize all the physics handlers that are needed to solve the ADR equations.
     * \see ISolverHandler.h
	 */
	void initializeHandlers(std::shared_ptr<xolotlFactory::IMaterialFactory> material,
			std::shared_ptr<xolotlCore::ITemperatureHandler> tempHandler,
			xolotlCore::Options &options) {

		// Set the flux handler
		fluxHandler = (xolotlCore::IFluxHandler *) material->getFluxHandler().get();

		// Set the temperature handler
		temperatureHandler = (xolotlCore::ITemperatureHandler *) tempHandler.get();

		// Set the diffusion handler
		diffusionHandler = (xolotlCore::IDiffusionHandler *) material->getDiffusionHandler().get();

		// Set the advection handler
		advectionHandler = (xolotlCore::IAdvectionHandler *) material->getAdvectionHandler().get();

		// Set the bubble bursting handler
		bubbleBurstingHandler = (xolotlCore::IBubbleBurstingHandler *) material->getBubbleBurstingHandler().get();

		// Set the grid step size
		h = options.getStepSize();

		// Set the initial vacancy concentration
		initialVConc = options.getInitialVConcentration();

		// Set the initial void portion
		voidPortion = options.getVoidPortion();

		return;
	}

	/**
	 * Initialize the network and network file name.
	 * \see ISolverHandler.h
	 */
	void initializeNetwork(std::string fileName,
			xolotlCore::PSIClusterReactionNetwork *net) {

		// Set the network loader
		networkName = fileName;

		// Set the network
		network = net;

		return;
	}

	/**
	 * Get the step size.
	 * \see ISolverHandler.h
	 */
	double getStepSize() const {return h;}

	/**
	 * Get the surface position.
	 * \see ISolverHandler.h
	 */
	int getSurfacePosition() const {return surfacePos;}

	/**
	 * Set the surface position.
	 * \see ISolverHandler.h
	 */
	int setSurfacePosition(int index) {surfacePos = index;}

	/**
	 * Get the flux handler.
	 * \see ISolverHandler.h
	 */
	xolotlCore::IFluxHandler *getFluxHandler() const {return fluxHandler;}

	/**
	 * Get the diffusion handler.
	 * \see ISolverHandler.h
	 */
	xolotlCore::IDiffusionHandler *getDiffusionHandler() const {return diffusionHandler;}

	/**
	 * Get the bubble bursting handler.
	 * \see ISolverHandler.h
	 */
	xolotlCore::IBubbleBurstingHandler *getBubbleBurstingHandler() const {return bubbleBurstingHandler;}

	/**
	 * Get the network.
	 * \see ISolverHandler.h
	 */
	xolotlCore::PSIClusterReactionNetwork *getNetwork() const {return network;}



}; //end class SolverHandler

} /* end namespace xolotlSolver */
#endif
