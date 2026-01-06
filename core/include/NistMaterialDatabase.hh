#pragma once

#include <string>
#include <vector>
#include <map>

namespace geantcad {

/**
 * NistMaterialDatabase provides access to G4NistManager materials.
 * Contains metadata about NIST materials for UI display and selection.
 */
class NistMaterialDatabase {
public:
    /**
     * Material category for UI organization
     */
    enum class Category {
        Elements,           // Individual elements (G4_H, G4_He, ...)
        Compounds,          // Simple compounds (G4_WATER, G4_AIR, ...)
        Biological,         // Biological materials (G4_BONE, G4_MUSCLE, ...)
        Scintillators,      // Scintillator materials (G4_CESIUM_IODIDE, ...)
        Gases,              // Gas materials
        Metals,             // Metallic materials
        Plastics,           // Plastic materials
        Shielding,          // Shielding materials (G4_LEAD, G4_CONCRETE, ...)
        Optical,            // Materials for optical simulations
        Custom              // User-defined
    };
    
    /**
     * Material information structure
     */
    struct MaterialInfo {
        std::string nistName;       // NIST name (e.g., "G4_AIR")
        std::string displayName;    // Human-readable name
        Category category;          // Category for UI
        double density;             // Density in g/cm³ (for display)
        std::string formula;        // Chemical formula
        std::string description;    // Brief description
    };
    
    /**
     * Get singleton instance
     */
    static NistMaterialDatabase& instance();
    
    /**
     * Get all available materials
     */
    const std::vector<MaterialInfo>& getAllMaterials() const { return materials_; }
    
    /**
     * Get materials by category
     */
    std::vector<MaterialInfo> getMaterialsByCategory(Category category) const;
    
    /**
     * Find material by NIST name
     */
    const MaterialInfo* findByNistName(const std::string& nistName) const;
    
    /**
     * Search materials by name or formula
     */
    std::vector<MaterialInfo> search(const std::string& query) const;
    
    /**
     * Get category name for display
     */
    static std::string getCategoryName(Category category);
    
    /**
     * Get all categories
     */
    static std::vector<Category> getAllCategories();

private:
    NistMaterialDatabase();
    void populateDatabase();
    
    std::vector<MaterialInfo> materials_;
    std::map<std::string, size_t> nameIndex_;
};

} // namespace geantcad

