#include "TemplateEngine.hh"
#include <regex>
#include <sstream>

namespace geantcad {

TemplateEngine::TemplateEngine() {
}

TemplateEngine::~TemplateEngine() {
}

std::string TemplateEngine::render(const std::string& templateStr, const std::map<std::string, std::string>& variables) {
    std::string result = templateStr;
    
    // Simple template replacement: {{variable}}
    for (const auto& pair : variables) {
        std::string pattern = "\\{\\{" + pair.first + "\\}\\}";
        std::regex re(pattern);
        result = std::regex_replace(result, re, pair.second);
    }
    
    return result;
}

std::string TemplateEngine::extractUserCode(const std::string& content, const std::string& tag) {
    std::string beginMarker = "// ==== USER CODE BEGIN " + tag;
    std::string endMarker = "// ==== USER CODE END " + tag;
    
    size_t beginPos = content.find(beginMarker);
    if (beginPos == std::string::npos) {
        return "";
    }
    
    // Find the end of the begin marker line
    size_t beginLineEnd = content.find('\n', beginPos);
    if (beginLineEnd == std::string::npos) {
        return "";
    }
    
    // Start after the begin marker line
    size_t codeStart = beginLineEnd + 1;
    
    // Find the end marker
    size_t endPos = content.find(endMarker, codeStart);
    if (endPos == std::string::npos) {
        return "";
    }
    
    // Extract the code between markers
    std::string userCode = content.substr(codeStart, endPos - codeStart);
    
    // Remove trailing newline if present
    if (!userCode.empty() && userCode.back() == '\n') {
        userCode.pop_back();
    }
    
    return userCode;
}

std::string TemplateEngine::renderWithPreservation(
    const std::string& templateStr,
    const std::map<std::string, std::string>& variables,
    const std::string& existingContent)
{
    // First render the template
    std::string rendered = render(templateStr, variables);
    
    // Extract and preserve user code regions
    std::regex regionRegex(R"(// ==== USER CODE BEGIN (\w+))");
    std::sregex_iterator iter(rendered.begin(), rendered.end(), regionRegex);
    std::sregex_iterator end;
    
    // Collect all regions first
    struct Region {
        std::string tag;
        size_t beginPos;
        size_t codeStart;
        size_t endPos;
        std::string userCode;
    };
    
    std::vector<Region> regions;
    for (auto it = iter; it != end; ++it) {
        std::smatch match = *it;
        std::string tag = match[1].str();
        size_t pos = match.position();
        
        // Find the corresponding end marker in rendered template
        std::string endMarker = "// ==== USER CODE END " + tag;
        size_t endPos = rendered.find(endMarker, pos);
        
        if (endPos != std::string::npos) {
            // Find end of begin marker line
            size_t beginLineEnd = rendered.find('\n', pos);
            if (beginLineEnd != std::string::npos) {
                size_t codeStart = beginLineEnd + 1;
                
                // Extract user code from existing file
                std::string userCode = extractUserCode(existingContent, tag);
                
                Region region;
                region.tag = tag;
                region.beginPos = pos;
                region.codeStart = codeStart;
                region.endPos = endPos;
                region.userCode = userCode;
                regions.push_back(region);
            }
        }
    }
    
    // Process regions in reverse order to preserve positions
    std::string result = rendered;
    for (auto it = regions.rbegin(); it != regions.rend(); ++it) {
        const Region& region = *it;
        if (!region.userCode.empty()) {
            // Replace the generated code with preserved user code
            std::string before = result.substr(0, region.codeStart);
            std::string after = result.substr(region.endPos);
            result = before + region.userCode + "\n" + after;
        }
    }
    
    return result;
}

} // namespace geantcad

