#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <algorithm>
#include <iomanip>

class BDFakeG {
private:
    std::map<std::string, int> elementMap;
    bool debugMode;  // 添加调试模式标志
    
    struct Atom {
        std::string symbol;
        int atomicNumber;
        double x, y, z;
    };
    
    struct OptStep {
        int stepNumber;
        std::vector<Atom> atoms;
        double energy;
        double rmsGrad, maxGrad, rmsStep, maxStep;
        bool converged;
    };
    
    struct FreqMode {
        double frequency;
        double irIntensity;
        std::string irrep;  // 对称性信息
        std::vector<std::vector<double>> displacements; // [atom][xyz]
    };
    
    struct ThermoData {
        double temperature;
        double pressure;
        double electronicEnergy;
        double zpe;
        double thermalEnergyCorr;
        double thermalEnthalpyCorr;
        double thermalGibbsCorr;
        bool hasData;
        
        ThermoData() : temperature(298.15), pressure(1.0), electronicEnergy(0.0),
                       zpe(0.0), thermalEnergyCorr(0.0), thermalEnthalpyCorr(0.0),
                       thermalGibbsCorr(0.0), hasData(false) {}
    };
    
    std::vector<OptStep> optSteps;
    std::vector<FreqMode> frequencies;
    ThermoData thermoData;
    bool hasOpt;
    bool hasFreq;
    
    void initElementMap() {
    // Period 1
    elementMap["H"] = 1; elementMap["He"] = 2;
    
    // Period 2
    elementMap["Li"] = 3; elementMap["Be"] = 4; elementMap["B"] = 5; 
    elementMap["C"] = 6; elementMap["N"] = 7; elementMap["O"] = 8; 
    elementMap["F"] = 9; elementMap["Ne"] = 10;
    
    // Period 3
    elementMap["Na"] = 11; elementMap["Mg"] = 12; elementMap["Al"] = 13; 
    elementMap["Si"] = 14; elementMap["P"] = 15; elementMap["S"] = 16; 
    elementMap["Cl"] = 17; elementMap["Ar"] = 18;
    
    // Period 4
    elementMap["K"] = 19; elementMap["Ca"] = 20;
    elementMap["Sc"] = 21; elementMap["Ti"] = 22; elementMap["V"] = 23; 
    elementMap["Cr"] = 24; elementMap["Mn"] = 25; elementMap["Fe"] = 26; 
    elementMap["Co"] = 27; elementMap["Ni"] = 28; elementMap["Cu"] = 29; 
    elementMap["Zn"] = 30;
    elementMap["Ga"] = 31; elementMap["Ge"] = 32; elementMap["As"] = 33; 
    elementMap["Se"] = 34; elementMap["Br"] = 35; elementMap["Kr"] = 36;
    
    // Period 5
    elementMap["Rb"] = 37; elementMap["Sr"] = 38; elementMap["Y"] = 39; 
    elementMap["Zr"] = 40; elementMap["Nb"] = 41; elementMap["Mo"] = 42; 
    elementMap["Tc"] = 43; elementMap["Ru"] = 44; elementMap["Rh"] = 45; 
    elementMap["Pd"] = 46; elementMap["Ag"] = 47; elementMap["Cd"] = 48;
    elementMap["In"] = 49; elementMap["Sn"] = 50; elementMap["Sb"] = 51; 
    elementMap["Te"] = 52; elementMap["I"] = 53; elementMap["Xe"] = 54;
    
    // Period 6
    elementMap["Cs"] = 55; elementMap["Ba"] = 56; elementMap["La"] = 57;
    // Lanthanides
    elementMap["Ce"] = 58; elementMap["Pr"] = 59; elementMap["Nd"] = 60; 
    elementMap["Pm"] = 61; elementMap["Sm"] = 62; elementMap["Eu"] = 63; 
    elementMap["Gd"] = 64; elementMap["Tb"] = 65; elementMap["Dy"] = 66; 
    elementMap["Ho"] = 67; elementMap["Er"] = 68; elementMap["Tm"] = 69; 
    elementMap["Yb"] = 70; elementMap["Lu"] = 71;
    // Period 6 continuation
    elementMap["Hf"] = 72; elementMap["Ta"] = 73; elementMap["W"] = 74; 
    elementMap["Re"] = 75; elementMap["Os"] = 76; elementMap["Ir"] = 77; 
    elementMap["Pt"] = 78; elementMap["Au"] = 79; elementMap["Hg"] = 80;
    elementMap["Tl"] = 81; elementMap["Pb"] = 82; elementMap["Bi"] = 83; 
    elementMap["Po"] = 84; elementMap["At"] = 85; elementMap["Rn"] = 86;
    
    // Period 7
    elementMap["Fr"] = 87; elementMap["Ra"] = 88; elementMap["Ac"] = 89;
    // Actinides
    elementMap["Th"] = 90; elementMap["Pa"] = 91; elementMap["U"] = 92; 
    elementMap["Np"] = 93; elementMap["Pu"] = 94; elementMap["Am"] = 95; 
    elementMap["Cm"] = 96; elementMap["Bk"] = 97; elementMap["Cf"] = 98; 
    elementMap["Es"] = 99; elementMap["Fm"] = 100; elementMap["Md"] = 101; 
    elementMap["No"] = 102; elementMap["Lr"] = 103;
    // Period 7 continuation
    elementMap["Rf"] = 104; elementMap["Db"] = 105; elementMap["Sg"] = 106; 
    elementMap["Bh"] = 107; elementMap["Hs"] = 108; elementMap["Mt"] = 109; 
    elementMap["Ds"] = 110; elementMap["Rg"] = 111; elementMap["Cn"] = 112; 
    elementMap["Nh"] = 113; elementMap["Fl"] = 114; elementMap["Mc"] = 115; 
    elementMap["Lv"] = 116; elementMap["Ts"] = 117; elementMap["Og"] = 118;
    }
    
    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(' ');
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(' ');
        return str.substr(first, (last - first + 1));
    }
    
    std::vector<std::string> split(const std::string& str, char delimiter = ' ') {
        std::vector<std::string> tokens;
        std::stringstream ss(str);
        std::string token;
        while (std::getline(ss, token, delimiter)) {
            if (!token.empty() && token != " ") {
                tokens.push_back(trim(token));
            }
        }
        return tokens;
    }
    
    bool findLine(std::ifstream& file, const std::string& pattern) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.find(pattern) != std::string::npos) {
                return true;
            }
        }
        return false;
    }
    
    bool findLineFromBeginning(std::ifstream& file, const std::string& pattern) {
        file.clear();
        file.seekg(0, std::ios::beg);
        return findLine(file, pattern);
    }
    
    void parseGeometryStep(std::ifstream& file, OptStep& step) {
        std::string line;
        
        // Find "Atom         Coord" section
        if (!findLine(file, "Atom         Coord")) {
            if (debugMode) {
                std::cout << "Warning: Cannot find Atom Coord section for step " << step.stepNumber << std::endl;
            }
            return;
        }
        
        // Read atoms until we hit State= or Energy= or empty line
        while (std::getline(file, line)) {
            line = trim(line);
            
            if (line.empty() || line.find("State=") != std::string::npos) {
                break;
            }
            
            if (line.find("Energy=") != std::string::npos) {
                // Parse energy from this line
                size_t pos = line.find("Energy=");
                std::string energyStr = line.substr(pos + 7);
                step.energy = std::stod(trim(energyStr));
                break;
            }
            
            // Parse atom line: Element X Y Z
            std::istringstream iss(line);
            std::string element;
            double x, y, z;
            
            if (iss >> element >> x >> y >> z) {
                Atom atom;
                atom.symbol = element;
                atom.atomicNumber = elementMap.count(element) ? elementMap[element] : 1;
                atom.x = x;
                atom.y = y;
                atom.z = z;
                step.atoms.push_back(atom);
                
                if (debugMode) {
                    std::cout << "Step " << step.stepNumber << " - Read atom: " << element << " (" << atom.atomicNumber 
                             << ") at (" << x << ", " << y << ", " << z << ")" << std::endl;
                }
            }
        }
        
        // If we didn't get energy from the coordinate section, look for it separately
        if (step.energy == 0.0) {
            // Continue reading to find Energy=
            while (std::getline(file, line)) {
                if (line.find("Energy=") != std::string::npos) {
                    size_t pos = line.find("Energy=");
                    std::string energyStr = line.substr(pos + 7);
                    step.energy = std::stod(trim(energyStr));
                    break;
                }
                // Stop if we hit the next section
                if (line.find("Force-RMS") != std::string::npos || 
                    line.find("Geometry Optimization step") != std::string::npos) {
                    break;
                }
            }
        }
    }
    
    void parseConvergence(std::ifstream& file, OptStep& step) {
        std::string line;
        
        // Look for convergence values after the current geometry
        // The "Current values" line should appear after the coordinates for this step
        while (std::getline(file, line)) {
            if (line.find("Current values") != std::string::npos) {
                // Parse the values from the same line or next line
                std::istringstream iss(line);
                std::string dummy;
                
                // Skip "Current values  :"
                iss >> dummy >> dummy >> dummy;
                
                if (iss >> step.rmsGrad >> step.maxGrad >> step.rmsStep >> step.maxStep) {
                    if (debugMode) {
                        std::cout << "Step " << step.stepNumber << " convergence: RMS Grad=" << step.rmsGrad 
                                 << ", Max Grad=" << step.maxGrad << ", RMS Step=" << step.rmsStep 
                                 << ", Max Step=" << step.maxStep << std::endl;
                    }
                } else {
                    // Values might be on the next line
                    if (std::getline(file, line)) {
                        std::istringstream iss2(line);
                        iss2 >> step.rmsGrad >> step.maxGrad >> step.rmsStep >> step.maxStep;
                    }
                }
                break;
            }
            
            // Stop if we hit the next optimization step or other sections
            if (line.find("Geometry Optimization step") != std::string::npos ||
                line.find("Good Job") != std::string::npos ||
                line.find("Results of vibrations") != std::string::npos) {
                break;
            }
        }
        
        // Check if this step is converged
        step.converged = (step.rmsGrad < 3.0e-4 && step.maxGrad < 4.5e-4 && 
                         step.rmsStep < 1.2e-3 && step.maxStep < 1.8e-3);
    }
    
    void parseFrequencies(std::ifstream& file) {
        std::string line;
        
        file.clear();
        file.seekg(0, std::ios::beg);
        
        if (!findLineFromBeginning(file, "Results of vibrations:")) {
            std::cout << "No frequency analysis found" << std::endl;
            return;
        }
        
        hasFreq = true;
        if (debugMode) {
            std::cout << "Found frequency analysis" << std::endl;
        }
        
        // Skip header lines
        std::getline(file, line); // Normal frequencies line
        std::getline(file, line); // blank line
        
        while (std::getline(file, line)) {
            line = trim(line);
            
            if (line.find("Results of translations") != std::string::npos) {
                break;
            }
            
            // Look for frequency block headers (numbers like 1, 2, 3)
            if (!line.empty() && std::isdigit(line[0])) {
                int nFreqs = countFrequenciesInLine(line);
                if (nFreqs > 0) {
                    parseFrequencyBlock(file, nFreqs);
                }
            }
        }
        
        std::cout << "Total frequencies parsed: " << frequencies.size() << std::endl;
        
        // Debug output: print parsed displacement vectors
        if (debugMode) {
            for (size_t i = 0; i < frequencies.size(); i++) {
                std::cout << "Frequency " << (i+1) << ": " << frequencies[i].frequency << " cm^-1" << std::endl;
                std::cout << "  Irrep: " << frequencies[i].irrep << std::endl;
                std::cout << "  IR Intensity: " << frequencies[i].irIntensity << std::endl;
                std::cout << "  Displacements (" << frequencies[i].displacements.size() << " atoms):" << std::endl;
                for (size_t j = 0; j < frequencies[i].displacements.size(); j++) {
                    if (frequencies[i].displacements[j].size() >= 3) {
                        std::cout << "    Atom " << (j+1) << ": " 
                                 << frequencies[i].displacements[j][0] << " "
                                 << frequencies[i].displacements[j][1] << " "
                                 << frequencies[i].displacements[j][2] << std::endl;
                    }
                }
            }
        }
    }
    
    int countFrequenciesInLine(const std::string& line) {
        std::istringstream iss(line);
        std::string token;
        int count = 0;
        
        while (iss >> token) {
            try {
                int num = std::stoi(token);
                if (num >= 1 && num <= 100) count++;
            } catch (...) {
                break;
            }
        }
        return count;
    }
    
    void parseFrequencyBlock(std::ifstream& file, int nFreqs) {
        std::string line;
        
        // Read Irreps line and extract symmetry information
        std::getline(file, line);
        std::vector<std::string> irreps;
        if (line.find("Irreps") != std::string::npos) {
            // Parse irreps from the line
            std::istringstream iss(line);
            std::string word;
            bool foundIrreps = false;
            while (iss >> word) {
                if (foundIrreps && irreps.size() < nFreqs) {
                    irreps.push_back(word);
                }
                if (word == "Irreps") {
                    foundIrreps = true;
                }
            }
        }
        
        // Read frequencies
        std::getline(file, line);
        std::vector<double> freqValues = parseValuesFromLine(line, nFreqs);
        
        // Skip reduced masses and force constants
        std::getline(file, line); // reduced masses
        std::getline(file, line); // force constants
        
        // Read IR intensities
        std::getline(file, line);
        std::vector<double> irValues = parseValuesFromLine(line, nFreqs);
        
        // Create frequency modes with symmetry information
        int startIdx = frequencies.size();
        for (int i = 0; i < nFreqs; i++) {
            FreqMode mode;
            mode.frequency = (i < freqValues.size()) ? freqValues[i] : 0.0;
            mode.irIntensity = (i < irValues.size()) ? irValues[i] : 0.0;
            mode.irrep = (i < irreps.size()) ? irreps[i] : "A";  // 使用提取的对称性
            frequencies.push_back(mode);
        }
        
        // Read atomic displacements
        if (!optSteps.empty()) {
            int nAtoms = optSteps.back().atoms.size();
            if (debugMode) {
                std::cout << "DEBUG: Expected " << nAtoms << " atoms for displacement data" << std::endl;
            }
            
            // Initialize displacement vectors for all frequencies in this block
            for (int i = 0; i < nFreqs; i++) {
                if ((startIdx + i) < frequencies.size()) {
                    frequencies[startIdx + i].displacements.resize(nAtoms);
                    // Initialize each atom's displacement vector with 3 components
                    for (int j = 0; j < nAtoms; j++) {
                        frequencies[startIdx + i].displacements[j].resize(3, 0.0);
                    }
                }
            }
            
            // Skip the header line (usually contains "Atom  ZA               X         Y         Z")
            std::getline(file, line);
            if (debugMode) {
                std::cout << "DEBUG: Read potential header line: " << line << std::endl;
            }
            
            if (line.find("Atom") != std::string::npos && line.find("ZA") != std::string::npos) {
                if (debugMode) {
                    std::cout << "DEBUG: Confirmed header line, skipping" << std::endl;
                }
                // This is a header line, read all atoms
                for (int iatom = 0; iatom < nAtoms; iatom++) {
                    if (std::getline(file, line)) {
                        if (debugMode) {
                            std::cout << "DEBUG: Reading atom " << (iatom + 1) << " data: " << line << std::endl;
                        }
                        parseAtomDisplacements(line, startIdx, nFreqs);
                    } else {
                        std::cout << "Warning: Could not read displacement data for atom " << (iatom + 1) << std::endl;
                        break;
                    }
                }
            } else {
                if (debugMode) {
                    std::cout << "DEBUG: Not a header line, treating as first atom data" << std::endl;
                }
                // This isn't a header line, process it as atom data
                parseAtomDisplacements(line, startIdx, nFreqs);
                
                // Read remaining atom displacement data
                for (int iatom = 1; iatom < nAtoms; iatom++) {
                    if (std::getline(file, line)) {
                        if (debugMode) {
                            std::cout << "DEBUG: Reading atom " << (iatom + 1) << " data: " << line << std::endl;
                        }
                        parseAtomDisplacements(line, startIdx, nFreqs);
                    } else {
                        std::cout << "Warning: Could not read displacement data for atom " << (iatom + 1) << std::endl;
                        break;
                    }
                }
            }
        }
        
        // Skip blank line
        std::getline(file, line);
        if (debugMode) {
            std::cout << "DEBUG: Skipped blank line: " << line << std::endl;
        }
    }
    
    std::vector<double> parseValuesFromLine(const std::string& line, int nVals) {
        std::vector<double> values;
        std::istringstream iss(line);
        std::string token;
        
        // Skip initial text
        while (iss >> token && values.size() < nVals) {
            try {
                double val = std::stod(token);
                values.push_back(val);
            } catch (...) {
                // Continue if conversion fails
            }
        }
        
        return values;
    }
    
    void parseAtomDisplacements(const std::string& line, int startIdx, int nFreqs) {
        std::istringstream iss(line);
        std::string token;
        int atomNum, za;
        
        // Read atom number and ZA
        if (!(iss >> atomNum >> za)) {
            if (debugMode) {
                std::cout << "Warning: Failed to parse atom number and ZA from line: " << line << std::endl;
            }
            return;
        }
        
        if (debugMode) {
            std::cout << "Parsing displacements for atom " << atomNum << " (ZA=" << za << ")" << std::endl;
        }
        
        // Read displacement vectors for each frequency
        for (int ifreq = 0; ifreq < nFreqs && (startIdx + ifreq) < frequencies.size(); ifreq++) {
            double x, y, z;
            if (iss >> x >> y >> z) {
                // Store displacement at correct atom position (atomNum is 1-based)
                int atomIdx = atomNum - 1;
                if (atomIdx >= 0 && atomIdx < frequencies[startIdx + ifreq].displacements.size()) {
                    frequencies[startIdx + ifreq].displacements[atomIdx][0] = x;
                    frequencies[startIdx + ifreq].displacements[atomIdx][1] = y;
                    frequencies[startIdx + ifreq].displacements[atomIdx][2] = z;
                    
                    if (debugMode) {
                        std::cout << "  Freq " << (startIdx + ifreq + 1) << ", Atom " << atomNum 
                                 << ": (" << x << ", " << y << ", " << z << ")" << std::endl;
                    }
                } else {
                    if (debugMode) {
                        std::cout << "Warning: Invalid atom index " << atomIdx << " for displacement storage" << std::endl;
                    }
                }
            } else {
                if (debugMode) {
                    std::cout << "Warning: Failed to parse displacement values for atom " << atomNum 
                             << ", frequency " << (ifreq + 1) << std::endl;
                }
                break;
            }
        }
    }
    
    void parseThermoData(std::ifstream& file) {
        std::string line;
        
        file.clear();
        file.seekg(0, std::ios::beg);
        
        // Find the thermodynamic section
        bool foundThermoSection = false;
        while (std::getline(file, line)) {
            if (line.find("Thermal Contributions to Energies") != std::string::npos) {
                foundThermoSection = true;
                break;
            }
        }
        
        if (!foundThermoSection) {
            if (debugMode) {
                std::cout << "No thermodynamic data found" << std::endl;
            }
            return;
        }
        
        thermoData.hasData = true;
        if (debugMode) {
            std::cout << "Found thermodynamic data" << std::endl;
        }
        
        // Continue reading from current position
        while (std::getline(file, line)) {
            // Remove leading/trailing whitespace
            line = trim(line);
            
            if (debugMode) {
                std::cout << "DEBUG: Processing thermo line: '" << line << "'" << std::endl;
            }
            
            // Parse electronic energy - format: "Electronic total energy   :        -1.170752    Hartree"
            if (line.find("Electronic total energy") != std::string::npos && line.find(":") != std::string::npos) {
                size_t colonPos = line.find(":");
                std::string valueStr = line.substr(colonPos + 1);
                
                // Extract first numerical value after colon
                std::istringstream iss(valueStr);
                double value;
                if (iss >> value) {
                    thermoData.electronicEnergy = value;
                    if (debugMode) {
                        std::cout << "DEBUG: Parsed Electronic energy: " << thermoData.electronicEnergy << std::endl;
                    }
                }
            }
            
            // Parse temperature and pressure - format: "#   1    Temperature =       298.15000 Kelvin         Pressure =         1.00000 Atm"
            else if (line.find("Temperature") != std::string::npos && line.find("Kelvin") != std::string::npos) {
                // Extract temperature using simple string parsing
                size_t tempPos = line.find("Temperature");
                if (tempPos != std::string::npos) {
                    // Find the "=" after Temperature
                    size_t eqPos = line.find("=", tempPos);
                    if (eqPos != std::string::npos) {
                        // Find "Kelvin" to get the end position
                        size_t kelvinPos = line.find("Kelvin", eqPos);
                        if (kelvinPos != std::string::npos) {
                            // Extract the substring between "=" and "Kelvin"
                            std::string tempStr = line.substr(eqPos + 1, kelvinPos - eqPos - 1);
                            // Parse the numerical value
                            std::istringstream iss(tempStr);
                            double temp;
                            if (iss >> temp) {
                                thermoData.temperature = temp;
                                if (debugMode) {
                                    std::cout << "DEBUG: Parsed Temperature: " << thermoData.temperature << std::endl;
                                }
                            }
                        }
                    }
                }
                
                // Extract pressure
                size_t pressPos = line.find("Pressure");
                if (pressPos != std::string::npos) {
                    size_t eqPos = line.find("=", pressPos);
                    if (eqPos != std::string::npos) {
                        size_t atmPos = line.find("Atm", eqPos);
                        if (atmPos != std::string::npos) {
                            std::string pressStr = line.substr(eqPos + 1, atmPos - eqPos - 1);
                            std::istringstream iss(pressStr);
                            double press;
                            if (iss >> press) {
                                thermoData.pressure = press;
                                if (debugMode) {
                                    std::cout << "DEBUG: Parsed Pressure: " << thermoData.pressure << std::endl;
                                }
                            }
                        }
                    }
                }
            }
            
            // Parse zero-point energy - format: "Zero-point Energy                          :            0.010179            6.387623"
            else if (line.find("Zero-point Energy") != std::string::npos && line.find(":") != std::string::npos) {
                size_t colonPos = line.find(":");
                std::string valueStr = line.substr(colonPos + 1);
                
                std::istringstream iss(valueStr);
                double value;
                if (iss >> value) {
                    thermoData.zpe = value;
                    if (debugMode) {
                        std::cout << "DEBUG: Parsed Zero-point Energy: " << thermoData.zpe << std::endl;
                    }
                }
            }
            
            // Parse thermal correction to energy - format: "Thermal correction to Energy               :            0.012540            7.868837"
            else if (line.find("Thermal correction to Energy") != std::string::npos && line.find(":") != std::string::npos) {
                size_t colonPos = line.find(":");
                std::string valueStr = line.substr(colonPos + 1);
                
                std::istringstream iss(valueStr);
                double value;
                if (iss >> value) {
                    thermoData.thermalEnergyCorr = value;
                    if (debugMode) {
                        std::cout << "DEBUG: Parsed Thermal correction to Energy: " << thermoData.thermalEnergyCorr << std::endl;
                    }
                }
            }
            
            // Parse thermal correction to enthalpy - format: "Thermal correction to Enthalpy             :            0.013484            8.461322"
            else if (line.find("Thermal correction to Enthalpy") != std::string::npos && line.find(":") != std::string::npos) {
                size_t colonPos = line.find(":");
                std::string valueStr = line.substr(colonPos + 1);
                
                std::istringstream iss(valueStr);
                double value;
                if (iss >> value) {
                    thermoData.thermalEnthalpyCorr = value;
                    if (debugMode) {
                        std::cout << "DEBUG: Parsed Thermal correction to Enthalpy: " << thermoData.thermalEnthalpyCorr << std::endl;
                    }
                }
            }
            
            // Parse thermal correction to Gibbs free energy - format: "Thermal correction to Gibbs Free Energy    :           -0.001315           -0.825417"
            else if (line.find("Thermal correction to Gibbs Free Energy") != std::string::npos && line.find(":") != std::string::npos) {
                size_t colonPos = line.find(":");
                std::string valueStr = line.substr(colonPos + 1);
                
                std::istringstream iss(valueStr);
                double value;
                if (iss >> value) {
                    thermoData.thermalGibbsCorr = value;
                    if (debugMode) {
                        std::cout << "DEBUG: Parsed Thermal correction to Gibbs Free Energy: " << thermoData.thermalGibbsCorr << std::endl;
                    }
                }
            }
            
            // Stop when we reach the next major section - use more specific markers
            if (line.find("Gradient Information") != std::string::npos ||
                line.find("UniMoVib job terminated") != std::string::npos ||
                line.find("Cartesian gradients") != std::string::npos) {
                if (debugMode) {
                    std::cout << "DEBUG: Reached end of thermo section at: " << line << std::endl;
                }
                break;
            }
        }
        
        // Print summary of parsed data for debugging
        if (debugMode) {
            std::cout << "\n=== Thermodynamic Data Summary ===" << std::endl;
            std::cout << "HasData: " << (thermoData.hasData ? "true" : "false") << std::endl;
            std::cout << "Temperature: " << thermoData.temperature << " K" << std::endl;
            std::cout << "Pressure: " << thermoData.pressure << " atm" << std::endl;
            std::cout << "Electronic Energy: " << thermoData.electronicEnergy << " Hartree" << std::endl;
            std::cout << "Zero-point Energy: " << thermoData.zpe << " Hartree" << std::endl;
            std::cout << "Thermal correction to Energy: " << thermoData.thermalEnergyCorr << " Hartree" << std::endl;
            std::cout << "Thermal correction to Enthalpy: " << thermoData.thermalEnthalpyCorr << " Hartree" << std::endl;
            std::cout << "Thermal correction to Gibbs: " << thermoData.thermalGibbsCorr << " Hartree" << std::endl;
            std::cout << "=================================" << std::endl;
        }
    }
    
    void writeGaussianOutput(const std::string& outputFile) {
        std::ofstream out(outputFile);
        
        // Header
        out << "! This file was generated by BDFakeG version 1.0" << std::endl;
        out << "! Converted from BDF format to Gaussian format" << std::endl;
        out << std::endl;
        out << "0 basis functions" << std::endl;
        out << "0 alpha electrons" << std::endl;
        out << "0 beta electrons" << std::endl;
        out << "GradGradGradGradGradGradGradGradGradGradGradGradGradGradGradGradGradGrad" << std::endl;
        out << "GradGradGradGradGradGradGradGradGradGradGradGradGradGradGradGradGradGrad" << std::endl;
        
        // Write optimization steps
        for (const auto& step : optSteps) {
            writeOptimizationStep(out, step);
        }
        
        if (hasOpt) {
            out << std::endl << "Normal termination of Gaussian" << std::endl;
        }
        
        // Write frequencies
        if (hasFreq && !frequencies.empty()) {
            writeFrequencies(out);
            out << std::endl << "Normal termination of Gaussian" << std::endl;
        }
        
        out.close();
    }
    
    void writeOptimizationStep(std::ofstream& out, const OptStep& step) {
        // Write geometry
        out << std::endl;
        out << "                        Standard orientation:" << std::endl;
        out << "---------------------------------------------------------------------" << std::endl;
        out << "Center     Atomic      Atomic             Coordinates (Angstroms)" << std::endl;
        out << "Number     Number       Type             X           Y           Z" << std::endl;
        out << "---------------------------------------------------------------------" << std::endl;
        
        for (size_t i = 0; i < step.atoms.size(); i++) {
            const auto& atom = step.atoms[i];
            out << std::setw(7) << (i + 1)
                << std::setw(11) << atom.atomicNumber
                << std::setw(12) << 0
                << "    "
                << std::fixed << std::setprecision(6)
                << std::setw(12) << atom.x
                << std::setw(12) << atom.y
                << std::setw(12) << atom.z << std::endl;
        }
        out << "---------------------------------------------------------------------" << std::endl;
        
        // Write energy
        out << std::endl;
        out << " SCF Done:  E(theory) = " << std::fixed << std::setprecision(9) 
            << step.energy << std::endl;
        
        // Write convergence info if this is an optimization
        if (hasOpt) {
            out << std::endl;
            out << " GradGradGradGradGradGradGradGradGradGradGradGradGradGradGradGradGradGrad" << std::endl;
            out << " Step number" << std::setw(4) << step.stepNumber << std::endl;
            out << "         Item               Value     Threshold  Converged?" << std::endl;
            
            // Convergence criteria (using typical BDF values)
            double tolRMSG = 3.0e-4, tolMAXG = 4.5e-4, tolRMSD = 1.2e-3, tolMAXD = 1.8e-3;
            
            out << " Maximum Force       " << std::fixed << std::setprecision(6)
                << std::setw(13) << step.maxGrad 
                << std::setw(13) << tolMAXG << "     "
                << (step.maxGrad < tolMAXG ? "YES" : "NO") << std::endl;
            out << " RMS     Force       " 
                << std::setw(13) << step.rmsGrad 
                << std::setw(13) << tolRMSG << "     "
                << (step.rmsGrad < tolRMSG ? "YES" : "NO") << std::endl;
            out << " Maximum Displacement" 
                << std::setw(13) << step.maxStep 
                << std::setw(13) << tolMAXD << "     "
                << (step.maxStep < tolMAXD ? "YES" : "NO") << std::endl;
            out << " RMS     Displacement" 
                << std::setw(13) << step.rmsStep 
                << std::setw(13) << tolRMSD << "     "
                << (step.rmsStep < tolRMSD ? "YES" : "NO") << std::endl;
            out << " GradGradGradGradGradGradGradGradGradGradGradGradGradGradGradGradGradGrad" << std::endl;
        }
    }
    
    void writeFrequencies(std::ofstream& out) {
        out << std::endl;
        out << " Harmonic frequencies (cm**-1), IR intensities (KM/Mole), Raman scattering" << std::endl;
        out << " activities (A**4/AMU), depolarization ratios for plane and unpolarized" << std::endl;
        out << " incident light, reduced masses (AMU), force constants (mDyne/A)," << std::endl;
        out << " and normal coordinates:" << std::endl;
        
        int nAtoms = optSteps.empty() ? 0 : optSteps.back().atoms.size();
        int nFreqs = frequencies.size();
        int nFrames = (nFreqs + 2) / 3; // 3 frequencies per frame
        
        for (int iframe = 0; iframe < nFrames; iframe++) {
            int istart = iframe * 3;
            int iend = std::min(istart + 3, nFreqs);
            
            // Row 1: Write frequency indices
            for (int i = istart; i < iend; i++) {
                out << std::setw(23) << (i + 1);
            }
            out << std::endl;
            
            // Row 2: Write irreps (使用提取的对称性信息)
            for (int i = istart; i < iend; i++) {
                out << std::setw(22) << "" << frequencies[i].irrep;  // 使用实际的对称性
            }
            out << std::endl;
            
            // Row 3: Write frequencies
            for (int icol = 0; icol < (iend - istart); icol++) {
                int i = istart + icol;
                if (icol == 0) {
                    out << " Frequencies --" << std::fixed << std::setprecision(4) << std::setw(12) << frequencies[i].frequency;
                } else {
                    out << std::setw(11) << "" << std::fixed << std::setprecision(4) << std::setw(12) << frequencies[i].frequency;
                }
            }
            out << std::endl;
            
            // Row 4: Write IR intensities
            for (int icol = 0; icol < (iend - istart); icol++) {
                int i = istart + icol;
                if (icol == 0) {
                    out << " IR Inten    --" << std::fixed << std::setprecision(4) << std::setw(12) << frequencies[i].irIntensity;
                } else {
                    out << std::setw(11) << "" << std::fixed << std::setprecision(4) << std::setw(12) << frequencies[i].irIntensity;
                }
            }
            out << std::endl;
            
            // Row 5: Write header for atomic displacements
            for (int icol = 0; icol < (iend - istart); icol++) {
                if (icol == 0) {
                    out << "  Atom  AN      X      Y      Z  ";
                } else {
                    out << "      X      Y      Z  ";
                }
            }
            out << std::endl;
            
            // Rows 6+: Write atomic displacements
            for (int iatom = 0; iatom < nAtoms; iatom++) {
                if (!optSteps.empty()) {
                    int atomicNumber = optSteps.back().atoms[iatom].atomicNumber;
                    
                    for (int icol = 0; icol < (iend - istart); icol++) {
                        int i = istart + icol;
                        if (icol == 0) {
                            // First column: atom info + displacement
                            out << std::setw(6) << (iatom + 1) << std::setw(4) << atomicNumber << "  ";
                            if (i < frequencies.size() && 
                                iatom < frequencies[i].displacements.size() &&
                                frequencies[i].displacements[iatom].size() >= 3) {
                                out << std::fixed << std::setprecision(2)
                                    << std::setw(7) << frequencies[i].displacements[iatom][0]
                                    << std::setw(7) << frequencies[i].displacements[iatom][1]
                                    << std::setw(7) << frequencies[i].displacements[iatom][2];
                            } else {
                                out << "   0.00   0.00   0.00";
                            }
                        } else {
                            // Other columns: just displacement
                            out << "  ";
                            if (i < frequencies.size() && 
                                iatom < frequencies[i].displacements.size() &&
                                frequencies[i].displacements[iatom].size() >= 3) {
                                out << std::fixed << std::setprecision(2)
                                    << std::setw(7) << frequencies[i].displacements[iatom][0]
                                    << std::setw(7) << frequencies[i].displacements[iatom][1]
                                    << std::setw(7) << frequencies[i].displacements[iatom][2];
                            } else {
                                out << "   0.00   0.00   0.00";
                            }
                        }
                    }
                    out << std::endl;
                }
            }
        }
        
        // 输出热力学数据（修复后的版本）
        if (thermoData.hasData) {
            out << std::endl;
            out << " Temperature" << std::fixed << std::setprecision(3) << std::setw(10) << thermoData.temperature 
                << " Kelvin.  Pressure" << std::setprecision(5) << std::setw(10) << thermoData.pressure << " Atm." << std::endl;
            out << " Zero-point correction=                           " << std::setprecision(6) << std::setw(8) << thermoData.zpe << " Hartree" << std::endl;
            out << " Thermal correction to Energy=                    " << std::setprecision(6) << std::setw(8) << thermoData.thermalEnergyCorr << std::endl;
            out << " Thermal correction to Enthalpy=                  " << std::setprecision(6) << std::setw(8) << thermoData.thermalEnthalpyCorr << std::endl;
            out << " Thermal correction to Gibbs Free Energy=        " << std::setprecision(6) << std::setw(8) << thermoData.thermalGibbsCorr << std::endl;
            out << " Electronic energy=                          " << std::setprecision(6) << std::setw(20) << thermoData.electronicEnergy << std::endl;
            out << " Sum of electronic and zero-point Energies=  " << std::setprecision(6) << std::setw(20) << (thermoData.electronicEnergy + thermoData.zpe) << std::endl;
            out << " Sum of electronic and thermal Energies=     " << std::setprecision(6) << std::setw(20) << (thermoData.electronicEnergy + thermoData.thermalEnergyCorr) << std::endl;
            out << " Sum of electronic and thermal Enthalpies=   " << std::setprecision(6) << std::setw(20) << (thermoData.electronicEnergy + thermoData.thermalEnthalpyCorr) << std::endl;
            out << " Sum of electronic and thermal Free Energies=" << std::setprecision(6) << std::setw(20) << (thermoData.electronicEnergy + thermoData.thermalGibbsCorr) << std::endl;
        }
    }

public:
    BDFakeG(bool debug = false) : hasOpt(false), hasFreq(false), debugMode(debug) {
        initElementMap();
    }
    
    bool parseBDFFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Cannot open file " << filename << std::endl;
            return false;
        }
        
        std::cout << "BDFakeG: Generate fake Gaussian output from BDF file" << std::endl;
        std::cout << "Author: Bane-Dysta" << std::endl;
        std::cout << "Processing file: " << filename << std::endl;
        
        // Check for optimization
        file.clear();
        file.seekg(0, std::ios::beg);
        if (findLineFromBeginning(file, "Geometry Optimization step")) {
            hasOpt = true;
            std::cout << "Found geometry optimization" << std::endl;
            parseOptimizationSteps(file);
        } else {
            // Single point calculation
            std::cout << "Single point calculation detected" << std::endl;
            parseSinglePoint(file);
        }
        
        // Parse frequencies
        parseFrequencies(file);
        
        // Parse thermodynamic data
        parseThermoData(file);
        
        file.close();
        return true;
    }
    
    void parseOptimizationSteps(std::ifstream& file) {
        file.clear();
        file.seekg(0, std::ios::beg);
        
        std::string line;
        while (std::getline(file, line)) {
            if (line.find("Geometry Optimization step :") != std::string::npos) {
                OptStep step;
                step.energy = 0.0; // Initialize
                step.rmsGrad = step.maxGrad = step.rmsStep = step.maxStep = 0.0;
                step.converged = false;
                
                // Extract step number
                size_t pos = line.find(":");
                if (pos != std::string::npos) {
                    std::string stepStr = trim(line.substr(pos + 1));
                    step.stepNumber = std::stoi(stepStr);
                    if (debugMode) {
                        std::cout << "Processing optimization step " << step.stepNumber << std::endl;
                    }
                }
                
                // Parse geometry for this step (continue from current position)
                parseGeometryStep(file, step);
                
                // Parse convergence for this step (continue from current position)
                parseConvergence(file, step);
                
                if (!step.atoms.empty()) {
                    optSteps.push_back(step);
                    if (debugMode) {
                        std::cout << "Added step " << step.stepNumber << " with " << step.atoms.size() 
                                 << " atoms, energy = " << step.energy << std::endl;
                    }
                }
            }
        }
        
        std::cout << "Total optimization steps: " << optSteps.size() << std::endl;
    }
    
    void parseSinglePoint(std::ifstream& file) {
        file.clear();
        file.seekg(0, std::ios::beg);
        
        OptStep step;
        step.stepNumber = 1;
        step.converged = true;
        
        parseGeometryStep(file, step);
        
        if (!step.atoms.empty()) {
            optSteps.push_back(step);
        }
    }
    
    void generateOutput(const std::string& inputFile) {
        size_t dotPos = inputFile.find_last_of('.');
        std::string outputFile = inputFile.substr(0, dotPos) + "_fake.log";
        
        writeGaussianOutput(outputFile);
        
        std::cout << std::endl << "Fake Gaussian output written to: " << outputFile << std::endl;
    }
};

int main(int argc, char* argv[]) {
    std::string inputFile;
    bool debugMode = false;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--debug") {
            debugMode = true;
        } else if (inputFile.empty()) {
            inputFile = arg;
        }
    }
    
    if (inputFile.empty()) {
        std::cout << "Enter BDF output file path: ";
        std::getline(std::cin, inputFile);
    }
    
    BDFakeG converter(debugMode);
    if (converter.parseBDFFile(inputFile)) {
        converter.generateOutput(inputFile);
    } else {
        std::cerr << "Failed to process file" << std::endl;
        return 1;
    }
    
    return 0;
}