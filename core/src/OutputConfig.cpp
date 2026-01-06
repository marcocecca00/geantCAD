#include "OutputConfig.hh"
#include <sstream>

namespace geantcad {

OutputConfig::OutputConfig() {
    // Initialize default fields
    fields = {
        {"x", true},
        {"y", true},
        {"z", true},
        {"edep", true},
        {"event_id", true},
        {"track_id", true},
        {"volume_name", false},
        {"time", false},
        {"kinetic_energy", false}
    };
}

OutputConfig::~OutputConfig() {
}

std::string OutputConfig::getSchemaName() const {
    switch (schema) {
        case Schema::EventSummary: return "EventSummary";
        case Schema::StepHits: return "StepHits";
        case Schema::Custom: return "Custom";
        default: return "EventSummary";
    }
}

nlohmann::json OutputConfig::toJson() const {
    nlohmann::json j;
    j["root_enabled"] = rootEnabled;
    j["root_file_path"] = rootFilePath;
    j["schema"] = static_cast<int>(schema);
    j["per_event"] = perEvent;
    j["save_frequency"] = saveFrequency;
    j["csv_fallback"] = csvFallback;
    j["compression"] = compression;
    
    // Fields
    j["fields"] = nlohmann::json::object();
    j["fields"]["x"] = fieldX;
    j["fields"]["y"] = fieldY;
    j["fields"]["z"] = fieldZ;
    j["fields"]["edep"] = fieldEdep;
    j["fields"]["event_id"] = fieldEventId;
    j["fields"]["track_id"] = fieldTrackId;
    j["fields"]["volume_name"] = fieldVolumeName;
    j["fields"]["time"] = fieldTime;
    j["fields"]["kinetic_energy"] = fieldKineticEnergy;
    
    return j;
}

void OutputConfig::fromJson(const nlohmann::json& j) {
    if (j.contains("root_enabled")) {
        rootEnabled = j["root_enabled"];
    }
    if (j.contains("root_file_path")) {
        rootFilePath = j["root_file_path"];
    }
    if (j.contains("schema")) {
        schema = static_cast<Schema>(j["schema"]);
    }
    if (j.contains("per_event")) {
        perEvent = j["per_event"];
    }
    if (j.contains("save_frequency")) {
        saveFrequency = j["save_frequency"];
    }
    if (j.contains("csv_fallback")) {
        csvFallback = j["csv_fallback"];
    }
    if (j.contains("compression")) {
        compression = j["compression"];
    }
    
    // Fields
    if (j.contains("fields")) {
        const auto& fieldsJson = j["fields"];
        if (fieldsJson.contains("x")) fieldX = fieldsJson["x"];
        if (fieldsJson.contains("y")) fieldY = fieldsJson["y"];
        if (fieldsJson.contains("z")) fieldZ = fieldsJson["z"];
        if (fieldsJson.contains("edep")) fieldEdep = fieldsJson["edep"];
        if (fieldsJson.contains("event_id")) fieldEventId = fieldsJson["event_id"];
        if (fieldsJson.contains("track_id")) fieldTrackId = fieldsJson["track_id"];
        if (fieldsJson.contains("volume_name")) fieldVolumeName = fieldsJson["volume_name"];
        if (fieldsJson.contains("time")) fieldTime = fieldsJson["time"];
        if (fieldsJson.contains("kinetic_energy")) fieldKineticEnergy = fieldsJson["kinetic_energy"];
    }
}

std::string OutputConfig::generateOutputCode() const {
    std::ostringstream oss;
    
    oss << "    // Output configuration:\n";
    oss << "    // Schema: " << getSchemaName() << "\n";
    oss << "    // ROOT enabled: " << (rootEnabled ? "yes" : "no") << "\n";
    oss << "    // File: " << rootFilePath << "\n";
    
    if (rootEnabled) {
        oss << "    // ROOT output initialization would go here\n";
        oss << "    // Example: TFile* file = new TFile(\"" << rootFilePath << "\", \"RECREATE\");\n";
        oss << "    // Example: TTree* tree = new TTree(\"events\", \"Event data\");\n";
    }
    
    return oss.str();
}

std::string OutputConfig::generateEventActionCode() const {
    std::ostringstream oss;
    
    oss << "    // Collect hit collections\n";
    oss << "    G4HCofThisEvent* hce = event->GetHCofThisEvent();\n";
    oss << "    if (!hce) return;\n\n";
    
    // Generate code to access hit collections based on schema
    if (schema == Schema::EventSummary) {
        oss << "    // Event summary mode: aggregate data per event\n";
        oss << "    // ==== USER CODE BEGIN EventSummary\n";
        oss << "    // Collect hits from all collections and aggregate\n";
        oss << "    // ==== USER CODE END EventSummary\n";
    } else if (schema == Schema::StepHits) {
        oss << "    // Step hits mode: save each step\n";
        oss << "    // ==== USER CODE BEGIN StepHits\n";
        oss << "    // Iterate through all hit collections and save steps\n";
        oss << "    // ==== USER CODE END StepHits\n";
    }
    
    if (rootEnabled) {
        oss << "\n    // Save to ROOT file\n";
        oss << "    // ==== USER CODE BEGIN ROOTOutput\n";
        oss << "    // tree->Fill(); // Fill ROOT tree\n";
        oss << "    // ==== USER CODE END ROOTOutput\n";
    }
    
    return oss.str();
}

std::string OutputConfig::generateRunActionCode() const {
    std::ostringstream oss;
    
    oss << "    // Output file setup\n";
    if (rootEnabled) {
        oss << "    // ROOT file: " << rootFilePath << "\n";
        oss << "    // ==== USER CODE BEGIN ROOTSetup\n";
        oss << "    // TFile* file = new TFile(\"" << rootFilePath << "\", \"RECREATE\");\n";
        oss << "    // TTree* tree = new TTree(\"events\", \"Event data\");\n";
        oss << "    // ==== USER CODE END ROOTSetup\n";
    } else {
        oss << "    // CSV output: " << rootFilePath << "\n";
        oss << "    // ==== USER CODE BEGIN CSVSetup\n";
        oss << "    // std::ofstream csvFile(\"" << rootFilePath << "\");\n";
        oss << "    // ==== USER CODE END CSVSetup\n";
    }
    
    return oss.str();
}

} // namespace geantcad

