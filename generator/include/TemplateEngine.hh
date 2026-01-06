#pragma once

#include <string>
#include <map>

namespace geantcad {

class TemplateEngine {
public:
    TemplateEngine();
    ~TemplateEngine();
    
    std::string render(const std::string& templateStr, const std::map<std::string, std::string>& variables);
    
    // Render template and preserve user code regions from existing file
    // Returns rendered template with preserved regions inserted
    std::string renderWithPreservation(
        const std::string& templateStr,
        const std::map<std::string, std::string>& variables,
        const std::string& existingContent
    );
    
    // Extract user code from a region tag
    // Returns empty string if region not found
    std::string extractUserCode(const std::string& content, const std::string& tag);
    
private:
    // Implementation
};

} // namespace geantcad

