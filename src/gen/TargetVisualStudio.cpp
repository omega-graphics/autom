#include <fstream>
#include "Gen.h"

namespace autom {

#define VCXPROJ_PROJECT "<Project DefaultTargets=\"Build\" ToolsVersion=\"4.0\" xmlns='http://schemas.microsoft.com/developer/msbuild/2003'>"
#define VCXPROJ_PROJECT_END "</Project>"
#define VCXPROJ_ITEMGROUP(body) "<ItemGroup "#body" >"
#define VCXPROJ_ITEMGROUP_END "</ItemGroup>"
#define VCXPROJ_CLCOMPILE(srcs) "<ClCompile Include=\""#srcs"\"/>"

    class GenVisualStudio : public Gen {
        OutputTargetOpts & outputOpts;
        GenVisualStudioOpts & opts;
     
        std::ofstream solutionOut;
    public:
        GenVisualStudio(
                OutputTargetOpts & outputOpts,
                GenVisualStudioOpts & opts):
                outputOpts(outputOpts),opts(opts){

        };
        
        void configGenContext() override {
            solutionOut.open(std::filesystem::path(context->outputDir.data()).append(context->projectDesc.name).concat(".sln"));
        }
        
        void consumeToolchainDefaults(ToolchainDefaults &conf) override {
            
        }
        
        bool supportsCustomToolchainRules() override {
            return true;
        }
        void genToolchainRules(std::shared_ptr<Toolchain> &toolchain) override {

        }
        void consumeTarget(std::shared_ptr<Target> & target) override {
            std::ofstream vcxprojOut {std::filesystem::path(context->outputDir.data()).append(std::string(target->name->value()) + ".vcxproj"),std::ios::out};
            vcxprojOut << VCXPROJ_PROJECT;



            vcxprojOut << VCXPROJ_PROJECT_END;
        }
        void finish() override {
            solutionOut.close();
        }
    };

    Gen *TargetVisualStudio(OutputTargetOpts & outputOpts,GenVisualStudioOpts & opts){
        return new GenVisualStudio(outputOpts,opts);
    }


}
