#include "NistMaterialDatabase.hh"
#include <algorithm>
#include <cctype>

namespace geantcad {

NistMaterialDatabase& NistMaterialDatabase::instance() {
    static NistMaterialDatabase db;
    return db;
}

NistMaterialDatabase::NistMaterialDatabase() {
    populateDatabase();
}

void NistMaterialDatabase::populateDatabase() {
    // ===== ELEMENTS =====
    materials_.push_back({"G4_H", "Hydrogen", Category::Elements, 0.0000899, "H", "Hydrogen gas"});
    materials_.push_back({"G4_He", "Helium", Category::Elements, 0.000179, "He", "Helium gas"});
    materials_.push_back({"G4_Li", "Lithium", Category::Elements, 0.534, "Li", "Lithium metal"});
    materials_.push_back({"G4_Be", "Beryllium", Category::Elements, 1.848, "Be", "Beryllium metal"});
    materials_.push_back({"G4_B", "Boron", Category::Elements, 2.37, "B", "Boron"});
    materials_.push_back({"G4_C", "Carbon", Category::Elements, 2.0, "C", "Carbon (amorphous)"});
    materials_.push_back({"G4_N", "Nitrogen", Category::Elements, 0.001251, "N", "Nitrogen gas"});
    materials_.push_back({"G4_O", "Oxygen", Category::Elements, 0.001429, "O", "Oxygen gas"});
    materials_.push_back({"G4_F", "Fluorine", Category::Elements, 0.001696, "F", "Fluorine gas"});
    materials_.push_back({"G4_Ne", "Neon", Category::Elements, 0.0009, "Ne", "Neon gas"});
    materials_.push_back({"G4_Na", "Sodium", Category::Elements, 0.971, "Na", "Sodium metal"});
    materials_.push_back({"G4_Mg", "Magnesium", Category::Elements, 1.74, "Mg", "Magnesium metal"});
    materials_.push_back({"G4_Al", "Aluminum", Category::Elements, 2.699, "Al", "Aluminum metal"});
    materials_.push_back({"G4_Si", "Silicon", Category::Elements, 2.33, "Si", "Silicon crystal"});
    materials_.push_back({"G4_P", "Phosphorus", Category::Elements, 2.2, "P", "Phosphorus"});
    materials_.push_back({"G4_S", "Sulfur", Category::Elements, 2.0, "S", "Sulfur"});
    materials_.push_back({"G4_Cl", "Chlorine", Category::Elements, 0.003214, "Cl", "Chlorine gas"});
    materials_.push_back({"G4_Ar", "Argon", Category::Elements, 0.001784, "Ar", "Argon gas"});
    materials_.push_back({"G4_K", "Potassium", Category::Elements, 0.862, "K", "Potassium metal"});
    materials_.push_back({"G4_Ca", "Calcium", Category::Elements, 1.55, "Ca", "Calcium metal"});
    materials_.push_back({"G4_Ti", "Titanium", Category::Elements, 4.54, "Ti", "Titanium metal"});
    materials_.push_back({"G4_V", "Vanadium", Category::Elements, 6.11, "V", "Vanadium metal"});
    materials_.push_back({"G4_Cr", "Chromium", Category::Elements, 7.18, "Cr", "Chromium metal"});
    materials_.push_back({"G4_Mn", "Manganese", Category::Elements, 7.44, "Mn", "Manganese metal"});
    materials_.push_back({"G4_Fe", "Iron", Category::Elements, 7.874, "Fe", "Iron metal"});
    materials_.push_back({"G4_Co", "Cobalt", Category::Elements, 8.9, "Co", "Cobalt metal"});
    materials_.push_back({"G4_Ni", "Nickel", Category::Elements, 8.902, "Ni", "Nickel metal"});
    materials_.push_back({"G4_Cu", "Copper", Category::Elements, 8.96, "Cu", "Copper metal"});
    materials_.push_back({"G4_Zn", "Zinc", Category::Elements, 7.133, "Zn", "Zinc metal"});
    materials_.push_back({"G4_Ga", "Gallium", Category::Elements, 5.904, "Ga", "Gallium metal"});
    materials_.push_back({"G4_Ge", "Germanium", Category::Elements, 5.323, "Ge", "Germanium crystal"});
    materials_.push_back({"G4_As", "Arsenic", Category::Elements, 5.73, "As", "Arsenic"});
    materials_.push_back({"G4_Se", "Selenium", Category::Elements, 4.5, "Se", "Selenium"});
    materials_.push_back({"G4_Br", "Bromine", Category::Elements, 3.1028, "Br", "Bromine"});
    materials_.push_back({"G4_Kr", "Krypton", Category::Elements, 0.003733, "Kr", "Krypton gas"});
    materials_.push_back({"G4_Mo", "Molybdenum", Category::Elements, 10.22, "Mo", "Molybdenum metal"});
    materials_.push_back({"G4_Ag", "Silver", Category::Elements, 10.5, "Ag", "Silver metal"});
    materials_.push_back({"G4_Cd", "Cadmium", Category::Elements, 8.65, "Cd", "Cadmium metal"});
    materials_.push_back({"G4_Sn", "Tin", Category::Elements, 7.31, "Sn", "Tin metal"});
    materials_.push_back({"G4_I", "Iodine", Category::Elements, 4.93, "I", "Iodine"});
    materials_.push_back({"G4_Xe", "Xenon", Category::Elements, 0.005887, "Xe", "Xenon gas"});
    materials_.push_back({"G4_Cs", "Cesium", Category::Elements, 1.873, "Cs", "Cesium metal"});
    materials_.push_back({"G4_Ba", "Barium", Category::Elements, 3.5, "Ba", "Barium metal"});
    materials_.push_back({"G4_W", "Tungsten", Category::Elements, 19.3, "W", "Tungsten metal"});
    materials_.push_back({"G4_Pt", "Platinum", Category::Elements, 21.45, "Pt", "Platinum metal"});
    materials_.push_back({"G4_Au", "Gold", Category::Elements, 19.32, "Au", "Gold metal"});
    materials_.push_back({"G4_Pb", "Lead", Category::Elements, 11.35, "Pb", "Lead metal"});
    materials_.push_back({"G4_Bi", "Bismuth", Category::Elements, 9.747, "Bi", "Bismuth metal"});
    materials_.push_back({"G4_U", "Uranium", Category::Elements, 18.95, "U", "Uranium metal"});
    
    // ===== COMPOUNDS =====
    materials_.push_back({"G4_AIR", "Air", Category::Compounds, 0.001205, "N₂ + O₂", "Standard air at STP"});
    materials_.push_back({"G4_WATER", "Water", Category::Compounds, 1.0, "H₂O", "Liquid water"});
    materials_.push_back({"G4_WATER_VAPOR", "Water Vapor", Category::Gases, 0.000756, "H₂O", "Water vapor"});
    materials_.push_back({"G4_CARBON_DIOXIDE", "Carbon Dioxide", Category::Gases, 0.001977, "CO₂", "Carbon dioxide gas"});
    materials_.push_back({"G4_lAr", "Liquid Argon", Category::Compounds, 1.396, "Ar", "Liquid argon"});
    materials_.push_back({"G4_lKr", "Liquid Krypton", Category::Compounds, 2.418, "Kr", "Liquid krypton"});
    materials_.push_back({"G4_lXe", "Liquid Xenon", Category::Compounds, 2.953, "Xe", "Liquid xenon"});
    materials_.push_back({"G4_lN2", "Liquid Nitrogen", Category::Compounds, 0.807, "N₂", "Liquid nitrogen"});
    materials_.push_back({"G4_lO2", "Liquid Oxygen", Category::Compounds, 1.141, "O₂", "Liquid oxygen"});
    materials_.push_back({"G4_lH2", "Liquid Hydrogen", Category::Compounds, 0.0708, "H₂", "Liquid hydrogen"});
    materials_.push_back({"G4_Galactic", "Galactic Vacuum", Category::Compounds, 1e-25, "", "Ultra-high vacuum"});
    
    // ===== METALS =====
    materials_.push_back({"G4_STAINLESS-STEEL", "Stainless Steel", Category::Metals, 8.0, "Fe-Cr-Ni", "316L stainless steel"});
    materials_.push_back({"G4_BRASS", "Brass", Category::Metals, 8.52, "Cu-Zn", "Standard brass"});
    materials_.push_back({"G4_BRONZE", "Bronze", Category::Metals, 8.82, "Cu-Sn", "Standard bronze"});
    
    // ===== PLASTICS =====
    materials_.push_back({"G4_POLYETHYLENE", "Polyethylene", Category::Plastics, 0.94, "(C₂H₄)ₙ", "PE plastic"});
    materials_.push_back({"G4_POLYPROPYLENE", "Polypropylene", Category::Plastics, 0.9, "(C₃H₆)ₙ", "PP plastic"});
    materials_.push_back({"G4_POLYSTYRENE", "Polystyrene", Category::Plastics, 1.06, "(C₈H₈)ₙ", "PS plastic"});
    materials_.push_back({"G4_PLEXIGLASS", "Plexiglass (PMMA)", Category::Plastics, 1.19, "(C₅O₂H₈)ₙ", "Acrylic (PMMA)"});
    materials_.push_back({"G4_NYLON-6-6", "Nylon 6-6", Category::Plastics, 1.14, "(C₁₂H₂₂N₂O₂)ₙ", "Polyamide 6-6"});
    materials_.push_back({"G4_TEFLON", "Teflon (PTFE)", Category::Plastics, 2.2, "(C₂F₄)ₙ", "Polytetrafluoroethylene"});
    materials_.push_back({"G4_KAPTON", "Kapton", Category::Plastics, 1.42, "(C₂₂H₁₀N₂O₅)ₙ", "Polyimide film"});
    materials_.push_back({"G4_MYLAR", "Mylar", Category::Plastics, 1.4, "(C₁₀H₈O₄)ₙ", "PET film"});
    materials_.push_back({"G4_LUCITE", "Lucite", Category::Plastics, 1.19, "(C₅O₂H₈)ₙ", "Polymethyl methacrylate"});
    materials_.push_back({"G4_PVC", "PVC", Category::Plastics, 1.3, "(C₂H₃Cl)ₙ", "Polyvinyl chloride"});
    
    // ===== SCINTILLATORS =====
    materials_.push_back({"G4_SODIUM_IODIDE", "Sodium Iodide", Category::Scintillators, 3.67, "NaI", "NaI(Tl) scintillator"});
    materials_.push_back({"G4_CESIUM_IODIDE", "Cesium Iodide", Category::Scintillators, 4.51, "CsI", "CsI(Tl) scintillator"});
    materials_.push_back({"G4_BGO", "BGO", Category::Scintillators, 7.13, "Bi₄Ge₃O₁₂", "Bismuth germanate"});
    materials_.push_back({"G4_LYSO", "LYSO", Category::Scintillators, 7.1, "Lu₂SiO₅", "Lutetium-yttrium oxyorthosilicate"});
    materials_.push_back({"G4_PbWO4", "Lead Tungstate", Category::Scintillators, 8.28, "PbWO₄", "PWO crystal"});
    materials_.push_back({"G4_ANTHRACENE", "Anthracene", Category::Scintillators, 1.25, "C₁₄H₁₀", "Organic scintillator"});
    materials_.push_back({"G4_STILBENE", "Stilbene", Category::Scintillators, 0.9707, "C₁₄H₁₂", "Organic scintillator"});
    materials_.push_back({"G4_PLASTIC_SC_VINYLTOLUENE", "Plastic Scintillator", Category::Scintillators, 1.032, "", "Vinyltoluene based"});
    
    // ===== BIOLOGICAL =====
    materials_.push_back({"G4_BONE_COMPACT_ICRU", "Compact Bone", Category::Biological, 1.85, "", "Cortical bone (ICRU)"});
    materials_.push_back({"G4_BONE_CORTICAL_ICRP", "Cortical Bone", Category::Biological, 1.92, "", "Cortical bone (ICRP)"});
    materials_.push_back({"G4_MUSCLE_SKELETAL_ICRP", "Skeletal Muscle", Category::Biological, 1.04, "", "Skeletal muscle"});
    materials_.push_back({"G4_MUSCLE_STRIATED_ICRU", "Striated Muscle", Category::Biological, 1.04, "", "Striated muscle"});
    materials_.push_back({"G4_ADIPOSE_TISSUE_ICRP", "Adipose Tissue", Category::Biological, 0.95, "", "Fat tissue"});
    materials_.push_back({"G4_BRAIN_ICRP", "Brain", Category::Biological, 1.03, "", "Brain tissue"});
    materials_.push_back({"G4_LUNG_ICRP", "Lung", Category::Biological, 1.05, "", "Lung tissue (ICRP)"});
    materials_.push_back({"G4_TISSUE_SOFT_ICRP", "Soft Tissue", Category::Biological, 1.0, "", "Soft tissue (ICRP)"});
    materials_.push_back({"G4_SKIN_ICRP", "Skin", Category::Biological, 1.09, "", "Skin (ICRP)"});
    materials_.push_back({"G4_BLOOD_ICRP", "Blood", Category::Biological, 1.06, "", "Blood (ICRP)"});
    materials_.push_back({"G4_EYE_LENS_ICRP", "Eye Lens", Category::Biological, 1.07, "", "Eye lens tissue"});
    
    // ===== SHIELDING =====
    materials_.push_back({"G4_CONCRETE", "Concrete", Category::Shielding, 2.3, "", "Standard concrete"});
    materials_.push_back({"G4_BARITE", "Barite Concrete", Category::Shielding, 3.5, "BaSO₄", "High-density concrete"});
    materials_.push_back({"G4_PARAFFIN", "Paraffin", Category::Shielding, 0.93, "CₙH₂ₙ₊₂", "Neutron moderator"});
    materials_.push_back({"G4_BORON_CARBIDE", "Boron Carbide", Category::Shielding, 2.52, "B₄C", "Neutron absorber"});
    materials_.push_back({"G4_GRAPHITE", "Graphite", Category::Shielding, 2.21, "C", "Reactor-grade graphite"});
    materials_.push_back({"G4_LITHIUM_FLUORIDE", "Lithium Fluoride", Category::Shielding, 2.635, "LiF", "Neutron absorber"});
    
    // ===== OPTICAL =====
    materials_.push_back({"G4_GLASS_PLATE", "Glass", Category::Optical, 2.4, "SiO₂", "Plate glass"});
    materials_.push_back({"G4_SILICON_DIOXIDE", "Silica", Category::Optical, 2.2, "SiO₂", "Fused silica (quartz)"});
    materials_.push_back({"G4_LITHIUM_OXIDE", "Lithium Oxide", Category::Optical, 2.013, "Li₂O", "Glass component"});
    materials_.push_back({"G4_BORON_OXIDE", "Boron Oxide", Category::Optical, 1.812, "B₂O₃", "Glass component"});
    
    // ===== GASES =====
    materials_.push_back({"G4_METHANE", "Methane", Category::Gases, 0.000717, "CH₄", "Methane gas"});
    materials_.push_back({"G4_ETHANE", "Ethane", Category::Gases, 0.001356, "C₂H₆", "Ethane gas"});
    materials_.push_back({"G4_PROPANE", "Propane", Category::Gases, 0.001879, "C₃H₈", "Propane gas"});
    materials_.push_back({"G4_BUTANE", "Butane", Category::Gases, 0.00249, "C₄H₁₀", "Butane gas"});
    materials_.push_back({"G4_AMMONIA", "Ammonia", Category::Gases, 0.000826, "NH₃", "Ammonia gas"});
    
    // Build name index
    for (size_t i = 0; i < materials_.size(); ++i) {
        nameIndex_[materials_[i].nistName] = i;
    }
}

std::vector<NistMaterialDatabase::MaterialInfo> NistMaterialDatabase::getMaterialsByCategory(Category category) const {
    std::vector<MaterialInfo> result;
    for (const auto& mat : materials_) {
        if (mat.category == category) {
            result.push_back(mat);
        }
    }
    return result;
}

const NistMaterialDatabase::MaterialInfo* NistMaterialDatabase::findByNistName(const std::string& nistName) const {
    auto it = nameIndex_.find(nistName);
    if (it != nameIndex_.end()) {
        return &materials_[it->second];
    }
    return nullptr;
}

std::vector<NistMaterialDatabase::MaterialInfo> NistMaterialDatabase::search(const std::string& query) const {
    std::vector<MaterialInfo> result;
    
    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
    
    for (const auto& mat : materials_) {
        std::string lowerName = mat.displayName;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        
        std::string lowerNist = mat.nistName;
        std::transform(lowerNist.begin(), lowerNist.end(), lowerNist.begin(), ::tolower);
        
        std::string lowerFormula = mat.formula;
        std::transform(lowerFormula.begin(), lowerFormula.end(), lowerFormula.begin(), ::tolower);
        
        if (lowerName.find(lowerQuery) != std::string::npos ||
            lowerNist.find(lowerQuery) != std::string::npos ||
            lowerFormula.find(lowerQuery) != std::string::npos) {
            result.push_back(mat);
        }
    }
    
    return result;
}

std::string NistMaterialDatabase::getCategoryName(Category category) {
    switch (category) {
        case Category::Elements: return "Elements";
        case Category::Compounds: return "Compounds";
        case Category::Biological: return "Biological";
        case Category::Scintillators: return "Scintillators";
        case Category::Gases: return "Gases";
        case Category::Metals: return "Metals";
        case Category::Plastics: return "Plastics";
        case Category::Shielding: return "Shielding";
        case Category::Optical: return "Optical";
        case Category::Custom: return "Custom";
        default: return "Unknown";
    }
}

std::vector<NistMaterialDatabase::Category> NistMaterialDatabase::getAllCategories() {
    return {
        Category::Elements,
        Category::Compounds,
        Category::Gases,
        Category::Metals,
        Category::Plastics,
        Category::Scintillators,
        Category::Biological,
        Category::Shielding,
        Category::Optical
    };
}

} // namespace geantcad

