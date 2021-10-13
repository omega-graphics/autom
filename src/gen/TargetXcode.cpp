#include "Gen.h"
#include "Diagnostic.h"

#include <fstream>
#include <queue>


namespace autom {

    /**
      @brief Generates an Xcode Object ID.
      @param[in] type The object type string.
      @param[in] otherData Other string data to include in the generation of the id.
      @returns std::string
     */
    inline std::string generateObjectID(autom::StrRef type,autom::StrRef otherData){
        SHA256Hash shaHash;
        shaHash.addData((void *)type.data(),type.size());
        shaHash.addData((void *)otherData.data(),otherData.size());
        std::string out;
        shaHash.getResultAsHex(out);
        
        /// Each byte encoded in hexadecimal is two digits in length,
        /// therefore the number of characters needed to be extracted is double that of the byte count.
        return out.substr(0,24);
    };
    
    
    
auto pbxHeader = R"(// !$*UTF8*$!
    {
        archiveVersion = 1;
        classes = {
        };
        objectVersion = 46;
        objects = {)";
    
    struct PBXAggregateTargetDesc {
        
    };

    struct PBXNativeTargetDesc {
        
    };

    struct PBXFileReferenceDesc {
        
    };
    
    struct PBXBuildFileDesc {
        
    };

    struct PBXGroupDesc {
        
    };

    struct PBXFrameworksBuildPhaseDesc {
        
    };
    
    struct PBXSourcesBuildPhaseDesc {
        
    };

    struct PBXTargetDependencyDesc {
        
    };

    struct PBXProjectDesc {
        
    };

    struct XCBuildConfigurationDesc {
        
    };

    struct XCConfigurationListDesc {
        
    };
    
    class XcodeGen : public Gen {
        
        OutputTargetOpts & outputOpts;
        
        GenXcodeOpts & genOpts;
        
        std::ofstream pbxprojOut;
        
        typedef enum : int {
            PBXAggregateTarget,
            PBXNativeTarget,
            PBXBuildFile,
            PBXBuildPhase
        } XcodeObjectType;
        
        
        std::queue<PBXNativeTargetDesc> nativeTargets;
        
        std::queue<PBXBuildFileDesc> buildFiles;
        
    public:
        
        explicit XcodeGen(OutputTargetOpts & outputOpts,GenXcodeOpts & genOpts):outputOpts(outputOpts),genOpts(genOpts){
        
        }
        
        void configGenContext() override {
            auto projectDir = std::string(context->projectDesc.name) + ".xcodeproj";
           
            std::filesystem::create_directory(std::filesystem::path(context->outputDir.data()).append(projectDir));
            
            pbxprojOut.open(std::filesystem::path(context->outputDir.data()).append(projectDir).append("project.pbxproj"));
            pbxprojOut << pbxHeader;
        }
        
        void consumeToolchainDefaults(ToolchainDefaults &conf) override {
            
        }
        
        inline void writeXcodeObject(XcodeObjectType type){
            
        }
        
        void consumeTarget(std::shared_ptr<Target> & target) override {
            
        }
        
        bool supportsCustomToolchainRules() override {
            return false;
        }
        
        void genToolchainRules(std::shared_ptr<Toolchain> &toolchain) override {
            
        }
        
        void writePBXSections(){
            
        }
        
        void finish() override {
            writePBXSections();
            pbxprojOut << "}";
            pbxprojOut.close();
        }
    };
    
    Gen* TargetXcode(OutputTargetOpts & outputOpts,GenXcodeOpts & genOpts){
        return new XcodeGen(outputOpts,genOpts);
    }
    
    
}
