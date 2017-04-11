#ifndef OPTIONS_H
#define OPTIONS_H

#include "IOptions.h"
#include "optionhandlers/IOptionHandler.h"

namespace xolotlCore {

/**
 * Options realizes the IOptions interface.
 * All private members will be accessed through getters and setters.
 */
class Options: public IOptions {

protected:
	/**
	 * Map of options we support, keyed by option switch string.
	 */
	typedef std::map<std::string, IOptionHandler*> OptionsMap;
	OptionsMap optionsMap;

	/**
	 * The flag that says if Xolotl should run.
	 */
	bool shouldRunFlag;

	/**
	 * The value of the exit code. Should be 0 if everything went well.
	 */
	int exitCode;

	/**
	 * The name of the file where the network is stored.
	 */
	std::string networkFilename;

	/**
	 * The number of options that will be given to PETSc.
	 */
	int petscArgc;

	/**
	 * The pointer to the options that will be given to PETSc.
	 */
	char **petscArgv;

	/**
	 * Use the constant temperature set of handlers?
	 */
	bool constTempFlag;

	/**
	 * Value for the constant temperature.
	 */
	double constTemperature;

	/**
	 * Value for the temperature gradient
	 */
	double temperatureGradient;

	/**
	 * Use the temperature profile set of handlers?
	 */
	bool tempProfileFlag;

	/**
	 * Name of the input temperature profile file.
	 */
	std::string tempProfileFilename;

	/**
	 * Use the flux amplitude option?
	 */
	bool fluxFlag;

	/**
	 * Value for the  flux.
	 */
	double fluxAmplitude;

	/**
	 * Use a time profile for the flux?
	 */
	bool fluxProfileFlag;

	/**
	 * Name of the input time profile file for the flux.
	 */
	std::string fluxProfileFilename;

	/**
	 * Which type of performance infrastructure should we use?
	 */
	xolotlPerf::IHandlerRegistry::RegistryType perfRegistryType;

	/**
	 * Use the "standard" set of handlers for the visualization infrastructure?
	 */
	bool vizStandardHandlersFlag;

	/**
	 * Name of the material.
	 */
	std::string materialName;

	/**
	 * Value of the initial vacancy concentration.
	 */
	double initialVConcentration;

	/**
	 * Value of the portion of the void on the grid at the start of the simulation.
	 */
	double voidPortion;

	/**
	 * Number of dimensions for the simulation.
	 */
	int dimensionNumber;

	/**
	 * Use a regular grid on the x direction?
	 */
	bool useRegularGridFlag;

	/**
	 * The map of physical processes to use in the simulation.
	 */
	std::map<std::string, bool> processMap;

	/**
	 * String of the list of wanted GB.
	 */
	std::string gbList;

	/**
	 * Minimum size for the grouping.
	 */
	int groupingMin;

	/**
	 * Width for the grouping in the first direction.
	 */
	int groupingWidthA;

	/**
	 * Width for the grouping in the second direction.
	 */
	int groupingWidthB;

	/**
	 * Value of the sputtering yield.
	 */
	double sputteringYield;

	/**
	 * Which type of memory usage infrastructure should we use?
	 */
	xolotlMemUsage::IHandlerRegistry::RegistryType memUsageRegistryType;

    /**
     * What sampling interval should the memory usage infrastructure use?
     */
    xolotlMemUsage::IHandlerRegistry::SamplingInterval memUsageSamplingInterval;

public:

	/**
	 * The constructor.
	 */
	Options();

	/**
	 * The destructor.
	 */
	~Options();

	/**
	 * Read the parameters from the given file to set the different
	 * xolotl options.
	 * \see IOptions.h
	 */
	void readParams(char* argv[]);

	/**
	 * Show our help message.
	 * \see IOptions.h
	 */
	void showHelp(std::ostream& os) const;

	/**
	 * Should the program run after parsing the parameter file?
	 * \see IOptions.h
	 */
	bool shouldRun() const {
		return shouldRunFlag;
	}

	/**
	 * Set the shouldRunFlag.
	 * \see IOptions.h
	 */
	void setShouldRunFlag(bool flag) {
		shouldRunFlag = flag;
	}

	/**
	 * If program shouldn't run, what should its exit code be?
	 * \see IOptions.h
	 */
	int getExitCode() const {
		return exitCode;
	}

	/**
	 * Set the value for the exit code.
	 * \see IOptions.h
	 */
	void setExitCode(int code) {
		exitCode = code;
	}

	/**
	 * Get the name of the network file.
	 * \see IOptions.h
	 */
	std::string getNetworkFilename() const {
		return networkFilename;
	}

	/**
	 * Set the name of the network file.
	 * \see IOptions.h
	 */
	void setNetworkFilename(const std::string& name) {
		networkFilename = name;
	}

	/**
	 * Get the Argc for PETSc.
	 * \see IOptions.h
	 */
	int getPetscArgc() const {
		return petscArgc;
	}

	/**
	 * Set the Argc for PETSc.
	 * \see IOptions.h
	 */
	void setPetscArgc(int argc) {
		petscArgc = argc;
	}

	/**
	 * Get the Argv for PETSc.
	 * \see IOptions.h
	 */
	char** getPetscArgv() const {
		return petscArgv;
	}

	/**
	 * Set the Argv for PETSc.
	 * \see IOptions.h
	 */
	void setPetscArgv(char** argv) {
		petscArgv = argv;
	}

	/**
	 * Should we use const temperature handlers?
	 * \see IOptions.h
	 */
	bool useConstTemperatureHandlers() const {
		return constTempFlag;
	}

	/**
	 * Set the constTempFlag.
	 * \see IOptions.h
	 */
	void setConstTempFlag(bool flag) {
		constTempFlag = flag;
	}

	/**
	 * Obtain the value of the constant temperature to be used.
	 * \see IOptions.h
	 */
	double getConstTemperature() const {
		return constTemperature;
	}

	/**
	 * Set the constant temperature.
	 * \see IOptions.h
	 */
	void setConstTemperature(double temp) {
		constTemperature = temp;
	}

	/**
	 * Obtain the value of the temperature gradient to be used.
	 * \see IOptions.h
	 */
	double getTemperatureGradient() const {
		return temperatureGradient;
	}

	/**
	 * Set the temperature gradient.
	 * \see IOptions.h
	 */
	void setTemperatureGradient(double grad) {
		temperatureGradient = grad;
	}

	/**
	 * Should we use temperature profile handlers?
	 * \see IOptions.h
	 */
	bool useTemperatureProfileHandlers() const {
		return tempProfileFlag;
	}

	/**
	 * Set the tempProfileFlag.
	 * \see IOptions.h
	 */
	void setTempProfileFlag(bool flag) {
		tempProfileFlag = flag;
	}

	/**
	 * Obtain the name of the file containing the temperature profile data.
	 * \see IOptions.h
	 */
	std::string getTempProfileFilename() const {
		return tempProfileFilename;
	}

	/**
	 * Set the name of the profile file to use.
	 * \see IOptions.h
	 */
	void setTempProfileFilename(const std::string& name) {
		tempProfileFilename = name;
	}

	/**
	 * Should we use the flux option?
	 * \see IOptions.h
	 */
	bool useFluxAmplitude() const {
		return fluxFlag;
	}
	;

	/**
	 * Set the fluxFlag.
	 * \see IOptions.h
	 */
	void setFluxFlag(bool flag) {
		fluxFlag = flag;
	}

	/**
	 * Obtain the value of the flux intensity to be used.
	 * \see IOptions.h
	 */
	double getFluxAmplitude() const {
		return fluxAmplitude;
	}

	/**
	 * Set the value for the flux intensity to use.
	 * \see IOptions.h
	 */
	void setFluxAmplitude(double flux) {
		fluxAmplitude = flux;
	}

	/**
	 * Should we use a time profile for the flux?
	 * \see IOptions.h
	 */
	bool useFluxTimeProfile() const {
		return fluxProfileFlag;
	}

	/**
	 * Set the fluxProfileFlag.
	 * \see IOptions.h
	 */
	void setFluxProfileFlag(bool flag) {
		fluxProfileFlag = flag;
	}

	/**
	 * Obtain the name of the file containing the time profile data for the
	 * flux.
	 * \see IOptions.h
	 */
	std::string getFluxProfileName() const {
		return fluxProfileFilename;
	}

	/**
	 * Set the name of the time profile file to use.
	 * \see IOptions.h
	 */
	void setFluxProfileName(const std::string& name) {
		fluxProfileFilename = name;
	}

	/**
	 * Which type of performance handlers should we use?
	 * \see IOptions.h
	 */
	xolotlPerf::IHandlerRegistry::RegistryType getPerfHandlerType(void) const {
		return perfRegistryType;
	}

	/**
	 * Set the type of performance handlers to use.
	 * \see IOptions.h
	 */
	void setPerfHandlerType(xolotlPerf::IHandlerRegistry::RegistryType rtype) {
		perfRegistryType = rtype;
	}

	/**
	 * Should we use the "standard" set of handlers for the visualization?
	 * If false, use dummy (stub) handlers.
	 * \see IOptions.h
	 */
	bool useVizStandardHandlers() const {
		return vizStandardHandlersFlag;
	}

	/**
	 * Set the vizStandardHandlersFlag.
	 * \see IOptions.h
	 */
	void setVizStandardHandlers(bool flag) {
		vizStandardHandlersFlag = flag;
	}

	/**
	 * Obtain the name of the material to be used for simulation.
	 * \see IOptions.h
	 */
	std::string getMaterial() const {
		return materialName;
	}

	/**
	 * Set the name of the material to be used for the simulation.
	 * \see IOptions.h
	 */
	void setMaterial(const std::string& material) {
		materialName = material;
	}

	/**
	 * Obtain the value of the concentration for the vacancies.
	 * \see IOptions.h
	 */
	double getInitialVConcentration() const {
		return initialVConcentration;
	}

	/**
	 * Set the value of the concentration for the vacancies.
	 * \see IOptions.h
	 */
	void setInitialVConcentration(double conc) {
		initialVConcentration = conc;
	}

	/**
	 * Obtain the number of dimensions for the simulation.
	 * \see IOptions.h
	 */
	int getDimensionNumber() const {
		return dimensionNumber;
	}

	/**
	 * Set the number of dimensions for the simulation.
	 * \see IOptions.h
	 */
	void setDimensionNumber(int number) {
		dimensionNumber = number;
	}

	/**
	 * Obtain the value of the void portion for the simulation.
	 * \see IOptions.h
	 */
	double getVoidPortion() const {
		return voidPortion;
	}

	/**
	 * Set the value of the void portion for the surface to grow.
	 * \see IOptions.h
	 */
	void setVoidPortion(double portion) {
		voidPortion = portion;
	}

	/**
	 * Should we use a regular grid on the x direction?
	 * \see IOptions.h
	 */
	bool useRegularXGrid() const {
		return useRegularGridFlag;
	}

	/**
	 * Set the useRegularGridFlag.
	 * \see IOptions.h
	 */
	void setRegularXGrid(bool flag) {
		useRegularGridFlag = flag;
	}

	/**
	 * Obtain the physical process map.
	 *
	 * @return The map
	 */
	std::map<std::string, bool> getProcesses() const {
		return processMap;
	}

	/**
	 * Set the physical process map.
	 *
	 * @param map The map
	 */
	void setProcesses(std::map<std::string, bool> map) {
		processMap = map;
	}

	/**
	 * Obtain the string listing the wanted GB.
	 * \see IOptions.h
	 */
	std::string getGbString() const {
		return gbList;
	}

	/**
	 * Set the string listing the wanted GB.
	 * \see IOptions.h
	 */
	void setGbString(const std::string& gbString) {
		gbList = gbString;
	}

	/**
	 * Obtain the minimum size for the grouping.
	 * \see IOptions.h
	 */
	int getGroupingMin() const {
		return groupingMin;
	}

	/**
	 * Set the minimum size for the grouping.
	 * \see IOptions.h
	 */
	void setGroupingMin(int size) {
		groupingMin = size;
	}

	/**
	 * Obtain the first width for the grouping.
	 * \see IOptions.h
	 */
	int getGroupingWidthA() const {
		return groupingWidthA;
	}

	/**
	 * Set the first width for the grouping.
	 * \see IOptions.h
	 */
	void setGroupingWidthA(int width) {
		groupingWidthA = width;
	}

	/**
	 * Obtain the second width for the grouping.
	 * \see IOptions.h
	 */
	int getGroupingWidthB() const {
		return groupingWidthB;
	}

	/**
	 * Set the second width for the grouping.
	 * \see IOptions.h
	 */
	void setGroupingWidthB(int width) {
		groupingWidthB = width;
	}

	/**
	 * Obtain the value of the intensity of the sputtering yield to be used.
	 * \see IOptions.h
	 */
	double getSputteringYield() const {
		return sputteringYield;
	}

	/**
	 * Set the value for the sputtering yield to use.
	 * \see IOptions.h
	 */
	virtual void setSputteringYield(double yield) {
		sputteringYield = yield;
	}

    
	/**
	 * Which type of memory usage handlers should we use?
	 *
	 * @return The type of memory usage handler registry to use
	 */
	virtual xolotlMemUsage::IHandlerRegistry::RegistryType getMemUsageHandlerType(void) const {
        return memUsageRegistryType;
    }

	/**
	 * Set the type of memory usage handlers to use.
	 *
	 * @param rtype The type of memory usage handler registry to use
	 */
	virtual void setMemUsageHandlerType(
			xolotlMemUsage::IHandlerRegistry::RegistryType rtype) {

        memUsageRegistryType = rtype;
    }

    /**
     * Retrieve the interval used for sampling memory usage.
     *
     * @return Interval to use for sampling memory usage.
     */
    virtual xolotlMemUsage::IHandlerRegistry::SamplingInterval getMemUsageSamplingInterval(void) const {
        return memUsageSamplingInterval;
    }

    /**
     * Set the sampling interval to use for memory usage tracking.
     *
     * @param interval The memory usage tracking sampling interval to use.
     */
    virtual void setMemUsageSamplingInterval(xolotlMemUsage::IHandlerRegistry::SamplingInterval interval) {

        memUsageSamplingInterval = interval;
    }
};
//end class Options

} /* namespace xolotlCore */

#endif // OPTIONS_H
