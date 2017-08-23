#ifndef PSI_CLUSTER_REACTION_NETWORK_H
#define PSI_CLUSTER_REACTION_NETWORK_H

// Includes
//#include <xolotlPerf.h>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <unordered_map>
#include <ReactionNetwork.h>
#include <iostream>
#include <iomanip>
#include <algorithm>

namespace xolotlCore {

/**
 *  This class manages the set of reactants and compound reactants (
 *  combinations of normal reactants) for PSI clusters. It also manages a
 *  set of properties that describes the total collection.
 *
 *  This class is a very heavyweight class that should not be abused.
 *
 *  Reactants that are added to this network must be added as with shared_ptrs.
 *  Furthermore, reactants that are added to this network have their ids set to
 *  a network specific id. Reactants should not be shared between separate
 *  instances of a PSIClusterReactionNetwork.
 */
class PSIClusterReactionNetwork: public ReactionNetwork {

private:

	/**
	 * The map of single-species clusters, indexed by a string representation
	 * of a map that contains the name of the reactant and its size.
	 */
	std::unordered_map<std::string, std::shared_ptr<IReactant> > singleSpeciesMap;

	/**
	 * The map of mixed or compound species clusters, indexed by a string
	 * representation of a map that contains the name of the constituents
	 * of the compound reactant and their sizes.
	 */
	std::unordered_map<std::string, std::shared_ptr<IReactant> > mixedSpeciesMap;

	/**
	 * The map of super species clusters, indexed by a map that
	 * contains the name of the constituents of the compound reactant and their
	 * sizes.
	 */
	std::unordered_map<std::string, std::shared_ptr<IReactant> > superSpeciesMap;

	/**
	 * Number of He clusters in our network.
	 */
	int numHeClusters;

	/**
	 * Number of HeV clusters in our network.
	 */
	int numHeVClusters;

	/**
	 * Number of HeI clusters in our network.
	 */
	int numHeIClusters;

	/**
	 * Maximum size of He clusters in our network.
	 */
    IReactant::SizeType maxHeClusterSize;

	/**
	 * Maximum size of HeV clusters in our network.
	 */
    IReactant::SizeType maxHeVClusterSize;

	/**
	 * Maximum size of HeI clusters in our network.
	 */
    IReactant::SizeType  maxHeIClusterSize;

	/**
	 * Calculate the dissociation constant of the first cluster with respect to
	 * the single-species cluster of the same type based on the current clusters
	 * atomic volume, reaction rate constant, and binding energies.
	 *
	 * @param reaction The reaction
	 * @return The dissociation constant
	 */
	double calculateDissociationConstant(DissociationReaction * reaction) const;

	/**
	 * Calculate the binding energy for the dissociation cluster to emit the single
	 * and second cluster.
	 *
	 * @param reaction The reaction
	 * @return The binding energy corresponding to this dissociation
	 */
	virtual double computeBindingEnergy(DissociationReaction * reaction) const;

	/**
	 * This operation returns the super cluster that contains the original cluster
	 * with nHe helium atoms and nV vacancies.
	 *
	 * @param nHe the type of the compound reactant
	 * @param nV an array containing the sizes of each piece of the reactant.
	 * @return A pointer to the super cluster
	 */
	IReactant * getSuperFromComp(int nHe, int nV) const;


public:

    /**
     * Default constructor, deleted to force construction using parameters.
     */
    PSIClusterReactionNetwork() = delete;

	/**
	 * The Constructor
	 *
	 * @param registry The performance handler registry
	 */
	PSIClusterReactionNetwork(
			std::shared_ptr<xolotlPerf::IHandlerRegistry> registry);

	/**
	 * Computes the full reaction connectivity matrix for this network.
	 */
	void createReactionConnectivity();

	/**
	 * Add the dissociation connectivity for the reverse reaction if it is allowed.
	 *
	 * @param emittingReactant The reactant that would emit the pair
	 * @param reaction The reaction we want to reverse
	 * @param a The helium number for the emitting superCluster
	 * @param b The vacancy number for the emitting superCluster
	 * @param c The helium number for the emitted superCluster
	 * @param d The vacancy number for the emitted superCluster
	 *
	 */
	void checkDissociationConnectivity(IReactant * emittingReactant,
			std::shared_ptr<ProductionReaction> reaction, int a = 0, int b = 0,
			int c = 0, int d = 0);

	/**
	 * This operation sets the temperature at which the reactants currently
	 * exists. It calls setTemperature() on each reactant.
	 *
	 * This is the simplest way to set the temperature for all reactants is to
	 * call the ReactionNetwork::setTemperature() operation.
	 *
	 * @param temp The new temperature
	 */
	virtual void setTemperature(double temp);

	/**
	 * This operation returns the temperature at which the cluster currently exists.
	 *
	 * @return The temperature.
	 */
	virtual double getTemperature() const;

	/**
	 * This operation returns a reactant with the given type and size if it
	 * exists in the network or null if not.
	 *
	 * @param type the type of the reactant
	 * @param size the size of the reactant
	 * @return A pointer to the reactant
	 */
	IReactant * get(Species type, IReactant::SizeType size) const override;

	/**
	 * This operation returns a compound reactant with the given type and size
	 * if it exists in the network or null if not.
	 *
	 * @param type the type of the compound reactant
	 * @param sizes an array containing the sizes of each piece of the reactant.
	 * For PSIClusters, this array must be ordered in size by He, V and I. This
	 * array must contain an entry for He, V and I, even if only He and V or He
	 * and I are contained in the mixed-species cluster.
	 * @return A pointer to the compound reactant
	 */
	IReactant * getCompound(Species type,
                            const std::vector<IReactant::SizeType>& sizes) const override;

	/**
	 * This operation returns a super reactant with the given type and size
	 * if it exists in the network or null if not.
	 *
	 * @param type the type of the compound reactant
	 * @param sizes an array containing the sizes of each piece of the reactant.
	 * For PSIClusters, this array must be ordered in size by He, V and I. This
	 * array must contain an entry for He, V and I, even if only He and V or He
	 * and I are contained in the mixed-species cluster.
	 * @return A pointer to the compound reactant
	 */
	IReactant * getSuper(Species type, 
                const std::vector<IReactant::SizeType>& sizes) const;


	/**
	 * This operation adds a reactant or a compound reactant to the network.
	 * Adding a reactant to the network does not set the network as the
	 * reaction network for the reactant. This step must be performed
	 * separately to allow for the scenario where the network is generated
	 * entirely before running.
	 *
	 * This operation sets the id of the reactant to one that is specific
	 * to this network. Do not share reactants across networks! This id is
	 * guaranteed to be between 1 and n, including both, for n reactants in
	 * the network.
	 *
	 * The reactant will not be added to the network if the PSICluster does
	 * not recognize it as a type of reactant that it cares about (including
	 * adding null). This operation throws an exception of type std::string
	 * if the reactant is  already in the network.
	 *
	 * @param reactant The reactant that should be added to the network.
	 */
	void add(std::shared_ptr<IReactant> reactant);

	/**
	 * This operation adds a super reactant to the network.
	 * Adding a reactant to the network does not set the network as the
	 * reaction network for the reactant. This step must be performed
	 * separately to allow for the scenario where the network is generated
	 * entirely before running.
	 *
	 * This operation sets the id of the reactant to one that is specific
	 * to this network. Do not share Reactants across networks! This id is
	 * guaranteed to be between 1 and n, including both, for n reactants in
	 * the network.
	 *
	 * The reactant will not be added to the network if the PSICluster does
	 * not recognize it as a type of reactant that it cares about (including
	 * adding null). This operation throws an exception of type std::string
	 * if the reactant is  already in the network.
	 *
	 * @param reactant The reactant that should be added to the network.
	 */
	void addSuper(std::shared_ptr<IReactant> reactant);

	/**
	 * This operation removes a group of reactants from the network.
	 *
	 * @param reactants The reactants that should be removed.
	 */
	void removeReactants(const std::vector<IReactant*>& reactants);

	/**
	 * This operation reinitializes the network.
	 *
	 * It computes the cluster Ids and network size from the allReactants vector.
	 */
	void reinitializeNetwork();

	/**
	 * This method redefines the connectivities for each cluster in the
	 * allReactans vector.
	 */
	void reinitializeConnectivities();

	/**
	 * This operation updates the concentrations for all reactants in the
	 * network from an array.
	 *
	 * @param concentrations The array of doubles that will be for the
	 * concentrations. This operation does NOT create, destroy or resize the
	 * array. Properly aligning the array in memory so that this operation
	 * does not overrun is up to the caller.
	 */
	void updateConcentrationsFromArray(double * concentrations);

	/**
	 * This operation returns the size or number of reactants and momentums in the network.
	 *
	 * @return The number of degrees of freedom
	 */
	virtual int getDOF() {
		return networkSize + 2 * numSuperClusters;
	}

	/**
	 * Get the diagonal fill for the Jacobian, corresponding to the reactions.
	 *
	 * @param diagFill The pointer to the vector where the connectivity information is kept
	 */
	void getDiagonalFill(int *diagFill);

	/**
	 * Get the total concentration of atoms contained in the network.
	 *
	 * Here the atoms that are considered are helium atoms.
	 *
	 * @return The total concentration
	 */
	double getTotalAtomConcentration();

	/**
	 * Get the total concentration of atoms contained in bubbles in the network.
	 *
	 * Here the atoms that are considered are helium atoms.
	 *
	 * @return The total concentration
	 */
	double getTotalTrappedAtomConcentration();

	/**
	 * Get the total concentration of vacancies contained in the network.
	 *
	 * @return The total concentration
	 */
	double getTotalVConcentration();

	/**
	 * Get the total concentration of tungsten interstitials in the network.
	 *
	 * @return The total concentration
	 */
	double getTotalIConcentration();

	/**
	 * Calculate all the rate constants for the reactions and dissociations of the network.
	 * Need to be called only when the temperature changes.
	 */
	void computeRateConstants();

	/**
	 * Compute the fluxes generated by all the reactions
	 * for all the clusters and their momentums.
	 *
	 * @param updatedConcOffset The pointer to the array of the concentration at the grid
	 * point where the fluxes are computed used to find the next solution
	 */
	void computeAllFluxes(double *updatedConcOffset);

	/**
	 * Compute the partial derivatives generated by all the reactions
	 * for all the clusters and their momentum.
	 *
	 * @param vals The pointer to the array that will contain the values of
	 * partials for the reactions
	 * @param indices The pointer to the array that will contain the indices
	 * of the clusters
	 * @param size The pointer to the array that will contain the number of reactions for
	 * this cluster
	 */
	virtual void computeAllPartials(double *vals, int *indices, int *size);

	/**
	 * Number of He clusters in our network.
	 */
	int getNumHeClusters() const {
		return numHeClusters;
	}

	/**
	 * Number of HeV clusters in our network.
	 */
	int getNumHeVClusters() const {
		return numHeVClusters;
	}

	/**
	 * Number of HeI clusters in our network.
	 */
	int getNumHeIClusters() const {
		return numHeIClusters;
	}

	/**
	 * Maximum size of He clusters in our network.
	 */
    IReactant::SizeType getMaxHeClusterSize() const {
		return maxHeClusterSize;
	}

	/**
	 * Maximum size of HeV clusters in our network.
	 */
    IReactant::SizeType getMaxHeVClusterSize() const {
		return maxHeVClusterSize;
	}

	/**
	 * Maximum size of HeI clusters in our network.
	 */
    IReactant::SizeType getMaxHeIClusterSize() const {
		return maxHeIClusterSize;
	}


    /**
     * Add a value to our bounds.
     *
     * @param val The value to add to our bounds.
     */
    void addBound(int val) {
        boundVector.push_back(val);
    }
};

}

#endif
