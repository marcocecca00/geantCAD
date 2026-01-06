#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace geantcad {

/**
 * Output configuration for Geant4 output (ROOT/CSV)
 */
class OutputConfig {
public:
    OutputConfig();
    ~OutputConfig();
    
    // ROOT output
    bool rootEnabled = false;
    std::string rootFilePath = "output.root";
    
    // Schema type
    enum class Schema {
        EventSummary,  // One entry per event
        StepHits,      // One entry per step
        Custom         // Custom fields
    };
    Schema schema = Schema::EventSummary;
    
    // Fields to save
    struct Field {
        std::string name;
        bool enabled = true;
    };
    std::vector<Field> fields;
    
    // Common fields
    bool fieldX = true;
    bool fieldY = true;
    bool fieldZ = true;
    bool fieldEdep = true;
    bool fieldEventId = true;
    bool fieldTrackId = true;
    bool fieldVolumeName = false;
    bool fieldTime = false;
    bool fieldKineticEnergy = false;
    
    // Output mode
    bool perEvent = true;  // true = per-event, false = per-step
    int saveFrequency = 1; // Save every N events
    
    // Options
    bool csvFallback = true;  // Fallback to CSV if ROOT unavailable
    bool compression = false; // Enable compression
    
    // Serialization
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& j);
    
    // Generate output code for Geant4
    std::string generateOutputCode() const;
    
    // Helper: get schema name
    std::string getSchemaName() const;
};

} // namespace geantcad

